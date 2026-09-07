// Adapted from the CPPGM PA1 starter; see NOTICE for attribution.
#include "preprocess/source.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace cppgm {

bool TextView::equals(const char* text) const
{
    return std::strlen(text) == size && std::memcmp(data, text, size) == 0;
}

int hex_value(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool scalar_value(std::uint32_t c)
{
    return c <= 0x10ffff && !(c >= 0xd800 && c <= 0xdfff);
}

void append_utf8(std::string& out, int c)
{
    if (c < 0x80) out.push_back(static_cast<char>(c));
    else if (c < 0x800) {
        out.push_back(static_cast<char>(0xc0 | (c >> 6)));
        out.push_back(static_cast<char>(0x80 | (c & 63)));
    } else if (c < 0x10000) {
        out.push_back(static_cast<char>(0xe0 | (c >> 12)));
        out.push_back(static_cast<char>(0x80 | ((c >> 6) & 63)));
        out.push_back(static_cast<char>(0x80 | (c & 63)));
    } else {
        out.push_back(static_cast<char>(0xf0 | (c >> 18)));
        out.push_back(static_cast<char>(0x80 | ((c >> 12) & 63)));
        out.push_back(static_cast<char>(0x80 | ((c >> 6) & 63)));
        out.push_back(static_cast<char>(0x80 | (c & 63)));
    }
}

bool decimal_digit(int c) { return c >= '0' && c <= '9'; }

bool identifier_continue(int c)
{
    if (c < 0x80)
        return decimal_digit(c) || (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') || c == '_';
    // N3485 Annex E.1, inherited from the PA1 starter. Immutable metadata.
    static const int ranges[][2] = {
        {0xA8,0xA8}, {0xAA,0xAA}, {0xAD,0xAD}, {0xAF,0xAF},
        {0xB2,0xB5}, {0xB7,0xBA}, {0xBC,0xBE}, {0xC0,0xD6},
        {0xD8,0xF6}, {0xF8,0xFF}, {0x100,0x167F}, {0x1681,0x180D},
        {0x180F,0x1FFF}, {0x200B,0x200D}, {0x202A,0x202E},
        {0x203F,0x2040}, {0x2054,0x2054}, {0x2060,0x206F},
        {0x2070,0x218F}, {0x2460,0x24FF}, {0x2776,0x2793},
        {0x2C00,0x2DFF}, {0x2E80,0x2FFF}, {0x3004,0x3007},
        {0x3021,0x302F}, {0x3031,0x303F}, {0x3040,0xD7FF},
        {0xF900,0xFD3D}, {0xFD40,0xFDCF}, {0xFDF0,0xFE44},
        {0xFE47,0xFFFD}, {0x10000,0x1FFFD}, {0x20000,0x2FFFD},
        {0x30000,0x3FFFD}, {0x40000,0x4FFFD}, {0x50000,0x5FFFD},
        {0x60000,0x6FFFD}, {0x70000,0x7FFFD}, {0x80000,0x8FFFD},
        {0x90000,0x9FFFD}, {0xA0000,0xAFFFD}, {0xB0000,0xBFFFD},
        {0xC0000,0xCFFFD}, {0xD0000,0xDFFFD}, {0xE0000,0xEFFFD}
    };
    std::size_t lo = 0, hi = sizeof(ranges) / sizeof(ranges[0]);
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (c < ranges[mid][0]) hi = mid;
        else if (c > ranges[mid][1]) lo = mid + 1;
        else return true;
    }
    return false;
}

bool identifier_start(int c)
{
    return identifier_continue(c) && !decimal_digit(c) &&
           !(c >= 0x300 && c <= 0x36f) &&
           !(c >= 0x1dc0 && c <= 0x1dff) &&
           !(c >= 0x20d0 && c <= 0x20ff) &&
           !(c >= 0xfe20 && c <= 0xfe2f);
}

CharacterCursor::CharacterCursor(const SourceBuffer& source, LexStats* stats)
    : source_(source), stats_(stats)
{
    if (source.bytes.compare(0, 3, "\xef\xbb\xbf") == 0)
        consumed_.offset = 3;
    content_begin_ = consumed_.offset;
    scanned_ = consumed_;
}

SourceCharacter CharacterCursor::decode(Position& p)
{
    SourceCharacter result = {-1, p.offset, p.offset, p.line, p.column, true};
    if (p.offset == source_.bytes.size()) return result;
    const unsigned char lead = source_.bytes[p.offset++];
    std::uint32_t c = lead;
    if (lead >= 0x80) {
        int continuation;
        std::uint32_t minimum;
        if (lead >= 0xc2 && lead <= 0xdf) {
            continuation = 1; minimum = 0x80; c = lead & 31;
        } else if (lead >= 0xe0 && lead <= 0xef) {
            continuation = 2; minimum = 0x800; c = lead & 15;
        } else if (lead >= 0xf0 && lead <= 0xf4) {
            continuation = 3; minimum = 0x10000; c = lead & 7;
        } else throw std::runtime_error("invalid UTF-8 leading byte");
        for (int i = 0; i < continuation; ++i) {
            if (p.offset == source_.bytes.size())
                throw std::runtime_error("truncated UTF-8 sequence");
            unsigned char byte = source_.bytes[p.offset++];
            if ((byte & 0xc0) != 0x80)
                throw std::runtime_error("invalid UTF-8 continuation byte");
            c = (c << 6) | (byte & 63);
        }
        if (c < minimum || !scalar_value(c))
            throw std::runtime_error("invalid UTF-8 scalar value");
    }
    if (c == '\n') { ++p.line; p.column = 1; }
    else p.column += p.offset - result.begin;
    result.value = static_cast<int>(c);
    result.end = p.offset;
    if (stats_) ++stats_->decoded_units;
    return result;
}

SourceCharacter CharacterCursor::phase_one(Position& p)
{
    SourceCharacter c = decode(p);
    const std::string& bytes = source_.bytes;
    if (c.value == '?' && bytes.size() - p.offset >= 2 && bytes[p.offset] == '?') {
        const char* keys = "=/'()!<>-";
        const char* values = "#\\^[]|{}~";
        const char* match = std::strchr(keys, bytes[p.offset + 1]);
        if (bytes[p.offset + 1] != '\0' && match) {
            c.value = values[match - keys];
            p.offset += 2; p.column += 2;
            c.end = p.offset; c.unchanged = false;
        }
    }
    // Trigraphs may introduce the backslash of a UCN. Malformed UCN-like
    // text remains ordinary characters (in particular in comments).
    if (ucn_ && c.value == '\\' && p.offset < bytes.size() &&
        (bytes[p.offset] == 'u' || bytes[p.offset] == 'U')) {
        const std::size_t digits = bytes[p.offset] == 'u' ? 4 : 8;
        if (bytes.size() - p.offset >= digits + 1) {
            std::uint32_t value = 0;
            bool valid = true;
            for (std::size_t i = 1; i <= digits; ++i) {
                int digit = hex_value(static_cast<unsigned char>(bytes[p.offset + i]));
                if (digit < 0) { valid = false; break; }
                value = (value << 4) | static_cast<unsigned>(digit);
            }
            if (valid) {
                if (!scalar_value(value)) throw std::runtime_error("invalid UCN scalar value");
                c.value = static_cast<int>(value);
                p.offset += digits + 1; p.column += digits + 1;
                c.end = p.offset; c.unchanged = false;
            }
        }
    }
    return c;
}

SourceCharacter CharacterCursor::translate(Position& p)
{
    SourceCharacter c;
    for (;;) {
        c = raw_ || translated_ ? decode(p) : phase_one(p);
        // Only a physical LF can splice a source line. Do not speculatively
        // interpret the following escape before the literal scanner sees it.
        if (!raw_ && !translated_ && c.value == '\\' && p.offset < source_.bytes.size() &&
            source_.bytes[p.offset] == '\n') {
            decode(p);
            p.spliced_tail = true;
            continue;
        }
        break;
    }
    if (!raw_ && c.value == -1 && source_.bytes.size() != content_begin_ &&
        (p.last != '\n' || p.spliced_tail)) {
        c.value = '\n'; c.unchanged = false;
    }
    if (c.value != -1) { p.last = c.value; p.spliced_tail = false; }
    if (stats_) ++stats_->translated_units;
    return c;
}

const SourceCharacter& CharacterCursor::peek(std::size_t ahead)
{
    assert(ahead < 18);
    while (count_ <= ahead) {
        Pending& slot = pending_[(head_ + count_) % 18];
        slot.character = translate(scanned_);
        slot.after = scanned_;
        ++count_;
    }
    return pending_[(head_ + ahead) % 18].character;
}

SourceCharacter CharacterCursor::take()
{
    SourceCharacter result = peek();
    consumed_ = pending_[head_].after;
    head_ = (head_ + 1) % 18;
    --count_;
    return result;
}

void CharacterCursor::raw_mode(bool enabled)
{
    raw_ = enabled;
    scanned_ = consumed_;
    head_ = count_ = 0;
}

void CharacterCursor::ucn_mode(bool enabled)
{
    ucn_ = enabled;
    scanned_ = consumed_;
    head_ = count_ = 0;
}

} // namespace cppgm
