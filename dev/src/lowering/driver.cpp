#include "lowering/procedural.h"
#include "lowir/emission_demand.h"
#include "preprocess/preprocessor.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sys/resource.h>
namespace cppgm { namespace lowering {
void build_program(lowir_model::Program& program, const std::vector<std::string>& inputs, bool stats,
    const std::vector<std::string>& includes, const std::vector<std::string>& macros, bool presentation, bool host, bool debug)
{
    typedef std::chrono::steady_clock Clock;
    const std::time_t now = std::time(0);
    const std::string stamp = std::asctime(std::localtime(&now));
    Linkage linkage(inputs.size() > 1,presentation); linkage.host = host; linkage.debug = debug;
    double frontend_ms = 0, lowering_ms = 0;
    std::size_t nodes = 0, static_requests = 0, static_hits = 0;
    std::size_t control_work = 0, discard_work = 0;
    std::size_t fallthrough_functions = 0, fallthrough_work = 0, fallthrough_edges = 0;
    std::size_t full_expression_work = 0, full_expression_regions = 0;
    for (const std::string& input : inputs) {
        auto start = Clock::now();
        Preprocessor pp(input, stamp.substr(4, 7) + stamp.substr(20, 4), stamp.substr(11, 8), stats, host);
        pp.include_paths(includes); pp.command_options(macros);
        PostTokenCursor post(pp, pp.identifiers(), false, 0, true, host);
        syntax::Ast ast(stats);
        syntax::Cursor cursor(post, pp.identifiers(), ast);
        syntax::Parser parser(cursor, ast, pp.identifiers());
        semantic::Analyzer sem(ast, pp.identifiers(), true, true, host);
        parser.translation_unit(&sem); sem.finish();
        auto parsed = Clock::now();
        Procedural lower(ast, sem, pp.identifiers(), program, linkage); lower.run();
        frontend_ms += std::chrono::duration<double, std::milli>(parsed-start).count();
        lowering_ms += std::chrono::duration<double, std::milli>(Clock::now()-parsed).count();
        nodes += ast.nodes.size();
        static_requests += sem.static_requests; static_hits += sem.static_hits;
        control_work += lower.control_work; discard_work += lower.discard_work;
        fallthrough_functions += lower.fallthrough_functions;
        fallthrough_work += lower.fallthrough_work; fallthrough_edges += lower.fallthrough_edges;
        full_expression_work += lower.full_expression_work; full_expression_regions += lower.full_expression_regions;
        if (stats) {
            std::cerr << "{\"tokens\":" << cursor.produced << ",\"max_pending\":" << cursor.max_pending
                << ",\"node_growths\":" << ast.node_growths << ",\"delimiter_work\":" << cursor.delimiter_work
                << ",\"asm_source_statements\":" << ast.assemblies.size()-1
                << ",\"asm_source_instructions\":" << ast.assembly_instructions.size();
            sem.telemetry(std::cerr);
            std::cerr << ",\"lower_virtual_cache_bytes\":" << lower.virtual_cache_bytes()
                << ",\"rtti_expressions\":" << sem.rtti_expressions.size()-1
                << ",\"rtti_records\":" << lower.rtti_work
                << ",\"rtti_cache_hits\":" << lower.rtti_hits
                << ",\"statement_regions\":" << lower.statement_regions
                << ",\"lifetime_mapping_work\":" << lower.lifetime_mapping_work
                << ",\"lifetime_mapping_hits\":" << lower.lifetime_mapping_hits
                << ",\"list_plans\":" << sem.list_plans.size()-1
                << ",\"list_objects\":" << sem.list_objects.size()-1
                << ",\"initializer_list_types\":" << sem.initializer_list_types.size()-1
                << ",\"lower_deleting_entries\":" << lower.deleting_entry_count()
                << ",\"lower_emission_rounds\":" << lower.emission_rounds;
            std::cerr << ",\"lower_local_statics\":" << lower.local_static_count()
                << ",\"lower_constant_data_records\":" << lower.constant_data_count()
                << ",\"lower_constant_data_work\":" << lower.constant_data_work
                << ",\"lower_constant_data_hits\":" << lower.constant_data_hits
                << ",\"lower_aggregate_array_work\":" << lower.aggregate_array_work
                << ",\"lower_aggregate_array_hits\":" << lower.aggregate_array_hits;
            std::cerr << "}\n";
        }
    }
    auto lifecycle_start = Clock::now();
    linkage.finish_lifecycle(program);
    if (!linkage.conditional_abi_roots.empty()) {
        // Preserve ABI entries used by the source before optimization, even
        // when all of their calls later inline. Checking an unreachable inline
        // definition alone does not establish that source demand. The completed
        // program includes cross-TU and initialization consumers at this point.
        auto live = lowir_model::emission_demand(program,linkage.conditional_abi_roots);
        for (auto entry : linkage.conditional_abi_roots)
            if (live[entry.target.index]) program.symbols[entry.target.index-1].metadata.object_root = true;
    }
    lowering_ms += std::chrono::duration<double, std::milli>(Clock::now()-lifecycle_start).count();
    if (stats) {
        struct rusage usage; getrusage(RUSAGE_SELF, &usage);
        std::cerr << "{\"frontend_ms\":" << frontend_ms << ",\"lowering_ms\":" << lowering_ms
            << ",\"peak_rss_kib\":" << usage.ru_maxrss << ",\"nodes\":" << nodes
            << ",\"static_requests\":" << static_requests << ",\"static_hits\":" << static_hits
            << ",\"control_work\":" << control_work << ",\"discard_work\":" << discard_work
            << ",\"fallthrough_functions\":" << fallthrough_functions
            << ",\"fallthrough_work\":" << fallthrough_work << ",\"fallthrough_edges\":" << fallthrough_edges
            << ",\"full_expression_work\":" << full_expression_work << ",\"full_expression_regions\":" << full_expression_regions
            << ",\"linkage_requests\":" << linkage.requests << ",\"linkage_hits\":" << linkage.hits
            << ",\"value_base_argument_facts\":" << linkage.value_base_arguments.size()
            << ",\"value_parameter_abi_facts\":" << linkage.parameter_abis.size()-1
            << ",\"initializer_units\":" << linkage.initializers.size() << ",\"finalizer_units\":" << linkage.finalizers.size()
            << ",\"abi_nodes\":" << linkage.abi.size() << ",\"abi_bytes\":" << linkage.abi.storage_bytes()
            << ",\"instructions\":" << program.instructions.size() << ",\"operands\":" << program.operands.size()
            << ",\"ir_pool_growths\":" << program.pool_allocations()
            << ",\"ir_capacity_bytes\":" << program.pool_storage_bytes() << "}\n";
    }
}
int emit_lowir(const std::string& output, const std::vector<std::string>& inputs, bool stats, bool audit)
{
    lowir_model::Program program;
    build_program(program,inputs,stats,{},{},true);
    if (audit) lowir_model::validate(program);
    std::ofstream out(output.c_str());
    if (!out) throw std::runtime_error("cannot create LowIR output");
    auto start = std::chrono::steady_clock::now();
    lowir_model::write_program(program,out); out.close();
    if (!out) throw std::runtime_error("cannot write LowIR output");
    if (stats) std::cerr << "{\"write_ms\":" << std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count() << "}\n";
    return 0;
}
} }
