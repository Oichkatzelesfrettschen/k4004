#include <gtest/gtest.h>
#include <cstring>
#include "emulator_core/source/K4004.hpp"
#include "emulator_core/source/ram.hpp"
#include "emulator_core/source/rom.hpp"
#include "shared/source/assembly.hpp"

// Cycle-Accurate Timing Tests
// Validates that instruction cycle counts match Intel 4004/4040 specifications
//
// Intel 4004 Timing:
// - One instruction cycle = 8 clock cycles at 740kHz
// - Single-cycle instructions: 1 instruction cycle (8 clock cycles)
// - Double-cycle instructions: 2 instruction cycles (16 clock cycles)
//
// This test suite validates instruction cycle counts (not clock cycles)
// To convert to clock cycles: multiply by 8

class CycleTimingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Format object code: 0xFE (header), 0xFF (end of masks), then program
        objectCode[0] = 0xFE;  // Header byte
        objectCode[1] = 0xFF;  // End of I/O masks
        std::memcpy(&objectCode[2], program, 254);

        rom.load(objectCode, 256);
        cpu = std::make_unique<K4004>(rom, ram);
        cpu->resetCycleCount();
    }

    void executeSingleInstruction() {
        cpu->resetCycleCount();
        cpu->clock();
    }

    uint64_t getCycles() {
        return cpu->getCycleCount();
    }

    ROM rom;
    RAM ram;
    std::unique_ptr<K4004> cpu;
    uint8_t program[254] = {0};  // Program data (without header)
    uint8_t objectCode[256] = {0};  // Full object code with header
};

// ============================================================================
// Single-Cycle Instructions (1 instruction cycle each)
// ============================================================================

TEST_F(CycleTimingTest, NOP_OneCycle) {
    program[0] = +AsmIns::NOP;
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, CLB_OneCycle) {
    program[0] = +AsmIns::CLB;
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, IAC_OneCycle) {
    program[0] = +AsmIns::IAC;
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, DAA_OneCycle) {
    program[0] = +AsmIns::DAA;
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, WRM_OneCycle) {
    program[0] = +AsmIns::WRM;
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, RDM_OneCycle) {
    program[0] = +AsmIns::RDM;
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, ADD_OneCycle) {
    program[0] = (+AsmIns::ADD) | 0;  // ADD R0
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, INC_OneCycle) {
    program[0] = (+AsmIns::INC) | 0;  // INC R0
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, LDM_OneCycle) {
    program[0] = (+AsmIns::LDM) | 5;  // LDM 5
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

// ============================================================================
// Double-Cycle Instructions (2 instruction cycles each)
// ============================================================================

TEST_F(CycleTimingTest, JCN_TwoCycles) {
    program[0] = (+AsmIns::JCN) | 0;  // JCN with no jump
    program[1] = 0x00;  // Address byte
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 2u);
}

TEST_F(CycleTimingTest, FIM_TwoCycles) {
    program[0] = (+AsmIns::FIM) | 0;  // FIM R0R1
    program[1] = 0x55;  // Data byte
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 2u);
}

TEST_F(CycleTimingTest, FIN_TwoCycles) {
    program[0] = (+AsmIns::FIN) | 0;  // FIN R0R1
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 2u);
}

TEST_F(CycleTimingTest, JUN_TwoCycles) {
    program[0] = (+AsmIns::JUN) | 0;  // JUN
    program[1] = 0x00;  // Address byte
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 2u);
}

TEST_F(CycleTimingTest, JMS_TwoCycles) {
    program[0] = (+AsmIns::JMS) | 0;  // JMS
    program[1] = 0x00;  // Address byte
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 2u);
}

TEST_F(CycleTimingTest, ISZ_TwoCycles) {
    program[0] = (+AsmIns::ISZ) | 0;  // ISZ R0
    program[1] = 0x00;  // Address byte
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 2u);
}

// ============================================================================
// Single-Cycle Instructions that might be confused with double-cycle
// ============================================================================

TEST_F(CycleTimingTest, SRC_OneCycle) {
    program[0] = (+AsmIns::SRC) | 0;  // SRC R0R1
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, JIN_OneCycle) {
    program[0] = (+AsmIns::JIN) | 0;  // JIN R0R1
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, BBL_OneCycle) {
    program[0] = (+AsmIns::BBL) | 0;  // BBL 0
    SetUp();
    executeSingleInstruction();
    EXPECT_EQ(getCycles(), 1u);
}

// ============================================================================
// Multi-Instruction Sequences
// ============================================================================

TEST_F(CycleTimingTest, MultipleInstructions_AccumulateCycles) {
    program[0] = +AsmIns::NOP;   // 1 cycle
    program[1] = +AsmIns::IAC;   // 1 cycle
    program[2] = +AsmIns::DAA;   // 1 cycle
    program[3] = (+AsmIns::JUN) | 0;  // 2 cycles
    program[4] = 0x00;  // Address byte for JUN
    SetUp();

    cpu->resetCycleCount();
    cpu->clock();  // NOP - 1 cycle
    cpu->clock();  // IAC - 1 cycle
    cpu->clock();  // DAA - 1 cycle
    cpu->clock();  // JUN - 2 cycles

    EXPECT_EQ(getCycles(), 5u);  // 1+1+1+2 = 5 cycles
}

TEST_F(CycleTimingTest, ResetCycleCount_ClearsCounter) {
    program[0] = +AsmIns::NOP;
    program[1] = +AsmIns::NOP;
    program[2] = +AsmIns::NOP;
    SetUp();

    cpu->resetCycleCount();
    cpu->clock();  // 1 cycle
    cpu->clock();  // 2 cycles
    cpu->clock();  // 3 cycles
    EXPECT_EQ(getCycles(), 3u);

    cpu->resetCycleCount();
    EXPECT_EQ(getCycles(), 0u);

    cpu->clock();  // 1 cycle
    EXPECT_EQ(getCycles(), 1u);
}

TEST_F(CycleTimingTest, LongProgram_CycleAccumulation) {
    // Fill program with NOPs
    for (int i = 0; i < 100; i++) {
        program[i] = +AsmIns::NOP;
    }
    SetUp();

    cpu->resetCycleCount();
    for (int i = 0; i < 100; i++) {
        cpu->clock();
    }

    EXPECT_EQ(getCycles(), 100u);  // 100 NOPs × 1 cycle each
}

TEST_F(CycleTimingTest, MixedSingleAndDouble_Cycles) {
    program[0] = +AsmIns::NOP;          // 1 cycle
    program[1] = (+AsmIns::FIM) | 0;    // 2 cycles
    program[2] = 0x12;
    program[3] = +AsmIns::IAC;          // 1 cycle
    program[4] = (+AsmIns::JMS) | 0;    // 2 cycles
    program[5] = 0x10;
    SetUp();

    cpu->resetCycleCount();
    cpu->clock();  // NOP - 1
    cpu->clock();  // FIM - 2
    cpu->clock();  // IAC - 1
    cpu->clock();  // JMS - 2

    EXPECT_EQ(getCycles(), 6u);  // 1+2+1+2 = 6 cycles
}
