#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
bool Analyzer::deduce_sequence(const std::vector<ArgumentId>& pattern, const std::vector<ArgumentId>& actual,
    Index& bindings, DeductionKind kind)
{
    auto expansion = [&](ArgumentId arg) { return !value_argument(arg) && types[arg].kind == TypeKind::PackExpansion; };
    unsigned fixed = pattern.size();
    bool pack = fixed && expansion(pattern.back());
    if (pack) --fixed;
    if (actual.size() < fixed || (!pack && actual.size() != fixed)) return false;
    for (unsigned j = 0; j < fixed; ++j) {
        if (expansion(actual[j]) || !deduce_type(pattern[j],actual[j],bindings,kind)) return false;
    }
    return !pack || deduce_expansion(types[pattern.back()].bound,
        std::vector<ArgumentId>(actual.begin()+fixed,actual.end()),bindings,0,kind);
}
bool Analyzer::deduce_type(TypeId pattern, TypeId actual, Index& bindings, DeductionKind kind)
{
    if (!value_argument(pattern) && types[pattern].kind == TypeKind::AliasApplication)
        return deduce_type(types.qualify(types[pattern].child,types[pattern].cv),actual,bindings,kind);
    if (!value_argument(actual) && types[actual].kind == TypeKind::AliasApplication)
        return deduce_type(pattern,types.qualify(types[actual].child,types[actual].cv),bindings,kind);
    if (value_argument(pattern) || value_argument(actual)) {
        if (!value_argument(pattern) || !value_argument(actual)) return false;
        auto q = type_queries[argument_query(pattern)];
        while (q.kind == QueryKind::Cast && q.op == TOK_INVALID) q = type_queries[query_edges[q.offset]];
        if (q.kind == QueryKind::TemplateValueParameter) {
            auto previous = bindings.get(q.entity);
            if (previous && previous != actual) return false;
            bindings.put(q.entity,actual); return true;
        }
        return pattern == actual || dependent_argument(pattern);
    }
    Type p = types[pattern], a = types[actual];
    if (p.kind == TypeKind::ArgumentPack) {
        if (a.kind != TypeKind::ArgumentPack) return false;
        auto x = argument_packs[p.bound], y = argument_packs[a.bound];
        return deduce_sequence(std::vector<ArgumentId>(argument_types.begin()+x.offset,argument_types.begin()+x.offset+x.count),
            std::vector<ArgumentId>(argument_types.begin()+y.offset,argument_types.begin()+y.offset+y.count),bindings,kind);
    }
    if (p.kind == TypeKind::DependentArray) {
        if (a.kind != TypeKind::Array && a.kind != TypeKind::DependentArray) return false;
        ArgumentId bound;
        if (a.kind == TypeKind::DependentArray) bound = value_argument_id(a.bound);
        else {
            TypeQuery q; q.type = types.fundamental(FT_UNSIGNED_LONG_INT); q.value = a.bound;
            bound = value_argument_id(intern_query(q,{}));
        }
        auto query = type_queries[p.bound];
        if (query.kind == QueryKind::TemplateValueParameter) {
            bound = convert_argument(bound,query.type);
            if (!bound) return false;
        }
        return deduce_type(value_argument_id(p.bound),bound,bindings,kind) && deduce_type(p.child,a.child,bindings,kind);
    }
    if (p.kind == TypeKind::DependentName || p.kind == TypeKind::Decltype) return true; // non-deduced context
    if (p.kind == TypeKind::Named && entities[p.entity].template_parameter) {
        if (entities[p.entity].key == KW_TEMPLATE && (a.kind != TypeKind::Named || !template_compatible(p.entity,a.entity,bindings))) return false;
        TypeId old = bindings.get(p.entity);
        auto element = actual;
        while (types[element].kind == TypeKind::Array || types[element].kind == TypeKind::DependentArray) element = types[element].child;
        auto cv = types[element].cv;
        if (kind != DeductionKind::Call && ((p.cv & cv) != p.cv ||
            (p.cv && (a.kind == TypeKind::Function || a.kind == TypeKind::LRef || a.kind == TypeKind::RRef)))) return false;
        TypeId value = actual;
        if (p.cv && a.kind != TypeKind::Function) {
            value = types.qualify(types.unqualified(element),cv & ~p.cv);
            std::vector<Type> arrays;
            for (auto t = actual; t != element; t = types[t].child) arrays.push_back(types[t]);
            for (auto t = arrays.rbegin(); t != arrays.rend(); ++t) value = types.compound(t->kind,value,t->bound);
        }
        if (old && old != value) return false;
        bindings.put(p.entity, value); return true;
    }
    if (p.kind != a.kind) return false;
    if (kind != DeductionKind::Call && p.cv != a.cv) return false;
    if (p.kind == TypeKind::Array && p.bound != a.bound) return false;
    if (p.kind == TypeKind::MemberPointer && p.entity != a.entity) return false;
    if (p.kind == TypeKind::Named && entities[p.entity].specialization && entities[a.entity].specialization &&
        entities[specialization_pattern(p.entity)].template_parameter) {
        auto ps = specializations[entities[p.entity].specialization], as = specializations[entities[a.entity].specialization];
        if (!deduce_type(entities[ps.pattern].type,types.named(as.pattern),bindings,kind)) return false;
        auto flatten = [&](std::uint32_t id) {
            std::vector<ArgumentId> out; auto pack = argument_packs[id];
            for (unsigned j = 0; j < pack.count; ++j) {
                auto arg = argument_types[pack.offset+j];
                if (argument_pack(arg)) {
                    auto slice = pack_arguments(arg);
                    out.insert(out.end(),argument_types.begin()+slice.offset,argument_types.begin()+slice.offset+slice.count);
                } else out.push_back(arg);
            }
            return out;
        };
        auto x = flatten(ps.arguments), y = flatten(as.arguments);
        bool pack = !x.empty() && !value_argument(x.back()) && types[x.back()].kind == TypeKind::PackExpansion;
        if (!pack && y.size() > x.size()) y.resize(x.size()); // compatible head supplied the omitted defaults
        return deduce_sequence(x,y,bindings,kind);
    }
    if (p.kind == TypeKind::Named && entities[p.entity].specialization && entities[a.entity].class_info) {
        auto ps = specializations[entities[p.entity].specialization];
        auto same_primary = [&](EntityId entity) {
            return entities[entity].specialization && specializations[entities[entity].specialization].pattern == ps.pattern;
        };
        auto match = [&](EntityId entity, Index& trial) {
            auto as = specializations[entities[entity].specialization];
            auto x = argument_packs[ps.arguments], y = argument_packs[as.arguments];
            if (x.count != y.count) return false;
            for (unsigned j = 0; j < x.count; ++j)
                if (!deduce_type(argument_types[x.offset+j],argument_types[y.offset+j],trial,kind)) return false;
            return true;
        };
        if (same_primary(a.entity)) {
            Index direct = bindings;
            if (match(a.entity,direct)) { bindings = std::move(direct); return true; }
        }
        if (kind != DeductionKind::Call) return false;
        // [temp.deduct.call]/4-5: only failed direct deduction admits base
        // alternatives, including another specialization of the same primary.
        // Visit each reachable class once; repeated paths to one base type do
        // not create a second deduced A. Conversion separately checks subobjects.
        std::vector<EntityId> pending(1,a.entity); Index visited, selected;
        visited.put(a.entity,1); unsigned matches = 0;
        while (!pending.empty()) {
            auto entity = pending.back(); pending.pop_back(); complete_class(entity);
            for (auto b = class_facts[entities[entity].class_info].first_base; b; b = bases[b].next) {
                auto base = bases[b].base;
                if (visited.get(base)) continue;
                visited.put(base,1);
                if (!same_primary(base)) { pending.push_back(base); continue; }
                Index trial = bindings;
                if (match(base,trial)) {
                    if (++matches > 1) return false;
                    selected = std::move(trial);
                } else pending.push_back(base);
            }
        }
        if (matches) bindings = std::move(selected);
        return matches != 0;
    }
    if (p.kind == TypeKind::Function) {
        if (p.variadic != a.variadic || p.cv != a.cv || p.ref != a.ref) return false;
        if (!deduce_sequence(std::vector<ArgumentId>(types.parameters.begin()+p.offset,types.parameters.begin()+p.offset+p.count),
            std::vector<ArgumentId>(types.parameters.begin()+a.offset,types.parameters.begin()+a.offset+a.count),bindings,kind)) return false;
    }
    if (p.child) return deduce_type(p.child, a.child, bindings,kind);
    return types.unqualified(pattern) == types.unqualified(actual);
}
} }
