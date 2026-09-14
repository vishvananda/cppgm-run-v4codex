#pragma once
#include "semantic/model.h"
namespace cppgm { namespace semantic {
using QueryId = std::uint32_t;
enum class QueryKind : unsigned char { Value, Parameter, TemplateValueParameter, Name, TypeValue, Unary, Binary, Call, Member, Parenthesized, Sizeof, QualifiedValue, Conditional, Cast };
// Canonical semantic type queries. Source locations remain on their source
// nodes; these records retain resolved declaration/type/operation identities.
struct TypeQuery {
    QueryKind kind = QueryKind::Value;
    ETokenType op = TOK_INVALID;
    TypeId type = 0;
    EntityId entity = 0;
    IdentifierId name = 0;
    ScopeId context = 0;
    bool null_pointer_constant = false;
    std::uint32_t arguments = 0, offset = 0, count = 0;
    std::uint64_t value = 0;
};
struct TypeQueryFact {
    Expression expression;
    TypeId declared_type = 0;
    EntityId selected = 0; // Unevaluated call/operator choice; no body demand.
    FactState state = FactState::NotStarted;
    bool dependent = false;
};
} }
