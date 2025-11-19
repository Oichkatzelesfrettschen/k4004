#include "emulator_core/source/instructions.hpp"
#include "emulator_core/source/ram.hpp"
#include "emulator_core/source/rom.hpp"

#include "shared/source/assembly.hpp"

uint8_t getRegisterValue(const uint8_t* registers, uint8_t reg)
{
    bool isOdd = reg % 2;
    uint8_t regPairValue = registers[reg / 2];
    return (regPairValue >> (isOdd ? 0u : 4u)) & 0x0Fu;
}

void setRegisterValue(uint8_t* registers, uint8_t reg, uint8_t value)
{
    bool isOdd = reg % 2;
    registers[reg / 2] &= (isOdd ? 0xF0u : 0x0Fu);
    registers[reg / 2] |= (value << (isOdd ? 0u : 4u)) & (isOdd ? 0x0Fu : 0xF0u);
}

void NOP()
{
}

void JCN(uint16_t* stack, uint8_t SP, uint8_t IR, uint8_t ACC, uint8_t test, const ROM& rom)
{
    uint8_t con = IR & 0x0Fu;
    uint8_t address = rom.readByte(stack[SP]);
    stack[SP] = ++stack[SP] & 0x0FFFu;  // 12-bit PC

    bool shouldJump = false;
    switch (con) {
    case +AsmCon::AEZ: shouldJump = (ACC & 0x0Fu) == 0u; break;
    case +AsmCon::ANZ: shouldJump = (ACC & 0x0Fu) != 0u; break;
    case +AsmCon::CEZ: shouldJump = !(ACC & 0x10u); break;
    case +AsmCon::CNZ: shouldJump = ACC & 0x10u; break;
    case +AsmCon::TEZ: shouldJump = test == 0u; break;
    case +AsmCon::TNZ: shouldJump = test != 0u; break;
    }

    if (shouldJump) {
        if ((stack[SP] & 0x00FFu) == 0xFE) stack[SP] += 2;
        stack[SP] &= 0x0F00u;  // Keep upper 4 bits (page)
        stack[SP] |= address;
    }
}

void FIM(uint16_t* stack, uint8_t SP, uint8_t* registers, uint8_t IR, const ROM& rom)
{
    uint8_t reg = (IR & 0x0Fu) >> 1;
    registers[reg] = rom.readByte(stack[SP]);
    stack[SP] = ++stack[SP] & 0x0FFFu;  // 12-bit PC
}

void SRC(RAM& ram, ROM& rom, const uint8_t* registers, uint8_t IR)
{
    uint8_t reg = (IR & 0x0Fu) >> 1;
    uint8_t addr = registers[reg];
    rom.writeSrcAddress(addr);
    ram.writeSrcAddress(addr);
}

void FIN(uint8_t* registers, uint16_t PC, uint8_t IR, const ROM& rom)
{
    uint8_t reg = (IR & 0x0Fu) >> 1;
    uint8_t addr = registers[0];
    if ((PC & 0x00FFu) == 0xFF) addr += ROM::PAGE_SIZE;
    registers[reg] = rom.readByte(addr);
}

void JIN(uint16_t* stack, uint8_t SP, const uint8_t* registers, uint8_t IR)
{
    uint8_t reg = (IR & 0x0Fu) >> 1;
    uint8_t addr = registers[reg];
    if ((stack[SP] & 0x00FFu) == 0xFFu) ++stack[SP];
    stack[SP] &= 0x0F00u;  // Keep upper 4 bits (page)
    stack[SP] |= addr;
}

void JUN(uint16_t* stack, uint8_t SP, uint8_t IR, const ROM& rom)
{
    uint16_t address = (IR & 0x0Fu) << 8;
    stack[SP] = address | rom.readByte(stack[SP]);
}

