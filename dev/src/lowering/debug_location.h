#pragma once
#include "lowir/model.h"
namespace cppgm { namespace lowering {
// A lowering invocation owns its source step; recursive expressions restore
// their caller's location without mutating the shared semantic/source graph.
struct DebugScope {
    lowir_model::DebugLocation& current;
    lowir_model::DebugLocation saved;
    DebugScope(lowir_model::DebugLocation& current, lowir_model::DebugLocation next)
        : current(current), saved(current) { if (next.file) current = next; }
    ~DebugScope() { current = saved; }
};
} }
