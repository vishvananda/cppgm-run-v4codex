#pragma once

#include "preprocess/expression.h"

namespace cppgm {

bool promote_pp_literal(const PostToken& token, PPValue& value);
PPValue pp_unary(ETokenType op, PPValue value);
PPValue pp_binary(ETokenType op, PPValue left, PPValue right);
PPValue pp_conditional(PPValue condition, PPValue yes, PPValue no);

}
