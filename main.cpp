#include <fstream>
#include <iostream>
#include <string>

#include <cstdlib>

#include <boost/program_options.hpp>

#include "color_ostream.hpp"

auto color_cout = co::make_colored(std::cout);
auto color_cerr = co::make_colored(std::cerr);

bool readFileToStdout(const std::string inFileName) {
  // open file as binary since we don't care about the content
  std::ifstream inFile(inFileName, std::ios::binary);
  if (!inFile.is_open()) {
    return false;
  }
  std::cout << inFile.rdbuf();
  return true;
}

bool parseArguments(int argc, char *argv[]) {
  std::string inputFile;

  namespace bpo = boost::program_options;

  bpo::positional_options_description posArgs;
  posArgs.add("input-file", 1); // max one input file
  bpo::options_description desc("MJC Compiler Options");
  desc.add_options()
      // help
      ("help,h", "print this help message")
      // echo
      ("echo", "print the content of the input file")
      // input file
      ("input-file", bpo::value<std::string>(&inputFile), "input file");

  bpo::variables_map var_map;
  try {
    bpo::store(bpo::command_line_parser(argc, argv)
                   .options(desc)
                   .positional(posArgs)
                   .run(),
               var_map);

    if (var_map.count("help")) {
      color_cout << co::mode(co::bold) << "Usage: " << co::reset << argv[0]
                 << " [options] input-file" << std::endl;
      std::cout << desc << std::endl;
      exit(EXIT_SUCCESS); // nothing more to do
    }

    bpo::notify(var_map);

    if (!var_map.count("input-file")) {
      color_cerr << co::mode(co::bold) << co::color(co::red)
                 << "error: " << co::reset << "no input files" << std::endl;
      exit(EXIT_FAILURE);
    }
    if (var_map.count("echo")) {
      if (!readFileToStdout(inputFile))
        exit(EXIT_FAILURE);
      exit(EXIT_SUCCESS);
    }
  } catch (bpo::required_option &e) {
    color_cerr << co::mode(co::bold) << co::color(co::red)
               << "error: " << co::reset << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (bpo::error &e) {
    color_cerr << co::mode(co::bold) << co::color(co::red)
               << "error: " << co::reset << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[]) {
  if (!parseArguments(argc, argv)) {
    return EXIT_FAILURE;
  }
  //     int* err = nullptr;
  //     return *err;
  return EXIT_SUCCESS;
}
