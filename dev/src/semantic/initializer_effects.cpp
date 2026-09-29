#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
bool Analyzer::independent_constructor(EntityId ctor)
{
    auto m = members[entities[ctor].member_info];
    // Only completed bodies/actions are stable facts. An unavailable body is
    // a conservative answer for this use, not a cached negative fact.
    if (entities[ctor].body_state != FactState::Success || m.actions_state != FactState::Success)
        return false;
    auto identity = (std::uint64_t(1) << 63) | ctor;
    if (auto known = independent_initializers.get(identity)) { ++initializer_independence_hits; return known == 2; }
    ++initializer_independence_work;
    using syntax::Kind;
    bool safe = m.constructor && !m.inherited_constructor && !m.delegated_constructor &&
        ast[entities[ctor].body].kind == Kind::Compound && !ast[entities[ctor].body].first;
    for (unsigned i = 0; safe && i < m.action_count; ++i) {
        auto action = subobject_actions[m.action_begin+i];
        auto t = types[action.type];
        safe = action.field && !action.constructor && !(t.cv & 2) &&
            t.kind != TypeKind::Array && t.kind != TypeKind::LRef && t.kind != TypeKind::RRef && !class_value(action.type);
        auto source = action.initializer;
        while (ast[source].kind == Kind::Initializer || ast[source].kind == Kind::ParenInitializer ||
            ast[source].kind == Kind::ParenArguments || ast[source].kind == Kind::BracedInit)
            source = ast[source].first;
        safe &= independent_initializer(source);
    }
    independent_initializers.put(identity,safe ? 2 : 1);
    return safe;
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
    switch (ast[n].kind) {
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
        safe &= ast[n].op != KW_THIS; break;
    case Kind::Sizeof: case Kind::SizeofPack: case Kind::TypeTrait:
        safe = true; break; // C++11 operands are unevaluated.
    case Kind::IdExpression: {
        auto kind = types[entities[value.entity].type].kind;
        safe &= value.entity && !nonstatic_field(value.entity) &&
            kind != TypeKind::LRef && kind != TypeKind::RRef && !class_value(value.type);
        break;
    }
    case Kind::Unary:
        safe &= ast[n].op == OP_PLUS || ast[n].op == OP_MINUS || ast[n].op == OP_COMPL || ast[n].op == OP_LNOT;
        // fall through
    case Kind::Binary: case Kind::Conditional: case Kind::Parenthesized:
        for (unsigned i = 0; safe && i < value.count; ++i) {
            auto c = conversions[value.conversions+i];
            safe &= c.kind == Conversion::Kind::Standard && !c.function;
        }
        for (auto child = ast[n].first; safe && child; child = ast[child].next)
            safe &= independent_initializer(child);
        break;
    default: safe = false; break;
    }
    independent_initializers.put(n,safe ? 2 : 1);
    return safe;
}
} }
