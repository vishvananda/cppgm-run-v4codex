#include "native/selection.h"
namespace native {
using namespace lowir_model;
void Selector::index_exception_clauses()
{
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        auto id = p.block_order[b].index;
        auto instructions = p.blocks[id-1].instructions;
        ExceptionHandler handler; handler.clauses.begin = f.exception_clauses.size();
        bool present = false;
        for (unsigned n = instructions.begin; n < instructions.end(); ++n) {
            const auto& i = p.instructions[n];
            if (i.opcode == Opcode::EhCatch) {
                require(i.operands.count == 2,"native typed catch requires an explicit selector");
                f.exception_clauses.push_back({SymbolId(arg(i,0).ref),unsigned(arg(i,1).data.integer),i.catch_binding,0});
            } else if (i.opcode == Opcode::EhFilter) {
                require(f.host,"exception filters require host exception runtime");
                require(i.operands.count && arg(i,i.operands.count-1).kind == lowir_model::Operand::Integer,
                    "native exception filter requires an explicit selector");
                ExceptionClause clause(SymbolId(),unsigned(arg(i,i.operands.count-1).data.integer),ir_model::CatchBinding::Value,0);
                clause.filtered = true; clause.filter.begin = f.exception_filter_types.size();
                for (unsigned j = 0; j+1 < i.operands.count; ++j)
                    f.exception_filter_types.push_back(SymbolId(arg(i,j).ref));
                clause.filter.count = f.exception_filter_types.size()-clause.filter.begin;
                f.exception_clauses.push_back(clause);
            } else if (i.opcode == Opcode::EhCatchAll) {
                require(i.operands.count == 1,"native catch-all requires an explicit selector");
                f.exception_clauses.push_back({SymbolId(),unsigned(arg(i,0).data.integer),ir_model::CatchBinding::Value,0});
            } else if (i.opcode == Opcode::EhCleanup && !i.operands.count) handler.cleanup = true;
            else break;
            present = true;
        }
        if (!present) continue;
        handler.clauses.count = f.exception_clauses.size()-handler.clauses.begin;
        f.exception_handlers.push_back(handler); workspace.exception_handlers[id] = f.exception_handlers.size();
    }
}
} // namespace native
