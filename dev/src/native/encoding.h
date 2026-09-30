#pragma once
#include "native/model.h"
namespace native {
constexpr unsigned executable_code_offset = 64+2*56;
struct Fixup {
    enum Kind { RelativeSymbol, AbsoluteSymbol, ThreadOffset, Absolute32, Absolute32Signed, CallSymbol, GotSymbol } kind = RelativeSymbol;
    std::size_t offset = 0, end = 0;
    unsigned symbol = 0;
    std::int64_t addend = 0;
    // Producer-recorded definition identity; zero denotes unconditional bytes.
    unsigned owner = 0;
};
struct UnwindRecord {
    unsigned symbol = 0;
    std::size_t begin = 0, end = 0, lsda = 0, cfi_pc = 0;
    bool has_lsda = false;
    std::vector<unsigned char> cfi;
};
struct Image {
    std::vector<unsigned char> code, data;
    std::vector<Fixup> code_fixups, data_fixups;
    std::vector<std::uint64_t> symbols;
    std::vector<bool> data_symbols, defined;
    std::vector<bool> indirect_functions;
    std::vector<unsigned> tls_targets;
    bool has_tls = false, host = false;
    std::vector<UnwindRecord> unwind;
    std::vector<unsigned char> lsda;
    std::vector<Fixup> lsda_fixups;
    unsigned host_resume = 0;
    unsigned runtime_begin;
    explicit Image(std::size_t count) : symbols(count+1+unsigned(RuntimeEntity::Count)), data_symbols(symbols.size()), defined(symbols.size()), tls_targets(symbols.size()), runtime_begin(count+1) {}
    Operand runtime(RuntimeEntity entity) const { return Operand::symbol(SymbolId(runtime_begin+unsigned(entity)),false); }
};
class Encoder {
    Image& image;
    std::vector<unsigned char>& code;
    struct Branch { std::size_t offset; unsigned label; };
    std::vector<Branch> branches;
    std::vector<std::size_t> labels;
    std::vector<SymbolId> label_owners;
    const Function* function = nullptr;
    unsigned epilogue = 0, resume_label = 0;
    struct HostSite { std::size_t begin, end; unsigned handler; };
    std::vector<HostSite> host_sites;
    std::vector<unsigned> host_landings;
    std::vector<std::size_t> host_landing_offsets;
    UnwindRecord unwind_record;
    void host_landing_pads();
    void host_tables();
    void cfi_advance(std::size_t from, std::size_t to);
    void host_runtime(const Instruction&);
    void byte(unsigned n);
    void number(std::uint64_t n, unsigned width);
    void modrm(unsigned reg, const Operand& operand);
    void form(unsigned opcode, unsigned width, unsigned reg, Operand rm, unsigned immediate_bytes = 0, std::uint64_t immediate = 0, unsigned prefix = 0);
    void mov(Operand to, Operand from);
    void load(Operand to, Operand from, Type type, bool sign);
    void store(Operand to, Operand from, Type type);
    void arithmetic(const Instruction& i);
    void multiply(const Instruction& i);
    void bulk(const Instruction& i);
    void floating(const Instruction& i);
    void fmove(Operand to, Operand from, Type type);
    void sse(unsigned opcode, Type type, int reg, Operand rm);
    Operand scratch(unsigned offset = 0) const;
    void x87_load(Operand from, Type type, unsigned scratch_offset = 0);
    void x87_store(Operand to, Type type);
    void float_compare(Operand left, Operand right, Type type);
    void wide_to_float(const Instruction& i);
    void float_to_wide(const Instruction& i);
    void float_convert(const Instruction& i);
    std::size_t local_jump(int condition);
    void local_target(std::size_t offset);
    void instruction(const Instruction& i);
    void branch(unsigned label, int condition = -1);
    void call(Operand target);
    void epilogue_code();
    void runtime(const Instruction& i);
    void tls_address(Operand to, Operand symbol);
    void tls_startup();
public:
    explicit Encoder(Image& image) : image(image), code(image.code) {}
    void startup(const std::vector<Instruction>& instructions);
    void encode(const Function& function);
};
std::vector<Instruction> startup(SymbolId, unsigned, const std::vector<SymbolId>&, const std::vector<SymbolId>&, Type argc_type = Type::I32);
std::vector<Instruction> startup(const lowir_model::Program& p);
void encode_data(const lowir_model::Program& p, Image& image);
void write_executable(Image& image, const std::string& path);
void compile_image(const lowir_model::Program&, Image&, const std::vector<Instruction>&, std::ostream*, Statistics&);
void compile(const lowir_model::Program& p, const std::string& output, std::ostream* mir, Statistics& stats);
} // namespace native
