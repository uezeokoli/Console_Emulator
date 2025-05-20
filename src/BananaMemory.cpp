#include "BananaMemory.h"

#include <string.h>

#include <fstream>
#include <iostream>

#include "BananaConsole.h"

// Using the class's mem_array (declared in BananaMemory.h) for storage.

BananaMemory::BananaMemory(BananaConsole *const console,
                           const std::string &filename) {
  // Initialize the lower RAM region (0x0000 to 0x7000) to zero.
  this->console = console;
  memset(mem_array, 0, 0x7000);

  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return;
  }

  const size_t file_size_ = file.tellg();
  if (file_size_ != 0x8000) {
    std::cerr << "Bad file size" << std::endl;
    exit(1);
  }

  file.seekg(0, std::ios::beg);
  // Load the SLUG file into memory starting at address 0x8000.
  file.read(reinterpret_cast<char *>(mem_array + 0x8000), 0x8000);
  file.close();
}

// Helper: returns true if the given address is in a readable range.
bool BananaMemory::is_readable(const uint16_t address) const {
  // RAM: 0x0000 to 0x7000 is readable.
  if (address < 0x7000) return true;
  // I/O: 0x7000 (controller data) and 0x7100 (debug stdin) are readable.
  if (address == 0x7000 || address == 0x7100) return true;
  // SLUG File area: 0x8000 and above is readable (and executable).
  if (address >= 0x8000) return true;
  return false;
}

// Helper: returns true if the given address is writable.
bool BananaMemory::is_writeable(const uint16_t address) const {
  // RAM: 0x0000 to 0x7000 is writable.
  if (address < 0x7000) return true;
  // I/O: debug stdout (0x7110), debug stderr (0x7120), and stop execution
  // (0x7200) are writable.
  if (address == 0x7110 || address == 0x7120 || address == 0x7200) return true;
  // Other regions are not writable.
  return false;
}

uint8_t BananaMemory::l8_u(const uint16_t address) const {
  if (!is_readable(address)) return 0;
  // Special handling for debug standard input.
  if (address == 0x7100) return getchar();

  if (address == 0x7000) return (uint8_t)510;
  return mem_array[address];
}

uint16_t BananaMemory::l16(const uint16_t address) const {
  // Read two consecutive bytes (big-endian) using l8_u to benefit from the
  // special handling.
  return (static_cast<uint16_t>(l8_u(address)) << 8) | l8_u(address + 1);
}

void BananaMemory::s8(uint16_t address, uint8_t value) {
  if (!is_writeable(address)) return;
  // Special handling for debug output.
  if (address == 0x7110) {
    std::cout << static_cast<char>(value);
    return;
  }
  if (address == 0x7120) {
    std::cerr << static_cast<char>(value);
    return;
  }
  mem_array[address] = value;
}

void BananaMemory::s16(uint16_t address, uint16_t value) {
  if (!is_writeable(address)) return;
  mem_array[address] = (value >> 8) & 0xFF;
  mem_array[address + 1] = value & 0xFF;
}

uint32_t BananaMemory::l32(const uint16_t load_address) const {
  uint32_t out = 0;
  // Check for proper word alignment.
  if (load_address & 0b11) {
    std::cerr << "Warning: Trying to read word not on an word boundary."
              << std::endl;
  }
  out |= (static_cast<uint32_t>(mem_array[load_address]) << 24);
  out |= (static_cast<uint32_t>(mem_array[load_address + 1]) << 16);
  out |= (static_cast<uint32_t>(mem_array[load_address + 2]) << 8);
  out |= static_cast<uint32_t>(mem_array[load_address + 3]);
  return out;
}
