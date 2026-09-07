// (C) 2013 CPPGM Foundation www.cppgm.org. All rights reserved.
// Immutable vocabulary adapted from the PA2 starter; see NOTICE.
#include "posttoken/token_types.h"
#include <algorithm>
#include <cstring>

namespace cppgm {

ETokenType classify_simple(TextView spelling)
{
    struct Entry { const char* spelling; ETokenType type; };
    static const Entry entries[] = {
        {"!", OP_LNOT},
        {"!=", OP_NE},
        {"%", OP_MOD},
        {"%=", OP_MODASS},
        {"%>", OP_RBRACE},
        {"&", OP_AMP},
        {"&&", OP_LAND},
        {"&=", OP_BANDASS},
        {"(", OP_LPAREN},
        {")", OP_RPAREN},
        {"*", OP_STAR},
        {"*=", OP_STARASS},
        {"+", OP_PLUS},
        {"++", OP_INC},
        {"+=", OP_PLUSASS},
        {",", OP_COMMA},
        {"-", OP_MINUS},
        {"--", OP_DEC},
        {"-=", OP_MINUSASS},
        {"->", OP_ARROW},
        {"->*", OP_ARROWSTAR},
        {".", OP_DOT},
        {".*", OP_DOTSTAR},
        {"...", OP_DOTS},
        {"/", OP_DIV},
        {"/=", OP_DIVASS},
        {":", OP_COLON},
        {"::", OP_COLON2},
        {":>", OP_RSQUARE},
        {";", OP_SEMICOLON},
        {"<", OP_LT},
        {"<%", OP_LBRACE},
        {"<:", OP_LSQUARE},
        {"<<", OP_LSHIFT},
        {"<<=", OP_LSHIFTASS},
        {"<=", OP_LE},
        {"=", OP_ASS},
        {"==", OP_EQ},
        {">", OP_GT},
        {">=", OP_GE},
        {">>", OP_RSHIFT},
        {">>=", OP_RSHIFTASS},
        {"?", OP_QMARK},
        {"[", OP_LSQUARE},
        {"]", OP_RSQUARE},
        {"^", OP_XOR},
        {"^=", OP_XORASS},
        {"alignas", KW_ALIGNAS},
        {"alignof", KW_ALIGNOF},
        {"and", OP_LAND},
        {"and_eq", OP_BANDASS},
        {"asm", KW_ASM},
        {"auto", KW_AUTO},
        {"bitand", OP_AMP},
        {"bitor", OP_BOR},
        {"bool", KW_BOOL},
        {"break", KW_BREAK},
        {"case", KW_CASE},
        {"catch", KW_CATCH},
        {"char", KW_CHAR},
        {"char16_t", KW_CHAR16_T},
        {"char32_t", KW_CHAR32_T},
        {"class", KW_CLASS},
        {"compl", OP_COMPL},
        {"const", KW_CONST},
        {"const_cast", KW_CONST_CAST},
        {"constexpr", KW_CONSTEXPR},
        {"continue", KW_CONTINUE},
        {"decltype", KW_DECLTYPE},
        {"default", KW_DEFAULT},
        {"delete", KW_DELETE},
        {"do", KW_DO},
        {"double", KW_DOUBLE},
        {"dynamic_cast", KW_DYNAMIC_CAST},
        {"else", KW_ELSE},
        {"enum", KW_ENUM},
        {"explicit", KW_EXPLICIT},
        {"export", KW_EXPORT},
        {"extern", KW_EXTERN},
        {"false", KW_FALSE},
        {"float", KW_FLOAT},
        {"for", KW_FOR},
        {"friend", KW_FRIEND},
        {"goto", KW_GOTO},
        {"if", KW_IF},
        {"inline", KW_INLINE},
        {"int", KW_INT},
        {"long", KW_LONG},
        {"mutable", KW_MUTABLE},
        {"namespace", KW_NAMESPACE},
        {"new", KW_NEW},
        {"noexcept", KW_NOEXCEPT},
        {"not", OP_LNOT},
        {"not_eq", OP_NE},
        {"nullptr", KW_NULLPTR},
        {"operator", KW_OPERATOR},
        {"or", OP_LOR},
        {"or_eq", OP_BORASS},
        {"private", KW_PRIVATE},
        {"protected", KW_PROTECTED},
        {"public", KW_PUBLIC},
        {"register", KW_REGISTER},
        {"reinterpret_cast", KW_REINTERPET_CAST},
        {"return", KW_RETURN},
        {"short", KW_SHORT},
        {"signed", KW_SIGNED},
        {"sizeof", KW_SIZEOF},
        {"static", KW_STATIC},
        {"static_assert", KW_STATIC_ASSERT},
        {"static_cast", KW_STATIC_CAST},
        {"struct", KW_STRUCT},
        {"switch", KW_SWITCH},
        {"template", KW_TEMPLATE},
        {"this", KW_THIS},
        {"thread_local", KW_THREAD_LOCAL},
        {"throw", KW_THROW},
        {"true", KW_TRUE},
        {"try", KW_TRY},
        {"typedef", KW_TYPEDEF},
        {"typeid", KW_TYPEID},
        {"typename", KW_TYPENAME},
        {"union", KW_UNION},
        {"unsigned", KW_UNSIGNED},
        {"using", KW_USING},
        {"virtual", KW_VIRTUAL},
        {"void", KW_VOID},
        {"volatile", KW_VOLATILE},
        {"wchar_t", KW_WCHAR_T},
        {"while", KW_WHILE},
        {"xor", OP_XOR},
        {"xor_eq", OP_XORASS},
        {"{", OP_LBRACE},
        {"|", OP_BOR},
        {"|=", OP_BORASS},
        {"||", OP_LOR},
        {"}", OP_RBRACE},
        {"~", OP_COMPL},
    };
    std::size_t lo = 0, hi = sizeof(entries) / sizeof(entries[0]);
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        const char* word = entries[mid].spelling;
        std::size_t length = std::strlen(word);
        int cmp = std::memcmp(spelling.data, word, std::min(spelling.size, length));
        if (!cmp) cmp = spelling.size < length ? -1 : spelling.size > length ? 1 : 0;
        if (!cmp) return entries[mid].type;
        if (cmp < 0) hi = mid; else lo = mid + 1;
    }
    return TOK_INVALID;
}

