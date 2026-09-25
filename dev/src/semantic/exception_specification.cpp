#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::check_exception_redeclaration(EntityId e, unsigned old, unsigned spec, bool destructor)
{
    if (!(old & 128)) return;
    unsigned previous = old & 3;
    bool prior_throwing = previous == 0 || previous == 2;
    bool current_throwing = spec == 0 || spec == 2;
    if (destructor && (previous || spec) && (!previous || !spec)) {
        auto cls = scopes[entities[e].owner].entity;
        require_destructor_class(cls);
        bool implicit_throwing = !implicit_destructor_nonthrowing(cls);
        if (!previous) prior_throwing = implicit_throwing;
        if (!spec) current_throwing = implicit_throwing;
    }
    if (prior_throwing != current_throwing) throw std::runtime_error("conflicting exception specifications");
}
void Analyzer::exception_specification(EntityId e, NodeId d, ScopeId s)
{
    ExceptionSpecificationFact fact; fact.declarator = d; fact.scope = s;
    fact.destructor = ast[ast[decl_name(d)].last].op == OP_COMPL;
    for (auto c = ast[d].first; c; c = ast[c].next) {
        if ((ast[c].kind != Kind::FunctionQualifier && ast[c].kind != Kind::Noexcept) || ast[c].op != KW_NOEXCEPT) continue;
        if (!ast[c].first) fact.specification = 1;
        else fact.expression = ast[c].first;
    }
    fact.previous = exception_specification_index.get(e);
    fact.prior_specification = entities[e].exception_spec;
    if (!fact.specification && !fact.expression && entities[e].key == KW_DELETE) fact.specification = 1;
    if (!fact.expression && !fact.previous) {
        check_exception_redeclaration(e,fact.prior_specification,fact.specification,fact.destructor);
        entities[e].exception_spec = 128 | (fact.specification ? fact.specification : fact.destructor ? fact.prior_specification & 3 : 0);
        return;
    }
    auto id = exception_specifications.size(); exception_specifications.push_back(fact);
    exception_specification_index.put(e,id);
    if (entities[e].template_info || entities[e].template_pattern) return;
    if (class_depth) {
        // The concrete declaration retains its immutable environment, even if
        // its class contains an incomplete recursive member. Only non-template
        // source declarations must be checked at this class completion event.
        if (!definition_owner(scopes[entities[e].owner].entity).specialization) declaration_exceptions.push_back(e);
    } else demand_exception_specification(e);
}
unsigned Analyzer::evaluate_exception_specification(EntityId e, std::uint32_t id)
{
    auto fact = exception_specifications[id];
    if (fact.state == FactState::Success) return fact.specification;
    if (fact.state == FactState::Active) throw std::runtime_error("recursive exception specification demand");
    if (fact.state == FactState::Failure) throw std::runtime_error("failed exception specification");
    exception_specifications[id].state = FactState::Active;
    auto saved_constant = active_constant; active_constant = 0;
    ++unevaluated_depth;
    try {
        ++exception_work;
        unsigned spec = fact.specification;
        if (fact.expression) {
            auto node = fact.expression; auto scope = fact.scope;
            Constant value;
            if (fact.pattern) {
                auto index = entities[e].specialization;
                auto head = templates[entities[fact.pattern].template_info];
                auto parent = head.parent_frame;
                if (head.source_count) parent = substitution_frame(index,head.source_parameters,head.source_count,parent);
                auto frame = substitution_frame(index,head.offset,head.count,parent);
                auto query = template_exception_query(fact.pattern,exception_specification_index.get(fact.pattern));
                Index bindings, cache;
                query = substitute_query(query,bindings,cache,frame);
                if (!query) throw std::runtime_error("invalid substituted exception query");
                auto conversion = boolean_conversion_value(query_fact(query).expression);
                check_fixed_conversion(query_fact(query).expression,0,conversion,entities[e].owner);
                value = constant_query_conversion(query,conversion);
            } else {
            // Parameter names in exception specifications denote prototype
            // entities. They can supply types, but never runtime values.
            auto parameters = child(fact.declarator,Kind::Parameters);
            // A checked body already owns raw parameter types and pack
            // bindings. Do not shadow them with adjusted signature types.
            bool body_scope = scope == entities[e].scope && scopes[scope].kind == ScopeKind::Function;
            if ((ast[parameters].first || entities[e].member_info) && !body_scope) {
                auto parent = scope; scope = make_scope(ScopeKind::Block,parent);
                if (entities[e].member_info) {
                    TemplateObjectContext object; object.owner = scopes[entities[e].owner].entity;
                    object.available = !entities[e].is_static; object.cv = types[entities[e].type].cv;
                    template_object_context_index.put(scope,template_object_contexts.size());
                    template_object_contexts.push_back(object);
                }
                unsigned ordinal = 0; auto f = types[entities[e].type];
                for (auto p = ast[parameters].first; p && ordinal < f.count; p = ast[p].next) {
                    if (ast[p].kind != Kind::Parameter) continue;
                    auto name = terminal(decl_name(ast[ast[p].first].next));
                    auto parameter = make_entity(EntityKind::Parameter,scope,name,p);
                    entities[parameter].type = types.parameters[f.offset+ordinal++];
                    bind(scope,name,parameter);
                }
            }
            expression(node,scope);
            auto conversion = boolean_conversion(node);
            value = constant_node_conversion(node,conversion,scope);
            }
            if (!value.valid) throw std::runtime_error("nonconstant noexcept specification");
            spec = constant_truth(value) ? 3 : 2;
        }
        auto old = fact.previous ? 128 | evaluate_exception_specification(e,fact.previous) : fact.prior_specification;
        check_exception_redeclaration(e,old,spec,fact.destructor);
        exception_specifications[id].specification = spec;
        exception_specifications[id].state = FactState::Success;
        --unevaluated_depth; active_constant = saved_constant; return spec;
    } catch (const UnavailableSemanticFact&) {
        --unevaluated_depth; active_constant = saved_constant;
        exception_specifications[id].state = FactState::NotStarted; throw;
    } catch (...) {
        --unevaluated_depth; active_constant = saved_constant;
        exception_specifications[id].state = FactState::Failure; throw;
    }
}
void Analyzer::demand_exception_specification(EntityId e)
{
    auto id = exception_specification_index.get(e);
    if (!id && entities[e].specialization && !entities[e].explicit_specialization) {
        auto pattern = specializations[entities[e].specialization].pattern;
        auto source = exception_specification_index.get(pattern);
        if (source) {
            auto fact = exception_specifications[source];
            fact.state = FactState::NotStarted; fact.pattern = pattern;
            fact.previous = 0; fact.prior_specification = 0;
            id = exception_specifications.size(); exception_specifications.push_back(fact);
            exception_specification_index.put(e,id);
        } else entities[e].exception_spec = entities[pattern].exception_spec;
    }
    if (id) entities[e].exception_spec = 128 | evaluate_exception_specification(e,id);
}
} }
