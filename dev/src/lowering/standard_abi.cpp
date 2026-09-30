#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
abi_mangle::Id Procedural::abi_template_type(abi_mangle::Id name,
    const std::vector<abi_mangle::Id>& arguments)
{
    using namespace abi_mangle;
    // Itanium 5.1.8's prescribed substitutions, matched by canonical ABI
    // components. Inline namespaces and ABI tags are distinct identities.
    auto parent = abi[name].a;
    if (abi[name].kind != Kind::Name || !parent || abi[parent].kind != Kind::Name || abi[parent].a)
        return abi.make(Kind::Template,name,0,0,0,arguments);
    auto scope = abi.text(abi[parent].b);
    if (scope.size != 3 || scope.data[0] != 's' || scope.data[1] != 't' || scope.data[2] != 'd')
        return abi.make(Kind::Template,name,0,0,0,arguments);
    auto standard = parent;
    if (name == abi.name(standard,"allocator"))
        name = abi.make(Kind::Standard,ABI_STANDARD_SUBSTITUTION_ALLOCATOR);
    else if (name == abi.name(standard,"basic_string"))
        name = abi.make(Kind::Standard,ABI_STANDARD_SUBSTITUTION_BASIC_STRING);
    if (arguments.size() == 2 || arguments.size() == 3) {
        auto character = abi.make(Kind::TypeArgument,abi.builtin(ABI_BUILTIN_TYPE_CHAR));
        if (arguments[0] == character) {
            auto traits = abi.make(Kind::Template,abi.name(standard,"char_traits"),0,0,0,{character});
            if (arguments[1] == abi.make(Kind::TypeArgument,traits)) {
                if (arguments.size() == 3 && abi[name].kind == Kind::Standard &&
                    abi[name].a == ABI_STANDARD_SUBSTITUTION_BASIC_STRING) {
                    auto allocator = abi.make(Kind::Template,
                        abi.make(Kind::Standard,ABI_STANDARD_SUBSTITUTION_ALLOCATOR),0,0,0,{character});
                    if (arguments[2] == abi.make(Kind::TypeArgument,allocator))
                        return abi.make(Kind::Standard,ABI_STANDARD_SUBSTITUTION_STRING);
                } else if (arguments.size() == 2) {
                    if (name == abi.name(standard,"basic_istream"))
                        return abi.make(Kind::Standard,ABI_STANDARD_SUBSTITUTION_ISTREAM);
                    if (name == abi.name(standard,"basic_ostream"))
                        return abi.make(Kind::Standard,ABI_STANDARD_SUBSTITUTION_OSTREAM);
                    if (name == abi.name(standard,"basic_iostream"))
                        return abi.make(Kind::Standard,ABI_STANDARD_SUBSTITUTION_IOSTREAM);
                }
            }
        }
    }
    return abi.make(Kind::Template,name,0,0,0,arguments);
}
} }
