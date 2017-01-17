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

#include <fcntl.h>

#include "dotvisitor.hpp"
#include "error.hpp"
#include "firm_visitor.hpp"
#include "input_file.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semantic_visitor.hpp"
#include "optimizer.hpp"
#include "asm_pass.hpp"

#ifndef LIBSEARCHDIR
#define LIBSEARCHDIR "."
#endif

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
      std::cout << t << '\n';
      // if EOF print eof-Token, then break
      if (t.type == Token::Type::Eof)
        break;
    }
    std::cout << std::flush;
    return EXIT_SUCCESS;
  } catch (LexError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

int Compiler::lexFuzz() {
  Lexer lexer{inputFile};
  try {
    while (lexer.nextToken().type != Token::Type::Eof) {
      // do nothing
    }
    return EXIT_SUCCESS;
  } catch (LexError &e) {
    // don't print error message
    return EXIT_FAILURE;
  }
}

int Compiler::parserTest() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    parser.parseFileOnly();
    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

int Compiler::parserFuzz() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    parser.parseFileOnly();
    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    // don't print error message
    return EXIT_FAILURE;
  }
}

int Compiler::astPrint() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    parser.parseAndPrintAst();
    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

int Compiler::astDot() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    parser.parseAndDotAst();
    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

int Compiler::checkSemantic() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    auto ast = parser.parseProgram();
    analyzeAstSemantic(ast.get(), parser.getLexer());
    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

int Compiler::fuzzSemantic() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    auto ast = parser.parseProgram();
    analyzeAstSemantic(ast.get(), parser.getLexer());
    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    // don't print error message
    return EXIT_FAILURE;
  }
}

int Compiler::attrAstDot() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    auto ast = parser.parseProgram();
    analyzeAstSemantic(ast.get(), parser.getLexer());
    DotVisitor dotVisitor{std::cout};
    ast->accept(&dotVisitor);
    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

int Compiler::compileWithFirmBackend() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    auto ast = parser.parseProgram();
    analyzeAstSemantic(ast.get(), parser.getLexer());
    FirmVisitor firmVisitor{options.printFirmGraph};
    ast->accept(&firmVisitor);
    if (options.optimize) {
      Optimizer opt(firmVisitor.getFirmGraphs(), options.printFirmGraph, !options.noVerify);
      if (!opt.run()) {
        return EXIT_FAILURE;
      }
    }
    std::string outputName = options.outputFileName.empty() ? "a.out" : options.outputFileName;
    if (!lowerFirmGraphsWithFirmBackend(firmVisitor.getFirmGraphs(), options.printFirmGraph, !options.noVerify, options.compileFirm, options.outputAssembly, outputName))
      return EXIT_FAILURE;

    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

bool Compiler::lowerFirmGraphsWithFirmBackend(std::vector<ir_graph*> &graphs, bool printGraphs, bool verifyGraphs, bool generateCode, bool outputAssembly, const std::string &outFileName) {
  int graphErrors = 0;
  for (auto g : graphs) {
    lower_highlevel_graph(g);

    if (printGraphs) {
      dump_ir_graph(g, "lowered");
    }
    if (verifyGraphs) {
      if (irg_verify(g) == 0)
        graphErrors++;
    }
  }
  if (graphErrors)
    return false;

  if (generateCode) {
    FILE *f = nullptr;
    std::string assemblyName;
    if (outputAssembly) {
      assemblyName = outFileName + ".s";
      f = fopen(assemblyName.c_str(), "w");
    } else {
      f = tmpfile();
      assemblyName = "/proc/self/fd/" + std::to_string(fileno(f));
    }
    // XXX This only "works" on 64bit cpus
    be_parse_arg("isa=amd64");
    be_main(f, "test.java");
    fflush(f);
    int res = 0;
//     res |= system("gcc -c ../src/runtime.c -o runtime.o");
//     res |= system("ar rcs libruntime.a runtime.o");
    res |= system(("gcc -static -x assembler " + assemblyName + " -o " + outFileName + " -L" LIBSEARCHDIR " -lruntime").c_str());
    fclose(f);
    if (res) {
      throw std::runtime_error("Error while linking binary");
    }
  }
  return true;
}

