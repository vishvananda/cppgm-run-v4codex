namespace left { namespace name { int value; } }
namespace right { namespace name { int value; } }
using namespace left; using namespace right;
int value = name::value;
