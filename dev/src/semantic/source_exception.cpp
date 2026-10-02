#include "semantic/analyzer.h"
#include "ir_symbol_model.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
Expression Analyzer::throw_expression(NodeId n, ScopeId scope)
{
    Expression result; result.type = types.fundamental(FT_VOID);
    auto source = ast.first(n);
    if (!source) return result;
    auto value = expression(source,scope);
    ThrowUse use; use.source = source; use.type = types.unqualified(decay(value.type));
    if (!unevaluated_depth) record_rtti_type(use.type);
    size(use.type);
    if (class_value(use.type)) {
        reject_abstract(use.type);
        use.destructor = destination_destructor(use.type,scope);
        auto id = source;
        while (ast.kind(id) == Kind::Parenthesized) id = ast.first(id);
        auto local = ast.kind(id) == Kind::IdExpression ? expressions[id].entity : 0;
        // C++11 [class.copy]: only a nonvolatile automatic object whose
        // scope ends within the innermost try can be implicitly moved here.
        // Parameters (including exception declarations) are excluded.
        auto boundary = scope;
        while (boundary && !try_scopes.get(boundary) && scopes[boundary].kind != ScopeKind::Function)
            boundary = scopes[boundary].parent;
        bool eligible = local && entities[local].kind == EntityKind::Variable &&
            ast.kind(entities[local].source) != Kind::ExceptionDeclaration &&
            !entities[local].is_static && !entities[local].external_decl &&
            !(types[entities[local].type].cv & 2) && class_value(entities[local].type) &&
            boundary && encloses(boundary,entities[local].owner);
        auto selected = return_conversion(source,value,use.type,eligible);
        record_class_initialization(n,use.type,source,&selected);
        use.conversion = class_initialization(n,use.type).conversion;
        auto c = conversions[use.conversion];
        if (c.kind == Conversion::Kind::Construction && !conversion_objects[c.materialization].elided)
            demand_member(c.function);
    } else {
        auto c = conversion(source,use.type);
        if (!c.valid()) throw std::runtime_error("invalid exception operand");
        apply_conversion(source,c);
        use.conversion = conversions.size(); conversions.push_back(c);
    }
    throw_index.put(n,throws.size()); throws.push_back(use);
    return result;
}
void Analyzer::resolve_handler(NodeId n, ScopeId parent, bool pattern, bool function_try)
{
    auto scope = make_scope(ScopeKind::Block,parent,0,0,!pattern);
    if (pattern) template_pattern_scopes.put(scope,1);
    facts.edit(n).scope = scope;
    if (function_try && constructor_member(current_function)) constructor_handler_scopes.put(scope,1);
    auto parameter = ast.first(n), specs = ast.first(parameter);
    if (ast.kind(specs) != Kind::Ellipsis) {
        auto decl = ast.next(specs);
        auto t = declarator(decl,specifiers(specs,scope),scope);
        if (types[t].kind == TypeKind::RRef) throw std::runtime_error("rvalue reference catch parameter");
        if (types[t].kind != TypeKind::LRef) t = decay(t);
        auto value = types.unqualified(value_type(t));
        if (!dependent_type(value)) {
            if (fundamental(value,FT_VOID)) throw std::runtime_error("void catch parameter");
            size(value);
            if (types[value].kind == TypeKind::Pointer && class_value(types[value].child))
                size(types[value].child); // [except.handle]/1: no pointer to incomplete class.
        }
        auto name = terminal(decl_name(decl));
        auto e = pattern ? pattern_declaration(EntityKind::Variable,scope,name,parameter,dependent_type(t)) :
            make_entity(EntityKind::Variable,scope,name,parameter);
        entities[e].type = t;
        facts.edit(n).entity = e; facts.edit(n).type = value;
        facts.edit(parameter).entity = e; facts.edit(parameter).type = t;
        if (decl) { facts.edit(decl).entity = e; facts.edit(decl).type = t; }
        if (!pattern) {
            if (types[t].kind == TypeKind::LRef && (types[value].kind == TypeKind::Pointer || types[value].kind == TypeKind::MemberPointer))
                handler_bindings.put(n,unsigned(types[types[t].child].cv & 1 ?
                    ir_model::CatchBinding::ConstReference : ir_model::CatchBinding::Reference));
            record_rtti_type(value);
            if (name) bind(scope,name,e);
            publish_template_binding(parameter,e);
            if (class_value(t)) {
                reject_abstract(t);
                Expression source; source.type = value; source.category = ValueCategory::Lvalue;
                auto c = conversion_value(source,t);
                if (!c.valid()) throw std::runtime_error("invalid catch parameter initialization");
                c = prepare_typed_conversion(source,c,scope,true);
                handler_initializations.put(n,conversions.size()); conversions.push_back(c);
                register_destruction(e);
            }
        }
    }
    auto body = ast.next(parameter);
    if (pattern) bind_template_statement(body,scope);
    else resolve_statement(body,scope);
}
} }
