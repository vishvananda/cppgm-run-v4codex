#include "semantic/template_binding.h"
#include <iostream>
int main() {
    using namespace cppgm::semantic;
    std::cout << sizeof(Entity) << ' ' << sizeof(Expression) << ' '
              << sizeof(ObjectUse) << ' ' << sizeof(TemplateSubstitutionFrame)
              << ' ' << sizeof(TemplateObjectContext) << ' '
              << sizeof(TemplateMemberUse) << '\n';
}
