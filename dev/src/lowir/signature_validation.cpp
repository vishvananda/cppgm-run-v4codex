#include "lowir/metadata.h"
namespace lowir_model {
void validate_signature(const Program& p, const Signature& s, bool indirect)
{
    require(s.parameters.end() <= p.parameters.size(), "invalid signature range");
    for (unsigned i = 0; i < s.parameters.count; ++i) {
        const Parameter& a = p.parameters[s.parameters.begin + i];
        require(a.type != Type(), "void parameter");
        if (a.passing != PPM_DIRECT || a.alias != PALM_DEFAULT || a.object_bytes)
            require(a.type == Type::Ptr, "pointer metadata on non-pointer");
        if (a.passing == PPM_INDIRECT_RESULT) require(i == 0 && s.result == Type(), "invalid indirect result boundary");
    }
    if (s.boundary.query != CQM_DEFAULT) {
        require(!indirect && s.boundary.arity == CAM_FIXED && s.parameters.count && s.result.scalar(), "invalid query boundary");
        require(p.parameters[s.parameters.end()-1].type.integer(), "query index must be integer");
    }
}
}
