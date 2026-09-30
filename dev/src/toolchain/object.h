#pragma once
#include "native/encoding.h"
namespace cppgm { namespace toolchain {
// The disk contract contains final native bytes, indexed fixups and only facts
// needed by linking. It contains neither source/semantic graphs nor LowIR.
struct Symbol {
    lowir_model::Name name = 0;
    ir_model::SymbolBindingMode binding = ir_model::SBM_INTERNAL;
    ir_model::SymbolRole role = ir_model::SR_NONE;
    unsigned parameters = 0;
    bool fragment = true;
};
struct Object {
    native::Image image;
    cppgm::IdentifierTable names;
    std::vector<Symbol> symbols;
    unsigned alignment = 16;
    explicit Object(unsigned count = 0) : image(count), symbols(image.symbols.size()) {}
    std::string name(unsigned id) const;
    lowir_model::Name intern(const std::string& text);
};
Object compile_object(const lowir_model::Program&, native::Statistics&);
void write_object(const Object&, const std::string&);
Object read_object(const std::string&);
Object read_elf(const std::vector<unsigned char>&);
class Linker {
    native::Image image_;
    cppgm::IdentifierTable names_;
    lowir_model::NameIndex externals_;
    std::vector<Symbol> symbols_;
    std::vector<unsigned> initializers_, finalizers_;
    unsigned entry_ = 0, parameters_ = 0, alignment_ = 16;
    unsigned new_symbol();
public:
    Linker();
    void add(Object&&);
    std::size_t finish(const std::string&);
};
int run(const std::vector<std::string>&);
} }
