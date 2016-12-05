#include <iostream>
#include <stdexcept>
#include <string>

#include <cstdlib>

#include <boost/program_options.hpp>

#include "color_ostream.hpp"
#include "compiler.hpp"

auto cl_cout = co::make_colored(std::cout);
auto cl_cerr = co::make_colored(std::cerr);

CompilerOptions parseArguments(int argc, char *argv[]) {
  CompilerOptions compilerOptions;

  namespace bpo = boost::program_options;

  bpo::positional_options_description posArgs;
  posArgs.add("input-file", 1); // max one input file
  bpo::options_description desc("MJC Compiler Options");
  desc.add_options()
      // help
      ("help,h", "print this help message")
      // echo
      ("echo", "print the content of the input file")
      // input file (named version)
      ("input-file", bpo::value<std::string>(&compilerOptions.inputFileName),
       "input file")
      // test lexer
      ("lextest", "test the lexer by printing out tokens on per line")
      // fuzz lexer
      ("lexfuzz", "fuzz lexer, no output")
      // test parser
      ("parsetest", "test input file for validity")
      // fuzz parser
      ("parsefuzz", "fuzz parser, no output")
      // print ast
      ("print-ast", "pretty print input program")
      // create graphviz graph from AST
      ("dot-ast", "print AST to dot output")
      // check semantic of program
      ("check", "check whether input program is semantically valid")
      // fuzz semantic check
      ("fuzz-check", "fuzz semantic checker, no output")
      // check semantic of program
      ("dot-attr-ast", "print attributed AST as dot output")
      // check semantic of program
      ("firm-graph", "generate firm graph and dump to file (per function)")
      // libfirm codegen
      ("gen-code", "Let libfirm generate code")
      // compile input file with firm backend
      ("compile-firm", "compile input program with default x86 firm backend")
      // disable verification
      ("no-verify", "disable verification when building firm graph")
      // optimize
      ("optimize,O", "optimize program")
      // output file
      ("output,o", bpo::value<std::string>(&compilerOptions.outputFileName),
       "output file name");

  bpo::variables_map var_map;
  try {
    bpo::store(bpo::command_line_parser(argc, argv)
                   .options(desc)
                   .positional(posArgs)
                   .run(),
               var_map);

    if (var_map.count("help")) {
      compilerOptions.help = true;
      cl_cout << co::mode(co::bold) << "Usage: " << co::reset << argv[0]
              << " [options] file" << std::endl;
      std::cout << desc << std::endl;
      exit(EXIT_SUCCESS); // nothing more to do
    }

    bpo::notify(var_map);

    if (!var_map.count("input-file")) {
      cl_cerr << co::mode(co::bold) << co::color(co::red)
              << "error: " << co::reset << "no input files" << std::endl;
      exit(EXIT_FAILURE);
    }
    if (var_map.count("echo")) {
      compilerOptions.echoFile = true;
    }
    if (var_map.count("lextest")) {
      compilerOptions.testLexer = true;
    }
    if (var_map.count("lexfuzz")) {
      compilerOptions.fuzzLexer = true;
    }
    if (var_map.count("parsetest")) {
      compilerOptions.testParser = true;
    }
    if (var_map.count("parsefuzz")) {
      compilerOptions.fuzzParser = true;
    }
    if (var_map.count("print-ast")) {
      compilerOptions.printAst = true;
    }
    if (var_map.count("dot-ast")) {
      compilerOptions.dotAst = true;
    }
    if (var_map.count("check")) {
      compilerOptions.checkSemantic = true;
    }
    if (var_map.count("fuzz-check")) {
      compilerOptions.fuzzSemantic = true;
    }
    if (var_map.count("dot-attr-ast")) {
      compilerOptions.dotAttrAst = true;
    }
    if (var_map.count("firm-graph")) {
      compilerOptions.printFirmGraph = true;
    }
    if (var_map.count("gen-code")) {
      compilerOptions.genCode = true;
    }
    if (var_map.count("compile-firm")) {
      compilerOptions.compileFirm = true;
    }
    if (var_map.count("no-verify")) {
      compilerOptions.noVerify = true;
    }
    if (var_map.count("optimize") || var_map.count("O")) {
      compilerOptions.optimize = true;
    }
  } catch (bpo::required_option &e) {
    cl_cerr << co::mode(co::bold) << co::color(co::red)
            << "error: " << co::reset << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (bpo::error &e) {
    cl_cerr << co::mode(co::bold) << co::color(co::red)
            << "error: " << co::reset << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  return compilerOptions;
}

int main(int argc, char *argv[]) try {
  CompilerOptions options = parseArguments(argc, argv);
  Compiler compiler(InputFile{options.inputFileName}, options);
  return compiler.run();
} catch (CompilerError &e) {
  e.writeErrorMessage(std::cerr);
  return EXIT_FAILURE;
} catch (std::exception &e) {
  cl_cerr << co::mode(co::bold) << co::color(co::red)
          << "error (std::exception): " << co::color(co::regular) << e.what()
          << std::endl;
  return EXIT_FAILURE;
} catch (...) {
  // catch any exceptions uncaught until now
  cl_cerr << co::mode(co::bold) << co::color(co::red) << "unknown error!"
          << std::endl;
  return EXIT_FAILURE;
}
