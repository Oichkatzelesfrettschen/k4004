/**
 * @file instruction_audit_tests.cpp
 * @brief Comprehensive instruction-level audit tests for Intel 4004/4040
 *
 * This test suite verifies critical instruction behaviors discovered during
 * the granular instruction-level audit, including:
 * - Inverted carry logic in subtraction (DAC, SUB, SBM, TCS)
 * - DAA add-6 algorithm (not subtract-10)
 * - KBP carry dependency for ACC=0
 * - Stack overflow behavior
 * - INC vs IAC carry behavior
 * - Edge cases and boundary conditions
 *
 * NOTE: ACC format is 0x0C where C = carry flag (bit 4), 0-3 = value
 */

#include <gtest/gtest.h>
#include "emulator_core/source/instructions.hpp"
#include "emulator_core/source/ram.hpp"
#include "emulator_core/source/rom.hpp"

// Helper macros for ACC manipulation
#define ACC_VALUE(acc) ((acc) & 0x0F)
#define ACC_CARRY(acc) (((acc) >> 4) & 1)
#define MAKE_ACC(val, cy) (((val) & 0x0F) | (((cy) & 1) << 4))
#define SET_CARRY(acc) ((acc) | (1 << 4))
#define CLEAR_CARRY(acc) ((acc) & 0x0F)

// ============================================================================
// CRITICAL FINDING #1: INVERTED CARRY LOGIC IN SUBTRACTION
// ============================================================================

/**
 * DAC (Decrement Accumulator) - 0xF8
 *
 * CRITICAL: Inverted carry semantics
 * - Borrow occurred: CY = 0 (INVERTED!)
 * - No borrow: CY = 1
 */
TEST(InvertedCarryAudit, DAC_NoBorrow) {
    // Test: 5 - 1 = 4, no borrow, CY should be 1
    uint8_t acc = MAKE_ACC(0x5, 1);  // Start with CY=1 (though DAC ignores input)

    DAC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x4) << "5 - 1 should equal 4";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "No borrow: CY should be 1 (INVERTED LOGIC)";
}

TEST(InvertedCarryAudit, DAC_WithBorrow) {
    // Test: 0 - 1 = F (with wrap), borrow occurred, CY should be 0
    uint8_t acc = MAKE_ACC(0x0, 0);

    DAC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0xF) << "0 - 1 should wrap to F";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "Borrow occurred: CY should be 0 (INVERTED LOGIC)";
}

TEST(InvertedCarryAudit, DAC_FromOne) {
    // Test: 1 - 1 = 0, no borrow, CY should be 1
    uint8_t acc = MAKE_ACC(0x1, 0);

    DAC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x0);
    EXPECT_EQ(ACC_CARRY(acc), 1) << "No borrow from 1 to 0";
}

TEST(InvertedCarryAudit, DAC_FromMax) {
    // Test: F - 1 = E, no borrow, CY should be 1
    uint8_t acc = MAKE_ACC(0xF, 0);

    DAC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0xE);
    EXPECT_EQ(ACC_CARRY(acc), 1) << "No borrow from F to E";
}

/**
 * SUB Rr (Subtract Register) - 0x9R
 *
 * CRITICAL: Inverted carry semantics + input carry means borrow
 * - Input CY = 0: Previous digit borrowed, subtract 1 extra
 * - Input CY = 1: No previous borrow
 * - Output CY = 0: Borrow occurred
 * - Output CY = 1: No borrow
 */
TEST(InvertedCarryAudit, SUB_NoBorrow_NoPreviousBorrow) {
    // Test: 7 - 3 (no previous borrow) = 4, no borrow, CY should be 1
    uint8_t registers[8] = {0};
    setRegisterValue(registers, 3, 3);  // R3 = 3
    uint8_t acc = MAKE_ACC(0x7, 1);   // ACC=7, CY=1 (no previous borrow)

    SUB(acc, registers, 0x93);  // SUB R3

    EXPECT_EQ(ACC_VALUE(acc), 0x4) << "7 - 3 = 4";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "No borrow: CY should be 1";
}

TEST(InvertedCarryAudit, SUB_WithBorrow_NoPreviousBorrow) {
    // Test: 3 - 7 (no previous borrow) = C (wrap), borrow, CY should be 0
    uint8_t registers[8] = {0};
    registers[5 / 2] = (5 << 4) | 7;  // R5 = 7
    uint8_t acc = MAKE_ACC(0x3, 1);   // ACC=3, CY=1 (no previous borrow)

    SUB(acc, registers, 0x95);  // SUB R5

    // 3 - 7 = -4 = 16 - 4 = 12 = 0xC
    EXPECT_EQ(ACC_VALUE(acc), 0xC) << "3 - 7 should wrap to C";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "Borrow occurred: CY should be 0 (INVERTED)";
}

