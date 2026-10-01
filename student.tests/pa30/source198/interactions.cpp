namespace grammar {
#include "../source195/parser-boundaries.cpp"
}
namespace construction {
#include "../source196/dependent-construction.cpp"
}
namespace qualified {
#include "../source196/dependent-qualified-member.cpp"
}
namespace ordering {
#include "../source197/partial-alias-order.cpp"
}
namespace identity {
#include "base-alias.cpp"
}
// The looked-up base typedef is itself a demanded template argument. Its
// result crosses nested closing angles, substitution and dependent construction.
construction::pointer<identity::both::number> value;
static_assert(ordering::fold<ordering::yes,
              ordering::pack<decltype(value)>>::value == 1, "combined identity");
int main() {
    value.value = 3;
    value.reset();
    return grammar::main() || construction::main() || qualified::main() ||
           ordering::main() || identity::main() || value.value != 7;
}
