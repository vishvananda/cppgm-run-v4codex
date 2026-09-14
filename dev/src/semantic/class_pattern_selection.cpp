#include "semantic/analyzer.h"
#include <stdexcept>
#include <algorithm>
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
    bool dependent = false;
    for (auto arg : args) dependent |= dependent_argument(arg);
    if (!dependent) throw std::runtime_error("class partial specialization requires dependent arguments");
    for (auto d = scopes[s].first_decl; d; d = declarations[d].next)
        if (entities[declarations[d].entity].template_parameter && entities[declarations[d].entity].initializer)
            throw std::runtime_error("default argument in partial specialization head");
    auto identity = key(primary,template_owner_shape(s,args));
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
    validate_partial_pattern(e);
    bind(s,entities[e].name,e); record(s,e,n,entities[e].type,EntityKind::Type);
    if (ast[n].kind == Kind::Class) {
        index_template_members(n,definition_root(e),s);
        bind_template_class(n,s,e);
    }
    return entities[e].type;
}
void Analyzer::validate_partial_pattern(EntityId e)
{
    auto head = templates[entities[e].template_info];
    for (unsigned j = 0; j < head.count; ++j)
        if (entities[template_parameters[head.offset+j]].initializer)
            throw std::runtime_error("default argument in partial specialization head");
    std::vector<ArgumentId> self;
    if (!match_partial_pattern(e,head.explicit_arguments,self))
        throw std::runtime_error("partial specialization has nondeducible parameters");
    auto primary_head = templates[entities[head.primary].template_info];
    std::vector<ArgumentId> generic, equivalent;
    for (unsigned j = 0; j < primary_head.count; ++j) {
        auto p = template_parameters[primary_head.offset+j];
        auto arg = parameter_argument(p);
        generic.push_back(entities[p].parameter_pack ? make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)}) : arg);
    }
    if (match_partial_pattern(e,intern_arguments(generic),equivalent))
        throw std::runtime_error("partial specialization does not specialize the primary");
}
bool Analyzer::match_partial_pattern(EntityId pattern, std::uint32_t arguments, std::vector<ArgumentId>& deduced)
{
    auto head = templates[entities[pattern].template_info];
    auto source = argument_packs[head.explicit_arguments], actual = argument_packs[arguments];
    if (source.count != actual.count) return false;
    Index bindings, cache;
    for (unsigned j = 0; j < source.count; ++j)
        if (!deduce_type(argument_types[source.offset+j],argument_types[actual.offset+j],bindings,DeductionKind::ClassPattern)) return false;
    bool packs = false;
    for (unsigned j = 0; j < head.count; ++j) {
        auto value = bindings.get(template_parameters[head.offset+j]);
        if (!value) return false;
        deduced.push_back(value);
        packs |= entities[template_parameters[head.offset+j]].parameter_pack;
    }
    // Call deduction permits conversions/base matches. A class pattern must
    // reproduce the exact canonical argument tuple, including cv and values.
    auto parent = head.parent_frame ? head.parent_frame : template_lexical_frame(scopes[head.environment].parent);
    auto frame = packs || parent ? substitution_frame(0,head.offset,head.count,parent,intern_arguments(deduced)) : 0;
    struct Probe {
        bool& value; bool prior;
        Probe(bool& v) : value(v), prior(v) { value = true; }
        ~Probe() { value = prior; }
    } probe(template_type_probe);
    for (unsigned j = 0; j < source.count; ++j)
        if (argument_types[source.offset+j] != argument_types[actual.offset+j] &&
            substitute_argument(argument_types[source.offset+j],bindings,cache,frame) != argument_types[actual.offset+j]) return false;
    return true;
}
void Analyzer::select_partial_pattern(std::uint32_t index)
{
    auto spec = specializations[index];
    if (spec.definition_pattern) return;
    struct Coverage { std::vector<unsigned> omissions, applications; };
    struct Candidate { EntityId entity; std::uint32_t arguments; Coverage coverage; };
    // The course's relaxed template-template matching permits omitted defaults.
    // Retain which actual argument positions were covered. Coverage is a fact
    // of this match, never a context-free candidate-ordering cache entry.
    Index paths; unsigned path_count = 0;
    auto path = [&](unsigned parent, unsigned ordinal) {
        auto k = key(parent,ordinal); auto id = paths.get(k);
        if (!id) { id = ++path_count; paths.put(k,id); }
        return id;
    };
    auto flatten = [&](std::uint32_t id) {
        std::vector<ArgumentId> out; auto pack = argument_packs[id];
        for (unsigned j = 0; j < pack.count; ++j) {
            auto arg = argument_types[pack.offset+j];
            if (argument_pack(arg)) {
                auto a = pack_arguments(arg);
                out.insert(out.end(),argument_types.begin()+a.offset,argument_types.begin()+a.offset+a.count);
            } else out.push_back(arg);
        }
        return out;
    };
    auto coverage = [&](EntityId entity) {
        struct Pair { ArgumentId pattern, actual; unsigned path; };
        std::vector<Pair> work;
        Coverage result;
        auto sequence = [&](const std::vector<ArgumentId>& x, const std::vector<ArgumentId>& y, unsigned parent) {
            auto fixed = x.size();
            bool pack = fixed && !value_argument(x.back()) && types[x.back()].kind == TypeKind::PackExpansion;
            if (pack) --fixed;
            for (unsigned j = 0; j < y.size(); ++j) {
                auto position = path(parent,j+1);
                if (j < fixed) work.push_back({x[j],y[j],position});
                else if (pack) work.push_back({types[x.back()].bound,y[j],position});
                else result.omissions.push_back(position);
            }
        };
        sequence(flatten(templates[entities[entity].template_info].explicit_arguments),flatten(spec.arguments),0);
        for (unsigned j = 0; j < work.size(); ++j) {
            auto pair = work[j];
            if (value_argument(pair.pattern) || value_argument(pair.actual)) continue;
            auto p = types[pair.pattern], a = types[pair.actual];
            if (p.kind == TypeKind::Named && a.kind == TypeKind::Named &&
                entities[p.entity].specialization && entities[a.entity].specialization) {
                if (entities[specialization_pattern(p.entity)].template_parameter) result.applications.push_back(pair.path);
                sequence(flatten(specializations[entities[p.entity].specialization].arguments),
                    flatten(specializations[entities[a.entity].specialization].arguments),pair.path);
            } else if (p.kind == TypeKind::Function && a.kind == TypeKind::Function) {
                sequence(std::vector<ArgumentId>(types.parameters.begin()+p.offset,types.parameters.begin()+p.offset+p.count),
                    std::vector<ArgumentId>(types.parameters.begin()+a.offset,types.parameters.begin()+a.offset+a.count),pair.path);
            }
            if (p.child && a.child) work.push_back({p.child,a.child,path(pair.path,0)});
        }
        std::sort(result.omissions.begin(),result.omissions.end());
        std::sort(result.applications.begin(),result.applications.end());
        return result;
    };
    std::vector<Candidate> matches;
    const bool variable = entities[spec.pattern].kind == EntityKind::Variable;
    const auto& heads = variable ? variable_partial_heads : class_partial_heads;
    const auto& next = variable ? variable_partial_next : class_partial_next;
    for (auto p = heads.get(spec.pattern); p; p = next.get(p)) {
        if (variable) ++variable_candidates;
        else ++candidate_work;
        std::vector<ArgumentId> args;
        if (match_partial_pattern(p,spec.arguments,args)) matches.push_back({p,intern_arguments(args),{}});
    }
    // Coverage only participates when selection has competing viable patterns.
    // The common single-match path needs no positions, traversal or sorting.
    if (matches.size() > 1) for (auto& candidate : matches) candidate.coverage = coverage(candidate.entity);
    auto more = [&](const Candidate& x, const Candidate& y) {
        const auto& xc = x.coverage; const auto& yc = y.coverage;
        if (xc.applications == yc.applications && xc.omissions != yc.omissions)
            return std::includes(yc.omissions.begin(),yc.omissions.end(),xc.omissions.begin(),xc.omissions.end());
        auto a = x.entity, b = y.entity;
        auto identity = key(a,b);
        if (auto known = class_partial_ordering.get(identity)) { ++class_ordering_hits; return known == 2; }
        ++class_ordering_work;
        std::vector<ArgumentId> ab, ba;
        bool result = match_partial_pattern(b,templates[entities[a].template_info].explicit_arguments,ba) &&
            !match_partial_pattern(a,templates[entities[b].template_info].explicit_arguments,ab);
        // Ordering depends on immutable candidate shapes, not the actual
        // specialization. Renamed redeclarations preserve that identity.
        class_partial_ordering.put(identity,result ? 2 : 1);
        return result;
    };
    unsigned best = 0;
    for (unsigned j = 1; j < matches.size(); ++j)
        if (more(matches[j],matches[best])) best = j;
    for (unsigned j = 0; j < matches.size(); ++j)
        if (j != best && !more(matches[best],matches[j]))
            throw std::runtime_error("ambiguous partial specialization");
    auto selected = matches.empty() ? Candidate{spec.pattern,spec.arguments,{}} : matches[best];
    specializations[index].definition_pattern = selected.entity;
    specializations[index].definition_arguments = selected.arguments;
}
} }
