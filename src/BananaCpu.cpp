#include "BananaCpu.h"

#include <cstdlib>
#include <iostream>

#include "BananaGpu.h"
#include "BananaMemory.h"

void parse_instr(const uint32_t instr, instr_t *const in) {
  in->opcode = instr >> 26;
  in->reg_a = (instr >> 21) & 0x1F;
  in->reg_b = (instr >> 16) & 0x1F;
  in->imm = instr & 0xFFFF;
  in->offset = static_cast<int16_t>(in->imm);
  in->reg_c = (instr >> 11) & 0x1F;
  in->shift_value = (instr >> 6) & 0x1F;
  in->function = instr & 0x3F;
}

BananaCpu::BananaCpu(BananaMemory *const mem, BananaGpu *const gpu)
    : memory(mem), gpu(gpu) {
  reset();
}

void BananaCpu::reset() {
  pc = memory->address_to_setup(); // Start execution at the setup address from
                                   // the SLUG header
  for (int i = 0; i < 32; i++) {
    registers[i] = 0;
  }
  // Initialize stack pointer (register 29) to STACK_START (defined in
  // BananaCpu.h)
  registers[29] = STACK_START;
  const uint16_t rom = memory->address_to_rom();
  const uint16_t ram = memory->address_to_ram();
  const uint16_t d_size = memory->address_to_datasize();
  for (int i = 0; i < d_size; i++) {
    memory->s8(ram + i, memory->l8_u(rom + i));
  }
}

bool BananaCpu::check_stack_bounds() const {
  const uint16_t sp = registers[29];
  return sp >= STACK_BOTTOM && sp <= STACK_TOP;
}

uint32_t BananaCpu::fetch_instruction() {
  // Check if PC is in an executable region (SLUG file is marked as rx, at
  // 0x8000 and above)
  if (pc < 0x8000 || pc >= 0x10000) {
    std::cerr << "Invalid PC value (not in executable range): 0x" << std::hex
              << pc << std::endl;
    exit(1);
  }
  const uint32_t instruction = memory->l32(pc);
  // Increment PC by 4 (unless a branch or jump instruction updates it later)
  pc += 4;
  return instruction;
}

