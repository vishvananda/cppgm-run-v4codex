#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;

NodeId Analyzer::angle_less(NodeId name, NodeId argument, std::uint32_t location)
{
    // A parsed template argument can contain operators binding less tightly
    // than '<'. Insert the selected relation at its left edge using the same
    // grammar precedence table as parsing, retaining every original operand.
    NodeId parent = 0, operand = argument;
    for (;;) {
        auto n = ast[operand];
        int precedence = n.kind == Kind::Conditional ? 3 : n.kind == Kind::Assignment ? 2 :
            n.kind == Kind::Binary ? syntax::expression_precedence(n.op) : 0;
        if (!precedence || precedence > syntax::expression_precedence(OP_LT)) break;
        parent = operand; operand = n.first;
    }
    auto left = ast.make_source(Kind::IdExpression);
    auto l = ast[left]; l.detail = name; l.location = ast[name].location; l.next = operand;
    ast.resolve_source_node(left,l);
    auto relation = ast.make_source(Kind::Binary);
    auto r = ast[relation]; r.op = OP_LT; r.location = location;
    r.first = left; r.last = operand; r.next = parent ? ast[operand].next : 0;
    ast.resolve_source_node(relation,r);
    auto value = ast[operand]; value.next = 0; ast.resolve_source_node(operand,value);
    if (!parent) return relation;
    auto p = ast[parent]; p.first = relation; ast.resolve_source_node(parent,p);
    return argument;
}

NodeId Analyzer::angle_operand(NodeId d)
{
    std::vector<NodeId> prefixes;
    auto c = ast[d].first;
    while (ast[c].kind == Kind::Pointer && !ast[c].detail &&
           (ast[c].op == OP_STAR || ast[c].op == OP_AMP)) {
        prefixes.push_back(c); c = ast[c].next;
    }
    auto suffix = ast[c].next;
    auto value = ast[c];
    if (value.kind == Kind::Identifier) value.kind = Kind::IdExpression;
    else if (value.kind == Kind::NestedDeclarator) {
        value.kind = Kind::Parenthesized;
        value.first = value.last = angle_operand(value.first);
    } else throw std::runtime_error("invalid relational operand");
    value.next = 0; ast.resolve_source_node(c,value);
    NodeId result = c;
    for (auto next = suffix; next;) {
        auto node = ast[next]; auto following = node.next; node.next = 0;
        if (node.kind == Kind::Array && node.first) {
            auto base = ast[result]; base.next = node.first; ast.resolve_source_node(result,base);
            node.kind = Kind::Subscript; node.last = node.first; node.first = result;
        } else if (node.kind == Kind::Parameters && !node.first) {
            node.kind = Kind::Arguments; ast.resolve_source_node(next,node);
            auto call = ast.make_source(Kind::Call);
            auto base = ast[result]; base.next = next; ast.resolve_source_node(result,base);
            auto applied = ast[call]; applied.first = result; applied.last = next; applied.location = node.location;
            ast.resolve_source_node(call,applied); result = call; next = following; continue;
        } else throw std::runtime_error("invalid postfix relational operand");
        ast.resolve_source_node(next,node); result = next; next = following;
    }
    for (auto i = prefixes.rbegin(); i != prefixes.rend(); ++i) {
        auto unary = ast[*i]; unary.kind = Kind::Unary;
        unary.first = unary.last = result; unary.next = 0;
        ast.resolve_source_node(*i,unary); result = *i;
    }
    return result;
}

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
            if (remainder.arguments) {
                auto first = ast[a]; first.next = remainder.arguments; ast.resolve_source_node(a,first);
                auto args = ast[list]; args.last = remainder.argument_last; ast.resolve_source_node(list,args);
            }
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
            if (!expression_node || !a || ast[a].kind == Kind::TypeId)
                throw std::runtime_error("non-template name followed by template arguments");
            AngleRemainder result; result.relational = true;
            result.suffix = ast[p].next; result.last = ast[name].last;
            result.close = ast[list].literal;
            result.arguments = ast[a].next; result.argument_last = ast[list].last;
            auto part = ast[p]; part.first = part.last = part.next = 0;
            ast.resolve_source_node(p,part);
            auto n = ast[name]; n.last = p;
            ast.resolve_source_node(name,n);
            auto continuation = ast[expression_node].next;
            auto root = angle_less(name,a,ast[list].location);
            auto value = ast[root]; value.next = continuation;
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
    auto remainder = resolve_angle_name(name,spec,s);
    if (!remainder.relational) return;
    ++angle_interpretations;
    auto list = child(n,Kind::InitDeclarators), item = ast[list].first, d = ast[item].first;
    if (remainder.suffix || remainder.arguments || !d ||
        ast[d].next || ast[item].next)
        throw std::runtime_error("invalid expression after relational template-id");
    auto id = angle_operand(d);
    auto left = spec;
    auto l = ast[left];
    if (l.kind == Kind::DeclSpecifier) {
        l.kind = Kind::IdExpression; l.text = 0; l.op = TOK_INVALID;
        l.first = l.last = 0; l.detail = name;
    }
    l.next = id;
    ast.resolve_source_node(left,l);
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
