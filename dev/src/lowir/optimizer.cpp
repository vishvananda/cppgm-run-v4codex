#include "lowir/optimizer.h"
#include "lowir/folding.h"
#include "lowir/ordinary_flow.h"
#include "lowir/inline_policy.h"
#include "lowir/call_effects.h"
#include "lowir/loop_simplify.h"
#include "lowir/memory_values.h"
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
    if (split_local_objects(p,work)) {
        forward_local_slots(p,work);
        simplify_scalars(p,work);
    }
    if (inline_small_calls(p,level,work)) {
        // Call-site proofs admit exception-bearing bodies only when their
        // ordinary paths cannot unwind. Materialize those same constant facts
        // and retire dead ordinary paths before removing the registrations.
        simplify_scalars(p,work);
        simplify_control(p,work);
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
        forward_local_slots(p,work);
        simplify_scalars(p,work);
    }
    if (promote_scalar_slots(p,call_cycles,work)) simplify_scalars(p,work,&dataflow);
    MemoryStats memory;
    auto memory_start = work;
    bool memory_changed = simplify_memory_values(p,call_cycles,work,memory);
    auto memory_work = work-memory_start;
    if (memory_changed) {
        simplify_scalars(p,work);
        forward_local_slots(p,work);
        simplify_scalars(p,work);
    }
    eliminate_local_expressions(p,call_cycles,work);
    simplify_control(p,work);
    forward_local_slots(p,work);
    simplify_scalars(p,work);
    eliminate_local_expressions(p,call_cycles,work,true,&dataflow);
    simplify_scalars(p,work,&dataflow);
    LoopStats loops;
    if (simplify_loops(p,level,work,loops)) {
        simplify_control(p,work);
        simplify_scalars(p,work);
    }
    simplify_diamond_values(p,work);
    bypass_empty_jumps(p,work);
    merge_forward_blocks(p,work);
    simplify_control(p,work);
    simplify_scalars(p,work,&dataflow);
    // Phi repair above can expose constant terminators. This last structural
    // sweep closes those edges; it does not restart the optimization pipeline.
    simplify_control(p,work);
    prune_support_functions(p,work);
    simplify_control(p,work);
    if (retire_private_writes(p,work)) simplify_scalars(p,work);
    retire_unused_slots(p,work);
    if (telemetry) {
        rusage usage; getrusage(RUSAGE_SELF,&usage);
        std::cerr << "{\"optimize_ms\":" << std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()
            << ",\"optimize_work\":" << work
            << ",\"optimize_instructions\":" << p.instructions.size()
            << ",\"inline_context_work\":" << p.stats.inline_context_work
            << ",\"inline_context_sites\":" << p.stats.inline_context_sites
            << ",\"split_objects\":" << p.stats.split_objects
            << ",\"split_fields\":" << p.stats.split_fields
            << ",\"split_growth_reserved\":" << p.stats.split_growth_reserved
            << ",\"loop_candidates\":" << loops.candidates
            << ",\"loops_removed\":" << loops.removed
            << ",\"loops_filled\":" << loops.fills
            << ",\"loops_unrolled\":" << loops.unrolled
            << ",\"loop_clone_reserved\":" << loops.cloned
            << ",\"loops_declined\":" << loops.declined
            << ",\"memory_reused\":" << memory.reused
            << ",\"memory_work\":" << memory_work
            << ",\"memory_conditional\":" << memory.conditional
            << ",\"memory_copies\":" << memory.copies
            << ",\"memory_diamonds\":" << memory.diamonds
            << ",\"memory_declined\":" << memory.declined
            << ",\"memory_snapshots\":" << memory.snapshots
            << ",\"memory_peak_snapshots\":" << memory.peak_snapshots
            << ",\"optimize_peak_rss_kib\":" << usage.ru_maxrss << "}\n";
    }
}
}
