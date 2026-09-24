#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::template_signature_shape(ArgumentId argument)
{
    if (!argument) return 0;
    if (auto known = template_signature_shapes.get(argument)) { ++template_signature_shape_hits; return known; }
    ++template_signature_shape_work;
    std::vector<ArgumentId> shape;
    auto add = [&](ArgumentId arg) { shape.push_back(template_signature_shape(arg)); };
    if (value_argument(argument)) {
        auto q = type_queries[argument_query(argument)];
        // [temp.over.link]: equivalent expressions refer to the same bound
        // names/parameters and operations. Access/evaluation context remains
        // on the semantic query, not on this separate declaration-shape key.
        // In-class and out-of-class declarations have different lexical
        // contexts even when both expressions denote the same signature.
        shape = {1,unsigned(q.kind),unsigned(q.op),q.entity,q.name,q.naming,
            unsigned(q.value),unsigned(q.value>>32),unsigned(q.null_pointer_constant),unsigned(q.arguments!=0)};
        add(q.type);
        auto args = argument_packs[q.arguments];
        shape.push_back(args.count);
        for (unsigned j = 0; j < args.count; ++j) add(argument_types[args.offset+j]);
        shape.push_back(q.count);
        for (unsigned j = 0; j < q.count; ++j) add(0x80000000U|query_edges[q.offset+j]);
    } else {
        auto t = types[argument];
        shape = {2,unsigned(t.kind),t.cv,unsigned(t.ref),unsigned(t.fundamental),unsigned(t.variadic)};
        add(t.child);
        if (t.kind == TypeKind::Named && entities[t.entity].specialization) {
            auto spec = specializations[entities[t.entity].specialization];
            shape.push_back(spec.pattern);
            auto args = argument_packs[spec.arguments];
            for (unsigned j = 0; j < args.count; ++j) add(argument_types[args.offset+j]);
        } else if (t.kind == TypeKind::Decltype) {
            add(0x80000000U|t.entity); shape.push_back(t.bound);
        } else if (t.kind == TypeKind::DependentArray) add(0x80000000U|t.bound);
        else if (t.kind == TypeKind::PackExpansion) add(t.bound);
        else if (t.kind == TypeKind::ArgumentPack) {
            auto args = argument_packs[t.bound];
            for (unsigned j = 0; j < args.count; ++j) add(argument_types[args.offset+j]);
        } else {
            shape.push_back(t.entity); shape.push_back(t.bound); shape.push_back(t.bound>>32);
            for (unsigned j = 0; j < t.count; ++j) add(types.parameters[t.offset+j]);
        }
    }
    auto result = intern_arguments(shape); template_signature_shapes.put(argument,result); return result;
}
} }
