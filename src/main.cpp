#include <iostream>

#include "BananaConsole.h"
#include "BananaCpu.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <slug_file>" << std::endl;
    return 0;
  }

  // Create a console with the binary file.
  BananaConsole console(argv[1]);
  
  // Execute the binary.
  console.run();

  return 0;
};