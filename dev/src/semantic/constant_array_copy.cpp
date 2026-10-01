#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
Constant Analyzer::constant_array_copy(TypeId target, std::uint32_t source, std::uint32_t destination,
    const Conversion& c, ScopeId s)
{
    if (!source || !constant_step()) return Constant();
    auto t = types[target];
    if (t.kind == TypeKind::Array) {
        std::vector<EvaluatedPart> parts;
        auto from = types[constant_addresses[source].type].child;
        for (std::uint64_t i = 0; i < t.bound; ++i) {
            auto src = constant_subobject(source,from,i);
            auto dst = destination ? constant_subobject(destination,t.child,i) : 0;
            auto value = constant_array_copy(t.child,src,dst,c,s);
            if (!value.valid) return Constant();
            EvaluatedPart part; part.selector = i; part.value = value; parts.push_back(part);
        }
        return evaluated_object(target,parts);
    }
    if (c.kind == Conversion::Kind::Construction) {
        auto recipe = conversion_objects[c.materialization];
        auto call = recipe.call;
        auto from = constant_addresses[source].type;
        auto first = conversions[call.conversions];
        auto argument = constant_result_conversion(Constant(types.compound(TypeKind::LRef,from),source),first);
        if (!argument.valid) return Constant();
        std::vector<Constant> args{argument};
        for (unsigned i = 1; i < call.argument_count; ++i) {
            auto value = constant_node_conversion(call_argument(call,i),conversions[call.conversions+i],s);
            if (!value.valid) return Constant();
            args.push_back(value);
        }
        auto saved = constant_destination; constant_destination = destination;
        Constant result;
        try { result = constant_construct(recipe.constructor,args,false); }
        catch (...) { constant_destination = saved; throw; }
        constant_destination = saved; return result;
    }
    return constant_result_conversion(constant_read(source),c);
}
} }