TEST(InvertedCarryAudit, SUB_WithPreviousBorrow) {
    // Test: 5 - 5 with previous borrow (CY=0) = F, borrow, CY should be 0
    uint8_t registers[8] = {0};
    registers[7 / 2] = (7 << 4) | 5;  // R7 = 5
    uint8_t acc = MAKE_ACC(0x5, 0);   // ACC=5, CY=0 (previous borrow: subtract 1 extra!)

    SUB(acc, registers, 0x97);  // SUB R7

    // 5 - 5 - 1 (from CY=0) = -1 = 15 = 0xF
    EXPECT_EQ(ACC_VALUE(acc), 0xF) << "5 - 5 - 1 (borrow) = F";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "Borrow with previous borrow: CY=0";
}

TEST(InvertedCarryAudit, SUB_Zero) {
    // Test: 5 - 5 (no previous borrow) = 0, no borrow, CY should be 1
    uint8_t registers[8] = {0};
    setRegisterValue(registers, 2, 5);  // R2 = 5
    uint8_t acc = MAKE_ACC(0x5, 1);   // ACC=5, CY=1 (no previous borrow)

    SUB(acc, registers, 0x92);  // SUB R2

    EXPECT_EQ(ACC_VALUE(acc), 0x0);
    EXPECT_EQ(ACC_CARRY(acc), 1) << "Equal values: no borrow, CY=1";
}

/**
 * SBM (Subtract RAM from Accumulator) - 0xE8
 *
 * Same inverted carry logic as SUB
 */
TEST(InvertedCarryAudit, SBM_NoBorrow) {
    // Test: 8 - 3 = 5, no borrow, CY should be 1
    RAM ram;
    ram.writeSrcAddress(0x00);  // Chip 0, Reg 0, Char 0
    ram.writeRAM(0x3);          // RAM = 3

    uint8_t acc = MAKE_ACC(0x8, 1);  // ACC=8, CY=1

    SBM(acc, ram);

    EXPECT_EQ(ACC_VALUE(acc), 0x5);
    EXPECT_EQ(ACC_CARRY(acc), 1) << "SBM: No borrow, CY should be 1";
}

TEST(InvertedCarryAudit, SBM_WithBorrow) {
    // Test: 2 - 5 = D (wrap), borrow, CY should be 0
    RAM ram;
    ram.writeSrcAddress(0x00);
    ram.writeRAM(0x5);  // RAM = 5

    uint8_t acc = MAKE_ACC(0x2, 1);  // ACC=2, CY=1 (no previous borrow)

    SBM(acc, ram);

    EXPECT_EQ(ACC_VALUE(acc), 0xD) << "2 - 5 = -3 = 13 = 0xD";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "Borrow occurred: CY=0";
}

/**
 * TCS (Transfer Carry Subtract) - 0xF9
 *
 * Returns 10 - CY for BCD subtraction
 * - CY=0 (borrow) → ACC=10
 * - CY=1 (no borrow) → ACC=9
 */
TEST(InvertedCarryAudit, TCS_WithBorrow) {
    // CY=0 means borrow occurred, should return 10
    uint8_t acc = MAKE_ACC(0x5, 0);  // Previous value irrelevant, CY=0

    TCS(acc);

    EXPECT_EQ(ACC_VALUE(acc), 10) << "TCS with CY=0 should return 10";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "TCS clears carry";
}

TEST(InvertedCarryAudit, TCS_NoBorrow) {
    // CY=1 means no borrow, should return 9
    uint8_t acc = MAKE_ACC(0xA, 1);  // Previous value irrelevant, CY=1

    TCS(acc);

    EXPECT_EQ(ACC_VALUE(acc), 9) << "TCS with CY=1 should return 9";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "TCS clears carry";
}

// ============================================================================
// CRITICAL FINDING #2: DAA ADD-6 ALGORITHM (NOT SUBTRACT-10)
// ============================================================================

/**
 * DAA (Decimal Adjust Accumulator) - 0xFB
 *
 * DISCOVERED ALGORITHM: Add 6 to adjust, not subtract 10!
 * - If ACC > 9: ACC = (ACC + 6) & 0xF, CY = 1
 * - If ACC <= 9: No change
 */
