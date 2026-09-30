#include "toolchain/host_elf.h"
#include "support/id_index.h"
namespace cppgm { namespace toolchain {
namespace {
void finish_record(std::vector<unsigned char>& out, std::size_t start) {
    while (out.size()%8) out.push_back(0);
    auto length = out.size()-start-4;
    for (unsigned k = 0; k < 4; ++k) out[start+k] = length>>(k*8);
}
}
void HostElf::unwind(const Object& obj)
{
    IdIndex pointers;
    auto indirect = [&](unsigned target) {
        auto previous = pointers.get(target); if (previous) return previous-1;
        auto offset = sections[Refs].bytes.size();
        relocate(RelaRefs,offset,target,R_X86_64_64); host_number(sections[Refs].bytes,0,8);
        pointers.put(target,offset+1); return unsigned(offset);
    };
    for (const auto& fix : obj.image.lsda_fixups)
        relocate(RelaLsda,fix.offset,section_symbols[Refs],R_X86_64_PC32,indirect(mapping[fix.symbol]));
    std::size_t cies[2] = {}; bool present[2] = {};
    auto& out = sections[EhFrame].bytes;
    for (const auto& u : obj.image.unwind) {
        unsigned eh = u.has_lsda;
        if (!present[eh]) {
            cies[eh] = out.size(); present[eh] = true;
            host_number(out,0,4); host_number(out,0,4); out.push_back(1);
            const char* aug = eh ? "zPLR" : "zR";
            do { out.push_back(*aug); } while (*aug++);
            out.insert(out.end(),{1,0x78,16});
            host_uleb(out,eh ? 7 : 1);
            if (eh) {
                auto personality = symbol("__gxx_personality_v0",STB_GLOBAL,STT_NOTYPE,0,0);
                auto ref = indirect(personality);
                out.push_back(0x9b);
                relocate(RelaEh,out.size(),section_symbols[Refs],R_X86_64_PC32,ref);
                host_number(out,0,4); out.push_back(0x1b);
            }
            out.push_back(0x1b);
            out.insert(out.end(),{0x0c,7,8,0x90,1}); // CFA rsp+8, return at CFA-8
            finish_record(out,cies[eh]);
        }
        auto start = out.size(); host_number(out,0,4); host_number(out,out.size()-cies[eh],4);
        fdes.push_back(start);
        const auto& location = placement[u.symbol];
        relocate(RelaEh,out.size(),section_symbols[location.section],R_X86_64_PC32,location.offset);
        host_number(out,0,4); host_number(out,u.end-u.begin,4);
        host_uleb(out,eh ? 4 : 0);
        if (eh) { relocate(RelaEh,out.size(),section_symbols[Lsda],R_X86_64_PC32,u.lsda); host_number(out,0,4); }
        out.insert(out.end(),u.cfi.begin(),u.cfi.end()); finish_record(out,start);
    }
}
} }
