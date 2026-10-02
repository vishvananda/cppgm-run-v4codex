#include "lowir/optimizer.h"
#include "lowir/folding.h"
#include "lowir/ordinary_flow.h"
#include "lowir/inline_policy.h"
#include "lowir/call_effects.h"
#include <chrono>
#include <iostream>
#include <sys/resource.h>
namespace lowir_model {
void optimize(Program& p, unsigned level, bool telemetry)
{
    require(level <= 3,"invalid optimization level");
    if (!level) return;
    auto start = std::chrono::steady_clock::now();
    std::uint64_t work = 0;
    prune_support_functions(p,work);
    simplify_control(p,work);
    if (level >= 2) propagate_call_constants(p,work);
    simplify_scalars(p,work);
    simplify_call_regions(p,work);
    simplify_control(p,work);
    // Split original aggregate homes before call admission, so compact bodies
    // are costed once. A second bounded split handles homes introduced by
    // cloning; neither step restarts interprocedural expansion.
    bool objects_split = split_local_objects(p,work);
    if (objects_split) {
        forward_local_slots(p,work);
        simplify_scalars(p,work);
    }
    if (inline_small_calls(p,level,work)) {
        simplify_call_regions(p,work);
        simplify_control(p,work);
    }
    // Immutable admission summary for this pipeline. Its transforms add no
    // calls or cycles, so a declined function remains a safe conservative
    // choice even when later simplification removes a cycle.
    std::vector<bool> call_cycles(p.functions.size());
    for (unsigned fn = 0; fn < p.functions.size(); ++fn)
        if (!p.functions[fn].declaration) call_cycles[fn] = has_call_cycle(p,p.functions[fn],work);
    std::vector<bool> dataflow(p.functions.size());
    for (unsigned fn = 0; fn < p.functions.size(); ++fn)
        dataflow[fn] = p.functions[fn].blocks.count > 1 && !call_cycles[fn];
    forward_local_slots(p,work);
    simplify_scalars(p,work);
    if (split_local_objects(p,work)) {
        objects_split = true;
        forward_local_slots(p,work);
        simplify_scalars(p,work);
    }
    if (promote_scalar_slots(p,call_cycles,work)) simplify_scalars(p,work,&dataflow);
    eliminate_local_expressions(p,call_cycles,work);
    simplify_control(p,work);
    forward_local_slots(p,work);
    simplify_scalars(p,work);
    eliminate_local_expressions(p,call_cycles,work,true,&dataflow);
    simplify_scalars(p,work,&dataflow);
    simplify_diamond_values(p,work);
    bypass_empty_jumps(p,work);
    simplify_control(p,work);
    simplify_scalars(p,work,&dataflow);
    // Phi repair above can expose constant terminators. This last structural
    // sweep closes those edges; it does not restart the optimization pipeline.
    simplify_control(p,work);
    prune_support_functions(p,work);
    simplify_control(p,work);
    if (objects_split) retire_unused_slots(p,work);
    if (telemetry) {
        rusage usage; getrusage(RUSAGE_SELF,&usage);
        std::cerr << "{\"optimize_ms\":" << std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()
            << ",\"optimize_work\":" << work
            << ",\"optimize_instructions\":" << p.instructions.size()
            << ",\"split_objects\":" << p.stats.split_objects
            << ",\"split_fields\":" << p.stats.split_fields
            << ",\"split_growth_reserved\":" << p.stats.split_growth_reserved
            << ",\"optimize_peak_rss_kib\":" << usage.ru_maxrss << "}\n";
    }
}
}
