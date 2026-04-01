#pragma once

#include <type_traits>

template <typename T>
using raw_type = typename std::remove_cv_t < std::remove_reference_t<T>>;