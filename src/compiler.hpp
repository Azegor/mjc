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

#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <string>

#include "color_ostream.hpp"

struct CompilerOptions {
  std::string inputFile;
  bool help = false;
  bool echoFile = false;
  bool testLexer = false;
  bool testParser = false;
  // ...
};

class Compiler {
  static co::color_ostream<std::ostream> cl_cout;
  static co::color_ostream<std::ostream> cl_cerr;

  CompilerOptions options;

  int echoFile(const std::string &fileName);
  int lexTest(const std::string &inputFileName);

  void checkOptions();

public:
  Compiler(const CompilerOptions &opt) : options(opt) { checkOptions(); }
  int run();
};

#endif // COMPILER_H
