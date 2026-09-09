#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
void Analyzer::prepare_value_initialization(TypeId t, ScopeId s)
{
    // Access is contextual; the immutable action shape is shared by type.
    if (access_override) s = access_override;
    auto context = key(t, s);
    if (value_contexts.get(context) == 2) {
        if (!base_initialization) {
            auto ctor = value_constructor(t);
            if (ctor) members[entities[ctor].member_info].complete_entry = true;
        }
        return;
    }
    if (value_contexts.get(context) == 1) throw std::runtime_error("recursive value initialization");
    value_contexts.put(context, 1);
    Type type = types[t];
    if (type.kind == TypeKind::LRef || type.kind == TypeKind::RRef)
        throw std::runtime_error("value-initialized reference");
    InitAction action; action.type = t; action.kind = InitKind::Value;
    std::vector<InitAction> children;
    auto add = [&](TypeId child, EntityId field, std::uint64_t count) {
        prepare_value_initialization(child, s);
        InitAction item = initializers[initializer_plan(0, child)];
        item.field = field; item.count = count; children.push_back(item);
    };
    if (type.kind == TypeKind::Array) {
        action.kind = InitKind::Group;
        add(type.child, 0, type.bound);
    } else if (type.kind == TypeKind::Named && entities[type.entity].class_info) {
        auto c = entities[type.entity].class_info;
        if (!class_facts[c].aggregate) {
            EntityId ctor = default_constructor(t, s);
            class_facts[c].value_constructor = ctor;
            if (!base_initialization) members[entities[ctor].member_info].complete_entry = true;
        } else {
            action.kind = InitKind::Group;
            size(t);
            for (auto d = scopes[entities[type.entity].scope].first_decl; d; d = declarations[d].next) {
                EntityId field = declarations[d].entity;
                if (!nonstatic_field(field)) continue;
                add(initialized_field_type(t, field), field, 1);
                if (entities[type.entity].key == KW_UNION) break;
            }
        }
    }
    if (!initializer_plan(0, t)) {
        std::uint32_t tail = 0;
        for (auto item : children) {
            auto id = initializers.size();
            item.next = 0; initializers.push_back(item);
            if (tail) initializers[tail].next = id; else action.first = id;
            tail = id;
        }
        initializer_index.put(key(0, t), initializers.size());
        initializers.push_back(action);
    }
    value_contexts.put(context, 2);
}
} }
