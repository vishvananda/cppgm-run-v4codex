#include "semantic/analyzer.h"
#include <stdexcept>
#include <limits>
namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::initialized_field_type(TypeId owner, EntityId field)
{
    unsigned cv = types[owner].cv;
    if (entities[field].mutable_field) cv &= ~1u;
    return types.qualify(entities[field].type, cv);
}
bool Analyzer::aggregate_type(TypeId t) const
{
    auto type = types[t];
    return type.kind == TypeKind::Array || (type.kind == TypeKind::Named &&
        entities[type.entity].class_info && class_facts[entities[type.entity].class_info].aggregate);
}
bool Analyzer::string_initialization(NodeId n, TypeId t) const
{
    if (types[t].kind != TypeKind::Array || ast[n].kind != Kind::Literal) return false;
    auto literal = ast.literals[ast[n].literal];
    if (literal.kind != LiteralKind::string || literal.suffix) return false;
    Type element = types[types[t].child];
    if (element.kind != TypeKind::Fundamental) return false;
    bool ordinary = literal.type == FT_CHAR && (element.fundamental == FT_CHAR ||
        element.fundamental == FT_SIGNED_CHAR || element.fundamental == FT_UNSIGNED_CHAR);
    return ordinary || element.fundamental == literal.type;
}
std::uint32_t Analyzer::initializer_plan(NodeId n, TypeId t) const
{
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    return initializer_index.get(key(n, t));
}
void Analyzer::list_conversion(NodeId n, TypeId target)
{
    TypeId from = expressions[n].type;
    if (!arithmetic(from) || !arithmetic(target)) return;
    bool a = integral(from), b = integral(target);
    bool narrowing = false;
    if (!a && b) narrowing = true;
    else if (a && b) {
        bool covers = width(target) > width(from) && (is_unsigned(from) || !is_unsigned(target));
        covers |= width(target) == width(from) && is_unsigned(target) == is_unsigned(from);
        if (fundamental(target, FT_BOOL) && !fundamental(from, FT_BOOL)) covers = false;
        if (!covers) {
            Constant value = evaluate(n, facts[n].scope);
            if (!value.valid) narrowing = true;
            else if (fundamental(target, FT_BOOL)) narrowing = value.bits > 1;
            else {
                auto converted = convert(value, target, true);
                auto restored = convert(converted, from, true);
                narrowing = restored.bits != value.bits;
                if (is_unsigned(target) && !is_unsigned(from) && std::int64_t(value.bits) < 0) narrowing = true;
                if (!is_unsigned(target) && is_unsigned(from) && (converted.bits >> (width(target)-1))) narrowing = true;
            }
        }
    } else {
        auto value = static_value(n, from);
        if (a) {
            if (value.kind != StaticValue::Integer) narrowing = true;
            else {
                long double exact = is_unsigned(from) ? static_cast<long double>(value.bits) : static_cast<long double>(std::int64_t(value.bits));
                long double rounded = fundamental(target, FT_FLOAT) ? static_cast<long double>(float(exact)) : fundamental(target, FT_DOUBLE) ? static_cast<long double>(double(exact)) : exact;
                narrowing = rounded != exact;
            }
        } else if (width(target) < width(from)) {
            if (value.kind != StaticValue::Floating) narrowing = true;
            else {
                long double rounded = fundamental(target, FT_FLOAT) ? static_cast<long double>(float(value.floating)) : static_cast<long double>(double(value.floating));
                narrowing = rounded != value.floating;
            }
        }
    }
    if (narrowing) throw std::runtime_error("narrowing list initialization");
}
std::uint32_t Analyzer::initializer_item(NodeId& cursor, TypeId t, ScopeId s)
{
    std::uint32_t id = initializers.size(); initializers.push_back(InitAction());
    initializers[id].type = t; initializers[id].source = cursor;
    if (!cursor) {
        prepare_value_initialization(t, s);
        initializers[id] = initializers[initializer_plan(0, t)];
        return id;
    }
    NodeId source = cursor;
    bool aggregate = aggregate_type(t);
    bool braced = ast[source].kind == Kind::BracedInit || ast[source].kind == Kind::ParenArguments || ast[source].kind == Kind::ParenInitializer;
    NodeId inner = braced ? ast[source].first : source;
    if (aggregate && class_value(t) && !braced) {
        expression(source,s);
        Conversion c = conversion(source,t);
        if (c.valid()) {
            record_class_initialization(source,t,source,&c);
            initializers[id].kind = InitKind::Constructor;
            cursor = ast[source].next; return id;
        }
    }
    if (string_initialization(inner, t)) {
        if (braced && ast[inner].next) throw std::runtime_error("excess string initializer");
        auto lit = ast.literals[ast[inner].literal];
        if (types[t].bound && lit.elements > types[t].bound) throw std::runtime_error("string exceeds array bound");
        expression(inner, s);
        expressions[inner].evaluated = false; // Direct character initialization has no backing-array address use.
        initializers[id].source = inner; initializers[id].kind = InitKind::String;
        cursor = ast[source].next; return id;
    }
    if (!aggregate) {
        if (types[t].kind == TypeKind::Named && entities[types[t].entity].class_info) {
            initialize(source, t, s); initializers[id].kind = InitKind::Constructor;
        } else {
            initialize(source, t, s);
            NodeId scalar = source;
            while (ast[scalar].kind == Kind::BracedInit || ast[scalar].kind == Kind::ParenArguments || ast[scalar].kind == Kind::ParenInitializer) scalar = ast[scalar].first;
            if (scalar) list_conversion(scalar, t);
            initializers[id].source = scalar;
        }
        cursor = ast[source].next; return id;
    }
    initializers[id].kind = InitKind::Group;
    std::uint32_t tail = 0;
    auto append = [&](std::uint32_t item) {
        if (tail) initializers[tail].next = item; else initializers[id].first = item;
        tail = item;
    };
    Type target = types[t];
    if (target.kind == TypeKind::Array) {
        std::uint64_t index = 0;
        while (inner && (!target.bound || index < target.bound)) {
            auto item = initializer_item(inner, target.child, s);
            initializers[item].index = index++; append(item);
        }
        if (index < target.bound) {
            NodeId omitted = 0;
            auto item = initializer_item(omitted, target.child, s);
            initializers[item].index = index; initializers[item].count = target.bound-index; append(item);
        }
    } else {
        size(t);
        for (auto d = scopes[entities[target.entity].scope].first_decl; d; d = declarations[d].next) {
            EntityId field = declarations[d].entity;
            if (!nonstatic_field(field)) continue;
            auto item = initializer_item(inner, initialized_field_type(t, field), s);
            initializers[item].field = field; append(item);
            if (entities[target.entity].key == KW_UNION) break;
        }
    }
    if (braced && inner) throw std::runtime_error("excess aggregate initializer");
    cursor = braced ? ast[source].next : inner;
    return id;
}
bool Analyzer::zero_value(TypeId t)
{
    if (auto known = zero_value_index.get(t)) return known == 2;
    auto type = types[t];
    bool result = !(type.cv & 2) && type.kind != TypeKind::LRef && type.kind != TypeKind::RRef;
    if (type.kind == TypeKind::MemberPointer && types[type.child].kind != TypeKind::Function) result = false;
    if (result && type.kind == TypeKind::Array) result = zero_value(type.child);
    if (result && type.kind == TypeKind::Named && entities[type.entity].class_info) {
        result = !value_constructor(t) && entities[type.entity].key != KW_UNION;
        for (auto d = scopes[entities[type.entity].scope].first_decl; result && d; d = declarations[d].next) {
            EntityId field = declarations[d].entity;
            if (!nonstatic_field(field)) continue;
            result = zero_value(entities[field].type);
            if (entities[type.entity].key == KW_UNION) break;
        }
    }
    zero_value_index.put(t, result ? 2 : 1); return result;
}
bool Analyzer::initializer_work(std::uint32_t plan)
{
    if (!plan) return true;
    if (auto known = initializer_work_index.get(plan)) return known == 2;
    auto action = initializers[plan];
    bool work = true;
    if (action.kind == InitKind::Group) {
        work = false;
        for (auto c = action.first; c; c = initializers[c].next) work |= initializer_work(c);
    } else if (action.kind == InitKind::Value) work = !empty_value(action.type);
    initializer_work_index.put(plan, work ? 2 : 1);
    return work;
}
void Analyzer::aggregate_initialization(NodeId n, TypeId t, ScopeId s)
{
    if (initializer_plan(n, t)) return;
    NodeId cursor = n;
    auto plan = initializer_item(cursor, t, s);
    if (cursor && cursor != ast[n].next) throw std::runtime_error("excess initializer at object boundary");
    initializer_index.put(key(n, t), plan);
    facts[n].type = t; facts[n].scope = s;
    expressions[n].type = t; expressions[n].category = ValueCategory::Lvalue;
    expressions[n].ready = true; expressions[n].evaluated = initializers[plan].kind != InitKind::String;
}
} }
