#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
Constant Analyzer::constant_fold_conversion(Constant value, const Conversion& c)
{
    if (!value.valid || c.constant_forbidden) return {};
    if (c.kind == Conversion::Kind::User) {
        auto kind = types[value.type].kind;
        auto address = kind == TypeKind::LRef || kind == TypeKind::RRef ? unsigned(value.bits) : constant_storage_address(value.type,value);
        address = constant_base_address(address,entities[scopes[entities[c.function].owner].entity].type);
        if (!address || members[entities[c.function].member_info].virtual_member) return {};
        return constant_result_conversion(execute_constant(c.function,{},address),c);
    }
    if (c.kind == Conversion::Kind::Construction) {
        auto material = conversion_objects[c.materialization];
        if (material.call.argument_count != 1) return {};
        value = constant_fold_conversion(value,conversions[material.call.conversions]);
        value = constant_construct(material.constructor,{value});
        if (c.reference && value.valid) return Constant(c.target,constant_storage_address(value.type,value));
        return value;
    }
    if (c.reference && c.temporary) {
        value = convert(value,types[c.target].child,true);
        return value.valid ? Constant(c.target,constant_storage_address(value.type,value)) : Constant();
    }
    return convert(value,c.target,true);
}
Constant Analyzer::constant_fold(NodeId n, ScopeId s, Constant* callable)
{
    struct Frame { unsigned id, phase = 0; Constant left; explicit Frame(unsigned i) : id(i) {} };
    std::vector<Frame> work; work.emplace_back(fold_root(n)); Constant value;
    while (!work.empty()) {
        auto& frame = work.back(); auto step = fold_steps[frame.id]; auto op = step.operation;
        if (active_constant && !constant_step()) return {};
        if (step.source) {
            auto expression = expressions[step.source];
            if (expression.category == ValueCategory::Prvalue) value = evaluate(step.source,s);
            else {
                auto address = constant_address(step.source,s);
                value = address ? Constant(types.compound(TypeKind::LRef,expression.type),address) : Constant();
            }
            if (!value.valid) return {};
            work.pop_back(); continue;
        }
        if (!frame.phase) { frame.phase = 1; work.emplace_back(step.left); continue; }
        if (frame.phase == 1) {
            frame.left = op.result.count ? constant_fold_conversion(value,conversions[op.result.conversions]) : value;
            if (!frame.left.valid) return {};
            if (!op.function && ((op.op == OP_LAND && !constant_truth(frame.left)) || (op.op == OP_LOR && constant_truth(frame.left)))) {
                value = Constant(types.fundamental(FT_BOOL),op.op == OP_LOR); work.pop_back(); continue;
            }
            frame.phase = 2; work.emplace_back(step.right); continue;
        }
        auto a = frame.left;
        auto b = op.result.count ? constant_fold_conversion(value,conversions[op.result.conversions+1]) : value;
        if (!b.valid) return {};
        if (op.function) {
            unsigned receiver = 0; std::vector<Constant> args;
            if (op.receiver) {
                receiver = constant_base_address(a.bits,entities[scopes[entities[op.function].owner].entity].type);
                if (!receiver || op.virtual_slot) return {};
            } else args.push_back(a);
            args.push_back(b); value = execute_constant(op.function,args,receiver);
        } else if (op.op == OP_COMMA) value = b;
        else if (op.op == OP_DOTSTAR || op.op == OP_ARROWSTAR) {
            b = constant_indirect(b);
            if (!b.valid || !b.bits || types[b.type].kind != TypeKind::MemberPointer) return {};
            if (op.op == OP_ARROWSTAR) a = constant_indirect(a);
            auto address = unsigned(a.bits);
            address = constant_base_projection(address,op.adjustment);
            address = constant_member_receiver(address,b);
            if (!address) return {};
            auto member = member_constant_value(b).member;
            if (op.result.form == ExpressionForm::BoundMember) {
                if (!callable) return {};
                *callable = b;
                value = Constant(types.compound(TypeKind::LRef,constant_addresses[address].type),address);
            } else {
                address = constant_subobject(address,entities[member].type,member);
                value = Constant(types.compound(TypeKind::LRef,op.result.type),address);
            }
        }
        else if (syntax::expression_precedence(op.op) == 2) {
            auto address = unsigned(a.bits); auto storage = constant_addresses[address].storage;
            if (!active_constant || !address || !constant_storage[storage].live || !constant_storage[storage].frame) return {};
            if (op.op != OP_ASS) {
                auto common = conversions[op.result.conversions+2].target;
                b = binary(compound_operation(op.op),convert(constant_read(address),common,true),b,true);
            }
            b = convert(b,op.result.type,true); if (!b.valid) return {};
            constant_write(address,b); value = a;
        } else value = binary(op.op,a,b,true);
        if (!value.valid) return {};
        work.pop_back();
    }
    return value;
}
} }
