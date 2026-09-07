#include "abi/itanium/fact_reader.h"
#include <stdexcept>

namespace abi_mangle {
Function FactReader::function(const Words& w, std::size_t& p) {
    Function f;
    std::string op = take(w, p);
    if (op == "encoding") return f;
    if (op == "local" || op == "lambda") {
        f.context = reference(take(w, p), BindingKind::Context);
        Id source = op == "local" ? g.string(take(w, p)) : 0;
        auto ordinal = op == "lambda" ? index_value(take(w, p)) : 0;
        const std::string terminal = take(w, p);
        if (!abi_find_terminal_kind(terminal, &f.terminal)) f.name = g.name(0, terminal);
        if (op == "local") ordinal = index_value(take(w, p));
        std::vector<Id> params;
        while (p < w.size()) params.push_back(type(w, p));
        f.local_owner = g.make(op == "local" ? Kind::Local : Kind::Lambda,
            f.context, source, 0, ordinal, params);
        return f;
    }
    if (op == "namespace-lambda") {
        std::string source = take(w, p), terminal = take(w, p); Id owner = 0;
        while (p < w.size()) owner = g.name(owner, take(w, p));
        owner = g.name(owner, source);
        f.name = g.name(owner, terminal);
        abi_find_terminal_kind(terminal, &f.terminal);
        return f;
    }
    if (op == "local-owner") {
        f.local_owner = type(w, p); f.context = g[f.local_owner].a;
    } else if (op == "member") {
        Id owner = type(w, p); f.name = g.name(owner, take(w, p));
    } else f.name = g.path(op == "path" ? take(w, p) : op);
    while (p < w.size()) {
        if (w[p] == "...") { f.variadic = true; ++p; }
        else if (w[p] == "member-shape") {
            ++p; f.category = boolean(take(w, p)) ? FunctionCategory::Member : FunctionCategory::Nonmember;
        }
        else if (w[p] == "param") { ++p; f.parameters.push_back(type(w, p)); }
        else if (w[p] == "argument") { ++p; f.arguments.push_back(reference(take(w, p), BindingKind::Argument)); }
        else if (w[p] == "template-prefix") { ++p; f.template_prefix = true; }
        else if (w[p] == "qualifier") { ++p; f.qualifiers |= qualifier(take(w, p)); }
        else if (w[p] == "terminal") { ++p; f.terminal = abi_terminal_kind(take(w, p)); }
        else if (w[p] == "source") { ++p; f.name = g.name(0, take(w, p)); }
        else if (w[p] == "literal-suffix") { ++p; f.literal_suffix = g.string(take(w, p)); }
        else if (w[p] == "conversion") { ++p; f.conversion = type(w, p); }
        else if (w[p] == "tag") { ++p; f.tags.push_back(g.string(take(w, p))); }
        else if (w[p] == "c-linkage") { ++p; f.c_linkage = true; }
        else if (w[p] == "result") { ++p; f.result = type(w, p); }
        else if (lookup(w[p]).kind == BindingKind::Argument) {
            f.arguments.push_back(reference(take(w, p), BindingKind::Argument)); f.template_prefix = true;
        } else f.parameters.push_back(type(w, p));
    }
    return f;
}
Id FactReader::context(const Words& w, std::size_t& p) {
    std::string op = take(w, p);
    if (op == "raw") return g.make(Kind::RawContext, g.string(take(w, p)));
    if (op == "function") return function_entity(g, function(w, p));
    throw std::runtime_error("invalid ABI context");
}
Id FactReader::entity(const Words& w, std::size_t& p) {
    std::string op = take(w, p);
    if (op == "symbol") return g.make(Kind::SymbolEntity, g.string(take(w, p)));
    if (op == "function") return function_entity(g, function(w, p));
    if (op == "variable-type" || op == "internal-variable-type")
        return g.make(Kind::VariableEntity, type(w, p), op == "internal-variable-type");
    if (op == "variable" || op == "internal-variable")
        return g.make(Kind::VariableEntity, g.path(take(w, p)), op == "internal-variable");
    throw std::runtime_error("invalid ABI entity");
}
void FactReader::target_record(const Words& w) {
    std::size_t p = 1; const std::string& op = w[0];
    if (op == "function" || op == "c-function") {
        target.kind = TargetKind::Function; target.function = function(w, p);
        target.function.c_linkage |= op == "c-function";
    } else if (op == "thunk" || op == "virtual-base-thunk") {
        target.kind = op == "thunk" ? TargetKind::Thunk : TargetKind::VirtualThunk;
        auto adjust = static_cast<std::int64_t>(integral_value(take(w, p)));
        if (op == "virtual-base-thunk") target.vcall_offset = adjust;
        else target.this_adjust = adjust;
        if (p < w.size() && w[p] != "function") {
            target.has_result_adjust = true;
            if (w[p] == "virtual-result") { target.virtual_result = true; ++p; }
            target.result_adjust = integral_value(take(w, p));
            if (target.virtual_result) target.result_vcall_offset = integral_value(take(w, p));
        }
        if (take(w, p) != "function") throw std::runtime_error("thunk requires function");
        target.function = function(w, p);
    } else if (op == "variable-type" || op == "internal-variable-type" || op == "tls-wrapper-type") {
        target.kind = op == "tls-wrapper-type" ? TargetKind::TlsWrapper : TargetKind::Variable;
        target.internal = op == "internal-variable-type"; target.type = type(w, p);
    } else if (op == "variable" || op == "tls-wrapper") {
        target.kind = op == "variable" ? TargetKind::Variable : TargetKind::TlsWrapper;
        if (op == "tls-wrapper" && take(w, p) != "variable")
            throw std::runtime_error("TLS wrapper requires variable");
        target.type = g.path(take(w, p));
    } else {
        if (op == "type") target.kind = TargetKind::Type;
        else if (op == "typeinfo") target.kind = TargetKind::Typeinfo;
        else if (op == "typeinfo-name") target.kind = TargetKind::TypeinfoName;
        else if (op == "vtable") target.kind = TargetKind::Vtable;
        else if (op == "vtt") target.kind = TargetKind::Vtt;
        else if (op == "construction-vtable") target.kind = TargetKind::ConstructionVtable;
        else throw std::runtime_error("unknown ABI target: " + op);
        target.type = type(w, p);
        if (op == "construction-vtable") {
            target.this_adjust = index_value(take(w, p)); target.base = type(w, p);
        }
    }
    exhausted(w, p); target_seen = true;
}
void FactReader::function_record(const Words& w) {
    if (target.kind != TargetKind::Function && target.kind != TargetKind::Thunk &&
        target.kind != TargetKind::VirtualThunk)
        throw std::runtime_error("function record after nonfunction target");
    Function& f = target.function; const std::string& op = w[0]; std::size_t p = 1;
    if (op == "name-source") {
        std::string name = take(w, p);
        if (name == "-") name.clear();
        f.name = g.name(f.name, name);
        if (p < w.size()) ++p; // obsolete textual substitution hint; identity is structural
    } else if (op == "name-std") f.name = g.name(f.name, "std");
    else if (op == "name-template") {
        std::string name = take(w, p); take(w, p); take(w, p);
        std::string standard = take(w, p); bool complete = boolean(take(w, p));
        Id prefix = f.name;
        prefix = g.name(prefix, name);
        if (standard != "-") prefix = g.make(Kind::Standard, abi_standard_substitution_kind(standard));
        auto args = refs(w, p, BindingKind::Argument);
        f.name = complete ? prefix : g.make(Kind::Template, prefix, 0, 0, 0, args);
    } else if (op == "terminal-source") {
        Id owner = f.name ? g[f.name].a : 0;
        f.name = g.name(owner, take(w, p)); f.terminal = ABI_TERMINAL_NONE;
    } else if (op == "operator-terminal" || op == "terminal") {
        f.terminal = abi_terminal_kind(take(w, p));
        if (f.terminal == ABI_TERMINAL_LITERAL) f.literal_suffix = g.string(take(w, p));
    } else if (op == "conversion-terminal") f.conversion = type(w, p);
    else if (op == "param") f.parameters.push_back(type(w, p));
    else if (op == "result") f.result = type(w, p);
    else if (op == "variadic") f.variadic = true;
    else if (op == "abi-tag") f.tags.push_back(g.string(take(w, p)));
    else if (op == "component-abi-tag") {
        std::vector<Id> tags{g.string(take(w, p))};
        f.name = g.make(Kind::Tagged, f.name, 0, 0, 0, tags);
    } else if (op == "qualifier" || op == "function-qualifier") {
        while (p < w.size()) f.qualifiers |= qualifier(take(w, p));
    } else if (op == "function-template-prefix") {
        take(w, p); f.template_prefix = true;
    } else if (op == "function-template-arg")
        f.arguments.push_back(reference(take(w, p), BindingKind::Argument));
    else if (op == "local-context" || op == "lambda-context") {
        f.context = reference(take(w, p), BindingKind::Context);
        Id name = op == "local-context" ? g.string(take(w, p)) : 0;
        auto ordinal = index_value(take(w, p)); std::vector<Id> params;
        while (p < w.size()) params.push_back(type(w, p));
        f.local_owner = g.make(op == "local-context" ? Kind::Local : Kind::Lambda,
            f.context, name, 0, ordinal, params);
        f.terminal = ABI_TERMINAL_CALL;
    } else if (op == "namespace-lambda-context") {
        std::string source = take(w, p); Id owner = 0;
        while (p < w.size()) owner = g.name(owner, take(w, p));
        f.name = g.name(g.name(owner, source), "operator"); f.terminal = ABI_TERMINAL_CALL;
    } else throw std::runtime_error("unknown ABI function record: " + op);
    exhausted(w, p);
}
} // namespace abi_mangle