TEST(DAAAlgorithmAudit, DAA_ValidBCD_NoChange) {
    // Test: Valid BCD digits (0-9) should not change
    for (uint8_t digit = 0; digit <= 9; digit++) {
        uint8_t acc = MAKE_ACC(digit, 0);

        DAA(acc);

        EXPECT_EQ(ACC_VALUE(acc), digit)
            << "DAA should not change valid BCD digit " << (int)digit;
        EXPECT_EQ(ACC_CARRY(acc), 0)
            << "No carry for valid BCD digit " << (int)digit;
    }
}

TEST(DAAAlgorithmAudit, DAA_InvalidBCD_0xA) {
    // Test: 0xA (10) should become 0x0 with carry
    // Algorithm: (10 + 6) & 0xF = 16 & 0xF = 0, CY=1
    uint8_t acc = MAKE_ACC(0xA, 0);

    DAA(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x0) << "0xA + 6 = 0x10 → 0x0";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "0xA adjustment sets carry";
}

TEST(DAAAlgorithmAudit, DAA_InvalidBCD_0xB) {
    // Test: 0xB (11) → (11 + 6) & 0xF = 17 & 0xF = 1, CY=1
    uint8_t acc = MAKE_ACC(0xB, 0);

    DAA(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x1);
    EXPECT_EQ(ACC_CARRY(acc), 1);
}

TEST(DAAAlgorithmAudit, DAA_InvalidBCD_0xC) {
    // Test: 0xC (12) → (12 + 6) & 0xF = 18 & 0xF = 2, CY=1
    uint8_t acc = MAKE_ACC(0xC, 0);

    DAA(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x2);
    EXPECT_EQ(ACC_CARRY(acc), 1);
}

TEST(DAAAlgorithmAudit, DAA_InvalidBCD_0xD) {
    // Test: 0xD (13) → (13 + 6) & 0xF = 19 & 0xF = 3, CY=1
    uint8_t acc = MAKE_ACC(0xD, 0);

    DAA(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x3);
    EXPECT_EQ(ACC_CARRY(acc), 1);
}

TEST(DAAAlgorithmAudit, DAA_InvalidBCD_0xE) {
    // Test: 0xE (14) → (14 + 6) & 0xF = 20 & 0xF = 4, CY=1
    uint8_t acc = MAKE_ACC(0xE, 0);

    DAA(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x4);
    EXPECT_EQ(ACC_CARRY(acc), 1);
}

TEST(DAAAlgorithmAudit, DAA_InvalidBCD_0xF) {
    // Test: 0xF (15) → (15 + 6) & 0xF = 21 & 0xF = 5, CY=1
    uint8_t acc = MAKE_ACC(0xF, 0);

    DAA(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x5);
    EXPECT_EQ(ACC_CARRY(acc), 1);
}

/**
 * Complete BCD addition sequence
 */
TEST(DAAAlgorithmAudit, BCD_Addition_6_Plus_7) {
    // BCD: 6 + 7 = 13 (decimal)
    // Binary: 6 + 7 = 0xD
    // DAA: 0xD → 0x3, CY=1 (represents 13)

    uint8_t registers[8] = {0};
    setRegisterValue(registers, 0, 7);  // R0 = 7
    uint8_t acc = MAKE_ACC(0x6, 0);

    ADD(acc, registers, 0x80);  // ADD R0: ACC = 0xD
    EXPECT_EQ(ACC_VALUE(acc), 0xD) << "Binary 6+7=13=0xD";

    DAA(acc);  // Adjust to BCD
    EXPECT_EQ(ACC_VALUE(acc), 0x3) << "BCD adjusted: digit 3";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "Carry represents '1' in '13'";
}

TEST(DAAAlgorithmAudit, BCD_Addition_9_Plus_9) {
    // BCD: 9 + 9 = 18 (decimal)
    // Binary: 9 + 9 = 0x12 (carry already set!)
    // ACC will be 0x12 (value=2, carry=1) after ADD
    // DAA: Checks full ACC (0x12) > 9? YES!
    //      Adds 6: 0x12 + 6 = 0x18 (value=8, carry=1)
    // Result: BCD '18' = digit 8, carry 1

    uint8_t registers[8] = {0};
    setRegisterValue(registers, 0, 9);  // R0 = 9
    uint8_t acc = MAKE_ACC(0x9, 0);

    ADD(acc, registers, 0x80);  // ADD R0
    EXPECT_EQ(ACC_VALUE(acc), 0x2) << "0x9 + 0x9 = 0x12 → ACC value=2";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "Carry set from binary add";

    DAA(acc);
    EXPECT_EQ(ACC_VALUE(acc), 0x8) << "DAA: 0x12 + 6 = 0x18 → value=8 (BCD digit)";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "Carry preserved (represents tens digit)";
}

