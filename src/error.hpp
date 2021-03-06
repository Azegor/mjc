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

#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>

#include "color_ostream.hpp"

class CompilerError : public std::exception {
public:
  virtual void writeErrorMessage(std::ostream &out) const = 0;
};

class ArgumentError : public CompilerError {
  std::string errorMessage;

public:
  ArgumentError(std::string errorMsg) : errorMessage(std::move(errorMsg)) {}
  const char *what() const noexcept override { return errorMessage.c_str(); }
  virtual void writeErrorMessage(std::ostream &out) const override {
    co::color_ostream<std::ostream> cl_out(out);
    cl_out << co::color(co::red) << co::mode(co::bold)
           << "error: " << co::color(co::regular) << errorMessage << std::endl;
  }
};

#endif // ERROR_H
