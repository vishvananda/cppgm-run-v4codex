#include "abi/itanium/fact_writer.h"
#include "abi/itanium/operations.h"
#include <stdexcept>

namespace abi_mangle {
FactWriter::FactWriter(const Graph& graph) : g(graph), seen(graph.size(), 0) {}
std::string FactWriter::ref(char family, Id id) {
    unsigned bit = family == 't' ? 1 : family == 'a' ? 2 : family == 'x' ? 4 : family == 'c' ? 8 : 16;
    if (!id || id >= seen.size()) throw std::runtime_error("invalid fact serialization ID");
    std::string name = std::string(1, family) + std::to_string(id);
    if (seen[id] & bit) return name;
    if (++depth > 1024) throw std::runtime_error("ABI fact serialization nesting limit exceeded");
    std::string form, command;
    if (family == 't') { form = type(id); command = "let-type"; }
    else if (family == 'a') { form = argument(id); command = "let-arg"; }
    else if (family == 'x') { form = expression(id); command = "let-expr"; }
    else if (family == 'c') { form = context(id); command = "let-context"; }
    else { form = entity(id); command = "let-entity"; }
    --depth; seen[id] |= bit;
    definitions += command + ' ' + name + ' ' + form + '\n';
    return name;
}
std::string FactWriter::list(const Node& n, char family) {
    std::string result;
    for (Id i = 0; i < n.count; ++i) result += ' ' + ref(family, g.child(n, i));
    return result;
}
std::string FactWriter::type(Id id) {
    const Node& n = g[id];
    switch (n.kind) {
    case Kind::Name:
        if (!n.a) return "named:" + g.spelling(n.b);
        return "member " + ref('t', n.a) + ' ' + g.spelling(n.b);
    case Kind::Standard:
        return std::string("std-template ") + abi_standard_substitution_code(
            static_cast<AbiStandardSubstitutionKind>(n.a)) + " true standard";
    case Kind::Template: {
        const Node& prefix = g[n.a]; std::string result;
        if (prefix.kind == Kind::Name) {
            if (prefix.a) result = "member-template " + ref('t', prefix.a) + ' ' + g.spelling(prefix.b);
            else result = "template " + g.spelling(prefix.b);
        } else if (prefix.kind == Kind::Parameter)
            result = "template-param-template " + std::to_string(prefix.value);
        else if (prefix.kind == Kind::Standard)
            result = std::string("std-template ") + abi_standard_substitution_code(
                static_cast<AbiStandardSubstitutionKind>(prefix.a)) + " false standard";
        else result = "template-name " + ref('t', n.a);
        return result + list(n, 'a');
    }
    case Kind::Tagged: {
        std::string result = "tagged " + ref('t', n.a);
        for (Id i = 0; i < n.count; ++i) result += ' ' + g.spelling(g.child(n, i));
        return result;
    }
    case Kind::Builtin: return abi_builtin_type_word(static_cast<AbiBuiltinTypeKind>(n.a));
    case Kind::Parameter: return std::string(n.b ? "template-param-subst " : "template-param ") + std::to_string(n.value);
    case Kind::Pointer: return "ptr " + ref('t', n.a);
    case Kind::Reference: return "ref " + ref('t', n.a);
    case Kind::RvalueReference: return "rref " + ref('t', n.a);
    case Kind::Cv: return std::string(n.b & 1 ? "const " : "") + (n.b & 2 ? "volatile " : "") + ref('t', n.a);
    case Kind::Pack: return "pack " + ref('t', n.a);
    case Kind::Vendor: return "vendor " + g.spelling(n.b) + ' ' + ref('t', n.a);
    case Kind::Array:
        if (n.b) return "array-expression " + ref('x', n.b) + ' ' + ref('t', n.a);
        return "array " + std::to_string(n.value) + ' ' + ref('t', n.a);
    case Kind::Vector: return "vector " + std::to_string(n.value) + ' ' + ref('t', n.a);
    case Kind::Transform: return "builtin-transform " + g.spelling(n.b) + list(n, 't');
    case Kind::FunctionType:
        return std::string(n.c ? "function-type-variadic " : "function-type ") + ref('t', n.a) + list(n, 't');
    case Kind::MemberPointer: return "member-pointer " + ref('t', n.a) + ' ' + ref('t', n.b);
    case Kind::Decltype: return "decltype " + ref('x', n.a);
    case Kind::Local: return "local-type " + ref('c', n.a) + ' ' + g.spelling(n.b) + ' ' + std::to_string(n.value);
    case Kind::Lambda: return "lambda-closure " + ref('c', n.a) + ' ' + std::to_string(n.value) + list(n, 't');
    default: throw std::runtime_error("invalid type fact serialization");
    }
}
std::string FactWriter::argument(Id id) {
    const Node& n = g[id];
    switch (n.kind) {
    case Kind::TypeArgument: return "type " + ref('t', n.a);
    case Kind::Value: return "value " + ref('t', n.a) + ' ' + std::to_string(static_cast<std::int64_t>(n.value));
    case Kind::DependentValue:
        return "dependent-value " + ref('t', n.a) + ' ' + ref('t', g[n.b].a) + ' ' +
            std::to_string(static_cast<std::int64_t>(g[n.b].value));
    case Kind::ExpressionArgument: return "expression " + ref('x', n.a);
    case Kind::ArgumentPack: return "pack" + list(n, 'a');
    case Kind::TemplateEntity:
        if (g[n.a].kind == Kind::Parameter) return "template-param-template " + std::to_string(g[n.a].value);
        return "template-entity-type " + ref('t', n.a);
    case Kind::MemberTemplateEntity: return "member-template-entity " + ref('t', n.a) + ' ' + g.spelling(n.b) + " -";
    case Kind::EntityArgument: return std::string(n.b ? "entity-address " : "entity-reference ") + ref('e', n.a);
    default: throw std::runtime_error("invalid argument fact serialization");
    }
}
std::string FactWriter::expression(Id id) {
    const Node& n = g[id];
    switch (n.kind) {
    case Kind::ExprParameter: return "template-param " + std::to_string(n.value);
    case Kind::ExprFunctionParameter: return "function-param " + std::to_string(n.value);
    case Kind::Value: return "value " + ref('t', n.a) + ' ' + std::to_string(static_cast<std::int64_t>(n.value));
    case Kind::Unary: return std::string("unary ") + operation_code(n.b) + ' ' + ref('x', n.a);
    case Kind::Binary: return std::string("binary ") + operation_code(n.c) + ' ' + ref('x', n.a) + ' ' + ref('x', n.b);
    case Kind::Conditional: return "conditional " + ref('x', n.a) + ' ' + ref('x', n.b) + ' ' + ref('x', n.c);
    case Kind::ExprPack: return "pack " + ref('x', n.a);
    case Kind::Call: return "call " + ref('x', n.a) + list(n, 'x');
    case Kind::Conversion: return "conversion " + ref('t', n.a) + list(n, 'x');
    case Kind::Cast: return std::string("cast ") + operation_code(n.c) + ' ' + ref('t', n.a) + ' ' + ref('x', n.b);
    case Kind::TemplateId: return "template-id " + g.spelling(n.a) + list(n, 'a');
    case Kind::TypeTrait: return "type-trait " + g.spelling(n.a) + list(n, 't');
    case Kind::SizeofType: return "sizeof-type " + ref('t', n.a);
    case Kind::Member: return "member " + ref('t', n.a) + (n.c ? " yes " : " no ") + g.spelling(n.b) + list(n, 'a');
    case Kind::ObjectMember: return std::string("object-member ") + operation_code(n.c) + ' ' + ref('x', n.a) + ' ' + g.spelling(n.b) + list(n, 'a');
    case Kind::EntityExpression: return std::string(n.b ? "entity-address " : "entity-reference ") + ref('e', n.a);
    default: throw std::runtime_error("invalid expression fact serialization");
    }
}
std::string FactWriter::qualifier_words(unsigned bits) {
    std::string result;
    if (bits & 1) result += " qualifier const";
    if (bits & 2) result += " qualifier volatile";
    if (bits & 4) result += " qualifier lvalue-ref";
    if (bits & 8) result += " qualifier rvalue-ref";
    return result;
}
std::string FactWriter::function(const Function& original) {
    Function f = original;
    if (f.name && g[f.name].kind == Kind::Template) {
        f.arguments = g.children(f.name); f.name = g[f.name].a; f.template_prefix = true;
    }
    std::string result;
    if (f.local_owner) {
        result = "local-owner " + ref('t', f.local_owner);
        if (f.name) result += " source " + g.spelling(g[f.name].b);
    } else {
        const Node& name = g[f.name]; std::string source = g.spelling(name.b);
        if (source.empty()) source = "operator";
        if (name.a) result = "member " + ref('t', name.a) + ' ' + source;
        else result = "path " + source;
    }
    // The full normalized inline shape is also usable in let-context/entity
    // records. Shape markers disambiguate parameters from template arguments.
    if (f.category != FunctionCategory::Inferred)
        result += f.category == FunctionCategory::Member ? " member-shape yes" : " member-shape no";
    if (f.template_prefix) result += " template-prefix";
    for (Id a : f.arguments) result += " argument " + ref('a', a);
    for (Id p : f.parameters) result += " param " + ref('t', p);
    if (f.result) result += " result " + ref('t', f.result);
    if (f.variadic) result += " ...";
    if (f.c_linkage) result += " c-linkage";
    if (f.terminal != ABI_TERMINAL_NONE) result += std::string(" terminal ") + abi_terminal_word(f.terminal);
    if (f.literal_suffix) result += " literal-suffix " + g.spelling(f.literal_suffix);
    if (f.conversion) result += " conversion " + ref('t', f.conversion);
    for (Id tag : f.tags) result += " tag " + g.spelling(tag);
    return result + qualifier_words(f.qualifiers);
}
std::string FactWriter::entity(Id id) {
    const Node& n = g[id];
    if (n.kind == Kind::SymbolEntity) return "symbol " + g.spelling(n.a);
    if (n.kind == Kind::FunctionEntity) return "function " + function(entity_function(g, id));
    if (n.kind == Kind::VariableEntity) return std::string(n.b ? "internal-variable-type " : "variable-type ") + ref('t', n.a);
    throw std::runtime_error("invalid entity fact serialization");
}
std::string FactWriter::context(Id id) {
    const Node& n = g[id];
    if (n.kind == Kind::RawContext) return "raw " + g.spelling(n.a);
    return "function " + function(entity_function(g, id));
}
std::string FactWriter::write(const Target& t) {
    std::string result;
    switch (t.kind) {
    case TargetKind::Type: result = "type " + ref('t', t.type); break;
    case TargetKind::Function: result = "function " + function(t.function); break;
    case TargetKind::Variable: result = std::string(t.internal ? "internal-variable-type " : "variable-type ") + ref('t', t.type); break;
    case TargetKind::Typeinfo: result = "typeinfo " + ref('t', t.type); break;
    case TargetKind::TypeinfoName: result = "typeinfo-name " + ref('t', t.type); break;
    case TargetKind::Vtable: result = "vtable " + ref('t', t.type); break;
    case TargetKind::Vtt: result = "vtt " + ref('t', t.type); break;
    case TargetKind::ConstructionVtable: result = "construction-vtable " + ref('t', t.type) + ' ' + std::to_string(t.this_adjust) + ' ' + ref('t', t.base); break;
    case TargetKind::TlsWrapper: result = "tls-wrapper-type " + ref('t', t.type); break;
    case TargetKind::VirtualThunk:
        result = "virtual-base-thunk " + std::to_string(t.vcall_offset) + " function " + function(t.function); break;
    case TargetKind::Thunk:
        result = "thunk " + std::to_string(t.this_adjust);
        if (t.has_result_adjust) {
            if (t.virtual_result) result += " virtual-result";
            result += ' ' + std::to_string(t.result_adjust);
            if (t.virtual_result) result += ' ' + std::to_string(t.result_vcall_offset);
        }
        result += " function " + function(t.function); break;
    }
    return definitions + result + '\n';
}
std::string serialize_fact_file(const AbiFactFile& file) {
    std::string result;
    for (std::size_t i = 0; i < file.cases.size(); ++i) {
        FactWriter writer(file.graph);
        result += "case case" + std::to_string(i) + '\n' + writer.write(file.cases[i]);
    }
    return result;
}
} // namespace abi_mangle
