// PA32 text adapter. Production paths share the typed optimizer directly.
#include "lowir/optimizer.h"
#include "support/tool_help_text.h"
#include <fstream>
#include <iostream>
int main(int argc, char** argv)
{
    try {
        for (int n = 1; n < argc; ++n) if (std::string(argv[n]) == "--help" || std::string(argv[n]) == "-h") {
            std::cout << lowiropt_help_text(); return 0;
        }
        int level = -1; bool stats = false;
        std::string output; std::vector<std::string> inputs;
        for (int n = 1; n < argc; ++n) {
            std::string a = argv[n];
            if (a.size() == 3 && a[0] == '-' && a[1] == 'O' && a[2] >= '0' && a[2] <= '3') {
                lowir_model::require(level == -1,"multiple optimization levels"); level = a[2]-'0';
            } else if (a == "-o") {
                lowir_model::require(output.empty() && ++n < argc,"expected one output path"); output = argv[n];
            } else if (a == "--stats") stats = true;
            else if (!a.empty() && a[0] == '-') throw lowir_model::ParseError("unknown option: "+a);
            else inputs.push_back(a);
        }
        lowir_model::require(level >= 0 && !output.empty() && !inputs.empty(),"expected level, -o and input files");
        auto program = lowir_model::parse_lowir_program_files(inputs);
        lowir_model::optimize(program,level,stats);
        std::ofstream out(output,std::ios::binary);
        lowir_model::require(bool(out),"cannot open output");
        lowir_model::write_program(program,out); out.close();
        lowir_model::require(bool(out),"cannot write output");
        return 0;
    } catch (const std::exception& e) { std::cerr << "ERROR: " << e.what() << '\n'; return 1; }
}
