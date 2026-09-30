#include "semantic/analyzer.h"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::integral(TypeId id) const
{
    const Type& t = types[id];
    if (t.kind == TypeKind::LRef || t.kind == TypeKind::RRef) return integral(t.child);
    if (t.kind == TypeKind::Named) return entities[t.entity].key == KW_ENUM;
    return t.kind == TypeKind::Fundamental && (t.fundamental <= FT_BOOL || t.fundamental == FT_INT128 || t.fundamental == FT_UINT128);
}
bool Analyzer::scoped_enum(TypeId t) const { return types[t].kind == TypeKind::Named && entities[types[t].entity].scoped; }
bool Analyzer::is_unsigned(TypeId id) const
{
    const Type& t = types[id];
    if (t.kind == TypeKind::Named) return is_unsigned(entities[t.entity].underlying);
    return (t.fundamental >= FT_UNSIGNED_CHAR && t.fundamental <= FT_UNSIGNED_LONG_LONG_INT) ||
        t.fundamental == FT_UINT128 || t.fundamental == FT_CHAR16_T || t.fundamental == FT_CHAR32_T || t.fundamental == FT_BOOL;
}
unsigned Analyzer::width(TypeId id) const
{
    const Type& t = types[id];
    if (t.kind == TypeKind::Named) return width(entities[t.entity].underlying);
    return fundamental_width(t.fundamental) * 8;
}
Constant Analyzer::convert(Constant v, TypeId to, bool explicit_cast)
{
    if (!v.valid || !v.type || !to) return Constant();
    if (!calls && (types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef)) to = types[to].child;
    auto target = types[to];
    if (target.kind == TypeKind::LRef || target.kind == TypeKind::RRef) {
        if (types[v.type].kind != TypeKind::LRef && types[v.type].kind != TypeKind::RRef) return Constant();
        if (class_value(target.child)) v.bits = constant_base_address(v.bits,target.child);
        return v.bits ? Constant(to,v.bits) : Constant();
    }
    auto source = types[v.type];
    bool scalar = source.kind == TypeKind::Fundamental && target.kind == TypeKind::Fundamental &&
        (integral(v.type) || floating_type(v.type)) && (integral(to) || floating_type(to));
    if (!scalar) {
        v = constant_indirect(v);
        if (!v.valid) return v;
        if (!integral(v.type) && !floating_type(v.type) && types.unqualified(v.type) == types.unqualified(to)) { v.type = to; return v; }
        if (fundamental(to,FT_VOID)) return Constant(to,0);
        if (types[v.type].kind == TypeKind::MemberPointer) {
            if (fundamental(to,FT_BOOL)) return Constant(to,v.bits != 0);
            if (target.kind == TypeKind::MemberPointer) {
                unsigned added = 0;
                auto owner = entities[types[v.type].entity].type, destination = entities[target.entity].type;
                if (qualification(types[v.type].child,target.child,added)) {
                    std::uint64_t adjustment = 0;
                    if (owner != destination) {
                        if (derived_from(destination,owner)) adjustment = base_adjustments[base_steps(destination,types[v.type].entity)].total;
                        else if (explicit_cast && derived_from(owner,destination)) adjustment = 0-base_adjustments[base_steps(owner,target.entity)].total;
                        else return Constant();
                    }
                    auto value = member_constant_value(v);
                    return member_constant(to,value.member,std::uint64_t(value.adjustment)+adjustment);
                }
            }
            return Constant();
        }
        if (target.kind == TypeKind::MemberPointer)
            return (integral(v.type) || fundamental(v.type,FT_NULLPTR_T)) && !v.bits ? Constant(to,0) : Constant();
        if (pointer(v.type) || fundamental(v.type,FT_NULLPTR_T)) {
            if (fundamental(to,FT_BOOL)) return Constant(to,v.bits != 0);
            if (target.kind == TypeKind::Pointer) {
                if (v.bits && class_value(target.child)) v.bits = constant_base_address(v.bits,target.child);
                return Constant(to,v.bits);
            }
            return Constant();
        }
        if (target.kind == TypeKind::Pointer) return integral(v.type) && !v.bits ? Constant(to,0) : Constant();
    }
    if (floating_type(v.type) || floating_type(to)) return floating_conversion(v,to);
    if (!integral(v.type) || !integral(to)) return Constant();
    if (!explicit_cast && (scoped_enum(v.type) || scoped_enum(to)) && types.unqualified(to) != types.unqualified(v.type))
        throw std::runtime_error("implicit scoped enum conversion");
    auto bits = integer_value(v);
    if (fundamental(to,FT_BOOL)) bits = bits != 0;
    return integer_constant(to,bits);
}
TypeId Analyzer::expression_type(NodeId n, ScopeId s, bool decltype_form)
{
    if (definitions && decltype_form && (closure_functions.get(current_function) || active_template_scope || scopes[s].kind == ScopeKind::Template ||
        (scopes[s].kind == ScopeKind::Block && template_object_context_index.get(s)))) return dependent_decltype(n,s);
    if (calls) {
        if (decltype_form) ++unevaluated_depth;
        Expression e = expression(n, s);
        if (decltype_form) --unevaluated_depth;
        if (decltype_form && (ast[n].kind == Kind::IdExpression || ast[n].kind == Kind::Member) && e.entity) return entities[e.entity].type;
        if (decltype_form && e.category != ValueCategory::Prvalue)
            return types.compound(e.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef, e.type);
        return e.type;
    }
    if (ast[n].kind == Kind::Literal) return types.fundamental(ast.literals[ast[n].literal].type);
    if (ast[n].kind == Kind::KeywordLiteral && ast[n].op == KW_NULLPTR) return types.fundamental(FT_NULLPTR_T);
    if (ast[n].kind == Kind::Parenthesized) {
        TypeId t = expression_type(ast[n].first, s);
        NodeId inner = ast[n].first;
        while (ast[inner].kind == Kind::Parenthesized) inner = ast[inner].first;
        if (decltype_form && ast[inner].kind == Kind::IdExpression) {
            EntityId e = resolve(ast[inner].detail, s);
            if (e && entities[e].kind != EntityKind::Enumerator) return types.compound(TypeKind::LRef, t);
        }
        return t;
    }
    if (ast[n].kind == Kind::IdExpression) {
        EntityId e = resolve(ast[n].detail, s);
        if (!e) throw std::runtime_error("unknown decltype/sizeof name");
        { auto& published = facts.edit(n); published.entity = e; published.type = entities[e].type; }
        return entities[e].type;
    }
    Constant v = evaluate(n, s);
    if (!v.valid) throw std::runtime_error("unsupported type-forming expression");
    return v.type;
}
Constant Analyzer::evaluate(NodeId n, ScopeId s)
{
    if (!n) return Constant();
    if (active_constant) return execute_constant_node(n,s);
    if (definitions && !ast.nodes.occurrences[n].context &&
        template_value_dependence.get(ast.nodes.occurrences[n].source)) return Constant();
    if (calls && !expressions[n].ready) {
        switch (ast[n].kind) {
        case Kind::Literal: case Kind::KeywordLiteral: case Kind::IdExpression: case Kind::Parenthesized:
        case Kind::Member: case Kind::Subscript: case Kind::Call: case Kind::Unary: case Kind::Binary: case Kind::Conditional: case Kind::Cast: case Kind::Sizeof: case Kind::TypeTrait:
            expression(n, s); break;
        default: break;
        }
    }
    if (facts[n].value) return constants[facts[n].value];
    if (calls && expressions[n].form == ExpressionForm::OperatorCall) return constant_indirect(constant_call(n,s));
    Constant result = evaluate_value(n, s);
    facts.edit(n).scope = s;
    if (result.valid) {
        facts.edit(n).value = constants.size();
        if (!calls || !expressions[n].ready) facts.edit(n).type = result.type;
        constants.push_back(result);
    } else facts.edit(n).value = 1; // Expected non-constant, owned by this parsed region.
    return result;
}
Constant Analyzer::evaluate_value(NodeId n, ScopeId s)
{
    ++constant_work;
    // A scalar value request performs lvalue-to-rvalue conversion. Invocation
    // substitution must not erase the parameter's volatile access semantics.
    if (calls && expressions[n].category != ValueCategory::Prvalue && (types[expressions[n].type].cv & 2))
        return Constant();
    NodeId first = ast[n].first;
    if (calls && expressions[n].ready && (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::BracedInit || ast[n].kind == Kind::ParenInitializer) &&
        (class_value(expressions[n].type) || types[expressions[n].type].kind == TypeKind::Array))
        return constant_initialize(n,expressions[n].type,s);
    switch (ast[n].kind) {
    case Kind::Initializer: case Kind::Parenthesized: case Kind::BracedInit: case Kind::ParenInitializer:
        if (!first && (ast[n].kind == Kind::BracedInit || ast[n].kind == Kind::ParenInitializer))
            return convert(Constant(types.fundamental(FT_INT),0),facts[n].type,true);
        return evaluate(first, s);
    case Kind::Literal: {
        const syntax::LiteralValue& literal = ast.literals[ast[n].literal];
        TypeId t = types.fundamental(literal.type);
        if (literal.suffix) {
            if (!calls) return Constant();
            auto kind = literal_call_kind(n);
            auto fn = facts[n].entity;
            if (!fn || !entities[fn].constexpr_function) return Constant();
            if (kind == LiteralCallKind::Pack) return constant_indirect(execute_constant(fn,{},0));
            if (kind != LiteralCallKind::Scalar) return Constant();
            Constant value;
            if (literal.kind == LiteralKind::floating) {
                long double number = 0; std::memcpy(&number,literal.scalar.data(),10);
                value = floating_constant(t,number);
            } else {
                std::uint64_t bits = 0; std::memcpy(&bits,literal.scalar.data(),fundamental_width(literal.type));
                value = convert(Constant(t,bits),t);
            }
            value = convert(value,conversions[expressions[n].conversions].target);
            return constant_indirect(execute_constant(fn,{value},0));
        }
        if ((!integral(t) && !floating_type(t)) || literal.kind == LiteralKind::string) return Constant();
        if (floating_type(t)) {
            long double value = 0;
            if (literal.type == FT_FLOAT) { float f; std::memcpy(&f,literal.scalar.data(),sizeof f); value = f; }
            else if (literal.type == FT_DOUBLE) { double f; std::memcpy(&f,literal.scalar.data(),sizeof f); value = f; }
            else std::memcpy(&value,literal.scalar.data(),sizeof value);
            return floating_constant(t,value);
        }
        std::uint64_t bits = 0;
        std::memcpy(&bits, literal.scalar.data(), fundamental_width(literal.type));
        return convert(Constant(t, bits), t);
    }
    case Kind::KeywordLiteral:
        if (ast[n].op == KW_NULLPTR) return Constant(types.fundamental(FT_NULLPTR_T),0);
        if (ast[n].op == KW_THIS) return active_constant && constant_activations[active_constant].object ?
            Constant(expressions[n].type,constant_activations[active_constant].object) : Constant();
        if (ast[n].op == KW_TRUE || ast[n].op == KW_FALSE)
            return Constant(types.fundamental(FT_BOOL), ast[n].op == KW_TRUE);
        return Constant();
    case Kind::IdExpression: {
        EntityId e = calls ? expressions[n].entity : resolve(ast[n].detail, s);
        if (!e) return Constant();
        if (active_constant && constant_frame) {
            if (auto slot = constant_frame->bindings.get(e)) return constant_indirect(constant_frame->values[slot]);
            if (entities[e].kind == EntityKind::Parameter) return Constant();
        }
        if (active_constant && entities[e].kind == EntityKind::Variable && !entities[e].constant.valid &&
            (types[entities[e].type].cv & 1) && !entities[e].definition)
            constant_unavailable = true;
        if (!calls) { auto& published = facts.edit(n); published.entity = e; published.type = entities[e].type; }
        if (calls && nonstatic_field(e)) return constant_indirect(constant_read(constant_address(n,s)));
        if (calls && types[entities[e].type].kind == TypeKind::Function) return Constant(types.compound(TypeKind::Pointer,entities[e].type),constant_entity_address(e));
        return calls ? constant_indirect(constant_entity_value(e)) : entities[e].constant;
    }
    case Kind::Member: return constant_indirect(constant_read(constant_address(n,s)));
    case Kind::Subscript: {
        if (calls) return constant_indirect(constant_read(constant_address(n,s)));
        auto literal = first, index = ast[first].next;
        if (calls && (types[expressions[literal].type].kind == TypeKind::Array || types[expressions[index].type].kind == TypeKind::Array)) {
            if (types[expressions[literal].type].kind != TypeKind::Array) std::swap(literal,index);
            if (auto plan = constant_array_projection(literal,s)) return constant_array_element(plan,evaluate(index,s));
            literal = first; index = ast[first].next;
        }
        while (ast[literal].kind == Kind::Parenthesized) literal = ast[literal].first;
        if (ast[literal].kind != Kind::Literal || ast.literals[ast[literal].literal].kind != LiteralKind::string) {
            literal = ast[first].next; index = first;
            while (ast[literal].kind == Kind::Parenthesized) literal = ast[literal].first;
        }
        if (ast[literal].kind != Kind::Literal) return Constant();
        return literal_element(ast[literal].literal,evaluate(index,s));
    }
    case Kind::SizeofPack: return constants[query_value(expression_query(n,s))];
    case Kind::Sizeof: case Kind::TypeTrait: {
        if (ast[n].kind == Kind::TypeTrait && ast[n].flags) return constants[query_value(expression_query(n,s))];
        if (ast[n].op == KW_TYPEID) return Constant();
        if (ast[n].op == KW_NOEXCEPT) return constants[facts[n].value];
        TypeId t = ast[first].kind == Kind::TypeId ? type_id(first, s) : expression_type(first, s);
        return Constant(types.fundamental(FT_UNSIGNED_LONG_INT), size(t, ast[n].op == KW_ALIGNOF));
    }
    case Kind::Cast:
        if (calls && expressions[n].count) return constant_node_conversion(ast[first].next,conversions[expressions[n].conversions],s);
        return convert(evaluate(ast[first].next, s), type_id(first, s), true);
    case Kind::Call: {
        if (calls && expressions[n].form != ExpressionForm::Cast) return constant_indirect(constant_call_result(n,s));
        if (!calls || expressions[n].form != ExpressionForm::Cast || (!integral(expressions[n].type) && !floating_type(expressions[n].type) && types[expressions[n].type].kind != TypeKind::MemberPointer)) return Constant();
        auto argument = ast[ast[first].next].first;
        if (argument && expressions[n].count) return constant_node_conversion(argument,conversions[expressions[n].conversions],s);
        return argument ? convert(evaluate(argument,s),expressions[n].type,true) : convert(Constant(types.fundamental(FT_INT),0),expressions[n].type,true);
    }
    case Kind::Conditional: {
        auto cx = calls ? expressions[n] : Expression();
        Constant cond = calls && cx.count ? constant_node_conversion(first,conversions[cx.conversions],s) : evaluate(first, s);
        if (!cond.valid || scoped_enum(cond.type)) return Constant();
        NodeId yes = ast[first].next;
        auto branch = constant_truth(cond) ? yes : ast[yes].next;
        Constant result = calls && cx.count == 3 ? constant_node_conversion(branch,conversions[cx.conversions+(branch == yes ? 1 : 2)],s) : evaluate(branch,s);
        return calls ? convert(result, expressions[n].type) : result;
    }
    case Kind::Assignment: case Kind::Postfix:
        return constant_mutation(n,s);
    case Kind::Unary: {
        if (ast[n].op == OP_INC || ast[n].op == OP_DEC) return constant_mutation(n,s);
        if (ast[n].op == OP_AMP) {
            if (types[expressions[n].type].kind == TypeKind::MemberPointer)
                return member_address_constant(expressions[n].type,expressions[first].entity);
            auto a = constant_address(first,s); return a ? Constant(expressions[n].type,a) : Constant();
        }
        if (ast[n].op == OP_STAR) return constant_indirect(constant_read(constant_address(n,s)));
        Constant v = calls && expressions[n].count ? constant_node_conversion(first,conversions[expressions[n].conversions],s) : evaluate(first, s);
        if (!v.valid || scoped_enum(v.type)) return Constant();
        if (ast[n].op == OP_LNOT) return Constant(types.fundamental(FT_BOOL), !constant_truth(v));
        v = convert(v, calls ? expressions[n].type : promote(v.type));
        if (ast[n].op == OP_PLUS) return v;
        if (ast[n].op == OP_COMPL) return integer_constant(v.type, ~integer_value(v));
        if (ast[n].op == OP_MINUS) return floating_type(v.type) ? floating_constant(v.type,-floating_value(v),true) : binary(OP_MINUS, Constant(v.type, 0), v, true);
        return Constant();
    }
    case Kind::Binary: {
        if (ast[n].op == OP_DOTSTAR || ast[n].op == OP_ARROWSTAR)
            return constant_indirect(constant_read(constant_address(n,s)));
        auto cx = calls ? expressions[n] : Expression();
        Constant a = calls && cx.count == 2 ? constant_node_conversion(first,conversions[cx.conversions],s) : evaluate(first, s);
        if (!a.valid) return Constant();
        ETokenType op = ast[n].op;
        if (op == OP_COMMA) return evaluate(ast[first].next, s);
        if ((op == OP_LAND || op == OP_LOR) && scoped_enum(a.type)) return Constant();
        if (op == OP_LAND && !constant_truth(a)) return Constant(types.fundamental(FT_BOOL), 0);
        if (op == OP_LOR && constant_truth(a)) return Constant(types.fundamental(FT_BOOL), 1);
        Constant b = calls && cx.count == 2 ? constant_node_conversion(ast[first].next,conversions[cx.conversions+1],s) : evaluate(ast[first].next, s);
        if (calls) {
            if (expressions[n].count != 2) throw std::logic_error("missing binary operand conversions");
            std::uint32_t begin = expressions[n].conversions;
            a = convert(a, conversions[begin].target, true);
            b = convert(b, conversions[begin + 1].target, true);
            return binary(op, a, b, true);
        }
        return binary(op, a, b);
    }
    default: return Constant();
    }
}
Constant Analyzer::binary(ETokenType op, Constant a, Constant b, bool converted)
{
    if (!a.valid || !b.valid) return Constant();
    if (types[a.type].kind == TypeKind::MemberPointer && types[b.type].kind == TypeKind::MemberPointer) {
        if (op != OP_EQ && op != OP_NE) return Constant();
        // Members of the same union compare equal; virtual function equality
        // is unspecified and is not a core constant expression in C++11.
        auto av = member_constant_value(a), bv = member_constant_value(b);
        if (a.bits && b.bits && (members[entities[av.member].member_info].virtual_member ||
            members[entities[bv.member].member_info].virtual_member)) return Constant();
        bool same = a.bits == b.bits;
        if (a.bits && b.bits && av.adjustment == bv.adjustment && nonstatic_field(av.member) && nonstatic_field(bv.member) &&
            entities[av.member].owner == entities[bv.member].owner && entities[scopes[entities[av.member].owner].entity].key == KW_UNION) same = true;
        return Constant(types.fundamental(FT_BOOL),op == OP_EQ ? same : !same);
    }
    if (floating_type(a.type) || floating_type(b.type)) return floating_binary(op,a,b,converted);
    if (pointer(a.type) || pointer(b.type) || fundamental(a.type,FT_NULLPTR_T) || fundamental(b.type,FT_NULLPTR_T)) return constant_pointer_binary(op,a,b);
    if (!integral(a.type) || !integral(b.type)) return Constant();
    bool compare = op == OP_EQ || op == OP_NE || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE;
    if (scoped_enum(a.type) || scoped_enum(b.type)) {
        if (!compare || types.unqualified(a.type) != types.unqualified(b.type)) throw std::runtime_error("invalid scoped enum operation");
    }
    TypeId at = types[a.type].kind == TypeKind::Named ? entities[types[a.type].entity].underlying : a.type;
    TypeId bt = types[b.type].kind == TypeKind::Named ? entities[types[b.type].entity].underlying : b.type;
    if (!converted) { at = promote(at); bt = promote(bt); }
    TypeId common = converted ? at : arithmetic_type(at, bt);
    if (op == OP_LSHIFT || op == OP_RSHIFT) common = at;
    a = convert(a, common, true); b = convert(b, (op == OP_LSHIFT || op == OP_RSHIFT) ? bt : common, true);
    bool unsign = is_unsigned(common);
    auto x = integer_value(a), y = integer_value(b);
    WideInteger result = 0;
    auto bits = width(common);
    auto sign = WideInteger(1) << (bits-1);
    auto mask = bits == 128 ? ~WideInteger(0) : (WideInteger(1) << bits)-1;
    bool negative_x = negative_constant(a), negative_y = negative_constant(b);
    bool boolean = compare || op == OP_LAND || op == OP_LOR;
    switch (op) {
    case OP_PLUS: case OP_MINUS: {
        result = op == OP_PLUS ? x+y : x-y;
        if (!unsign) {
            bool negative_result = (result & sign) != 0;
            if (op == OP_PLUS ? (negative_x == negative_y && negative_result != negative_x) :
                (negative_x != negative_y && negative_result != negative_x)) return Constant();
        }
        break;
    }
    case OP_STAR: {
        if (!unsign) {
            auto ax = negative_x ? 0-x : x, ay = negative_y ? 0-y : y;
            auto limit = negative_x != negative_y ? sign : sign-1;
            if (ay && ax > limit/ay) return Constant();
        }
        result = x*y; break;
    }
    case OP_DIV: case OP_MOD:
        if (!y || (!unsign && x == (0-sign) && y == ~WideInteger(0))) return Constant();
        if (unsign) result = op == OP_DIV ? x/y : x%y;
        else {
            auto ax = negative_x ? 0-x : x, ay = negative_y ? 0-y : y;
            result = op == OP_DIV ? ax/ay : ax%ay;
            if (op == OP_DIV ? negative_x != negative_y : negative_x) result = 0-result;
        }
        break;
    case OP_AMP: result = x & y; break;
    case OP_BOR: result = x | y; break;
    case OP_XOR: result = x ^ y; break;
    case OP_EQ: result = x == y; break;
    case OP_NE: result = x != y; break;
    case OP_LT: result = unsign ? x < y : __int128(x) < __int128(y); break;
    case OP_GT: result = unsign ? x > y : __int128(x) > __int128(y); break;
    case OP_LE: result = unsign ? x <= y : __int128(x) <= __int128(y); break;
    case OP_GE: result = unsign ? x >= y : __int128(x) >= __int128(y); break;
    case OP_LAND: result = x && y; break;
    case OP_LOR: result = x || y; break;
    case OP_LSHIFT: case OP_RSHIFT:
        if (negative_y || y >= bits) return Constant();
        if (op == OP_LSHIFT) {
            // C++11 [expr.shift]: signed products may enter the sign bit but
            // must fit the corresponding unsigned type. Unsigned wraps.
            if (!unsign && (negative_x || x > (mask >> unsigned(y)))) return Constant();
            result = x << unsigned(y);
        } else result = unsign ? x >> unsigned(y) : WideInteger(__int128(x) >> unsigned(y));
        break;
    default: return Constant();
    }
    return boolean ? Constant(types.fundamental(FT_BOOL),result != 0) : integer_constant(common,result);
}
} }
