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
std::uint64_t Analyzer::size(TypeId id, bool alignment, bool probe)
{
    Type t = types[id];
    if (definitions && t.kind == TypeKind::Named && entities[t.entity].class_info) complete_class(t.entity);
    if (t.kind == TypeKind::LRef || t.kind == TypeKind::RRef) return size(t.child, alignment,probe);
    if (t.kind == TypeKind::Pointer) return 8;
    if (t.kind == TypeKind::MemberPointer) return !alignment && types[t.child].kind == TypeKind::Function ? 16 : 8;
    if (t.kind == TypeKind::Array) {
        if (!t.bound) { if (probe) return 0; throw std::runtime_error("sizeof incomplete array"); }
        if (alignment) return size(t.child, true,probe);
        std::uint64_t element = size(t.child,false,probe);
        if (!element) return 0;
        if (t.bound > std::numeric_limits<std::uint64_t>::max() / element) {
            if (probe) return 0;
            throw std::runtime_error("array size overflow");
        }
        return t.bound * element;
    }
    if (t.kind == TypeKind::Fundamental && t.fundamental != FT_VOID) return fundamental_width(t.fundamental);
    if (t.kind == TypeKind::Named && entities[t.entity].key == KW_ENUM) return size(entities[t.entity].underlying, alignment,probe);
    if (t.kind == TypeKind::Named && entities[t.entity].complete) {
        EntityId e = t.entity;
        std::uint32_t layout = entities[e].class_info;
        class_layout(e);
        return alignment ? class_facts[layout].alignment : class_facts[layout].size;
    }
    if (probe) return 0;
    throw std::runtime_error("sizeof unsupported or incomplete type");
}
void Analyzer::class_layout(EntityId e)
{
    auto info = entities[e].class_info;
    if (class_facts[info].layout_state == FactState::Success) return;
    if (class_facts[info].layout_state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::ClassLayout,e,entities[e].definition);
    if (class_facts[info].layout_state == FactState::Active) throw std::runtime_error("recursive class layout");
    class_facts[info].layout_state = FactState::Active;
    try {
    std::uint64_t cursor = 0, align = 1, ordinary_end = 0;
    bool is_union = entities[e].key == KW_UNION;
    bool nearly_empty = dynamic_class(e);
    unsigned dynamic_bases = 0;
    auto primary = class_facts[info].primary_base;
    if (dynamic_class(e)) {
        align = 8; class_facts[info].empty = false;
        if (!primary) cursor = 64;
    }
    EmptyLayout empty(*this);
    std::uint64_t extent = cursor/8;
    std::vector<std::uint32_t> order;
    if (primary && !bases[primary].virtual_base) order.push_back(primary);
    if (primary && bases[primary].virtual_base) {
        auto base = bases[primary].base;
        const auto& fact = class_facts[entities[base].class_info];
        cursor = fact.nonvirtual_size*8; extent = fact.nonvirtual_size;
        align = std::max(align,fact.nonvirtual_alignment);
        empty.merge(entities[base].type);
    }
    for (auto b = class_facts[info].first_base; b; b = bases[b].next)
        if (b != primary && !bases[b].virtual_base) order.push_back(b);
    for (auto b : order) {
        TypeId base = entities[bases[b].base].type;
        size(base);
        auto base_info = entities[types[base].entity].class_info;
        auto bytes = class_facts[base_info].nonvirtual_size;
        bases[b].offset = class_facts[entities[types[base].entity].class_info].empty ? 0 : layout_align(cursor/8, class_facts[base_info].nonvirtual_alignment);
        // A repeated empty type may not share an address. Type summaries are
        // conservative: on overlap (or budget exhaustion), place this entire
        // subobject beyond existing storage instead of searching for a hole.
        if (empty.merge(base) && bases[b].offset < extent)
            bases[b].offset = layout_align(extent,class_facts[base_info].nonvirtual_alignment);
        if (!bases[b].virtual_base) {
            const auto& fact = class_facts[base_info];
            if (fact.empty) nearly_empty &= bases[b].offset == 0;
            else nearly_empty &= fact.nearly_empty && ++dynamic_bases == 1;
        }
        extent = std::max(extent,layout_add(bases[b].offset,bytes));
        if (b == class_facts[info].first_base) class_facts[info].base_offset = bases[b].offset;
        if (!class_facts[entities[types[base].entity].class_info].empty) cursor = bases[b].offset*8;
        if (!class_facts[entities[types[base].entity].class_info].empty) {
            if (bytes > std::numeric_limits<std::uint64_t>::max()/8) throw std::runtime_error("base layout overflow");
            cursor = layout_add(cursor, bytes*8); class_facts[info].empty = false;
        }
        align = std::max(align, class_facts[base_info].nonvirtual_alignment);
    }
    for (auto d = scopes[entities[e].scope].first_decl; d; d = declarations[d].next) {
        EntityId id = declarations[d].entity;
        Entity member = entities[id];
        if (member.kind != EntityKind::Variable || member.is_static || member.owner != entities[e].scope) continue;
        auto f = field_fact(id);
        if (!f.bit_field || f.declared_width) nearly_empty = false;
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
            if (!reference && empty.merge(member.type) && !is_union && offset < extent)
                offset = layout_align(extent,field_align);
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
    class_facts[info].nonvirtual_alignment = align;
    auto data_size = std::max<std::uint64_t>(std::max<std::uint64_t>(1,extent),layout_add(cursor,7)/8);
    // Dynamic non-POD bases permit derived members to reuse their tail padding.
    class_facts[info].nonvirtual_size = host_abi && dynamic_class(e) ? data_size : layout_align(data_size,align);
    cursor = class_facts[info].nonvirtual_size;
    class_facts[info].nearly_empty = nearly_empty && class_facts[info].nonvirtual_size == 8;
    if (host_abi && virtual_base_count(e)) layout_host_virtual_bases(e,cursor,align);
    else for (unsigned j = 0; j < class_facts[info].virtual_bases_count; ++j) {
        auto at = class_facts[info].virtual_bases_begin+j;
        auto base = virtual_bases[at]; size(entities[base].type);
        auto base_info = entities[base].class_info;
        auto base_align = class_facts[base_info].nonvirtual_alignment;
        auto offset = layout_align(cursor,base_align);
        virtual_base_offsets[at] = offset;
        cursor = layout_add(offset,class_facts[base_info].nonvirtual_size);
        align = std::max(align,base_align);
    }
    for (auto b = class_facts[info].first_base; b; b = bases[b].next)
        if (bases[b].virtual_base) bases[b].offset = virtual_base_offset(e,bases[b].base);
    if (class_facts[info].first_base) class_facts[info].base_offset = bases[class_facts[info].first_base].offset;
    class_facts[info].alignment = align;
    class_facts[info].size = layout_align(cursor,align);
    empty.publish(info,class_facts[info].empty ? e : 0);
    class_facts[info].layout_state = FactState::Success;
    if (dynamic_class(e)) layout_virtual_views(e);
    layout_lifecycle(e);
    } catch (...) {
        class_facts[info].layout_state = FactState::Failure; throw;
    }
}
} }
