#include "semantic/analyzer.h"
#include <cstring>
namespace cppgm { namespace semantic {
Constant Analyzer::literal_element(std::uint32_t id, Constant index)
{
    auto lit = ast.literals[id];
    if (lit.kind != LiteralKind::string || lit.suffix || !index.valid || !integral(index.type) || index.bits >= lit.elements)
        return Constant();
    auto bytes = fundamental_width(lit.type); std::uint64_t value = 0;
    std::memcpy(&value,ast.literal_bytes.data()+lit.offset+index.bits*bytes,bytes);
    auto type = types.fundamental(lit.type); return convert(Constant(type,value),type);
}
} }
