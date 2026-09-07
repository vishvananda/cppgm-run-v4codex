#pragma once
#include "abi/itanium/abi_mangle.h"

namespace abi_mangle {
class FactWriter {
public:
    explicit FactWriter(const Graph& graph);
    std::string write(const Target& target);
private:
    const Graph& g;
    std::string definitions;
    std::vector<unsigned> seen;
    std::vector<Id> touched;
    void definition(Id id, unsigned bit, const std::string& command,
                    const std::string& name, const std::string& form);
    unsigned depth = 0;
    std::string ref(char family, Id id);
    std::string type(Id id);
    std::string argument(Id id);
    std::string expression(Id id);
    std::string entity(Id id);
    std::string context(Id id);
    std::string function(const Function& function);
    std::string list(const Node& node, char family);
    std::string qualifier_words(unsigned bits);
};
} // namespace abi_mangle
