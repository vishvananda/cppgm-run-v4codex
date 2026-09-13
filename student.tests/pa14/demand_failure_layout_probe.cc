#include "semantic/analyzer.h"
#include <iostream>
int main() {
    using namespace cppgm::semantic;
    std::cout << sizeof(Entity) << ' ' << sizeof(Expression) << ' '
              << sizeof(ObjectUse) << ' ' << sizeof(TemplateSubstitutionFrame)
              << ' ' << sizeof(cppgm::syntax::NodePool::Occurrence) << ' '
              << sizeof(cppgm::syntax::Ast) << ' ' << sizeof(TypeQuery) << ' '
              << sizeof(TypeQueryFact) << ' ' << sizeof(MemberFacts) << ' '
              << sizeof(TemplateDefinition) << ' ' << sizeof(Fact) << ' ' << sizeof(FactStore) << ' '
              << sizeof(Analyzer) << ' ' << sizeof(ClassFacts) << ' '
              << sizeof(Specialization) << ' ' << sizeof(FailedSemanticFact) << '\n';
}
