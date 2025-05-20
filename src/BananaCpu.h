#pragma once

#include <cstdint>
#include <iostream> // Added for std::cerr and std::endl

#include "BananaGpu.h" // Add GPU header
#include "BananaMemory.h"

typedef struct instr {
  uint8_t opcode;
  uint8_t reg_a;
  uint8_t reg_b;
  uint16_t imm;
  int16_t offset;
  uint8_t reg_c;
  uint8_t shift_value;
  uint8_t function;
} instr_t;

class BananaCpu {
private:
  //   uint16_t pc;             // Program Counter (16-bit)
  uint16_t registers[32]; // Register File with 32 16-bit registers
  BananaGpu *gpu;         // Add GPU pointer

  static const uint16_t STACK_START = 0x3400;
  static const uint16_t STACK_BOTTOM = 0x1400;
  static const uint16_t STACK_TOP = 0x33FF;

  bool check_stack_bounds() const;
  uint32_t fetch_instruction();
  void execute_instruction(instr_t *instruction);

public:
  BananaMemory *memory;
  uint16_t pc; // Program Counter (16-bit)
  BananaCpu(BananaMemory *const mem,
            BananaGpu *const gpu); // Modified constructor
  void reset();
  void step();
  //   void run();

  // Register access (reg_a, reg_b, reg_c indexing handled in instruction
  // decode)
  uint16_t get_register(uint8_t reg) const {
    return (reg == 0) ? 0 : registers[reg]; // Zero register always returns 0
  }
  void set_register(uint8_t reg, uint16_t value) {
    if (reg != 0) { // Zero register can't be written
      registers[reg] = value;
      // Check stack pointer bounds if SP (reg 29) is modified
      if (reg == 29 && !check_stack_bounds()) {
        std::cerr << "Stack pointer out of bounds" << std::endl;
        exit(1);
      }
    }
  }
  void jal(instr_t *instruction);
  void s8(instr_t *instruction);
  void bne(instr_t *instruction);
  void s16(instr_t *instruction);
  void or_(instr_t *instruction);
  void sra(instr_t *instruction);
  void srl(instr_t *instruction);
  void sub(instr_t *instruction);
  void jr(instr_t *instruction);
  void slt(instr_t *instruction);
  void sll(instr_t *instruction);
  void nor(instr_t *instruction);
  void add(instr_t *instruction);
  void and_(instr_t *instruction);
  void xor_(instr_t *instruction);
  void beq(instr_t *instruction);
  void l16(instr_t *instruction);
  void addi(instr_t *instruction);
  void j(instr_t *instruction);
  void l8(instr_t *instruction);
};