#include "semantic/analyzer.h"
#include "support/builtin_registry.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::direct_intrinsic(EntityId e) const
{
    auto kind = intrinsic_function(e);
    return kind == Intrinsic::Atomic || kind == Intrinsic::Complex || kind == Intrinsic::IsConstantEvaluated ||
        (kind >= Intrinsic::SourceFile && kind <= Intrinsic::SourceColumn) ||
        (kind >= Intrinsic::VectorInit && kind <= Intrinsic::VectorShuffle);
}
ExpressionForm Analyzer::intrinsic_expression(EntityId e) const
{
    switch (intrinsic_function(e)) {
    case Intrinsic::Expect: return ExpressionForm::Expect;
    case Intrinsic::Abort: return ExpressionForm::Abort;
    case Intrinsic::Unreachable: return ExpressionForm::Unreachable;
    default: return ExpressionForm::Ordinary;
    }
}
EntityId Analyzer::builtin_function(IdentifierId name)
{
    auto text = ids.spelling(name);
    if (text.equals("__builtin_shuffle")) {
        auto e = declare_function(global,name,0,types.function(types.fundamental(FT_VOID),{},false));
        entities[e].exception_spec = 129; intrinsic_functions.put(e,unsigned(Intrinsic::VectorShuffle)); return e;
    }
    if (auto id = x86_builtin(text)) {
        auto d = x86_builtin(id);
        auto type = [&](X86Type t) {
            using T = X86Type;
            auto lane = types.fundamental(t == T::Void || t == T::ConstVoidPtr ? FT_VOID : t == T::U32 ? FT_UNSIGNED_INT : t == T::U64 || t == T::ULongPtr ? FT_UNSIGNED_LONG_LONG_INT : t == T::I16 ? FT_SHORT_INT :
                t == T::I64 || t == T::Long2 || t == T::Long1 || t == T::Long2Ptr || t == T::LongPtr ? FT_LONG_LONG_INT :
                t == T::Float4 || t == T::FloatPtr || t == T::Float2Ptr || t == T::ConstFloat2Ptr ? FT_FLOAT :
                t == T::Double2 || t == T::DoublePtr ? FT_DOUBLE : t == T::Byte8 || t == T::Byte16 || t == T::ConstCharPtr ? FT_CHAR :
                t == T::Short4 || t == T::Short8 ? FT_SHORT_INT : FT_INT);
            if ((t >= T::Float4 && t <= T::ConstFloat2Ptr) || t == T::Long2Ptr || t == T::Int2Ptr)
                lane = types.compound(TypeKind::Vector,lane,t == T::Int2 || t == T::Long1 || t == T::Byte8 || t == T::Short4 ||
                    t == T::Float2Ptr || t == T::ConstFloat2Ptr || t == T::Int2Ptr ? 8 : 16);
            if (t >= T::Float2Ptr) {
                if (t == T::ConstFloat2Ptr || t == T::ConstVoidPtr || t == T::ConstCharPtr) lane = types.qualify(lane,1);
                lane = types.compound(TypeKind::Pointer,lane);
            }
            return lane;
        };
        std::vector<TypeId> args;
        if (d.first != X86Type::Void) args.push_back(type(d.first));
        if (d.second != X86Type::Void) args.push_back(type(d.second));
        if (d.form == X86Form::Shuffle || d.form == X86Form::Insert || d.form == X86Form::DynamicCompare) args.push_back(types.fundamental(FT_INT));
        if (d.form == X86Form::MaskStore) args.push_back(types.compound(TypeKind::Pointer,types.fundamental(FT_CHAR)));
        auto e = declare_function(global,name,0,types.function(type(d.result),args,false));
        entities[e].exception_spec = 129; intrinsic_functions.put(e,unsigned(Intrinsic::X86));
        x86_builtins.put(e,id); return e;
    }
    if (auto id = packed_builtin(text)) {
        auto d = packed_builtin(id);
        auto lane = [&](unsigned bytes) { return types.fundamental(bytes == 1 ? FT_CHAR : bytes == 2 ? FT_SHORT_INT : bytes == 4 ? FT_INT : FT_LONG_LONG_INT); };
        auto input = types.compound(TypeKind::Vector,lane(d.lane),d.bytes);
        auto output = types.compound(TypeKind::Vector,lane(d.result_lane),d.bytes);
        auto e = declare_function(global,name,0,types.function(output,{input,d.immediate ? types.fundamental(FT_INT) : input},false));
        entities[e].exception_spec = 129; intrinsic_functions.put(e,unsigned(Intrinsic::Packed));
        packed_builtins.put(e,id); return e;
    }
    auto vector = fixed_vector_builtin(text);
    if (vector.lane_bytes) {
        auto lane = types.fundamental(vector.lane_bytes == 1 ? FT_SIGNED_CHAR :
            vector.lane_bytes == 2 ? FT_SHORT_INT : FT_INT);
        auto packed = types.compound(TypeKind::Vector,lane,8);
        std::vector<TypeId> args(8/vector.lane_bytes,lane);
        if (vector.extract) args = {packed,types.fundamental(FT_INT)};
        auto e = declare_function(global,name,0,types.function(vector.extract ? lane : packed,args,false));
        entities[e].exception_spec = 129;
        intrinsic_functions.put(e,unsigned(vector.extract ? Intrinsic::VectorExtract : Intrinsic::VectorInit));
        return e;
    }
    auto builtin = function_builtin(text);
    if (builtin == FunctionBuiltin::OperatorNew || builtin == FunctionBuiltin::OperatorDelete) {
        auto e = global_allocation(builtin == FunctionBuiltin::OperatorNew ? KW_NEW : KW_DELETE,false);
        bind(global,name,e); return e;
    }
    if (builtin >= FunctionBuiltin::AddOverflow && builtin <= FunctionBuiltin::MulOverflow) {
        auto e = declare_function(global,name,0,types.function(types.fundamental(FT_BOOL),{},false));
        auto kind = builtin == FunctionBuiltin::AddOverflow ? Intrinsic::AddOverflow :
            builtin == FunctionBuiltin::SubOverflow ? Intrinsic::SubOverflow : Intrinsic::MulOverflow;
        entities[e].exception_spec = 129; intrinsic_functions.put(e,unsigned(kind)); return e;
    }
    if (builtin == FunctionBuiltin::Complex) {
        auto e = declare_function(global,name,0,types.function(types.fundamental(FT_VOID),{},false));
        entities[e].exception_spec = 129; intrinsic_functions.put(e,unsigned(Intrinsic::Complex)); return e;
    }
    if (builtin == FunctionBuiltin::IsConstantEvaluated) {
        evaluation_context_present = true;
        auto e = declare_function(global,name,0,types.function(types.fundamental(FT_BOOL),{},false));
        entities[e].exception_spec = 129;
        intrinsic_functions.put(e,unsigned(Intrinsic::IsConstantEvaluated)); return e;
    }
    if (builtin >= FunctionBuiltin::SourceFile && builtin <= FunctionBuiltin::SourceColumn) {
        source_builtins_present = true;
        auto kind = Intrinsic(unsigned(Intrinsic::SourceFile) + unsigned(builtin) - unsigned(FunctionBuiltin::SourceFile));
        auto result = kind == Intrinsic::SourceLine || kind == Intrinsic::SourceColumn ?
            types.fundamental(FT_UNSIGNED_INT) : types.compound(TypeKind::Pointer,types.qualify(types.fundamental(FT_CHAR),1));
        auto e = declare_function(global,name,0,types.function(result,{},false));
        entities[e].exception_spec = 129;
        intrinsic_functions.put(e,unsigned(kind)); return e;
    }
    if (auto hint = hint_builtin(name)) return hint;
    if (auto runtime = runtime_builtin(name)) return runtime;
    if (auto integer = integer_builtin_function(name)) return integer;
    auto atomic = atomic_builtin(text);
    if (atomic.op != AtomicOp::None) {
        auto e = declare_function(global,name,0,types.function(types.fundamental(FT_VOID),{},false));
        entities[e].exception_spec = 129;
        intrinsic_functions.put(e,unsigned(Intrinsic::Atomic));
        atomic_kinds.put(e,atomic.code()); return e;
    }
    if ((builtin == FunctionBuiltin::Strcmp) || (builtin == FunctionBuiltin::Strncmp)) {
        auto cp = types.compound(TypeKind::Pointer,types.qualify(types.fundamental(FT_CHAR),1));
        std::vector<TypeId> args{cp,cp};
        if ((builtin == FunctionBuiltin::Strncmp)) args.push_back(types.fundamental(FT_UNSIGNED_LONG_INT));
        auto e = declare_function(global,name,0,types.function(types.fundamental(FT_INT),args,false));
        entities[e].c_linkage = true; entities[e].exception_spec = 129;
        assembler_names.put(e,ids.intern(TextView(text.data+10,text.size-10)));
        return e;
    }
    auto kind = (builtin == FunctionBuiltin::VaStart) ? Intrinsic::VaStart :
        (builtin == FunctionBuiltin::VaEnd) ? Intrinsic::VaEnd :
        (builtin == FunctionBuiltin::VaCopy) ? Intrinsic::VaCopy :
        (builtin == FunctionBuiltin::Alloca) ? Intrinsic::StackAlloc :
        (builtin == FunctionBuiltin::Expect) ? Intrinsic::Expect :
        // GNU trap permits abnormal termination through abort. Reuse its
        // typed no-unwind/no-return call, including retained template queries.
        (builtin == FunctionBuiltin::Abort || builtin == FunctionBuiltin::Trap) ? Intrinsic::Abort :
        (builtin == FunctionBuiltin::Unreachable) ? Intrinsic::Unreachable : Intrinsic::None;
    bool bounded = (builtin == FunctionBuiltin::Vsnprintf), print = (builtin == FunctionBuiltin::Vsprintf);
    auto absolute = (builtin == FunctionBuiltin::Fabs) ? FT_DOUBLE : (builtin == FunctionBuiltin::Fabsf) ? FT_FLOAT :
        (builtin == FunctionBuiltin::Fabsl) ? FT_LONG_DOUBLE :
        (builtin == FunctionBuiltin::Abs) ? FT_INT : (builtin == FunctionBuiltin::Labs) ? FT_LONG_INT :
        (builtin == FunctionBuiltin::Llabs) ? FT_LONG_LONG_INT : FT_VOID;
    if (absolute != FT_VOID) {
        auto t = types.fundamental(absolute);
        auto e = declare_function(global,name,0,types.function(t,{t},false));
        entities[e].c_linkage = true; entities[e].exception_spec = 129;
        assembler_names.put(e,ids.intern(TextView(text.data+10,text.size-10))); return e;
    }
    if (kind == Intrinsic::None && !bounded && !print) return 0;
    auto v = types.fundamental(FT_VOID), size = types.fundamental(FT_UNSIGNED_LONG_INT);
    auto va = types.adjusted(variadic_list_type);
    auto ret = v; std::vector<TypeId> args;
    if (kind == Intrinsic::Expect) {
        ret = types.fundamental(FT_LONG_INT); args = {ret,ret};
    } else if (kind == Intrinsic::StackAlloc) { args.push_back(size); ret = types.compound(TypeKind::Pointer,v); }
    else if (kind == Intrinsic::VaStart || kind == Intrinsic::VaEnd || kind == Intrinsic::VaCopy) {
        args.push_back(va);
        if (kind == Intrinsic::VaCopy) args.push_back(va);
    } else if (kind == Intrinsic::None) {
        auto c = types.fundamental(FT_CHAR);
        args.push_back(types.compound(TypeKind::Pointer,c));
        if (bounded) args.push_back(size);
        args.push_back(types.compound(TypeKind::Pointer,types.qualify(c,1))); args.push_back(va);
        ret = types.fundamental(FT_INT);
    }
    auto e = declare_function(global,name,0,types.function(ret,args,kind == Intrinsic::VaStart));
    entities[e].c_linkage = true; entities[e].exception_spec = 129;
    if (kind != Intrinsic::None) intrinsic_functions.put(e,unsigned(kind));
    else assembler_names.put(e,ids.intern(bounded ? TextView("vsnprintf",9) : TextView("vsprintf",8)));
    return e;
}
unsigned Analyzer::intrinsic_immediate_index(EntityId e)
{
    auto kind = intrinsic_function(e);
    if (kind == Intrinsic::VectorExtract) return 1;
    if (kind != Intrinsic::X86) return 0;
    auto form = x86_builtin(x86_intrinsic(e)).form;
    return form == X86Form::Extract || form == X86Form::Insert || form == X86Form::Shuffle ||
        form == X86Form::ShuffleOne || form == X86Form::DynamicCompare || form == X86Form::ByteShift ? types[entities[e].type].count-1 : 0;
}
bool Analyzer::valid_intrinsic_immediate(EntityId e, Constant value)
{
    if (!value.valid || !integral(value.type) || negative_constant(value)) return false;
    auto kind = intrinsic_function(e);
    auto form = kind == Intrinsic::X86 ? x86_builtin(x86_intrinsic(e)).form : X86Form::Extract;
    auto vector = types.parameters[types[entities[e].type].offset];
    auto limit = form == X86Form::Extract || form == X86Form::Insert ? vector_elements(vector)-1 :
        form == X86Form::DynamicCompare ? 31 : form == X86Form::ByteShift ? 2040 : 255;
    return integer_value(value) <= limit && (form != X86Form::ByteShift || !(integer_value(value)&7));
}
void Analyzer::validate_intrinsic(EntityId e, const std::vector<NodeId>& args, ScopeId s)
{
    auto kind = intrinsic_function(e);
    if (auto index = intrinsic_immediate_index(e))
        if (!valid_intrinsic_immediate(e,evaluate(args[index],s)))
            throw std::runtime_error("invalid intrinsic immediate operand");
    if (kind == Intrinsic::Prefetch) {
        for (unsigned j = 1; j < args.size(); ++j) {
            auto value = evaluate(args[j],s);
            if (!value.valid || !integral(value.type) || negative_constant(value) || integer_value(value) > (j == 1 ? 1u : 3u))
                throw std::runtime_error("invalid prefetch hint");
        }
    }
    if (kind != Intrinsic::VaStart) return;
    if (args.size() != 2) throw std::runtime_error("va_start takes two arguments");
    EntityId fn = 0;
    for (auto scope = s; scope; scope = scopes[scope].parent)
        if (scopes[scope].kind == ScopeKind::Function) { fn = scopes[scope].entity; break; }
    if (!fn || !types[entities[fn].type].variadic)
        throw std::runtime_error("va_start requires a variadic function");
    auto last = args[1];
    while (ast.kind(last) == syntax::Kind::Parenthesized) last = ast.first(last);
    auto parameter = expressions[last].entity;
    if (ast.kind(last) != syntax::Kind::IdExpression || entities[parameter].kind != EntityKind::Parameter)
        throw std::runtime_error("va_start requires a named parameter");
    auto owner = entities[parameter].owner;
    if (scopes[owner].kind != ScopeKind::Function || scopes[owner].entity != fn)
        throw std::runtime_error("va_start parameter belongs to another function");
    auto ordinal = signature_parameters.get(parameter);
    if (ordinal) {
        if (ordinal != types[entities[fn].type].count) throw std::runtime_error("va_start requires the last parameter");
    } else {
        EntityId final = 0;
        for (auto d = scopes[owner].first_decl; d; d = declarations[d].next)
            if (entities[declarations[d].entity].kind == EntityKind::Parameter) final = declarations[d].entity;
        if (parameter != final) throw std::runtime_error("va_start requires the last parameter");
    }
}
TypeQueryFact Analyzer::query_builtin_operand(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    TypeQueryFact result;
    if (q.kind == QueryKind::ValueBuiltin) {
        result.expression.type = builtin_value_type(q.value,q.type,children[0].expression.type);
        if (!result.expression.type) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    } else if (q.kind == QueryKind::Typeof) {
        result.expression.type = children[0].expression.type;
        if (!result.expression.type && children[0].declared_type)
            result.expression.type = value_type(children[0].declared_type);
    } else {
        result.expression.type = q.type;
        if (!standard_conversion(children[0].expression,types.adjusted(variadic_list_type)).valid())
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    }
    return result;
}
EntityId Analyzer::predefined_function_name(NodeId n, ScopeId s)
{
    return function_name_string(function_context(s),ast.text(n),n);
}
Expression Analyzer::va_arg_expression(NodeId n, ScopeId s)
{
    auto arg = ast.first(n);
    expression(arg,s);
    auto c = conversion(arg,types.adjusted(variadic_list_type));
    if (!c.valid()) throw std::runtime_error("va_arg requires va_list");
    Expression result; result.type = type_id(ast.next(arg),s);
    auto type = types[result.type];
    if (class_value(result.type) || type.kind == TypeKind::Array || type.kind == TypeKind::Function ||
        type.kind == TypeKind::LRef || type.kind == TypeKind::RRef || fundamental(result.type,FT_VOID))
        throw std::runtime_error("unsupported va_arg result type");
    record_conversion(result,arg,c); return result;
}
} }
