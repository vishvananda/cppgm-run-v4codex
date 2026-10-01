#pragma once
#include "preprocess/source.h"
#include <cstdint>
#include <string>

namespace cppgm {
// A numeric carrier, never a substitute for the destination format. Ordinary
// operations still round/evaluate in their recorded binary32/64/80 precision.
using ExtendedFloat = __float128;
bool finite_float(ExtendedFloat value);
bool nan_float(ExtendedFloat value);
ExtendedFloat integer_float(unsigned __int128 bits, bool negative, unsigned precision);
ExtendedFloat trunc_float(ExtendedFloat value);
bool sign_float(ExtendedFloat value);
ExtendedFloat parse_extended_float(TextView text, unsigned precision = 113);
ExtendedFloat half_value(std::uint16_t bits);
std::uint16_t half_bits(ExtendedFloat value);
std::string extended_float_text(ExtendedFloat value);
}
