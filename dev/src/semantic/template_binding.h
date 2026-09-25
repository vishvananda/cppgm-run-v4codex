#pragma once
#include "semantic/model.h"
namespace cppgm { namespace semantic {
// A source name has one definition-time binding. Template-owned declarations
// retain lexical identity here; only fixed external declarations are reused by
// concrete occurrences. Type/conversion/layout facts have separate owners.
struct TemplateBinding {
    EntityId entity = 0;
    ScopeId scope = 0;
    EntityId qualifier_pack = 0;
    bool dependent = false;
};
struct TemplateTypeAccess { TypeId qualifier = 0; IdentifierId name = 0; ScopeId scope = 0; };
struct TemplateAliasFact { TypeId type = 0; FactState state = FactState::NotStarted; };
// A redeclaration can rename parameters or add a body, but dependent lookup
// in its signature remains owned by the first declaration [temp.over.link].
struct TemplateFirstSignature {
    TypeId type = 0;
    NodeId declarator = 0, selected_parameters = 0;
    ScopeId environment = 0;
    std::uint32_t frame = 0;
};
// Declaration/type identity is shared; a concrete owning class supplies layout.
struct TemplateObjectContext { EntityId owner = 0; unsigned char cv = 0; bool available = false; };
struct TemplateMemberUse { EntityId entity = 0; std::uint32_t object = 0; TypeId type = 0; };
// Immutable substitution overlays. A declaration head owns the parameter slice;
// its specialization owns the argument pack. An out-of-line head can overlay
// the defining class head without copying either set of bindings.
struct TemplateSubstitutionFrame {
    std::uint32_t specialization = 0, parameters = 0, count = 0, parent = 0;
    bool expansion = false, symbolic = false;
    std::uint32_t arguments = 0; // Explicit selected-owner tuple for a renamed declaration head.
    std::uint32_t overlay = 0; // immutable (parameter, argument) pairs for one expansion lane
};
} }