void BananaCpu::s8(instr_t *in) {
  uint8_t reg_a = in->reg_a;
  uint8_t reg_b = in->reg_b;
  uint16_t imm = in->imm;
  int16_t offset = static_cast<int16_t>(imm);
  uint16_t addr = registers[reg_a] + offset;
  uint8_t value = registers[reg_b] & 0xFF;
  memory->s8(addr, value);
}
void BananaCpu::bne(instr_t *in) {
  uint8_t reg_a = in->reg_a;
  uint8_t reg_b = in->reg_b;
  uint16_t imm = in->imm;
  int16_t offset = static_cast<int16_t>(imm);
  if (registers[reg_a] != registers[reg_b]) {
    pc += 4 * offset;
  }
}
void BananaCpu::jal(instr_t *in) {
  uint16_t imm = in->imm;
  registers[31] = pc;
  int16_t offset = static_cast<int16_t>(imm);
  pc = 4 * offset;
}
void BananaCpu::l8(instr_t *in) {
  uint8_t reg_a = in->reg_a;
  uint8_t reg_b = in->reg_b;
  uint16_t imm = in->imm;
  int16_t offset = static_cast<int16_t>(imm);
  uint16_t addr = registers[reg_a] + offset;
  uint8_t value = memory->l8_u(addr);
  registers[reg_b] = value;
}
void BananaCpu::j(instr_t *in) {
  uint16_t imm = in->imm;
  int16_t offset = static_cast<int16_t>(imm);
  pc = 4 * offset;
}
void BananaCpu::addi(instr_t *in) {
  uint8_t reg_a = in->reg_a;
  uint8_t reg_b = in->reg_b;
  uint16_t imm = in->imm;
  int16_t immediate = static_cast<int16_t>(imm);
  registers[reg_b] = registers[reg_a] + immediate;
}
void BananaCpu::l16(instr_t *in) {
  uint8_t reg_a = in->reg_a;
  uint8_t reg_b = in->reg_b;
  uint16_t imm = in->imm;
  int16_t offset = static_cast<int16_t>(imm);
  uint16_t addr = registers[reg_a] + offset;
  uint16_t value = memory->l16(addr);
  registers[reg_b] = value;
}
void BananaCpu::beq(instr_t *in) {
  uint8_t reg_a = in->reg_a;
  uint8_t reg_b = in->reg_b;
  uint16_t imm = in->imm;
  int16_t offset = static_cast<int16_t>(imm);
  if (registers[reg_a] == registers[reg_b]) {
    pc += 4 * offset;
  }
}
void BananaCpu::s16(instr_t *in) {
  uint8_t reg_a = in->reg_a;
  uint8_t reg_b = in->reg_b;
  uint16_t imm = in->imm;
  int16_t offset = static_cast<int16_t>(imm);
  uint16_t addr = registers[reg_a] + offset;
  memory->s16(addr, registers[reg_b]);
}
void BananaCpu::or_(instr_t *in) {
  registers[in->reg_c] = registers[in->reg_a] | registers[in->reg_b];
}
void BananaCpu::sra(instr_t *in) {
  registers[in->reg_c] = static_cast<uint16_t>(
      static_cast<int16_t>(registers[in->reg_b]) >> in->shift_value);
}
void BananaCpu::srl(instr_t *in) {
  registers[in->reg_c] = registers[in->reg_b] >> in->shift_value;
}
void BananaCpu::sub(instr_t *in) {
  registers[in->reg_c] = registers[in->reg_a] - registers[in->reg_b];
}
void BananaCpu::jr(instr_t *in) { pc = registers[in->reg_a]; }
void BananaCpu::slt(instr_t *in) {
  registers[in->reg_c] = (registers[in->reg_a] < registers[in->reg_b]) ? 1 : 0;
}
void BananaCpu::sll(instr_t *in) {
  registers[in->reg_c] = registers[in->reg_b] << in->shift_value;
}
void BananaCpu::nor(instr_t *in) {
  registers[in->reg_c] = ~(registers[in->reg_a] | registers[in->reg_b]);
}
void BananaCpu::add(instr_t *in) {
  registers[in->reg_c] = registers[in->reg_a] + registers[in->reg_b];
}
void BananaCpu::and_(instr_t *in) {
  registers[in->reg_c] = registers[in->reg_a] & registers[in->reg_b];
}
void BananaCpu::xor_(instr_t *in) {
  registers[in->reg_c] = registers[in->reg_a] ^ registers[in->reg_b];
}
void BananaCpu::execute_instruction(instr_t *instruction) {
  // uint8_t opcode = instruction >> 26;
  uint8_t opcode = instruction->opcode;
  // I-type instructions:
  switch (opcode) {
  case 0: { // S8: Store Byte: M[R[reg_a]+Immediate] = R[reg_b](7:0)
    s8(instruction);
    break;
  }
  case 3: { // BNE: if(R[reg_a]!=R[reg_b]) PC = PC + 4 + 4*Immediate
    bne(instruction);
    break;
  }
  case 12: { // JAL: R[31]=PC; PC = 4*Immediate
    jal(instruction);
    break;
  }
  case 19: { // l8_u: R[reg_b] = M[R[reg_a]+Immediate](7:0)
    l8(instruction);
    break;
  }
  case 37: { // J: PC = 4 * Immediate
    j(instruction);
    break;
  }
  case 46: { // ADDI: R[reg_b]=R[reg_a]+Immediate
    addi(instruction);
    break;
  }
  case 50: { // L16: R[reg_b] = M[R[reg_a]+Immediate]
    l16(instruction);
    break;
  }
  case 52: { // BEQ: if(R[reg_a]==R[reg_b]) PC = PC + 4 + 4*Immediate
    beq(instruction);
    break;
  }
  case 55: { // S16: M[R[reg_a]+Immediate]=R[reg_b]
    s16(instruction);
    break;
  }
  // R-type instructions (opcode 4)
  case 4: {
    uint8_t function = instruction->function;
    switch (function) {
    case 0: { // OR: R[reg_c] = R[reg_a] | R[reg_b]
      or_(instruction);
      break;
    }
    case 25: { // SRA: R[reg_c] = (signed) R[reg_b] >> shift_value
      sra(instruction);
      break;
    }
    case 29: { // SRL: R[reg_c] = R[reg_b] >> shift_value
      srl(instruction);
      break;
    }
    case 40: { // SUB: R[reg_c] = R[reg_a] - R[reg_b]
      sub(instruction);
      break;
    }
    case 46: { // JR: PC = R[reg_a]
      jr(instruction);
      break;
    }
    case 50: { // SLT: R[reg_c] = (R[reg_a] < R[reg_b]) ? 1 : 0
      slt(instruction);
      break;
    }
    case 54: { // SLL: R[reg_c] = R[reg_b] << shift_value
      sll(instruction);
      break;
    }
    case 56: { // NOR: R[reg_c] = ~(R[reg_a] | R[reg_b])
      nor(instruction);
      break;
    }
    case 57: { // ADD: R[reg_c] = R[reg_a] + R[reg_b]
      add(instruction);
      break;
    }
    case 58: { // AND: R[reg_c] = R[reg_a] & R[reg_b]
      and_(instruction);
      break;
    }
    case 62: { // XOR: R[reg_c] = R[reg_a] ^ R[reg_b]
      xor_(instruction);
      break;
    }
    default:
      // Unknown R-type function: treat as NOP.
      break;
    }
    break;
  }
  default:
    // Unknown opcode: treat as NOP.
    break;
  }
}

void BananaCpu::step() {
  uint32_t instr = fetch_instruction();
  instr_t *in = new instr_t();
  parse_instr(instr, in);
  if (instr == (0x7200)) {
    delete in;
    exit(0);
  }
  execute_instruction(in);
  delete in;

  // Only try to render after executing the instruction
  // The GPU's shouldRender() will determine if it actually needs to update the
  // screen
  gpu->render();
}
