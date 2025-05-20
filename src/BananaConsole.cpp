#include "BananaConsole.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

BananaConsole::BananaConsole(const std::string &filename)
    : filename_(filename), memory(this, filename), gpu(this) {}

void BananaConsole::run() {
  BananaCpu cpu(&memory, &gpu); // Create CPU with GPU reference

  while (cpu.pc != 0x0000) {
    cpu.step();
  }

  // Set pc to the address to the loop address
  uint16_t start = cpu.memory->address_to_loop();
  while (true) {
    cpu.pc = start;
    while (cpu.pc != 0x0000) {
      cpu.step();
    }
  }
}
