#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::check_fixed_conversion(Expression source, NodeId n, Conversion& c, ScopeId s)
{
    if (!valid_fixed_conversion(source,n,c,s)) throw std::runtime_error("invalid fixed call argument");
}
bool Analyzer::valid_fixed_conversion(Expression source, NodeId n, Conversion& c, ScopeId s)
{
    if (!c.valid()) return false;
    if (c.kind == Conversion::Kind::Discarded) {
        if (n && discarded_conversions.get(n)) return true;
        Conversion selected;
        if (!valid_discarded(source,n,s,selected)) return false;
        if (n && selected.valid()) {
            discarded_conversions.put(n,conversions.size()); conversions.push_back(selected);
        } else if (selected.valid()) {
            // A query has no source NodeId. Keep its checked copy recipe on
            // the discarded conversion for exception-effect consumers.
            c.materialization = conversions.size(); conversions.push_back(selected);
        }
        return true;
    }
    if (c.kind == Conversion::Kind::QueryList) return valid_query_list(c.materialization);
    if (c.kind == Conversion::Kind::ListPlan) { validate_list_plan(c.materialization); return true; }
    if (c.kind == Conversion::Kind::Construction) {
        if (deleted_transfer(c.function) || !accessible(c.function,s,entities[c.function].owner) ||
            !default_destruction_valid(value_type(c.target),s)) return false;
        auto f = types[entities[c.function].type];
        std::vector<NodeId> args; std::vector<Conversion> chosen;
        for (unsigned i = 0; i < f.count; ++i) {
            Conversion argument;
            auto a = i ? default_argument(c.function,i,&argument,DefaultReason::Recipe) : n;
            if (!i) {
                auto target = types.parameters[f.offset+i];
                argument = c.implicit_move ? transfer_conversion(source.type,ValueCategory::Xvalue,target) :
                    a ? conversion(a,target,false) : standard_conversion(source,target);
                if (!valid_fixed_conversion(source,a,argument,s)) return false;
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
        return true;
    }
    if (c.kind == Conversion::Kind::User) {
        if (deleted_transfer(c.function)) return false;
        auto record = user_conversions[c.materialization];
        auto from = record.object_entity ? value_type(entities[record.object_entity].type) : source.type;
        if (!accessible(c.function,s,entities[types[from].entity].scope,from)) return false;
        auto returned = types[entities[c.function].type].child;
        if (class_value(returned) && !default_destruction_valid(returned,s)) return false;
        Expression value; value.type = value_type(returned);
        value.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        if (!valid_fixed_conversion(value,0,record.result,s)) return false;
        user_conversions[c.materialization].result = record.result;
        return true;
    }
    if (c.derived && c.kind != Conversion::Kind::Explicit) {
        auto from = source.type, to = types[c.target].child;
        if (pointer(from)) from = types[from].child;
        if (pointer(to)) to = types[to].child;
        if (!base_accessible(types[from].entity,types[to].entity,s)) return false;
    }
    if (c.function) {
        if (deleted_transfer(c.function) || !accessible(c.function,s,object_uses[source.object_use].naming_scope)) return false;
    }
    return true;
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