int Compiler::compileWithOwnBackend() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    auto ast = parser.parseProgram();
    analyzeAstSemantic(ast.get(), parser.getLexer());
    FirmVisitor firmVisitor{options.printFirmGraph};
    ast->accept(&firmVisitor);
    if (options.optimize) {
      Optimizer opt(firmVisitor.getFirmGraphs(), options.printFirmGraph, !options.noVerify);
      if (!opt.run()) {
        return EXIT_FAILURE;
      }
    }
    std::string outputName = options.outputFileName.empty() ? "a.out" : options.outputFileName;
    // TODO
    if (!lowerFirmGraphsWithOwnBackend(firmVisitor.getFirmGraphs(), options.printFirmGraph, !options.noVerify, options.outputAssembly, outputName))
      return EXIT_FAILURE;

    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

bool Compiler::lowerFirmGraphsWithOwnBackend(std::vector<ir_graph*> &graphs, bool printGraphs, bool verifyGraphs, bool outputAssembly, const std::string &outFileName) {
  int graphErrors = 0;
  for (auto g : graphs) {
    lower_highlevel_graph(g);

    if (printGraphs) {
      dump_ir_graph(g, "lowered");
    }
    if (verifyGraphs) {
      if (irg_verify(g) == 0)
        graphErrors++;
    }
  }
  if (graphErrors)
    return false;

  FILE *f = nullptr;
  std::string assemblyName;
  if (outputAssembly) {
    assemblyName = outFileName + ".s";
    f = fopen(assemblyName.c_str(), "w");
  } else {
    f = tmpfile();
    assemblyName = "/proc/self/fd/" + std::to_string(fileno(f));
  }

  // create assembly code

  AsmPass asmPass(graphs, assemblyName);
  asmPass.run();


  int res = 0;
//     res |= system("gcc -c ../src/runtime.c -o runtime.o");
//     res |= system("ar rcs libruntime.a runtime.o");
  res |= system(("gcc -static -x assembler " + assemblyName + " -o " + outFileName + " -L" LIBSEARCHDIR " -lruntime").c_str());
  fclose(f);
  if (res) {
    throw std::runtime_error("Error while linking binary");
  }
  return true;
}

void Compiler::analyzeAstSemantic(ast::Program *astRoot, Lexer &lexer) {
  SemanticVisitor semantic_visitor(lexer);
  astRoot->accept(&semantic_visitor);
}

static int exclusiveOptionsSum(bool b) { return b ? 1 : 0; }

template <typename... Args>
static int exclusiveOptionsSum(bool b, Args... args) {
  return exclusiveOptionsSum(args...) + (b ? 1 : 0);
}

void Compiler::checkOptions() {
  if (exclusiveOptionsSum(options.echoFile, options.testLexer,
                          options.fuzzLexer, options.testParser,
                          options.fuzzParser, options.printAst, options.dotAst,
                          options.checkSemantic, options.fuzzSemantic,
                          options.dotAttrAst) > 1)
    throw ArgumentError(
        "Cannot have Options --echo, --lextext, --lexfuzz, "
        "--parsertest, --parserfuzz, --print-ast, --dot-ast, "
        "--check, --fuzz-check or --dot-attr-ast simultaneously");
}

int Compiler::run() {
  if (options.echoFile) {
    return echoFile();
  } else if (options.testLexer) {
    return lexTest();
  } else if (options.fuzzLexer) {
    return lexFuzz();
  } else if (options.testParser) {
    return parserTest();
  } else if (options.fuzzParser) {
    return parserFuzz();
  } else if (options.printAst) {
    return astPrint();
  } else if (options.dotAst) {
    return astDot();
  } else if (options.checkSemantic) {
    return checkSemantic();
  } else if (options.fuzzSemantic) {
    return fuzzSemantic();
  } else if (options.dotAttrAst) {
    return attrAstDot();
  } else if (options.printFirmGraph || options.compileFirm) {
    return compileWithFirmBackend();
  } else if (options.compile) {
    return compileWithOwnBackend();
  }
  return EXIT_FAILURE;
}

bool Compiler::sanityChecks() { return std::cout.good() && std::cerr.good(); }
