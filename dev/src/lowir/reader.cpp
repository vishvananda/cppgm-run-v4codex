#include "lowir/reader.h"
#include <cerrno>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>

namespace lowir_model {
Reader::Reader(Program& p, const std::string& text, const std::string& path)
    : p_(p), source_(text), path_(path) { p_.stats.source_bytes += text.size(); advance(); }
void Reader::advance()
{
    while (pos_ < source_.size()) {
        char c = source_[pos_];
        if (c == '#' || (c == '/' && pos_ + 1 < source_.size() && source_[pos_+1] == '/')) {
            while (pos_ < source_.size() && source_[pos_] != '\n') ++pos_;
        } else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (c == '\n') ++line_;
            ++pos_;
        } else break;
    }
    std::size_t begin = pos_;
    if (pos_ < source_.size()) {
        char c = source_[pos_++];
        if (c == '-' && pos_ < source_.size() && source_[pos_] == '>') ++pos_;
        else if (!std::strchr("(){}[],:=+-", c)) {
            while (pos_ < source_.size()) {
                char d = source_[pos_];
                if (std::strchr(" \t\r\n(){}[],:=#", d)) break;
                if ((d == '+' || d == '-') &&
                    !(pos_ > begin && std::strchr("eEpP", source_[pos_-1]) &&
                      (source_[begin] == '.' || (source_[begin] >= '0' && source_[begin] <= '9')))) break;
                ++pos_;
            }
        }
    }
    token_ = cppgm::TextView(source_.data() + begin, pos_ - begin);
    if (token_.size) ++p_.stats.tokens;
}
bool Reader::at(const char* s) const { return token_.equals(s); }
bool Reader::accept(const char* s) { if (!at(s)) return false; advance(); return true; }
void Reader::expect(const char* s)
{
    if (!accept(s)) throw ParseError(path_ + ":" + std::to_string(line_) + ": expected " + s);
}
std::string Reader::word()
{
    require(token_.size != 0, "unexpected end of input");
    std::string s(token_.data, token_.size);
    advance();
    return s;
}
Name Reader::name(char prefix)
{
    require(token_.size && (!prefix || (token_.data[0] == prefix && token_.size > 1)), "invalid name");
    Name id = p_.names.intern(token_);
    advance();
    return id;
}
std::uint64_t Reader::natural()
{
    std::string s = word();
    require(!s.empty() && s[0] >= '0' && s[0] <= '9', "expected nonnegative integer");
    char* end = 0;
    errno = 0;
    auto n = std::strtoull(s.c_str(), &end, 0);
    require(!errno && end == s.c_str() + s.size(), "invalid integer");
    return n;
}
void Reader::span(std::uint64_t& bytes, std::uint32_t& alignment)
{
    std::string s = word();
    auto x = s.find('x');
    auto parse = [](const std::string& v) -> std::uint64_t {
        require(!v.empty() && v[0] >= '0' && v[0] <= '9', "invalid storage span");
        char* end = 0;
        errno = 0;
        auto n = std::strtoull(v.c_str(), &end, 10);
        require(!errno && end == v.c_str() + v.size() && n, "invalid storage span");
        return n;
    };
    bytes = parse(s.substr(0, x));
    auto a = x == std::string::npos ? 1 : parse(s.substr(x + 1));
    require(a <= UINT32_MAX && !(a & (a - 1)), "invalid alignment");
    alignment = a;
}
Type Reader::type()
{
    static const char* const types[] = {"void","i1","i8","u8","i16","u16","i32","u32","i64","i128","f32","f64","f80","ptr"};
    std::string s = word();
    if (s == "i128a8") return Type::integer128_align8();
    for (unsigned k = 0; k < sizeof(types)/sizeof(*types); ++k) if (s == types[k]) return Type(Type::Kind(k));
    if (s.size() > 6 && s.substr(0,4) == "obj<" && s.back() == '>') {
        auto x = s.find('x', 4);
        require(x != std::string::npos, "missing object alignment");
        auto number = [](const std::string& v) -> std::uint32_t {
            char* end = 0;
            errno = 0;
            auto n = std::strtoull(v.c_str(), &end, 10);
            require(!v.empty() && v[0] >= '0' && v[0] <= '9' && !errno && end == v.c_str()+v.size() && n <= UINT32_MAX, "invalid object layout");
            return n;
        };
        return Type::object(number(s.substr(4, x-4)), number(s.substr(x+1, s.size()-x-2)));
    }
    throw ParseError("invalid LowIR type " + s);
}
Operand Reader::literal()
{
    bool negative = accept("-");
    std::string s = word();
    if (s == "nullptr") { require(!negative, "negative null pointer"); return Operand::null(); }
    bool special = s == "inf" || s == "INFINITY" || s == "nan" || s == "NAN" || s == "snan" || s == "SNAN";
    bool hex = s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X');
    bool fp = special || s.find('.') != std::string::npos ||
        s.find_first_of(hex ? "pP" : "eE") != std::string::npos;
    if (fp) {
        if (!special && !s.empty() && std::strchr("fFlL", s.back())) s.pop_back();
        long double value;
        bool signaling = s == "snan" || s == "SNAN";
        if (signaling) value = std::numeric_limits<long double>::quiet_NaN();
        else {
            char* end = 0;
            value = std::strtold(s.c_str(), &end);
            require(end != s.c_str() && end == s.c_str() + s.size(), "invalid floating literal");
        }
        Operand result = Operand::floating(negative ? -value : value, signaling);
        if (!special) {
            std::string signed_text = negative ? "-"+s : s;
            float f32 = std::strtof(signed_text.c_str(),0);
            double f64 = std::strtod(signed_text.c_str(),0);
            FloatingLiteral formats;
            std::memcpy(&formats.f32,&f32,4); std::memcpy(&formats.f64,&f64,8);
            formats.spelling = p_.intern(signed_text);
            p_.floating_literals.push_back(formats); result.ref = p_.floating_literals.size();
        }
        return result;
    }
    require(!s.empty() && s[0] >= '0' && s[0] <= '9', "expected scalar literal");
    unsigned base = 10, first = 0;
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; first = 2; }
    else if (s.size() > 1 && s[0] == '0') base = 8;
    std::uint64_t lo = 0, hi = 0;
    for (unsigned k = first; k < s.size(); ++k) {
        unsigned digit = s[k] >= '0' && s[k] <= '9' ? s[k]-'0' :
            s[k] >= 'a' && s[k] <= 'f' ? s[k]-'a'+10 : s[k] >= 'A' && s[k] <= 'F' ? s[k]-'A'+10 : 99;
        require(digit < base, "invalid integer literal");
        // Four base-2^32 limbs keep literal input independent of host wide types.
        std::uint64_t low = (lo & 0xffffffffu)*base + digit;
        std::uint64_t high = (lo >> 32)*base + (low >> 32);
        std::uint64_t carry = high >> 32;
        require(hi <= (~std::uint64_t(0)-carry)/base, "integer literal exceeds 128 bits");
        hi = hi*base + carry; lo = (high << 32) | std::uint32_t(low);
    }
    Operand result = Operand::integer(negative ? 0-lo : lo);
    result.negative_integer = negative && (lo || hi);
    if (hi) result.integer_high(negative ? ~hi + (lo == 0) : hi);
    return result;
}
Operand Reader::operand(FunctionBuilder& b)
{
    require(token_.size, "missing operand");
    switch (token_.data[0]) {
    case '%': return Operand::value(b.value(name('%')));
    case '$': return Operand::slot(b.slot(name('$')));
    case '@': return Operand::symbol(p_.symbol(name('@')));
    case '^': return Operand::label(b.block(name('^')));
    default: return literal();
    }
}
DebugLocation Reader::debug()
{
    DebugLocation d;
    if (!accept("!dbg")) return d;
    expect("(");
    d.file = name();
    expect(",");
    auto line = natural();
    expect(",");
    auto column = natural();
    require(line && column && line <= UINT32_MAX && column <= UINT32_MAX, "invalid debug location");
    d.line = line;
    d.column = column;
    expect(")");
    return d;
}
void Reader::read()
{
    while (token_.size) {
        bool declaration = accept("declare");
        if (accept("global")) global(declaration);
        else if (accept("function")) function(declaration);
        else if (!declaration && accept("alias")) {
            expect("object");
            ObjectAlias a;
            a.name = name();
            expect("=");
            a.target = p_.symbol(name('@'));
            p_.aliases.push_back(a);
        } else throw ParseError("expected top-level declaration");
    }
}
void read_program(Program& p, const std::string& text, const std::string& source) { Reader(p, text, source).read(); }
Program parse_lowir_program_text(const std::string& text, const std::string& source)
{
    Program p;
    read_program(p, text, source);
    validate(p);
    return p;
}
Program parse_lowir_program_files(const std::vector<std::string>& paths)
{
    Program p;
    for (const std::string& path : paths) {
        std::ifstream file(path, std::ios::binary);
        require(bool(file), "cannot open LowIR input");
        std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        read_program(p, text, path);
    }
    validate(p);
    return p;
}
std::string serialize_lowir_program(const Program& p) { std::ostringstream out; write_program(p, out); return out.str(); }
} // namespace lowir_model
