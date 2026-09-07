#include "lowir/writer.h"
#include "lowir/metadata.h"
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
namespace lowir_model {
void Writer::type(Type t)
{
    static const char* const names[] = {"void","i1","i8","u8","i16","u16","i32","u32","i64","f32","f64","f80","ptr"};
    if (t.kind() == Type::Object) out_ << "obj<" << t.bytes() << 'x' << t.alignment() << '>';
    else out_ << names[t.kind()];
}
void Writer::symbol(SymbolId id) { out_ << p_.name(p_.symbols.at(id.index-1).name); }
void Writer::operand(const Operand& v, Type context)
{
    switch (v.kind) {
    case Operand::Temporary: out_ << p_.name(p_.values.at(v.ref-1).name); break;
    case Operand::Slot: out_ << p_.name(p_.slots.at(v.ref-1).name); break;
    case Operand::Label: out_ << p_.name(p_.blocks.at(v.ref-1).name); break;
    case Operand::Symbol: symbol(SymbolId(v.ref)); break;
    case Operand::Integer: out_ << static_cast<std::int64_t>(v.data.integer); break;
    case Operand::Null: out_ << "nullptr"; break;
    case Operand::Floating: {
        long double n = v.data.floating;
        if (context == Type::F32) n = static_cast<float>(n);
        if (context == Type::F64) n = static_cast<double>(n);
        if (std::isnan(n)) { out_ << "nan"; break; }
        if (std::isinf(n)) { out_ << (std::signbit(n) ? "-inf" : "inf"); break; }
        // max_digits10 guarantees a value-preserving decimal roundtrip.
        std::ostringstream s;
        s << std::setprecision(std::numeric_limits<long double>::max_digits10) << n;
        std::string text = s.str();
        if (text.find_first_of(".eE") == std::string::npos) text += ".0";
        out_ << text;
        if (context == Type::F32) out_ << 'f';
        if (context == Type::F80) out_ << 'L';
        break;
    }
    }
}
void Writer::debug(const DebugLocation& d)
{
    if (d.file) out_ << " !dbg(" << p_.name(d.file) << ", " << d.line << ", " << d.column << ')';
}
void Writer::metadata(const SymbolMetadata* m, const FunctionBoundaryMetadata* b)
{
    bool any = false;
    auto field = [&](const char* key, const std::string& value) {
        out_ << (any ? ", " : " [") << key << '=' << value;
        any = true;
    };
    if (m) {
        if (m->role != SR_NONE) field("role", role_name(m->role));
        if (m->linkage != LLM_DEFAULT) field("linkage", "c");
        if (m->binding != SBM_DEFAULT) field("binding", m->binding == SBM_INTERNAL ? "internal" : m->binding == SBM_STRONG ? "strong" : "weak");
        if (m->storage != GSM_DEFAULT) field("storage", m->storage == GSM_READONLY ? "readonly" : "thread_local");
        if (m->object) field("object", p_.name(m->object));
        if (m->section) field("section", p_.name(m->section));
        if (m->tls_for) field("tls_for", p_.name(p_.symbols.at(m->tls_for.index-1).name));
        if (m->keep_alias) field("keep_alias", "yes");
        if (m->prefer_local) field("prefer_local", "yes");
        if (m->object_root) field("object_root", "yes");
        if (m->force_inline) field("force_inline", "yes");
        if (m->inline_hint) field("inline_hint", "yes");
        if (m->no_inline) field("no_inline", "yes");
    }
    if (b) {
        if (b->arity != CAM_FIXED) field("arity", "variadic");
        if (b->effects != CFXM_DEFAULT) field("effects", b->effects == CFXM_READNONE ? "readnone" : "readonly");
        if (b->unwind != CUM_DEFAULT) field("unwind", "no");
        if (b->returns != CRM_DEFAULT) field("return", "noreturn");
        if (b->query != CQM_DEFAULT) field("query", "stable_prefix");
    }
    if (any) out_ << ']';
}
void Writer::signature(const Signature& s)
{
    out_ << '(';
    for (unsigned j = 0; j < s.parameters.count; ++j) {
        if (j) out_ << ", ";
        const Parameter& a = p_.parameters[s.parameters.begin+j];
        out_ << p_.name(p_.values.at(a.value.index-1).name) << " : ";
        type(a.type);
        bool any = false;
        auto field = [&](const char* key, const std::string& value) {
            out_ << (any ? ", " : " [") << key << '=' << value;
            any = true;
        };
        if (a.passing != PPM_DIRECT) field("pass", a.passing == PPM_INDIRECT_RESULT ? "indirect_result" : "by_address");
        if (a.alias != PALM_DEFAULT) field("alias", "noalias");
        if (a.object_bytes) field("object_bytes", std::to_string(a.object_bytes));
        if (any) out_ << ']';
    }
    out_ << ") -> ";
    type(s.result);
}
void Writer::data(const DataItem& d, bool structured)
{
    if (d.kind == DataItem::Zero) { out_ << "zero"; if (structured) out_ << ' ' << d.zero_bytes; return; }
    if (structured) { type(d.type); out_ << ' '; }
    if (d.kind == DataItem::Scalar) operand(d.value, d.type);
    else {
        out_ << "addr "; symbol(d.symbol);
        if (d.addend > 0) out_ << " + " << d.addend;
        if (d.addend < 0) out_ << " - " << (std::uint64_t(0)-std::uint64_t(d.addend));
    }
}
void Writer::global(const Global& g)
{
    if (g.declaration) out_ << "declare ";
    out_ << "global "; symbol(g.symbol);
    if (!g.structured && g.type != Type()) { out_ << " : "; type(g.type); }
    metadata(&p_.symbols[g.symbol.index-1].metadata, 0);
    if (!g.declaration) {
        out_ << " = ";
        if (g.structured) {
            out_ << "{\n";
            for (unsigned j = g.data.begin; j < g.data.end(); ++j) { out_ << "  "; data(p_.data[j], true); out_ << '\n'; }
            out_ << '}';
        } else data(p_.data[g.data.begin], false);
    }
    out_ << '\n';
}
void Writer::function(const Function& f)
{
    if (f.declaration) out_ << "declare ";
    out_ << "function "; symbol(f.symbol);
    const Signature& s = p_.signatures[f.signature.index-1];
    signature(s);
    metadata(&p_.symbols[f.symbol.index-1].metadata, &s.boundary);
    if (f.declaration) { out_ << '\n'; return; }
    debug(f.debug);
    out_ << " {\n";
    for (unsigned j = f.slots.begin; j < f.slots.end(); ++j) {
        const Slot& slot = p_.slots[p_.slot_order[j].index-1];
        out_ << "  slot " << p_.name(slot.name) << " : "; type(slot.type); out_ << '\n';
    }
    for (unsigned j = f.blocks.begin; j < f.blocks.end(); ++j) {
        const Block& block = p_.blocks[p_.block_order[j].index-1];
        out_ << "  block " << p_.name(block.name) << ":\n";
        for (unsigned k = block.instructions.begin; k < block.instructions.end(); ++k) {
            out_ << "    "; instruction(p_.instructions[k]); out_ << '\n';
        }
    }
    out_ << "}\n";
}
void Writer::write()
{
    for (const Global& g : p_.globals) if (g.declaration) global(g);
    for (const Function& f : p_.functions) if (f.declaration) function(f);
    for (const Global& g : p_.globals) if (!g.declaration) global(g);
    for (const Function& f : p_.functions) if (!f.declaration) function(f);
    for (const ObjectAlias& a : p_.aliases) { out_ << "alias object " << p_.name(a.name) << " = "; symbol(a.target); out_ << '\n'; }
}
void write_program(const Program& p, std::ostream& out) { Writer(p, out).write(); }
} // namespace lowir_model
