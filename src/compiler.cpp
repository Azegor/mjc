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
#include "asm_optimizer.hpp"

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

int Compiler::compile() {
  SymbolTable::StringTable strTbl;
  Parser parser{inputFile, strTbl};
  try {
    auto ast = parser.parseProgram();
    analyzeAstSemantic(ast.get(), parser.getLexer());
    FirmVisitor firmVisitor{options.printFirmGraph};
    ast->accept(&firmVisitor);

    for (auto g : firmVisitor.getFirmGraphs()) {
      lower_highlevel_graph(g);
    }

    if (options.optimize) {
      Optimizer opt(firmVisitor.getFirmGraphs(), options.printFirmGraph, !options.noVerify);
      if (!opt.run()) {
        return EXIT_FAILURE;
      }
    }
    std::string outputName = options.outputFileName.empty() ? "a.out" : options.outputFileName;
    if (!lowerFirmGraphs(firmVisitor.getFirmGraphs(), options.printFirmGraph, !options.noVerify, options.outputAssembly, outputName))
      return EXIT_FAILURE;

    return EXIT_SUCCESS;
  } catch (CompilerError &e) {
    e.writeErrorMessage(std::cerr);
    return EXIT_FAILURE;
  }
}

bool Compiler::lowerFirmGraphs(std::vector<ir_graph*> &graphs, bool printGraphs, bool verifyGraphs, bool outputAssembly, const std::string &outFileName) {
  int graphErrors = 0;
  for (auto g : graphs) {
    // needs to happen in this order to correctly remove all bads that we insert ourselves
    remove_bads(g);
    remove_unreachable_code(g);
    remove_bads(g);

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
  if (options.compileFirm) {
    be_parse_arg("isa=amd64");
    be_main(f, "test.java");
    fflush(f);
  } else {
    // XXX This is a bit of a hack as we opened the tmpfile already...
    // but we won't write to it here so it's probably okay
    AsmPass asmPass(graphs, options.optimize);
    asmPass.run();
    Asm::Program *program = asmPass.getProgram();

    // Run optimizations on ASM code
    if (options.optimize) {
      AsmJumpOptimizer opt1(program);
      opt1.run();
      opt1.printOptimizations();

      AsmSimpleOptimizer opt2(program);
      opt2.run();
      opt2.printOptimizations();

      AsmMovOptimizer opt3(program);
      opt3.run();
      opt3.printOptimizations(); // TODO: Maybe add -g or something for these?
    }

    // Actually write ASM output
    std::ofstream outputFile(assemblyName);
    if (!outputFile.is_open()) {
      throw std::runtime_error("could not open file '" + assemblyName + '\'');
    }
    outputFile << *program << std::endl;
  }

  int res = 0;
  // XXX -g and -gstabs+ for debugging so we can step through asm instructions
  res |= system(("gcc -g -gstabs+ -static -x assembler " + assemblyName + " -o " + outFileName + " -L" LIBSEARCHDIR " -lruntime").c_str());
  //res |= system(("gcc -static -x assembler " + assemblyName + " -o " + outFileName + " -L" LIBSEARCHDIR " -lruntime").c_str());
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
                          options.dotAttrAst, options.compileFirm) > 1)
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
  } else {
    return compile();
  }
  return EXIT_FAILURE;
}

bool Compiler::sanityChecks() { return std::cout.good() && std::cerr.good(); }
