#include "semantic/analyzer.h"
#include "support/type_traits.h"
namespace cppgm { namespace semantic {
bool Analyzer::independent_constructor(EntityId ctor, bool local_objects)
{
    auto m = members[entities[ctor].member_info];
    // Only completed bodies/actions are stable facts. An unavailable body is
    // a conservative answer for this use, not a cached negative fact.
    if (entities[ctor].body_state != FactState::Success || m.actions_state != FactState::Success)
        return false;
    auto identity = (std::uint64_t(1) << 63) | (std::uint64_t(local_objects) << 62) | ctor;
    if (auto known = independent_initializers.get(identity)) { ++initializer_independence_hits; return known == 2; }
    ++initializer_independence_work;
    using syntax::Kind;
    bool safe = m.constructor && !m.inherited_constructor && !m.delegated_constructor &&
        ast.kind(entities[ctor].body) == Kind::Compound && !ast.first(entities[ctor].body);
    for (unsigned i = 0; safe && i < m.action_count; ++i) {
        auto action = subobject_actions[m.action_begin+i];
        auto t = types[action.type];
        safe = action.field && !action.constructor && !(t.cv & 2) &&
            t.kind != TypeKind::Array && t.kind != TypeKind::LRef && t.kind != TypeKind::RRef && !class_value(action.type);
        auto source = action.initializer;
        while (ast.kind(source) == Kind::Initializer || ast.kind(source) == Kind::ParenInitializer ||
            ast.kind(source) == Kind::ParenArguments || ast.kind(source) == Kind::BracedInit)
            if (conversions[expressions[source].incoming].kind == Conversion::Kind::List) break;
            else source = ast.first(source);
        safe &= independent_initializer(source) || (local_objects && constructor_local_operand(source,ctor));
    }
    independent_initializers.put(identity,safe ? 2 : 1);
    return safe;
}
bool Analyzer::constructor_local_operand(NodeId n, EntityId ctor, bool object)
{
    using syntax::Kind;
    auto x = expressions[n];
    if (!x.ready || x.form != ExpressionForm::Ordinary || (types[x.type].cv & 2)) return false;
    auto c = conversions[x.incoming];
    if (c.function || (c.kind != Conversion::Kind::Standard && c.kind != Conversion::Kind::Explicit)) return false;
    switch (ast.kind(n)) {
    case Kind::IdExpression:
        return (entities[x.entity].kind == EntityKind::Parameter && entities[x.entity].owner == entities[ctor].scope &&
            (object || !class_value(x.type))) ||
            (nonstatic_field(x.entity) && entities[x.entity].owner == entities[ctor].owner);
    case Kind::KeywordLiteral: return object && ast.op(n) == KW_THIS;
    case Kind::Member:
        // A direct member of the fresh source object, or of this destination,
        // is private construction state. Following an arbitrary pointer is not.
        if (ast.op(n) == OP_ARROW && ast.op(ast.first(n)) != KW_THIS) return false;
        return nonstatic_field(x.entity) && constructor_local_operand(ast.first(n),ctor,true);
    case Kind::Unary:
        if (ast.op(n) == OP_AMP) return constructor_local_operand(ast.first(n),ctor,true);
        if (ast.op(n) != OP_PLUS && ast.op(n) != OP_MINUS && ast.op(n) != OP_COMPL && ast.op(n) != OP_LNOT) return false;
        // fall through
    case Kind::Parenthesized: case Kind::Binary: case Kind::Conditional:
        for (unsigned i = 0; i < x.count; ++i) {
            auto c = conversions[x.conversions+i];
            if (c.function || c.kind != Conversion::Kind::Standard) return false;
        }
        for (auto child = ast.first(n); child; child = ast.next(child))
            if (!independent_initializer(child) && !constructor_local_operand(child,ctor,object)) return false;
        return true;
    default: return false;
    }
}
bool Analyzer::independent_materialization(NodeId n)
{
    if (independent_initializer(n)) return true;
    auto x = expressions[n]; auto ctor = facts[n].entity;
    if (x.form != ExpressionForm::Construction || !independent_constructor(ctor,true)) return false;
    for (unsigned i = 0; i < x.argument_count; ++i) {
        auto c = conversions[x.conversions+i];
        if (c.kind != Conversion::Kind::Standard || c.function || !independent_initializer(call_argument(x,i))) return false;
    }
    return true;
}
bool Analyzer::independent_initializer(NodeId n)
{
    if (!n) return true;
    if (auto known = independent_initializers.get(n)) { ++initializer_independence_hits; return known == 2; }
    ++initializer_independence_work;
    // Only checked, storage-independent scalar operands may be evaluated before
    // the aggregate helper installs earlier fields. Unknown calls, aliases and
    // volatile accesses retain ordinary ordered destination initialization.
    // Source occurrences own these immutable summaries; templates have distinct
    // substituted occurrences. Never resolve names or conversions in lowering.
    auto value = expressions[n];
    auto incoming = conversions[value.incoming];
    bool safe = value.ready && value.form == ExpressionForm::Ordinary &&
        incoming.kind == Conversion::Kind::Standard && !incoming.function &&
        !(types[value.type].cv & 2);
    using syntax::Kind;
    switch (ast.kind(n)) {
    case Kind::Call: {
        // A source temporary built by a checked empty constructor with only
        // independent scalar member initializers cannot observe the enclosing
        // aggregate. Its later selected move remains inside the helper, after
        // earlier fields have been stored. Unknown bodies keep the fallback.
        auto ctor = facts[n].entity;
        safe = value.ready && value.form == ExpressionForm::Construction &&
            independent_constructor(ctor);
        for (unsigned i = 0; safe && i < value.argument_count; ++i) {
            auto c = conversions[value.conversions+i];
            safe &= c.kind == Conversion::Kind::Standard && !c.function && independent_initializer(call_argument(value,i));
        }
        break;
    }
    case Kind::Literal: break;
    case Kind::KeywordLiteral:
        safe &= ast.op(n) != KW_THIS; break;
    case Kind::Sizeof: case Kind::SizeofPack: case Kind::TypeTrait:
        // A polymorphic typeid evaluates its operand. It may observe earlier
        // aggregate members, so it cannot be hoisted into helper arguments.
        safe = ast.op(n) != KW_TYPEID || !rtti_expression(n).dynamic;
        if (ast.kind(n) == Kind::TypeTrait && BuiltinTrait(ast.flags(n)) == BuiltinTrait::Offsetof)
            for (auto step = ast.next(ast.first(n)); step; step = ast.next(step))
                if (ast.kind(step) == Kind::Subscript) safe &= independent_initializer(ast.first(step));
        break;
    case Kind::IdExpression: {
        auto kind = types[entities[value.entity].type].kind;
        safe &= value.entity && !nonstatic_field(value.entity) &&
            kind != TypeKind::LRef && kind != TypeKind::RRef && !class_value(value.type);
        break;
    }
    case Kind::Unary:
        safe &= ast.op(n) == OP_PLUS || ast.op(n) == OP_MINUS || ast.op(n) == OP_COMPL || ast.op(n) == OP_LNOT;
        // fall through
    case Kind::Binary: case Kind::Conditional: case Kind::Parenthesized:
        for (unsigned i = 0; safe && i < value.count; ++i) {
            auto c = conversions[value.conversions+i];
            safe &= c.kind == Conversion::Kind::Standard && !c.function;
        }
        for (auto child = ast.first(n); safe && child; child = ast.next(child))
            safe &= independent_initializer(child);
        break;
    default: safe = false; break;
    }
    independent_initializers.put(n,safe ? 2 : 1);
    return safe;
}
} }
