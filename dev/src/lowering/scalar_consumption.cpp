#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::initialize_scalar(const semantic::ScalarConsumption& consumption, Value location)
{
    // The final conversion occurs in the selected branch before destruction.
    // Observable destinations receive the store there too; private storage can
    // consume the merged scalar after branch-local cleanup has completed.
    Value destination = consumption.private_destination ? Value() : location;
    Value value = conditional(consumption.expression,false,destination,0,true,&consumption);
    if (consumption.private_destination) store(value,location);
}
} }