void JMS(uint16_t* stack, uint8_t& SP, uint8_t IR, const ROM& rom, uint8_t stackSize)
{
    uint16_t address = (IR & 0x0Fu) << 8;
    address |= rom.readByte(stack[SP]);
    stack[SP] = ++stack[SP] & 0x0FFFu;  // 12-bit PC

    // Stack bounds check - prevent overflow
    if (SP >= stackSize - 1) {
        // Stack overflow - halt or handle error
        // For now, don't increment SP (stay at current level)
        return;
    }

    ++SP;
    stack[SP] = address & 0x0FFFu;  // 12-bit address
}

void INC(uint8_t* registers, uint8_t IR)
{
    uint8_t reg = IR & 0x0Fu;
    uint8_t temp = getRegisterValue(registers, reg);
    setRegisterValue(registers, reg, temp + 1);
}

void ISZ(uint16_t* stack, uint8_t SP, uint8_t* registers, uint8_t IR, const ROM& rom)
{
    uint8_t reg = IR & 0x0Fu;
    uint8_t value = getRegisterValue(registers, reg);
    setRegisterValue(registers, reg, value + 1);
    uint8_t addr = rom.readByte(stack[SP]);
    stack[SP] = ++stack[SP] & 0x0FFFu;  // 12-bit PC

    if (((value + 1) & 0x0Fu) != 0u) {
        if ((stack[SP] & 0x00FFu) == 0xFEu) stack[SP] += 2u;
        stack[SP] &= 0x0F00u;  // Keep upper 4 bits (page)
        stack[SP] |= addr;
    }
}

void ADD(uint8_t& ACC, const uint8_t* registers, uint8_t IR)
{
    uint8_t temp = IR & 0x0Fu;
    uint8_t CY = ACC >> 4;
    temp = getRegisterValue(registers, temp) + CY;
    ACC = (ACC & 0x0Fu) + temp;
}

void SUB(uint8_t& ACC, const uint8_t* registers, uint8_t IR)
{
    uint8_t CY = ACC >> 4 ? 0u : 1u;
    uint8_t temp = IR & 0x0Fu;
    temp = (~getRegisterValue(registers, temp) & 0x0Fu) + CY;
    ACC = (ACC & 0x0Fu) + temp;
}

void LD(uint8_t& ACC, const uint8_t* registers, uint8_t IR)
{
    uint8_t reg = IR & 0x0Fu;
    ACC = getRegisterValue(registers, reg) | (ACC & 0x10u);
}

void XCH(uint8_t& ACC, uint8_t* registers, uint8_t IR)
{
    uint8_t reg = IR & 0x0Fu;
    uint8_t temp = ACC & 0x0Fu;
    ACC = getRegisterValue(registers, reg) | (ACC & 0x10u);
    setRegisterValue(registers, reg, temp);
}

void BBL(uint16_t* stack, uint8_t& SP, uint8_t& ACC, const uint8_t* registers, uint8_t IR)
{
    // Stack bounds check - prevent underflow
    if (SP > 0) {
        stack[SP] = 0u;
        --SP;
    }
    // If SP == 0, we're already at the bottom - stay there

    // Load immediate data (lower 4 bits of IR) directly to accumulator
    ACC = (IR & 0x0Fu) | (ACC & 0x10u);  // Preserve carry flag
}

void LDM(uint8_t& ACC, uint8_t IR)
{
    ACC = (IR & 0x0Fu) | (ACC & 0x10u);
}

void WRM(RAM& ram, uint8_t ACC)
{
    ram.writeRAM(ACC & 0x0Fu);
}

void WMP(RAM& ram, uint8_t ACC)
{
    ram.writeOutputPort(ACC & 0x0Fu);
}

void WRR(ROM& rom, uint8_t ACC)
{
    rom.writeIOPort(ACC & 0x0Fu);
}

void WR0(RAM& ram, uint8_t ACC)
{
    ram.writeStatus(ACC & 0x0Fu, 0u);
}

