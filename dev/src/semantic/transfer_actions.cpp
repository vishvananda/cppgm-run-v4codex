#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::prepare_transfer(EntityId e)
{
    auto m = entities[e].member_info;
    if (!members[m].synthetic || members[m].transfer == TransferKind::None) return;
    if (members[m].transfer_state == 2) return;
    if (members[m].transfer_state == 1) { members[m].deleted = true; return; }
    members[m].transfer_state = 1;
    bool assignment = members[m].transfer == TransferKind::CopyAssignment || members[m].transfer == TransferKind::MoveAssignment;
    bool moving = members[m].transfer == TransferKind::MoveConstructor || members[m].transfer == TransferKind::MoveAssignment;
    EntityId cls = scopes[entities[e].owner].entity;
    TypeId target = entities[cls].type;
    size(target);
    if (!entities[e].scope) {
        ScopeId scope = make_scope(ScopeKind::Function, entities[e].owner, entities[e].name, e);
        entities[e].scope = scope;
        Type f = types[entities[e].type];
        for (unsigned j = 0; j < f.count; ++j) {
            EntityId p = make_entity(EntityKind::Parameter, scope, 0, 0);
            entities[p].type = types.parameters[f.offset+j];
            record(scope, p, 0, entities[p].type, EntityKind::Parameter);
            if (!j) members[m].transfer_parameter = p;
        }
    }
    Type f = types[entities[e].type];
    TypeId source = value_type(types.parameters[f.offset]);
    bool deleted = members[m].deleted, trivial = !members[m].defaulted_late, no_throw = true;
    bool is_union = entities[cls].key == KW_UNION;
    std::vector<TransferAction> actions;
    std::uint64_t unit_offset = 0, unit_bytes = 0;
    bool prior_unit = false;
    auto add = [&](EntityId field, TypeId type) {
        TransferAction action; action.field = field; action.type = type;
        TypeId element = type;
        while (types[element].kind == TypeKind::Array) element = types[element].child;
        bool ref = types[element].kind == TypeKind::LRef || types[element].kind == TypeKind::RRef;
        if (ref) {
            if (assignment || (!moving && types[element].kind == TypeKind::RRef)) deleted = true;
            action.kind = TransferAction::Reference;
        } else if (types[element].kind == TypeKind::Named && entities[types[element].entity].class_info) {
            TypeId from = types.qualify(element, types[source].cv & (field && entities[field].mutable_field ? 2 : 3));
            EntityId selected = select_transfer(element, from, moving ? ValueCategory::Xvalue : ValueCategory::Lvalue, assignment);
            if (!selected || deleted_transfer(selected) || !transfer_accessible(selected, entities[e].scope)) deleted = true;
            else {
                action.function = selected;
                bool simple = trivial_transfer(selected);
                trivial &= simple;
                no_throw &= function_nonthrowing(selected);
                if (is_union && !simple) deleted = true;
            }
            action.kind = TransferAction::Subobject;
        } else if (assignment && (types[element].cv & 1)) deleted = true;
        if (types[element].cv & 2) trivial = false;
        const FieldFacts layout = field_fact(field);
        if (layout.bit_field) {
            if (!layout.declared_width) { prior_unit = false; return; }
            std::uint64_t bytes = size(layout.storage_type);
            bool safe = !(types[type].cv & 2) && layout.declared_width <= bytes*8 && (bytes == 1 || bytes == 2 || bytes == 4 || bytes == 8);
            if (safe) {
                auto offset = entities[field].member_offset;
                if (prior_unit && unit_offset == offset && unit_bytes == bytes) return;
                action.kind = TransferAction::Unit; action.type = layout.storage_type;
                unit_offset = offset; unit_bytes = bytes; prior_unit = true;
            } else prior_unit = false;
        } else prior_unit = false;
        // Empty base subobjects have no transferable storage, even when the
        // complete-class size is one byte and a derived payload shares its address.
        if (!field && action.function && trivial_transfer(action.function) && class_facts[entities[types[element].entity].class_info].empty) {
            if (copy_storage_type(element)) return;
            action.kind = TransferAction::Empty; action.function = 0;
        }
        actions.push_back(action);
    };
    auto info = entities[cls].class_info;
    for (auto b = class_facts[info].first_base; b; b = bases[b].next) add(0, entities[bases[b].base].type);
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        EntityId field = declarations[d].entity;
        if ((nonstatic_field(field) || field_fact(field).bit_field) && entities[field].owner == entities[cls].scope)
            add(field, entities[field].type);
    }
    if (is_union && !deleted) {
        actions.clear(); TransferAction storage; storage.kind = TransferAction::Storage;
        storage.bytes = size(target); storage.alignment = size(target, true); actions.push_back(storage);
    } else {
        std::size_t prefix = 0; std::uint64_t bytes = 0;
        for (const auto& action : actions) {
            TypeId element = action.type;
            while (types[element].kind == TypeKind::Array) element = types[element].child;
            if (action.kind == TransferAction::Empty || action.kind == TransferAction::Unit || field_fact(action.field).bit_field || (types[element].cv & 2)) break;
            if (action.function && (!trivial_transfer(action.function) || !copy_storage_type(element))) break;
            bool ref = types[element].kind == TypeKind::LRef || types[element].kind == TypeKind::RRef;
            std::uint64_t end = (action.field ? entities[action.field].member_offset : 0) + (ref ? 8 : size(action.type));
            bytes = std::max(bytes, end); ++prefix;
        }
        if (prefix) {
            if (prefix == actions.size()) bytes = size(target);
            TransferAction storage; storage.kind = TransferAction::Storage; storage.bytes = bytes;
            storage.alignment = size(target, true);
            actions.erase(actions.begin(), actions.begin()+prefix); actions.insert(actions.begin(), storage);
        }
    }
    members[m].deleted = deleted;
    members[m].transfer_trivial = trivial && !deleted;
    members[m].transfer_noexcept = no_throw && !deleted;
    members[m].transfer_begin = transfers.size(); members[m].transfer_count = actions.size();
    transfers.insert(transfers.end(), actions.begin(), actions.end());
    members[m].transfer_state = 2;
}
} }
