#include "syntax/parser.h"
#include <stdexcept>
namespace cppgm { namespace syntax {
bool Parser::deduction_guide_ahead()
{
    std::size_t i = 0;
    if (in.is("explicit",i)) {
        ++i;
        if (in.is("(",i)) i = in.matching(i)+1;
    }
    // A guide has no decl-specifier-seq or declarator-id. Inspect delimiters
    // only; the parameter and result grammar is parsed once below.
    if (!identifier(i) || !in.is("(",i+1)) return false;
    i = in.matching(i+1)+1;
    if (in.is("noexcept",i)) {
        ++i;
        if (in.is("(",i)) i = in.matching(i)+1;
    }
    return in.is("->",i);
}
NodeId Parser::deduction_guide()
{
    auto result = make(Kind::DeductionGuide);
    if (in.is("explicit")) {
        auto spec = leaf(Kind::Specifier);
        if (in.eat("(")) { ast.append(spec,expression()); in.require(")"); }
        ast.append(result,spec);
    }
    ast[result].text = in.take().text;
    auto d = make(Kind::Declarator);
    ScopeId saved = scope, parameters_scope;
    ast.append(d,parameters(parameters_scope));
    scope = parameters_scope;
    // Reuse the parameter/exception/trailing-type nodes; no identifier node
    // is invented and the guide never changes the ordinary name index.
    function_suffix(d);
    auto trailing = ast[d].last;
    if (ast[trailing].kind != Kind::TrailingReturn) throw std::runtime_error("guide requires trailing template-id");
    auto type = ast[trailing].first, specs = ast[type].first, spec = ast[specs].first;
    auto name = ast[spec].detail, part = ast[name].first;
    if (ast[specs].next || ast[spec].next || ast[spec].kind != Kind::TypeName || ast[spec].flags ||
        ast[name].op == OP_COLON2 || part != ast[name].last || !ast[part].first ||
        ast[part].text != ast[result].text)
        throw std::runtime_error("guide result must be the same simple template-id");
    for (auto c = ast[d].first; c; c = ast[c].next)
        if (ast[c].kind != Kind::Parameters && ast[c].kind != Kind::TrailingReturn &&
            !(ast[c].kind == Kind::FunctionQualifier && ast[c].op == KW_NOEXCEPT))
            throw std::runtime_error("invalid deduction guide qualifier");
    ast.append(result,d);
    in.require(";");
    scope = saved;
    return result;
}
} }
