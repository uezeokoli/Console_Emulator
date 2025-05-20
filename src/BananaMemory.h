#pragma once

#include <memory>
#include <string>

class BananaConsole;

class BananaMemory {
private:
  BananaConsole *console;
  uint8_t mem_array[0x10000];

public:
  bool is_readable(const uint16_t address) const;
  bool is_writeable(const uint16_t address) const;
  BananaMemory(BananaConsole *const console, const std::string &filename);
  uint8_t l8_u(const uint16_t load_address) const;
  uint16_t l16(const uint16_t load_address) const;
  uint32_t l32(const uint16_t load_address) const;
  uint16_t address_to_setup() const { return l16(0x81e0 + 2); }
  uint16_t address_to_rom() const { return l16(0x81e8 + 2); }
  uint16_t address_to_ram() const { return l16(0x81ec + 2); }
  uint16_t address_to_datasize() const { return l16(0x81f0 + 2); }
  uint16_t address_to_loop() const { return l16(0x81e4 + 2); }
  void s8(uint16_t address, uint8_t value);
  void s16(uint16_t address, uint16_t value);
};
