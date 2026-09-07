#include "syntax/names.h"

namespace cppgm { namespace syntax {

Names::Names(bool telemetry) : telemetry_(telemetry), scopes_(1, Scope{0, 0, 0, 0}),
    imports_(1), entries_(1), slots_(32) {}

ScopeId Names::enter(ScopeId parent)
{
    Scope next{parent, 0, 0, 0}; next.depth = scopes_[parent].depth + 1;
    scopes_.push_back(next);
    return scopes_.size() - 1;
}

ScopeId Names::unnamed_namespace(ScopeId parent)
{
    if (!scopes_[parent].unnamed) {
        ScopeId child = enter(parent);
        scopes_[parent].unnamed = child;
        import(parent, child);
    }
    return scopes_[parent].unnamed;
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
        if (telemetry_) ++probes;
        const Binding& entry = entries_[slots_[i]];
        if (entry.owner == scope && entry.name == name) break;
        i = (i + 1) & (slots_.size() - 1);
    }
    if (telemetry_) ++probes;
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

Binding Names::imported(ScopeId scope, IdentifierId name, bool scope_only) const
{
    // Traversal stamps are scratch visitation state, not semantic cache keys.
    if (visited_.size() < scopes_.size()) visited_.resize(scopes_.size());
    ++traversal_;
    lookup_work_.clear();
    lookup_work_.push_back(scope);
    visited_[scope] = traversal_;
    for (std::size_t next = 0; next < lookup_work_.size(); ++next) {
        ScopeId owner = lookup_work_[next];
        if (telemetry_) ++lookup_scopes;
        Binding found = local(owner, name);
        if (scope_only ? found.target != 0 : found.category != Category::Unknown) return found;
        for (std::uint32_t i = scopes_[owner].imports; i; i = imports_[i].next) {
            ScopeId target = imports_[i].target;
            if (visited_[target] == traversal_) continue;
            visited_[target] = traversal_;
            lookup_work_.push_back(target);
        }
    }
    return Binding();
}

Binding Names::lexical(ScopeId scope, IdentifierId name, bool scope_only) const
{
    if (visited_.size() < scopes_.size()) visited_.resize(scopes_.size());
    if (nominated_.size() < scopes_.size()) {
        nominated_.resize(scopes_.size()); nomination_stamp_.resize(scopes_.size());
    }
    const std::uint64_t visit = ++traversal_;
    struct Work { ScopeId target, anchor; };
    std::vector<Work> work;
    for (;;) {
        Binding direct = local(scope, name);
        if (scope_only ? direct.target != 0 : direct.category != Category::Unknown) return direct;
        work.clear();
        for (std::uint32_t i = scopes_[scope].imports; i; i = imports_[i].next)
            work.push_back({imports_[i].target, imports_[i].directive ? unknown_scope : scope});
        for (std::size_t i = 0; i < work.size(); ++i) {
            ScopeId target = work[i].target;
            if (visited_[target] == visit) continue;
            visited_[target] = visit;
            if (telemetry_) ++lookup_scopes;
            ScopeId anchor = work[i].anchor;
            if (anchor == unknown_scope) {
                ScopeId a = scope, b = target;
                while (scopes_[a].depth > scopes_[b].depth) a = scopes_[a].parent;
                while (scopes_[b].depth > scopes_[a].depth) b = scopes_[b].parent;
                while (a != b) { a = scopes_[a].parent; b = scopes_[b].parent; }
                anchor = a;
            }
            Binding found = local(target, name);
            if (scope_only ? found.target != 0 : found.category != Category::Unknown) {
                if (nomination_stamp_[anchor] != visit) { nominated_[anchor] = found; nomination_stamp_[anchor] = visit; }
            }
            for (std::uint32_t edge = scopes_[target].imports; edge; edge = imports_[edge].next)
                work.push_back({imports_[edge].target, imports_[edge].directive ? unknown_scope : anchor});
        }
        if (nomination_stamp_[scope] == visit) return nominated_[scope];
        if (!scope) return Binding();
        scope = scopes_[scope].parent;
    }
}
Binding Names::lookup(ScopeId scope, IdentifierId name) const { return lexical(scope, name, false); }

Binding Names::qualified(ScopeId scope, IdentifierId name) const
{
    // Qualification visits the named scope and imports, never lexical parents.
    return scope == unknown_scope ? Binding() : imported(scope, name);
}

Binding Names::qualifier(ScopeId scope, IdentifierId name, bool qualified) const
{
    // A nested-name-specifier ignores non-scope declarations (including values
    // hiding a namespace alias). Terminal lookup still observes those values.
    if (scope == unknown_scope) return Binding();
    return qualified ? imported(scope, name, true) : lexical(scope, name, true);
}

void Names::bind(ScopeId scope, IdentifierId name, Category category, ScopeId target)
{
    if (!name || scope == unknown_scope) return;
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

void Names::import(ScopeId scope, ScopeId target, bool directive)
{
    imports_.push_back(Import{target, scopes_[scope].imports, directive});
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
