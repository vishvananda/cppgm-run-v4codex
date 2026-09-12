#pragma once
#include "posttoken/token.h"
#include "support/id_index.h"
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
    IdentifierId presumed_file = 0;
    std::uint32_t line = 0;
};

struct Token {
    IdentifierId text = 0;
    std::uint32_t location = 0;
    std::uint32_t literal = 0;
    // Absolute cursor ordinals, valid only while this deferred token is live.
    std::size_t delimiter_end = 0, angle_end = 0;
    ETokenType op = TOK_INVALID;
    PostTokenKind kind = PostTokenKind::eof;
    unsigned char packing = 0;
};

// One compact node array belongs to the TU. Indices remain stable on growth.
// Children are in source order; detail points to a structured name/type, never
// to a string serialization. Later semantic facts attach to NodeId directly.
struct Node {
    Kind kind = Kind::TranslationUnit;
    unsigned char flags = 0;
    ETokenType op = TOK_INVALID;
    IdentifierId text = 0;
    std::uint32_t location = 0;
    NodeId first = 0, last = 0, next = 0, detail = 0;
    std::uint32_t literal = 0;
};

struct AlignmentAttribute { NodeId operand; std::uint32_t next; bool type; };

struct ClassRegion { std::size_t begin, end; };

struct LiteralValue {
    LiteralKind kind;
    EFundamentalType type;
    IdentifierId suffix, prefix;
    std::array<char, 16> scalar;
    std::uint32_t offset, bytes, elements;
};

// Parsed nodes are stored once. Instantiated semantic occurrences retain only
// a source identity and a substitution-context identity in the shared ID space.
class NodePool {
public:
    struct Occurrence { NodeId source; std::uint32_t context; };
    explicit NodePool(std::size_t count = 0) : source_nodes(count), occurrences(count, {0,0}) {}
    Node& operator[](NodeId id) { return source_nodes[occurrences[id].source]; }
    const Node& operator[](NodeId id) const { return source_nodes[occurrences[id].source]; }
    void push_back(const Node& n) {
        occurrences.push_back({static_cast<NodeId>(source_nodes.size()),0}); source_nodes.push_back(n);
    }
    NodeId occurrence(NodeId source, std::uint32_t context) {
        NodeId id = occurrences.size(); occurrences.push_back({occurrences[source].source,context}); return id;
    }
    std::size_t size() const { return occurrences.size(); }
    std::size_t capacity() const { return occurrences.capacity(); }
    std::size_t parsed_size() const { return source_nodes.size(); }
    std::vector<Node> source_nodes;
    std::vector<Occurrence> occurrences;
};

class Ast {
public:
    explicit Ast(bool telemetry = false);
    NodeId make(Kind kind, Token token = Token());
    void append(NodeId parent, NodeId child);
    NodeId take_first(NodeId parent);
    Node& operator[](NodeId id) { return nodes[id]; }
    const Node& operator[](NodeId id) const { return nodes[id]; }
    // A view projects structural edges through a context without copying syntax.
    Node view(NodeId id) const {
        if (!nodes.occurrences[id].context) return nodes[id];
        return project_view(id);
    }
    Node project_view(NodeId id) const;
    NodeId instantiate(NodeId root, std::uint32_t context);
    NodeId projected(NodeId source, std::uint32_t context) const;
    std::uint32_t new_context() { return ++contexts; }
    std::uint32_t contexts = 0;
    IdIndex occurrence_index;
    std::uint32_t save_literal(const PostToken& token, IdentifierId prefix);
    bool telemetry;
    std::size_t node_growths = 0, location_growths = 0, literal_growths = 0;
    IdIndex alignment_owners, class_packing;
    std::vector<AlignmentAttribute> alignments = std::vector<AlignmentAttribute>(1);
    NodePool nodes;
    std::vector<Location> locations;
    std::vector<LiteralValue> literals;
    // Class nodes use their kind-discriminated auxiliary index for token ranges.
    std::vector<ClassRegion> class_regions;
    std::vector<char> literal_bytes;
};

class AstView {
    Ast& tree;
public:
    explicit AstView(Ast& a) : tree(a), telemetry(a.telemetry), nodes(a.nodes),
        literals(a.literals), literal_bytes(a.literal_bytes), class_regions(a.class_regions),
        alignment_owners(a.alignment_owners), class_packing(a.class_packing), alignments(a.alignments) {}
    Node operator[](NodeId id) const { return tree.view(id); }
    operator const Ast&() const { return tree; }
    NodeId instantiate(NodeId root, std::uint32_t context) { return tree.instantiate(root,context); }
    NodeId projected(NodeId source, std::uint32_t context) const { return tree.projected(source,context); }
    std::uint32_t new_context() { return tree.new_context(); }
    bool& telemetry;
    NodePool& nodes;
    std::vector<LiteralValue>& literals;
    std::vector<char>& literal_bytes;
    std::vector<ClassRegion>& class_regions;
    IdIndex& alignment_owners;
    IdIndex& class_packing;
    std::vector<AlignmentAttribute>& alignments;
};

const char* kind_name(Kind kind);
void write_ast(std::ostream& out, const Ast& ast, NodeId root, const IdentifierTable& ids);
void write_inline(std::ostream& out, const Ast& ast, NodeId node, const IdentifierTable& ids);

} }
