#pragma once
#include "posttoken/token.h"
#include <iosfwd>
#include <vector>

namespace cppgm { namespace syntax {

typedef std::uint32_t NodeId;
enum class Kind : unsigned char {
    TranslationUnit,
    EmptyDeclaration,
    SimpleDeclaration,
    DeclSpecifiers,
    DeclSpecifier,
    TypeSpecifiers,
    TypeSpecifier,
    TypeName,
    TypeId,
    Declarator,
    AbstractDeclarator,
    NestedDeclarator,
    Pointer,
    Array,
    Parameters,
    Parameter,
    DefaultArgument,
    InitDeclarators,
    InitDeclarator,
    Initializer,
    ParenInitializer,
    Function,
    Identifier,
    IdExpression,
    Literal,
    KeywordLiteral,
    Binary,
    Assignment,
    Conditional,
    Unary,
    Postfix,
    Parenthesized,
    Call,
    Arguments,
    Subscript,
    Member,
    BracedInit,
    Cast,
    Sizeof,
    SizeofPack,
    TypeTrait,
    New,
    Delete,
    Global,
    ArrayDelete,
    Placement,
    ParenArguments,
    Lambda,
    LambdaIntroducer,
    LambdaDeclarator,
    LambdaSpecifier,
    Compound,
    ExpressionStatement,
    Return,
    Throw,
    Break,
    Continue,
    Goto,
    Label,
    Case,
    Default,
    If,
    Switch,
    While,
    Do,
    For,
    ForInit,
    RangeFor,
    RangeDeclaration,
    RangeInitializer,
    Condition,
    ConditionDeclaration,
    Iteration,
    Then,
    Else,
    Try,
    FunctionTry,
    Handler,
    ExceptionDeclaration,
    Ellipsis,
    Namespace,
    NamespaceAlias,
    UsingDirective,
    UsingDeclaration,
    Alias,
    Target,
    Inline,
    Linkage,
    StaticAssert,
    Message,
    Class,
    ClassForward,
    ClassKey,
    Access,
    Bases,
    Base,
    BaseName,
    Virtual,
    Enum,
    EnumKey,
    Enumerator,
    BitField,
    BitFieldDeclarator,
    SpecialMember,
    SpecialDefinition,
    MemberSpecifiers,
    Specifier,
    SpecialInitializer,
    CtorInitializer,
    MemInitializer,
    MemInitializerId,
    CvQualifier,
    FunctionQualifier,
    VirtSpecifier,
    Noexcept,
    TrailingReturn,
    Decltype,
    Template,
    TemplateParameters,
    TemplateParameterList,
    TypeParameter,
    NonTypeParameter,
    TemplateTemplate,
    ParameterKey,
    ParameterPack,
    DefaultTemplateArgument,
    ExplicitInstantiation,
    PackExpansion,
    PackExpression,
    Name,
    NamePart,
    TemplateArguments,
    Capture,
};

struct Location {
    std::uint32_t file = 0, begin = 0, end = 0;
};

struct Token {
    IdentifierId text = 0;
    Location location;
    std::uint32_t literal = 0;
    ETokenType op = TOK_INVALID;
    PostTokenKind kind = PostTokenKind::eof;
};

// One compact node array belongs to the TU. Indices remain stable on growth.
// Children are in source order; detail points to a structured name/type, never
// to a string serialization. Later semantic facts attach to NodeId directly.
struct Node {
    Kind kind = Kind::TranslationUnit;
    unsigned char flags = 0;
    ETokenType op = TOK_INVALID;
    IdentifierId text = 0;
    Location location;
    NodeId first = 0, last = 0, next = 0, detail = 0;
    std::uint32_t literal = 0;
};

struct LiteralValue {
    LiteralKind kind;
    EFundamentalType type;
    IdentifierId suffix;
    std::array<char, 16> scalar;
    std::uint32_t offset, bytes, elements;
};

class Ast {
public:
    Ast();
    NodeId make(Kind kind, Token token = Token());
    void append(NodeId parent, NodeId child);
    Node& operator[](NodeId id) { return nodes[id]; }
    const Node& operator[](NodeId id) const { return nodes[id]; }
    std::uint32_t save_literal(const PostToken& token);
    std::vector<Node> nodes;
    std::vector<LiteralValue> literals;
    std::vector<char> literal_bytes;
};

const char* kind_name(Kind kind);
void write_ast(std::ostream& out, const Ast& ast, NodeId root, const IdentifierTable& ids);
void write_inline(std::ostream& out, const Ast& ast, NodeId node, const IdentifierTable& ids);

} }
