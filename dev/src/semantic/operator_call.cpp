#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::operator_expression(NodeId n, ScopeId s, ETokenType op, std::vector<NodeId> args, Expression& result)
{
    bool named = false;
    for (NodeId a : args) named |= types[expressions[a].type].kind == TypeKind::Named;
    if (!named) return false;
    IdentifierId name = operator_name(op);
    TypeId object = expressions[args[0]].type;
    ScopeId naming = types[object].kind == TypeKind::Named ? entities[types[object].entity].scope : 0;
    EntityId family = 0;
    if (op == OP_ASS && naming && scopes[naming].kind == ScopeKind::Class) ensure_transfers(object, true);
    if (naming && scopes[naming].kind == ScopeKind::Class) family = lookup(naming, name, Lookup::Ordinary, true);
    if (op != OP_LPAREN && op != OP_LSQUARE && op != OP_ASS && op != OP_ARROW) {
        EntityId ordinary = lookup(s, name);
        if (function_binding(ordinary)) family = merge_lookup(family, ordinary);
        family = merge_lookup(family, associated_lookup(name, args));
    }
    bool class_operand = false;
    for (NodeId a : args) if (a) class_operand |= class_value(expressions[a].type);
    if (!function_binding(family) && !class_operand) return false;
    struct Candidate { EntityId entity; std::size_t offset; bool member; unsigned builtin; TypeId surrogate; };
    std::vector<Candidate> viable;
    std::vector<Conversion> sequences;
    for (EntityId e : candidates(family)) {
        ++candidate_work;
        if (entities[e].template_info) continue;
        bool member = entities[e].member_info && !entities[e].is_static;
        Type f = types[entities[e].type];
        if (members[entities[e].member_info].transfer == TransferKind::MoveAssignment &&
            members[entities[e].member_info].synthetic && deleted_transfer(e)) continue;
        if (args.size() != f.count + member) continue;
        std::size_t begin = sequences.size();
        bool valid = true;
        for (std::size_t i = 0; valid && i < args.size(); ++i) {
            Conversion c;
            if (member && !i) {
                c = object_conversion(e, object, expressions[args[0]].category, naming);
            } else {
                TypeId wanted = types.parameters[f.offset+i-member];
                if (args[i]) c = conversion(args[i], wanted);
                else if (fundamental(wanted, FT_INT)) { c.rank = 0; c.target = wanted; }
            }
            valid = c.valid(); sequences.push_back(c);
        }
        if (valid) viable.push_back({e, begin, member, 0, 0}); else sequences.resize(begin);
    }
    if (op == OP_LPAREN) {
        Index seen;
        for (EntityId e : conversion_candidates(object)) {
            TypeId target = decay(types[entities[e].type].child);
            if (!pointer(target) || types[types[target].child].kind != TypeKind::Function || seen.get(target)) continue;
            seen.put(target,1);
            Type f = types[types[target].child];
            if (args.size()-1 < f.count || (!f.variadic && args.size()-1 != f.count)) continue;
            std::size_t begin = sequences.size();
            Conversion callee = conversion_function(args[0],target);
            bool valid = callee.valid(); sequences.push_back(callee);
            for (unsigned j = 1; valid && j < args.size(); ++j) {
                Conversion c = j <= f.count ? conversion(args[j],types.parameters[f.offset+j-1]) : ellipsis_conversion(args[j]);
                valid = c.valid(); sequences.push_back(c);
            }
            if (valid) viable.push_back({callee.function,begin,false,0,types[target].child}); else sequences.resize(begin);
        }
    }
    std::vector<BuiltinOperator> builtins;
    if (class_operand) builtin_operators(op,args,builtins);
    for (unsigned i = 0; i < builtins.size(); ++i) {
        viable.push_back({0,sequences.size(),false,i+1,0});
        for (unsigned j = 0; j < args.size(); ++j) sequences.push_back(builtins[i].arguments[j]);
    }
    // Built-in comma, address and enum operations remain candidates when no
    // user-defined candidate is viable. Expected rejection does not throw.
    if (viable.empty()) return false;
    std::size_t best = 0;
    for (std::size_t i = 1; i < viable.size(); ++i)
        if (better(sequences.data()+viable[i].offset, sequences.data()+viable[best].offset, args.size())) best = i;
    for (std::size_t i = 0; i < viable.size(); ++i)
        if (i != best && !better(sequences.data()+viable[best].offset, sequences.data()+viable[i].offset, args.size()))
            throw std::runtime_error("ambiguous operator overload");
    Candidate selected = viable[best];
    if (selected.surrogate) {
        Conversion callee = sequences[selected.offset]; apply_conversion(args[0],callee);
        record_object(result,0,0,0);
        object_uses[result.object_use].callee_conversion = conversions.size(); conversions.push_back(callee);
        std::vector<NodeId> arguments(args.begin()+1,args.end());
        std::vector<Conversion> chosen(sequences.begin()+selected.offset+1,sequences.begin()+selected.offset+args.size());
        record_call(result,arguments,chosen);
        TypeId returned = types[selected.surrogate].child;
        facts[n].type = returned; result.type = value_type(returned);
        result.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        return true;
    }
    if (selected.builtin) {
        auto builtin = builtins[selected.builtin-1];
        result.type = builtin.type; result.category = builtin.category;
        for (unsigned j = 0; j < args.size(); ++j) record_conversion(result,args[j],sequences[selected.offset+j]);
        return true;
    }
    if (deleted_transfer(selected.entity))
        throw std::runtime_error("deleted operator");
    check_access(selected.entity, s, naming, object);
    demand_member(selected.entity);
    if (selected.member) {
        record_object(result, args[0], types.parameters[types[call_type(selected.entity)].offset],
            base_steps(object, scopes[entities[selected.entity].owner].entity));
        object_uses[result.object_use].virtual_slot = members[entities[selected.entity].member_info].virtual_slot;
    }
    result.form = ExpressionForm::OperatorCall;
    std::vector<NodeId> arguments;
    std::vector<Conversion> selected_arguments;
    for (std::size_t i = selected.member; i < args.size(); ++i) {
        selected_arguments.push_back(sequences[selected.offset+i]); arguments.push_back(args[i]);
    }
    record_call(result, arguments, selected_arguments);
    TypeId returned = types[entities[selected.entity].type].child;
    facts[n].entity = selected.entity; facts[n].type = returned;
    result.type = value_type(returned);
    result.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
        types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
    return true;
}
} }
