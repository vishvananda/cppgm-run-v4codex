#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::excluded_from_class_instantiation(EntityId e) const
{
    while (e) {
        if (entities[e].exclude_instantiation) return true;
        e = native_attribute_patterns.get(e);
    }
    return false;
}
bool Analyzer::instantiation_suppressed(EntityId e) const
{
    auto entity = entities[e];
    if (entity.explicit_specialization || entity.instantiation_definition || entity.inline_function) return false;
    if (entity.instantiation_declaration) return true;
    // The attribute excludes the member from its enclosing class's explicit
    // instantiation only. A direct instantiation of the member still owns it.
    if (excluded_from_class_instantiation(e)) return false;
    if (scopes[entity.owner].kind != ScopeKind::Class || entity.template_info || entity.specialization) return false;
    if (entity.member_info && members[entity.member_info].synthetic) return false;
    // Naming a class also instantiates its non-template nested classes.
    // A member template specialization still owns an independent demand.
    for (auto scope = entity.owner; scopes[scope].kind == ScopeKind::Class;) {
        auto owner = scopes[scope].entity;
        auto cls = entities[owner];
        if (cls.explicit_specialization || cls.instantiation_definition) return false;
        if (cls.instantiation_declaration) return true;
        if (excluded_from_class_instantiation(owner)) return false;
        if (cls.specialization || cls.template_info) return false;
        scope = cls.owner;
    }
    return false;
}
void Analyzer::explicit_instantiation(NodeId n, ScopeId s)
{
    using syntax::Kind;
    auto source = ast.first(n);
    bool declaration_only = ast.flags(n) & 1;
    struct Naming {
        bool& flag; bool saved;
        Naming(bool& f) : flag(f), saved(f) { flag = true; }
        void restore() { flag = saved; }
        ~Naming() { restore(); }
    } naming(explicit_instantiation_naming);
    auto check_namespace = [&](ScopeId owner, NodeId name) {
        if (scopes[s].kind != ScopeKind::Namespace || !encloses(s,owner))
            throw std::runtime_error("explicit instantiation outside enclosing namespace");
        if (ast.first(name) != ast.last(name) || ast.op(name) == OP_COLON2) return;
        for (auto current = owner; current != s; current = scopes[current].parent) {
            auto parent = scopes[current].parent;
            bool inline_namespace = false;
            for (auto edge = scopes[parent].first_inline; edge; edge = edges[edge].inline_next)
                inline_namespace |= edges[edge].target == current;
            if (!inline_namespace) throw std::runtime_error("unqualified instantiation outside template namespace");
        }
    };
    auto publish = [&](EntityId e) {
        if (declaration_only && entities[e].instantiation_definition)
            throw std::runtime_error("explicit instantiation declaration follows definition");
        if (!declaration_only && entities[e].instantiation_definition)
            throw std::runtime_error("duplicate explicit instantiation definition");
        if (declaration_only) entities[e].instantiation_declaration = true;
        else entities[e].instantiation_definition = true;
    };
    if (ast.kind(source) != Kind::ClassForward) {
        bool special = ast.kind(source) == Kind::SpecialMember;
        if (ast.kind(source) != Kind::SimpleDeclaration && !special) throw std::runtime_error("invalid explicit instantiation");
        auto item = ast.first(child(source,Kind::InitDeclarators));
        if (!special && (!item || ast.next(item))) throw std::runtime_error("explicit instantiation requires one declarator");
        auto d = special ? child(source,Kind::Declarator) : ast.first(item), name = decl_name(d);
        auto owner = name_owner(name,s,true);
        auto type = types.signature(declarator(d,special ? types.fundamental(FT_VOID) : specifiers(ast.first(source),s),s));
        EntityId selected = 0;
        if (types[type].kind == TypeKind::Function) {
            auto list = child(ast.last(name),Kind::TemplateArguments);
            std::vector<ArgumentId> args;
            for (auto a = ast.first(list); a; a = ast.next(a)) args.push_back(template_argument_node(a,s));
            auto binding = lookup(owner,terminal(name),Lookup::Ordinary,true);
            if (special && scopes[owner].kind == ScopeKind::Class) {
                auto cls = entities[scopes[owner].entity].class_info;
                binding = ast.op(ast.last(name)) == OP_COMPL ? class_facts[cls].destructor : class_facts[cls].constructor;
            }
            std::vector<EntityId> matching;
            for (auto candidate : candidates(binding)) {
                auto e = candidate;
                if (entities[e].template_info) {
                    if (list) e = specialize(e,args,true);
                    if (e && entities[e].template_info) e = deduce_target(e,type);
                } else if (list) continue;
                if (!e || entities[e].type != type) continue;
                if (!entities[e].specialization && !entities[e].template_member) continue;
                matching.push_back(e);
                if (!selected || template_more_specialized(e,selected)) selected = e;
            }
            // Matching a complete signature can deduce several overloads.
            // [temp.deduct.decl] chooses the unique most specialized template,
            // using the same full-signature ordering as address deduction.
            for (auto e : matching) if (e != selected && !template_more_specialized(selected,e))
                throw std::runtime_error("ambiguous explicit instantiation");
        } else {
            selected = local(owner,terminal(name));
            if (!selected || entities[selected].kind != EntityKind::Variable || !entities[selected].template_member ||
                !entities[selected].is_static || entities[selected].type != type) selected = 0;
        }
        if (!selected) throw std::runtime_error("explicit instantiation has no matching entity");
        if (types[type].kind == TypeKind::Function) declare_operator(selected,name);
        auto ns = entities[selected].owner;
        while (scopes[ns].kind != ScopeKind::Namespace) ns = scopes[ns].parent;
        check_namespace(ns,name);
        if (entities[selected].explicit_specialization) return;
        publish(selected);
        naming.restore();
        if (declaration_only) return;
        if (entities[selected].kind == EntityKind::Function) {
            demand_specialization(selected);
            if (entities[selected].member_info) {
                members[entities[selected].member_info].retained_root = true;
                if (constructor_member(selected) || destructor_member(selected)) members[entities[selected].member_info].complete_entry = true;
                demand_member(selected);
            }
        } else demand_template_storage(selected);
        return;
    }
    auto e = resolve(ast.detail(source),s,Lookup::Qualifier);
    if (!e || !entities[e].class_info || (!entities[e].specialization &&
        ((!entities[e].template_member && !entities[e].explicit_specialization) || entities[e].template_info)))
        throw std::runtime_error("explicit instantiation requires a class specialization or member class");
    auto owner = entities[e].specialization ? entities[specialization_pattern(e)].owner : entities[e].owner;
    auto name = ast.detail(source);
    check_namespace(owner,name);
    if ((ast.op(ast.first(source)) == KW_UNION) != (entities[e].key == KW_UNION))
        throw std::runtime_error("explicit instantiation class key mismatch");
    if (entities[e].explicit_specialization) return;
    publish(e);
    naming.restore();
    if (declaration_only) return;
    std::vector<EntityId> work(1,e); Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto cls = work[i];
        if (seen.get(cls)) continue;
        seen.put(cls,1); complete_class(cls);
        if (!entities[cls].complete) throw std::runtime_error("explicit instantiation of incomplete class");
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            auto member = declarations[d].entity;
            if (entities[member].owner != entities[cls].scope || entities[member].template_info || excluded_from_class_instantiation(member)) continue;
            bool defined = instantiate_member_definition(member);
            if (entities[member].member_info) {
                auto m = entities[member].member_info;
                if (!members[m].body && !members[m].synthetic) continue;
                members[m].retained_root = true;
                if (members[m].constructor || members[m].destructor) members[m].complete_entry = true;
                demand_member(member);
            } else if (entities[member].class_info) {
                if (entities[member].complete || class_facts[entities[member].class_info].definition_source || defined)
                    work.push_back(member);
            } else if (entities[member].is_static && (defined || entities[member].inline_variable)) demand_template_storage(member);
        }
    }
}
} }
