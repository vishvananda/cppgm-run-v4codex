#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::prepare_zero_initialization(TypeId t)
{
    if (auto known = zero_initialization_index.get(t)) return known;
    auto type = types[t];
    ZeroInitialization plan; plan.type = t; plan.bulk = !(type.cv & 2);
    std::vector<ZeroPart> parts;
    if (type.kind == TypeKind::LRef || type.kind == TypeKind::RRef) {
        plan.kind = ZeroInitialization::Reference; plan.bytes = plan.alignment = 8; plan.bulk = false;
    } else {
        plan.bytes = size(t); plan.alignment = size(t,true);
        if (type.kind == TypeKind::MemberPointer) {
            plan.kind = ZeroInitialization::MemberPointer;
            plan.bulk &= types[type.child].kind == TypeKind::Function;
        } else if (type.kind == TypeKind::Array) {
            plan.kind = ZeroInitialization::Array;
            plan.child = prepare_zero_initialization(type.child); plan.elements = type.bound;
            auto child = zero_initializations[plan.child];
            plan.bulk &= child.bulk;
            if (types[type.child].kind == TypeKind::Array) {
                plan.elements *= child.elements; plan.child = child.child;
            }
            if (plan.bulk) plan.kind = ZeroInitialization::Representation;
        } else if (class_value(t)) {
            plan.kind = ZeroInitialization::Composite;
            bool is_union = entities[type.entity].key == KW_UNION;
            plan.bulk &= !is_union;
            auto add = [&](TypeId child, std::uint64_t offset) {
                auto id = prepare_zero_initialization(child);
                plan.bulk &= zero_initializations[id].bulk;
                parts.push_back({id,offset});
            };
            auto info = entities[type.entity].class_info;
            for (auto b = class_facts[info].first_base; b; b = bases[b].next) add(entities[bases[b].base].type,0);
            std::uint64_t unit_offset = 0, unit_bytes = 0; bool unit = false;
            for (auto d = scopes[entities[type.entity].scope].first_decl; d; d = declarations[d].next) {
                EntityId field = declarations[d].entity;
                if (!nonstatic_field(field) || entities[field].owner != entities[type.entity].scope) continue;
                TypeId child = entities[field].type; auto offset = entities[field].member_offset;
                auto bits = field_fact(field);
                if (bits.bit_field) {
                    plan.bulk &= !(types[child].cv & 2);
                    if (!bits.declared_width) { unit = false; continue; }
                    child = types.qualify(bits.storage_type,types[child].cv);
                    if (unit && unit_offset == offset && unit_bytes == size(child)) continue;
                    unit_offset = offset; unit_bytes = size(child); unit = true;
                } else unit = false;
                add(child,offset);
                if (is_union) break;
            }
            if (plan.bulk) { plan.kind = ZeroInitialization::Representation; parts.clear(); }
        }
    }
    plan.first = zero_parts.size(); plan.count = parts.size();
    zero_parts.insert(zero_parts.end(),parts.begin(),parts.end());
    auto id = zero_initializations.size(); zero_initializations.push_back(plan);
    zero_initialization_index.put(t,id); return id;
}
} }
