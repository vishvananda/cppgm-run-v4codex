#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::check_pointer_arithmetic(ETokenType op, TypeId left, TypeId right)
{
    if (op != OP_PLUS && op != OP_MINUS && op != OP_LSQUARE) return;
    auto type = pointer(left) ? left : pointer(right) ? right : 0;
    if (type) size(types[type].child);
}
std::vector<TypeId> Analyzer::builtin_operand_types(NodeId n)
{
    return builtin_operand_types_value(expressions[n]);
}
std::vector<TypeId> Analyzer::builtin_operand_types_value(Expression expression)
{
    TypeId source = expression.type;
    if (!source) return {};
    if (!class_value(source)) return {decay(source)};
    if (definitions) complete_class(types[source].entity);
    std::vector<TypeId> result;
    Index seen;
    for (EntityId e : conversion_candidates(source)) {
        auto member = members[entities[e].member_info];
        if (member.explicit_constructor || !object_conversion(e,source,expression.category).valid()) continue;
        TypeId t = decay(types[entities[e].type].child);
        if ((!arithmetic(t) && !pointer(t)) || seen.get(t)) continue;
        seen.put(t,1); result.push_back(t);
    }
    return result;
}
void Analyzer::builtin_operators(ETokenType op, const std::vector<NodeId>& args, std::vector<BuiltinOperator>& results)
{
    if (args.empty() || args.size() > 2 || (args.size() == 2 && !args[1])) return;
    std::vector<Expression> values;
    for (auto n : args) values.push_back(expressions[n]);
    builtin_operators_values(op,values,results,&args);
}
void Analyzer::builtin_operators_values(ETokenType op, const std::vector<Expression>& args, std::vector<BuiltinOperator>& results, const std::vector<NodeId>* nodes)
{
    if (args.empty() || args.size() > 2) return;
    auto node = [&](unsigned i) { return nodes ? (*nodes)[i] : 0; };
    auto null = [&](unsigned i) { return nodes ? null_constant((*nodes)[i]) :
        args[i].null_pointer_constant || fundamental(args[i].type,FT_NULLPTR_T); };
    auto convert_argument = [&](unsigned i, TypeId target) {
        return nodes ? conversion(node(i),target) : conversion_value(args[i],target);
    };
    Index seen;
    auto add = [&](TypeId a, TypeId b, TypeId type, ValueCategory category = ValueCategory::Prvalue) {
        if (!type || seen.get(key(a,b))) return;
        seen.put(key(a,b),1);
        BuiltinOperator candidate; candidate.type = type; candidate.category = category;
        bool contextual = op == OP_LNOT || op == OP_LAND || op == OP_LOR;
        candidate.arguments[0] = contextual ? boolean_conversion_value(args[0],node(0)) : convert_argument(0,a);
        if (args.size() == 2) candidate.arguments[1] = contextual ? boolean_conversion_value(args[1],node(1)) : convert_argument(1,b);
        if (candidate.arguments[0].valid() && (args.size() == 1 || candidate.arguments[1].valid())) results.push_back(candidate);
    };
    if (op == OP_LNOT || op == OP_LAND || op == OP_LOR) {
        TypeId boolean = types.fundamental(FT_BOOL); add(boolean,boolean,boolean); return;
    }
    auto left = builtin_operand_types_value(args[0]);
    if (op == OP_INC || op == OP_DEC) {
        for (TypeId t : left) {
            if (types[t].kind != TypeKind::Named && (arithmetic(t) || object_pointer(t)) &&
                !(op == OP_DEC && fundamental(t,FT_BOOL))) {
                if (object_pointer(t)) size(types[t].child);
                auto target = class_value(args[0].type) ? t : args[0].type;
                if (!(types[target].cv & 1))
                    add(types.compound(TypeKind::LRef,target),types.fundamental(FT_INT),
                        args.size() == 1 ? target : types.unqualified(target),
                        args.size() == 1 ? ValueCategory::Lvalue : ValueCategory::Prvalue);
            }
        }
        return;
    }
    if (args.size() == 1) {
        for (TypeId t : left) {
            if (op == OP_STAR && pointer(t) && !fundamental(types[t].child,FT_VOID)) add(t,0,types[t].child,ValueCategory::Lvalue);
            else if (op == OP_PLUS && pointer(t)) add(t,0,t);
            else if ((op == OP_PLUS || op == OP_MINUS || op == OP_COMPL) && arithmetic(t) && (op != OP_COMPL || integral(t)))
                add(promote(t),0,promote(t));
        }
        return;
    }
    auto right = builtin_operand_types_value(args[1]);
    bool equality = op == OP_EQ || op == OP_NE;
    bool comparison = equality || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE;
    for (TypeId a : left) for (TypeId b : right) {
        if (op == OP_QMARK) {
            TypeId common = arithmetic(a) && arithmetic(b) ? arithmetic_type(a,b) :
                pointer(a) && pointer(b) ? composite_pointer(a,b) :
                pointer(a) && null(1) ? a : pointer(b) && null(0) ? b : 0;
            if (common) add(common,common,common);
            continue;
        }
        bool ap = object_pointer(a), bp = object_pointer(b);
        if (op == OP_LSQUARE) {
            if (ap && integral(b) && !scoped_enum(b)) add(a,promote(b),types[a].child,ValueCategory::Lvalue);
            if (bp && integral(a) && !scoped_enum(a)) add(promote(a),b,types[b].child,ValueCategory::Lvalue);
            continue;
        }
        if (op == OP_PLUS || op == OP_MINUS) {
            if (ap && integral(b) && !scoped_enum(b)) add(a,promote(b),a);
            if (op == OP_PLUS && bp && integral(a) && !scoped_enum(a)) add(promote(a),b,b);
            if (op == OP_MINUS && ap && bp && types.unqualified(types[a].child) == types.unqualified(types[b].child)) {
                TypeId common = composite_pointer(a,b); add(common,common,types.fundamental(FT_LONG_INT));
            }
        }
        if (comparison) {
            TypeId common = pointer(a) && pointer(b) ? composite_pointer(a,b) : 0;
            if (equality && pointer(a) && null(1)) common = a;
            if (equality && pointer(b) && null(0)) common = b;
            if (common && (equality || types[types[common].child].kind != TypeKind::Function)) add(common,common,types.fundamental(FT_BOOL));
        }
        bool shift = op == OP_LSHIFT || op == OP_RSHIFT;
        bool integer = shift || op == OP_MOD || op == OP_AMP || op == OP_BOR || op == OP_XOR;
        bool number = comparison || integer || op == OP_PLUS || op == OP_MINUS || op == OP_STAR || op == OP_DIV;
        if (number && arithmetic(a) && arithmetic(b) && (!integer || (integral(a) && integral(b)))) {
            TypeId common = arithmetic_type(a,b);
            add(shift ? promote(a) : common,shift ? promote(b) : common,comparison ? types.fundamental(FT_BOOL) : shift ? promote(a) : common);
        }
    }
}
} }
