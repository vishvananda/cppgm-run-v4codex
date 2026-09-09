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
    if (!function_binding(family)) return false;
    struct Candidate { EntityId entity; std::size_t offset; bool member; };
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
        if (valid) viable.push_back({e, begin, member}); else sequences.resize(begin);
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
    if (deleted_transfer(selected.entity))
        throw std::runtime_error("deleted operator");
    check_access(selected.entity, s, naming, object);
    demand_member(selected.entity);
    if (selected.member) record_object(result, args[0], types.parameters[types[call_type(selected.entity)].offset],
        base_steps(object, scopes[entities[selected.entity].owner].entity));
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
