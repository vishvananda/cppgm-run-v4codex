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
    std::size_t captured_tokens = 0, borrowed_arguments = 0, max_prescan_depth = 0;
    std::size_t max_context_nodes = 0, scratch_growths = 0;
    std::size_t task_slabs = 0, argument_growths = 0, prescan_output_growths = 0;
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
    bool empty() const;
private:
    friend class Preprocessor;
    // A captured invocation is indexed once. Nested prescans borrow slices and
    // jump over balanced subexpressions instead of copying/scanning each tail.
    struct ArgumentStorage {
        std::vector<ExpansionToken> tokens;
        std::vector<std::size_t> boundary;
        std::vector<std::size_t> index_stack;
        void index();
    };
    struct Slice {
        const ArgumentStorage* storage = 0;
        std::size_t begin = 0, end = 0;
    };
    struct Argument {
        Slice raw;
        std::vector<ExpansionToken> expanded;
        bool ready = false;
    };
    struct Invocation {
        bool active = false;
        ExpansionToken head;
        std::uint32_t replacement_context = 0;
        IdentifierId macro = 0;
        std::size_t prescan = 0, waiting = 0;
        std::size_t argument_count = 0;
        ArgumentStorage captured;
        std::vector<Argument> arguments;
    };
    struct Task {
        Slice input;
        std::size_t position = 0, last_index = 0;
        bool last_from_slice = false;
        std::vector<ExpansionToken> pending, output, replacement;
        Invocation invocation;
    };
    Preprocessor& owner_;
    bool source_, expression_;
    // Stable slabs preserve borrowed ArgumentStorage addresses. Reuse each
    // depth's buffers across invocations, releasing the pool with the expander.
    // The root is inline so ordinary text/directives never allocate a slab.
    Task root_;
    enum { tasks_per_slab = 32 };
    std::vector<std::unique_ptr<Task[]> > task_slabs_;
    std::size_t depth_ = 0;
    Task& task();
    void descend(Slice input);
    ExpansionToken take();
    void collect(const ExpansionToken& open, const MacroDefinition& macro);
    bool resume();
    void substitute(Invocation& invocation, const MacroDefinition& macro);
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
    struct SpellingArena {
        std::deque<std::vector<char> > slabs;
        std::size_t current = 0, bytes = 0;
        TextView save(TextView text);
        void rewind();
    };
    bool telemetry_;
    LexStats lex_stats_;
    PreprocessStats stats_;
    IdentifierTable identifiers_;
    std::deque<SourceBuffer> sources_;
    std::vector<std::unique_ptr<FileFrame> > files_;
    SpellingArena persistent_, transient_;
    std::vector<MacroDefinition> macros_;
    std::vector<int> parameter_index_;
    std::vector<ContextNode> contexts_;
    std::vector<FileIdentity> once_;
    std::size_t once_count_ = 0, counter_ = 0;
    MacroExpander expander_;
    bool boundary_ = false, end_file_ = false;
    std::vector<ExpansionToken> directive_;
    std::size_t next_line_ = 1;

    ExpansionToken stabilize(PPToken token, FileFrame& file, bool persistent = false);
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
