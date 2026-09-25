#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::template_aggregate_type(TypeId target) const
{
    return aggregate_type(target) || (types[target].kind == TypeKind::Named &&
        template_pattern_aggregates.get(types[target].entity) == 2);
}
bool Analyzer::fixed_initializer_operands(NodeId n) const
{
    if (!n) return true;
    auto kind = ast[n].kind;
    if (kind == Kind::Initializer || kind == Kind::BracedInit || kind == Kind::ParenInitializer ||
        kind == Kind::ParenArguments || kind == Kind::Arguments) {
        for (auto c = ast[n].first; c; c = ast[c].next)
            if (!fixed_initializer_operands(c)) return false;
        return true;
    }
    return expressions[n].ready;
}
void Analyzer::remember_initialization(NodeId n, Conversion c)
{
    if (!n || ast.nodes.occurrences[n].context) return;
    auto source = ast.nodes.occurrences[n].source;
    if (auto old = template_initialization_conversions.get(source)) {
        if (conversions[old].target != c.target) throw std::logic_error("source initializer target changed");
        return;
    }
    template_initialization_conversions.put(source,conversions.size());
    conversions.push_back(c); ++initializer_recipe_work;
}
std::uint32_t Analyzer::retained_initialization(NodeId n, TypeId target)
{
    auto occurrence = ast.nodes.occurrences[n];
    auto id = occurrence.context ? template_initialization_conversions.get(occurrence.source) : 0;
    if (!id || conversions[id].target != target) return 0;
    ++initializer_recipe_uses; return id;
}
bool Analyzer::check_template_constructor(NodeId n, TypeId target, ScopeId s, InitializationMode mode,
    const std::vector<NodeId>* operands)
{
    auto list = ast[n].kind == Kind::Initializer ? ast[n].first : n;
    bool copy = mode == InitializationMode::Copy;
    bool grouped = operands || ast[list].kind == Kind::Arguments || ast[list].kind == Kind::ParenInitializer ||
        ast[list].kind == Kind::ParenArguments || ast[list].kind == Kind::BracedInit;
    std::vector<NodeId> args;
    std::vector<Expression> values;
    std::vector<NodeId> source_operands;
    if (operands) source_operands = *operands;
    else for (auto a = grouped ? ast[list].first : list; a; a = grouped ? ast[a].next : 0) source_operands.push_back(a);
    for (auto a : source_operands) {
        if (ast[a].kind == Kind::BracedInit && fixed_initializer_operands(a)) expression(a,s);
        auto value = template_statement_value(a,s);
        if (!value.type && value.form != ExpressionForm::InitializerList && value.form != ExpressionForm::Overload) return false;
        if (!grouped && copy) {
            auto c = expressions[a].ready ? conversion(a,target) : conversion_value(value,target);
            check_fixed_conversion(value,expressions[a].ready ? a : 0,c,s);
            remember_initialization(a,c); return true;
        }
        if (!grouped && value.category == ValueCategory::Prvalue &&
            types.unqualified(value.type) == types.unqualified(target)) {
            auto c = transfer_initialization(value,target,mode);
            check_fixed_conversion(value,expressions[a].ready ? a : 0,c,s);
            remember_initialization(a,c); return true;
        }
        args.push_back(a); values.push_back(value);
    }
    Expression result;
    auto ctor = choose_constructor(target,args,&result,s,!copy || ast[list].kind == Kind::BracedInit,true,&values);
    if (!ctor || deleted_transfer(ctor)) throw std::runtime_error("invalid fixed initializer constructor");
    check_default_constructor(ctor);
    auto access = ctor;
    while (members[entities[access].member_info].inherited_constructor)
        access = members[entities[access].member_info].inherited_constructor;
    check_access(access,s,entities[access].owner);
    if (copy && members[entities[ctor].member_info].explicit_constructor)
        throw std::runtime_error("explicit constructor in fixed copy initialization");
    default_destructor(target,s,false);
    auto f = types[entities[ctor].type];
    std::vector<Conversion> selected;
    auto supplied = args.size();
    for (unsigned i = 0; i < supplied; ++i) {
        auto c = conversions[result.conversions+i];
        check_fixed_conversion(values[i],expressions[args[i]].ready ? args[i] : 0,c,s);
        if (ast[list].kind == Kind::BracedInit && i < f.count)
            list_conversion_from(args[i],values[i].type,value_type(types.parameters[f.offset+i]),&c);
        selected.push_back(c);
    }
    for (unsigned i = supplied; i < f.count; ++i) {
        Conversion c; args.push_back(default_argument(ctor,i,&c,DefaultReason::Recipe)); selected.push_back(c);
    }
    result.type = target; result.ready = true; result.inputs = CallInputs::Source;
    store_call(result,args,selected); result.inputs = CallInputs::Source;
    expressions.set(n,result);
    auto& fact = facts.edit(n); fact.entity = ctor; fact.type = target; fact.scope = s;
    template_initializer_calls.put(ast.nodes.occurrences[n].source,n); ++initializer_recipe_work;
    return true;
}
bool Analyzer::reuse_template_constructor(NodeId n, TypeId target, const std::vector<NodeId>& args,
    Expression& result, ScopeId s, EntityId& selected)
{
    auto occurrence = ast.nodes.occurrences[n];
    auto source = occurrence.context ? template_initializer_calls.get(occurrence.source) : 0;
    if (!source || facts[source].type != target) return false;
    ++initializer_recipe_uses;
    auto recipe = expressions[source]; selected = facts[source].entity;
    std::vector<NodeId> operands; std::vector<Conversion> chosen;
    for (unsigned i = 0; i < recipe.argument_count; ++i) {
        auto a = i < args.size() ? args[i] : default_argument(selected,i);
        expression(a,s); operands.push_back(a);
        chosen.push_back(copy_conversion_recipe(conversions[recipe.conversions+i]));
    }
    record_call(result,operands,chosen);
    if (!converting_transfer(selected,result)) demand_member(selected);
    return true;
}
bool Analyzer::check_template_initializer_item(NodeId& cursor, TypeId target, ScopeId s, std::uint64_t* bound)
{
    // A dependent field can consume an unknown number of brace-elided clauses.
    // An explicit braced clause delimits one field even when its type is unknown.
    if (dependent_type(target) && types[target].kind != TypeKind::Array &&
        !(types[target].kind == TypeKind::Named && template_pattern_aggregates.get(types[target].entity) == 2)) {
        if (!cursor) return true;
        if (ast[cursor].kind != Kind::BracedInit) return false;
        cursor = ast[cursor].next; return true;
    }
    if (!cursor) {
        auto c = list_initialization(0,target,s); check_fixed_conversion(Expression(),0,c,s); return true;
    }
    auto source = cursor;
    bool grouped = ast[source].kind == Kind::BracedInit || ast[source].kind == Kind::ParenArguments ||
        ast[source].kind == Kind::ParenInitializer;
    auto inner = grouped ? ast[source].first : source;
    if (types[target].kind == TypeKind::Array)
        while (ast[inner].kind == Kind::Parenthesized) inner = ast[inner].first;
    if (string_initialization(inner,target)) {
        if (grouped && ast[ast[source].first].next) throw std::runtime_error("excess fixed string initializer");
        if (types[target].bound && ast.literals[ast[inner].literal].elements > types[target].bound)
            throw std::runtime_error("fixed string initializer exceeds array");
        if (bound) *bound = ast.literals[ast[inner].literal].elements;
        cursor = ast[source].next; return true;
    }
    if (!template_aggregate_type(target)) {
        check_template_initialization(source,target,s,InitializationMode::Copy);
        if (!grouped) {
            auto value = template_statement_value(source,s);
            if (value.type) {
                list_conversion_from(source,value.type,target);
                template_initializer_narrowing.put(ast.nodes.occurrences[source].source,target);
            }
        }
        cursor = ast[source].next; return true;
    }
    if (class_value(target) && !grouped) {
        auto value = template_statement_value(source,s);
        if (value.type) {
            auto c = conversion_value(value,target);
            if (c.valid()) {
                check_fixed_conversion(value,expressions[source].ready ? source : 0,c,s);
                remember_initialization(source,c); cursor = ast[source].next; return true;
            }
        }
    }
    auto type = types[target];
    if (type.kind == TypeKind::Array) {
        std::uint64_t i = 0;
        bool complete = true;
        while (inner && (!type.bound || i < type.bound)) {
            if (ast[inner].kind == Kind::PackExpression) {
                // Expansion length is unknown, but scalar element conversions
                // and following fixed clauses still have definition-time rules.
                if (dependent_type(type.child) || template_aggregate_type(type.child)) return false;
                auto pattern = ast[inner].first;
                check_template_initializer_item(pattern,type.child,s);
                inner = ast[inner].next; complete = false; continue;
            }
            auto clause = inner;
            if (!check_template_initializer_item(inner,type.child,s)) return false;
            if (inner == clause) throw std::runtime_error("array element consumed no fixed initializer");
            ++i;
        }
        if (!complete) return false;
        if (bound) *bound = i;
        if (i < type.bound) { NodeId omitted = 0; if (!check_template_initializer_item(omitted,type.child,s)) return false; }
    } else {
        if (!template_pattern_aggregates.get(type.entity)) size(target);
        for (auto d = scopes[entities[type.entity].scope].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (!nonstatic_field(field)) continue;
            if (!check_template_initializer_item(inner,initialized_field_type(target,field),s)) return false;
            if (entities[type.entity].key == KW_UNION) break;
        }
    }
    if (grouped && inner) throw std::runtime_error("excess fixed aggregate initializer");
    cursor = grouped ? ast[source].next : inner;
    return true;
}
void Analyzer::check_template_initialization(NodeId n, TypeId target, ScopeId s, InitializationMode mode)
{
    if (!target || ast.nodes.occurrences[n].context) return;
    check_array_initializer(n,target);
    if (ast[n].kind == Kind::Initializer && (ast[n].flags & 1)) mode = InitializationMode::Copy;
    if (types[target].kind == TypeKind::Named && template_pattern_aggregates.get(types[target].entity) == 3)
        throw std::runtime_error("initializer needs complete local class");
    if (dependent_type(target) && types[target].kind != TypeKind::Array &&
        !(types[target].kind == TypeKind::Named && template_pattern_aggregates.get(types[target].entity) == 2)) return;
    struct Unevaluated { unsigned& depth; Unevaluated(unsigned& d) : depth(d) { ++depth; } ~Unevaluated() { --depth; } } guard(unevaluated_depth);
    auto list = ast[n].kind == Kind::Initializer ? ast[n].first : n;
    if (class_value(target)) {
        complete_class(types[target].entity); reject_abstract(target);
        if (!aggregate_type(target) || ast[list].kind != Kind::BracedInit) {
            check_template_constructor(n,target,s,mode); return;
        }
    }
    if (ast[n].kind == Kind::Initializer) { check_template_initialization(list,target,s,mode); return; }
    if (template_aggregate_type(target)) {
        NodeId cursor = n; check_template_initializer_item(cursor,target,s); return;
    }
    if (ast[n].kind == Kind::BracedInit &&
        (types[target].kind == TypeKind::LRef || types[target].kind == TypeKind::RRef)) {
        if (!fixed_initializer_operands(n)) return;
        auto c = list_initialization(n,target,s);
        check_fixed_conversion(Expression(),n,c,s); remember_initialization(n,c); return;
    }
    bool braced = ast[n].kind == Kind::BracedInit;
    if (braced || ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments) {
        auto child = ast[n].first;
        if (!child) {
            auto c = list_initialization(0,target,s); check_fixed_conversion(Expression(),0,c,s); return;
        }
        if (child != ast[n].last) throw std::runtime_error("excess fixed scalar initializer");
        if (braced || ast[child].kind == Kind::BracedInit) {
            check_template_initialization(child,target,s,mode);
            if (braced) {
                auto value = template_statement_value(child,s);
                if (value.type) {
                    list_conversion_from(child,value.type,target);
                    template_initializer_narrowing.put(ast.nodes.occurrences[child].source,target);
                }
            }
            return;
        }
        auto value = template_statement_value(child,s);
        if (!value.type && value.form != ExpressionForm::Overload) return;
        auto c = class_value(value.type) ? conversion_function_value(value,target,true) : conversion_value(value,target);
        check_fixed_conversion(value,expressions[child].ready ? child : 0,c,s);
        remember_initialization(child,c); return;
    }
    auto value = template_statement_value(n,s);
    if (!value.type && value.form != ExpressionForm::Overload) return;
    auto c = expressions[n].ready ? conversion(n,target) : conversion_value(value,target);
    check_fixed_conversion(value,expressions[n].ready ? n : 0,c,s); remember_initialization(n,c);
}
} }
