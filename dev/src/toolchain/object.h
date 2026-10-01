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
    // Aliases share their producer's definition; section anchors have none.
    unsigned definition = 0;
    bool lazy = false; // synthetic GOT slots are demanded by live relocations
    unsigned object_type = 0;
    std::uint64_t size = 0;
    lowir_model::Name section = 0;
    unsigned alignment = 1;
};
struct ObjectUnwind { unsigned symbol; std::int64_t addend; std::size_t offset; };
struct ObjectReference { unsigned symbol; std::int64_t addend; };
struct Object {
    native::Image image;
    cppgm::IdentifierTable names;
    std::vector<Symbol> symbols;
    unsigned alignment = 16;
    std::vector<ObjectUnwind> unwind;
    std::vector<ObjectReference> initializers, finalizers;
    std::size_t frame_begin = std::size_t(-1);
    explicit Object(unsigned count = 0) : image(count), symbols(image.symbols.size()) {}
    std::string name(unsigned id) const;
    lowir_model::Name intern(const std::string& text);
};
Object compile_object(lowir_model::Program&, native::Statistics&, bool host = false);
void write_host_object(Object&&, const std::string&);
void write_object(const Object&, const std::string&);
Object read_object(const std::string&);
Object read_elf(const std::vector<unsigned char>&);
class Linker {
    native::Image image_;
    cppgm::IdentifierTable names_;
    lowir_model::NameIndex externals_;
    std::vector<Symbol> symbols_;
    std::vector<unsigned> symbol_definitions_;
    std::vector<bool> lazy_definitions_ = std::vector<bool>(1);
    std::vector<ObjectReference> initializers_, finalizers_;
    std::vector<ObjectUnwind> unwind_;
    std::size_t frame_begin_ = std::size_t(-1);
    bool host_;
    std::vector<unsigned> runtime_demands_;
    unsigned entry_ = 0, parameters_ = 0, alignment_ = 16;
    unsigned new_symbol();
    void retain_relocations();
    void supply_runtime();
public:
    std::size_t definition_work = 0, relocation_work = 0;
    native::Statistics runtime_stats;
    explicit Linker(bool host = false);
    void add(Object&&);
    std::size_t finish(const std::string&);
};
int run(const std::vector<std::string>&);
int query(const std::string&);
} }
