// Adapted from the CPPGM Foundation PA4 handouts/starter; see NOTICE.
#pragma once
#include "preprocess/expression.h"
#include <deque>
#include <memory>
#include <vector>

namespace cppgm {

struct PreprocessStats {
    std::size_t source_bytes = 0, files = 0, directives = 0, invocations = 0;
    std::size_t argument_prescans = 0, replacement_tokens = 0, output_tokens = 0;
    std::size_t paste_bytes = 0, arena_bytes = 0, context_nodes = 0;
    std::size_t max_pending = 0;
};

// Deferred tokens borrow immutable TU storage. Ancestry is a persistent set
// indexed by compact ID; permanent unavailable paint is independent of it.
struct ExpansionToken {
    PPToken token = {};
    IdentifierId filename = 0;
    std::uint32_t context = 0;
    int parameter = -1;
    bool space = false, unavailable = false, placemarker = false, substituted = false;
    bool is(const char* s) const { return token.spelling.equals(s); }
};

struct MacroDefinition {
    bool defined = false, function = false, variadic = false;
    unsigned char builtin = 0;
    std::vector<IdentifierId> parameters;
    std::vector<ExpansionToken> replacement;
};

class Preprocessor;
class MacroExpander {
public:
    explicit MacroExpander(Preprocessor& owner, bool source = false, bool expression = false);
    void push(const std::vector<ExpansionToken>& tokens);
    ExpansionToken next();
    bool empty() const { return pending_.empty(); }
private:
    friend class Preprocessor;
    Preprocessor& owner_;
    bool source_, expression_;
    std::vector<ExpansionToken> pending_;
    ExpansionToken take();
    std::vector<ExpansionToken> substitute(const ExpansionToken& head,
        const MacroDefinition& macro, std::vector<std::vector<ExpansionToken> >& args,
        std::uint32_t replacement_context);
};

// Pull cursor shared by the dump adapter and future parser. It retains sources,
// definitions and only language-required deferred tokens, never a whole output
// stream. All mutable state belongs to one primary source's translation unit.
class Preprocessor : public PPTokenSource {
public:
    Preprocessor(const std::string& path, const std::string& date, const std::string& time,
                 bool telemetry = false);
    PPToken next();
    IdentifierTable& identifiers() { return identifiers_; }
    const PreprocessStats& stats() const { return stats_; }
    const LexStats& lex_stats() const { return lex_stats_; }
private:
    friend class MacroExpander;
    struct Conditional { bool parent, active, taken, saw_else; };
    struct FileFrame {
        const SourceBuffer& source;
        PPTokenCursor cursor;
        IdentifierId filename;
        std::int64_t line_delta = 0;
        bool line_start = true, space = false;
        std::vector<Conditional> conditions;
        FileFrame(const SourceBuffer& s, IdentifierTable& ids, LexStats* stats, IdentifierId name)
            : source(s), cursor(s, ids, stats), filename(name) {}
        bool active() const { return conditions.empty() || conditions.back().active; }
    };
    struct ContextNode { std::uint32_t child[2]; };
    struct FileIdentity { std::uint64_t device, inode; bool used = false; };
    bool telemetry_;
    LexStats lex_stats_;
    PreprocessStats stats_;
    IdentifierTable identifiers_;
    std::deque<SourceBuffer> sources_;
    std::vector<std::unique_ptr<FileFrame> > files_;
    std::deque<std::vector<char> > arena_;
    std::vector<MacroDefinition> macros_;
    std::vector<int> parameter_index_;
    std::vector<ContextNode> contexts_;
    std::vector<FileIdentity> once_;
    std::size_t once_count_ = 0, counter_ = 0;
    MacroExpander expander_;
    bool boundary_ = false, end_file_ = false;
    std::vector<ExpansionToken> directive_;
    std::size_t next_line_ = 1;

    TextView save(TextView text);
    ExpansionToken stabilize(PPToken token, FileFrame& file);
    ExpansionToken raw();
    bool advance();
    void directive();
    void define(const std::vector<ExpansionToken>& line);
    bool condition(const std::vector<ExpansionToken>& tokens);
    void include(const std::string& path);
    void pragma(const std::string& text, IdentifierId filename);
    bool once(const std::string& path, bool insert);
    IdentifierId name(const std::string& text);
    std::string spelling(IdentifierId id) const;
    bool defined(IdentifierId id) const;
    static bool query(IdentifierId id, void* context);
    ExpansionToken generated(const std::string& text, const ExpansionToken& origin);
    ExpansionToken builtin(const ExpansionToken& head, unsigned kind,
                           MacroExpander& expansion);
    std::uint32_t paint(std::uint32_t root, IdentifierId id);
    std::uint32_t intersect(std::uint32_t a, std::uint32_t b);
    bool painted(std::uint32_t root, IdentifierId id) const;
    void reset_contexts();
};

std::string quote_pp_string(const std::string& text);
std::string decode_pp_string(const ExpansionToken& token);

} // namespace cppgm
