#include "toolchain/dynamic_image.h"
#include <algorithm>
#include <climits>
namespace cppgm { namespace toolchain {
namespace {
template<class T> void append(std::vector<unsigned char>& bytes, const T& value) {
    const auto* p = reinterpret_cast<const unsigned char*>(&value); bytes.insert(bytes.end(),p,p+sizeof(T));
}
unsigned elf_hash(const char* name) {
    unsigned h = 0;
    while (*name) { h = (h<<4)+static_cast<unsigned char>(*name++); auto g = h&0xf0000000; h ^= g>>24; h &= ~g; }
    return h;
}
}
void DynamicImage::metadata(const std::vector<std::string>& libraries,
    const std::vector<ObjectUnwind>& unwind, std::size_t frame_begin)
{
    using lowir_model::require;
    auto section = [&](unsigned id, const char* name, unsigned type, std::uint64_t flags,
                       unsigned align, unsigned entry_size, std::size_t offset, std::size_t size) {
        auto& s = sections[id]; s.sh_name = section_names.size();
        while (*name) section_names.push_back(*name++);
        section_names.push_back(0); s.sh_type = type; s.sh_flags = flags; s.sh_addralign = align;
        s.sh_entsize = entry_size; s.sh_offset = offset; s.sh_size = size;
        if (flags & SHF_ALLOC) s.sh_addr = base+offset;
    };
    auto align = [&](unsigned n) { image.data.resize((image.data.size()+n-1)&~std::size_t(n-1),0); };
    auto tag = [&](std::int64_t key, std::uint64_t value) { Elf64_Dyn d = {}; d.d_tag = key; d.d_un.d_val = value; dynamic.push_back(d); };
    for (const auto& library : libraries) tag(DT_NEEDED,string(library));
    section(Text,".text",SHT_PROGBITS,SHF_ALLOC|SHF_EXECINSTR,16,0,code_offset,image.code.size());
    section(Data,".data",SHT_PROGBITS,SHF_ALLOC|SHF_WRITE,16,0,data_offset,image.data.size());
    image.data.resize(tls_offset,0); image.data.insert(image.data.end(),image.tls.begin(),image.tls.end());
    section(Tdata,".tdata",SHT_PROGBITS,SHF_ALLOC|SHF_WRITE|SHF_TLS,image.tls_alignment,0,data_offset+tls_offset,image.tls.size());
    std::vector<unsigned char>().swap(image.tls);
    // SysV hash is required by the loader for exported definitions and COPY
    // relocations. Chains are built once in linear time from final dynsym IDs.
    unsigned buckets = 1; while (buckets < dynsym.size()) buckets <<= 1;
    std::vector<unsigned> heads(buckets), chains(dynsym.size());
    for (unsigned i = 1; i < dynsym.size(); ++i) {
        auto b = elf_hash(reinterpret_cast<const char*>(strings.data()+dynsym[i].st_name))&(buckets-1);
        chains[i] = heads[b]; heads[b] = i;
    }
    align(8); auto begin = image.data.size(); hash_offset = begin;
    dynamic_number(image.data,buckets,4); dynamic_number(image.data,dynsym.size(),4);
    for (auto n : heads) dynamic_number(image.data,n,4);
    for (auto n : chains) dynamic_number(image.data,n,4);
    section(Hash,".hash",SHT_HASH,SHF_ALLOC,8,4,data_offset+begin,image.data.size()-begin); sections[Hash].sh_link = Symbols;
    align(8); begin = image.data.size(); for (const auto& s : dynsym) append(image.data,s);
    section(Symbols,".dynsym",SHT_DYNSYM,SHF_ALLOC,8,sizeof(Elf64_Sym),data_offset+begin,image.data.size()-begin);
    sections[Symbols].sh_link = Strings; sections[Symbols].sh_info = 1;
    begin = image.data.size(); image.data.insert(image.data.end(),strings.begin(),strings.end());
    section(Strings,".dynstr",SHT_STRTAB,SHF_ALLOC,1,0,data_offset+begin,image.data.size()-begin);
    align(8); begin = image.data.size(); for (const auto& r : relocations) append(image.data,r);
    section(Relocations,".rela.dyn",SHT_RELA,SHF_ALLOC,8,sizeof(Elf64_Rela),data_offset+begin,image.data.size()-begin);
    sections[Relocations].sh_link = Symbols;
    align(4); begin = image.data.size();
    if (!unwind.empty()) {
        require(frame_begin != std::size_t(-1),"missing unwind section identity");
        auto header_address = base+data_offset+begin;
        auto relative = [&](std::int64_t value) {
            require(value >= INT32_MIN && value <= INT32_MAX,"unwind index address out of range"); dynamic_number(image.data,value,4);
        };
        dynamic_number(image.data,0x3b031b01,4); // version, pcrel frame ptr, count, datarel table
        relative(std::int64_t(base+data_offset+frame_begin)-std::int64_t(header_address+4));
        dynamic_number(image.data,unwind.size(),4);
        struct Entry { std::uint64_t pc, fde; }; std::vector<Entry> table;
        for (const auto& u : unwind) table.push_back({address(u.symbol)+u.addend,base+data_offset+u.offset});
        std::sort(table.begin(),table.end(),[](const Entry& a,const Entry& b) { return a.pc < b.pc; });
        for (auto e : table) { relative(std::int64_t(e.pc)-std::int64_t(header_address)); relative(std::int64_t(e.fde)-std::int64_t(header_address)); }
    }
    section(EhHeader,".eh_frame_hdr",SHT_PROGBITS,SHF_ALLOC,4,0,data_offset+begin,image.data.size()-begin);
    tag(DT_HASH,sections[Hash].sh_addr); tag(DT_STRTAB,sections[Strings].sh_addr); tag(DT_STRSZ,strings.size());
    tag(DT_SYMTAB,sections[Symbols].sh_addr); tag(DT_SYMENT,sizeof(Elf64_Sym));
    tag(DT_RELA,sections[Relocations].sh_addr); tag(DT_RELASZ,sections[Relocations].sh_size); tag(DT_RELAENT,sizeof(Elf64_Rela));
    if (init_count) { tag(DT_INIT_ARRAY,base+data_offset+init_offset); tag(DT_INIT_ARRAYSZ,init_count*8); }
    if (fini_count) { tag(DT_FINI_ARRAY,base+data_offset+fini_offset); tag(DT_FINI_ARRAYSZ,fini_count*8); }
    tag(DT_DEBUG,0); tag(DT_NULL,0);
    align(8); begin = image.data.size(); for (const auto& d : dynamic) append(image.data,d);
    section(Dynamic,".dynamic",SHT_DYNAMIC,SHF_ALLOC|SHF_WRITE,8,sizeof(Elf64_Dyn),data_offset+begin,image.data.size()-begin);
    sections[Dynamic].sh_link = Strings;
    section(Shstrings,".shstrtab",SHT_STRTAB,0,1,0,data_offset+image.data.size(),0);
    sections[Shstrings].sh_size = section_names.size();
}
} }