const char* simple_name(ETokenType type)
{
    switch (type) {
    case KW_ALIGNAS: return "KW_ALIGNAS";
    case KW_ALIGNOF: return "KW_ALIGNOF";
    case KW_ASM: return "KW_ASM";
    case KW_AUTO: return "KW_AUTO";
    case KW_BOOL: return "KW_BOOL";
    case KW_BREAK: return "KW_BREAK";
    case KW_CASE: return "KW_CASE";
    case KW_CATCH: return "KW_CATCH";
    case KW_CHAR: return "KW_CHAR";
    case KW_CHAR16_T: return "KW_CHAR16_T";
    case KW_CHAR32_T: return "KW_CHAR32_T";
    case KW_CLASS: return "KW_CLASS";
    case KW_CONST: return "KW_CONST";
    case KW_CONSTEXPR: return "KW_CONSTEXPR";
    case KW_CONST_CAST: return "KW_CONST_CAST";
    case KW_CONTINUE: return "KW_CONTINUE";
    case KW_DECLTYPE: return "KW_DECLTYPE";
    case KW_DEFAULT: return "KW_DEFAULT";
    case KW_DELETE: return "KW_DELETE";
    case KW_DO: return "KW_DO";
    case KW_DOUBLE: return "KW_DOUBLE";
    case KW_DYNAMIC_CAST: return "KW_DYNAMIC_CAST";
    case KW_ELSE: return "KW_ELSE";
    case KW_ENUM: return "KW_ENUM";
    case KW_EXPLICIT: return "KW_EXPLICIT";
    case KW_EXPORT: return "KW_EXPORT";
    case KW_EXTERN: return "KW_EXTERN";
    case KW_FALSE: return "KW_FALSE";
    case KW_FLOAT: return "KW_FLOAT";
    case KW_FOR: return "KW_FOR";
    case KW_FRIEND: return "KW_FRIEND";
    case KW_GOTO: return "KW_GOTO";
    case KW_IF: return "KW_IF";
    case KW_INLINE: return "KW_INLINE";
    case KW_INT: return "KW_INT";
    case KW_LONG: return "KW_LONG";
    case KW_MUTABLE: return "KW_MUTABLE";
    case KW_NAMESPACE: return "KW_NAMESPACE";
    case KW_NEW: return "KW_NEW";
    case KW_NOEXCEPT: return "KW_NOEXCEPT";
    case KW_NULLPTR: return "KW_NULLPTR";
    case KW_OPERATOR: return "KW_OPERATOR";
    case KW_PRIVATE: return "KW_PRIVATE";
    case KW_PROTECTED: return "KW_PROTECTED";
    case KW_PUBLIC: return "KW_PUBLIC";
    case KW_REGISTER: return "KW_REGISTER";
    case KW_REINTERPET_CAST: return "KW_REINTERPET_CAST";
    case KW_RETURN: return "KW_RETURN";
    case KW_SHORT: return "KW_SHORT";
    case KW_SIGNED: return "KW_SIGNED";
    case KW_SIZEOF: return "KW_SIZEOF";
    case KW_STATIC: return "KW_STATIC";
    case KW_STATIC_ASSERT: return "KW_STATIC_ASSERT";
    case KW_STATIC_CAST: return "KW_STATIC_CAST";
    case KW_STRUCT: return "KW_STRUCT";
    case KW_SWITCH: return "KW_SWITCH";
    case KW_TEMPLATE: return "KW_TEMPLATE";
    case KW_THIS: return "KW_THIS";
    case KW_THREAD_LOCAL: return "KW_THREAD_LOCAL";
    case KW_THROW: return "KW_THROW";
    case KW_TRUE: return "KW_TRUE";
    case KW_TRY: return "KW_TRY";
    case KW_TYPEDEF: return "KW_TYPEDEF";
    case KW_TYPEID: return "KW_TYPEID";
    case KW_TYPENAME: return "KW_TYPENAME";
    case KW_UNION: return "KW_UNION";
    case KW_UNSIGNED: return "KW_UNSIGNED";
    case KW_USING: return "KW_USING";
    case KW_VIRTUAL: return "KW_VIRTUAL";
    case KW_VOID: return "KW_VOID";
    case KW_VOLATILE: return "KW_VOLATILE";
    case KW_WCHAR_T: return "KW_WCHAR_T";
    case KW_WHILE: return "KW_WHILE";
    case OP_LBRACE: return "OP_LBRACE";
    case OP_RBRACE: return "OP_RBRACE";
    case OP_LSQUARE: return "OP_LSQUARE";
    case OP_RSQUARE: return "OP_RSQUARE";
    case OP_LPAREN: return "OP_LPAREN";
    case OP_RPAREN: return "OP_RPAREN";
    case OP_BOR: return "OP_BOR";
    case OP_XOR: return "OP_XOR";
    case OP_COMPL: return "OP_COMPL";
    case OP_AMP: return "OP_AMP";
    case OP_LNOT: return "OP_LNOT";
    case OP_SEMICOLON: return "OP_SEMICOLON";
    case OP_COLON: return "OP_COLON";
    case OP_DOTS: return "OP_DOTS";
    case OP_QMARK: return "OP_QMARK";
    case OP_COLON2: return "OP_COLON2";
    case OP_DOT: return "OP_DOT";
    case OP_DOTSTAR: return "OP_DOTSTAR";
    case OP_PLUS: return "OP_PLUS";
    case OP_MINUS: return "OP_MINUS";
    case OP_STAR: return "OP_STAR";
    case OP_DIV: return "OP_DIV";
    case OP_MOD: return "OP_MOD";
    case OP_ASS: return "OP_ASS";
    case OP_LT: return "OP_LT";
    case OP_GT: return "OP_GT";
    case OP_PLUSASS: return "OP_PLUSASS";
    case OP_MINUSASS: return "OP_MINUSASS";
    case OP_STARASS: return "OP_STARASS";
    case OP_DIVASS: return "OP_DIVASS";
    case OP_MODASS: return "OP_MODASS";
    case OP_XORASS: return "OP_XORASS";
    case OP_BANDASS: return "OP_BANDASS";
    case OP_BORASS: return "OP_BORASS";
    case OP_LSHIFT: return "OP_LSHIFT";
    case OP_RSHIFT: return "OP_RSHIFT";
    case OP_RSHIFTASS: return "OP_RSHIFTASS";
    case OP_LSHIFTASS: return "OP_LSHIFTASS";
    case OP_EQ: return "OP_EQ";
    case OP_NE: return "OP_NE";
    case OP_LE: return "OP_LE";
    case OP_GE: return "OP_GE";
    case OP_LAND: return "OP_LAND";
    case OP_LOR: return "OP_LOR";
    case OP_INC: return "OP_INC";
    case OP_DEC: return "OP_DEC";
    case OP_COMMA: return "OP_COMMA";
    case OP_ARROWSTAR: return "OP_ARROWSTAR";
    case OP_ARROW: return "OP_ARROW";
    default: return "invalid";
    }
}

const char* fundamental_name(EFundamentalType type)
{
    static const char* const names[] = {
        "signed char",
        "short int",
        "int",
        "long int",
        "long long int",
        "unsigned char",
        "unsigned short int",
        "unsigned int",
        "unsigned long int",
        "unsigned long long int",
        "wchar_t",
        "char",
        "char16_t",
        "char32_t",
        "bool",
        "float",
        "double",
        "long double",
        "void",
        "nullptr_t",
    };
    return names[type];
}

unsigned fundamental_width(EFundamentalType type)
{
    static const unsigned char widths[] = {1, 2, 4, 8, 8, 1, 2, 4, 8, 8, 4, 1, 2, 4, 1, 4, 8, 16, 0, 8};
    return widths[type];
}

}
