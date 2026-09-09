#include "semantic/analyzer.h"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace cppgm { namespace semantic {
namespace {
std::uint64_t layout_add(std::uint64_t a, std::uint64_t b)
{
    if (b > std::numeric_limits<std::uint64_t>::max() - a)
        throw std::runtime_error("class size overflow");
    return a + b;
}
std::uint64_t layout_align(std::uint64_t bytes, std::uint64_t alignment)
{
    std::uint64_t remainder = bytes % alignment;
    return remainder ? layout_add(bytes, alignment - remainder) : bytes;
}
}
std::uint64_t Analyzer::size(TypeId id, bool alignment)
{
    Type t = types[id];
    if (t.kind == TypeKind::LRef || t.kind == TypeKind::RRef) return size(t.child, alignment);
    if (t.kind == TypeKind::Pointer) return 8;
    if (t.kind == TypeKind::MemberPointer) return !alignment && types[t.child].kind == TypeKind::Function ? 16 : 8;
    if (t.kind == TypeKind::Array) {
        if (!t.bound) throw std::runtime_error("sizeof incomplete array");
        if (alignment) return size(t.child, true);
        std::uint64_t element = size(t.child);
        if (t.bound > std::numeric_limits<std::uint64_t>::max() / element)
            throw std::runtime_error("array size overflow");
        return t.bound * element;
    }
    if (t.kind == TypeKind::Fundamental && t.fundamental != FT_VOID) return fundamental_width(t.fundamental);
    if (t.kind == TypeKind::Named && entities[t.entity].key == KW_ENUM) return size(entities[t.entity].underlying, alignment);
    if (t.kind == TypeKind::Named && entities[t.entity].complete) {
        EntityId e = t.entity;
        std::uint32_t layout = entities[e].class_info;
        class_layout(e);
        return alignment ? class_facts[layout].alignment : class_facts[layout].size;
    }
    throw std::runtime_error("sizeof unsupported or incomplete type");
}
void Analyzer::class_layout(EntityId e)
{
    auto info = entities[e].class_info;
    if (class_facts[info].layout_state == 2) return;
    if (class_facts[info].layout_state == 1) throw std::runtime_error("recursive class layout");
    class_facts[info].layout_state = 1;
    std::uint64_t cursor = 0, align = 1, ordinary_end = 0;
    bool is_union = entities[e].key == KW_UNION;
    Index empty_bases;
    bool has_empty_base = false;
    for (auto b = class_facts[info].first_base; b; b = bases[b].next) {
        TypeId base = entities[bases[b].base].type;
        auto bytes = size(base);
        if (!class_facts[entities[types[base].entity].class_info].empty) {
            if (bytes > std::numeric_limits<std::uint64_t>::max()/8) throw std::runtime_error("base layout overflow");
            cursor = layout_add(cursor, bytes*8); class_facts[info].empty = false;
        } else for (EntityId empty = types[base].entity; empty;) {
            has_empty_base = true;
            empty_bases.put(empty, 1);
            auto edge = class_facts[entities[empty].class_info].first_base;
            empty = edge ? bases[edge].base : 0;
        }
        align = std::max(align, size(base, true));
    }
    for (auto d = scopes[entities[e].scope].first_decl; d; d = declarations[d].next) {
        EntityId id = declarations[d].entity;
        Entity member = entities[id];
        if (member.kind != EntityKind::Variable || member.is_static || member.owner != entities[e].scope) continue;
        auto f = field_fact(id);
        bool reference = types[member.type].kind == TypeKind::LRef || types[member.type].kind == TypeKind::RRef;
        auto field_align = reference ? 8 : size(member.type, true);
        auto field_size = reference ? 8 : size(member.type);
        if (f.alignment && f.alignment < field_align) throw std::runtime_error("weakened field alignment");
        if (class_facts[info].packing) field_align = std::min<std::uint64_t>(field_align, class_facts[info].packing);
        field_align = std::max(field_align, f.alignment);
        if (!f.bit_field || member.name) align = std::max(align, field_align);
        std::uint64_t at = is_union ? 0 : cursor;
        if (f.bit_field) {
            if (f.alignment) throw std::runtime_error("aligned bit-field");
            auto bits = field_size*8;
            if (!f.declared_width) {
                if (!is_union) cursor = layout_align(layout_add(cursor, 7)/8, field_align)*8;
                continue;
            }
            if (f.declared_width > bits) at = layout_align(layout_add(at, 7)/8, field_align)*8;
            else if (at%bits + f.declared_width > bits) at = layout_align(at, bits);
            entities[id].member_offset = at/bits*field_size;
            field_metadata(id).shift = at%bits;
            field_metadata(id).may_clear_unit = entities[id].member_offset >= ordinary_end;
            auto end = layout_add(at, f.declared_width);
            cursor = is_union ? std::max(cursor, end) : end;
        } else {
            std::uint64_t offset = layout_align(layout_add(at, 7)/8, field_align);
            // At offset zero, complete member objects must not overlap a base
            // subobject of the same empty type, including nested zero offsets.
            if (has_empty_base && !offset && types[member.type].kind == TypeKind::Named && entities[types[member.type].entity].class_info) {
                std::vector<EntityId> work(1, types[member.type].entity);
                Index seen; bool collision = false;
                for (std::size_t i = 0; i < work.size() && !collision; ++i) {
                    EntityId candidate = work[i];
                    if (seen.get(candidate)) continue;
                    seen.put(candidate, 1); collision = empty_bases.get(candidate);
                    for (auto edge = class_facts[entities[candidate].class_info].first_base; edge; edge = bases[edge].next) work.push_back(bases[edge].base);
                    for (auto decl = scopes[entities[candidate].scope].first_decl; decl; decl = declarations[decl].next) {
                        auto child = entities[declarations[decl].entity];
                        if (!nonstatic_field(declarations[decl].entity) || child.member_offset || types[child.type].kind != TypeKind::Named) continue;
                        if (entities[types[child.type].entity].class_info) work.push_back(types[child.type].entity);
                    }
                }
                if (collision) offset = field_align;
            }
            entities[id].member_offset = offset;
            auto end = layout_add(offset, field_size);
            ordinary_end = std::max(ordinary_end, end);
            if (end > std::numeric_limits<std::uint64_t>::max()/8) throw std::runtime_error("field layout overflow");
            cursor = is_union ? std::max(cursor, end*8) : end*8;
        }
        class_facts[info].empty = false;
    }
    auto requested = class_facts[info].requested_alignment;
    if (requested && requested < align) throw std::runtime_error("weakened class alignment");
    align = std::max(align, requested);
    class_facts[info].alignment = align;
    class_facts[info].size = layout_align(std::max<std::uint64_t>(1, layout_add(cursor, 7)/8), align);
    class_facts[info].layout_state = 2;
}
} }
