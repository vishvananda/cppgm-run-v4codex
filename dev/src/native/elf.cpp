#include "native/encoding.h"
#include <algorithm>
#include <climits>
#include <cstring>
#include <cmath>
#include <fstream>
#include <sys/stat.h>
namespace native {
using namespace lowir_model;
static void append(std::vector<unsigned char>& v, std::uint64_t bits, unsigned bytes)
{
    for (unsigned n = 0; n < bytes; ++n) { v.push_back(bits); bits >>= 8; }
}
static std::uint64_t aligned(std::uint64_t n, unsigned a) { return (n+a-1)&~std::uint64_t(a-1); }
static void scalar_data(std::vector<unsigned char>& data, const DataItem& item, const Program& p)
{
    if (item.type.floating()) {
        unsigned char bytes[16] = {};
        auto value = native::Operand::floating(item.value,item.type,&p);
        std::memcpy(bytes,&value.bits,8); std::memcpy(bytes+8,&value.displacement,2);
        data.insert(data.end(),bytes,bytes+item.type.bytes());
    } else {
        require(scalar_integer(item.type) || item.type == Type::I128, "native initializer type invalid");
        append(data,item.value.data.integer,std::min(8u,item.type.bytes()));
        if (item.type == Type::I128) append(data,item.value.integer_high(),8);
    }
}
void encode_data(const lowir_model::Program& p, Image& image)
{
    for (unsigned lane = 0; lane < 2; ++lane) for (const auto& g : p.globals) {
        bool tls = p.symbols[g.symbol.index-1].metadata.storage == GSM_THREAD_LOCAL;
        if (tls != bool(lane)) continue;
        if (tls) { image.has_tls = true; image.tls_targets[g.symbol.index] = g.symbol.index; }
        if (g.declaration) continue;
        bool explicit_layout = g.structured && g.type.kind() == Type::Object;
        unsigned alignment = 1; bool typed = false;
        for (unsigned n = g.data.begin; n != g.data.end(); ++n)
            if (p.data[n].kind != DataItem::Zero) { alignment = std::max(alignment,p.data[n].type.alignment()); typed = true; }
        if (g.structured && !typed) alignment = 16;
        if (!g.structured) alignment = g.type == Type::I128 ? 16 : g.type.alignment();
        if (explicit_layout) {
            alignment = g.type.alignment();
            require(alignment <= 4096,"unsupported native global alignment");
        }
        image.data.resize(aligned(image.data.size(),alignment),0);
        image.symbols[g.symbol.index] = image.data.size(); image.defined[g.symbol.index] = true; image.data_symbols[g.symbol.index] = true;
        for (unsigned n = g.data.begin; n != g.data.end(); ++n) {
            const auto& item = p.data[n];
            if (item.kind == DataItem::Zero) {
                auto count = g.structured ? item.zero_bytes : g.type.bytes();
                require(count < 0x70000000, "native data too large"); image.data.resize(image.data.size()+count,0);
            } else {
                if (!explicit_layout) image.data.resize(aligned(image.data.size(),item.type.alignment()),0);
                if (item.kind == DataItem::Scalar) scalar_data(image.data,item,p);
                else {
                    Fixup fix; fix.kind = Fixup::AbsoluteSymbol; fix.offset = image.data.size();
                    fix.symbol = item.symbol.index; fix.addend = item.addend; fix.owner = g.symbol.index; image.data_fixups.push_back(fix);
                    append(image.data,0,8);
                }
            }
        }
        if (explicit_layout) require(image.data.size()-image.symbols[g.symbol.index] == g.type.bytes(), "native global layout extent mismatch");
    }
    if (image.has_tls) {
        image.data.resize(aligned(image.data.size(),16),0);
        unsigned id = image.runtime_begin+unsigned(RuntimeEntity::ThreadPointer);
        image.symbols[id] = image.data.size(); image.defined[id] = image.data_symbols[id] = true;
        Fixup fix; fix.kind = Fixup::AbsoluteSymbol; fix.symbol = id; fix.owner = id; fix.offset = image.data.size();
        image.data_fixups.push_back(fix); append(image.data,0,8);
    }
    for (const auto& f : p.functions) {
        auto target = p.symbols[f.symbol.index-1].metadata.tls_for;
        if (!target) continue;
        image.tls_targets[f.symbol.index] = target.index;
    }
    bool exceptions = false;
    for (const auto& i : p.instructions)
        exceptions |= i.opcode >= Opcode::EhTry && i.opcode <= Opcode::Resume;
    if (exceptions && !image.host) {
        for (unsigned k = 0; k < unsigned(RuntimeEntity::Count); ++k) {
            if (k == unsigned(RuntimeEntity::ThreadPointer)) continue;
            image.data.resize(aligned(image.data.size(),16),0);
            unsigned id = image.runtime_begin+k;
            image.symbols[id] = image.data.size(); image.defined[id] = image.data_symbols[id] = true;
            image.data.resize(image.data.size()+16,0);
        }
    }
}
static void patch(std::vector<unsigned char>& bytes, const std::vector<Fixup>& fixes, const Image& image,
    std::uint64_t code_address, std::uint64_t data_address, std::uint64_t source_address)
{
    for (const auto& fix : fixes) {
        require(image.defined.at(fix.symbol), "unresolved native symbol");
        std::uint64_t address = (image.data_symbols[fix.symbol] ? data_address : code_address) + image.symbols[fix.symbol] + fix.addend;
        unsigned width = fix.kind == Fixup::AbsoluteSymbol ? 8 : 4;
        if (width == 4) {
            if (fix.kind == Fixup::ThreadOffset)
                address = image.symbols[fix.symbol] + fix.addend - image.symbols[image.runtime_begin+unsigned(RuntimeEntity::ThreadPointer)];
            else if (fix.kind == Fixup::RelativeSymbol) address -= source_address + fix.end;
            else if (fix.kind == Fixup::Absolute32) {
                require(address <= UINT32_MAX,"native absolute reference out of range");
            }
            if (fix.kind != Fixup::Absolute32) require(std::int64_t(address) >= INT32_MIN && std::int64_t(address) <= INT32_MAX,"native reference out of range");
        }
        for (unsigned n = 0; n < width; ++n) bytes[fix.offset+n] = address >> (n*8);
    }
}
static void segment(std::vector<unsigned char>& header, unsigned flags, std::uint64_t offset, std::uint64_t size)
{
    append(header,1,4); append(header,flags,4); append(header,offset,8);
    append(header,0x400000+offset,8); append(header,0x400000+offset,8);
    append(header,size,8); append(header,size,8); append(header,0x1000,8);
}
void write_executable(Image& image, const std::string& path)
{
    const std::uint64_t code_offset = executable_code_offset;
    std::uint64_t data_offset = aligned(code_offset+image.code.size(),4096);
    patch(image.code,image.code_fixups,image,0x400000+code_offset,0x400000+data_offset,0x400000+code_offset);
    patch(image.data,image.data_fixups,image,0x400000+code_offset,0x400000+data_offset,0x400000+data_offset);
    std::vector<unsigned char> header = {0x7f,'E','L','F',2,1,1,0,0,0,0,0,0,0,0,0};
    append(header,2,2); append(header,62,2); append(header,1,4);
    append(header,0x400000+code_offset,8); append(header,64,8); append(header,0,8); append(header,0,4);
    append(header,64,2); append(header,56,2); append(header,2,2);
    append(header,0,2); append(header,0,2); append(header,0,2);
    segment(header,5,0,code_offset+image.code.size());
    segment(header,6,data_offset,image.data.size());
    std::ofstream out(path,std::ios::binary);
    require(bool(out), "cannot create native executable");
    out.write(reinterpret_cast<const char*>(header.data()),header.size());
    out.write(reinterpret_cast<const char*>(image.code.data()),image.code.size());
    std::vector<char> padding(data_offset-code_offset-image.code.size(),0);
    out.write(padding.data(),padding.size());
    out.write(reinterpret_cast<const char*>(image.data.data()),image.data.size()); out.close();
    require(bool(out), "cannot write native executable");
    require(chmod(path.c_str(),0755) == 0,"cannot make native output executable");
}
} // namespace native
