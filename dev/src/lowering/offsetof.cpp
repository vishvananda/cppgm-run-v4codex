#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::offsetof_expression(NodeId n)
{
    auto type_id = sem.types.fundamental(FT_UNSIGNED_LONG_INT);
    Value result(Operand::integer(0),IRType::I64,type_id);
    std::uint64_t offset = 0;
    for (auto part = ast[ast[n].first].next; part; part = ast[part].next) {
        auto query = sem.offsetof_path_queries.get(part);
        auto layout = sem.offsetof_layout_index.get(query);
        if (!layout) throw std::logic_error("missing offsetof path layout");
        auto step = sem.offsetof_layouts[layout]; offset += step.offset;
        if (step.stride) {
            auto index = convert(expression(ast[part].first),type_id);
            auto bytes = emit(Opcode::Binary,IRType::I64,{index.operand,Operand::integer(step.stride)},Operation::Mul);
            result = emit(Opcode::Binary,IRType::I64,{result.operand,bytes.operand},Operation::Add);
        }
    }
    if (offset) result = emit(Opcode::Binary,IRType::I64,{result.operand,Operand::integer(offset)},Operation::Add);
    result.type = type_id; return result;
}
} }
