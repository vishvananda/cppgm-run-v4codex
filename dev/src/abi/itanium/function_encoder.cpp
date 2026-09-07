#include "abi/itanium/encoder.h"
#include <stdexcept>

namespace abi_mangle {
void Encoder::function(const Function& f) {
    if ((f.qualifiers & 12) == 12) throw std::runtime_error("conflicting ref qualifiers");
    Id name = f.name;
    std::vector<Id> arguments = f.arguments;
    bool template_prefix = f.template_prefix;
    if (name && g[name].kind == Kind::Template) {
        if (!arguments.empty()) throw std::runtime_error("two function template argument lists");
        arguments = g.children(name); name = g[name].a; template_prefix = true;
    }
    if (!name && !f.local_owner) throw std::runtime_error("missing function name");
    if (f.c_linkage) { output += g.spelling(g[name].b); return; }
    Id owner = name ? g[name].a : 0;
    if (f.context) context(f.context);
    bool is_nested = f.local_owner || (owner && !standard_namespace(owner)) || f.qualifiers;
    if (is_nested) { output += 'N'; qualifiers(f.qualifiers); }
    if (f.local_owner) local_component(f.local_owner);
    else if (owner) prefix(owner);
    if (f.conversion) { output += "cv"; type(f.conversion); }
    else if (f.terminal != ABI_TERMINAL_NONE) {
        bool member = f.category == FunctionCategory::Inferred ? (owner || f.local_owner) :
            f.category == FunctionCategory::Member;
        output += abi_terminal_code(f.terminal, member, f.parameters.size());
        if (f.terminal == ABI_TERMINAL_LITERAL) source(f.literal_suffix);
    } else if (name) source(g[name].b);
    else throw std::runtime_error("missing local function terminal");
    tags(f.tags);
    if (!arguments.empty()) {
        if (template_prefix) {
            Id key = name;
            if (!f.tags.empty()) key = g.make(Kind::Tagged, name, 0, 0, 0, f.tags);
            enter(key);
        }
        output += 'I';
        for (Id arg : arguments) argument(arg);
        output += 'E';
    }
    if (is_nested) output += 'E';
    bool special_member = f.terminal >= ABI_TERMINAL_CONSTRUCTOR_COMPLETE &&
                          f.terminal <= ABI_TERMINAL_DESTRUCTOR_BASE;
    if (f.result && !f.conversion && !special_member) type(f.result);
    for (Id param : f.parameters) type(param);
    if (!f.parameters.size() && !f.variadic) output += 'v';
    if (f.variadic) output += 'z';
}
std::string Encoder::target(const Target& t) {
    if (t.kind == TargetKind::Type) { type(t.type); return output; }
    if (t.kind == TargetKind::Function && t.function.c_linkage) {
        function(t.function); return output;
    }
    if (t.kind == TargetKind::Variable && !g[t.type].a && !t.internal) {
        output += g.spelling(g[t.type].b); return output;
    }
    output += "_Z";
    switch (t.kind) {
    case TargetKind::Type: break;
    case TargetKind::Function: function(t.function); break;
    case TargetKind::Variable:
        if (t.internal) { output.clear(); external(g.make(Kind::VariableEntity, t.type, 1)); }
        else type(t.type);
        break;
    case TargetKind::Typeinfo: output += "TI"; type(t.type); break;
    case TargetKind::TypeinfoName: output += "TS"; type(t.type); break;
    case TargetKind::Vtable: output += "TV"; type(t.type); break;
    case TargetKind::Vtt: output += "TT"; type(t.type); break;
    case TargetKind::ConstructionVtable:
        output += "TC"; type(t.type);
        integer(t.this_adjust, t.this_adjust < 0); output += '_'; type(t.base); break;
    case TargetKind::TlsWrapper: output += "TW"; type(t.type); break;
    case TargetKind::VirtualThunk:
        output += "Tv"; integer(t.this_adjust, t.this_adjust < 0); output += '_';
        integer(t.vcall_offset, t.vcall_offset < 0); output += '_';
        function(t.function); break;
    case TargetKind::Thunk:
        output += t.has_result_adjust ? "Tch" : "Th";
        integer(t.this_adjust, t.this_adjust < 0); output += '_';
        if (t.has_result_adjust) {
            output += t.virtual_result ? 'v' : 'h';
            integer(t.result_adjust, t.result_adjust < 0); output += '_';
            if (t.virtual_result) {
                integer(t.result_vcall_offset, t.result_vcall_offset < 0); output += '_';
            }
        }
        function(t.function); break;
    }
    return output;
}
void Encoder::external(Id id) {
    const Node n = g[id];
    // An external-name literal has its own substitution grammar state. Reusing
    // the outer sequence here would change both the symbol and later indices.
    if (n.kind == Kind::SymbolEntity) { output += g.spelling(n.a); return; }
    Encoder isolated(g);
    isolated.output = "_Z";
    if (n.kind == Kind::FunctionEntity) isolated.function(entity_function(g, id));
    else if (n.kind == Kind::VariableEntity) {
        const Node name = g[n.a];
        bool nest = isolated.nested(n.a);
        if (nest) isolated.output += 'N';
        if (name.a) isolated.prefix(name.a);
        if (n.b) isolated.output += 'L';
        isolated.source(name.b);
        if (nest) isolated.output += 'E';
    } else throw std::runtime_error("invalid external ABI entity");
    output += isolated.output;
}
} // namespace abi_mangle
