#pragma once
#include "preprocess/identifier_table.h"
#include <vector>

namespace cppgm { namespace syntax {

typedef std::uint32_t ScopeId;
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
    Names();
    ScopeId enter(ScopeId parent);
    ScopeId parent(ScopeId scope) const;
    Binding local(ScopeId scope, IdentifierId name) const;
    Binding lookup(ScopeId scope, IdentifierId name) const;
    void bind(ScopeId scope, IdentifierId name, Category category, ScopeId target = 0);
    void import(ScopeId scope, ScopeId target);
    std::size_t probes = 0;
private:
    struct Scope { ScopeId parent; std::uint32_t imports; };
    struct Import { ScopeId target; std::uint32_t next; };
    std::vector<Scope> scopes_;
    std::vector<Import> imports_;
    std::vector<Binding> entries_;
    std::vector<std::uint32_t> slots_;
    std::size_t slot(ScopeId scope, IdentifierId name) const;
    void grow();
};

bool type_category(Category category);
bool template_category(Category category);

} }
