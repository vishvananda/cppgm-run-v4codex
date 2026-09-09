#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::function_defaults(EntityId e, NodeId d, ScopeId s)
{
    Type f = types[entities[e].type];
    if (!f.count) return;
    if (!entities[e].defaults) {
        if (default_arguments.empty()) default_arguments.push_back(0);
        entities[e].defaults = default_arguments.size();
        default_arguments.resize(default_arguments.size() + f.count);
    }
    NodeId params = 0;
    while (d) {
        NodeId candidate = child(d, Kind::Parameters);
        if (candidate) params = candidate;
        NodeId nested = child(d, Kind::NestedDeclarator);
        d = nested ? ast[nested].first : 0;
    }
    unsigned i = 0;
    bool seen = false;
    for (NodeId p = ast[params].first; p && i < f.count; p = ast[p].next, ++i) {
        NodeId a = child(p, Kind::DefaultArgument);
        unsigned index = entities[e].defaults + i;
        if (a) {
            if (default_arguments[index]) throw std::runtime_error("duplicate default argument");
            NodeId value = ast[a].first;
            while (ast[value].kind == Kind::Initializer || ast[value].kind == Kind::ParenInitializer)
                value = ast[value].first;
            if (ast[value].kind == Kind::BracedInit) {
                expression(value,s); require_conversion(value,types.parameters[f.offset+i]);
            } else initialize(ast[a].first,types.parameters[f.offset+i],s);
            default_arguments[index] = value;
        }
        if (default_arguments[index]) seen = true;
        else if (seen) throw std::runtime_error("missing trailing default argument");
    }
}
} }
