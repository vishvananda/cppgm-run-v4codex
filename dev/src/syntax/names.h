#pragma once
#include "preprocess/identifier_table.h"
#include <vector>

namespace cppgm { namespace syntax {

typedef std::uint32_t ScopeId;
const ScopeId unknown_scope = static_cast<ScopeId>(-1);
enum class Category : unsigned char { Unknown, Type, Value, TemplateType, TemplateValue, Namespace };
struct Binding {
    ScopeId owner = 0, target = 0;
    IdentifierId name = 0;
    Category category = Category::Unknown;
};

// TU-owned flat table keyed by (scope identity, interned identifier).
// Namespace reopenings share identity; parameters get new lexical scopes.
class Names {
public:
    explicit Names(bool telemetry = false);
    ScopeId enter(ScopeId parent);
    ScopeId unnamed_namespace(ScopeId parent);
    ScopeId parent(ScopeId scope) const;
    Binding local(ScopeId scope, IdentifierId name) const;
    Binding lookup(ScopeId scope, IdentifierId name) const;
    Binding qualified(ScopeId scope, IdentifierId name) const;
    Binding qualifier(ScopeId scope, IdentifierId name, bool qualified = false) const;
    void bind(ScopeId scope, IdentifierId name, Category category, ScopeId target = 0);
    void import(ScopeId scope, ScopeId target);
    std::size_t scope_count() const { return scopes_.size(); }
    mutable std::size_t probes = 0, lookup_scopes = 0;
private:
    bool telemetry_;
    struct Scope { ScopeId parent; std::uint32_t imports; ScopeId unnamed; };
    struct Import { ScopeId target; std::uint32_t next; };
    std::vector<Scope> scopes_;
    std::vector<Import> imports_;
    std::vector<Binding> entries_;
    std::vector<std::uint32_t> slots_;
    mutable std::vector<ScopeId> lookup_work_;
    mutable std::vector<std::uint64_t> visited_;
    mutable std::uint64_t traversal_ = 0;
    Binding imported(ScopeId scope, IdentifierId name, bool scope_only = false) const;
    std::size_t slot(ScopeId scope, IdentifierId name) const;
    void grow();
};

bool type_category(Category category);
bool template_category(Category category);

} }
