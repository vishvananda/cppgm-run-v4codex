#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
QueryId Analyzer::template_exception_query(EntityId function, std::uint32_t id)
{
    if (auto query = template_exception_queries.get(id)) return query;
    auto fact = exception_specifications[id];
    auto occurrence = ast.nodes.occurrences[fact.declarator];
    if (occurrence.context) {
        auto source = template_declaration_sources.get(occurrence.source);
        auto source_fact = exception_specification_index.get(source);
        if (!source || source == function || !source_fact)
            throw std::logic_error("member exception lacks its source declaration fact");
        auto query = template_exception_query(source,source_fact);
        template_exception_queries.put(id,query); return query;
    }
    auto scope = make_scope(ScopeKind::Block,fact.scope,0,0,false);
    auto owner = entities[function].owner;
    if (scopes[owner].kind == ScopeKind::Class) {
        TemplateObjectContext object; object.owner = scopes[owner].entity;
        object.available = !entities[function].is_static;
        object.cv = types[entities[function].type].cv;
        template_object_context_index.put(scope,template_object_contexts.size());
        template_object_contexts.push_back(object);
    }
    // A prototype query consumes raw declared types and ordinal identities.
    // Pack expansion substitutes those types; it needs no runtime parameter
    // objects, projected declaration list, or function body.
    auto d = fact.declarator; NodeId parameters = 0;
    while (d) {
        if (auto p = child(d,syntax::Kind::Parameters)) parameters = p;
        auto nested = child(d,syntax::Kind::NestedDeclarator);
        d = nested ? ast[nested].first : 0;
    }
    unsigned ordinal = 0;
    for (auto p = ast[parameters].first; p; p = ast[p].next) {
        if (ast[p].kind != syntax::Kind::Parameter) continue;
        auto decl = ast[ast[p].first].next;
        auto name = terminal(decl_name(decl));
        auto type = facts[p].type;
        if (!type) throw std::logic_error("exception parameter lacks its declared type");
        auto e = make_entity(EntityKind::Parameter,scope,name,p);
        entities[e].type = parameter_body_type(type);
        entities[e].parameter_pack = child(decl,syntax::Kind::ParameterPack) != 0;
        signature_parameters.put(e,++ordinal); bind(scope,name,e);
    }
    auto query = expression_query(fact.expression,scope);
    if (!query) throw std::runtime_error("invalid exception specification query");
    template_exception_queries.put(id,query); return query;
}
} }
