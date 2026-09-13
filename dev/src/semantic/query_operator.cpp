#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeQueryFact Analyzer::query_operator(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    std::vector<Expression> args; std::vector<TypeId> argument_types;
    bool named = false, class_operand = false;
    for (auto c : children) {
        args.push_back(c.expression); argument_types.push_back(c.expression.type);
        named |= types[c.expression.type].kind == TypeKind::Named;
        class_operand |= class_value(c.expression.type);
    }
    auto object = args[0].type;
    ScopeId naming = 0; EntityId family = q.entity;
    if (class_value(object)) {
        complete_class(types[object].entity); naming = entities[types[object].entity].scope;
        family = merge_lookup(family,lookup(naming,q.name,Lookup::Ordinary,true));
    }
    if (named && q.op != OP_LSQUARE && q.op != OP_ASS && q.op != OP_ARROW)
        family = merge_lookup(family,associated_type_lookup(q.name,std::move(argument_types)));
    struct Candidate { EntityId entity; unsigned offset, builtin; };
    std::vector<Candidate> viable; std::vector<Conversion> sequences;
    for (auto e : candidates(family)) {
        ++candidate_work;
        if (entities[e].template_info) e = deduce_function(e,args,scopes[entities[e].owner].kind == ScopeKind::Class && !entities[e].is_static);
        if (!e) continue;
        bool member = entities[e].member_info && !entities[e].is_static;
        auto f = types[entities[e].type];
        if (args.size() != f.count+member) continue;
        if (!class_operand && !member) {
            bool exact_enum = false;
            for (unsigned i = 0; i < f.count; ++i) {
                auto actual = types.unqualified(args[i].type);
                exact_enum |= types[actual].kind == TypeKind::Named && entities[types[actual].entity].key == KW_ENUM && actual == types.unqualified(value_type(types.parameters[f.offset+i]));
            }
            if (!exact_enum) continue;
        }
        unsigned begin = sequences.size(); bool valid = true;
        for (unsigned i = 0; valid && i < args.size(); ++i) {
            auto c = member && !i ? object_conversion(e,object,args[0].category,naming) :
                conversion_value(args[i],types.parameters[f.offset+i-member]);
            valid = c.valid(); sequences.push_back(c);
        }
        if (valid) viable.push_back({e,begin,0}); else sequences.resize(begin);
    }
    std::vector<BuiltinOperator> builtins; builtin_operators_values(q.op,args,builtins);
    for (unsigned i = 0; i < builtins.size(); ++i) {
        viable.push_back({0,unsigned(sequences.size()),i+1});
        for (unsigned j = 0; j < args.size(); ++j) sequences.push_back(builtins[i].arguments[j]);
    }
    TypeQueryFact r;
    if (viable.empty()) {
        if (q.op == OP_COMMA) { r.expression = args[1]; return r; }
        if (q.op == OP_AMP && args.size() == 1 && args[0].category != ValueCategory::Prvalue && !field_fact(args[0].entity).bit_field) {
            r.expression.type = types.compound(TypeKind::Pointer,args[0].type); return r;
        }
        bool equality = q.op == OP_EQ || q.op == OP_NE;
        bool compare = equality || q.op == OP_LT || q.op == OP_GT || q.op == OP_LE || q.op == OP_GE;
        if (args.size() == 2 && compare && types.unqualified(args[0].type) == types.unqualified(args[1].type) &&
            (scoped_enum(args[0].type) || (equality && fundamental(args[0].type,FT_NULLPTR_T)))) {
            r.expression.type = types.fundamental(FT_BOOL); return r;
        }
        throw std::runtime_error("invalid type-query operator operands");
    }
    auto preferred = [&](unsigned a, unsigned b) {
        auto x = sequences.data()+viable[a].offset, y = sequences.data()+viable[b].offset;
        if (better(x,y,args.size())) return true;
        if (better(y,x,args.size())) return false;
        auto ea = viable[a].entity, eb = viable[b].entity;
        return ea && eb && ((!entities[ea].specialization && entities[eb].specialization) || template_more_specialized(ea,eb));
    };
    unsigned best = 0;
    for (unsigned i = 1; i < viable.size(); ++i) if (preferred(i,best)) best = i;
    for (unsigned i = 0; i < viable.size(); ++i) if (i != best && !preferred(best,i)) throw std::runtime_error("ambiguous type-query operator");
    auto selected = viable[best];
    if (selected.builtin) {
        if (args.size() == 2) check_pointer_arithmetic(q.op,sequences[selected.offset].target,sequences[selected.offset+1].target);
        r.expression.type = builtins[selected.builtin-1].type;
        r.expression.category = builtins[selected.builtin-1].category;
    } else {
        if (deleted_transfer(selected.entity)) throw std::runtime_error("deleted type-query operator");
        check_access(selected.entity,q.context,naming,object);
        auto returned = types[entities[selected.entity].type].child;
        r.expression.type = value_type(returned);
        r.expression.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    }
    r.selected = selected.entity;
    r.expression.conversions = conversions.size(); r.expression.count = args.size();
    conversions.insert(conversions.end(),sequences.begin()+selected.offset,sequences.begin()+selected.offset+args.size());
    return r;
}
Expression Analyzer::conditional_value(Expression b, Expression c)
{
    Expression result; TypeId common = 0;
    if (class_value(b.type) && types.unqualified(b.type) == types.unqualified(c.type))
        common = types.qualify(types.unqualified(b.type),types[b.type].cv | types[c.type].cv);
    else if (!(types[b.type].cv & ~types[c.type].cv) && derived_from(b.type,c.type)) common = c.type;
    else if (!(types[c.type].cv & ~types[b.type].cv) && derived_from(c.type,b.type)) common = b.type;
    if ((b.type == c.type || common) && b.category == c.category && b.category != ValueCategory::Prvalue) {
        result.type = common ? common : b.type; result.category = b.category;
        result.entity = b.entity == c.entity ? b.entity : 0; return result;
    }
    auto left = decay(b.type), right = decay(c.type);
    if (left == right) result.type = left;
    else if (pointer(left) && pointer(right)) result.type = composite_pointer(left,right);
    else if (pointer(left) && c.null_pointer_constant) result.type = left;
    else if (pointer(right) && b.null_pointer_constant) result.type = right;
    else result.type = arithmetic_type(left,right);
    if (!result.type) throw std::runtime_error("incompatible conditional operands");
    return result;
}
TypeQueryFact Analyzer::query_conditional(const TypeQuery& query, const std::vector<TypeQueryFact>& children)
{
    auto condition = boolean_conversion_value(children[0].expression);
    if (!condition.valid()) throw std::runtime_error("invalid query condition");
    TypeQueryFact result; result.expression = conditional_value(children[1].expression,children[2].expression);
    auto& value = result.expression;
    auto target = value.category == ValueCategory::Prvalue ? value.type :
        types.compound(value.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef,value.type);
    std::vector<Conversion> selected(1,condition);
    for (unsigned i = 1; i < 3; ++i) {
        auto conversion = conversion_value(children[i].expression,target);
        if (!conversion.valid()) throw std::runtime_error("invalid query branch conversion");
        if (conversion.derived && conversion.kind != Conversion::Kind::Explicit) {
            auto from = children[i].expression.type, to = types[target].child;
            if (pointer(from)) from = types[from].child;
            if (pointer(to)) to = types[to].child;
            check_base_access(from,to,query.context);
        }
        selected.push_back(conversion);
    }
    value.conversions = conversions.size(); value.count = selected.size();
    conversions.insert(conversions.end(),selected.begin(),selected.end()); return result;
}
} }
