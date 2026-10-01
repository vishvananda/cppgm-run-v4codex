#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
EntityId Analyzer::source_string(IdentifierId text, NodeId source)
{
    if (!text) text = ids.intern(TextView("",0));
    if (auto old = source_string_index.get(text)) return old;
    auto e = make_entity(EntityKind::Variable,global,0,source);
    entities[e].type = types.compound(TypeKind::Array,types.qualify(types.fundamental(FT_CHAR),1),ids.spelling(text).size+1);
    entities[e].is_static = true; entities[e].definition = source;
    predefined_strings.put(e,text); source_string_index.put(text,e);
    return e;
}
unsigned Analyzer::remember_source_site(NodeId n, ScopeId s)
{
    if (!n) return 0;
    if (auto old = source_site_index.get(n)) return old;
    auto location = static_cast<const syntax::Ast&>(ast).locations[ast[n].location];
    SourceSite site; site.line = location.line;
    site.file = source_string(location.presumed_file,n);
    std::vector<ScopeId> missing;
    auto context = s;
    while (context && scopes[context].kind != ScopeKind::Function && !source_function_scopes.get(context)) {
        missing.push_back(context); context = scopes[context].parent;
    }
    if (context && scopes[context].kind != ScopeKind::Function) context = source_function_scopes.get(context)-1;
    for (auto scope : missing) source_function_scopes.put(scope,context+1);
    if (context) site.function = function_name_string(context,ids.intern(TextView("__FUNCTION__",12)),n);
    else site.function = source_string(0,n);
    auto id = source_sites.size(); source_sites.push_back(site); source_site_index.put(n,id);
    return id;
}
Constant Analyzer::source_builtin_constant(Intrinsic kind, unsigned site)
{
    if (!site) throw std::logic_error("missing source invocation fact");
    auto fact = source_sites[site];
    if (kind == Intrinsic::SourceLine || kind == Intrinsic::SourceColumn)
        return Constant(types.fundamental(FT_UNSIGNED_INT),kind == Intrinsic::SourceLine ? fact.line : 0);
    auto c = types.qualify(types.fundamental(FT_CHAR),1);
    auto entity = kind == Intrinsic::SourceFile ? fact.file : fact.function;
    auto address = constant_subobject(constant_entity_address(entity),c,0);
    return Constant(types.compound(TypeKind::Pointer,c),address);
}
} }
