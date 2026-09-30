#pragma once
#include "semantic/model.h"
namespace cppgm { namespace semantic {
// Cached special-member properties use the definition's access context, not
// that of a trait, explicit instantiation or other caller demanding the fact.
struct DefinitionAccess {
    bool& naming; bool saved_naming; ScopeId& scope; ScopeId saved_scope;
    DefinitionAccess(bool& n, ScopeId& s) : naming(n), saved_naming(n), scope(s), saved_scope(s)
        { naming = false; scope = 0; }
    ~DefinitionAccess() { naming = saved_naming; scope = saved_scope; }
};
} }
