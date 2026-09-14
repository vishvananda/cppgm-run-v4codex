#include "lowering/procedural.h"
#include <stdexcept>
#include <cstring>
namespace cppgm { namespace lowering {
using syntax::Kind;
using namespace lowir_model;
void Procedural::string_literal(NodeId n)
{
    const auto lit = ast.literals[ast[n].literal];
    std::uint64_t hash = 1469598103934665603ULL ^ unsigned(lit.type);
    for (std::uint32_t j = 0; j < lit.bytes; ++j) hash = (hash ^ static_cast<unsigned char>(ast.literal_bytes[lit.offset+j])) * 1099511628211ULL;
    auto head = string_index.get(hash);
    for (auto i = head; i; i = string_records[i].next) {
        auto other = ast.literals[ast[string_records[i].node].literal];
        if (other.type == lit.type && other.bytes == lit.bytes &&
            !std::memcmp(ast.literal_bytes.data()+other.offset, ast.literal_bytes.data()+lit.offset, lit.bytes)) {
            strings[n] = strings[string_records[i].node]; return;
        }
    }
    string_records.push_back(StringRecord{n, head}); string_index.put(hash, string_records.size()-1);
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
    if (n && sem.constructor_member(sem.facts[n].entity)) { global_construction(n,t); return; }
    // The constant classifier admitted a no-effect empty construction.
    if (sem.class_initialization(n,t).source && sem.empty_value(t)) n = 0;
    if (auto plan = sem.initializer_plan(n, t)) { global_plan(plan); return; }
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto array = sem.types[t];
    if ((array.kind == TypeKind::Named && sem.entities[array.entity].class_info) || array.kind == TypeKind::Array) {
        if (n) throw std::logic_error("missing static aggregate initializer plan");
        DataItem zero; zero.zero_bytes = sem.object_size(t); p.data.push_back(zero);
    } else {
        DataItem d = constant_data(n, t);
        if (!n && d.type == IRType::Ptr && d.kind == DataItem::Scalar && !d.value.data.integer) {
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
        bool local = sem.local_static(e);
        bool dynamic = !sem.static_initialization(e);
        if (dynamic) {
            if (!local) {
                if (entity.thread_local_storage) prepare_tls(e);
                else global_initializers.push_back(e);
            }
            g.data.begin = p.data.size(); g.data.count = 1;
            DataItem zero; zero.zero_bytes = g.structured ? sem.object_size(t) : g.type.bytes(); p.data.push_back(zero);
        }
        else if (local && reference(t) && sem.static_value(entity.initializer,t).kind == semantic::StaticValue::Address) {
            // The PA16 LowIR contract materializes constant reference bindings
            // in the startup function, ahead of all dynamic initializers.
            static_reference_initializers.push_back(e);
            g.data.begin = p.data.size(); g.data.count = 1;
            DataItem zero; zero.zero_bytes = 8; p.data.push_back(zero);
        }
        else if (reference(t)) {
            auto value = sem.static_value(entity.initializer, t);
            if (value.kind != semantic::StaticValue::Address && value.kind != semantic::StaticValue::String) {
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
            if (auto cls = sem.static_vptr(e)) {
                DataItem item; item.kind = DataItem::Address; item.type = IRType::Ptr;
                item.symbol = vtable_symbol(cls); item.addend = 16; p.data.push_back(item);
                if (sem.object_size(t) > 8) { DataItem zero; zero.zero_bytes = sem.object_size(t)-8; p.data.push_back(zero); }
            }
            else if (g.structured && entity.constant.valid) global_constant_fields(sem.constant_value_data(entity.constant),t);
            else if (!entity.initializer && entity.constant.valid) {
                DataItem d; d.kind = DataItem::Scalar; d.type = g.type;
                d.value = type(entity.constant.type).floating() ? Operand::floating(sem.floating_value(entity.constant)) : Operand::integer(entity.constant.bits); p.data.push_back(d);
            }
            else if (!entity.initializer && !g.structured) { DataItem d; d.zero_bytes = g.type.bytes(); p.data.push_back(d); }
            else global_data(entity.initializer, t);
            g.data.count = p.data.size() - g.data.begin;
        }
    }
    if (prior.kind == Symbol::GlobalSymbol) { p.globals[prior.entity-1] = g; return; }
    p.globals.push_back(g);
    auto& sym = p.symbols[g.symbol.index-1]; sym.kind = Symbol::GlobalSymbol; sym.entity = p.globals.size();
    if (sem.local_static(e)) prepare_local_static(e,!sem.static_initialization(e));
}
void Procedural::object(EntityId e)
{
    if (sem.entities[e].external_decl && sem.scopes[sem.entities[e].owner].kind == semantic::ScopeKind::Namespace) return;
    if (sem.local_static(e)) { initialize_local_static(e); return; }
    initialized_units = semantic::Index();
    TypeId t = sem.entities[e].type;
    auto lifetime = sem.object_lifetime(e);
    if (lifetime) live = sem.lifetimes[lifetime].tail;
    auto initial_live = live;
    prepare_reference_guards(e);
    if (!object_addresses[e] && (!objects[e] || p.slots[objects[e].index-1].owner.index != function.index)) objects[e] = source_slot(e);
    Value location = object_addresses[e] ? Value(Operand::value(object_addresses[e]),type(t),t,true) : Value(Operand::slot(objects[e]), type(t), t, true);
    NodeId init = sem.entities[e].initializer;
    auto selected = sem.class_initialization(init,t);
    auto conversion = sem.conversion_fact(selected.conversion);
    bool omit = selected.source && conversion.kind == semantic::Conversion::Kind::Construction && sem.conversion_objects[conversion.materialization].elided;
    const auto& scalar = sem.scalar_consumption(e);
    begin_full_expression(scalar.expression ? scalar.expression : init,omit);
    if (sem.constant_array_plan(e)) initialize_constant_array(e,location);
    else if (scalar.expression && full_expression.enabled) initialize_scalar(scalar,location);
    else if (init && sem.class_initialization(init,t).source) initialize(init,t,location);
    else if (init && sem.types[t].kind == TypeKind::Named && sem.entities[sem.types[t].entity].class_info && !sem.facts[init].entity) {
        if (sem.initializer_work(sem.initializer_plan(init, t))) address(location);
        std::vector<InitProjection> path;
        aggregate_initialize(init, t, location, false, path);
    } else if (init) initialize(init, t, location);
    else if (sem.types[t].kind == TypeKind::Array && sem.object_constructor(e)) array_construct(sem.object_constructor(e), t, location, false, {});
    else if (sem.types[t].kind == TypeKind::Named && sem.entities[sem.types[t].entity].class_info) {
        Value base = address(location);
        construct(sem.object_constructor(e), 0, base);
    }
    finish_full_expression(initial_live);
    if (lifetime) live = lifetime;
}
void Procedural::initialize(NodeId n, TypeId t, Value location)
{
    if (auto source = sem.class_initialization(n,t).source) {
        construct_value(source,sem.conversion_fact(sem.class_initialization(n,t).conversion),address(location)); return;
    }
    if (auto plan = sem.initializer_plan(n, t)) { initialize_plan(plan, location); return; }
    if (n && sem.facts[n].entity && sem.constructor_member(sem.facts[n].entity)) {
        construct(sem.facts[n].entity, n, address(location)); return;
    }
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto target = sem.types[t];
    if ((target.kind == TypeKind::Named && sem.entities[target.entity].class_info) || target.kind == TypeKind::Array)
        throw std::logic_error("missing aggregate initializer plan");
    if ((ast[n].kind == Kind::BracedInit && !sem.expression_fact(n).incoming) || ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments) n = ast[n].first;
    Value value = n ? (location.bit_field ? load(expression(n)) : sem.expression_fact(n).incoming ? converted(n, sem.conversion_fact(sem.expression_fact(n).incoming)) : convert(expression(n, reference(t)), t)) : initialization_value(0,t);
    store(value, location);
}
} }

namespace cppgm { namespace lowering {
void Procedural::numeric_string_literal(NodeId n)
{
    auto prefix = ast.literals[ast[n].literal].prefix;
    if (auto old = numeric_strings.get(prefix)) { strings[n] = lowir_model::SymbolId(old); return; }
    auto text = identifiers.spelling(prefix);
    lowir_model::Global g; g.structured = true;
    g.symbol = fresh_symbol("@__string_"+std::to_string(p.symbols.size()+1));
    g.data.begin = p.data.size(); g.data.count = text.size+1;
    for (unsigned j = 0; j <= text.size; ++j) {
        lowir_model::DataItem d; d.kind = lowir_model::DataItem::Scalar; d.type = IRType::I8;
        d.value = lowir_model::Operand::integer(j < text.size ? static_cast<unsigned char>(text.data[j]) : 0); p.data.push_back(d);
    }
    strings[n] = g.symbol; numeric_strings.put(prefix,g.symbol.index); p.globals.push_back(g);
    auto& symbol = p.symbols[g.symbol.index-1]; symbol.kind = lowir_model::Symbol::GlobalSymbol; symbol.entity = p.globals.size();
    symbol.metadata.binding = ir_model::SBM_INTERNAL; symbol.metadata.storage = ir_model::GSM_READONLY;
}
} }
