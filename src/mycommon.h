#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <numeric>
#include <algorithm>
#include <cassert>

#define STRINGIFY_UTIL_(s) #s
#define STRINGIFY(s) STRINGIFY_UTIL_(s)

namespace fs = std::filesystem;

template <typename T>
using Array = std::vector<T>;
template <typename K, typename T>
using Dict = std::unordered_map<K, T>;
using String = std::string;
using WString = std::wstring;
using SringArray = Array<String>;
using BytesArray = Array<uint8_t>;

static const uint32_t   kInvalidValue32 = ~0u;
static const size_t     kInvalidValue = ~0;
static const String     kEmptyString;

#define rcast reinterpret_cast
#define scast static_cast

template <typename T>
inline void Swap(T& a, T& b) {
    T temp = b;
    b = a;
    a = temp;
}

template <typename T>
inline T Minimum(const T& a, const T& b) {
    return a < b ? a : b;
}

template <typename T>
inline T Maximum(const T& a, const T& b) {
    return a > b ? a : b;
}

template <typename T>
inline T Clamp(const T& value, const T& left, const T& right) {
    return value < left ? left : (value > right ? right : value);
}
