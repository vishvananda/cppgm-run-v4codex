#include "toolchain/dynamic_image.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
namespace cppgm { namespace toolchain {
void DynamicImage::write(const std::string& path)
{
    using lowir_model::require;
    unsigned phnum = image.has_tls ? 8 : 7;
    const char interpreter[] = "/lib64/ld-linux-x86-64.so.2";
    const std::uint64_t interp_offset = sizeof(Elf64_Ehdr)+phnum*sizeof(Elf64_Phdr);
    Elf64_Ehdr h = {}; std::memcpy(h.e_ident,ELFMAG,SELFMAG);
    h.e_ident[EI_CLASS] = ELFCLASS64; h.e_ident[EI_DATA] = ELFDATA2LSB; h.e_ident[EI_VERSION] = EV_CURRENT;
    h.e_type = ET_EXEC; h.e_machine = EM_X86_64; h.e_version = EV_CURRENT; h.e_entry = base+code_offset+entry_offset;
    h.e_phoff = sizeof(h); h.e_ehsize = sizeof(h); h.e_phentsize = sizeof(Elf64_Phdr); h.e_phnum = phnum;
    h.e_shoff = (data_offset+image.data.size()+section_names.size()+7)&~std::uint64_t(7);
    h.e_shentsize = sizeof(Elf64_Shdr); h.e_shnum = Count; h.e_shstrndx = Shstrings;
    std::vector<Elf64_Phdr> headers(phnum);
    auto segment = [&](unsigned id,unsigned type,unsigned flags,std::uint64_t offset,std::uint64_t size,unsigned align) {
        auto& p = headers[id]; p.p_type = type; p.p_flags = flags; p.p_offset = offset;
        p.p_vaddr = p.p_paddr = base+offset; p.p_filesz = p.p_memsz = size; p.p_align = align;
    };
    segment(0,PT_PHDR,PF_R,sizeof(h),headers.size()*sizeof(Elf64_Phdr),8);
    segment(1,PT_INTERP,PF_R,interp_offset,sizeof(interpreter),1);
    segment(2,PT_LOAD,PF_R|PF_X,0,code_offset+image.code.size(),4096);
    segment(3,PT_LOAD,PF_R|PF_W,data_offset,image.data.size(),4096);
    segment(4,PT_DYNAMIC,PF_R|PF_W,sections[Dynamic].sh_offset,sections[Dynamic].sh_size,8);
    segment(5,PT_GNU_EH_FRAME,PF_R,sections[EhHeader].sh_offset,sections[EhHeader].sh_size,4);
    segment(6,PT_GNU_STACK,PF_R|PF_W,0,0,16); headers[6].p_vaddr = headers[6].p_paddr = 0;
    if (image.has_tls) segment(7,PT_TLS,PF_R,data_offset+tls_offset,sections[Tdata].sh_size,image.tls_alignment);
    std::ofstream out(path,std::ios::binary); require(bool(out),"cannot create dynamic executable");
    std::uint64_t offset = 0;
    auto write = [&](const void* bytes,std::size_t count) { out.write(static_cast<const char*>(bytes),count); offset += count; };
    const char zeros[4096] = {};
    auto pad = [&](std::uint64_t end) { require(offset <= end,"invalid executable layout");
        while (offset < end) write(zeros,std::min<std::uint64_t>(end-offset,sizeof(zeros))); };
    write(&h,sizeof(h)); write(headers.data(),headers.size()*sizeof(Elf64_Phdr)); write(interpreter,sizeof(interpreter));
    pad(code_offset); write(image.code.data(),image.code.size()); pad(data_offset); write(image.data.data(),image.data.size());
    write(section_names.data(),section_names.size()); pad(h.e_shoff); write(sections.data(),sections.size()*sizeof(Elf64_Shdr));
    out.close(); require(bool(out),"cannot write dynamic executable"); require(chmod(path.c_str(),0755) == 0,"cannot mark executable");
}
std::size_t write_dynamic_executable(native::Image& image, const std::vector<Symbol>& symbols,
    const IdentifierTable& names, const std::vector<DynamicImport>& imports, const std::vector<std::string>& libraries,
    const std::vector<ObjectUnwind>& unwind, const std::vector<ObjectReference>& init,
    const std::vector<ObjectReference>& fini, std::size_t frame_begin, unsigned entry, unsigned startup, const std::string& path)
{
    DynamicImage out(image,symbols,names);
    out.imports(imports); out.startup(entry,startup); out.arrays(init,false); out.arrays(fini,true);
    out.patch(); out.metadata(libraries,unwind,frame_begin); out.write(path); return image.code.size();
}
} }
