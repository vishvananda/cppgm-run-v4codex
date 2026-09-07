#include "lowir/reader.h"
#include "lowir/metadata.h"
#include <cstdlib>
#include <cerrno>
namespace lowir_model {
static const char* const roles[] = {
    "", "entry", "init", "fini", "eh_allocate_exception", "eh_begin_catch", "eh_end_catch",
    "eh_rethrow", "eh_throw", "eh_personality", "eh_resume", "allocate_memory", "free_memory",
    "terminate", "pure_virtual", "dynamic_cast", "bad_cast", "bad_typeid", "rtti_class", "rtti_si", "rtti_vmi", "rtti_data"
};
const char* role_name(SymbolRole role) { return roles[role]; }
void Reader::boundary_field(FunctionBoundaryMetadata& m, const std::string& k, const std::string& v)
{
    if (k == "arity" && v == "variadic") m.arity = CAM_VARIADIC;
    else if (k == "effects" && (v == "readnone" || v == "readonly")) m.effects = v == "readnone" ? CFXM_READNONE : CFXM_READONLY;
    else if (k == "unwind" && v == "no") m.unwind = CUM_NO;
    else if (k == "return" && v == "noreturn") m.returns = CRM_NORETURN;
    else if (k == "query" && v == "stable_prefix") m.query = CQM_STABLE_PREFIX;
    else throw ParseError("invalid call-boundary metadata " + k + "=" + v);
}
SymbolMetadata Reader::metadata(bool function, FunctionBoundaryMetadata* boundary)
{
    SymbolMetadata m;
    // Key identity is adapter-local, with bounded fields; never retained as a
    // semantic key. The bitset detects duplicates across bracket groups.
    static const char* const keys[] = {"role","linkage","binding","object","tls_for","keep_alias","prefer_local",
        "object_root","force_inline","inline_hint","no_inline","storage","section","arity","effects","unwind","return","query"};
    std::uint32_t seen = 0;
    while (accept("[")) {
        do {
            std::string k = word();
            expect("=");
            std::string v = word();
            unsigned key = 0;
            for (; key < sizeof(keys)/sizeof(*keys); ++key) if (k == keys[key]) break;
            require(key < sizeof(keys)/sizeof(*keys), "unknown metadata key");
            require(!(seen & (1u << key)), "duplicate metadata key");
            seen |= 1u << key;
            if (key >= 13) {
                require(function && boundary, "call boundary on global");
                boundary_field(*boundary, k, v);
            } else if (k == "role") {
                unsigned r = 1;
                for (; r < sizeof(roles)/sizeof(*roles); ++r) if (v == roles[r]) break;
                require(r < sizeof(roles)/sizeof(*roles) && (function == (r < SR_RTTI_CLASS)), "invalid symbol role");
                m.role = SymbolRole(r);
            } else if (k == "linkage") { require(v == "c", "invalid linkage"); m.linkage = LLM_C; }
            else if (k == "binding") {
                require(v == "internal" || v == "strong" || v == "weak", "invalid binding");
                m.binding = v == "internal" ? SBM_INTERNAL : v == "strong" ? SBM_STRONG : SBM_WEAK;
            } else if (k == "object") m.object = p_.intern(v);
            else if (k == "tls_for") {
                require(function && v.size() > 1 && v[0] == '@', "invalid TLS target");
                m.tls_for = p_.symbol(p_.intern(v));
            } else if (k == "storage") {
                require(!function && (v == "readonly" || v == "thread_local"), "invalid storage");
                m.storage = v == "readonly" ? GSM_READONLY : GSM_THREAD_LOCAL;
            } else if (k == "section") {
                require(!function && !v.empty(), "invalid section placement");
                for (char c : v) require((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '.', "unsafe section name");
                m.section = p_.intern(v);
            } else {
                require(v == "yes", "invalid metadata flag");
                if (key >= 8 && key <= 10) require(function, "function flag on global");
                if (k == "keep_alias") m.keep_alias = true;
                if (k == "prefer_local") m.prefer_local = true;
                if (k == "object_root") m.object_root = true;
                if (k == "force_inline") m.force_inline = true;
                if (k == "inline_hint") m.inline_hint = true;
                if (k == "no_inline") m.no_inline = true;
            }
        } while (accept(","));
        expect("]");
    }
    return m;
}
void Reader::parameter_metadata(Parameter& p)
{
    unsigned seen = 0;
    while (accept("[")) {
        do {
            std::string k = word();
            expect("=");
            unsigned bit = k == "pass" ? 1 : k == "alias" ? 2 : k == "object_bytes" ? 4 : 0;
            require(bit && !(seen & bit) && p.type == Type::Ptr, "invalid parameter metadata");
            seen |= bit;
            if (k == "object_bytes") {
                p.object_bytes = natural();
                require(p.object_bytes != 0, "empty object extent");
            } else {
                std::string v = word();
                if (k == "pass") {
                    require(v == "indirect_result" || v == "by_address", "invalid parameter passing");
                    p.passing = v == "indirect_result" ? PPM_INDIRECT_RESULT : PPM_BY_ADDRESS;
                } else { require(v == "noalias", "invalid parameter alias"); p.alias = PALM_NOALIAS; }
            }
        } while (accept(","));
        expect("]");
    }
}
Signature Reader::signature(FunctionBuilder* b)
{
    Signature s;
    s.parameters.begin = p_.parameters.size();
    NameIndex names;
    expect("(");
    if (!at(")")) do {
        Parameter param;
        Name n;
        if (token_.size && token_.data[0] == '%') { n = name('%'); expect(":"); }
        else { require(!b, "unnamed function parameter"); n = p_.intern("%arg" + std::to_string(s.parameters.count)); }
        require(names.insert(n, 1), "duplicate parameter");
        param.type = type();
        parameter_metadata(param);
        if (b) { param.value = b->value(n); b->parameter(param); }
        else {
            Value v;
            v.name = n;
            v.type = param.type;
            v.defined = true;
            p_.values.push_back(v);
            param.value = ValueId(p_.values.size());
            p_.parameters.push_back(param);
        }
        ++s.parameters.count;
    } while (accept(","));
    expect(")");
    expect("->");
    s.result = type();
    return s;
}
void validate_signature(const Program& p, const Signature& s, bool indirect)
{
    require(s.parameters.end() <= p.parameters.size(), "invalid signature range");
    for (unsigned i = 0; i < s.parameters.count; ++i) {
        const Parameter& a = p.parameters[s.parameters.begin + i];
        require(a.type != Type(), "void parameter");
        if (a.passing != PPM_DIRECT || a.alias != PALM_DEFAULT || a.object_bytes)
            require(a.type == Type::Ptr, "pointer metadata on non-pointer");
        if (a.passing == PPM_INDIRECT_RESULT) require(i == 0 && s.result == Type(), "invalid indirect result boundary");
    }
    if (s.boundary.query != CQM_DEFAULT) {
        require(!indirect && s.boundary.arity == CAM_FIXED && s.parameters.count && s.result.scalar(), "invalid query boundary");
        require(p.parameters[s.parameters.end()-1].type.integer(), "query index must be integer");
    }
}
} // namespace lowir_model
