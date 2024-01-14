#include "Lib.h"
#include <clara.hpp>

#include <iostream>

int main(int argc, char **argv) {
  using namespace clara;

  std::string name;
  bool help = false;
  auto cli = Opt(name, "name")["-n"]["--name"]("name to greet") | Help(help);

  auto result = cli.parse(Args(argc, argv));
  if (!result) {
    std::cerr << "Error in command line: " << result.errorMessage() << '\n';
    exit(1);
  }
  if (help) {
    std::cout << cli;
    exit(0);
  }

  lib::say_hello(name);
  return 0;
}
