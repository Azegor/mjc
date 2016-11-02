/*
 * MIT License
 *
 * Copyright (c) 2016 morrisfeist
 * Copyright (c) 2016 tpriesner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef UTIL_H
#define UTIL_H

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#include <sstream>

template <typename T> struct Identity {
  T operator()(T &&t) { return t; }
};

template <typename L, typename Callback = Identity<decltype(*L().begin())>>
std::string
listToString(L &list, Callback callback = Identity<decltype(*L().begin())>()) {
  std::stringstream res;
  bool first = true;
  for (auto &&e : list) {
    if (first)
      first = false;
    else
      res << ", ";
    res << callback(std::forward<decltype(e)>(e));
  }
  return res.str();
}

constexpr const char *UTF8Ellipsis = "\xE2\x80\xA6";

inline std::string truncateString(std::string str, unsigned maxLen) {
  if (str.length() > maxLen) {
    str.resize(maxLen - 1);
    str += UTF8Ellipsis;
    return str;
  }
  return str;
}

inline std::string truncatedErrorLine(const std::string &line,
                                      unsigned lineOffset, unsigned maxLen) {
  std::string res;
  if (lineOffset > 0) {
    res += UTF8Ellipsis;
    maxLen -= 1;
  }
  if (maxLen + lineOffset < line.length()) {
    res += line.substr(lineOffset, maxLen - 1);
    res += UTF8Ellipsis;
  } else {
    res += line.substr(lineOffset, maxLen);
  }
  return res;
}

namespace Consts {
constexpr size_t maxErrorIdentLength = 64;
constexpr size_t maxErrorLineLength = 200;
}

#endif // UTIL_H
