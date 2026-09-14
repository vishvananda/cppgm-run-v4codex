#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::refresh_constant_storage(std::uint32_t id)
{
    auto& storage = constant_storage[id];
    if (!storage.entity || storage.readable || !entities[storage.entity].constant.valid) return;
    // A prior address can precede its declaration's initializer. Only this
    // storage changes when that initializer publishes a completed value.
    storage.value = entities[storage.entity].constant;
    storage.readable = true; ++storage.version;
}
std::uint32_t Analyzer::constant_temporary_address(TypeId t, Constant value, EntityId temporary)
{
    if (!temporary || !static_temporary(temporary).object) return constant_storage_address(t,value,0,0,value.valid);
    auto address = constant_entity_storage.get(temporary);
    if (!address) {
        address = constant_storage_address(t,value,temporary,0,value.valid);
        constant_entity_storage.put(temporary,address);
    }
    if (value.valid) {
        auto storage = constant_addresses[address].storage;
        constant_storage[storage].value = value; constant_storage[storage].readable = true;
        entities[temporary].constant = value;
    }
    return address;
}
std::uint32_t Analyzer::constant_storage_address(TypeId t, Constant value, EntityId e, NodeId literal, bool readable)
{
    ConstantStorage storage; storage.type = t; storage.entity = e; storage.literal = literal;
    storage.value = value; storage.readable = readable;
    auto id = constant_storage.size(); constant_storage.push_back(storage);
    ConstantAddress address; address.storage = id; address.type = t;
    auto result = constant_addresses.size(); constant_addresses.push_back(address); ++constant_address_work;
    return result;
}
std::uint32_t Analyzer::constant_subobject(std::uint32_t parent, TypeId t, std::uint64_t selector)
{
    if (!parent) return 0;
    auto h = (std::uint64_t(parent) * 1099511628211ULL) ^ selector;
    for (auto i = constant_address_index.get(h); i; i = constant_addresses[i].next)
        if (constant_addresses[i].parent == parent && constant_addresses[i].selector == selector) return i;
    ConstantAddress a; a.parent = parent; a.storage = constant_addresses[parent].storage;
    a.type = t; a.selector = selector; a.next = constant_address_index.get(h);
    auto id = constant_addresses.size(); constant_addresses.push_back(a); constant_address_index.put(h,id);
    ++constant_address_work; return id;
}
std::uint64_t Analyzer::constant_offset(std::uint32_t id)
{
    auto a = constant_addresses[id];
    if (!a.parent || a.located) return a.offset;
    auto parent = constant_addresses[a.parent];
    auto offset = constant_offset(a.parent);
    if (types[parent.type].kind == TypeKind::Array) offset += a.selector * size(a.type);
    else if (a.selector == ~std::uint64_t(0)) offset += size(a.type);
    else if (a.selector & 0x80000000U) offset += base_steps(parent.type,types[a.type].entity)-1;
    else { size(parent.type); offset += entities[a.selector].member_offset; }
    constant_addresses[id].offset = offset; constant_addresses[id].located = true; return offset;
}
std::uint32_t Analyzer::constant_entity_address(EntityId e)
{
    if (!e) return 0;
    if (constant_frame) {
        auto slot = constant_frame->bindings.get(e);
        if (slot) {
            auto v = constant_frame->values[slot];
            auto kind = types[entities[e].type].kind;
            if (kind == TypeKind::LRef || kind == TypeKind::RRef) return v.valid ? v.bits : 0;
            auto known = constant_frame->addresses.get(e);
            if (known) return known;
            auto id = constant_storage_address(entities[e].type,v);
            constant_frame->addresses.put(e,id); constant_frame->storage.push_back(constant_addresses[id].storage);
            return id;
        }
        if (entities[e].kind == EntityKind::Parameter) return 0;
    }
    if (auto known = constant_entity_storage.get(e)) return known;
    auto entity = entities[e]; auto kind = types[entity.type].kind;
    if (kind == TypeKind::LRef || kind == TypeKind::RRef) {
        auto v = constant_entity_value(e); return v.valid ? v.bits : 0;
    }
    if (nonstatic_field(e)) return 0;
    auto v = constant_entity_value(e);
    if (auto known = constant_entity_storage.get(e)) return known;
    auto result = constant_storage_address(entity.type,v,e,0,v.valid);
    constant_entity_storage.put(e,result); return result;
}
Constant Analyzer::constant_read(std::uint32_t id)
{
    if (!id) return Constant();
    auto a = constant_addresses[id]; refresh_constant_storage(a.storage);
    auto storage = constant_storage[a.storage];
    if (storage.live && types[a.type].kind == TypeKind::Function) return Constant(types.compound(TypeKind::Pointer,a.type),id);
    if (storage.builder && a.parent && !constant_addresses[a.parent].parent) {
        auto slot = storage.builder->slots.get(a.selector);
        return slot ? storage.builder->parts[slot-1].value : Constant();
    }
    if (!storage.live || !storage.readable || (types[a.type].cv & 2)) return Constant();
    if (!a.parent) return storage.value;
    if (a.selector == ~std::uint64_t(0)) return Constant();
    if (types[constant_addresses[a.parent].type].kind == TypeKind::Array && storage.literal && !constant_addresses[a.parent].parent)
        return literal_element(ast[storage.literal].literal,Constant(types.fundamental(FT_UNSIGNED_LONG_INT),a.selector));
    if (!(a.selector & 0x80000000U) && types[constant_addresses[a.parent].type].kind != TypeKind::Array && entities[a.selector].mutable_field)
        return Constant();
    auto v = evaluated_part(constant_read(a.parent),a.selector);
    if (v.valid && pointer(v.type) && v.bits && !constant_storage[constant_addresses[v.bits].storage].live) return Constant();
    return v;
}
Constant Analyzer::constant_indirect(Constant v)
{
    if (v.valid && pointer(v.type) && v.bits && !constant_storage[constant_addresses[v.bits].storage].live) return Constant();
    if (v.valid && (types[v.type].kind == TypeKind::LRef || types[v.type].kind == TypeKind::RRef)) return constant_read(v.bits);
    return v;
}
std::uint32_t Analyzer::constant_base_address(std::uint32_t id, TypeId target)
{
    if (!id) return 0;
    target = value_type(target);
    auto type = types[constant_addresses[id].type];
    if (types.unqualified(constant_addresses[id].type) == types.unqualified(target)) return id;
    if (!class_value(target) || type.kind != TypeKind::Named) return 0;
    // Downcasts recover the enclosing typed subobject, retaining its complete
    // storage identity. Upcasts visit only language-related base edges.
    for (auto p = constant_addresses[id].parent; p; p = constant_addresses[p].parent)
        if (types.unqualified(constant_addresses[p].type) == types.unqualified(target)) return p;
    for (auto b = class_facts[entities[type.entity].class_info].first_base; b; b = bases[b].next) {
        auto bt = entities[bases[b].base].type;
        if (types.unqualified(bt) != types.unqualified(target) && !class_derives(bases[b].base,types[target].entity)) continue;
        auto child = constant_subobject(id,bt,0x80000000U | bases[b].base);
        return constant_base_address(child,target);
    }
    return 0;
}
std::uint32_t Analyzer::constant_address(NodeId n, ScopeId s)
{
    auto x = expressions[n]; auto first = ast[n].first;
    if (ast[n].kind == Kind::Parenthesized) return constant_address(first,s);
    if (ast[n].kind == Kind::Binary && ast[n].op == OP_COMMA && x.form != ExpressionForm::OperatorCall) {
        if (!evaluate(first,s).valid) return 0;
        return constant_address(ast[first].next,s);
    }
    if (x.category == ValueCategory::Prvalue && class_value(x.type)) {
        auto temporary = object_fact(n).temporary;
        auto known = constant_entity_storage.get(temporary);
        if (known && constant_storage[constant_addresses[known].storage].readable) return known;
        auto address = constant_temporary_address(x.type,Constant(),temporary);
        auto saved = constant_destination; constant_destination = address;
        Constant value;
        try { value = (ast[n].kind == Kind::Call || x.form == ExpressionForm::OperatorCall) ? constant_call_result(n,s) : evaluate(n,s); }
        catch (...) { constant_destination = saved; throw; }
        constant_destination = saved;
        auto storage = constant_addresses[address].storage;
        constant_storage[storage].value = value; constant_storage[storage].readable = value.valid;
        if (value.valid && temporary && static_temporary(temporary).object) entities[temporary].constant = value;
        return value.valid ? address : 0;
    }
    if (x.form == ExpressionForm::OperatorCall) {
        auto value = constant_call(n,s);
        if (!value.valid) return 0;
        return types[value.type].kind == TypeKind::LRef || types[value.type].kind == TypeKind::RRef ? value.bits : constant_storage_address(value.type,value);
    }
    if (ast[n].kind == Kind::Literal && ast.literals[ast[n].literal].kind == LiteralKind::string) {
        if (auto known = constant_literal_storage.get(ast[n].literal)) return known;
        auto id = constant_storage_address(x.type,Constant(),0,n,true);
        constant_literal_storage.put(ast[n].literal,id); return id;
    }
    if (ast[n].kind == Kind::IdExpression && !nonstatic_field(x.entity)) return constant_entity_address(x.entity);
    if ((ast[n].kind == Kind::IdExpression || ast[n].kind == Kind::Member) && nonstatic_field(x.entity)) {
        auto use = object_uses[x.object_use];
        auto base = use.node ? constant_arrow(use.node,use.arrow) : active_constant ? constant_activations[active_constant].object : 0;
        if (!base) return 0;
        base = constant_base_address(base,entities[scopes[entities[x.entity].owner].entity].type);
        auto result = constant_subobject(base,entities[x.entity].type,x.entity);
        auto kind = types[entities[x.entity].type].kind;
        if (kind == TypeKind::LRef || kind == TypeKind::RRef) { auto v = constant_read(result); return v.valid ? v.bits : 0; }
        return result;
    }
    if (ast[n].kind == Kind::Member && x.entity && entities[x.entity].is_static) {
        if (!constant_arrow(first,object_uses[x.object_use].arrow)) return 0;
        return constant_entity_address(x.entity);
    }
    if (ast[n].kind == Kind::Unary && ast[n].op == OP_STAR) {
        auto v = constant_indirect(evaluate(first,s)); return v.valid && pointer(v.type) ? v.bits : 0;
    }
    if (ast[n].kind == Kind::Binary && (ast[n].op == OP_DOTSTAR || ast[n].op == OP_ARROWSTAR)) {
        auto member = evaluate(ast[first].next,s);
        if (!member.valid || !member.bits || types[member.type].kind != TypeKind::MemberPointer || !nonstatic_field(member.bits)) return 0;
        auto base = constant_node_object(first);
        base = constant_base_address(base,entities[types[member.type].entity].type);
        return constant_subobject(base,entities[member.bits].type,member.bits);
    }
    if (ast[n].kind == Kind::Subscript) {
        auto base = first, index = ast[first].next;
        if (!pointer(decay(expressions[base].type))) std::swap(base,index);
        Conversion c; c.target = decay(expressions[base].type);
        auto p = constant_node_conversion(base,c,s), i = evaluate(index,s);
        auto v = constant_pointer_binary(OP_PLUS,p,i);
        return v.valid ? v.bits : 0;
    }
    if (ast[n].kind == Kind::Conditional) {
        auto c = conversions[expressions[n].conversions];
        auto cond = c.target ? constant_node_conversion(first,c,s) : evaluate(first,s);
        if (!cond.valid) return 0;
        auto yes = ast[first].next;
        return constant_address(constant_truth(cond) ? yes : ast[yes].next,s);
    }
    if (ast[n].kind == Kind::Cast) {
        auto v = constant_node_conversion(ast[first].next,conversions[x.conversions],s);
        if (v.valid && (types[v.type].kind == TypeKind::LRef || types[v.type].kind == TypeKind::RRef)) return v.bits;
        return v.valid ? constant_storage_address(x.type,v) : 0;
    }
    auto value = (ast[n].kind == Kind::Call || x.form == ExpressionForm::OperatorCall) ? constant_call_result(n,s) : evaluate(n,s);
    if (!value.valid) return 0;
    if (types[value.type].kind == TypeKind::LRef || types[value.type].kind == TypeKind::RRef) return value.bits;
    return constant_storage_address(x.type,value);
}
Constant Analyzer::constant_pointer_binary(ETokenType op, Constant a, Constant b)
{
    a = constant_indirect(a); b = constant_indirect(b);
    if (!a.valid || !b.valid) return Constant();
    if (op == OP_PLUS && !pointer(a.type)) std::swap(a,b);
    if (pointer(a.type) && integral(b.type) && (op == OP_PLUS || op == OP_MINUS)) {
        __int128 delta = is_unsigned(b.type) ? __int128(b.bits) : __int128(std::int64_t(b.bits));
        if (op == OP_MINUS) delta = -delta;
        if (!a.bits) return delta ? Constant() : a;
        auto address = constant_addresses[a.bits];
        auto parent = address.parent;
        bool element = parent && types[constant_addresses[parent].type].kind == TypeKind::Array;
        bool onepast = address.selector == ~std::uint64_t(0);
        __int128 at = (element ? address.selector : onepast ? 1 : 0) + delta;
        auto bound = element ? types[constant_addresses[parent].type].bound : 1;
        if (at < 0 || at > bound) return Constant();
        if (element) return Constant(a.type,constant_subobject(parent,types[a.type].child,std::uint64_t(at)));
        if (!at) return onepast ? Constant(a.type,parent) : a;
        if (onepast) return a;
        // A nonarray object behaves as an array of length one. The one-past
        // address has a distinct path and cannot be read as a subobject.
        return Constant(a.type,constant_subobject(a.bits,types[a.type].child,~std::uint64_t(0)));
    }
    bool ap = pointer(a.type) || fundamental(a.type,FT_NULLPTR_T), bp = pointer(b.type) || fundamental(b.type,FT_NULLPTR_T);
    if (!ap || !bp) return Constant();
    if (op == OP_EQ || op == OP_NE) {
        bool same = a.bits == b.bits;
        if (!same && a.bits && b.bits && constant_addresses[a.bits].storage == constant_addresses[b.bits].storage)
            same = constant_offset(a.bits) == constant_offset(b.bits);
        return Constant(types.fundamental(FT_BOOL),op == OP_EQ ? same : !same);
    }
    if (!a.bits || !b.bits) return Constant();
    auto x = constant_addresses[a.bits], y = constant_addresses[b.bits];
    if (x.storage == y.storage) {
        auto owner = [&](std::uint32_t id) {
            auto p = constant_addresses[id];
            return p.selector == ~std::uint64_t(0) ? p.parent : id;
        };
        if (owner(a.bits) == owner(b.bits)) {
            auto left = x.selector == ~std::uint64_t(0), right = y.selector == ~std::uint64_t(0);
            if (op == OP_MINUS) return Constant(types.fundamental(FT_LONG_INT),std::int64_t(left)-std::int64_t(right));
            return binary(op,Constant(types.fundamental(FT_INT),left),Constant(types.fundamental(FT_INT),right));
        }
    }
    if (x.parent != y.parent || x.storage != y.storage || !x.parent || types[constant_addresses[x.parent].type].kind != TypeKind::Array) return Constant();
    if (op == OP_MINUS) return Constant(types.fundamental(FT_LONG_INT),x.selector-y.selector);
    return binary(op,Constant(types.fundamental(FT_UNSIGNED_LONG_INT),x.selector),Constant(types.fundamental(FT_UNSIGNED_LONG_INT),y.selector));
}
bool Analyzer::constant_persistent(Constant v)
{
    if (!v.valid) return false;
    auto persistent_address = [&](Constant value) {
        if (!value.bits) return types[value.type].kind == TypeKind::Pointer;
        auto s = constant_storage[constant_addresses[value.bits].storage];
        return s.live && (s.literal || (s.entity && (entities[s.entity].is_static ||
            entities[s.entity].kind == EntityKind::Function || scopes[entities[s.entity].owner].kind == ScopeKind::Namespace)));
    };
    auto k = types[v.type].kind;
    if (k == TypeKind::Pointer || k == TypeKind::LRef || k == TypeKind::RRef) return persistent_address(v);
    if ((!class_value(v.type) && k != TypeKind::Array) || !evaluated_objects[v.bits].address_count) return true;
    std::vector<Constant> work(1,v); Index seen;
    while (!work.empty()) {
        auto value = work.back(); work.pop_back();
        auto kind = types[value.type].kind;
        if (class_value(value.type) || kind == TypeKind::Array) {
            if (seen.get(value.bits)) continue;
            seen.put(value.bits,1); ++constant_persistence_work;
            auto object = evaluated_objects[value.bits];
            for (unsigned i = 0; i < object.address_count; ++i)
                work.push_back(evaluated_parts[evaluated_address_parts[object.addresses+i]].value);
        } else if (!persistent_address(value)) return false;
    }
    return true;
}
StaticValue Analyzer::constant_static_value(Constant v)
{
    StaticValue r;
    if (!v.valid) return r;
    auto k = types[v.type].kind;
    if (k == TypeKind::Pointer || k == TypeKind::LRef || k == TypeKind::RRef) {
        if (!constant_persistent(v)) return r;
        if (!v.bits) { r.kind = StaticValue::Integer; return r; }
        auto a = constant_addresses[v.bits]; auto storage = constant_storage[a.storage];
        r.kind = storage.literal ? StaticValue::String : StaticValue::Address; r.entity = storage.entity; r.string = storage.literal;
        r.addend = constant_offset(v.bits);
    } else if (k == TypeKind::MemberPointer) {
        if (types[types[v.type].child].kind == TypeKind::Function) {
            r.kind = StaticValue::MemberFunction; r.entity = v.bits;
        } else {
            r.kind = StaticValue::Integer; r.bits = v.bits ? entities[v.bits].member_offset : ~std::uint64_t(0);
        }
    } else if (floating_type(v.type)) { r.kind = StaticValue::Floating; r.floating = floating_value(v); }
    else if (integral(v.type) || fundamental(v.type,FT_NULLPTR_T)) { r.kind = StaticValue::Integer; r.bits = v.bits; }
    return r;
}
} }

