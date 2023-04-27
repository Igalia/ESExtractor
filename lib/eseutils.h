/* ESExtractor
 * Copyright (C) 2022 Igalia, S.L.
 *     Author: Stephane Cerveau <scerveau@igalia.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You
 * may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#pragma once

#include <cstdint>
#include <vector>

#if (__cplusplus < 201402L)
#  include <memory>
template <class T, class... Args>
std::unique_ptr<T>
make_unique (Args &&...args)
{
  return std::unique_ptr<T> (new T (std::forward<Args> (args)...));
}
#else
#  define make_unique std::make_unique
#endif

#define ESE_CHECK(expr, val)         \
  if (expr) {                        \
  } else {                           \
    ERR ("Check '%s' fails", #expr); \
    return val;                      \
  }
#define ESE_CHECK_VOID(expr)         \
  if (expr) {                        \
  } else {                           \
    ERR ("Check '%s' fails", #expr); \
    return;                          \
  }

template <typename T>
static inline std::vector<T>
subVector (std::vector<T> const &v, int pos, int size)
{
  auto first = v.cbegin () + pos;
  auto last  = v.cbegin () + pos + size;

  std::vector<T> vec (first, last);
  return vec;
}