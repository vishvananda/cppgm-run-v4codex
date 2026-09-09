#include "lowering/procedural.h"
#include <stdexcept>
#include <cstring>
namespace cppgm { namespace lowering {
using syntax::Kind;
using namespace lowir_model;
void Procedural::string_literal(NodeId n)
{
    const auto lit = ast.literals[ast[n].literal];
    Global g; g.structured = true;
    g.symbol = fresh_symbol("@__string_" + std::to_string(p.symbols.size()+1));
    strings[n] = g.symbol;
    g.data.begin = p.data.size(); g.data.count = lit.elements;
    TypeId element = sem.types.fundamental(lit.type);
    for (unsigned j = 0; j < lit.elements; ++j) {
        DataItem d; d.kind = DataItem::Scalar; d.type = type(element);
        std::uint64_t bits = 0;
        std::memcpy(&bits, ast.literal_bytes.data()+lit.offset+j*fundamental_width(lit.type), fundamental_width(lit.type));
        d.value = Operand::integer(bits); p.data.push_back(d);
    }
    p.globals.push_back(g);
    auto& s = p.symbols[g.symbol.index-1]; s.kind = Symbol::GlobalSymbol; s.entity = p.globals.size();
    s.metadata.binding = ir_model::SBM_INTERNAL; s.metadata.storage = ir_model::GSM_READONLY;
}
DataItem Procedural::constant_data(NodeId n, TypeId t)
{
    auto value = sem.static_value(n, t);
    DataItem d; d.type = type(t);
    if (value.kind == semantic::StaticValue::Address) { d.kind = DataItem::Address; d.symbol = symbol(value.entity); d.addend = value.addend; }
    else if (value.kind == semantic::StaticValue::String) { d.kind = DataItem::Address; d.symbol = strings[value.string]; }
    else if (value.kind == semantic::StaticValue::Integer) { d.kind = DataItem::Scalar; d.value = Operand::integer(value.bits); }
    else if (value.kind == semantic::StaticValue::Floating) { d.kind = DataItem::Scalar; d.value = Operand::floating(value.floating); }
    else throw std::runtime_error("unsupported static initializer");
    return d;
}
void Procedural::global_data(NodeId n, TypeId t)
{
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto array = sem.types[t];
    if (array.kind == TypeKind::Named && sem.entities[array.entity].class_info) {
        NodeId c = ast[n].first;
        std::uint64_t bytes = 0, total = sem.object_size(t);
        for (auto d = sem.scopes[sem.entities[array.entity].scope].first_decl; d; d = sem.declarations[d].next) {
            auto member = sem.entities[sem.declarations[d].entity];
            if (member.kind != semantic::EntityKind::Variable || member.is_static) continue;
            if (member.member_offset > bytes) { DataItem padding; padding.zero_bytes = member.member_offset-bytes; p.data.push_back(padding); }
            global_data(c, member.type); bytes = member.member_offset + sem.object_size(member.type);
            if (c) c = ast[c].next;
        }
        if (total > bytes) { DataItem padding; padding.zero_bytes = total-bytes; p.data.push_back(padding); }
        return;
    }
    if (array.kind == TypeKind::Array) {
        if (!n) {
            DataItem d; d.zero_bytes = sem.object_size(t); p.data.push_back(d); return;
        }
        unsigned count = 0;
        for (NodeId c = ast[n].first; c; c = ast[c].next) { global_data(c, array.child); ++count; }
        if (count > array.bound) throw std::runtime_error("excess array initializer");
        if (count < array.bound) { DataItem d; d.zero_bytes = (array.bound-count)*sem.object_size(array.child); p.data.push_back(d); }
    } else {
        DataItem d = constant_data(n, t);
        if (d.type == IRType::Ptr && d.kind == DataItem::Scalar && !d.value.data.integer) {
            d.kind = DataItem::Zero; d.zero_bytes = 8;
        }
        p.data.push_back(d);
    }
}
void Procedural::global(EntityId e)
{
    auto entity = sem.entities[e]; TypeId t = entity.type;
    Global g; g.symbol = symbols[e]; g.declaration = !entity.definition;
    auto prior = p.symbols[g.symbol.index-1];
    if (prior.kind == Symbol::GlobalSymbol) {
        if (g.declaration) return;
        if (!p.globals[prior.entity-1].declaration) throw std::runtime_error("multiple global definitions");
    }
    g.structured = sem.types[t].kind == TypeKind::Array || (sem.types[t].kind == TypeKind::Named && sem.entities[sem.types[t].entity].class_info);
    if (!g.structured) g.type = type(t);
    if (!g.declaration) {
        if (reference(t)) {
            auto value = sem.static_value(entity.initializer, t);
            if (value.kind != semantic::StaticValue::Address) {
                Global temp;
                temp.symbol = fresh_symbol("@__reference_" + std::to_string(p.symbols.size()+1));
                temp.type = type(sem.types[t].child); temp.data.begin = p.data.size(); temp.data.count = 1;
                p.data.push_back(constant_data(entity.initializer, sem.types[t].child)); p.globals.push_back(temp);
                auto& sym = p.symbols[temp.symbol.index-1]; sym.kind = Symbol::GlobalSymbol; sym.entity = p.globals.size(); sym.metadata.binding = ir_model::SBM_INTERNAL;
                g.data.begin = p.data.size(); g.data.count = 1;
                DataItem d; d.kind = DataItem::Address; d.type = IRType::Ptr; d.symbol = temp.symbol; p.data.push_back(d);
            } else { g.data.begin = p.data.size(); p.data.push_back(constant_data(entity.initializer, t)); g.data.count = 1; }
        } else {
            g.data.begin = p.data.size();
            if (!entity.initializer && !g.structured) { DataItem d; d.zero_bytes = g.type.bytes(); p.data.push_back(d); }
            else global_data(entity.initializer, t);
            g.data.count = p.data.size() - g.data.begin;
        }
    }
    if (prior.kind == Symbol::GlobalSymbol) { p.globals[prior.entity-1] = g; return; }
    p.globals.push_back(g);
    auto& sym = p.symbols[g.symbol.index-1]; sym.kind = Symbol::GlobalSymbol; sym.entity = p.globals.size();
}
void Procedural::object(EntityId e)
{
    TypeId t = sem.entities[e].type;
    if (!objects[e]) objects[e] = builder->add_slot(0, type(t));
    Value location(Operand::slot(objects[e]), type(t), t, true);
    NodeId init = sem.entities[e].initializer;
    if (init) initialize(init, t, location);
    else if (sem.types[t].kind == TypeKind::Named && sem.entities[sem.types[t].entity].class_info) address(location);
}
void Procedural::initialize(NodeId n, TypeId t, Value location)
{
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto target = sem.types[t];
    if (target.kind == TypeKind::Named && sem.entities[target.entity].class_info) {
        sem.object_size(t);
        Value base = address(location); NodeId c = ast[n].first;
        for (auto d = sem.scopes[sem.entities[target.entity].scope].first_decl; d; d = sem.declarations[d].next) {
            auto member = sem.entities[sem.declarations[d].entity];
            if (member.kind != semantic::EntityKind::Variable || member.is_static) continue;
            Instruction projection(Opcode::Index, IRType::I8); projection.projection = ir_model::IPK_FIELD;
            Value at = emit(projection, {base.operand, Operand::integer(member.member_offset)});
            at.type = member.type; at.address = true;
            initialize(c, member.type, at); if (c) c = ast[c].next;
        }
        return;
    }
    if (target.kind == TypeKind::Array) {
        Value base = address(location);
        unsigned count = 0;
        auto element = [&](NodeId c) {
            Value at = count ? emit(Opcode::Index, IRType::I8, {base.operand, Operand::integer(count*sem.object_size(target.child))}) : base;
            at.type = target.child; at.address = true;
            initialize(c, target.child, at); ++count;
        };
        for (NodeId c = ast[n].first; c; c = ast[c].next) element(c);
        if (count > target.bound) throw std::runtime_error("excess array initializer");
        while (count < target.bound) element(0);
        return;
    }
    if (ast[n].kind == Kind::BracedInit || ast[n].kind == Kind::ParenInitializer) n = ast[n].first;
    Value value = n ? (sem.expression_fact(n).incoming ? converted(n, sem.conversion_fact(sem.expression_fact(n).incoming)) : convert(expression(n, reference(t)), t)) : Value(type(t).floating() ? Operand::floating(0) : Operand::integer(0), type(t), t);
    store(value, location);
}
} }