void WR1(RAM& ram, uint8_t ACC)
{
    ram.writeStatus(ACC & 0x0Fu, 1u);
}

void WR2(RAM& ram, uint8_t ACC)
{
    ram.writeStatus(ACC & 0x0Fu, 2u);
}

void WR3(RAM& ram, uint8_t ACC)
{
    ram.writeStatus(ACC & 0x0Fu, 3u);
}

void SBM(uint8_t& ACC, const RAM& ram)
{
    uint8_t CY = ACC >> 4 ? 0u : 1u;
    uint8_t temp = (~ram.readRAM() & 0x0Fu) + CY;
    ACC = (ACC & 0x0Fu) + temp;
}

void RDM(uint8_t& ACC, const RAM& ram)
{
    ACC = (ram.readRAM() & 0x0Fu) | (ACC & 0x10u);
}

void RDR(uint8_t& ACC, const ROM& rom)
{
    ACC = (rom.readIOPort() & 0x0Fu) | (ACC & 0x10u);
}

void ADM(uint8_t& ACC, const RAM& ram)
{
    uint8_t CY = ACC >> 4;
    uint8_t temp = (ram.readRAM() & 0x0Fu) + CY;
    ACC = (ACC & 0x0Fu) + temp;
}

void RD0(uint8_t& ACC, const RAM& ram)
{
    ACC = (ram.readStatus(0u) & 0x0Fu) | (ACC & 0x10u);
}

void RD1(uint8_t& ACC, const RAM& ram)
{
    ACC = (ram.readStatus(1u) & 0x0Fu) | (ACC & 0x10u);
}

void RD2(uint8_t& ACC, const RAM& ram)
{
    ACC = (ram.readStatus(2u) & 0x0Fu) | (ACC & 0x10u);
}

void RD3(uint8_t& ACC, const RAM& ram)
{
    ACC = (ram.readStatus(3u) & 0x0Fu) | (ACC & 0x10u);
}

void CLB(uint8_t& ACC)
{
    ACC = 0u;
}

void CLC(uint8_t& ACC)
{
    ACC = ACC & 0x0Fu;
}

void IAC(uint8_t& ACC)
{
    ACC &= 0x0Fu;
    ++ACC;
}

void CMC(uint8_t& ACC)
{
    ACC = ACC >> 4 ? ACC & 0x0Fu : ACC | 0x10u;
}

void CMA(uint8_t& ACC)
{
    uint8_t temp = ACC & 0x0Fu;
    ACC &= 0x10u;
    ACC |= ~temp & 0x0Fu;
}

void RAL(uint8_t& ACC)
{
    uint8_t CY = ACC >> 4;
    ACC <<= 1;
    ACC |= CY;
    ACC &= 0x1Fu;
}

void RAR(uint8_t& ACC)
{
    uint8_t CY = ACC & 1u;
    ACC >>= 1;
    ACC |= CY << 4;
}

void TCC(uint8_t& ACC)
{
    ACC >>= 4;
}

void DAC(uint8_t& ACC)
{
    ACC &= 0x0Fu;
    --ACC;
    if (ACC > 0x0Fu)
        ACC &= 0x0Fu;
    else
        ACC |= 0x10u;
}

void TCS(uint8_t& ACC)
{
    ACC = ACC >> 4 ? 10u : 9u;
}

void STC(uint8_t& ACC)
{
    ACC |= 0x10u;
}

void DAA(uint8_t& ACC)
{
    if (ACC > 9u) {
        ACC += 6u;
    }
}

void KBP(uint8_t& ACC)
{
    uint8_t temp = ACC & 0x0Fu;
    if (temp == 0b0000u) return;
    if (temp == 0b0001u) return;
    if (temp == 0b0010u) return;
    if (temp == 0b0100u) { ACC = 0b0011u | (ACC & 0x10u); return; }
    if (temp == 0b1000u) { ACC = 0b0100u | (ACC & 0x10u); return; }
    ACC = 0b1111u | (ACC & 0x10u);
}

