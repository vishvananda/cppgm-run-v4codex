#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::deduction_guide(NodeId n, ScopeId s)
{
    if (explicit_specialization_source)
        throw std::runtime_error("deduction guide cannot be specialized");
    bool templated = s == active_template_scope;
    auto owner = templated ? scopes[s].parent : s;
    auto primary = lookup(s,ast.text(n),Lookup::Qualifier);
    if (!primary || entities[primary].kind != EntityKind::Type || !entities[primary].class_info ||
        !entities[primary].template_info || entities[primary].owner != owner ||
        (scopes[owner].kind != ScopeKind::Namespace && scopes[owner].kind != ScopeKind::Class))
        throw std::runtime_error("deduction guide must share its class template's scope");
    if (scopes[owner].kind == ScopeKind::Class && entities[primary].access != declaration_access(owner))
        throw std::runtime_error("deduction guide must share its class template's access");
    auto d = child(n,Kind::Declarator);
    DeductionGuide guide; guide.primary = primary; guide.source = n; guide.environment = s;
    if (templated) guide.head = retain_template_head(s);
    // Signature formation establishes parameter identities and the typed
    // result specialization. It neither completes that class nor declares a
    // function. Canonical types/queries remain owned by this translation unit.
    guide.signature = types.signature(declarator(d,types.fundamental(FT_VOID),s,0,true));
    const auto signature = types[guide.signature];
    if (argument_packs[expansion_parameters(guide.signature)].count)
        throw std::runtime_error("unexpanded parameter pack in deduction guide");
    if (templated) {
        auto deduced = deducible_parameters(guide.signature);
        auto head = templates[guide.head];
        for (unsigned j = 0; j < head.count; ++j) {
            auto parameter = template_parameters[head.offset+j];
            if (!entities[parameter].initializer && !entities[parameter].parameter_pack && !deduced.get(parameter))
                throw std::runtime_error("guide template parameter cannot be deduced");
        }
    }
    auto result = types[signature.child];
    if (result.kind != TypeKind::Named || !entities[result.entity].specialization ||
        specialization_pattern(result.entity) != primary)
        throw std::runtime_error("guide result is not its class template specialization");
    bool seen_default = false;
    auto params = child(d,Kind::Parameters);
    for (auto p = ast.first(params); p; p = ast.next(p)) {
        if (ast.kind(p) != Kind::Parameter) continue;
        ++deduction_guide_parameters;
        auto decl = ast.next(ast.first(p)), type = facts[p].type;
        bool pack = declarator_pack(decl) != 0;
        auto argument = child(p,Kind::DefaultArgument);
        if (fundamental(type,FT_VOID) && (decl_name(decl) || p != ast.first(params) || ast.next(p) || argument || pack))
            throw std::runtime_error("invalid void guide parameter");
        if (placeholder_type(type)) throw std::runtime_error("placeholder guide parameter");
        if (argument && pack) throw std::runtime_error("guide parameter pack has a default");
        if (seen_default && !argument && !pack) throw std::runtime_error("missing trailing guide default");
        seen_default |= argument != 0;
    }
    bind_template_defaults(d,s,templated ? s : 0);
    auto spec = child(n,Kind::Specifier);
    if (spec) {
        guide.explicit_guide = true;
        if (ast.first(spec)) {
            bind_template_expression(ast.first(spec),s);
            guide.explicit_condition = expression_query(ast.first(spec),s);
            if (!query_fact(guide.explicit_condition).dependent)
                guide.explicit_guide = explicit_condition_value(guide.explicit_condition,s);
        }
    }
    for (auto q = ast.next(params); q; q = ast.next(q)) {
        if (ast.kind(q) != Kind::FunctionQualifier) continue;
        guide.nonthrowing = true;
        if (ast.first(q)) {
            auto scope = facts[ast.first(child(d,Kind::TrailingReturn))].scope;
            bind_template_expression(ast.first(q),scope);
            guide.exception_condition = expression_query(ast.first(q),scope);
            if (!query_fact(guide.exception_condition).dependent)
                guide.nonthrowing = explicit_condition_value(guide.exception_condition,scope);
        }
    }
    std::uint32_t shape;
    if (templated) shape = intern_arguments({1,template_declaration_shape(guide.signature,s)});
    else {
        std::vector<TypeId> parameters(types.parameters.begin()+signature.offset,types.parameters.begin()+signature.offset+signature.count);
        shape = intern_arguments({0,types.function(types.fundamental(FT_VOID),parameters,signature.variadic)});
    }
    auto k = key(primary,shape);
    if (deduction_guide_signatures.get(k)) throw std::runtime_error("duplicate deduction guide");
    guide.next = deduction_guide_index.get(primary);
    auto id = deduction_guides.size(); deduction_guides.push_back(guide);
    deduction_guide_signatures.put(k,id); deduction_guide_index.put(primary,id);
    facts.edit(n).type = guide.signature; facts.edit(n).scope = s;
}
} }
