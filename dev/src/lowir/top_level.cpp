#include "lowir/reader.h"
namespace lowir_model {
DataItem Reader::data_item(Type t, bool structured)
{
    DataItem d;
    d.type = t;
    if (accept("zero")) { d.kind = DataItem::Zero; if (structured) d.zero_bytes = natural(); }
    else {
        if (structured) d.type = type();
        if (accept("addr")) {
            d.kind = DataItem::Address;
            d.symbol = p_.symbol(name('@'));
            if (accept("+")) d.addend = natural();
            else if (accept("-")) d.addend = std::uint64_t(0) - natural();
        } else { d.kind = DataItem::Scalar; d.value = literal(); }
    }
    return d;
}
void Reader::global(bool declaration)
{
    Global g;
    g.declaration = declaration;
    g.symbol = p_.symbol(name('@'));
    require(p_.symbols[g.symbol.index-1].kind == Symbol::Unknown, "duplicate top-level symbol");
    GlobalStorageMode legacy = GSM_DEFAULT;
    if (accept("readonly")) legacy = GSM_READONLY;
    else if (accept("thread_local")) legacy = GSM_THREAD_LOCAL;
    bool typed = accept(":");
    if (typed) g.type = type();
    SymbolMetadata m = metadata(false);
    require(legacy == GSM_DEFAULT || m.storage == GSM_DEFAULT || legacy == m.storage, "conflicting storage metadata");
    if (legacy != GSM_DEFAULT) m.storage = legacy;
    if (!declaration) {
        expect("=");
        g.data.begin = p_.data.size();
        if (accept("{")) {
            require(!typed, "typed structured global");
            g.structured = true;
            while (!at("}")) { p_.data.push_back(data_item(Type(), true)); ++g.data.count; }
            expect("}");
            require(g.data.count != 0, "empty structured global");
        } else {
            require(typed && g.type.scalar(), "invalid scalar global");
            p_.data.push_back(data_item(g.type, false));
            g.data.count = 1;
        }
    } else require(!typed || g.type.scalar(), "invalid global declaration type");
    p_.globals.push_back(g);
    Symbol& symbol = p_.symbols[g.symbol.index-1];
    symbol.kind = Symbol::GlobalSymbol;
    symbol.entity = p_.globals.size();
    symbol.metadata = m;
}
void Reader::function(bool declaration)
{
    Function f;
    f.declaration = declaration;
    f.symbol = p_.symbol(name('@'));
    require(p_.symbols[f.symbol.index-1].kind == Symbol::Unknown, "duplicate top-level symbol");
    p_.functions.push_back(f);
    FunctionId id(p_.functions.size());
    FunctionBuilder builder(p_, id);
    Signature sig = signature(&builder);
    SymbolMetadata m = metadata(true, &sig.boundary);
    if (m.role == SR_NONE) {
        std::string n = p_.name(p_.symbols[f.symbol.index-1].name);
        if (n == "@main") m.role = SR_ENTRY;
        if (n == "@__cppgm_init") m.role = SR_INIT;
        if (n == "@__cppgm_fini") m.role = SR_FINI;
    }
    p_.signatures.push_back(sig);
    p_.functions[id.index-1].signature = SignatureId(p_.signatures.size());
    Symbol& symbol = p_.symbols[f.symbol.index-1];
    symbol.kind = Symbol::FunctionSymbol;
    symbol.entity = id.index;
    symbol.metadata = m;
    if (declaration) return;
    p_.functions[id.index-1].debug = debug();
    expect("{");
    while (!at("}")) {
        if (accept("slot")) {
            Name n = name('$');
            expect(":");
            builder.add_slot(n, type());
        } else if (accept("block")) {
            Name n = name('^');
            expect(":");
            builder.start_block(n);
        } else instruction(builder);
    }
    expect("}");
}
} // namespace lowir_model
