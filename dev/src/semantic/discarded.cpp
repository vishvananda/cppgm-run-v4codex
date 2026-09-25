#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::prepare_expression_discard(NodeId n)
{
    auto x = expressions[n];
    if (x.form == ExpressionForm::Ordinary && ast[n].kind == Kind::Binary && ast[n].op == OP_COMMA)
        prepare_discarded(ast[n].first);
    else if (x.form == ExpressionForm::Cast && x.count && conversions[x.conversions].kind == Conversion::Kind::Discarded) {
        auto first = ast[n].first;
        prepare_discarded(ast[n].kind == Kind::Cast ? ast[first].next : ast[ast[first].next].first);
    }
}
bool Analyzer::valid_discarded(Expression x, NodeId n, ScopeId s, Conversion& c)
{
    if (!x.discarded_form || x.category != ValueCategory::Lvalue || !(types[x.type].cv & 2) || !class_value(x.type)) return true;
    ++discard_selections;
    c = conversion_value(x,types.unqualified(x.type),true,n);
    return c.valid() && c.kind == Conversion::Kind::Construction && valid_fixed_conversion(x,n,c,s);
}
void Analyzer::query_discarded(const TypeQuery& q, const std::vector<TypeQueryFact>& children, TypeQueryFact& r)
{
    if (r.state == FactState::Failure || r.dependent) return;
    auto& x = r.expression;
    x.discarded_form = false;
    if (q.kind == QueryKind::Parenthesized) x.discarded_form = children[0].expression.discarded_form;
    else if (q.kind == QueryKind::Conditional)
        x.discarded_form = children[1].expression.discarded_form && children[2].expression.discarded_form;
    else if (q.kind == QueryKind::Binary && q.op == OP_COMMA && !r.selected) {
        Conversion c;
        if (!valid_discarded(children[0].expression,0,q.context,c)) {
            r = TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands); return;
        }
        x.discarded_form = children[1].expression.discarded_form;
    } else if (!r.selected) x.discarded_form = q.kind == QueryKind::Name || q.kind == QueryKind::Parameter ||
        q.kind == QueryKind::QualifiedValue || q.kind == QueryKind::TemplateValueParameter || q.kind == QueryKind::Member ||
        (q.kind == QueryKind::Unary && q.op == OP_STAR) ||
        (q.kind == QueryKind::Binary && (q.op == OP_LSQUARE || q.op == OP_DOTSTAR || q.op == OP_ARROWSTAR));
}
void Analyzer::prepare_discarded(NodeId n)
{
    auto x = expressions[n];
    if (!x.discarded_form || x.category != ValueCategory::Lvalue || !(types[x.type].cv & 2) || !class_value(x.type)) return;
    auto known = discarded_conversions.get(n);
    Conversion c;
    if (known) {
        c = conversions[known];
        if (unevaluated_depth || conversion_objects[c.materialization].use != ConversionUse::Recipe) return;
    } else {
        auto occurrence = ast.nodes.occurrences[n];
        auto source = occurrence.context ? template_fixed_expressions.get(occurrence.source) : 0;
        auto retained = source ? discarded_conversions.get(source) : 0;
        if (retained) { c = copy_conversion_recipe(conversions[retained]); ++discard_recipe_uses; }
        else if (!valid_discarded(x,n,facts[n].scope,c))
            throw std::runtime_error("discarded volatile class requires an accessible volatile copy constructor");
    }
    // Unevaluated/fixed source uses own only the checked recipe. A concrete
    // discarded use supplies one temporary and its lifetime; no grammar replay
    // or unrelated member body demand is involved.
    if (!unevaluated_depth) { materialize_conversion(n,c); ++discard_materializations; }
    if (known) conversions[known] = c;
    else { discarded_conversions.put(n,conversions.size()); conversions.push_back(c); }
}
} }
