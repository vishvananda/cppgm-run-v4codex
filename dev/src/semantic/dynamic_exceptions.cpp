#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::evaluate_dynamic_exceptions(EntityId e, std::uint32_t id)
{
    const auto fact = exception_specifications[id];
    std::uint32_t frame = 0;
    if (fact.pattern) {
        const auto head = templates[entities[fact.pattern].template_info];
        frame = head.parent_frame;
        auto specialization = entities[e].specialization;
        if (head.source_count) frame = substitution_frame(specialization,head.source_parameters,head.source_count,frame);
        frame = substitution_frame(specialization,head.offset,head.count,frame);
    }
    Index bindings, cache;
    std::vector<TypeId> allowed;
    for (auto n = ast[fact.dynamic_types].first; n; n = ast[n].next) {
        auto type = type_id(n,fact.scope);
        if (frame) type = substitute_type(type,bindings,cache,frame);
        if (!type || dependent_type(type)) throw std::runtime_error("unresolved exception specification type");
        if (types[type].kind == TypeKind::RRef) throw std::runtime_error("rvalue reference exception specification");
        type = types.unqualified(decay(value_type(type)));
        if (fundamental(type,FT_VOID)) throw std::runtime_error("void exception specification");
        auto complete = types[type].kind == TypeKind::Pointer ? types[type].child : type;
        if (class_value(complete)) {
            auto cls = types[complete].entity;
            if (!entities[cls].complete) complete_class(cls);
            if (!entities[cls].complete && !encloses(entities[cls].scope,fact.scope))
                throw std::runtime_error("incomplete exception specification type");
        }
        record_rtti_type(type); allowed.push_back(type);
    }
    // A specification is a set. Canonical TypeIds provide stable comparison
    // without rendered keys; sorting costs O(k log k) in its actual types.
    std::sort(allowed.begin(),allowed.end());
    allowed.erase(std::unique(allowed.begin(),allowed.end()),allowed.end());
    exception_specifications[id].allowed_begin = allowed_exception_types.size();
    exception_specifications[id].allowed_count = allowed.size();
    allowed_exception_types.insert(allowed_exception_types.end(),allowed.begin(),allowed.end());
}
} }
