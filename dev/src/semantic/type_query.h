#pragma once
#include "semantic/model.h"
namespace cppgm { namespace semantic {
using QueryId = std::uint32_t;
enum class QueryKind : unsigned char { Value, This, String, Parameter, TemplateValueParameter, Name, TypeValue, Unary, Binary, Call, Member, Parenthesized, Sizeof, SizeofPack, Expansion, New, QualifiedValue, Conditional, Cast, Destructor, List, ListInitialization };
// Canonical semantic type queries. Source locations remain on their source
// nodes; these records retain resolved declaration/type/operation identities.
struct TypeQuery {
    QueryKind kind = QueryKind::Value;
    bool null_pointer_constant = false;
    bool dependent_name = false; // Source call/operator type dependence, distinct from value dependence.
    ETokenType op = TOK_INVALID;
    TypeId type = 0;
    EntityId entity = 0;
    IdentifierId name = 0;
    ScopeId context = 0, naming = 0;
    std::uint32_t arguments = 0, offset = 0, count = 0;
    std::uint64_t value = 0;
};
struct TypeQueryFact {
    Expression expression;
    TypeId declared_type = 0;
    EntityId selected = 0; // Unevaluated call/operator choice; no body demand.
    std::uint32_t arrow = 0;
    TypeId surrogate = 0;
    std::uint32_t initialization = 0;
    FactState state = FactState::NotStarted;
    bool dependent = false;
    bool incomplete = false; // Depends on incomplete prerequisites, possibly discarded candidates.
    enum class Failure : unsigned char { None, NoViable, Ambiguous, Deleted, InvalidOperands };
    Failure failure = Failure::None;
    static TypeQueryFact failed(Failure reason) {
        TypeQueryFact fact; fact.state = FactState::Failure; fact.failure = reason; return fact;
    }
};
// Stack-local collection of an unresolved query prerequisite of a failed
// substitution. Query reverse edges retain all of its completion dependencies.
struct SubstitutionDependency {
    QueryId& pending; QueryId prior; bool succeeded = false;
    explicit SubstitutionDependency(QueryId& p) : pending(p), prior(p) { pending = 0; }
    ~SubstitutionDependency() { pending = succeeded || !pending ? prior : pending; }
};
} }
