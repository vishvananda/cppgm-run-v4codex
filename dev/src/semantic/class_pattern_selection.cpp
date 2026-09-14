#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::declare_class_partial(NodeId n, ScopeId s, EntityId primary)
{
    if (!primary || !entities[primary].class_info || !entities[primary].template_info)
        throw std::runtime_error("class partial specialization requires a primary template");
    if ((entities[primary].key == KW_UNION) != (ast[ast[n].first].op == KW_UNION))
        throw std::runtime_error("incompatible partial specialization class key");
    auto name = ast[n].detail, list = child(ast[name].last,Kind::TemplateArguments);
    std::vector<ArgumentId> args;
    for (auto a = ast[list].first; a; a = ast[a].next)
        append_template_argument(a,s,template_argument_node(a,s),args);
    if (!template_defaults(primary,args)) throw std::runtime_error("invalid class partial arguments");
    Index bindings, cache; std::vector<ArgumentId> signature;
    unsigned count = 0;
    for (auto d = scopes[s].first_decl; d; d = declarations[d].next) {
        auto p = declarations[d].entity;
        if (!entities[p].template_parameter) continue;
        if (entities[p].initializer) throw std::runtime_error("default argument in partial specialization head");
        auto arg = canonical_argument(p,count++,bindings,cache);
        bindings.put(p,arg);
        signature.push_back(entities[p].parameter_pack ? types.compound(TypeKind::PackExpansion,0,arg) : arg);
    }
    auto head_shape = intern_arguments(signature); signature.clear();
    bool dependent = false;
    for (auto arg : args) {
        dependent |= dependent_argument(arg);
        signature.push_back(substitute_argument(arg,bindings,cache));
    }
    if (!dependent) throw std::runtime_error("class partial specialization requires dependent arguments");
    auto identity = key(primary,intern_arguments({head_shape,intern_arguments(signature)}));
    auto e = class_partial_signatures.get(identity);
    if (e && ast[n].kind == Kind::Class && templates[entities[e].template_info].body)
        throw std::runtime_error("class partial specialization redefinition");
    if (!e) {
        e = make_entity(EntityKind::Type,entities[primary].owner,entities[primary].name,n);
        entities[e].key = ast[ast[n].first].op; entities[e].type = types.named(e);
        entities[e].class_info = class_facts.size(); class_facts.push_back(ClassFacts());
        entities[e].scope = make_scope(ScopeKind::Class,s,entities[e].name,e,false);
        class_partial_signatures.put(identity,e);
        class_partial_next.put(e,class_partial_heads.get(primary)); class_partial_heads.put(primary,e);
    }
    if (!entities[e].template_info || !templates[entities[e].template_info].body) {
        template_facts(e,s);
        auto& head = templates[entities[e].template_info];
        head.primary = primary; head.explicit_arguments = intern_arguments(args);
        head.source = n; head.body = ast[n].kind == Kind::Class ? n : 0;
    }
    std::vector<ArgumentId> self;
    if (!match_class_pattern(e,intern_arguments(args),self))
        throw std::runtime_error("class partial specialization has nondeducible parameters");
    auto primary_head = templates[entities[primary].template_info];
    std::vector<ArgumentId> generic, equivalent;
    for (unsigned j = 0; j < primary_head.count; ++j) {
        auto p = template_parameters[primary_head.offset+j];
        auto arg = parameter_argument(p);
        generic.push_back(entities[p].parameter_pack ? make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)}) : arg);
    }
    if (match_class_pattern(e,intern_arguments(generic),equivalent))
        throw std::runtime_error("partial specialization does not specialize the primary");
    bind(s,entities[e].name,e); record(s,e,n,entities[e].type,EntityKind::Type);
    if (ast[n].kind == Kind::Class) {
        index_template_members(n,definition_root(e),s);
        bind_template_class(n,s,e);
    }
    return entities[e].type;
}
bool Analyzer::match_class_pattern(EntityId pattern, std::uint32_t arguments, std::vector<ArgumentId>& deduced)
{
    auto head = templates[entities[pattern].template_info];
    auto source = argument_packs[head.explicit_arguments], actual = argument_packs[arguments];
    if (source.count != actual.count) return false;
    Index bindings, cache;
    for (unsigned j = 0; j < source.count; ++j)
        if (!deduce_type(argument_types[source.offset+j],argument_types[actual.offset+j],bindings)) return false;
    bool packs = false;
    for (unsigned j = 0; j < head.count; ++j) {
        auto value = bindings.get(template_parameters[head.offset+j]);
        if (!value) return false;
        deduced.push_back(value);
        packs |= entities[template_parameters[head.offset+j]].parameter_pack;
    }
    // Call deduction permits conversions/base matches. A class pattern must
    // reproduce the exact canonical argument tuple, including cv and values.
    std::uint32_t frame = 0;
    if (packs) for (unsigned j = 0; j < head.count; ++j)
        frame = argument_frame(frame,template_parameters[head.offset+j],deduced[j]);
    for (unsigned j = 0; j < source.count; ++j)
        if (substitute_argument(argument_types[source.offset+j],bindings,cache,frame) != argument_types[actual.offset+j]) return false;
    return true;
}
void Analyzer::select_class_pattern(std::uint32_t index)
{
    auto spec = specializations[index];
    if (spec.definition_pattern) return;
    struct Candidate { EntityId entity; std::uint32_t arguments; };
    std::vector<Candidate> matches;
    for (auto p = class_partial_heads.get(spec.pattern); p; p = class_partial_next.get(p)) {
        ++candidate_work;
        std::vector<ArgumentId> args;
        if (match_class_pattern(p,spec.arguments,args)) matches.push_back({p,intern_arguments(args)});
    }
    auto more = [&](EntityId a, EntityId b) {
        std::vector<ArgumentId> ab, ba;
        return match_class_pattern(b,templates[entities[a].template_info].explicit_arguments,ba) &&
            !match_class_pattern(a,templates[entities[b].template_info].explicit_arguments,ab);
    };
    unsigned best = 0;
    for (unsigned j = 1; j < matches.size(); ++j)
        if (more(matches[j].entity,matches[best].entity)) best = j;
    for (unsigned j = 0; j < matches.size(); ++j)
        if (j != best && !more(matches[best].entity,matches[j].entity))
            throw std::runtime_error("ambiguous class partial specialization");
    auto selected = matches.empty() ? Candidate{spec.pattern,spec.arguments} : matches[best];
    specializations[index].definition_pattern = selected.entity;
    specializations[index].definition_arguments = selected.arguments;
    if (selected.entity == spec.pattern) return;
    auto environment = make_scope(ScopeKind::Template,entities[selected.entity].owner);
    auto head = templates[entities[selected.entity].template_info];
    auto args = argument_packs[selected.arguments];
    for (unsigned j = 0; j < head.count; ++j)
        bind_argument(environment,template_parameters[head.offset+j],argument_types[args.offset+j]);
    bind(environment,entities[spec.entity].name,spec.entity);
    specializations[index].environment = environment;
    auto scope = entities[spec.entity].scope;
    scopes[scope].parent = environment; scopes[scope].depth = scopes[environment].depth+1;
    auto jump = scopes[environment].jump, grand = scopes[jump].jump;
    scopes[scope].jump = scopes[environment].depth-scopes[jump].depth == scopes[jump].depth-scopes[grand].depth ? grand : environment;
}
} }
