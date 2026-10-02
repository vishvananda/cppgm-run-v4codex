#include "semantic/analyzer.h"
#include "support/builtin_registry.h"
namespace cppgm { namespace semantic {
EntityId Analyzer::runtime_builtin(IdentifierId name)
{
    auto text = ids.spelling(name);
    auto libm = libm_builtin(text);
    auto memory = function_builtin(text);
    if (libm.shape == LibmShape::None && (memory < FunctionBuiltin::Memcpy || memory > FunctionBuiltin::Strpbrk)) return 0;
    auto v = types.fundamental(FT_VOID), i = types.fundamental(FT_INT);
    auto ptr = types.compound(TypeKind::Pointer,v);
    auto cp = types.compound(TypeKind::Pointer,types.qualify(v,1));
    auto size = types.fundamental(FT_UNSIGNED_LONG_INT);
    TypeId result = ptr; std::vector<TypeId> args;
    if (libm.shape != LibmShape::None) {
        using S = LibmShape;
        auto fp = types.fundamental(libm.suffix == 1 ? FT_FLOAT : libm.suffix == 2 ? FT_LONG_DOUBLE : FT_DOUBLE);
        result = fp; args.push_back(fp);
        switch (libm.shape) {
        case S::ComplexUnary: case S::ComplexBinary: case S::ComplexReal: {
            auto complex = types.fundamental(libm.suffix == 1 ? FT_COMPLEX_FLOAT :
                libm.suffix == 2 ? FT_COMPLEX_LONG_DOUBLE : FT_COMPLEX_DOUBLE);
            args[0] = complex;
            if (libm.shape != S::ComplexReal) result = complex;
            if (libm.shape == S::ComplexBinary) args.push_back(complex);
            break;
        }
        case S::Binary: args.push_back(fp); break;
        case S::Ternary: args.push_back(fp); args.push_back(fp); break;
        case S::IntOut: args.push_back(types.compound(TypeKind::Pointer,i)); break;
        case S::IntIn: args.push_back(i); break;
        case S::LongIn: args.push_back(types.fundamental(FT_LONG_INT)); break;
        case S::FloatOut: args.push_back(types.compound(TypeKind::Pointer,fp)); break;
        case S::Quotient: args.push_back(fp); args.push_back(types.compound(TypeKind::Pointer,i)); break;
        case S::Toward: args.push_back(types.fundamental(FT_LONG_DOUBLE)); break;
        case S::IntResult: result = i; break;
        case S::LongResult: result = types.fundamental(FT_LONG_INT); break;
        case S::LongLongResult: result = types.fundamental(FT_LONG_LONG_INT); break;
        default: break;
        }
    } else {
        switch (memory) {
        case FunctionBuiltin::Memcpy: case FunctionBuiltin::Memmove: args = {ptr,cp,size}; break;
        case FunctionBuiltin::Memset: args = {ptr,i,size}; break;
        case FunctionBuiltin::Memcmp: result = i; args = {cp,cp,size}; break;
        case FunctionBuiltin::Memchr: args = {cp,i,size}; break;
        case FunctionBuiltin::Bzero: result = v; args = {ptr,size}; break;
        default: {
            auto c = types.fundamental(FT_CHAR);
            auto string = types.compound(TypeKind::Pointer,types.qualify(c,1));
            args.push_back(string);
            result = memory == FunctionBuiltin::Strlen ? size : types.compound(TypeKind::Pointer,c);
            if (memory != FunctionBuiltin::Strlen) args.push_back(memory == FunctionBuiltin::Strstr || memory == FunctionBuiltin::Strpbrk ? string : i);
            break;
        }
        }
    }
    auto e = declare_function(global,name,0,types.function(result,args,false));
    entities[e].c_linkage = true; entities[e].exception_spec = 129;
    assembler_names.put(e,ids.intern(TextView(text.data+10,text.size-10)));
    if (memory == FunctionBuiltin::Memcpy) entities[e].builtin = Entity::Memcpy;
    if (memory == FunctionBuiltin::Memmove) entities[e].builtin = Entity::Memmove;
    if (memory == FunctionBuiltin::Strlen) entities[e].builtin = Entity::Strlen;
    return e;
}
} }
