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

#include "compiler.hpp"

#include <fstream>

#include "error.hpp"
#include "input_file.hpp"
#include "lexer.hpp"

co::color_ostream<std::ostream> Compiler::cl_cout{std::cout};
co::color_ostream<std::ostream> Compiler::cl_cerr{std::cerr};

int Compiler::echoFile() {
  std::cout << inputFile.getStream()->rdbuf();
  return EXIT_SUCCESS;
}

int Compiler::lexTest() {
  Lexer lexer{inputFile};
  try {
    while (true) {
      Token t = lexer.nextToken();
      std::cout << t << std::endl;
      // if EOF print eof-Token, then break
      if (t.type == Token::Type::eof)
        break;
    }
  } catch (LexError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void Compiler::checkOptions() {
  if (!options.echoFile != options.testLexer)
    throw ArgumentError(
        "Cannot have Options --echo and --lextext simultaneously");
}

int Compiler::run() {
  if (options.echoFile) {
    echoFile();
  } else if (options.testLexer) {
    lexTest();
  }
}
