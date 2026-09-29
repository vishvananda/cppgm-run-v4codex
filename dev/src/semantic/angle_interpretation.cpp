#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;

Analyzer::AngleRemainder Analyzer::resolve_angle_name(NodeId name, NodeId expression_node, ScopeId s)
{
    AngleRemainder none;
    if (!(ast[name].flags & 4)) return none;
    ++angle_name_work;
    ScopeId owner = ast[name].op == OP_COLON2 ? global : s;
    bool qualified = ast[name].op == OP_COLON2;
    for (auto p = ast[name].first; p; p = ast[p].next) {
        ++angle_part_work;
        auto list = child(p,Kind::TemplateArguments);
        for (auto a = ast[list].first; a; a = ast[a].next) {
            auto argument_name = ast[a].kind == Kind::IdExpression ? ast[a].detail :
                ast[a].kind == Kind::TypeId ? ast[ast[ast[a].first].first].detail : 0;
            if (!argument_name || !(ast[argument_name].flags & 4)) continue;
            auto remainder = resolve_angle_name(argument_name,a,s);
            if (!remainder.relational) continue;
            if (ast[a].next) throw std::runtime_error("relational template argument has trailing clauses");
            // The inner '>' closes this template-id. Its following qualified
            // components therefore belong to this name, while our old closing
            // '>' is returned to the enclosing expression/argument owner.
            AngleRemainder result; result.relational = true;
            result.suffix = ast[p].next; result.last = ast[name].last;
            result.close = ast[list].literal;
            auto part = ast[p]; part.next = remainder.suffix;
            ast.resolve_source_node(p,part);
            auto n = ast[name]; n.last = remainder.suffix ? remainder.last : p;
            ast.resolve_source_node(name,n);
            return result;
        }
        // Ordinary lookup decides whether '<' introduces arguments. Qualifier
        // lookup alone would incorrectly bypass a value hiding a class name.
        auto e = lookup(owner,ast[p].text,list ? Lookup::Ordinary :
            p == ast[name].last ? Lookup::Ordinary : Lookup::Qualifier,qualified);
        if (!e || entities[e].template_parameter || (!list && dependent_type(entities[e].type))) return none;
        bool is_template = entities[e].template_info || template_entity(e);
        if (list && !is_template && function_binding(e))
            for (auto candidate : candidates(e)) is_template |= entities[candidate].template_info != 0;
        if (list && qualified && !(ast[p].flags & 1) && !is_template) {
            auto a = ast[list].first;
            if (!expression_node || !a || ast[a].next || ast[a].kind == Kind::TypeId)
                throw std::runtime_error("non-template name followed by template arguments");
            AngleRemainder result; result.relational = true;
            result.suffix = ast[p].next; result.last = ast[name].last;
            result.close = ast[list].literal;
            auto part = ast[p]; part.first = part.last = part.next = 0;
            ast.resolve_source_node(p,part);
            auto n = ast[name]; n.last = p;
            ast.resolve_source_node(name,n);
            auto left = ast.make_source(Kind::IdExpression);
            auto l = ast[left]; l.detail = name; l.location = ast[name].location; l.next = a;
            ast.resolve_source_node(left,l);
            auto value = ast[expression_node];
            value.kind = Kind::Binary; value.op = OP_LT; value.text = 0;
            value.first = left; value.last = a; value.detail = 0;
            value.location = ast[list].location;
            ast.resolve_source_node(expression_node,value);
            return result;
        }
        if (p == ast[name].last) return none;
        if (list) e = class_template_name(p,e,s);
        if (!e || dependent_type(entities[e].type)) return none;
        auto type = entities[e].type;
        auto cls = entities[e].class_info ? e : types[type].kind == TypeKind::Named ? types[type].entity : 0;
        if (cls && entities[cls].class_info) complete_class(cls);
        owner = target(e);
        if (!owner) return none;
        qualified = true;
    }
    return none;
}

void Analyzer::resolve_angle_statement(NodeId n, ScopeId s)
{
    if (ast[n].kind == Kind::ForInit && !ast.nodes.occurrences[n].context) {
        auto c = ast[n].first;
        resolve_angle_statement(c,s);
        if (ast[c].kind == Kind::ExpressionStatement) {
            auto init = ast[n]; init.first = init.last = ast[c].first;
            ast.resolve_source_node(n,init);
        }
        return;
    }
    if (ast[n].kind != Kind::SimpleDeclaration || ast.nodes.occurrences[n].context) return;
    auto specs = ast[n].first, spec = ast[specs].first, name = ast[spec].detail;
    if (!name || !(ast[name].flags & 4) || ast[spec].next) return;
    auto remainder = resolve_angle_name(name,0,s);
    if (!remainder.relational) return;
    ++angle_interpretations;
    auto list = child(n,Kind::InitDeclarators), item = ast[list].first, d = ast[item].first;
    auto id = ast[d].first;
    if (remainder.suffix || !id || ast[id].kind != Kind::Identifier || ast[id].next ||
        ast[d].next || ast[item].next)
        throw std::runtime_error("invalid expression after relational template-id");
    auto left = ast.make_source(Kind::IdExpression);
    auto l = ast[left]; l.detail = name; l.location = ast[name].location; l.next = id;
    ast.resolve_source_node(left,l);
    auto rhs = ast[id]; rhs.kind = Kind::IdExpression;
    ast.resolve_source_node(id,rhs);
    // Reuse declaration wrappers for the selected expression grammar. The
    // original source graph remains available; no token or tree is replayed.
    auto op = ast[specs]; op.kind = Kind::Binary; op.op = OP_GT;
    op.first = left; op.last = id; op.next = 0; op.location = remainder.close;
    ast.resolve_source_node(specs,op);
    auto statement = ast[n]; statement.kind = Kind::ExpressionStatement;
    statement.first = statement.last = specs;
    ast.resolve_source_node(n,statement);
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
}
} }
