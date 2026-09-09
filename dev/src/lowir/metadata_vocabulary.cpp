#include "lowir/metadata.h"
namespace lowir_model {
static const char* const roles[] = {
    "", "entry", "init", "fini", "eh_allocate_exception", "eh_begin_catch", "eh_end_catch",
    "eh_rethrow", "eh_throw", "eh_personality", "eh_resume", "allocate_memory", "free_memory",
    "terminate", "pure_virtual", "dynamic_cast", "bad_cast", "bad_typeid", "rtti_class", "rtti_si", "rtti_vmi", "rtti_data"
};
const char* role_name(SymbolRole role) { return roles[role]; }
}
