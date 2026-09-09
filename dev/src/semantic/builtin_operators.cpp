#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
std::vector<TypeId> Analyzer::builtin_operand_types(NodeId n)
{
    TypeId source = expressions[n].type;
    if (!class_value(source)) return {decay(source)};
    std::vector<TypeId> result;
    Index seen;
    for (EntityId e : conversion_candidates(source)) {
        auto member = members[entities[e].member_info];
        if (member.explicit_constructor || !object_conversion(e,source,expressions[n].category).valid()) continue;
        TypeId t = decay(types[entities[e].type].child);
        if ((!arithmetic(t) && !pointer(t)) || seen.get(t)) continue;
        seen.put(t,1); result.push_back(t);
    }
    return result;
}
void Analyzer::builtin_operators(ETokenType op, const std::vector<NodeId>& args, std::vector<BuiltinOperator>& results)
{
    if (args.empty() || args.size() > 2 || (args.size() == 2 && !args[1])) return;
    Index seen;
    auto add = [&](TypeId a, TypeId b, TypeId type, ValueCategory category = ValueCategory::Prvalue) {
        if (!type || seen.get(key(a,b))) return;
        seen.put(key(a,b),1);
        BuiltinOperator candidate; candidate.type = type; candidate.category = category;
        bool contextual = op == OP_LNOT || op == OP_LAND || op == OP_LOR;
        candidate.arguments[0] = contextual ? boolean_conversion(args[0]) : conversion(args[0],a);
        if (args.size() == 2) candidate.arguments[1] = contextual ? boolean_conversion(args[1]) : conversion(args[1],b);
        if (candidate.arguments[0].valid() && (args.size() == 1 || candidate.arguments[1].valid())) results.push_back(candidate);
    };
    if (op == OP_LNOT || op == OP_LAND || op == OP_LOR) {
        TypeId boolean = types.fundamental(FT_BOOL); add(boolean,boolean,boolean); return;
    }
    auto left = builtin_operand_types(args[0]);
    if (args.size() == 1) {
        for (TypeId t : left) {
            if (op == OP_STAR && pointer(t) && !fundamental(types[t].child,FT_VOID)) add(t,0,types[t].child,ValueCategory::Lvalue);
            else if (op == OP_PLUS && pointer(t)) add(t,0,t);
            else if ((op == OP_PLUS || op == OP_MINUS || op == OP_COMPL) && arithmetic(t) && (op != OP_COMPL || integral(t)))
                add(promote(t),0,promote(t));
        }
        return;
    }
    auto right = builtin_operand_types(args[1]);
    bool equality = op == OP_EQ || op == OP_NE;
    bool comparison = equality || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE;
    for (TypeId a : left) for (TypeId b : right) {
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
            if (equality && pointer(a) && null_constant(args[1])) common = a;
            if (equality && pointer(b) && null_constant(args[0])) common = b;
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
