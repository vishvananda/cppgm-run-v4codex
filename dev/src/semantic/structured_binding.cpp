#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::binding_object_type(NodeId specs, NodeId d, Expression value)
{
    bool automatic = false;
    unsigned cv = 0;
    for (auto c = ast[specs].first; c; c = ast[c].next) {
        if (ast[c].op == KW_AUTO) automatic = true;
        else if (ast[c].op == KW_CONST) cv |= 1;
        else if (ast[c].op == KW_VOLATILE) cv |= 2;
        else throw std::runtime_error("invalid structured binding specifier");
    }
    if (!automatic) throw std::runtime_error("structured binding requires auto");
    ETokenType ref = TOK_INVALID;
    for (auto c = ast[d].first; c; c = ast[c].next) {
        if (ast[c].kind == Kind::BindingNames) continue;
        if (ast[c].kind != Kind::Pointer || ref != TOK_INVALID ||
            (ast[c].op != OP_AMP && ast[c].op != OP_LAND))
            throw std::runtime_error("invalid structured binding declarator");
        ref = ast[c].op;
    }
    if (!value.type || dependent_type(value.type) || pattern_class_type(value.type)) return 0;
    if (ref == TOK_INVALID) return types.qualify(types.unqualified(value.type),cv);
    if (ref == OP_AMP && value.category != ValueCategory::Lvalue && !(cv & 1))
        throw std::runtime_error("lvalue binding requires lvalue initializer");
    auto t = types.qualify(value.type,cv);
    return types.compound(ref == OP_AMP || (!cv && value.category == ValueCategory::Lvalue) ? TypeKind::LRef : TypeKind::RRef,t);
}
void Analyzer::declare_bindings(NodeId d, ScopeId s, EntityId object, bool pattern)
{
    auto t = value_type(entities[object].type);
    bool fixed = t && !dependent_type(t) && !pattern_class_type(t);
    BindingShape shape;
    if (fixed) {
        auto known = binding_shape_index.get(t);
        if (known) shape = binding_shapes[known];
        else {
            shape.type = t;
            if (types[t].kind == TypeKind::Array) shape.count = types[t].bound;
            else if (class_value(t)) {
                auto cls = types[t].entity;
                if (entities[cls].key == KW_UNION) throw std::runtime_error("cannot decompose union");
                complete_class(cls);
                std::vector<EntityId> work{cls}, fields;
                Index seen;
                EntityId owner = 0;
                for (unsigned i = 0; i < work.size(); ++i) {
                    auto current = work[i];
                    if (seen.get(current)) continue;
                    seen.put(current,1); complete_class(current);
                    auto scope = entities[current].scope;
                    for (auto at = scopes[scope].first_decl; at; at = declarations[at].next) {
                        auto field = declarations[at].entity;
                        if (!nonstatic_field(field) || entities[field].owner != scope) continue;
                        if (!entities[field].name && field_fact(field).bit_field) continue;
                        if (!entities[field].name) throw std::runtime_error("anonymous member in decomposition");
                        if (owner && owner != current) throw std::runtime_error("decomposition members have different owners");
                        owner = current; fields.push_back(field);
                    }
                    for (auto b = first_base_edge(current); b; b = bases[b].next) work.push_back(bases[b].base);
                }
                if (owner && owner != cls && base_adjustments[base_path(t,owner)].ambiguous)
                    throw std::runtime_error("ambiguous decomposition base");
                shape.first = binding_members.size(); shape.count = fields.size();
                binding_members.insert(binding_members.end(),fields.begin(),fields.end());
            } else throw std::runtime_error("structured binding requires array or class");
            binding_shape_index.put(t,binding_shapes.size()); binding_shapes.push_back(shape);
        }
    }
    unsigned index = 0;
    auto names = child(d,Kind::BindingNames);
    for (auto n = ast[names].first; n; n = ast[n].next,++index) {
        if (fixed && index >= shape.count) throw std::runtime_error("too many structured binding names");
        BindingProjection projection; projection.object = object; projection.element = index;
        TypeId type = 0;
        if (fixed) {
            if (types[t].kind == TypeKind::Array) type = types[t].child;
            else {
                auto field = projection.member = binding_members[shape.first+index];
                check_access(field,s,entities[types[t].entity].scope,t);
                type = entities[field].type;
                if (types[type].kind != TypeKind::LRef && types[type].kind != TypeKind::RRef)
                    type = types.qualify(type,types[t].cv & (entities[field].mutable_field ? 2 : 3));
                if (!pattern) projection.adjustment = base_steps(t,scopes[entities[field].owner].entity);
            }
        }
        auto e = facts[n].entity;
        if (!e) {
            if (local(s,ast[n].text)) throw std::runtime_error("duplicate structured binding name");
            e = pattern ? pattern_declaration(EntityKind::Variable,s,ast[n].text,n,!fixed) :
                make_entity(EntityKind::Variable,s,ast[n].text,n);
            if (!pattern) { bind(s,ast[n].text,e); record(s,e,n,type,EntityKind::Variable); }
        }
        entities[e].type = type; facts.edit(n).entity = e; facts.edit(n).type = type;
        if (pattern) template_pattern_entities.put(e,fixed ? 1 : 2);
        placeholder_objects.put(e,fixed ? 0 : d);
        if (projection.member && field_fact(projection.member).bit_field)
            field_index.put(e,field_index.get(projection.member));
        binding_projection_index.put(e,binding_projections.size()); binding_projections.push_back(projection);
    }
    if (fixed && index != shape.count) throw std::runtime_error("too few structured binding names");
}
void Analyzer::resolve_bindings(NodeId specs, NodeId d, ScopeId s, bool pattern)
{
    if (scopes[s].kind != ScopeKind::Block && scopes[s].kind != ScopeKind::Control)
        throw std::runtime_error("structured binding requires block scope");
    auto init = ast[d].next, source = init;
    while (ast[source].kind == Kind::Initializer || ast[source].kind == Kind::ParenInitializer || ast[source].kind == Kind::BracedInit) {
        auto first = ast[source].first;
        if (!first || ast[first].next) throw std::runtime_error("structured binding requires one initializer");
        source = first;
    }
    if (!source) throw std::runtime_error("structured binding requires initializer");
    auto object = pattern ? pattern_declaration(EntityKind::Variable,s,0,d,true) : make_entity(EntityKind::Variable,s,0,d);
    entities[object].initializer = init;
    facts.edit(d).entity = object;
    // Publish pending names before checking the initializer (including shadowing).
    declare_bindings(d,s,object,pattern);
    if (pattern) bind_template_expression(init,s);
    auto value = pattern ? template_statement_value(source,s) : expression(source,s);
    auto t = binding_object_type(specs,d,value);
    entities[object].type = t; facts.edit(d).type = t;
    if (pattern) {
        if (t) check_template_initialization(init,t,s,InitializationMode::Direct);
    } else {
        if (!t) throw std::logic_error("unresolved decomposition type");
        record(s,object,d,t,EntityKind::Variable);
        finish_object_initializer(object,init,d,specs,s,s,t,false);
    }
    declare_bindings(d,s,object,pattern);
}
} }
