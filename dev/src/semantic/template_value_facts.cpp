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
    Expression result; result.type = types.fundamental(FT_UNSIGNED_LONG_INT); result.ready = true;
    expressions.set(node,result); facts.edit(node).type = result.type; facts.edit(node).scope = scope;
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
        if (fact.dependent) throw std::logic_error("dependent query demanded as a concrete value");
        auto query = type_queries[id]; Constant value;
        if (query.kind == QueryKind::Sizeof) {
            auto type = query.type ? query.type : query_fact(query_edges[query.offset]).expression.type;
            value = Constant(types.fundamental(FT_UNSIGNED_LONG_INT),size(type,query.op == KW_ALIGNOF));
        } else if (query.kind == QueryKind::Value && integral(query.type)) {
            value = convert(Constant(query.type,query.value),query.type);
        } else if (query.kind == QueryKind::Name || query.kind == QueryKind::QualifiedValue) {
            value = entities[fact.expression.entity].constant;
        } else if (query.kind == QueryKind::Parenthesized) {
            value = constants[query_value(query_edges[query.offset])];
        } else if (query.kind == QueryKind::Cast) {
            value = convert(constants[query_value(query_edges[query.offset])],query.type,true);
        } else if (query.kind == QueryKind::Conditional) {
            auto condition = constants[query_value(query_edges[query.offset])];
            if (condition.valid && !scoped_enum(condition.type))
                value = convert(constants[query_value(query_edges[query.offset+(condition.bits ? 1 : 2)])],fact.expression.type,true);
        } else if (!fact.selected && query.kind == QueryKind::Unary) {
            value = constants[query_value(query_edges[query.offset])];
            if (value.valid && !scoped_enum(value.type)) {
                if (query.op == OP_LNOT) value = Constant(types.fundamental(FT_BOOL),!value.bits);
                else {
                    value = convert(value,fact.expression.type,true);
                    if (query.op == OP_MINUS) value = binary(OP_MINUS,Constant(value.type,0),value,true);
                    else if (query.op == OP_COMPL) value = convert(Constant(value.type,~value.bits),value.type);
                    else if (query.op != OP_PLUS) value = Constant();
                }
            } else value = Constant();
        } else if (!fact.selected && query.kind == QueryKind::Binary) {
            auto a = constants[query_value(query_edges[query.offset])];
            if (a.valid) {
                if (query.op == OP_LAND && !a.bits) value = Constant(types.fundamental(FT_BOOL),0);
                else if (query.op == OP_LOR && a.bits) value = Constant(types.fundamental(FT_BOOL),1);
                else {
                    auto b = constants[query_value(query_edges[query.offset+1])];
                    if (query.op == OP_COMMA) value = b;
                    else if (fact.expression.count == 2) {
                        auto begin = fact.expression.conversions;
                        a = convert(a,conversions[begin].target,true);
                        b = convert(b,conversions[begin+1].target,true);
                        value = binary(query.op,a,b,true);
                    } else value = binary(query.op,a,b);
                }
            }
        } else if (query.kind == QueryKind::Call) {
            auto callee = type_queries[query_edges[query.offset]];
            if (callee.kind == QueryKind::TypeValue && integral(callee.type)) {
                if (query.count == 1) value = Constant(callee.type,0);
                if (query.count == 2) value = convert(constants[query_value(query_edges[query.offset+1])],callee.type,true);
            }
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
    facts.edit(node).type = result.type; facts.edit(node).scope = scope; facts.edit(node).value = value;
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