void DCL(RAM& ram, uint8_t ACC)
{
    uint8_t temp = ACC & 0x07u;
    switch (temp) {
    case 0b000:
        ram.setRAMBank(0u);
        break;
    case 0b001:
        ram.setRAMBank(1u);
        break;
    case 0b010:
        ram.setRAMBank(2u);
        break;
    case 0b100:
        ram.setRAMBank(3u);
        break;
    }
}

// ============================================================================
// Intel 4040 New Instructions (14 additional instructions)
// ============================================================================

// HLT - Halt processor until interrupt or STP pin
void HLT(bool& halted)
{
    halted = true;
}

// BBS - Branch Back from interrupt, restore SRC
void BBS(uint16_t* stack, uint8_t& SP, RAM& ram, ROM& rom, uint8_t srcBackup, bool& interruptEnabled)
{
    // Restore SRC register from backup
    ram.writeSrcAddress(srcBackup);
    rom.writeSrcAddress(srcBackup);

    // Re-enable interrupts
    interruptEnabled = true;

    // Return from interrupt (pop stack)
    if (SP > 0) {
        stack[SP] = 0u;
        --SP;
    }
}

// LCR - Load Command Register to accumulator
void LCR(uint8_t& ACC, uint8_t commandRegister)
{
    ACC = (commandRegister & 0x0Fu) | (ACC & 0x10u);
}

// OR4 - OR index register 4 with accumulator
void OR4(uint8_t& ACC, const uint8_t* registers)
{
    uint8_t reg4Value = getRegisterValue(registers, 4);
    ACC = ((ACC & 0x0Fu) | reg4Value) | (ACC & 0x10u);
}

// OR5 - OR index register 5 with accumulator
void OR5(uint8_t& ACC, const uint8_t* registers)
{
    uint8_t reg5Value = getRegisterValue(registers, 5);
    ACC = ((ACC & 0x0Fu) | reg5Value) | (ACC & 0x10u);
}

// AN6 - AND index register 6 with accumulator
void AN6(uint8_t& ACC, const uint8_t* registers)
{
    uint8_t reg6Value = getRegisterValue(registers, 6);
    ACC = ((ACC & 0x0Fu) & reg6Value) | (ACC & 0x10u);
}

// AN7 - AND index register 7 with accumulator
void AN7(uint8_t& ACC, const uint8_t* registers)
{
    uint8_t reg7Value = getRegisterValue(registers, 7);
    ACC = ((ACC & 0x0Fu) & reg7Value) | (ACC & 0x10u);
}

// DB0 - Designate ROM Bank 0
void DB0(uint8_t& romBank)
{
    romBank = 0;
}

// DB1 - Designate ROM Bank 1
void DB1(uint8_t& romBank)
{
    romBank = 1;
}

// SB0 - Select index register Bank 0
void SB0(uint8_t& registerBank)
{
    registerBank = 0;
}

// SB1 - Select index register Bank 1
void SB1(uint8_t& registerBank)
{
    registerBank = 1;
}

// EIN - Enable Interrupt system
void EIN(bool& interruptEnabled)
{
    interruptEnabled = true;
}

// DIN - Disable Interrupt system
void DIN(bool& interruptEnabled)
{
    interruptEnabled = false;
}

// RPM - Read Program Memory to accumulator
void RPM(uint8_t& ACC, const ROM& rom, uint16_t PC)
{
    // Read the byte at current PC from program memory
    uint8_t value = rom.readByte(PC);
    ACC = (value & 0x0Fu) | (ACC & 0x10u);
}

// WPM - Write Program Memory (stub - ROM is read-only in most systems)
void WPM()
{
    // This instruction is typically not implemented in emulators
    // as program memory (ROM) is read-only
    // In real hardware with EPROM, this would program the ROM
}
