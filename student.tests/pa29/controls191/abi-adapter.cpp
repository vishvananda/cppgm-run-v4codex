#include "abi/itanium/abi_mangle.h"
#include <iostream>
int main() {
    const char* text =
        "let-arg I type int\n"
        "let-expr P function-param 0\n"
        "let-arg X expression P\n"
        "let-expr V vendor-expression __builtin_bit_cast I X\n"
        "let-arg A expression V\n"
        "type template Holder A\n";
    abi_mangle::AbiFactFile first = abi_mangle::parse_fact_text(text);
    std::string encoded = abi_mangle::mangle_fact_file(first);
    std::string written = abi_mangle::serialize_fact_file(first);
    abi_mangle::AbiFactFile second = abi_mangle::parse_fact_text(written);
    if (encoded != abi_mangle::mangle_fact_file(second)) return 1;
    // Itanium vendor expression: u <source-name> <template-arg>* E.
    if (encoded.find("6HolderIXu18__builtin_bit_castiXfp_EEEE") == std::string::npos) return 2;
    if (written != abi_mangle::serialize_fact_file(second)) {
        // IDs may change, but re-reading the second serialization must be stable.
        abi_mangle::AbiFactFile third = abi_mangle::parse_fact_text(abi_mangle::serialize_fact_file(second));
        if (encoded != abi_mangle::mangle_fact_file(third)) return 3;
    }
    std::cout << encoded;
}
