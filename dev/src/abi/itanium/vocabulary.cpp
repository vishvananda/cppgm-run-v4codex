#include "abi/itanium/abi_mangle_terminal.h"
#include "abi/itanium/abi_mangle_type_vocabulary.h"
#include <stdexcept>

namespace abi_mangle {
namespace {
struct WordCode { const char* word; const char* code; };
const WordCode builtins[] = {
    {"", ""}, {"void", "v"}, {"bool", "b"}, {"char", "c"},
    {"schar", "a"}, {"uchar", "h"}, {"wchar", "w"}, {"char16", "Ds"},
    {"char32", "Di"}, {"short", "s"}, {"ushort", "t"}, {"int", "i"},
    {"uint", "j"}, {"long", "l"}, {"ulong", "m"}, {"longlong", "x"},
    {"ulonglong", "y"}, {"int128", "n"}, {"uint128", "o"}, {"float", "f"},
    {"double", "d"}, {"longdouble", "e"}, {"float16", "DF16_"},
    {"float32", "DF32_"}, {"float32x", "DF32x"}, {"float64", "DF64_"},
    {"float64x", "DF64x"}, {"stdfloat128", "DF128_"}, {"float128", "g"},
    {"complex-float", "Cf"}, {"complex-double", "Cd"},
    {"complex-longdouble", "Ce"}, {"nullptr", "Dn"}
};
const WordCode terminals[] = {
    {"", ""}, {"constructor-complete", "C1"}, {"constructor-base", "C2"},
    {"destructor-deleting", "D0"}, {"destructor-complete", "D1"},
    {"destructor-base", "D2"}, {"literal", "li"}, {"plus", "pl"},
    {"minus", "mi"}, {"unary-plus", "ps"}, {"binary-plus", "pl"},
    {"unary-minus", "ng"}, {"binary-minus", "mi"}, {"address-of", "ad"},
    {"deref", "de"}, {"new", "nw"}, {"new-array", "na"}, {"delete", "dl"},
    {"delete-array", "da"}, {"multiply", "ml"}, {"divide", "dv"},
    {"remainder", "rm"}, {"bit-and", "an"}, {"bit-or", "or"}, {"bit-xor", "eo"},
    {"complement", "co"}, {"assign", "aS"}, {"plus-assign", "pL"},
    {"minus-assign", "mI"}, {"multiply-assign", "mL"}, {"divide-assign", "dV"},
    {"remainder-assign", "rM"}, {"and-assign", "aN"}, {"or-assign", "oR"},
    {"xor-assign", "eO"}, {"left-shift", "ls"}, {"right-shift", "rs"},
    {"left-shift-assign", "lS"}, {"right-shift-assign", "rS"}, {"equal", "eq"},
    {"not-equal", "ne"}, {"less", "lt"}, {"greater", "gt"}, {"less-equal", "le"},
    {"greater-equal", "ge"}, {"logical-not", "nt"}, {"logical-and", "aa"},
    {"logical-or", "oo"}, {"increment", "pp"}, {"decrement", "mm"},
    {"comma", "cm"}, {"member-pointer", "pm"}, {"arrow", "pt"},
    {"call", "cl"}, {"index", "ix"}
};
}
AbiBuiltinTypeKind abi_builtin_type_kind(const std::string& word, std::size_t* width) {
    if (width) *width = 0;
    for (unsigned i = 1; i < sizeof(builtins) / sizeof(*builtins); ++i)
        if (word == builtins[i].word) return static_cast<AbiBuiltinTypeKind>(i);
    return ABI_BUILTIN_TYPE_NONE;
}
bool abi_is_builtin_type_word(const std::string& word) {
    return abi_builtin_type_kind(word, nullptr) != ABI_BUILTIN_TYPE_NONE;
}
const char* abi_builtin_type_code(AbiBuiltinTypeKind kind) {
    if (kind <= ABI_BUILTIN_TYPE_NONE || kind > ABI_BUILTIN_TYPE_NULLPTR)
        throw std::runtime_error("unsupported builtin ABI type");
    return builtins[kind].code;
}
const char* abi_builtin_type_word(AbiBuiltinTypeKind kind) {
    if (kind <= ABI_BUILTIN_TYPE_NONE || kind > ABI_BUILTIN_TYPE_NULLPTR)
        throw std::runtime_error("unsupported builtin ABI type");
    return builtins[kind].word;
}
std::string abi_builtin_type_text_code(const std::string& word) {
    return abi_builtin_type_code(abi_builtin_type_kind(word, nullptr));
}
bool abi_find_terminal_kind(const std::string& word, AbiTerminalKind* kind) {
    if (word == "operator-call") { *kind = ABI_TERMINAL_CALL; return true; }
    for (unsigned i = 1; i < sizeof(terminals) / sizeof(*terminals); ++i)
        if (word == terminals[i].word) {
            *kind = static_cast<AbiTerminalKind>(i); return true;
        }
    return false;
}
AbiTerminalKind abi_terminal_kind(const std::string& word) {
    AbiTerminalKind kind;
    if (!abi_find_terminal_kind(word, &kind))
        throw std::runtime_error("unknown ABI terminal: " + word);
    return kind;
}
const char* abi_terminal_code(AbiTerminalKind kind, bool member, std::size_t params) {
    bool unary = params + (member ? 1 : 0) == 1;
    if (kind == ABI_TERMINAL_PLUS) return unary ? "ps" : "pl";
    if (kind == ABI_TERMINAL_MINUS) return unary ? "ng" : "mi";
    if (kind == ABI_TERMINAL_ADDRESS_OF) return unary ? "ad" : "an";
    if (kind == ABI_TERMINAL_DEREFERENCE) return unary ? "de" : "ml";
    if (kind <= ABI_TERMINAL_NONE || kind > ABI_TERMINAL_INDEX)
        throw std::runtime_error("invalid ABI terminal");
    return terminals[kind].code;
}
AbiStandardSubstitutionKind abi_standard_substitution_kind(const std::string& word) {
    const char* codes[] = {"", "Sa", "Sb", "Ss", "Si", "So", "Sd"};
    for (unsigned i = 1; i < 7; ++i)
        if (word == codes[i]) return static_cast<AbiStandardSubstitutionKind>(i);
    throw std::runtime_error("invalid standard ABI substitution");
}
const char* abi_standard_substitution_code(AbiStandardSubstitutionKind kind) {
    const char* codes[] = {"", "Sa", "Sb", "Ss", "Si", "So", "Sd"};
    if (kind <= ABI_STANDARD_SUBSTITUTION_TEXT || kind > ABI_STANDARD_SUBSTITUTION_IOSTREAM)
        throw std::runtime_error("invalid standard ABI substitution");
    return codes[kind];
}
} // namespace abi_mangle
