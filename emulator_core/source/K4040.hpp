#pragma once
#include <cstdint>

class ROM;
class RAM;

class K4040 {
public:
    static constexpr uint8_t REGISTERS_SIZE = 12u;
    static constexpr uint8_t STACK_SIZE = 7u;  // Intel 4040 has 7-level stack

    K4040(ROM& rom, RAM& ram);

    void reset();
    void step();

    const uint16_t* getStack() const { return m_stack; }
    const uint8_t* getRegisters() const { return m_registers; }
    uint16_t getPC() const { return m_stack[m_SP]; }
    uint8_t getIR() const { return m_IR; }
    uint8_t getACC() const { return m_ACC & 0x0Fu; }
    uint8_t getCY() const { return m_ACC >> 4; }
    uint8_t getTest() const { return m_test; }
    void setTest(uint8_t test) { m_test = test & 1u; }

    // 4040-specific accessors
    uint8_t getRegisterBank() const { return m_currentRegisterBank; }
    uint8_t getROMBank() const { return m_currentROMBank; }
    bool isInterruptEnabled() const { return m_interruptEnabled; }
    bool isHalted() const { return m_halted; }
    void setInterruptPending(bool pending) { m_interruptPending = pending; }

private:
    void incStack() { m_stack[m_SP] = ++m_stack[m_SP] & 0x0FFFu; }  // 12-bit PC base (13-bit with bank)

    // Register banks (4040 has 2 banks of 12 register pairs = 24 total registers)
    uint8_t m_registers_bank0[REGISTERS_SIZE];
    uint8_t m_registers_bank1[REGISTERS_SIZE];
    uint8_t* m_registers;  // Points to current bank
    uint8_t m_currentRegisterBank;  // 0 or 1

    // Stack and control registers
    uint16_t m_stack[STACK_SIZE];
    uint8_t m_SP;
    uint8_t m_IR;
    uint8_t m_ACC;
    uint8_t m_test;

    // 4040-specific hardware state
    uint8_t m_currentROMBank;       // 0 or 1 (for 13-bit addressing, 8KB ROM)
    uint8_t m_commandRegister;      // Command register for LCR instruction
    uint8_t m_srcBackup;            // SRC backup for BBS instruction
    bool m_interruptEnabled;        // Interrupt enable flag (EIN/DIN)
    bool m_halted;                  // Halt state (HLT instruction)
    bool m_interruptPending;        // INT pin state

    // Memory references
    ROM& m_rom;
    RAM& m_ram;
};
