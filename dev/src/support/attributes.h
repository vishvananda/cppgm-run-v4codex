#pragma once
#include "preprocess/source.h"
namespace cppgm {
enum { UsingIfExists = 8 };
inline bool using_if_exists_attribute(TextView name)
{
    return name.equals("using_if_exists") || name.equals("__using_if_exists__");
}
}
