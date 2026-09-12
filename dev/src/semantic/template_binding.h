#pragma once
#include "semantic/model.h"
namespace cppgm { namespace semantic {
// A source name has one definition-time binding. Template-owned declarations
// retain lexical identity here; only fixed external declarations are reused by
// concrete occurrences. Type/conversion/layout facts have separate owners.
struct TemplateBinding {
    EntityId entity = 0;
    ScopeId scope = 0;
    bool dependent = false;
};
} }
