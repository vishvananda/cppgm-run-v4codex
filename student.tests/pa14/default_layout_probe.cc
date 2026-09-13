#include "semantic/analyzer.h"
#include <iostream>
int main() {
    using namespace cppgm::semantic;
#ifdef ONLY_LIST
    std::cout << sizeof(ListPlan) << '\n';
#else
    std::cout << sizeof(DefaultArgumentFact) << ' ' << sizeof(DefaultDependency)
              << ' ' << sizeof(ListPlan) << '\n';
#endif
}
