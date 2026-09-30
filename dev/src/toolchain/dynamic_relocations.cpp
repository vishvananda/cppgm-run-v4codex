#include "toolchain/dynamic_image.h"
#include <algorithm>
#include <climits>
namespace cppgm { namespace toolchain {
using lowir_model::require;
void dynamic_number(std::vector<unsigned char>& v, std::uint64_t value, unsigned width) {
    for (unsigned i = 0; i < width; ++i) { v.push_back(value); value >>= 8; }
}
void dynamic_patch(std::vector<unsigned char>& v, std::size_t offset, std::uint64_t value, unsigned width) {
    require(offset <= v.size() && width <= v.size()-offset,"invalid dynamic patch range");
    for (unsigned i = 0; i < width; ++i) v[offset+i] = value>>(8*i);
}
DynamicImage::DynamicImage(native::Image& im, const std::vector<Symbol>& syms, const IdentifierTable& ids)
    : image(im), symbols(syms), names(ids), dynamic_ids(syms.size()), import_types(syms.size()), copies(syms.size()) {}
unsigned DynamicImage::string(const std::string& s) {
    auto offset = strings.size(); strings.insert(strings.end(),s.begin(),s.end()); strings.push_back(0); return offset;
}
std::uint64_t DynamicImage::address(unsigned id) const {
    require(image.defined.at(id),"unresolved dynamic image symbol");
    return base+(image.data_symbols[id] ? data_offset : code_offset)+image.symbols[id];
}
void DynamicImage::imports(const std::vector<DynamicImport>& imports)
{
    for (const auto& s : imports) import_types[s.symbol] = s.type;
    for (const auto* fixes : {&image.code_fixups,&image.data_fixups,&image.tls_fixups}) for (const auto& f : *fixes)
        if (import_types[f.symbol] == STT_OBJECT && f.kind != native::Fixup::AbsoluteSymbol) copies[f.symbol] = true;
    for (unsigned i = 1; i < symbols.size(); ++i) {
        if (!symbols[i].name || symbols[i].binding == ir_model::SBM_INTERNAL || (!image.defined[i] && !import_types[i])) continue;
        Elf64_Sym s = {}; auto text = names.spelling(symbols[i].name);
        s.st_name = string(std::string(text.data,text.size));
        s.st_info = ELF64_ST_INFO(symbols[i].binding == ir_model::SBM_WEAK ? STB_WEAK : STB_GLOBAL,
            import_types[i] ? import_types[i] == STT_GNU_IFUNC ? STT_FUNC : import_types[i] : symbols[i].object_type);
        s.st_size = symbols[i].size;
        dynamic_ids[i] = dynsym.size(); dynsym.push_back(s);
    }
    for (const auto& s : imports) {
        auto id = s.symbol; auto& sym = dynsym[dynamic_ids[id]]; sym.st_size = s.size;
        if (s.type == STT_OBJECT && !copies[id]) continue;
        image.data.resize((image.data.size()+15)&~std::size_t(15),0);
        auto offset = image.data.size();
        Elf64_Rela relocation = {}; relocation.r_offset = offset;
        relocation.r_info = ELF64_R_INFO(dynamic_ids[id],copies[id] ? R_X86_64_COPY : R_X86_64_64);
        relocations.push_back(relocation);
        if (copies[id]) {
            require(s.size && s.size < 0x70000000,"invalid host data import extent");
            image.data.resize(offset+s.size,0); image.symbols[id] = offset; image.data_symbols[id] = true;
        } else {
            image.data.resize(offset+8,0); image.symbols[id] = image.code.size(); image.data_symbols[id] = false;
            // Eager GOT binding preserves the host function's canonical address.
            // A direct call reaches it through a six-byte tail-jump veneer.
            image.code.push_back(0xff); image.code.push_back(0x25);
            stubs.push_back({image.code.size(),offset}); dynamic_number(image.code,0,4);
        }
        image.defined[id] = true;
    }
}
void DynamicImage::startup(unsigned main, unsigned runtime)
{
    using namespace native;
    std::vector<Instruction> code;
    auto emit = [&](Op op, Operand a, Operand b = Operand()) {
        Instruction i(op,Type::I64); i.args[0] = a; i.args[1] = b; i.count = b.kind == Operand::None ? 1 : 2; code.push_back(i);
    };
    // SysV process entry supplies argc/argv on the stack and the dynamic
    // loader's finalizer in rdx. The binary libc startup ABI takes seven args.
    emit(Op::Mov,Operand::r(XR_R9),Operand::r(XR_RDX));
    emit(Op::Load,Operand::r(XR_RSI),Operand::mem(XR_RSP));
    emit(Op::Lea,Operand::r(XR_RDX),Operand::mem(XR_RSP,8));
    emit(Op::And,Operand::r(XR_RSP),Operand::imm(-std::uint64_t(16)));
    emit(Op::Mov,Operand::r(XR_RAX),Operand::r(XR_RSP));
    emit(Op::Sub,Operand::r(XR_RSP),Operand::imm(16));
    emit(Op::Store,Operand::mem(XR_RSP),Operand::r(XR_RAX));
    emit(Op::Mov,Operand::r(XR_RDI),Operand::symbol(lowir_model::SymbolId(main)));
    emit(Op::Mov,Operand::r(XR_RCX),Operand::imm(0));
    emit(Op::Mov,Operand::r(XR_R8),Operand::imm(0));
    emit(Op::Mov,Operand::r(XR_RBP),Operand::imm(0));
    emit(Op::Call,Operand::symbol(lowir_model::SymbolId(runtime))); emit(Op::Trap,Operand());
    entry_offset = image.code.size();
    bool host = image.host, tls = image.has_tls; image.host = image.has_tls = false;
    Encoder(image).startup(code); image.host = host; image.has_tls = tls;
}
void DynamicImage::arrays(const std::vector<ObjectReference>& refs, bool fini)
{
    image.data.resize((image.data.size()+7)&~std::size_t(7),0);
    (fini ? fini_offset : init_offset) = image.data.size(); (fini ? fini_count : init_count) = refs.size();
    for (auto r : refs) {
        native::Fixup f; f.kind = native::Fixup::AbsoluteSymbol; f.symbol = r.symbol; f.addend = r.addend;
        f.offset = image.data.size(); image.data_fixups.push_back(f); dynamic_number(image.data,0,8);
    }
}
void DynamicImage::patch()
{
    data_offset = (code_offset+image.code.size()+4095)&~std::uint64_t(4095);
    tls_offset = (image.data.size()+image.tls_alignment-1)&~std::size_t(image.tls_alignment-1);
    tls_size = (image.tls.size()+image.tls_alignment-1)&~std::size_t(image.tls_alignment-1);
    for (auto& r : relocations) r.r_offset += base+data_offset;
    for (auto s : stubs) {
        auto relative = std::int64_t(data_offset+s.slot)-std::int64_t(code_offset+s.displacement+4);
        require(relative >= INT32_MIN && relative <= INT32_MAX,"host call veneer out of range");
        dynamic_patch(image.code,s.displacement,relative,4);
    }
    for (unsigned lane = 0; lane < 3; ++lane) {
        auto& bytes = lane == 2 ? image.tls : lane ? image.data : image.code;
        auto origin = base+(lane == 2 ? data_offset+tls_offset : lane ? data_offset : code_offset);
        const auto& fixes = lane == 2 ? image.tls_fixups : lane ? image.data_fixups : image.code_fixups;
        for (const auto& f : fixes) {
            if (import_types[f.symbol] && !copies[f.symbol] && f.kind == native::Fixup::AbsoluteSymbol) {
                require(lane,"dynamic absolute address in executable text");
                Elf64_Rela r = {}; r.r_offset = origin+f.offset; r.r_info = ELF64_R_INFO(dynamic_ids[f.symbol],R_X86_64_64); r.r_addend = f.addend;
                relocations.push_back(r); continue;
            }
            std::uint64_t value = 0;
            if (f.kind == native::Fixup::ThreadOffset) {
                require(image.defined[f.symbol] && image.tls_targets[f.symbol] == f.symbol,"invalid host TLS definition");
                value = image.symbols[f.symbol]+f.addend-tls_size;
            } else value = address(f.symbol)+f.addend;
            unsigned width = f.kind == native::Fixup::AbsoluteSymbol ? 8 : 4;
            if (f.kind == native::Fixup::RelativeSymbol) value -= origin+f.end;
            if (width == 4) {
                if (f.kind == native::Fixup::Absolute32) require(value <= UINT32_MAX,"dynamic absolute reference out of range");
                else require(std::int64_t(value) >= INT32_MIN && std::int64_t(value) <= INT32_MAX,"dynamic relative reference out of range");
            }
            dynamic_patch(bytes,f.offset,value,width);
        }
    }
    for (unsigned i = 1; i < dynamic_ids.size(); ++i) if (dynamic_ids[i] && (!import_types[i] || copies[i])) {
        auto& s = dynsym[dynamic_ids[i]]; s.st_value = address(i); s.st_shndx = image.data_symbols[i] ? Data : Text;
        if (image.tls_targets[i] == i) { s.st_value = image.symbols[i]; s.st_shndx = Tdata; s.st_info = ELF64_ST_INFO(ELF64_ST_BIND(s.st_info),STT_TLS); }
    }
}
} }
