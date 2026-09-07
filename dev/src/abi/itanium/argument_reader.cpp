#include "abi/itanium/fact_reader.h"
#include "abi/itanium/operations.h"
#include <stdexcept>

namespace abi_mangle {
Id FactReader::argument(const Words& w, std::size_t& p) {
    std::string op = take(w, p);
    if (op == "type") return g.make(Kind::TypeArgument, type(w, p));
    if (op == "value" || op == "dependent-value") {
        Id dependent = 0;
        if (op == "dependent-value") dependent = type(w, p);
        Id value_type = type(w, p); auto value = integral_value(take(w, p));
        if (g[value_type].kind == Kind::Builtin) {
            switch (static_cast<AbiBuiltinTypeKind>(g[value_type].a)) {
            case ABI_BUILTIN_TYPE_BOOL: value = value != 0; break;
            case ABI_BUILTIN_TYPE_UNSIGNED_CHAR: value &= 255; break;
            case ABI_BUILTIN_TYPE_UNSIGNED_SHORT: value &= 65535; break;
            case ABI_BUILTIN_TYPE_UNSIGNED_INT: value &= 0xffffffffull; break;
            default: break;
            }
        }
        Id literal = g.make(Kind::Value, value_type, 0, 0, value);
        return dependent ? g.make(Kind::DependentValue, dependent, literal) : literal;
    }
    if (op == "expression") return g.make(Kind::ExpressionArgument, reference(take(w, p), BindingKind::Expression));
    if (op == "pack") return g.make(Kind::ArgumentPack, 0, 0, 0, 0, refs(w, p, BindingKind::Argument));
    if (op == "template-param-template") {
        auto index = index_value(take(w, p));
        if (index > UINT32_MAX) throw std::runtime_error("ABI template index too large");
        return g.make(Kind::TemplateEntity, g.make(Kind::Parameter, index));
    }
    if (op == "template-entity") return g.make(Kind::TemplateEntity, g.path(take(w, p)));
    if (op == "member-template-entity") {
        Id owner = type(w, p); Id name = g.string(take(w, p));
        if (p < w.size()) take(w, p);
        return g.make(Kind::MemberTemplateEntity, owner, name);
    }
    if (op == "entity-address" || op == "entity-reference" || op == "entity")
        return g.make(Kind::EntityArgument, reference(take(w, p), BindingKind::Entity), op == "entity-address");
    if (op == "external-address" || op == "external" || op == "external-entity")
        return g.make(Kind::EntityArgument, g.make(Kind::SymbolEntity, g.string(take(w, p))), op == "external-address");
    if (op == "member-external-address" || op == "member-external") {
        take(w, p); // external spelling is informational; structured owner/shape owns encoding
        Id owner = type(w, p); Id name = g.name(owner, take(w, p));
        bool is_function = boolean(take(w, p)); Function f; f.name = name;
        if (boolean(take(w, p))) f.qualifiers |= 1;
        if (boolean(take(w, p))) f.qualifiers |= 2;
        if (boolean(take(w, p))) f.qualifiers |= 4;
        if (boolean(take(w, p))) f.qualifiers |= 8;
        f.variadic = boolean(take(w, p));
        while (p < w.size()) f.parameters.push_back(type(w, p));
        Id entity = is_function ? function_entity(g, f) : g.make(Kind::VariableEntity, name);
        return g.make(Kind::EntityArgument, entity, op == "member-external-address");
    }
    throw std::runtime_error("unknown ABI template argument: " + op);
}
Id FactReader::expression(const Words& w, std::size_t& p) {
    std::string op = take(w, p);
    if (op == "template-param" || op == "function-param") {
        auto index = index_value(take(w, p));
        if (index > UINT32_MAX) throw std::runtime_error("ABI expression index too large");
        return g.make(op == "template-param" ? Kind::ExprParameter : Kind::ExprFunctionParameter, index);
    }
    if (op == "literal" || op == "value") {
        Id t = op == "value" ? type(w, p) : g.builtin(ABI_BUILTIN_TYPE_INT);
        return g.make(Kind::Value, t, 0, 0, integral_value(take(w, p)));
    }
    if (op == "unary" || op == "binary") {
        Id code = operation(take(w, p)); Id left = reference(take(w, p), BindingKind::Expression);
        if (op == "unary") return g.make(Kind::Unary, left, code);
        Id right = reference(take(w, p), BindingKind::Expression);
        return g.make(Kind::Binary, left, right, code);
    }
    if (op == "conditional") {
        Id cond = reference(take(w, p), BindingKind::Expression);
        Id left = reference(take(w, p), BindingKind::Expression);
        Id right = reference(take(w, p), BindingKind::Expression);
        return g.make(Kind::Conditional, cond, left, right);
    }
    if (op == "pack") return g.make(Kind::ExprPack, reference(take(w, p), BindingKind::Expression));
    if (op == "call") {
        Id callee = reference(take(w, p), BindingKind::Expression);
        return g.make(Kind::Call, callee, 0, 0, 0, refs(w, p, BindingKind::Expression));
    }
    if (op == "cast") {
        Id code = operation(take(w, p)); Id t = type(w, p);
        Id expr = reference(take(w, p), BindingKind::Expression);
        return g.make(Kind::Cast, t, expr, code);
    }
    if (op == "conversion") {
        Id t = type(w, p);
        return g.make(Kind::Conversion, t, 0, 0, 0, refs(w, p, BindingKind::Expression));
    }
    if (op == "template-id") {
        Id name = g.string(take(w, p));
        return g.make(Kind::TemplateId, name, 0, 0, 0, refs(w, p, BindingKind::Argument));
    }
    if (op == "type-trait") {
        Id name = g.string(take(w, p)); std::vector<Id> types;
        while (p < w.size()) types.push_back(type(w, p));
        return g.make(Kind::TypeTrait, name, 0, 0, 0, types);
    }
    if (op == "sizeof-type") return g.make(Kind::SizeofType, type(w, p));
    if (op == "member") {
        Id owner = type(w, p); bool close = boolean(take(w, p)); Id name = g.string(take(w, p));
        return g.make(Kind::Member, owner, name, close, 0, refs(w, p, BindingKind::Argument));
    }
    if (op == "object-member") {
        Id code = operation(take(w, p)); Id object = reference(take(w, p), BindingKind::Expression);
        Id name = g.string(take(w, p));
        return g.make(Kind::ObjectMember, object, name, code, 0, refs(w, p, BindingKind::Argument));
    }
    if (op == "entity-reference" || op == "entity-address")
        return g.make(Kind::EntityExpression, reference(take(w, p), BindingKind::Entity), op == "entity-address");
    throw std::runtime_error("unknown ABI dependent expression: " + op);
}
} // namespace abi_mangle
