#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::bind_template_size(NodeId node, ScopeId scope)
{
    auto source = ast.nodes.occurrences[node].source;
    if (template_value_queries.get(source)) return true;
    struct Probe {
        bool& mode; unsigned& depth; bool prior;
        Probe(bool& m, unsigned& d) : mode(m), depth(d), prior(m) { mode = true; ++depth; }
        ~Probe() { mode = prior; --depth; }
    } probe(template_type_probe,unevaluated_depth);
    auto query = expression_query(node,scope);
    if (!query) return false; // Local identity or another expression owner is not yet typed.
    template_value_queries.put(source,query); ++template_value_work;
    Expression result; result.type = types.fundamental(ast[node].op == KW_NOEXCEPT ? FT_BOOL : FT_UNSIGNED_LONG_INT); result.ready = true;
    expressions.set(node,result); { auto& published = facts.edit(node); published.type = result.type; published.scope = scope; }
    template_fixed_expressions.put(source,node); ++template_fixed_work;
    return true;
}
std::uint32_t Analyzer::query_value(QueryId id)
{
    auto slot = query_value_index.get(id);
    if (!slot) {
        slot = query_values.size(); query_values.push_back(QueryValue()); query_value_index.put(id,slot);
    }
    auto state = query_values[slot].state;
    if (state == FactState::Success) return query_values[slot].constant;
    if (state == FactState::Active) throw std::runtime_error("recursive constant query");
    if (state == FactState::Failure) throw std::runtime_error("failed constant query");
    query_values[slot].state = FactState::Active; ++query_value_work;
    try {
        auto fact = query_fact(id);
        if (fact.state == FactState::Failure) {
            // The structured expression failure is cached by query_fact. Do
            // not evaluate its operands or manufacture a constant from it.
            query_values[slot].state = fact.incomplete ? FactState::NotStarted : FactState::Success;
            query_values[slot].constant = 0; return 0;
        }
        if (fact.dependent) throw std::logic_error("dependent query demanded as a concrete value");
        auto query = type_queries[id]; Constant value;
        if (query.kind == QueryKind::Sizeof) {
            auto type = query.type ? query.type : query_fact(query_edges[query.offset]).expression.type;
            value = query.op == KW_NOEXCEPT ? Constant(types.fundamental(FT_BOOL),query_nonthrowing(query_edges[query.offset])) :
                Constant(types.fundamental(FT_UNSIGNED_LONG_INT),size(type,query.op == KW_ALIGNOF));
        } else if (query.kind == QueryKind::Value) {
            value = convert(Constant(query.type,query.value),query.type);
        } else if (query.kind == QueryKind::Name || query.kind == QueryKind::QualifiedValue) {
            if (!(types[fact.expression.type].cv & 2)) value = constant_indirect(constant_entity_value(fact.expression.entity));
        } else if (query.kind == QueryKind::Member) {
            value = constant_indirect(constant_read(constant_query_object(id)));
        } else if (query.kind == QueryKind::Parenthesized) {
            value = constants[query_value(query_edges[query.offset])];
        } else if (query.kind == QueryKind::Cast) {
            auto operand = query_edges[query.offset];
            if (fact.expression.count) {
                auto result = constant_query_conversion(operand,conversions[fact.expression.conversions]);
                value = convert(result,query.type,true);
                if (value.valid && query.op == TOK_INVALID) {
                    __int128 before = is_unsigned(result.type) ? __int128(result.bits) : __int128(static_cast<std::int64_t>(result.bits));
                    __int128 after = is_unsigned(value.type) ? __int128(value.bits) : __int128(static_cast<std::int64_t>(value.bits));
                    if (before != after) value = Constant();
                }
            } else if (query.op == TOK_INVALID) {
                auto arg = convert_argument(value_argument_id(operand),query.type);
                if (!arg) throw std::runtime_error("invalid implicit constant template conversion");
                value = constants[query_value(argument_query(arg))];
            } else value = convert(constants[query_value(operand)],query.type,true);
        } else if (query.kind == QueryKind::Conditional) {
            auto begin = fact.expression.conversions;
            auto condition = constant_query_conversion(query_edges[query.offset],conversions[begin]);
            if (condition.valid && !scoped_enum(condition.type))
                value = constant_query_conversion(query_edges[query.offset+(constant_truth(condition) ? 1 : 2)],conversions[begin+(constant_truth(condition) ? 1 : 2)]);
        } else if (!fact.selected && query.kind == QueryKind::Unary && query.op == OP_AMP && types[fact.expression.type].kind == TypeKind::MemberPointer) {
            value = Constant(fact.expression.type,fact.expression.entity);
        } else if (!fact.selected && query.kind == QueryKind::Unary) {
            if (query.op == OP_AMP) {
                auto address = constant_query_object(query_edges[query.offset]);
                if (address) value = Constant(fact.expression.type,address);
            } else if (query.op == OP_STAR) value = constant_indirect(constant_read(constant_query_object(id)));
            else {
            value = constants[query_value(query_edges[query.offset])];
            if (value.valid && !scoped_enum(value.type)) {
                if (query.op == OP_LNOT) value = Constant(types.fundamental(FT_BOOL),!constant_truth(value));
                else {
                    value = convert(value,fact.expression.type,true);
                    if (query.op == OP_MINUS) value = floating_type(value.type) ? floating_constant(value.type,-floating_value(value)) : binary(OP_MINUS,Constant(value.type,0),value,true);
                    else if (query.op == OP_COMPL) value = convert(Constant(value.type,~value.bits),value.type);
                    else if (query.op != OP_PLUS) value = Constant();
                }
            } else value = Constant();
            }
        } else if (!fact.selected && query.kind == QueryKind::Binary && query.op == OP_LSQUARE) {
            auto left = query_edges[query.offset], right = query_edges[query.offset+1];
            while (type_queries[left].kind == QueryKind::Parenthesized) left = query_edges[type_queries[left].offset];
            while (type_queries[right].kind == QueryKind::Parenthesized) right = query_edges[type_queries[right].offset];
            if (type_queries[right].kind == QueryKind::String) std::swap(left,right);
            if (type_queries[left].kind == QueryKind::String)
                value = literal_element(type_queries[left].value,constants[query_value(right)]);
            else value = constant_indirect(constant_read(constant_query_object(id)));
        } else if (!fact.selected && query.kind == QueryKind::Binary) {
            auto a = fact.expression.count == 2 ? constant_query_conversion(query_edges[query.offset],conversions[fact.expression.conversions]) :
                constants[query_value(query_edges[query.offset])];
            if (a.valid) {
                if (query.op == OP_LAND && !constant_truth(a)) value = Constant(types.fundamental(FT_BOOL),0);
                else if (query.op == OP_LOR && constant_truth(a)) value = Constant(types.fundamental(FT_BOOL),1);
                else {
                    auto b = fact.expression.count == 2 ? constant_query_conversion(query_edges[query.offset+1],conversions[fact.expression.conversions+1]) :
                        constants[query_value(query_edges[query.offset+1])];
                    if (query.op == OP_COMMA) value = b;
                    else if (fact.expression.count == 2) {
                        auto begin = fact.expression.conversions;
                        a = convert(a,conversions[begin].target,true);
                        b = convert(b,conversions[begin+1].target,true);
                        value = binary(query.op,a,b,true);
                    } else value = binary(query.op,a,b);
                }
            }
        } else if (fact.selected && (query.kind == QueryKind::Unary || query.kind == QueryKind::Binary)) {
            value = constant_query_call(id);
        } else if (query.kind == QueryKind::ListInitialization || fact.initialization) {
            value = constant_query_list(fact.initialization);
        } else if (query.kind == QueryKind::Call) {
            auto callee = type_queries[query_edges[query.offset]];
            if (callee.kind == QueryKind::TypeValue && (integral(callee.type) || floating_type(callee.type))) {
                if (query.count == 1) value = convert(Constant(types.fundamental(FT_INT),0),callee.type,true);
                if (query.count == 2) value = convert(fact.selected ?
                    constant_query_conversion(query_edges[query.offset+1],conversions[fact.expression.conversions]) :
                    constants[query_value(query_edges[query.offset+1])],callee.type,true);
            } else value = constant_query_call(id);
        }
        auto constant = value.valid ? constants.size() : 1;
        if (value.valid) constants.push_back(value);
        query_values[slot].constant = constant; query_values[slot].state = FactState::Success;
        return constant;
    } catch (...) { query_values[slot].state = FactState::Failure; throw; }
}
bool Analyzer::reuse_template_value(NodeId node, ScopeId scope, Expression& result)
{
    auto occurrence = ast.nodes.occurrences[node];
    auto source = template_value_queries.get(occurrence.source);
    if (!source) return false;
    auto frame = template_type_contexts.get(occurrence.context);
    if (!frame) throw std::logic_error("missing body value substitution frame");
    Index bindings, cache;
    auto query = substitute_query(source,bindings,cache,frame);
    if (!query) throw std::runtime_error("invalid substituted body value query");
    auto value = query_value(query); ++template_value_uses;
    result.type = constants[value].type;
    { auto& published = facts.edit(node); published.type = result.type; published.scope = scope; published.value = value; }
    auto operand = type_queries[query].type;
    if (!operand) operand = query_fact(query_edges[type_queries[query].offset]).expression.type;
    facts.edit(ast[node].first).type = operand;
    return true;
}
bool Analyzer::fixed_layout_operand(NodeId node) const
{
    if (!facts[node].value) return false;
    if (expressions[node].form == ExpressionForm::ConstantQuery) return true;
    if (ast[node].kind != Kind::Sizeof) return false;
    auto operand = facts[ast[node].first].type;
    return types[operand].kind != TypeKind::Named || !entities[types[operand].entity].specialization;
}
void Analyzer::reuse_value_conversions(NodeId node, NodeId source, Expression& result)
{
    auto op = ast[node].op;
    if ((op != OP_PLUS && op != OP_MINUS && op != OP_PLUSASS && op != OP_MINUSASS) || result.count < 2 ||
        !template_value_dependence.get(ast.nodes.occurrences[node].source)) return;
    auto original = result.conversions;
    if (pointer(conversions[original].target) || pointer(conversions[original+1].target)) return;
    auto first = ast[node].first, second = ast[first].next;
    unsigned flags = unsigned(fixed_layout_operand(second)) | (unsigned(fixed_layout_operand(first)) << 1);
    auto previous = unsigned(conversions[original].fold_widen) | (unsigned(conversions[original+1].fold_widen) << 1);
    if (flags == previous) return;
    // Value/layout substitution can change this O0 immediate policy while the
    // selected types/conversions stay fixed. Share each source/flag variant;
    // do not rebuild a conversion sequence for every expression occurrence.
    auto k = key(source,flags+1);
    auto begin = template_value_conversions.get(k);
    if (!begin) {
        begin = conversions.size();
        for (unsigned i = 0; i < result.count; ++i) {
            auto conversion = conversions[original+i];
            if (i < 2) conversion.fold_widen = (flags >> i) & 1;
            conversions.push_back(conversion);
        }
        template_value_conversions.put(k,begin); ++value_conversion_variants;
        value_conversion_records += result.count;
    }
    result.conversions = begin;
    expressions.inherit_conversions(node,source,begin);
    for (auto child = first; child; child = ast[child].next) {
        auto incoming = expressions[child].incoming;
        if (incoming >= original && incoming-original < result.count)
            expressions.incoming(child,begin+incoming-original);
    }
}
} }
