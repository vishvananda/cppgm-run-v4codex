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
        require(scalar_integer(item.type), "native wide initializer not implemented");
        append(data,item.value.data.integer,item.type.bytes());
    }
}
void encode_data(const lowir_model::Program& p, Image& image)
{
    for (const auto& g : p.globals) {
        if (g.declaration) continue;
        require(p.symbols[g.symbol.index-1].metadata.storage != GSM_THREAD_LOCAL, "native TLS not implemented");
        unsigned alignment = 1; bool typed = false;
        for (unsigned n = g.data.begin; n != g.data.end(); ++n)
            if (p.data[n].kind != DataItem::Zero) { alignment = std::max(alignment,p.data[n].type.alignment()); typed = true; }
        if (g.structured && !typed) alignment = 16;
        if (!g.structured) alignment = g.type.alignment();
        image.data.resize(aligned(image.data.size(),alignment),0);
        image.symbols[g.symbol.index] = image.data.size(); image.defined[g.symbol.index] = true; image.data_symbols[g.symbol.index] = true;
        for (unsigned n = g.data.begin; n != g.data.end(); ++n) {
            const auto& item = p.data[n];
            if (item.kind == DataItem::Zero) {
                auto count = g.structured ? item.zero_bytes : g.type.bytes();
                require(count < 0x70000000, "native data too large"); image.data.resize(image.data.size()+count,0);
            } else {
                image.data.resize(aligned(image.data.size(),item.type.alignment()),0);
                if (item.kind == DataItem::Scalar) scalar_data(image.data,item,p);
                else {
                    Fixup fix; fix.kind = Fixup::AbsoluteSymbol; fix.offset = image.data.size();
                    fix.symbol = item.symbol.index; fix.addend = item.addend; image.data_fixups.push_back(fix);
                    append(image.data,0,8);
                }
            }
        }
    }
}
static void patch(std::vector<unsigned char>& bytes, const std::vector<Fixup>& fixes, const Image& image,
    std::uint64_t code_address, std::uint64_t data_address)
{
    for (const auto& fix : fixes) {
        require(image.defined.at(fix.symbol), "unresolved native symbol");
        std::uint64_t address = (image.data_symbols[fix.symbol] ? data_address : code_address) + image.symbols[fix.symbol] + fix.addend;
        unsigned width = fix.kind == Fixup::AbsoluteSymbol ? 8 : 4;
        if (width == 4) {
            address -= code_address + fix.end;
            require(std::int64_t(address) >= INT32_MIN && std::int64_t(address) <= INT32_MAX,"native reference out of range");
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
    const std::uint64_t code_offset = 64+2*56;
    std::uint64_t data_offset = aligned(code_offset+image.code.size(),4096);
    patch(image.code,image.code_fixups,image,0x400000+code_offset,0x400000+data_offset);
    patch(image.data,image.data_fixups,image,0x400000+code_offset,0x400000+data_offset);
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
