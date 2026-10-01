#include "semantic/analyzer.h"
#include <sstream>
#include <stdexcept>
namespace cppgm { namespace semantic {
ScopeId Analyzer::function_context(ScopeId s) const
{
    while (s && scopes[s].kind != ScopeKind::Function) s = scopes[s].parent;
    return s;
}
void Analyzer::pretty_argument(std::ostream& out, ArgumentId arg)
{
    if (argument_pack(arg)) {
        auto pack = pack_arguments(arg); out << '<';
        for (unsigned j = 0; j < pack.count; ++j) {
            if (j) out << ", ";
            pretty_argument(out,argument_types[pack.offset+j]);
        }
        out << '>';
    } else if (value_argument(arg)) {
        auto value = constants[query_value(argument_query(arg))];
        if (!value.valid) throw std::logic_error("unresolved pretty-function argument");
        if (types[value.type].kind == TypeKind::LRef) {
            auto storage = constant_storage[constant_addresses[value.bits].storage];
            if (!storage.entity) throw std::logic_error("unnamed pretty-function reference");
            pretty_entity(out,storage.entity);
        } else if (fundamental(value.type,FT_BOOL)) out << (value.bits ? "true" : "false");
        else if (integral(value.type)) out << integer_text(value);
        else if (fundamental(value.type,FT_NULLPTR_T) || (pointer(value.type) && !value.bits)) out << "nullptr";
        else if (pointer(value.type)) {
            auto storage = constant_storage[constant_addresses[value.bits].storage];
            if (!storage.entity) throw std::logic_error("unnamed pretty-function address");
            out << '&'; pretty_entity(out,storage.entity);
        } else if (types[value.type].kind == TypeKind::MemberPointer) {
            if (!value.bits) out << "nullptr";
            else { out << '&'; pretty_entity(out,member_constant_value(value).member); }
        } else throw std::logic_error("unsupported pretty-function argument");
    } else pretty_type(out,arg);
}
void Analyzer::pretty_entity(std::ostream& out, EntityId e)
{
    auto owner = entities[e].owner;
    while (owner && owner != global && scopes[owner].kind != ScopeKind::Namespace &&
        scopes[owner].kind != ScopeKind::Class) owner = scopes[owner].parent;
    // The namespace edge is an indexed semantic fact, including reopenings.
    while (owner && owner != global && scopes[owner].kind == ScopeKind::Namespace &&
        edges[edge_index.get(key(scopes[owner].parent,owner))].inline_namespace)
        owner = scopes[owner].parent;
    if (owner && owner != global && scopes[owner].entity) {
        pretty_entity(out,scopes[owner].entity); out << "::";
    }
    auto conversion = members[entities[e].member_info].conversion_target;
    if (conversion) { out << "operator "; pretty_type(out,conversion); }
    else spelling(out,entities[e].name);
    auto spec = specializations[entities[e].specialization];
    if (spec.pattern && entities[e].kind != EntityKind::Function) {
        auto args = argument_packs[spec.arguments]; out << '<';
        for (unsigned j = 0; j < args.count; ++j) {
            if (j) out << ", ";
            pretty_argument(out,argument_types[args.offset+j]);
        }
        out << '>';
    }
}
void Analyzer::pretty_type_prefix(std::ostream& out, TypeId id)
{
    auto t = types[id];
    switch (t.kind) {
    case TypeKind::AliasApplication: pretty_type_prefix(out,t.child); return;
    case TypeKind::Pointer: case TypeKind::LRef: case TypeKind::RRef: case TypeKind::MemberPointer:
        pretty_type_prefix(out,t.child);
        if (types[t.child].kind == TypeKind::Array || types[t.child].kind == TypeKind::Function) out << '(';
        if (t.kind == TypeKind::MemberPointer) { out << ' '; pretty_type(out,t.member_owner()); out << "::*"; }
        else out << (t.kind == TypeKind::Pointer ? "*" : t.kind == TypeKind::LRef ? "&" : "&&");
        break;
    case TypeKind::Array: case TypeKind::Function: pretty_type_prefix(out,t.child); return;
    case TypeKind::Named:
        if (t.entity == placeholder_parameter) out << "auto";
        else pretty_entity(out,t.entity);
        break;
    case TypeKind::Fundamental: out << fundamental_name(t.fundamental); break;
    default: throw std::logic_error("dependent pretty-function type");
    }
    if (t.cv & 1) out << " const";
    if (t.cv & 2) out << " volatile";
}
void Analyzer::pretty_type_suffix(std::ostream& out, TypeId id)
{
    auto t = types[id];
    switch (t.kind) {
    case TypeKind::Pointer: case TypeKind::LRef: case TypeKind::RRef: case TypeKind::MemberPointer:
        if (types[t.child].kind == TypeKind::Array || types[t.child].kind == TypeKind::Function) out << ')';
        pretty_type_suffix(out,t.child); break;
    case TypeKind::Array:
        out << '['; if (t.bound) out << t.bound; out << ']'; pretty_type_suffix(out,t.child); break;
    case TypeKind::Function:
        pretty_parameters(out,t);
        pretty_type_suffix(out,t.child); break;
    case TypeKind::AliasApplication: pretty_type_suffix(out,t.child); break;
    default: break;
    }
}
void Analyzer::pretty_parameters(std::ostream& out, const Type& t)
{
    out << '(';
    for (unsigned j = 0; j < t.count; ++j) {
        if (j) out << ", ";
        pretty_type(out,types.parameters[t.offset+j]);
    }
    if (t.variadic) out << (t.count ? ", ..." : "...");
    out << ')';
    if (t.cv & 1) out << " const";
    if (t.cv & 2) out << " volatile";
    if (t.ref != RefQualifier::None) out << (t.ref == RefQualifier::Lvalue ? " &" : " &&");
}
void Analyzer::pretty_type(std::ostream& out, TypeId id)
{
    id = types.signature(id);
    pretty_type_prefix(out,id); pretty_type_suffix(out,id);
}
void Analyzer::pretty_bindings(std::ostream& out, EntityId e, bool& first)
{
    auto spec = specializations[entities[e].specialization];
    if (!spec.pattern) return;
    auto pattern = spec.definition_pattern ? spec.definition_pattern : spec.pattern;
    auto head = templates[entities[pattern].template_info];
    auto args = argument_packs[spec.definition_pattern ? spec.definition_arguments : spec.arguments];
    if (head.count != args.count) throw std::logic_error("pretty-function template argument shape");
    for (unsigned j = 0; j < head.count; ++j) {
        out << (first ? " [" : ", "); first = false;
        spelling(out,entities[template_parameters[head.offset+j]].name); out << " = ";
        pretty_argument(out,argument_types[args.offset+j]);
    }
}
EntityId Analyzer::function_name_string(ScopeId s, IdentifierId name, NodeId source)
{
    if (!s) throw std::runtime_error("predefined function name outside function");
    if (scopes[s].kind != ScopeKind::Function) throw std::logic_error("invalid function name context");
    if (auto old = local(s,name)) return old;
    auto fn = scopes[s].entity;
    IdentifierId content = entities[fn].name;
    if (ids.spelling(name).equals("__PRETTY_FUNCTION__")) {
        std::ostringstream out;
        auto f = types[entities[fn].type];
        auto member = members[entities[fn].member_info];
        if (!member.constructor && !member.destructor && !member.conversion_target) { pretty_type(out,f.child); out << ' '; }
        pretty_entity(out,fn);
        // Only the function declarator suffix belongs here, not its return type.
        pretty_parameters(out,f);
        std::vector<EntityId> owners;
        for (auto at = entities[fn].owner; at; at = scopes[at].parent)
            if (scopes[at].kind == ScopeKind::Class) owners.push_back(scopes[at].entity);
        bool first = true;
        for (auto at = owners.rbegin(); at != owners.rend(); ++at) pretty_bindings(out,*at,first);
        pretty_bindings(out,fn,first);
        if (!first) out << ']';
        auto value = out.str(); content = ids.intern(TextView(value.data(),value.size()));
    } else if (auto conversion = members[entities[fn].member_info].conversion_target) {
        std::ostringstream out; out << "operator "; pretty_type(out,conversion);
        auto value = out.str(); content = ids.intern(TextView(value.data(),value.size()));
    }
    auto text = ids.spelling(content);
    auto e = make_entity(EntityKind::Variable,s,name,source);
    entities[e].type = types.compound(TypeKind::Array,types.qualify(types.fundamental(FT_CHAR),1),text.size+1);
    entities[e].is_static = true;
    entities[e].definition = source ? source : entities[fn].definition ? entities[fn].definition : entities[fn].source;
    predefined_strings.put(e,content); bind(s,name,e);
    return e;
}
} }
