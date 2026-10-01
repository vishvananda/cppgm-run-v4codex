namespace origin { struct object { int value; }; namespace sub { typedef int value_type; } }
namespace left { typedef origin::object object; typedef const int number; namespace sub = origin::sub; }
namespace right { using object = origin::object; using number = const int; namespace sub = origin::sub; }
namespace joined { using namespace left; using namespace right; }
using namespace origin;
using namespace joined;
object o = {17};
number n = 5;
sub::value_type count = 2;
static_assert(sizeof(joined::object) == sizeof(origin::object),"qualified aliases");
namespace cycle_a { using namespace left; }
namespace cycle_b { using namespace cycle_a; using namespace right; }
namespace cycle_a { using namespace cycle_b; }
cycle_a::number q = 8;
namespace repeated { using namespace origin; using namespace left; object p = {3}; }
int main() { return o.value+n+count+q+repeated::p.value != 35; }
