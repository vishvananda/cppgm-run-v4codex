#include "semantic/analyzer.h"
#include "support/type_traits.h"
namespace cppgm { namespace semantic {
bool Analyzer::atomic_operand(TypeId id)
{
    auto t = types[id];
    if ((t.kind == TypeKind::Fundamental && bit_integer_kind(t.fundamental)) || t.cv || t.kind == TypeKind::Array || t.kind == TypeKind::Function ||
        t.kind == TypeKind::LRef || t.kind == TypeKind::RRef || fundamental(id,FT_VOID)) return false;
    return dependent_type(id) || builtin_type_property(unsigned(BuiltinTrait::TriviallyCopyable),id);
}
EntityId Analyzer::atomic_signature(EntityId family, TypeId operand)
{
    auto kind = atomic_kind(family);
    auto op = kind.op;
    auto v = types.fundamental(FT_VOID), i = types.fundamental(FT_INT), b = types.fundamental(FT_BOOL);
    TypeId target = 0;
    if (!kind.fence() && op != AtomicOp::LockFree) {
        operand = decay(operand);
        if (!pointer(operand)) return 0;
        target = types[operand].child;
        if (op != AtomicOp::Load && (types[target].cv & 1)) return 0;
        if (kind.form == AtomicForm::C11 && !(types[target].cv & 4)) return 0;
        if (kind.form != AtomicForm::C11 && (types[target].cv & 4)) return 0;
        target = types.non_atomic(types.unqualified(target));
        if (op == AtomicOp::TestSet || (op == AtomicOp::Clear && !kind.sync())) target = types.fundamental(FT_UNSIGNED_CHAR);
        if (kind.form != AtomicForm::Generic && kind.form != AtomicForm::C11 &&
            !pointer(target) && (types[target].kind != TypeKind::Fundamental || !integral(target))) return 0;
        if (kind.arithmetic() && ((!pointer(target) && !integral(target)) || fundamental(target,FT_BOOL))) return 0;
        if (types[target].kind == TypeKind::Array || types[target].kind == TypeKind::Function || fundamental(target,FT_VOID)) return 0;
        if (!builtin_type_property(unsigned(BuiltinTrait::TriviallyCopyable),target)) return 0;
        auto bytes = size(target,false,true);
        if (!bytes) return 0;
        if (kind.form != AtomicForm::Generic && kind.form != AtomicForm::C11 && (bytes > 16 || (bytes & (bytes-1)))) return 0;
    }
    auto identity = key(kind.code(),target);
    if (auto old = atomic_signatures.get(identity)) return old;
    auto ret = target ? target : v;
    std::vector<TypeId> args;
    if (kind.fence()) {
        if (!kind.sync()) args.push_back(i);
    } else if (op == AtomicOp::LockFree) {
        ret = b; args.push_back(types.fundamental(FT_UNSIGNED_LONG_INT));
        if (kind.form != AtomicForm::C11) args.push_back(types.compound(TypeKind::Pointer,types.qualify(v,3)));
    } else {
        auto ptr = [&](TypeId t, unsigned cv) { return types.compound(TypeKind::Pointer,types.qualify(t,cv)); };
        args.push_back(ptr(target,(op == AtomicOp::Load ? 3 : 2) | (kind.form == AtomicForm::C11 ? 4 : 0)));
        if (op == AtomicOp::TestSet || (op == AtomicOp::Clear && !kind.sync())) args[0] = ptr(v,2);
        if (op == AtomicOp::Load) {
            if (kind.form == AtomicForm::Generic) { args.push_back(ptr(target,0)); ret = v; }
        } else if (op == AtomicOp::Clear) ret = v;
        else if (op == AtomicOp::TestSet) ret = b;
        else if (op == AtomicOp::Compare) {
            args.push_back(kind.sync() ? target : ptr(target,0));
            args.push_back(kind.form == AtomicForm::Generic ? ptr(target,1) : target);
            ret = kind.form == AtomicForm::SyncValue ? target : b;
            if (!kind.sync()) {
                if (kind.form != AtomicForm::C11) args.push_back(b);
                args.push_back(i); // success order; failure order is appended below
            }
        } else {
            auto delta = pointer(target) && kind.arithmetic() ? types.fundamental(FT_LONG_INT) : target;
            args.push_back(kind.form == AtomicForm::Generic ? ptr(target,1) : delta);
            if (kind.form == AtomicForm::Generic && op == AtomicOp::Exchange) args.push_back(ptr(target,0));
            if (op == AtomicOp::Store || op == AtomicOp::Init || kind.form == AtomicForm::Generic) ret = v;
        }
        if (!kind.sync() && op != AtomicOp::Init) args.push_back(i);
    }
    auto e = make_entity(EntityKind::Function,global,entities[family].name,0);
    entities[e].type = types.function(ret,args,kind.sync() && !kind.fence());
    entities[e].exception_spec = 129;
    intrinsic_functions.put(e,unsigned(Intrinsic::Atomic)); atomic_kinds.put(e,kind.code());
    atomic_signatures.put(identity,e); return e;
}
Constant Analyzer::atomic_constant(NodeId n, ScopeId s)
{
    auto kind = atomic_kind(facts[n].entity);
    if (kind.op != AtomicOp::LockFree) return {};
    auto call = expressions[n];
    auto size = constant_node_conversion(call_argument(call,0),conversions[call.conversions],s);
    if (!size.valid) return {};
    if (kind.form != AtomicForm::Always && call.argument_count > 1 &&
        !constant_node_conversion(call_argument(call,1),conversions[call.conversions+1],s).valid) return {};
    auto bytes = integer_value(size);
    // General GNU atomics cannot promise 16-byte alignment from size alone.
    return Constant(call.type,bytes && bytes <= (kind.form == AtomicForm::C11 ? 16 : 8) && !(bytes & (bytes-1)));
}
} }
