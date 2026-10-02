#include "lowir/optimizer.h"
#include "lowir/folding.h"
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
    forward_local_slots(p,work);
    simplify_scalars(p,work);
    eliminate_local_expressions(p,work);
    simplify_control(p,work);
    forward_local_slots(p,work);
    simplify_scalars(p,work);
    if (telemetry) {
        rusage usage; getrusage(RUSAGE_SELF,&usage);
        std::cerr << "{\"optimize_ms\":" << std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()
            << ",\"optimize_work\":" << work
            << ",\"optimize_instructions\":" << p.instructions.size()
            << ",\"optimize_peak_rss_kib\":" << usage.ru_maxrss << "}\n";
    }
}
}
