#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::check_fixed_conversion(Expression source, NodeId n, Conversion& c, ScopeId s)
{
    if (!c.valid()) throw std::runtime_error("invalid fixed call argument");
    if (c.kind == Conversion::Kind::ListPlan) { validate_list_plan(c.materialization); return; }
    if (c.kind == Conversion::Kind::Construction) {
        if (deleted_transfer(c.function)) throw std::runtime_error("deleted fixed converting constructor");
        check_access(c.function,s,entities[c.function].owner);
        default_destructor(value_type(c.target),s,false);
        auto f = types[entities[c.function].type];
        std::vector<NodeId> args; std::vector<Conversion> chosen;
        for (unsigned i = 0; i < f.count; ++i) {
            Conversion argument;
            auto a = i ? default_argument(c.function,i,&argument) : n;
            if (!i) {
                auto target = types.parameters[f.offset+i];
                argument = c.implicit_move ? transfer_conversion(source.type,ValueCategory::Xvalue,target) :
                    a ? conversion(a,target,false) : standard_conversion(source,target);
                check_fixed_conversion(source,a,argument,s);
            }
            args.push_back(a); chosen.push_back(argument);
        }
        if (!f.count && f.variadic) {
            args.push_back(n); chosen.push_back(ellipsis_conversion_value(source));
        }
        // A zero temporary identifies an immutable constructor recipe. Concrete
        // argument application adds an object and consumes these conversions.
        ConversionObject recipe; recipe.constructor = c.function;
        store_call(recipe.call,args,chosen);
        c.materialization = conversion_objects.size(); conversion_objects.push_back(recipe);
        return;
    }
    if (c.kind == Conversion::Kind::User) {
        if (deleted_transfer(c.function)) throw std::runtime_error("deleted fixed conversion function");
        auto record = user_conversions[c.materialization];
        auto from = record.object_entity ? value_type(entities[record.object_entity].type) : source.type;
        check_access(c.function,s,entities[types[from].entity].scope,from);
        if (record.adjustment) check_base_access(from,entities[scopes[entities[c.function].owner].entity].type,s);
        auto returned = types[entities[c.function].type].child;
        if (class_value(returned)) default_destructor(returned,s,false);
        Expression value; value.type = value_type(returned);
        value.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        check_fixed_conversion(value,0,record.result,s);
        user_conversions[c.materialization].result = record.result;
        return;
    }
    if (c.derived && c.kind != Conversion::Kind::Explicit) {
        auto from = source.type, to = types[c.target].child;
        if (pointer(from)) from = types[from].child;
        if (pointer(to)) to = types[to].child;
        check_base_access(from,to,s);
    }
    if (c.function) {
        if (deleted_transfer(c.function)) throw std::runtime_error("deleted fixed conversion target");
        check_access(c.function,s,entities[c.function].owner);
    }
}
Conversion Analyzer::copy_conversion_recipe(Conversion c)
{
    if (c.kind == Conversion::Kind::User) {
        auto recipe = user_conversions[c.materialization];
        if (recipe.prepared) throw std::logic_error("fixed call owns a concrete user conversion");
        c.materialization = user_conversions.size(); user_conversions.push_back(recipe);
    }
    return c;
}
} }
