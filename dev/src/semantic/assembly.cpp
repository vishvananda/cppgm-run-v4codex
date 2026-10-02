#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::resolve_assembly(NodeId n, ScopeId s, bool pattern)
{
    using namespace syntax;
    std::vector<Expression> operands; std::vector<NodeId> nodes;
    for (auto c = ast.first(n); c; c = ast.next(c)) {
        auto source = ast.first(c); Expression x;
        if (pattern) {
            bool dependent = bind_template_expression(source,s);
            if (!dependent) x = template_statement_value(source,s);
        } else x = expression(source,s);
        auto flags = ast.flags(c);
        if (x.type && !dependent_type(x.type)) {
            if (!integral(x.type) || fundamental(x.type,FT_BOOL) || size(x.type) > 8)
                throw std::runtime_error("asm operand requires an integer machine word");
            if ((flags & (AsmOutput|AsmMemory)) && x.category != ValueCategory::Lvalue)
                throw std::runtime_error("asm operand requires an lvalue");
            if ((flags & AsmOutput) && (types[x.type].cv & 1)) throw std::runtime_error("asm output is const");
            if (x.entity && field_fact(x.entity).bit_field)
                throw std::runtime_error("asm bit-field operand is unsupported");
            if ((flags & AsmImmediate) && !evaluate(source,s).valid) throw std::runtime_error("asm immediate is not constant");
            // Outputs modify their source storage even when no C++ assignment
            // node exists. Memory operands expose that storage to the recipe.
            // Retire the same private-local proof as ordinary writes/addresses.
            if (flags & (AsmOutput|AsmMemory)) observe_scalar(source,flags & AsmOutput);
        } else x.type = 0;
        nodes.push_back(c); operands.push_back(x);
    }
    for (unsigned i = 0; i < nodes.size(); ++i) if (ast.literal(nodes[i])) {
        auto match = ast.literal(nodes[i])-1;
        if (operands[i].type && operands[match].type && size(operands[i].type) != size(operands[match].type))
            throw std::runtime_error("asm matching operands have different widths");
    }
    const auto& plan = ast.assemblies[ast.literal(n)];
    for (unsigned i = 0; i < plan.count; ++i) {
        auto instruction = ast.assembly_instructions[plan.begin+i];
        if (instruction.op <= AsmOp::Fence) continue;
        auto type = operands[instruction.destination].type;
        if (!type) continue;
        auto bytes = size(type);
        if (instruction.width && instruction.width != bytes) throw std::runtime_error("asm instruction width disagrees with operand");
        if (instruction.op == AsmOp::Bswap && bytes != 4 && bytes != 8) throw std::runtime_error("bswap requires 32 or 64 bits");
        bool binary = (instruction.op >= AsmOp::Move && instruction.op <= AsmOp::Xor) || instruction.op >= AsmOp::Exchange;
        if (binary && !instruction.immediate && operands[instruction.source].type && size(operands[instruction.source].type) != bytes &&
            !(ast.flags(nodes[instruction.source]) & AsmImmediate)) throw std::runtime_error("asm source and destination widths differ");
    }
}
} }
