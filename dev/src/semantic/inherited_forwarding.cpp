#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::inherited_forwarding(EntityId e)
{
    auto m = entities[e].member_info;
    if (members[m].inherited_state == FactState::Success) return;
    if (members[m].inherited_state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::ConstructorActions,e,0);
    if (members[m].inherited_state == FactState::Active)
        throw std::runtime_error("recursive inherited forwarding");
    members[m].inherited_state = FactState::Active;
    ++unevaluated_depth;
    try {
        auto target = members[m].inherited_constructor;
        auto f = types[entities[e].type], base = types[entities[target].type];
        std::vector<InheritedArgument> arguments;
        for (unsigned j = 0; j < base.count; ++j) {
            InheritedArgument argument;
            Conversion c;
            if (j >= f.count) {
                argument.value = default_argument(target,j,&c,DefaultReason::Recipe);
            } else {
                auto type = types.parameters[f.offset+j];
                Expression source; source.type = value_type(type);
                source.category = types[type].kind == TypeKind::LRef ? ValueCategory::Lvalue : ValueCategory::Xvalue;
                auto to = types.parameters[base.offset+j];
                c = class_value(to) ? transfer_initialization(source,to,InitializationMode::Copy) : standard_conversion(source,to);
                if (!c.valid()) throw std::runtime_error("invalid inherited parameter forwarding");
                if (c.function) {
                    if (deleted_transfer(c.function)) throw std::runtime_error("deleted inherited parameter transfer");
                    check_access(c.function,entities[e].owner,entities[c.function].owner);
                    argument.transfer = c.function;
                    auto transfer = types[entities[c.function].type];
                    std::vector<Conversion> defaults;
                    for (unsigned k = 1; k < transfer.count; ++k) {
                        Conversion d; default_argument(c.function,k,&d,DefaultReason::Recipe);
                        defaults.push_back(d);
                    }
                    argument.transfer_defaults = conversions.size();
                    conversions.insert(conversions.end(),defaults.begin(),defaults.end());
                }
            }
            argument.conversion = conversions.size(); conversions.push_back(c);
            arguments.push_back(argument);
        }
        members[m].inherited_arguments = inherited_arguments.size();
        inherited_arguments.insert(inherited_arguments.end(),arguments.begin(),arguments.end());
        members[m].inherited_state = FactState::Success;
        --unevaluated_depth;
    } catch (const UnavailableSemanticFact&) {
        --unevaluated_depth;
        members[m].inherited_state = FactState::NotStarted; throw;
    } catch (...) {
        --unevaluated_depth;
        members[m].inherited_state = FactState::Failure; throw;
    }
}
} }
