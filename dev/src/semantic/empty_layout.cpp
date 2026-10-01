#include "semantic/analyzer.h"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace cppgm { namespace semantic {
unsigned Analyzer::EmptyLayout::offset_id(std::uint64_t offset)
{
    if (offsets.empty()) offsets.push_back(0);
    auto id = offset_ids.get(offset);
    if (!id) { id = offsets.size(); offsets.push_back(offset); offset_ids.put(offset,id); }
    return id;
}
void Analyzer::EmptyLayout::insert(EntityId e, std::uint64_t offset)
{
    auto k = sem.key(e,offset_id(offset));
    if (seen.get(k)) return;
    seen.put(k,1); positions.push_back({e,offset});
}
bool Analyzer::EmptyLayout::has(TypeId t)
{
    while (sem.types[t].kind == TypeKind::Array) t = sem.types[t].child;
    return sem.class_value(t) && sem.class_facts[sem.entities[sem.types[t].entity].class_info].empty_types_count;
}
bool Analyzer::EmptyLayout::contains(TypeId t, EntityId empty, std::uint64_t offset)
{
    if (!has(t) || offset >= sem.size(t)) return false;
    ++sem.layout_empty_work;
    if (sem.types[t].kind == TypeKind::Array)
        return contains(sem.types[t].child,empty,offset%sem.size(sem.types[t].child));
    auto cls = sem.types[t].entity;
    const auto& info = sem.class_facts[sem.entities[cls].class_info];
    if (info.empty_types_count <= 64) {
        for (unsigned i=0;i<info.empty_types_count;++i) {
            auto p = sem.layout_empty_types[info.empty_types_begin+i];
            if (p.type == empty && p.offset == offset) return true;
        }
        return false;
    }
    if (info.empty && cls == empty && !offset) return true;
    for (auto b=info.first_base;b;b=sem.bases[b].next) {
        auto at = sem.bases[b].offset;
        if (offset >= at && contains(sem.entities[sem.bases[b].base].type,empty,offset-at)) return true;
    }
    for (auto d=sem.scopes[sem.entities[cls].scope].first_decl;d;d=sem.declarations[d].next) {
        auto e = sem.declarations[d].entity;
        if (!sem.nonstatic_field(e) || sem.entities[e].owner != sem.entities[cls].scope) continue;
        auto at = sem.entities[e].member_offset;
        if (offset >= at && contains(sem.entities[e].type,empty,offset-at)) return true;
    }
    return false;
}
bool Analyzer::EmptyLayout::overlaps(TypeId a, std::uint64_t at, TypeId b, std::uint64_t bt)
{
    if (!has(a) || !has(b)) return false;
    auto bytes = sem.size(a), other = sem.size(b);
    if ((at >= bt && at-bt >= other) || (bt >= at && bt-at >= bytes)) return false;
    ++sem.layout_empty_work;
    if (sem.types[a].kind == TypeKind::Array) {
        auto child = sem.types[a].child; auto stride = sem.size(child);
        auto begin = bt > at ? (bt-at)/stride : 0;
        auto span = bt >= at ? ((bt-at > bytes || other > bytes-(bt-at)) ? bytes : bt-at+other) : std::min(bytes,other-(at-bt));
        auto end = (span-1)/stride+1;
        for (auto i=begin;i<end;++i) if (overlaps(child,at+i*stride,b,bt)) return true;
        return false;
    }
    auto cls = sem.types[a].entity;
    const auto& info = sem.class_facts[sem.entities[cls].class_info];
    if (info.empty_types_count <= 64) {
        for (unsigned i=0;i<info.empty_types_count;++i) {
            auto p = sem.layout_empty_types[info.empty_types_begin+i];
            if (at+p.offset >= bt && contains(b,p.type,at+p.offset-bt)) return true;
        }
        return false;
    }
    if (info.empty && at >= bt && contains(b,cls,at-bt)) return true;
    for (auto edge=info.first_base;edge;edge=sem.bases[edge].next)
        if (overlaps(sem.entities[sem.bases[edge].base].type,at+sem.bases[edge].offset,b,bt)) return true;
    for (auto d=sem.scopes[sem.entities[cls].scope].first_decl;d;d=sem.declarations[d].next) {
        auto e = sem.declarations[d].entity;
        if (sem.nonstatic_field(e) && sem.entities[e].owner == sem.entities[cls].scope &&
            overlaps(sem.entities[e].type,at+sem.entities[e].member_offset,b,bt)) return true;
    }
    return false;
}
bool Analyzer::EmptyLayout::conflicts(TypeId t, std::uint64_t at)
{
    if (!has(t)) return false;
    const auto& info = sem.class_facts[sem.class_value(t) ? sem.entities[sem.types[t].entity].class_info : 0];
    if (info.empty_types_count && info.empty_types_count <= 64) {
        for (unsigned i=0;i<info.empty_types_count;++i) {
            auto p = sem.layout_empty_types[info.empty_types_begin+i]; ++sem.layout_empty_work;
            auto id = offset_ids.get(at+p.offset);
            if (id && seen.get(sem.key(p.type,id))) return true;
        }
    } else for (auto p : positions)
        if (p.offset >= at && contains(t,p.type,p.offset-at)) return true;
    for (auto p : large) if (overlaps(t,at,p.type,p.offset)) return true;
    return false;
}
void Analyzer::EmptyLayout::add(TypeId t, std::uint64_t at)
{
    if (!has(t)) return;
    const auto& info = sem.class_facts[sem.class_value(t) ? sem.entities[sem.types[t].entity].class_info : 0];
    if (!info.empty_types_count || info.empty_types_count > 64) { large.push_back({t,at}); return; }
    for (unsigned i=0;i<info.empty_types_count;++i) {
        auto p = sem.layout_empty_types[info.empty_types_begin+i];
        insert(p.type,at+p.offset);
    }
}
std::uint64_t Analyzer::EmptyLayout::place(TypeId t, std::uint64_t start, std::uint64_t alignment, bool zero)
{
    if (!has(t)) return start;
    std::uint64_t at = start;
    if (zero && !conflicts(t,0)) at = 0;
    else {
        auto k = sem.key(t,offset_id(alignment));
        auto hint = frontiers.get(k);
        if (hint) at = std::max(at,offsets[hint]);
        while (conflicts(t,at)) {
            if (at > std::numeric_limits<std::uint64_t>::max()-alignment) throw std::runtime_error("empty layout overflow");
            at += alignment;
        }
        // Every earlier trial is permanently occupied. Repeated placements
        // of the same shape/alignment resume here, rather than rescanning.
        frontiers.put(k,offset_id(at));
    }
    add(t,at); return at;
}
void Analyzer::EmptyLayout::publish(std::uint32_t index, EntityId empty)
{
    if (empty) insert(empty,0);
    auto& fact = sem.class_facts[index];
    if (!large.empty() || positions.size() > 64) { fact.empty_types_count = 65; ++sem.layout_empty_fallbacks; return; }
    fact.empty_types_begin = sem.layout_empty_types.size(); fact.empty_types_count = positions.size();
    sem.layout_empty_types.insert(sem.layout_empty_types.end(),positions.begin(),positions.end());
}
} }