// ============================================================================
// CRITICAL FINDING #3: KBP CARRY DEPENDENCY
// ============================================================================

/**
 * KBP (Keyboard Process) - 0xFC
 *
 * DISCOVERED: Special handling for ACC=0 depends on input carry!
 * - ACC=0, CY=0 → Returns 9
 * - ACC=0, CY=1 → Returns 10
 * - This distinguishes "no key pressed" from "key 0"
 */
TEST(KBPCarryAudit, KBP_NoKey_CarryClear) {
    // ACC=0, CY=0 should return 9 (no key pressed)
    uint8_t acc = MAKE_ACC(0x0, 0);

    KBP(acc);

    EXPECT_EQ(ACC_VALUE(acc), 9)
        << "KBP with ACC=0, CY=0 should return 9 (no key)";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "KBP clears carry";
}

TEST(KBPCarryAudit, KBP_NoKey_CarrySet) {
    // ACC=0, CY=1 should return 10 (alternative no-key value)
    uint8_t acc = MAKE_ACC(0x0, 1);

    KBP(acc);

    EXPECT_EQ(ACC_VALUE(acc), 10)
        << "KBP with ACC=0, CY=1 should return 10";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "KBP clears carry";
}

TEST(KBPCarryAudit, KBP_SingleBit) {
    // Test bit position conversion (normal behavior)
    struct TestCase {
        uint8_t input;
        uint8_t expected;
    };
    TestCase test_cases[] = {
        {0b0001, 1},  // Bit 0 → 1
        {0b0010, 2},  // Bit 1 → 2
        {0b0100, 3},  // Bit 2 → 3
        {0b1000, 4}   // Bit 3 → 4
    };

    for (const auto& tc : test_cases) {
        uint8_t acc = MAKE_ACC(tc.input, 1);  // Carry shouldn't matter for non-zero

        KBP(acc);

        EXPECT_EQ(ACC_VALUE(acc), tc.expected)
            << "KBP(" << (int)tc.input << ") should return " << (int)tc.expected;
    }
}

TEST(KBPCarryAudit, KBP_MultipleBits) {
    // Test with multiple bits set (should return 15 for invalid patterns)
    uint8_t acc = MAKE_ACC(0b0011, 0);  // Bits 0,1
    KBP(acc);
    EXPECT_EQ(ACC_VALUE(acc), 15) << "KBP(0b0011) = 15 (invalid/multiple bits)";

    acc = MAKE_ACC(0b1111, 0);  // All bits
    KBP(acc);
    EXPECT_EQ(ACC_VALUE(acc), 15) << "KBP(0b1111) = 15";
}

// ============================================================================
// CRITICAL FINDING #4: INC vs IAC CARRY BEHAVIOR
// ============================================================================

/**
 * CRITICAL DIFFERENCE:
 * - IAC (0xF2): Sets carry on overflow (0xF+1)
 * - INC Rr (0x6R): Does NOT set carry on overflow
 */
TEST(INCvsIACAudit, IAC_SetsCarryOnOverflow) {
    uint8_t acc = MAKE_ACC(0xF, 0);

    IAC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x0) << "IAC: 0xF+1 wraps to 0";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "IAC SETS carry on overflow";
}

TEST(INCvsIACAudit, IAC_NoCarryNoOverflow) {
    uint8_t acc = MAKE_ACC(0x5, 0);

    IAC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x6);
    EXPECT_EQ(ACC_CARRY(acc), 0) << "IAC: no carry when no overflow";
}

TEST(INCvsIACAudit, INC_NoCarryOnOverflow) {
    uint8_t registers[8] = {0};
    setRegisterValue(registers, 5, 0xF);  // R5 = 0xF

    INC(registers, 0x65);  // INC R5

    uint8_t r5 = getRegisterValue(registers, 5);
    EXPECT_EQ(r5, 0x0) << "INC: 0xF+1 wraps to 0";

    // INC doesn't touch carry at all - we can't test it directly here
    // since INC operates on registers, not ACC
    // The key is that INC doesn't have a carry output mechanism
}

TEST(INCvsIACAudit, INC_BasicIncrement) {
    uint8_t registers[8] = {0};
    setRegisterValue(registers, 3, 7);  // R3 = 7

    INC(registers, 0x63);  // INC R3

    uint8_t r3 = getRegisterValue(registers, 3);
    EXPECT_EQ(r3, 0x8);
}

