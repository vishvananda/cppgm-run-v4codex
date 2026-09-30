#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
Expression Analyzer::statement_expression(NodeId n, ScopeId s)
{
    auto owner = s;
    while (owner && scopes[owner].kind != ScopeKind::Function) owner = scopes[owner].parent;
    if (!owner) throw std::runtime_error("statement expression outside function");
    auto body = ast[n].first, last = ast[body].last;
    auto block = make_scope(ScopeKind::Block,s);
    facts.edit(body).scope = block;
    Expression result; result.type = types.fundamental(FT_VOID);
    for (auto c = ast[body].first; c; c = ast[c].next) {
        if (c != last || ast[c].kind != Kind::ExpressionStatement || !ast[c].first) {
            resolve_statement(c,block); continue;
        }
        auto value = ast[c].first;
        auto source = expression(value,block);
        if (source.form == ExpressionForm::Overload) throw std::runtime_error("unresolved statement expression result");
        result.type = types.unqualified(decay(source.type));
        facts.edit(n).target = value;
        if (!fundamental(result.type,FT_VOID)) {
            auto conversion = this->conversion(value,result.type);
            if (!conversion.valid()) throw std::runtime_error("invalid statement expression result");
            if (conversion.kind == Conversion::Kind::Construction)
                materialize_conversion(value,conversion,false,ConversionUse::Destination);
            record_conversion(result,value,conversion);
        }
    }
    expression_nonthrowing(body);
    return result;
}
QueryId Analyzer::statement_result_query(NodeId n, ScopeId s)
{
    // Body-bearing expressions have scoped query rules. A lambda's body is
    // never an unevaluated operand in C++11, including inside a statement body.
    if (ast[n].kind == Kind::Lambda) {
        if (template_type_probe) return 0;
        throw std::runtime_error("lambda in unevaluated operand");
    }
    auto body = ast[n].first;
    TypeQuery q; q.kind = QueryKind::StatementResult; q.context = s; q.value = n;
    q.dependent_name = pattern_scope(s);
    if (!facts[body].scope) {
        if (template_type_probe) bind_template_statement(body,s);
        else expression(n,s);
    }
    auto last = ast[body].last;
    std::vector<QueryId> children;
    if (ast[last].kind == Kind::ExpressionStatement && ast[last].first) {
        auto child = expression_query(ast[last].first,facts[body].scope);
        if (!child && template_type_probe) return 0;
        children.push_back(child);
    }
    return intern_query(q,children);
}
} }
