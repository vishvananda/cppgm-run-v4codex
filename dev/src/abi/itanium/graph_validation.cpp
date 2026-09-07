#include "abi/itanium/graph.h"
#include "abi/itanium/operations.h"
#include <stdexcept>

namespace abi_mangle {
namespace {
enum class Role { Type, Argument, Expression, Context, Entity };
bool accepts(Kind kind, Role role) {
    switch (role) {
    case Role::Type: return kind <= Kind::Lambda;
    case Role::Argument: return kind >= Kind::TypeArgument && kind <= Kind::EntityArgument;
    case Role::Expression:
        return kind == Kind::Value || (kind >= Kind::ExprParameter && kind <= Kind::EntityExpression);
    case Role::Context: return kind == Kind::RawContext || kind == Kind::FunctionEntity;
    case Role::Entity: return kind >= Kind::FunctionEntity && kind <= Kind::SymbolEntity;
    }
    return false;
}
void require(bool valid) {
    if (!valid) throw std::runtime_error("invalid ABI fact shape");
}
}

void Graph::validate(Kind kind, Id a, Id b, Id c, const std::vector<Id>& children) const {
    // Validate only the newly published node and its immediate edges. This
    // establishes an acyclic, kind-correct graph without whole-graph rescans.
    auto edge = [&](Id id, Role role) { require(accepts((*this)[id].kind, role)); };
    auto text = [&](Id id) { require(id && id <= strings_.size()); };
    auto sequence = [&](Role role) { for (Id id : children) edge(id, role); };
    auto qualifiers = [&](Id bits) { require(bits <= 15 && (bits & 12) != 12); };
    switch (kind) {
    case Kind::Name: if (a) edge(a, Role::Type); text(b); break;
    case Kind::Standard: abi_standard_substitution_code(static_cast<AbiStandardSubstitutionKind>(a)); break;
    case Kind::Builtin: abi_builtin_type_code(static_cast<AbiBuiltinTypeKind>(a)); break;
    case Kind::Parameter: require(b <= 1); break;
    case Kind::Template:
        require((*this)[a].kind <= Kind::Tagged || (*this)[a].kind == Kind::Parameter);
        sequence(Role::Argument); return;
    case Kind::Tagged:
        edge(a, Role::Type);
        require((*this)[a].kind <= Kind::Tagged);
        for (Id tag : children) text(tag);
        return;
    case Kind::Cv: require(b <= 3); edge(a, Role::Type); break;
    case Kind::Pointer: case Kind::Reference: case Kind::RvalueReference:
    case Kind::Pack: case Kind::Vector: case Kind::TypeArgument: case Kind::Value:
    case Kind::TemplateEntity: case Kind::SizeofType: edge(a, Role::Type); break;
    case Kind::Vendor: edge(a, Role::Type); text(b); break;
    case Kind::Array: edge(a, Role::Type); if (b) edge(b, Role::Expression); break;
    case Kind::Transform: text(b); sequence(Role::Type); return;
    case Kind::FunctionType:
        edge(a, Role::Type); qualifiers(b); require(c <= 1); sequence(Role::Type); return;
    case Kind::MemberPointer: edge(a, Role::Type); edge(b, Role::Type); break;
    case Kind::Decltype: case Kind::ExpressionArgument: case Kind::ExprPack:
        edge(a, Role::Expression); break;
    case Kind::Local: edge(a, Role::Context); text(b); sequence(Role::Type); return;
    case Kind::Lambda: edge(a, Role::Context); sequence(Role::Type); return;
    case Kind::RawContext: case Kind::SymbolEntity: text(a); break;
    case Kind::DependentValue:
        edge(a, Role::Type); require((*this)[b].kind == Kind::Value); break;
    case Kind::ArgumentPack: sequence(Role::Argument); return;
    case Kind::MemberTemplateEntity: edge(a, Role::Type); text(b); break;
    case Kind::EntityArgument: case Kind::EntityExpression:
        edge(a, Role::Entity); require(b <= 1); break;
    case Kind::ExprParameter: case Kind::ExprFunctionParameter: break;
    case Kind::Unary: edge(a, Role::Expression); operation_code(b); break;
    case Kind::Binary: edge(a, Role::Expression); edge(b, Role::Expression); operation_code(c); break;
    case Kind::Conditional:
        edge(a, Role::Expression); edge(b, Role::Expression); edge(c, Role::Expression); break;
    case Kind::Call: edge(a, Role::Expression); sequence(Role::Expression); return;
    case Kind::Conversion: edge(a, Role::Type); sequence(Role::Expression); return;
    case Kind::Cast: edge(a, Role::Type); edge(b, Role::Expression); operation_code(c); break;
    case Kind::TemplateId: text(a); sequence(Role::Argument); return;
    case Kind::TypeTrait: text(a); sequence(Role::Type); return;
    case Kind::Member: edge(a, Role::Type); text(b); require(c <= 1); sequence(Role::Argument); return;
    case Kind::ObjectMember:
        edge(a, Role::Expression); text(b); operation_code(c); sequence(Role::Argument); return;
    case Kind::VariableEntity: edge(a, Role::Type); require(b <= 1); break;
    case Kind::FunctionEntity: {
        require(children.size() >= 8);
        if (a) {
            require((*this)[a].kind == Kind::Name || (*this)[a].kind == Kind::Template);
        }
        if (children[0]) edge(children[0], Role::Context);
        if (children[1]) require((*this)[children[1]].kind == Kind::Local || (*this)[children[1]].kind == Kind::Lambda);
        require(a || children[1]); qualifiers(b); require(c < 24);
        if (children[2]) abi_terminal_word(static_cast<AbiTerminalKind>(children[2]));
        if (children[3]) edge(children[3], Role::Type);
        if (children[4]) text(children[4]);
        if (children[5]) edge(children[5], Role::Type);
        const std::size_t args_end = 8ull + children[6], params_end = args_end + children[7];
        require(params_end <= children.size());
        for (std::size_t i = 8; i < children.size(); ++i) {
            if (i < args_end) edge(children[i], Role::Argument);
            else if (i < params_end) edge(children[i], Role::Type);
            else text(children[i]);
        }
        return;
    }
    default: require(false);
    }
    require(children.empty());
}
} // namespace abi_mangle