namespace cppgm { namespace semantic {
void Analyzer::constant_dependencies(Constant value, std::vector<ArgumentId>& args, Index& seen)
{
    if (!value.valid) return;
    auto k = types[value.type].kind;
    if ((k == TypeKind::Pointer || k == TypeKind::LRef || k == TypeKind::RRef) && value.bits) {
        auto storage_id = constant_addresses[value.bits].storage;
        if (seen.get(storage_id)) return;
        seen.put(storage_id,1); ++constant_dependency_work; refresh_constant_storage(storage_id);
        auto storage = constant_storage[storage_id];
        args.push_back(storage_id); args.push_back(storage.version);
        args.push_back(unsigned(storage.live) | (unsigned(storage.readable)<<1));
        constant_dependencies(storage.value,args,seen);
        if (storage.builder) for (auto part : storage.builder->parts) constant_dependencies(part.value,args,seen);
    } else if (class_value(value.type) || k == TypeKind::Array) {
        auto object_key = key(1,value.bits);
        if (seen.get(object_key)) return;
        seen.put(object_key,1); ++constant_dependency_work;
        auto o = evaluated_objects[value.bits];
        for (unsigned i = 0; i < o.address_count; ++i) constant_dependencies(evaluated_parts[evaluated_address_parts[o.addresses+i]].value,args,seen);
    }
}
} }
