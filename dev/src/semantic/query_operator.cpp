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
        class_operand |= class_value(c.expression.type) || pattern_class_type(c.expression.type);
    }
    auto object = args[0].type;
    ScopeId naming = 0; EntityId family = q.entity;
    if (class_value(object) || pattern_class_type(object)) {
        if (class_value(object)) complete_class(types[object].entity);
        naming = entities[types[object].entity].scope;
        family = merge_lookup(family,lookup(naming,q.name,Lookup::Ordinary,true));
    }
    if (named && q.op != OP_LPAREN && q.op != OP_LSQUARE && q.op != OP_ASS && q.op != OP_ARROW)
        family = merge_lookup(family,associated_type_lookup(q.name,std::move(argument_types)));
    struct Candidate { EntityId entity; unsigned offset, builtin; TypeId surrogate; };
    std::vector<Candidate> viable; std::vector<Conversion> sequences;
    for (auto e : candidates(family)) {
        ++candidate_work;
        if (entities[e].template_info) e = deduce_function(e,args,scopes[entities[e].owner].kind == ScopeKind::Class && !entities[e].is_static);
        if (!e) continue;
        bool member = entities[e].member_info && !entities[e].is_static;
        auto f = types[entities[e].type];
        auto supplied = args.size()-member;
        if (q.op == OP_LPAREN ? ((!f.variadic && supplied > f.count) ||
            (supplied < f.count && (!entities[e].defaults || !default_arguments[entities[e].defaults+supplied]))) :
            supplied != f.count) continue;
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
                i-member < f.count ? conversion_value(args[i],types.parameters[f.offset+i-member]) : ellipsis_conversion_value(args[i]);
            valid = c.valid(); sequences.push_back(c);
        }
        if (valid) viable.push_back({e,begin,0,0}); else sequences.resize(begin);
    }
    if (q.op == OP_LPAREN) {
        Index seen;
        for (EntityId e : conversion_candidates(object)) {
            TypeId target = decay(types[entities[e].type].child);
            if (!pointer(target) || types[types[target].child].kind != TypeKind::Function || seen.get(target)) continue;
            seen.put(target,1);
            auto f = types[types[target].child];
            if (args.size()-1 < f.count || (!f.variadic && args.size()-1 != f.count)) continue;
            auto begin = sequences.size();
            auto callee = conversion_function_value(args[0],target);
            bool valid = callee.valid(); sequences.push_back(callee);
            for (unsigned j = 1; valid && j < args.size(); ++j) {
                auto c = j <= f.count ? conversion_value(args[j],types.parameters[f.offset+j-1]) : ellipsis_conversion_value(args[j]);
                valid = c.valid(); sequences.push_back(c);
            }
            if (valid) viable.push_back({callee.function,unsigned(begin),0,types[target].child});
            else sequences.resize(begin);
        }
    }
    std::vector<BuiltinOperator> builtins; builtin_operators_values(q.op,args,builtins);
    for (unsigned i = 0; i < builtins.size(); ++i) {
        viable.push_back({0,unsigned(sequences.size()),i+1,0});
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
        auto returned = types[selected.surrogate ? selected.surrogate : entities[selected.entity].type].child;
        r.expression.type = value_type(returned);
        r.expression.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    }
    r.selected = selected.entity;
    std::vector<Conversion> chosen(sequences.begin()+selected.offset,sequences.begin()+selected.offset+args.size());
    for (unsigned i = 0; i < args.size(); ++i) check_fixed_conversion(args[i],0,chosen[i],q.context);
    if (selected.entity && !selected.surrogate) {
        auto f = types[entities[selected.entity].type];
        bool member = entities[selected.entity].member_info && !entities[selected.entity].is_static;
        for (unsigned i = args.size()-member; i < f.count; ++i) {
            Conversion c; default_argument(selected.entity,i,&c,DefaultReason::Recipe); chosen.push_back(c);
        }
        for (unsigned i = 0; i < f.count; ++i) reject_abstract(types.parameters[f.offset+i]);
    }
    r.expression.conversions = conversions.size(); r.expression.count = chosen.size();
    conversions.insert(conversions.end(),chosen.begin(),chosen.end());
    return r;
}
Expression Analyzer::conditional_value(Expression b, Expression c, std::vector<Conversion>& selected)
{
    Expression result; TypeId common = 0;
    Expression original[2] = {b,c};
    Conversion matched[2];
    if (b.type != c.type && (class_value(b.type) || class_value(c.type))) {
        auto match = [&](Expression from, Expression to) {
            Conversion conversion;
            if (to.category != ValueCategory::Prvalue) {
                auto ref = types.compound(to.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef,to.type);
                conversion = conversion_value(from,ref);
                bool direct = conversion.valid() && !conversion.temporary;
                if (direct && conversion.kind != Conversion::Kind::User && to.category == ValueCategory::Lvalue)
                    direct = from.category == ValueCategory::Lvalue;
                if (direct && conversion.kind == Conversion::Kind::User) {
                    auto returned = types[entities[conversion.function].type].child;
                    direct = types[returned].kind == TypeKind::LRef ||
                        (to.category != ValueCategory::Lvalue && types[returned].kind == TypeKind::RRef);
                    direct &= !user_conversions[conversion.materialization].result.temporary;
                }
                if (direct) return conversion;
            }
            if (class_value(from.type) && class_value(to.type) &&
                (types.unqualified(from.type) == types.unqualified(to.type) ||
                 derived_from(from.type,to.type) || derived_from(to.type,from.type)) &&
                ((types[from.type].cv & ~types[to.type].cv) ||
                 (types.unqualified(from.type) != types.unqualified(to.type) && !derived_from(from.type,to.type))))
                return Conversion();
            auto target = class_value(from.type) && class_value(to.type) &&
                (types.unqualified(from.type) == types.unqualified(to.type) || derived_from(from.type,to.type)) ?
                to.type : decay(to.type);
            return conversion_value(from,target);
        };
        matched[0] = match(b,c); matched[1] = match(c,b);
        if (matched[0].ambiguous || matched[1].ambiguous || (matched[0].valid() && matched[1].valid()))
            throw std::runtime_error("ambiguous conditional conversion");
        for (unsigned i = 0; i < 2; ++i) if (matched[i].valid()) {
            auto& value = i ? c : b;
            auto target = matched[i].target;
            value.type = value_type(target); value.entity = 0;
            value.category = types[target].kind == TypeKind::LRef ? ValueCategory::Lvalue :
                types[target].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        }
    }
    if (class_value(b.type) && types.unqualified(b.type) == types.unqualified(c.type))
        common = types.qualify(types.unqualified(b.type),types[b.type].cv | types[c.type].cv);
    else if (!(types[b.type].cv & ~types[c.type].cv) && derived_from(b.type,c.type)) common = c.type;
    else if (!(types[c.type].cv & ~types[b.type].cv) && derived_from(c.type,b.type)) common = b.type;
    if ((b.type == c.type || common) && b.category == c.category && b.category != ValueCategory::Prvalue) {
        result.type = common ? common : b.type; result.category = b.category;
        result.entity = b.entity == c.entity ? b.entity : 0;
    } else if (b.type != c.type && (class_value(b.type) || class_value(c.type))) {
        std::vector<BuiltinOperator> candidates;
        builtin_operators_values(OP_QMARK,{b,c},candidates);
        if (candidates.empty()) throw std::runtime_error("incompatible conditional operands");
        unsigned best = 0;
        for (unsigned i = 1; i < candidates.size(); ++i)
            if (better(candidates[i].arguments,candidates[best].arguments,2)) best = i;
        for (unsigned i = 0; i < candidates.size(); ++i)
            if (i != best && !better(candidates[best].arguments,candidates[i].arguments,2))
                throw std::runtime_error("ambiguous conditional operands");
        result.type = candidates[best].type;
        selected.assign(candidates[best].arguments,candidates[best].arguments+2);
        return result;
    } else {
        auto left = decay(b.type), right = decay(c.type);
        if (left == right) result.type = left;
        else if (pointer(left) && pointer(right)) result.type = composite_pointer(left,right);
        else if (pointer(left) && c.null_pointer_constant) result.type = left;
        else if (pointer(right) && b.null_pointer_constant) result.type = right;
        else result.type = arithmetic_type(left,right);
    }
    if (!result.type) throw std::runtime_error("incompatible conditional operands");
    auto target = result.category == ValueCategory::Prvalue ? result.type :
        types.compound(result.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef,result.type);
    for (unsigned i = 0; i < 2; ++i) {
        auto conversion = matched[i];
        if (conversion.valid() && !conversion.reference &&
            types.unqualified(conversion.target) == types.unqualified(target)) conversion.target = target;
        if (conversion.valid() && conversion.kind == Conversion::Kind::User && conversion.target != target) {
            // Preserve the conversion function chosen by the directional match;
            // only its final standard conversion changes when a glvalue decays.
            auto returned = types[entities[conversion.function].type].child;
            Expression value; value.type = value_type(returned);
            value.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
                types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
            user_conversions[conversion.materialization].result = standard_conversion(value,target);
            conversion.target = target; conversion.reference = result.category != ValueCategory::Prvalue;
        }
        if (!conversion.valid() || conversion.target != target) conversion = conversion_value(original[i],target);
        selected.push_back(conversion);
    }
    return result;
}
TypeQueryFact Analyzer::query_conditional(const TypeQuery& query, const std::vector<TypeQueryFact>& children)
{
    auto condition = boolean_conversion_value(children[0].expression);
    check_fixed_conversion(children[0].expression,0,condition,query.context);
    std::vector<Conversion> branches;
    TypeQueryFact result; result.expression = conditional_value(children[1].expression,children[2].expression,branches);
    auto& value = result.expression;
    std::vector<Conversion> selected(1,condition);
    for (unsigned i = 1; i < 3; ++i) {
        auto conversion = branches[i-1];
        check_fixed_conversion(children[i].expression,0,conversion,query.context);
        selected.push_back(conversion);
    }
    value.conversions = conversions.size(); value.count = selected.size();
    conversions.insert(conversions.end(),selected.begin(),selected.end()); return result;
}
} }
