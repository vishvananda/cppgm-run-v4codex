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
// Declaration/type identity is shared; a concrete owning class supplies layout.
struct TemplateObjectContext { EntityId owner = 0; unsigned char cv = 0; bool available = false; };
struct TemplateMemberUse { EntityId entity = 0; std::uint32_t object = 0; TypeId type = 0; };
} }
