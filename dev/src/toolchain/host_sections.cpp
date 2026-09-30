#include "toolchain/host_elf.h"
#include "support/id_index.h"
#include <algorithm>
#include <cstring>
namespace cppgm { namespace toolchain {
unsigned HostElf::section(const std::string& name, unsigned type, std::uint64_t flags, unsigned alignment)
{
    unsigned id = sections.size(); sections.emplace_back();
    auto& s = sections.back(); s.name = name;
    s.header.sh_type = type; s.header.sh_flags = flags; s.header.sh_addralign = alignment;
    auto& strings = sections[Shstrtab].bytes;
    s.header.sh_name = strings.size(); strings.insert(strings.end(),name.begin(),name.end()); strings.push_back(0);
    section_symbols.push_back(type == SHT_PROGBITS ? symbol("",STB_LOCAL,STT_SECTION,id,0) : 0);
    relocation_sections.push_back(0); section_groups.push_back(0); return id;
}
unsigned HostElf::relocation_section(unsigned target)
{
    if (relocation_sections[target]) return relocation_sections[target];
    auto id = section(".rela"+sections[target].name,SHT_RELA,sections[target].header.sh_flags&SHF_GROUP,8);
    sections[id].header.sh_link = Symtab; sections[id].header.sh_info = target;
    sections[id].header.sh_entsize = sizeof(Elf64_Rela);
    if (auto group = section_groups[target]) host_number(sections[group].bytes,id,4);
    relocation_sections[target] = id; return id;
}
void HostElf::place(const Object& obj)
{
    placement.resize(obj.symbols.size()); relocation_sections.resize(Count); section_groups.resize(Count);
    for (unsigned s = Text; s <= Tdata; ++s) relocation_sections[s] = RelaText+s-Text;
    IdIndex named;
    for (unsigned lane : {Text,Data,Tdata}) {
        std::vector<unsigned> owners;
        bool split = false;
        for (unsigned i = 1; i < obj.symbols.size(); ++i) if (obj.image.defined[i]) {
            const auto& s = obj.symbols[i];
            auto native_lane = obj.image.tls_targets[i] == i ? Tdata : obj.image.data_symbols[i] ? Data : Text;
            if (native_lane != lane) continue;
            placement[i].section = lane; placement[i].offset = obj.image.symbols[i];
            if (s.definition != i) continue;
            owners.push_back(i); split |= s.binding == ir_model::SBM_WEAK || s.section;
        }
        if (!split) continue; // Transfer the ordinary lane without copying it.
        std::sort(owners.begin(),owners.end(),[&](unsigned a,unsigned b) { return obj.image.symbols[a] < obj.image.symbols[b]; });
        std::size_t read = 0, write = 0;
        for (auto owner : owners) {
            const auto& s = obj.symbols[owner]; auto begin = obj.image.symbols[owner];
            lowir_model::require(begin >= read && begin+s.size <= sections[lane].bytes.size(),"invalid native definition extent");
            auto& bytes = sections[lane].bytes;
            if (begin > read) std::memmove(bytes.data()+write,bytes.data()+read,begin-read);
            write += begin-read;
            unsigned target = lane;
            bool weak = s.binding == ir_model::SBM_WEAK;
            if (s.section || weak) {
                if (s.section && !weak) target = named.get(s.section);
                else target = 0;
                if (!target) {
                    auto spelling = s.section ? obj.names.spelling(s.section) : cppgm::TextView();
                    std::string name = s.section ? std::string(spelling.data,spelling.size) : sections[lane].name+"."+obj.name(owner);
                    target = section(name,SHT_PROGBITS,sections[lane].header.sh_flags|(weak ? SHF_GROUP : 0),s.alignment);
                    if (s.section && !weak) named.put(s.section,target);
                    if (weak) {
                        auto group = section(".group",SHT_GROUP,0,4);
                        sections[group].header.sh_link = Symtab; sections[group].header.sh_entsize = 4;
                        host_number(sections[group].bytes,GRP_COMDAT,4);
                        host_number(sections[group].bytes,target,4);
                        section_groups[target] = group;
                        groups.push_back({group,owner});
                    }
                }
                auto& out = sections[target];
                lowir_model::require((out.header.sh_flags&~std::uint64_t(SHF_GROUP)) == sections[lane].header.sh_flags,"incompatible named section flags");
                out.header.sh_addralign = std::max(out.header.sh_addralign,std::uint64_t(s.alignment));
                out.bytes.resize((out.bytes.size()+s.alignment-1)&~std::size_t(s.alignment-1),0);
                placement[owner].offset = out.bytes.size();
                const auto& input = sections[lane].bytes;
                out.bytes.insert(out.bytes.end(),input.begin()+begin,input.begin()+begin+s.size);
            } else {
                auto aligned = (write+s.alignment-1)&~std::size_t(s.alignment-1);
                auto& input = sections[lane].bytes;
                lowir_model::require(aligned <= begin,"invalid native alignment");
                std::fill(input.begin()+write,input.begin()+aligned,0); write = aligned;
                placement[owner].offset = write;
                if (s.size) std::memmove(input.data()+write,input.data()+begin,s.size);
                write += s.size;
            }
            placement[owner].section = target; read = begin+s.size;
        }
        auto& bytes = sections[lane].bytes;
        if (read < bytes.size()) std::memmove(bytes.data()+write,bytes.data()+read,bytes.size()-read);
        bytes.resize(write+bytes.size()-read);
    }
    for (unsigned i = 1; i < obj.symbols.size(); ++i) {
        auto owner = obj.symbols[i].definition;
        if (owner && owner != i) placement[i] = placement[owner];
    }
}
} }
