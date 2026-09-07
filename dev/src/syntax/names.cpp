#include "syntax/names.h"

namespace cppgm { namespace syntax {

Names::Names() : scopes_(1, Scope{0, 0}), imports_(1), entries_(1), slots_(32) {}

ScopeId Names::enter(ScopeId parent)
{
    scopes_.push_back(Scope{parent, 0});
    return scopes_.size() - 1;
}

ScopeId Names::parent(ScopeId scope) const { return scopes_[scope].parent; }

std::size_t Names::slot(ScopeId scope, IdentifierId name) const
{
    std::uint64_t hash = (std::uint64_t(scope) << 32) | name;
    hash ^= hash >> 30;
    hash *= 0xbf58476d1ce4e5b9ULL;
    hash ^= hash >> 27;
    std::size_t i = hash & (slots_.size() - 1);
    while (slots_[i]) {
        const Binding& entry = entries_[slots_[i]];
        if (entry.owner == scope && entry.name == name) break;
        i = (i + 1) & (slots_.size() - 1);
    }
    return i;
}

void Names::grow()
{
    slots_.assign(slots_.size() * 2, 0);
    for (std::size_t i = 1; i < entries_.size(); ++i)
        slots_[slot(entries_[i].owner, entries_[i].name)] = i;
}

Binding Names::local(ScopeId scope, IdentifierId name) const
{
    return entries_[slots_[slot(scope, name)]];
}

Binding Names::lookup(ScopeId scope, IdentifierId name) const
{
    for (;;) {
        Binding found = local(scope, name);
        if (found.category != Category::Unknown) return found;
        for (std::uint32_t i = scopes_[scope].imports; i; i = imports_[i].next) {
            found = local(imports_[i].target, name);
            if (found.category != Category::Unknown) return found;
        }
        if (!scope) return Binding();
        scope = scopes_[scope].parent;
    }
}

void Names::bind(ScopeId scope, IdentifierId name, Category category, ScopeId target)
{
    if (!name) return;
    if (entries_.size() * 2 >= slots_.size()) grow();
    std::size_t i = slot(scope, name);
    if (!slots_[i]) {
        slots_[i] = entries_.size();
        entries_.push_back(Binding());
    }
    Binding& entry = entries_[slots_[i]];
    entry.owner = scope;
    entry.name = name;
    entry.category = category;
    entry.target = target;
}

void Names::import(ScopeId scope, ScopeId target)
{
    imports_.push_back(Import{target, scopes_[scope].imports});
    scopes_[scope].imports = imports_.size() - 1;
}

bool type_category(Category category)
{
    return category == Category::Type || category == Category::TemplateType;
}

bool template_category(Category category)
{
    return category == Category::TemplateType || category == Category::TemplateValue;
}

} }
