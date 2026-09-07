// Adapted from the CPPGM PA1 starter; see NOTICE for attribution.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace cppgm {

struct TextView {
    const char* data;
    std::size_t size;
    TextView(const char* data = "", std::size_t size = 0) : data(data), size(size) {}
    bool equals(const char* text) const;
};

// A translation unit owns its sources and identifier table until all consumers
// finish. Cursors borrow them; physical byte offsets never change.
struct SourceBuffer {
    const std::string bytes;
    const std::uint32_t file_id;
    explicit SourceBuffer(std::string bytes, std::uint32_t file_id = 0)
        : bytes(std::move(bytes)), file_id(file_id) {}
};

struct LexStats {
    std::size_t decoded_units = 0;
    std::size_t translated_units = 0;
    std::size_t tokens = 0;
    std::size_t spelling_bytes = 0;
    std::size_t intern_probes = 0;
    std::size_t storage_growths = 0;
};

int hex_value(int c);
void append_utf8(std::string& out, int c);
bool identifier_start(int c);
bool identifier_continue(int c);
bool decimal_digit(int c);

struct SourceCharacter {
    int value;
    std::size_t begin, end, line, column;
    bool unchanged;
};

// At most a raw delimiter plus ')' and '"' of lookahead. No translated file
// buffer. Raw mode rewinds only this bounded speculative window.
class CharacterCursor {
public:
    explicit CharacterCursor(const SourceBuffer& source, LexStats* stats = 0);
    const SourceCharacter& peek(std::size_t ahead = 0);
    SourceCharacter take();
    void raw_mode(bool enabled);

private:
    struct Position {
        std::size_t offset = 0, line = 1, column = 1;
        int last = -1;
    };
    struct Pending { SourceCharacter character; Position after; };
    const SourceBuffer& source_;
    LexStats* stats_;
    Position consumed_, scanned_;
    Pending pending_[18];
    std::size_t head_ = 0, count_ = 0;
    bool raw_ = false;

    SourceCharacter decode(Position& position);
    SourceCharacter phase_one(Position& position);
    SourceCharacter translate(Position& position);
};

} // namespace cppgm
