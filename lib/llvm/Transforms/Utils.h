#pragma once

#include <string_view>

#include "microcl/plugin/Attributes.h"

template <typename T>
static bool isAnnot(std::string_view Annotation) {
   return Annotation.rfind(microcl::plugin::GetAttrName<T>(), 0) == 0;
}
