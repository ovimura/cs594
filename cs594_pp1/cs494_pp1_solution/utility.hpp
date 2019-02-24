#pragma once
#include <type_traits>
#include <utility>
#include <fstream>
#include <string.h>

using namespace std;

template <class T>
constexpr auto enum_cast(const T a)
{
  return static_cast<typename std::underlying_type<T>::type>(a);
}

template <class T, class U = T>
U *trivial_copy(U *dest, const T *src, std::size_t count)
{
  static_assert(is_trivially_copyable<T>::value);
  return reinterpret_cast<U *>(memcpy(dest, src, count * sizeof(T)));
}

template <class T>
T *trivial_zero(T *dest, std::size_t count)
{
  static_assert(is_trivially_copyable<T>::value);
  return reinterpret_cast<T *>(memset(dest, 0, count * sizeof(T)));
}

template <class T>
size_t getFileSize(T &stream)
{
  streampos pos = stream.tellg();
  stream.seekg(0, stream.end);
  size_t length = stream.tellg();
  stream.seekg(pos);

  return length;
}
