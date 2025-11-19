#include "emulator_core/source/K4040.hpp"
#include "emulator_core/source/instructions.hpp"
#include "emulator_core/source/ram.hpp"
#include "emulator_core/source/rom.hpp"

#include "shared/source/assembly.hpp"

#include <cstring>

K4040::K4040(ROM& rom, RAM& ram) :
    m_rom(rom),
    m_ram(ram),
    m_registers(m_registers_bank0),  // Start with bank 0
    m_currentRegisterBank(0),
    m_currentROMBank(0),
    m_interruptEnabled(false),
    m_halted(false),
    m_interruptPending(false)
{
    reset();
}

void K4040::reset()
{
    std::memset(m_registers_bank0, 0, REGISTERS_SIZE);
    std::memset(m_registers_bank1, 0, REGISTERS_SIZE);
    std::memset(m_stack, 0, STACK_SIZE * 2);

    m_registers = m_registers_bank0;
    m_currentRegisterBank = 0;
    m_currentROMBank = 0;
    m_SP = 0u;
    m_ACC = 0u;
    m_test = 0u;
    m_commandRegister = 0u;
    m_srcBackup = 0u;
    m_interruptEnabled = false;
    m_halted = false;
    m_interruptPending = false;

    m_ram.reset();
}

void K4040::step()
{
    // Check if halted
    if (m_halted) {
        // Check for interrupt to wake up
        if (m_interruptPending && m_interruptEnabled) {
            m_halted = false;
            // Handle interrupt (simplified - would need full interrupt vector logic)
            // For now, just clear the interrupt
            m_interruptPending = false;
        } else {
            return;  // Stay halted
        }
    }

    m_IR = m_rom.readByte(getPC() | (m_currentROMBank << 12));  // 13-bit addressing with bank
    incStack();

    uint8_t opcode = getOpcodeFromByte(m_IR);
    switch (opcode) {
    case +AsmIns::NOP: NOP(); break;
    // 4040 new instructions
    case +AsmIns::HLT: HLT(m_halted); break;
    case +AsmIns::BBS: BBS(m_stack, m_SP, m_ram, m_rom, m_srcBackup, m_interruptEnabled); break;
    case +AsmIns::LCR: LCR(m_ACC, m_commandRegister); break;
    case +AsmIns::OR4: OR4(m_ACC, m_registers); break;
    case +AsmIns::OR5: OR5(m_ACC, m_registers); break;
    case +AsmIns::AN6: AN6(m_ACC, m_registers); break;
    case +AsmIns::AN7: AN7(m_ACC, m_registers); break;
    case +AsmIns::DB0: DB0(m_currentROMBank); break;
    case +AsmIns::DB1: DB1(m_currentROMBank); break;
    case +AsmIns::SB0: SB0(m_currentRegisterBank); m_registers = m_registers_bank0; break;
    case +AsmIns::SB1: SB1(m_currentRegisterBank); m_registers = m_registers_bank1; break;
    case +AsmIns::EIN: EIN(m_interruptEnabled); break;
    case +AsmIns::DIN: DIN(m_interruptEnabled); break;
    case +AsmIns::RPM: RPM(m_ACC, m_rom, getPC()); break;
    case +AsmIns::WRM: WRM(m_ram, m_ACC); break;
    case +AsmIns::WMP: WMP(m_ram, m_ACC); break;
    case +AsmIns::WRR: WRR(m_rom, m_ACC); break;
    case +AsmIns::WR0: WR0(m_ram, m_ACC); break;
    case +AsmIns::WR1: WR1(m_ram, m_ACC); break;
    case +AsmIns::WR2: WR2(m_ram, m_ACC); break;
    case +AsmIns::WR3: WR3(m_ram, m_ACC); break;
    case +AsmIns::SBM: SBM(m_ACC, m_ram); break;
    case +AsmIns::RDM: RDM(m_ACC, m_ram); break;
    case +AsmIns::RDR: RDR(m_ACC, m_rom); break;
    case +AsmIns::ADM: ADM(m_ACC, m_ram); break;
    case +AsmIns::RD0: RD0(m_ACC, m_ram); break;
    case +AsmIns::RD1: RD1(m_ACC, m_ram); break;
    case +AsmIns::RD2: RD2(m_ACC, m_ram); break;
    case +AsmIns::RD3: RD3(m_ACC, m_ram); break;
    case +AsmIns::CLB: CLB(m_ACC); break;
    case +AsmIns::CLC: CLC(m_ACC); break;
    case +AsmIns::IAC: IAC(m_ACC); break;
    case +AsmIns::CMC: CMC(m_ACC); break;
    case +AsmIns::CMA: CMA(m_ACC); break;
    case +AsmIns::RAL: RAL(m_ACC); break;
    case +AsmIns::RAR: RAR(m_ACC); break;
    case +AsmIns::TCC: TCC(m_ACC); break;
    case +AsmIns::DAC: DAC(m_ACC); break;
    case +AsmIns::TCS: TCS(m_ACC); break;
    case +AsmIns::STC: STC(m_ACC); break;
    case +AsmIns::DAA: DAA(m_ACC); break;
    case +AsmIns::KBP: KBP(m_ACC); break;
    case +AsmIns::DCL: DCL(m_ram, m_ACC); break;
    case +AsmIns::JCN: JCN(m_stack, m_SP, m_IR, m_ACC, m_test, m_rom); break;
    case +AsmIns::FIM: FIM(m_stack, m_SP, m_registers, m_IR, m_rom); break;
    case +AsmIns::SRC: SRC(m_ram, m_rom, m_registers, m_IR); break;
    case +AsmIns::FIN: FIN(m_registers, getPC(), m_IR, m_rom); break;
    case +AsmIns::JIN: JIN(m_stack, m_SP, m_registers, m_IR); break;
    case +AsmIns::JUN: JUN(m_stack, m_SP, m_IR, m_rom); break;
    case +AsmIns::JMS: JMS(m_stack, m_SP, m_IR, m_rom, STACK_SIZE); break;
    case +AsmIns::WPM: WPM(); break;
    case +AsmIns::INC: INC(m_registers, m_IR); break;
    case +AsmIns::ISZ: ISZ(m_stack, m_SP, m_registers, m_IR, m_rom); break;
    case +AsmIns::ADD: ADD(m_ACC, m_registers, m_IR); break;
    case +AsmIns::SUB: SUB(m_ACC, m_registers, m_IR); break;
    case +AsmIns::LD:  LD(m_ACC, m_registers, m_IR);  break;
    case +AsmIns::XCH: XCH(m_ACC, m_registers, m_IR); break;
    case +AsmIns::BBL: BBL(m_stack, m_SP, m_ACC, m_registers, m_IR); break;
    case +AsmIns::LDM: LDM(m_ACC, m_IR); break;
    }
}
