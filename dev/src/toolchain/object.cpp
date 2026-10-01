#include "toolchain/object.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
namespace cppgm { namespace toolchain {
using lowir_model::require;
lowir_model::Name Object::intern(const std::string& s) { return names.intern({s.data(),s.size()}); }
std::string Object::name(unsigned id) const {
    auto view = names.spelling(symbols[id].name);
    return std::string(view.data,view.size);
}
Object compile_object(lowir_model::Program& p, native::Statistics& stats, bool host)
{
    Object obj(p.symbols.size()); obj.image.host = host;
    obj.image.host_resume = obj.image.runtime_begin;
    for (const auto& g : p.globals) if (!g.declaration && g.type.kind() == lowir_model::Type::Object) {
        require(g.type.alignment() <= 4096,"unsupported native object alignment");
        obj.alignment = std::max(obj.alignment,g.type.alignment());
    }
    native::compile_image(p,obj.image,{},nullptr,stats);
    for (unsigned i = 1; i < obj.symbols.size(); ++i)
        if (obj.image.defined[i]) { obj.symbols[i].definition = i; obj.symbols[i].size = obj.image.symbol_sizes[i]; }
    for (unsigned i = 1; i <= p.symbols.size(); ++i) {
        const auto& s = p.symbols[i-1];
        auto& symbol = obj.symbols[i];
        symbol.name = obj.intern(s.metadata.object ? p.name(s.metadata.object) : p.name(s.name).substr(1));
        symbol.binding = s.metadata.binding;
        if (s.metadata.section) symbol.section = obj.intern(p.name(s.metadata.section));
        symbol.alignment = obj.image.symbol_alignments[i];
        // Backend-provided definitions (TLS address wrapper, strlen) have one
        // runtime identity, even when independently demanded in multiple TUs.
        if (obj.image.defined[i] && s.kind == lowir_model::Symbol::FunctionSymbol &&
            p.functions[s.entity-1].declaration && symbol.binding != ir_model::SBM_INTERNAL)
            symbol.binding = ir_model::SBM_WEAK;
        symbol.role = s.metadata.role;
        if (symbol.role == ir_model::SR_ENTRY) {
            symbol.name = obj.intern("main");
            const auto& f = p.functions[s.entity-1];
            symbol.parameters = p.signatures[f.signature.index-1].parameters.count;
        }
    }
    if (host) {
        auto& resume = obj.symbols[obj.image.host_resume];
        resume.name = obj.intern("_Unwind_Resume"); resume.binding = ir_model::SBM_STRONG;
    }
    // An alias is another exported identity for the exact same native location.
    for (const auto& alias : p.aliases) {
        auto target = alias.target.index;
        Symbol record = obj.symbols[target]; record.name = obj.intern(p.name(alias.name));
        record.role = ir_model::SR_NONE; obj.symbols.push_back(record);
        obj.image.symbols.push_back(obj.image.symbols[target]);
        obj.image.defined.push_back(obj.image.defined[target]);
        obj.image.data_symbols.push_back(obj.image.data_symbols[target]);
        obj.image.tls_targets.push_back(obj.image.tls_targets[target] == target ? obj.image.symbols.size()-1 : 0);
    }
    return obj;
}
namespace {
void number(std::ostream& out, std::uint64_t n, unsigned bytes = 8) {
    for (unsigned i = 0; i < bytes; ++i) { out.put(n & 255); n >>= 8; }
}
struct Reader {
    const std::vector<unsigned char>& bytes;
    std::size_t pos = 0;
    explicit Reader(const std::vector<unsigned char>& b) : bytes(b) {}
    void need(std::size_t n) { require(n <= bytes.size()-pos,"truncated compiler object"); }
    std::uint64_t number(unsigned n = 8) {
        need(n); std::uint64_t result = 0;
        for (unsigned i = 0; i < n; ++i) result |= std::uint64_t(bytes[pos++]) << (8*i);
        return result;
    }
    unsigned count(unsigned minimum) {
        auto n = number(); require(n <= (bytes.size()-pos)/minimum,"invalid object count"); return n;
    }
    void data(std::vector<unsigned char>& out) {
        auto n = count(1); out.assign(bytes.begin()+pos,bytes.begin()+pos+n); pos += n;
    }
    std::string text() {
        auto n = count(1); std::string result(bytes.begin()+pos,bytes.begin()+pos+n); pos += n; return result;
    }
};
void write_fixes(std::ostream& out, const std::vector<native::Fixup>& fixes) {
    number(out,fixes.size());
    for (const auto& f : fixes) {
        number(out,f.kind); number(out,f.offset); number(out,f.end); number(out,f.symbol); number(out,f.addend);
        number(out,f.owner);
    }
}
void read_fixes(Reader& r, std::vector<native::Fixup>& fixes, std::size_t bytes, unsigned symbols) {
    auto count = r.count(48); fixes.reserve(count);
    for (unsigned i = 0; i < count; ++i) {
        native::Fixup f; auto kind = r.number();
        require(kind <= native::Fixup::Absolute32Signed,"invalid object relocation");
        f.kind = native::Fixup::Kind(kind); f.offset = r.number(); f.end = r.number();
        auto id = r.number(); require(id && id < symbols,"invalid object relocation symbol"); f.symbol = id;
        f.addend = r.number(); auto owner = r.number();
        require(owner < symbols,"invalid object relocation owner"); f.owner = owner;
        unsigned width = f.kind == native::Fixup::AbsoluteSymbol ? 8 : 4;
        require(f.offset <= bytes && width <= bytes-f.offset && f.end <= bytes,"invalid object relocation range");
        fixes.push_back(f);
    }
}
}
void write_object(const Object& obj, const std::string& path)
{
    std::ofstream out(path,std::ios::binary); require(bool(out),"cannot create compiler object");
    out.write("CPPGMOBJ",8); number(out,4); number(out,62); // format, x86-64 target
    number(out,obj.image.runtime_begin); number(out,obj.alignment); number(out,obj.image.has_tls);
    number(out,obj.symbols.size());
    for (unsigned i = 0; i < obj.symbols.size(); ++i) {
        const auto& s = obj.symbols[i]; auto name = s.name ? obj.name(i) : std::string();
        number(out,name.size()); out.write(name.data(),name.size());
        number(out,s.binding); number(out,s.role); number(out,s.parameters); number(out,s.definition); number(out,s.lazy);
        number(out,obj.image.symbols[i]); number(out,obj.image.defined[i]); number(out,obj.image.data_symbols[i]);
    }
    for (const auto* bytes : {&obj.image.code,&obj.image.data}) {
        number(out,bytes->size()); out.write(reinterpret_cast<const char*>(bytes->data()),bytes->size());
    }
    write_fixes(out,obj.image.code_fixups); write_fixes(out,obj.image.data_fixups);
    out.close(); require(bool(out),"cannot write compiler object");
}
Object read_object(const std::string& path)
{
    std::ifstream in(path,std::ios::binary); require(bool(in),"cannot open object");
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(in)),{});
    require(!in.bad(),"cannot read object");
    if (bytes.size() >= 4 && bytes[0] == 127 && bytes[1] == 'E' && bytes[2] == 'L' && bytes[3] == 'F') return read_elf(bytes);
    require(bytes.size() >= 8 && std::string(bytes.begin(),bytes.begin()+8) == "CPPGMOBJ","invalid compiler object");
    Reader r{bytes}; r.pos = 8;
    require(r.number() == 4 && r.number() == 62,"unsupported compiler object version/target");
    auto runtime = r.number(), alignment = r.number(), tls = r.number();
    auto count = r.count(72);
    require(runtime && runtime <= count && count-runtime >= unsigned(native::RuntimeEntity::Count),"invalid runtime symbol range");
    require(alignment && alignment <= 4096 && !(alignment&(alignment-1)) && tls <= 1,"invalid object layout");
    Object obj(count-1-unsigned(native::RuntimeEntity::Count));
    obj.image.runtime_begin = runtime; obj.alignment = alignment; obj.image.has_tls = tls;
    for (unsigned i = 0; i < count; ++i) {
        auto name = r.text(); if (!name.empty()) obj.symbols[i].name = obj.intern(name);
        auto binding = r.number(), role = r.number(), parameters = r.number(), definition = r.number(), lazy = r.number();
        require(binding <= ir_model::SBM_WEAK && role <= ir_model::SR_RTTI_DATA && parameters <= 2 && definition < count && lazy <= 1,"invalid object symbol metadata");
        auto& s = obj.symbols[i]; s.binding = ir_model::SymbolBindingMode(binding); s.role = ir_model::SymbolRole(role); s.parameters = parameters; s.definition = definition; s.lazy = lazy;
        obj.image.symbols[i] = r.number(); auto defined = r.number(), data = r.number();
        require(defined <= 1 && data <= 1,"invalid object symbol flags");
        obj.image.defined[i] = defined; obj.image.data_symbols[i] = data;
    }
    r.data(obj.image.code); r.data(obj.image.data);
    for (unsigned i = 1; i < count; ++i) if (obj.image.defined[i])
        require(obj.image.symbols[i] <= (obj.image.data_symbols[i] ? obj.image.data.size() : obj.image.code.size()),"invalid object symbol offset");
    read_fixes(r,obj.image.code_fixups,obj.image.code.size(),count);
    read_fixes(r,obj.image.data_fixups,obj.image.data.size(),count);
    for (unsigned i = 0; i < count; ++i) if (auto owner = obj.symbols[i].definition)
        require(obj.image.defined[i] && obj.image.defined[owner] && obj.symbols[owner].definition == owner &&
            obj.image.data_symbols[i] == obj.image.data_symbols[owner] && obj.image.symbols[i] == obj.image.symbols[owner],"invalid object definition identity");
    for (const auto* fixes : {&obj.image.code_fixups,&obj.image.data_fixups}) for (const auto& f : *fixes)
        require(!f.owner || obj.symbols[f.owner].definition == f.owner,"invalid object fixup definition");
    require(r.pos == bytes.size(),"trailing compiler object data"); return obj;
}
} }
