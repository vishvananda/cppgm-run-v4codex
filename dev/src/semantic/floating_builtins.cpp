#include "semantic/analyzer.h"
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace cppgm { namespace semantic {
bool Analyzer::floating_builtin(NodeId n, ScopeId scope, IdentifierId name,
    const std::vector<NodeId>& args, Expression& result)
{
    auto text = ids.spelling(name);
    EFundamentalType type = FT_VOID;
    bool nan = false;
    const char* const generators[] = {"__builtin_nan", "__builtin_nanf", "__builtin_nanl",
        "__builtin_inf", "__builtin_inff", "__builtin_infl",
        "__builtin_huge_val", "__builtin_huge_valf", "__builtin_huge_vall"};
    const EFundamentalType types_by_suffix[] = {FT_DOUBLE,FT_FLOAT,FT_LONG_DOUBLE};
    for (unsigned i = 0; i < 9; ++i) if (text.equals(generators[i])) {
        type = types_by_suffix[i%3]; nan = i < 3; break;
    }
    if (type != FT_VOID) {
        if (args.size() != unsigned(nan)) throw std::runtime_error("floating constant builtin arity");
        std::uint64_t payload = 0;
        if (nan) {
            auto arg = args[0];
            while (ast[arg].kind == syntax::Kind::Parenthesized) arg = ast[arg].first;
            const auto lit = ast.literals[ast[arg].literal];
            if (ast[arg].kind != syntax::Kind::Literal || lit.kind != LiteralKind::string || lit.type != FT_CHAR)
                throw std::runtime_error("NaN payload requires a narrow string literal");
            auto bytes = ast.literal_bytes.data()+lit.offset;
            unsigned base = 10, at = 0;
            if (lit.elements > 1 && bytes[0] == '0') {
                base = 8; at = 1;
                if (lit.elements > 2 && (bytes[1] == 'x' || bytes[1] == 'X')) { base = 16; at = 2; }
            }
            for (; at+1 < lit.elements; ++at) {
                unsigned digit = bytes[at] >= '0' && bytes[at] <= '9' ? bytes[at]-'0' :
                    bytes[at] >= 'a' && bytes[at] <= 'f' ? bytes[at]-'a'+10 :
                    bytes[at] >= 'A' && bytes[at] <= 'F' ? bytes[at]-'A'+10 : 255;
                if (digit >= base) { payload = 0; break; }
                payload = payload*base+digit;
            }
        }
        long double value = std::numeric_limits<long double>::infinity();
        if (nan && type == FT_FLOAT) {
            std::uint32_t bits = 0x7fc00000U | (payload & 0x3fffffU);
            float v; std::memcpy(&v,&bits,4); value = v;
        } else if (nan && type == FT_DOUBLE) {
            std::uint64_t bits = 0x7ff8000000000000ULL | (payload & 0x7ffffffffffffULL);
            double v; std::memcpy(&v,&bits,8); value = v;
        } else if (nan) {
            std::uint64_t bits = 0xc000000000000000ULL | (payload & 0x3fffffffffffffffULL);
            std::uint16_t exponent = 0x7fff;
            std::memcpy(&value,&bits,8); std::memcpy(reinterpret_cast<char*>(&value)+8,&exponent,2);
        }
        result.type = types.fundamental(type); result.form = ExpressionForm::ConstantQuery;
        facts.edit(n).value = constants.size(); constants.push_back(floating_constant(result.type,value,true));
        return true;
    }
    auto form = text.equals("__builtin_isfinite") ? ExpressionForm::FloatFinite :
        text.equals("__builtin_isnan") || text.equals("__builtin_isnanf") || text.equals("__builtin_isnanl") ? ExpressionForm::FloatNaN :
        text.equals("__builtin_isinf") ? ExpressionForm::FloatInfinite :
        text.equals("__builtin_isnormal") ? ExpressionForm::FloatNormal :
        text.equals("__builtin_fpclassify") ? ExpressionForm::FloatClassify : ExpressionForm::Ordinary;
    if (form == ExpressionForm::Ordinary) return false;
    if (args.size() != (form == ExpressionForm::FloatClassify ? 6U : 1U)) throw std::runtime_error("floating builtin arity");
    auto t = expressions[args.back()].type;
    if (!floating_type(t)) throw std::runtime_error("floating builtin argument type");
    std::vector<Conversion> chosen;
    for (unsigned i = 0; i < args.size(); ++i) {
        auto c = conversion(args[i],i+1 == args.size() ? t : types.fundamental(FT_INT));
        if (!c.valid()) throw std::runtime_error("floating classification result type");
        chosen.push_back(c);
    }
    record_call(result,args,chosen);
    result.type = types.fundamental(FT_INT); result.form = form;
    auto constant = evaluate(args.back(),scope);
    if (constant.valid) {
        auto x = floating_value(constant);
        auto minimum = fundamental(t,FT_FLOAT) ? std::numeric_limits<float>::min() :
            fundamental(t,FT_DOUBLE) ? std::numeric_limits<double>::min() : std::numeric_limits<long double>::min();
        bool normal = std::isfinite(x) && std::fabs(x) >= minimum;
        auto value = form == ExpressionForm::FloatClassify ? convert(evaluate(args[std::isnan(x) ? 0 :
            std::isinf(x) ? 1 : normal ? 2 : x == 0 ? 4 : 3],scope),result.type) :
            Constant(result.type,form == ExpressionForm::FloatNaN ? std::isnan(x) :
                form == ExpressionForm::FloatFinite ? std::isfinite(x) :
                form == ExpressionForm::FloatInfinite ? std::isinf(x) : normal);
        if (value.valid) { facts.edit(n).value = constants.size(); constants.push_back(value); }
    }
    return true;
}
} }
