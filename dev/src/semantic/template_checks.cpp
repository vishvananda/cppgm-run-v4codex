#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::check_template_parameters(NodeId n, ScopeId s)
{
    Index parameters;
    for (auto scope = s; scope; scope = scopes[scope].parent) {
        if (scopes[scope].kind != ScopeKind::Template) continue;
        for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
            auto e = declarations[d].entity;
            if (entities[e].template_parameter && entities[e].name) parameters.put(entities[e].name,1);
        }
    }
    struct Work { NodeId node; bool callee; };
    std::vector<Work> work(1,Work{n,false}); Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto item = work[i]; auto node = ast[item.node];
        if (!item.node || seen.get(item.node)) continue;
        seen.put(item.node,1);
        IdentifierId declared = 0;
        NodeId name = 0;
        if (node.kind == Kind::Declarator) name = decl_name(item.node);
        if (node.kind == Kind::Class || node.kind == Kind::ClassForward || node.kind == Kind::Enum) name = node.detail;
        if (name && ast[name].first == ast[name].last && ast[name].op != OP_COLON2) declared = terminal(name);
        if (node.kind == Kind::Alias || node.kind == Kind::Enumerator) declared = node.text;
        if (node.kind == Kind::UsingDeclaration) declared = terminal(ast[node.first].detail);
        if (declared && parameters.get(declared)) throw std::runtime_error("declaration redeclares template parameter");
        if (node.kind == Kind::IdExpression && !item.callee && ast[node.detail].first == ast[node.detail].last &&
            parameters.get(terminal(node.detail))) throw std::runtime_error("type template parameter used as a value");
        if (node.detail) work.push_back({node.detail,false});
        for (auto child = node.first; child; child = ast[child].next)
            work.push_back({child,node.kind == Kind::Call && child == node.first});
    }
}
void Analyzer::index_template_members(NodeId n, std::uint32_t path, ScopeId s)
{
    auto add = [&](NodeId d) {
        if (!d) return;
        auto name = decl_name(d), member = terminal(name);
        if (ast[ast[name].last].op == OP_COMPL) {
            auto text = ids.spelling(ast[ast[ast[name].last].first].text);
            std::string spelling = "~" + std::string(text.data,text.size);
            member = ids.intern(TextView(spelling.data(),spelling.size()));
        }
        auto k = key(path,member);
        TemplatePrototype prototype{d,s,template_prototype_index.get(k)};
        template_prototype_index.put(k,template_prototypes.size()); template_prototypes.push_back(prototype);
    };
    for (auto c = ast[n].first; c; c = ast[c].next) {
        if (ast[c].kind == Kind::Class) {
            index_template_members(c,definition_path(path,terminal(ast[c].detail)),s); continue;
        }
        if (ast[c].kind == Kind::SimpleDeclaration) {
            auto first = ast[child(c,Kind::InitDeclarators)].first;
            for (auto spec = ast[ast[c].first].first; spec; spec = ast[spec].next)
                if (ast[spec].kind == Kind::Class) {
                    auto name = ast[spec].detail ? terminal(ast[spec].detail) : terminal(decl_name(ast[first].first));
                    if (name) index_template_members(spec,definition_path(path,name),s);
                }
            for (auto item = first; item; item = ast[item].next) add(ast[item].first);
        } else add(ast[c].kind == Kind::Function ? ast[ast[c].first].next : child(c,Kind::Declarator));
    }
}
bool Analyzer::nullary_declarator(NodeId d) const
{
    auto list = child(d,Kind::Parameters);
    if (!list) return false;
    auto p = ast[list].first;
    if (!p) return true;
    auto spec = ast[ast[p].first].first;
    return !ast[p].next && ast[spec].op == KW_VOID && !ast[spec].next && !ast[ast[p].first].next;
}
int Analyzer::template_exception(NodeId d, ScopeId s)
{
    for (auto c = ast[d].first; c; c = ast[c].next) {
        if (ast[c].kind != Kind::FunctionQualifier || ast[c].op != KW_NOEXCEPT) continue;
        if (!ast[c].first) return 1;
        // Names in this fact may need class declaration facts or substitution.
        // Keep that dependency instead of evaluating in the wrong environment.
        std::vector<NodeId> work(1,ast[c].first);
        for (std::size_t j = 0; j < work.size(); ++j) {
            auto n = ast[work[j]];
            if (n.kind == Kind::IdExpression || n.kind == Kind::TypeId) return -1;
            for (auto child = n.first; child; child = ast[child].next) work.push_back(child);
        }
        auto value = evaluate(ast[c].first,s);
        if (!value.valid) throw std::runtime_error("nonconstant template exception specification");
        return value.bits != 0;
    }
    return ast[ast[decl_name(d)].last].op == OP_COMPL ? -1 : 0;
}
void Analyzer::check_template_member_exception(NodeId d, std::uint32_t path, IdentifierId name, ScopeId s)
{
    if (!nullary_declarator(d)) return;
    auto current = template_exception(d,s);
    if (current < 0) return;
    auto qualifiers = [&](NodeId decl) {
        unsigned result = 0;
        for (auto n = ast[child(decl,Kind::Parameters)].next; n; n = ast[n].next) {
            auto op = ast[n].op;
            if (op == KW_CONST) result |= 1; if (op == KW_VOLATILE) result |= 2;
            if (op == OP_AMP) result |= 4; if (op == OP_LAND) result |= 8;
        }
        return result;
    };
    for (auto p = template_prototype_index.get(key(path,name)); p; p = template_prototypes[p].next) {
        auto previous = template_prototypes[p];
        if (!nullary_declarator(previous.declarator) || qualifiers(d) != qualifiers(previous.declarator)) continue;
        auto spec = template_exception(previous.declarator,previous.environment);
        if (spec >= 0 && current != spec) throw std::runtime_error("conflicting template member exception specifications");
    }
}
} }