// ============================================================================
// CRITICAL FINDING #5: STACK OVERFLOW BEHAVIOR
// ============================================================================

/**
 * 4004: 3-level stack, 4th JMS overwrites stack[2]
 * 4040: 7-level stack, 8th JMS overwrites stack[6]
 */
TEST(StackOverflowAudit, Stack4004_ThreeLevels) {
    ROM rom;
    uint16_t stack[3] = {0x100, 0, 0};  // Start at PC=0x100
    uint8_t SP = 0;  // Stack pointer

    // Fill stack with 3 calls
    JMS(stack, SP, 0x52, rom, 3);  // JMS 0x200
    EXPECT_EQ(SP, 1);
    EXPECT_EQ(stack[1], 0x102);  // Return address (PC+2)

    stack[SP] = 0x200;  // Simulate PC at new location
    JMS(stack, SP, 0x53, rom, 3);  // JMS 0x300
    EXPECT_EQ(SP, 2);
    EXPECT_EQ(stack[2], 0x202);

    stack[SP] = 0x300;
    JMS(stack, SP, 0x54, rom, 3);  // JMS 0x400 (fills stack)
    EXPECT_EQ(SP, 2) << "Stack full, SP wraps to 2 (overflow)";
    EXPECT_EQ(stack[2], 0x302) << "4th JMS overwrites stack[2]";
}

TEST(StackOverflowAudit, BBL_Returns) {
    uint16_t stack[3] = {0x100, 0x200, 0x300};
    uint8_t SP = 2;  // Stack full
    uint8_t acc = 0;
    uint8_t registers[8] = {0};

    BBL(stack, SP, acc, registers, 0xC5);  // BBL 5

    EXPECT_EQ(SP, 1) << "BBL decrements SP";
    EXPECT_EQ(ACC_VALUE(acc), 5) << "BBL loads immediate";
}

// ============================================================================
// EDGE CASES: Additional Tests
// ============================================================================

TEST(EdgeCasesAudit, CLB_ClearsBoth) {
    uint8_t acc = MAKE_ACC(0xF, 1);

    CLB(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0) << "CLB clears accumulator";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "CLB clears carry";
}

TEST(EdgeCasesAudit, CLC_ClearsOnlyCarry) {
    uint8_t acc = MAKE_ACC(0x7, 1);

    CLC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x7) << "CLC preserves accumulator";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "CLC clears carry";
}

TEST(EdgeCasesAudit, STC_SetsCarry) {
    uint8_t acc = MAKE_ACC(0x3, 0);

    STC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0x3) << "STC preserves accumulator";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "STC sets carry";
}

TEST(EdgeCasesAudit, CMC_ComplementsCarry) {
    uint8_t acc = MAKE_ACC(0x5, 0);

    CMC(acc);
    EXPECT_EQ(ACC_CARRY(acc), 1) << "CMC: 0→1";

    CMC(acc);
    EXPECT_EQ(ACC_CARRY(acc), 0) << "CMC: 1→0";
}

TEST(EdgeCasesAudit, CMA_ComplementsAcc) {
    uint8_t acc = MAKE_ACC(0b1010, 1);

    CMA(acc);

    EXPECT_EQ(ACC_VALUE(acc), 0b0101) << "CMA complements bits";
    EXPECT_EQ(ACC_CARRY(acc), 1) << "CMA preserves carry";
}

TEST(EdgeCasesAudit, RAL_RotateLeft) {
    uint8_t acc = MAKE_ACC(0b1010, 1);

    RAL(acc);

    // [CY=1][1010] → [1][0101] (bit 3 → CY, CY → bit 0)
    EXPECT_EQ(ACC_VALUE(acc), 0b0101);
    EXPECT_EQ(ACC_CARRY(acc), 1) << "Bit 3 rotated to carry";
}

TEST(EdgeCasesAudit, RAR_RotateRight) {
    uint8_t acc = MAKE_ACC(0b0101, 1);

    RAR(acc);

    // [0101][CY=1] → [1010][1] (CY → bit 3, bit 0 → CY)
    EXPECT_EQ(ACC_VALUE(acc), 0b1010);
    EXPECT_EQ(ACC_CARRY(acc), 1) << "Bit 0 rotated to carry";
}

TEST(EdgeCasesAudit, TCC_TransferCarry) {
    uint8_t acc = MAKE_ACC(0x7, 1);

    TCC(acc);

    EXPECT_EQ(ACC_VALUE(acc), 1) << "TCC: ACC ← CY";
    EXPECT_EQ(ACC_CARRY(acc), 0) << "TCC clears carry";
}
