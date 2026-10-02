#pragma once
#include "lowir/model.h"
namespace lowir_model {
// Shared source/object/text boundary. Level zero has no optimizer work.
void optimize(Program&, unsigned level, bool telemetry = false);
}
