#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::bind_function_packs(NodeId parameters, ScopeId scope)
{
    auto context = ast.nodes.occurrences[parameters].context;
    if (!context) return;
    auto frame = template_type_contexts.get(context);
    Index lists;
    std::vector<std::vector<ArgumentId>> groups;
    for (auto p = ast[parameters].first; p; p = ast[p].next) {
        if (!child(ast[ast[p].first].next,syntax::Kind::ParameterPack)) continue;
        auto source = ast.nodes.occurrences[p].source;
        auto index = lists.get(source);
        if (!index) { groups.emplace_back(); index = groups.size(); lists.put(source,index); }
        groups[index-1].push_back(facts[p].entity);
    }
    for (auto p = ast.nodes[parameters].first; p; p = ast.nodes[p].next) {
        if (!child(ast[ast[p].first].next,syntax::Kind::ParameterPack)) continue;
        auto source = ast.nodes.occurrences[p].source;
        auto pattern = template_declaration_sources.get(source);
        auto name = terminal(decl_name(ast[ast[p].first].next));
        auto aggregate = make_entity(EntityKind::Parameter,scope,name,p);
        entities[aggregate].parameter_pack = true;
        auto group = lists.get(source);
        entity_pack_arguments.put(aggregate,make_argument_pack(group ? groups[group-1] : std::vector<ArgumentId>()));
        if (pattern) substitution_binding_cache.put(key(frame,pattern),aggregate);
        bind(scope,name,aggregate);
    }
}
ArgumentId Analyzer::make_argument_pack(const std::vector<ArgumentId>& args)
{
    return types.compound(TypeKind::ArgumentPack,0,intern_arguments(args));
}
std::uint32_t Analyzer::expansion_parameters(ArgumentId pattern)
{
    if (auto old = expansion_parameter_index.get(pattern)) return old;
    std::vector<ArgumentId> work(1,pattern), result; Index seen, parameters;
    for (unsigned i = 0; i < work.size(); ++i) {
        auto arg = work[i];
        if (!arg || seen.get(arg)) continue;
        seen.put(arg,1); ++expansion_work;
        EntityId parameter = 0;
        if (value_argument(arg)) {
            auto q = type_queries[argument_query(arg)];
            if (q.kind == QueryKind::SizeofPack || q.kind == QueryKind::Expansion) continue;
            auto args = argument_packs[q.arguments];
            for (unsigned j = 0; j < args.count; ++j) work.push_back(argument_types[args.offset+j]);
            if (q.kind == QueryKind::TemplateValueParameter) parameter = q.entity;
            if (q.type) work.push_back(q.type);
            for (unsigned j = 0; j < q.count; ++j) work.push_back(0x80000000U|query_edges[q.offset+j]);
        } else {
            auto t = types[arg];
            if (t.kind == TypeKind::Named) {
                if (entities[t.entity].template_parameter) parameter = t.entity;
                else if (entities[t.entity].specialization) {
                    auto primary = specialization_pattern(t.entity);
                    if (entities[primary].template_parameter) work.push_back(entities[primary].type);
                    auto args = specialization_arguments(t.entity);
                    for (unsigned j = 0; j < args.count; ++j) work.push_back(argument_types[args.offset+j]);
                }
            }
            if (t.kind == TypeKind::PackExpansion) continue; // nested expansion owns its packs
            if (t.kind == TypeKind::ArgumentPack) {
                auto args = pack_arguments(arg);
                for (unsigned j = 0; j < args.count; ++j) work.push_back(argument_types[args.offset+j]);
            }
            if (t.kind == TypeKind::Decltype) work.push_back(0x80000000U|t.entity);
            if (t.kind == TypeKind::DependentArray) work.push_back(0x80000000U|t.bound);
            if (t.child) work.push_back(t.child);
            for (unsigned j = 0; j < t.count; ++j) work.push_back(types.parameters[t.offset+j]);
        }
        if (parameter && entities[parameter].parameter_pack && !parameters.get(parameter)) {
            parameters.put(parameter,1); result.push_back(parameter);
        }
    }
    auto id = intern_arguments(result); expansion_parameter_index.put(pattern,id); return id;
}
bool Analyzer::deduce_expansion(ArgumentId pattern, const std::vector<TypeId>& actual, Index& bindings, std::uint32_t prefix_frame, DeductionKind kind)
{
    auto list = argument_packs[expansion_parameters(pattern)];
    std::vector<std::vector<ArgumentId>> values(list.count);
    std::vector<ArgumentId> previous;
    for (unsigned j = 0; j < list.count; ++j) previous.push_back(bindings.get(argument_types[list.offset+j]));
    for (unsigned j = 0; prefix_frame && j < list.count; ++j) {
        auto prefix = unexpanded_argument(prefix_frame,argument_types[list.offset+j]);
        if (prefix && argument_pack(prefix) && pack_arguments(prefix).count > actual.size()) return false;
    }
    unsigned lane = 0;
    for (auto a : actual) {
        bool symbolic = !value_argument(a) && types[a].kind == TypeKind::PackExpansion;
        if (symbolic) a = types[a].bound;
        auto frame = prefix_frame;
        for (unsigned j = 0; j < list.count; ++j) {
            auto p = argument_types[list.offset+j]; ArgumentId fixed = 0;
            auto prefix = prefix_frame ? unexpanded_argument(prefix_frame,p) : 0;
            if (prefix && argument_pack(prefix)) {
                auto args = pack_arguments(prefix);
                if (lane < args.count) fixed = argument_types[args.offset+lane];
            }
            bindings.put(p,fixed);
            if (prefix_frame) frame = argument_frame(frame,p,fixed ? fixed : parameter_argument(p));
        }
        Index empty, cache;
        auto element = frame ? substitute_argument(pattern,empty,cache,frame) : pattern;
        // A braced list is a non-deduced lane. An explicit pack prefix may
        // already supply its complete target; otherwise deduction cannot bind it.
        if (!a && element && dependent_argument(element)) return false;
        if (!element || (dependent_argument(element) && !deduce_type(element,a,bindings,kind))) return false;
        if (!frame && !dependent_argument(element) && !deduce_type(element,a,bindings,kind)) return false;
        ++lane;
        for (unsigned j = 0; j < list.count; ++j) {
            auto value = bindings.get(argument_types[list.offset+j]);
            if (!value) return false;
            values[j].push_back(symbolic ? types.compound(TypeKind::PackExpansion,0,value) : value);
        }
    }
    for (unsigned j = 0; j < list.count; ++j) {
        auto value = make_argument_pack(values[j]);
        if (previous[j] && previous[j] != value) return false;
        bindings.put(argument_types[list.offset+j],value);
    }
    return true;
}
ArgumentId Analyzer::unexpanded_argument(std::uint32_t frame, EntityId parameter) const
{
    // Nested ellipses always select the whole pack; an outer lane remains an
    // overlay only for ordinary uses of the same parameter.
    while (frame && substitution_frames[frame].expansion) frame = substitution_frames[frame].parent;
    if (entities[parameter].template_parameter) return substitution_argument(frame,parameter);
    return entity_pack_arguments.get(substitution_entity(frame,parameter));
}
int Analyzer::expansion_count(std::uint32_t parameters, const Index& bindings, std::uint32_t frame)
{
    auto list = argument_packs[parameters]; int count = -1;
    for (unsigned i = 0; i < list.count; ++i) {
        auto parameter = argument_types[list.offset+i];
        auto arg = frame ? unexpanded_argument(frame,parameter) : bindings.get(parameter);
        if (!arg || !argument_pack(arg)) return -1;
        auto n = pack_arguments(arg).count;
        if (count >= 0 && unsigned(count) != n) throw std::runtime_error("pack expansion has unequal lengths");
        count = n;
    }
    return count;
}
std::uint32_t Analyzer::argument_frame(std::uint32_t parent, EntityId parameter, ArgumentId arg)
{
    auto pack = intern_arguments({parameter,arg}); auto k = key(parent,pack);
    if (auto known = argument_frame_index.get(k)) return known;
    TemplateSubstitutionFrame f; f.parent = parent; f.overlay = pack;
    auto id = substitution_frames.size(); substitution_frames.push_back(f);
    argument_frame_index.put(k,id); return id;
}
std::uint32_t Analyzer::expansion_context(std::uint32_t frame)
{
    if (auto context = substitution_frame_contexts.get(frame)) return context;
    auto context = ast.new_context(); attach_template_context(context,frame); return context;
}
std::uint32_t Analyzer::expansion_frame(std::uint32_t parent, std::uint32_t parameters, unsigned lane)
{
    auto list = argument_packs[parameters]; std::vector<ArgumentId> overlay; bool symbolic = false;
    for (unsigned i = 0; i < list.count; ++i) {
        auto parameter = argument_types[list.offset+i];
        auto arg = unexpanded_argument(parent,parameter);
        if (!argument_pack(arg)) throw std::logic_error("expansion lacks pack arguments");
        auto args = pack_arguments(arg);
        if (lane >= args.count) throw std::logic_error("pack expansion lane out of bounds");
        auto value = argument_types[args.offset+lane];
        // A function-parameter pack contains EntityIds, not type arguments.
        // Its lane binds the existing runtime object. Interpreting that ID as
        // a TypeId can alias an unrelated expansion or exceed the type arena.
        if (entities[parameter].template_parameter && !value_argument(value) && types[value].kind == TypeKind::PackExpansion) {
            symbolic = true; value = types[value].bound;
        }
        overlay.push_back(parameter); overlay.push_back(value);
    }
    auto pack = intern_arguments(overlay);
    auto identity = intern_arguments({pack,parameters,lane,symbolic ? 1u : 0u});
    auto k = key(parent,identity);
    if (auto old = expansion_frame_index.get(k)) return old;
    TemplateSubstitutionFrame f; f.parent = parent; f.overlay = pack; f.expansion = true; f.symbolic = symbolic;
    auto id = substitution_frames.size(); substitution_frames.push_back(f);
    expansion_frame_index.put(k,id); ++expansion_lanes; return id;
}
void Analyzer::substitute_arguments(ArgumentId arg, const Index& bindings, Index& cache,
    std::uint32_t frame, std::vector<ArgumentId>& out)
{
    if (!value_argument(arg) && types[arg].kind == TypeKind::PackExpansion) {
        auto pattern = types[arg].bound;
        auto params = expansion_parameters(pattern);
        auto count = expansion_count(params,bindings,frame);
        if (count >= 0 && frame) {
            for (int j = 0; j < count; ++j) {
                auto lane = expansion_frame(frame,params,j);
                auto value = substitute_argument(pattern,bindings,cache,lane);
                out.push_back(substitution_frames[lane].symbolic ? types.compound(TypeKind::PackExpansion,0,value) : value);
            }
            return;
        }
    }
    out.push_back(substitute_argument(arg,bindings,cache,frame));
}
} }
