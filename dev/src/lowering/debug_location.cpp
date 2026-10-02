#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
lowir_model::DebugLocation Procedural::debug_location(NodeId node)
{
    lowir_model::DebugLocation result;
    if (!linkage.debug || !node) return result;
    const auto& location = static_cast<const syntax::Ast&>(ast).locations[ast[node].location];
    if (!location.presumed_file || !location.line) return result;
    result.file = p.intern(spelling(location.presumed_file));
    result.line = location.line; result.column = location.column ? location.column : 1;
    return result;
}
} }
