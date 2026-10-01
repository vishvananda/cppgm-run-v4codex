#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
syntax::FunctionEffects Analyzer::function_effects(EntityId e) const
{
    auto effect = entities[e].effects;
    // An instance may precede a stronger redeclaration of its template.
    // Follow its canonical pattern edge at publication, without retrying or
    // invalidating the completed body/signature of any specialization.
    while (entities[e].specialization) {
        e = specializations[entities[e].specialization].pattern;
        effect = std::max(effect,entities[e].effects);
    }
    return effect;
}
void Analyzer::inherit_native_attributes(EntityId e, EntityId pattern)
{
    entities[e].effects = std::max(entities[e].effects,entities[pattern].effects);
    for (auto t = abi_tag_heads.get(pattern); t; t = abi_tags[t].next) {
        auto tag = abi_tags[t].name; auto k = key(e,tag);
        if (abi_tag_members.get(k)) continue;
        abi_tag_members.put(k,1);
        abi_tags.push_back({tag,abi_tag_heads.get(e)}); abi_tag_heads.put(e,abi_tags.size()-1);
    }
}
void Analyzer::native_attributes(EntityId e, NodeId n)
{
    if (!n) return;
    auto a = ast.native_attribute_owners.get(ast.nodes.occurrences[n].source);
    if (!a) return;
    auto value = ast.native_attributes[a];
    for (auto t = value.tags; t; t = ast.abi_tags[t].next) {
        auto tag = ast.abi_tags[t].name; auto k = key(e,tag);
        if (abi_tag_members.get(k)) continue;
        abi_tag_members.put(k,1);
        abi_tags.push_back({tag,abi_tag_heads.get(e)}); abi_tag_heads.put(e,abi_tags.size()-1);
    }
    if (entities[e].kind == EntityKind::Function) entities[e].effects = std::max(entities[e].effects,value.effects);
    if (value.weak) weak_symbols.put(e,1);
    if (value.no_unique_address && entities[e].kind == EntityKind::Variable) field_metadata(e).no_unique_address = true;
    if (!value.section) return;
    const auto& entity = entities[e];
    if (entity.kind != EntityKind::Variable ||
        (scopes[entity.owner].kind != ScopeKind::Namespace && !entity.is_static))
        throw std::runtime_error("section requires static storage object");
    auto prior = section_names.get(e);
    if (prior && prior != value.section) throw std::runtime_error("conflicting section redeclaration");
    section_names.put(e,value.section);
}
void Analyzer::declaration_attributes(EntityId e, NodeId specs, NodeId source, NodeId declarator)
{
    for (auto n : {source,specs,declarator}) native_attributes(e,n);
    entities[e].c_linkage |= c_linkage;
    for (auto n : {source,specs,declarator}) if (n) {
        entities[e].no_inline |= ast[n].flags & 64;
        entities[e].force_inline |= ast[n].flags & 128;
    }
    if (calls && (ast[source].flags & 16)) {
        Type f = types[entities[e].type];
        if (f.kind != TypeKind::Function || f.variadic || !f.count || !(arithmetic(f.child) || integral(f.child) || pointer(f.child)) ||
            !integral(types.parameters[f.offset+f.count-1])) throw std::runtime_error("invalid stable-prefix query signature");
        entities[e].stable_prefix = true;
    }
    if (entities[e].kind == EntityKind::Function) {
        bool constant = spec_has(specs,KW_CONSTEXPR) ||
            spec_has(child(source,Kind::MemberSpecifiers),KW_CONSTEXPR);
        if (calls && !entities[e].specialization && !entities[e].template_member) {
            auto known = constexpr_declarations.get(e);
            if (known && known != (constant ? 2U : 1U)) throw std::runtime_error("inconsistent constexpr declaration");
            constexpr_declarations.put(e,constant ? 2 : 1);
        }
        entities[e].constexpr_function |= constant;
        entities[e].inline_function |= spec_has(specs, KW_INLINE) || spec_has(specs, KW_CONSTEXPR);
        entities[e].inline_function |= spec_has(child(source, Kind::MemberSpecifiers), KW_INLINE) || entities[e].constexpr_function;
    }
    if (entities[e].kind == EntityKind::Variable && spec_has(specs,KW_CONSTEXPR)) constexpr_declarations.put(e,2);
    entities[e].thread_local_storage |= spec_has(specs, KW_THREAD_LOCAL);
    entities[e].external_decl |= spec_has(specs, KW_EXTERN) || linkage_extern_declarations.get(source);
}
} }
