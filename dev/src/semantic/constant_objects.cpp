#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
namespace {
std::uint64_t mix(std::uint64_t h, std::uint64_t x) { return (h ^ x) * 1099511628211ULL; }
}
Constant Analyzer::evaluated_object(TypeId t, const std::vector<EvaluatedPart>& parts)
{
    t = types.unqualified(t);
    std::uint64_t h = t;
    for (auto p : parts) {
        if (!p.value.valid) return Constant();
        h = mix(mix(mix(mix(h,p.selector),p.count),p.value.type),p.value.bits);
    }
    for (auto i = evaluated_object_index.get(h); i; i = evaluated_objects[i].next) {
        auto v = evaluated_objects[i];
        if (v.type != t || v.count != parts.size()) continue;
        bool same = true;
        for (unsigned j = 0; same && j < v.count; ++j) {
            auto a = evaluated_parts[v.first+j], b = parts[j];
            same = a.selector == b.selector && a.count == b.count && a.value.type == b.value.type && a.value.bits == b.value.bits;
        }
        if (same) return Constant(t,i);
    }
    EvaluatedObject v; v.type = t; v.first = evaluated_parts.size(); v.count = parts.size(); v.next = evaluated_object_index.get(h);
    auto id = evaluated_objects.size(); evaluated_objects.push_back(v);
    evaluated_parts.insert(evaluated_parts.end(),parts.begin(),parts.end());
    evaluated_object_index.put(h,id);
    if (types[t].kind != TypeKind::Array)
        for (unsigned i = 0; i < parts.size(); ++i) evaluated_part_index.put(key(id,parts[i].selector),v.first+i+1);
    constant_object_work += parts.size()+1;
    return Constant(t,id);
}
Constant Analyzer::evaluated_part(Constant v, std::uint64_t selector)
{
    if (!v.valid || (!class_value(v.type) && types[v.type].kind != TypeKind::Array)) return Constant();
    auto o = evaluated_objects[v.bits];
    if (types[v.type].kind != TypeKind::Array) {
        auto p = evaluated_part_index.get(key(v.bits,selector));
        return p ? evaluated_parts[p-1].value : Constant();
    }
    if (selector >= types[v.type].bound) return Constant();
    auto begin = evaluated_parts.begin()+o.first, end = begin+o.count;
    auto found = std::upper_bound(begin,end,selector,[](std::uint64_t i,const EvaluatedPart& p){return i < p.selector;});
    if (found == begin) return Constant();
    auto p = *--found;
    return selector-p.selector < p.count ? p.value : Constant();
}
Constant Analyzer::constant_zero(TypeId t)
{
    auto type = types[t];
    if (!class_value(t) && type.kind != TypeKind::Array) return convert(Constant(types.fundamental(FT_INT),0),t,true);
    std::vector<EvaluatedPart> parts;
    auto add = [&](std::uint64_t selector, TypeId child, std::uint64_t count) {
        EvaluatedPart p; p.selector = selector; p.count = count; p.value = constant_zero(child); parts.push_back(p);
    };
    if (type.kind == TypeKind::Array) add(0,type.child,type.bound);
    else {
        auto cls = type.entity;
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next)
            add(0x80000000U | bases[b].base,entities[bases[b].base].type,1);
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            auto f = declarations[d].entity;
            if (!nonstatic_field(f) || entities[f].owner != entities[cls].scope) continue;
            add(f,entities[f].type,1);
            if (entities[cls].key == KW_UNION) break;
        }
    }
    return evaluated_object(t,parts);
}
Constant Analyzer::constant_init_plan(std::uint32_t id, ScopeId s)
{
    auto a = initializers[id];
    if (a.kind == InitKind::Value) return value_constructor(a.type) ? constant_construct(value_constructor(a.type),{},true) : constant_zero(a.type);
    if (a.kind == InitKind::Constructor) return constant_initialize(a.source,a.type,s);
    if (a.kind == InitKind::Converted) return constant_node_conversion(a.source,conversions[a.conversion],s);
    if (a.kind == InitKind::Scalar) return constant_initialize(a.source,a.type,s);
    std::vector<EvaluatedPart> parts;
    if (a.kind == InitKind::String) {
        auto literal = ast[a.source].literal;
        for (std::uint64_t i = 0; i < ast.literals[literal].elements; ++i) {
            EvaluatedPart p; p.selector = i; p.value = literal_element(literal,Constant(types.fundamental(FT_UNSIGNED_LONG_INT),i)); parts.push_back(p);
        }
        if (types[a.type].bound > parts.size()) {
            EvaluatedPart p; p.selector = parts.size(); p.count = types[a.type].bound-p.selector; p.value = constant_zero(types[a.type].child); parts.push_back(p);
        }
    } else if (a.kind == InitKind::Group) {
        for (auto c = a.first; c; c = initializers[c].next) {
            auto child = initializers[c]; EvaluatedPart p;
            p.selector = child.field ? child.field : child.index; p.count = child.count;
            p.value = constant_init_plan(c,s); parts.push_back(p);
        }
    } else return Constant();
    return evaluated_object(a.type,parts);
}
Constant Analyzer::constant_initialize(NodeId n, TypeId t, ScopeId s, EntityId ctor)
{
    auto init = class_initialization(n,t);
    if (init.source) return constant_node_conversion(init.source,conversions[init.conversion],s);
    if (auto plan = initializer_plan(n,t)) return constant_init_plan(plan,s);
    if (!ctor && n && constructor_member(facts[n].entity)) ctor = facts[n].entity;
    if (ctor) {
        auto x = expressions[n]; std::vector<Constant> args;
        for (unsigned i = 0; i < x.argument_count; ++i) {
            auto v = constant_node_conversion(call_argument(x,i),conversions[x.conversions+i],s);
            if (!v.valid) return Constant();
            args.push_back(v);
        }
        return constant_construct(ctor,args,object_uses[x.object_use].value_initialize);
    }
    while (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenInitializer ||
        ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::BracedInit) {
        if (!ast[n].first) return constant_zero(t);
        n = ast[n].first;
    }
    if (!n) return Constant();
    auto c = conversions[expressions[n].incoming];
    if (!c.target) { c.target = t; c.reference = types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef; }
    return constant_node_conversion(n,c,s);
}
Constant Analyzer::constant_entity_value(EntityId e)
{
    if (!e) return Constant();
    if (entities[e].constant.valid) return entities[e].constant;
    auto state = constant_declaration_state.get(e);
    if (state == 1 || state == 3) return Constant();
    auto entity = entities[e]; auto t = entity.type;
    bool reference = types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef;
    if (!reference && (!constexpr_declarations.get(e) || constexpr_declarations.get(e) != 2) &&
        !(integral(t) && (types[t].cv & 1))) return Constant();
    if (!entity.initializer && !object_constructor(e)) { constant_unavailable = true; return Constant(); }
    constant_declaration_state.put(e,1);
    auto saved_destination = constant_destination;
    std::uint32_t address = 0;
    if (class_value(t) || types[t].kind == TypeKind::Array) {
        address = constant_entity_storage.get(e);
        if (!address) { address = constant_storage_address(t,Constant(),e,0,true); constant_entity_storage.put(e,address); }
        constant_destination = address;
    }
    Constant value;
    try { value = constant_initialize(entity.initializer,t,entity.owner,object_constructor(e)); }
    catch (...) { constant_destination = saved_destination; throw; }
    constant_destination = saved_destination;
    if (address) { auto storage = constant_addresses[address].storage; constant_storage[storage].value = value; constant_storage[storage].readable = value.valid; }
    constant_declaration_state.put(e,value.valid ? 2 : constant_unavailable ? 0 : 3);
    if (value.valid) entities[e].constant = value;
    return value;
}
bool Analyzer::constant_object_fields(Constant v, std::uint64_t offset)
{
    if (!v.valid) return false;
    if (class_value(v.type) || types[v.type].kind == TypeKind::Array) {
        auto object = evaluated_objects[v.bits];
        for (unsigned i = 0; i < object.count; ++i) {
            auto part = evaluated_parts[object.first+i];
            auto at = offset;
            if (types[v.type].kind == TypeKind::Array) at += part.selector * size(part.value.type);
            else if (part.selector & 0x80000000U) at += base_steps(v.type,types[part.value.type].entity)-1;
            else at += entities[part.selector].member_offset;
            for (std::uint64_t j = 0; j < part.count; ++j)
                if (!constant_object_fields(part.value,at+j*size(part.value.type))) return false;
        }
        return true;
    }
    auto scalar = constant_static_value(v);
    if (scalar.kind == StaticValue::Invalid) return false;
    constant_fields.push_back({0,v.type,scalar,offset}); return true;
}
void Analyzer::check_constant_object(EntityId e)
{
    auto value = constant_entity_value(e);
    if (!value.valid || !constant_persistent(value)) throw std::runtime_error("nonconstant constexpr object initializer");
}
} }
