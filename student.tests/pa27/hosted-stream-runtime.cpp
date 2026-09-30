#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

std::string parse_with_stream(const std::string& source)
{
    std::istringstream input(source);
    std::string value;
    input >> value;
    return value;
}
int main(int argc, char**)
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
    const std::string value((std::istreambuf_iterator<char>(std::cin)),
        std::istreambuf_iterator<char>());
    long matches = 0;
    for (int i = 0; i < argc*200000; ++i)
        matches += parse_with_stream(value) == "extern-template-vtable";
    return matches == argc*200000 ? 0 : 1;
}
