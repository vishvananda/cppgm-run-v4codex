#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
TypeQueryFact Analyzer::incomplete_query(TypeId type)
{
    auto result = TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    while (types[type].kind == TypeKind::Array || types[type].kind == TypeKind::LRef || types[type].kind == TypeKind::RRef)
        type = types[type].child;
    if (class_value(type) && !entities[types[type].entity].complete) {
        auto entity = types[type].entity;
        result.incomplete = true;
        incomplete_substitution = active_type_query;
        auto key = this->key(entity,active_type_query);
        if (active_type_query && !class_query_edges.get(key)) {
            class_query_edges.put(key,1);
            auto edge = query_dependencies.size();
            query_dependencies.push_back({active_type_query,class_query_heads.get(entity)});
            class_query_heads.put(entity,edge);
        }
    }
    return result;
}
void Analyzer::record_query_dependency(QueryId source)
{
    if (!active_type_query || source == active_type_query) return;
    auto k = key(source,active_type_query);
    if (query_dependency_edges.get(k)) return;
    query_dependency_edges.put(k,1);
    auto edge = query_dependencies.size();
    query_dependencies.push_back({active_type_query,query_dependency_heads.get(source)});
    query_dependency_heads.put(source,edge);
}
void Analyzer::complete_query_class(EntityId entity)
{
    std::vector<QueryId> work;
    for (auto edge = class_query_heads.get(entity); edge; edge = query_dependencies[edge].next)
        work.push_back(query_dependencies[edge].consumer);
    for (unsigned i = 0; i < work.size(); ++i) {
        auto query = work[i];
        if (query_facts[query].state == FactState::NotStarted) continue;
        // A completion demanded by this query cannot invalidate its active
        // evaluation: it has not published a result using an incomplete fact.
        if (query_facts[query].state == FactState::Active) continue;
        query_facts[query] = TypeQueryFact();
        ++query_invalidations;
        if (auto value = query_value_index.get(query)) query_values[value] = QueryValue();
        for (auto edge = query_dependency_heads.get(query); edge; edge = query_dependencies[edge].next)
            work.push_back(query_dependencies[edge].consumer);
    }
}
} }
