#pragma once
#include <cstdint>
namespace cppgm { namespace semantic {
// Stack-local evaluation state, separate from the immutable checked recipe.
struct SourceInvocation { std::uint32_t site = 0; bool defaulted = false; };
struct SourceInvocationScope {
    SourceInvocation& state;
    SourceInvocation prior;
    SourceInvocationScope(SourceInvocation& s, std::uint32_t site, bool defaulted = false, bool reset = false)
        : state(s), prior(s) {
        if (reset) { state.site = site; state.defaulted = defaulted; }
        else {
            if (!state.defaulted && site) state.site = site;
            state.defaulted |= defaulted && state.site;
        }
    }
    ~SourceInvocationScope() { state = prior; }
};
} }
