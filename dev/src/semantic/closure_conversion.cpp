#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::prepare_closure_conversion(unsigned id)
{
    auto closure = closures[id];
    auto cls = closure.entity, fn = closure.function;
    auto call = types[entities[fn].type];
    auto function = types.function(call.child,std::vector<TypeId>(types.parameters.begin()+call.offset,
        types.parameters.begin()+call.offset+call.count),call.variadic);
    auto pointer = types.compound(TypeKind::Pointer,function);
    auto scope = entities[cls].scope;
    auto conversion = make_entity(EntityKind::Function,scope,ids.intern(TextView("__closure_conversion",20)),closure.source);
    entities[conversion].type = types.function(pointer,{},false,1);
    entities[conversion].inline_function = true; entities[conversion].exception_spec = 129;
    member_facts(conversion);
    auto cm = entities[conversion].member_info;
    members[cm].conversion_target = pointer;
    members[cm].synthetic = members[cm].in_class_body = true;
    class_facts[entities[cls].class_info].first_conversion = conversion;
    auto thunk = make_entity(EntityKind::Function,scope,ids.intern(TextView("_FUN",4)),closure.source);
    entities[thunk].type = function;
    entities[thunk].is_static = entities[thunk].inline_function = true;
    entities[thunk].exception_spec = entities[fn].exception_spec;
    member_facts(thunk);
    members[entities[thunk].member_info].synthetic = members[entities[thunk].member_info].in_class_body = true;
    closures[id].conversion = conversion; closures[id].thunk = thunk;
    if (entities[fn].template_info) {
        auto head = templates[entities[fn].template_info];
        template_facts(conversion,head.environment); template_facts(thunk,head.environment);
        templates[entities[conversion].template_info].parent_frame = head.parent_frame;
        templates[entities[thunk].template_info].parent_frame = head.parent_frame;
        closure_conversion_templates.put(conversion,id);
    } else {
        ClosureAdapter entry; entry.function = fn; entry.conversion = conversion; entry.thunk = thunk;
        closure_adapters.put(conversion,closure_adapter_facts.size());
        closure_adapters.put(thunk,closure_adapter_facts.size()); closure_adapter_facts.push_back(entry);
        conversion_bindings.put(key(scope,pointer),conversion);
    }
}
EntityId Analyzer::deduce_closure_conversion(EntityId pattern, TypeId target)
{
    if (!pointer(target) || types[types[target].child].kind != TypeKind::Function) return 0;
    auto closure = closures[closure_conversion_templates.get(pattern)];
    auto desired = types[types[target].child];
    auto call = types[entities[closure.function].type];
    auto type = types.function(desired.child,std::vector<TypeId>(types.parameters.begin()+desired.offset,
        types.parameters.begin()+desired.offset+desired.count),desired.variadic,call.cv,call.ref);
    auto fn = deduce_target(closure.function,type);
    if (!fn) return 0;
    auto identity = key(pattern,fn);
    if (auto known = closure_conversion_instances.get(identity)) return known;
    auto pack = specialization_arguments(fn);
    std::vector<ArgumentId> args(argument_types.begin()+pack.offset,argument_types.begin()+pack.offset+pack.count);
    auto conversion = specialize(pattern,args), thunk = specialize(closure.thunk,args);
    if (!conversion || !thunk) return 0;
    entities[conversion].type = types.function(target,{},false,1);
    entities[conversion].exception_spec = 129;
    member_facts(conversion);
    members[entities[conversion].member_info].conversion_target = target;
    members[entities[conversion].member_info].synthetic = members[entities[conversion].member_info].in_class_body = true;
    entities[thunk].type = types[target].child;
    demand_exception_specification(fn);
    entities[thunk].exception_spec = entities[fn].exception_spec;
    member_facts(thunk);
    members[entities[thunk].member_info].synthetic = members[entities[thunk].member_info].in_class_body = true;
    specializations[entities[conversion].specialization].body = FactState::Success;
    specializations[entities[thunk].specialization].body = FactState::Success;
    ClosureAdapter entry; entry.function = fn; entry.conversion = conversion; entry.thunk = thunk;
    closure_adapters.put(conversion,closure_adapter_facts.size());
    closure_adapters.put(thunk,closure_adapter_facts.size()); closure_adapter_facts.push_back(entry);
    closure_conversion_instances.put(identity,conversion); return conversion;
}
} }
