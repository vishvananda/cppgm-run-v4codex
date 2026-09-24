#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
EntityId Analyzer::deduced_specialization(EntityId primary, std::vector<ArgumentId>& arguments)
{
    // Defaults participate in deduction before a complete specialization key
    // exists. A nested probe of this same candidate observes its active head
    // and deduced tuple (including holes); it must not restart those defaults.
    // This is active state only, not a negative result cached across completion
    // or a later redeclaration that adds defaults.
    ++candidate_substitution_work;
    bool defaults = arguments.size() < templates[entities[primary].template_info].count;
    for (auto argument : arguments) defaults |= !argument;
    auto identity = defaults ? key(entities[primary].template_info,intern_arguments(arguments)) : 0;
    if (identity && active_candidate_substitutions.get(identity)) { ++candidate_substitution_cycles; return 0; }
    struct Active {
        Index& index; std::uint64_t identity; bool& probe; bool saved;
        bool& immediate; bool saved_immediate;
        Active(Index& i, std::uint64_t k, bool& p, bool& q) : index(i), identity(k), probe(p), saved(p), immediate(q), saved_immediate(q) {
            if (identity) index.put(identity,1);
            probe = true; immediate = true;
        }
        ~Active() { if (identity) index.put(identity,0); probe = saved; immediate = saved_immediate; }
    } active(active_candidate_substitutions,identity,template_type_probe,immediate_query_probe);
    if (defaults && !template_defaults(primary,arguments)) return 0;
    return specialize(primary,arguments);
}
} }
