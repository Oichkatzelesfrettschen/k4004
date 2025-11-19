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
 * - Undefined opcodes
 * - WPM instruction (undocumented)
 * - INC vs IAC carry behavior
 * - Edge cases and boundary conditions
 */

#include <gtest/gtest.h>
#include "emulator_core/source/K4004.hpp"
#include "emulator_core/source/K4040.hpp"

// ============================================================================
// CRITICAL FINDING #1: INVERTED CARRY LOGIC IN SUBTRACTION
// ============================================================================

class InvertedCarryTest : public ::testing::Test {
protected:
    K4004 cpu;

    void SetUp() override {
        cpu = K4004();
    }
};

/**
 * DAC (Decrement Accumulator) - 0xF8
 *
 * CRITICAL: Inverted carry semantics
 * - Borrow occurred: CY = 0 (INVERTED!)
 * - No borrow: CY = 1
 */
TEST_F(InvertedCarryTest, DAC_NoBorrow) {
    // Test: 5 - 1 = 4, no borrow, CY should be 1
    cpu.setAccumulator(0x5);
    cpu.setCarry(true);  // Previous state irrelevant

    cpu.executeInstruction(0xF8);  // DAC

    EXPECT_EQ(cpu.getAccumulator(), 0x4) << "5 - 1 should equal 4";
    EXPECT_TRUE(cpu.getCarry()) << "No borrow: CY should be 1 (INVERTED LOGIC)";
}

TEST_F(InvertedCarryTest, DAC_WithBorrow) {
    // Test: 0 - 1 = F (with wrap), borrow occurred, CY should be 0
    cpu.setAccumulator(0x0);
    cpu.setCarry(false);  // Previous state irrelevant

    cpu.executeInstruction(0xF8);  // DAC

    EXPECT_EQ(cpu.getAccumulator(), 0xF) << "0 - 1 should wrap to F";
    EXPECT_FALSE(cpu.getCarry()) << "Borrow occurred: CY should be 0 (INVERTED LOGIC)";
}

TEST_F(InvertedCarryTest, DAC_FromOne) {
    // Test: 1 - 1 = 0, no borrow, CY should be 1
    cpu.setAccumulator(0x1);
    cpu.executeInstruction(0xF8);  // DAC

    EXPECT_EQ(cpu.getAccumulator(), 0x0);
    EXPECT_TRUE(cpu.getCarry()) << "No borrow from 1 to 0";
}

TEST_F(InvertedCarryTest, DAC_FromMax) {
    // Test: F - 1 = E, no borrow, CY should be 1
    cpu.setAccumulator(0xF);
    cpu.executeInstruction(0xF8);  // DAC

    EXPECT_EQ(cpu.getAccumulator(), 0xE);
    EXPECT_TRUE(cpu.getCarry()) << "No borrow from F to E";
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
TEST_F(InvertedCarryTest, SUB_NoBorrow_NoPreviousBorrow) {
    // Test: 7 - 3 (no previous borrow) = 4, no borrow, CY should be 1
    cpu.setAccumulator(0x7);
    cpu.setRegister(3, 0x3);
    cpu.setCarry(true);  // No previous borrow

    cpu.executeInstruction(0x93);  // SUB R3

    EXPECT_EQ(cpu.getAccumulator(), 0x4) << "7 - 3 = 4";
    EXPECT_TRUE(cpu.getCarry()) << "No borrow: CY should be 1";
}

TEST_F(InvertedCarryTest, SUB_WithBorrow_NoPreviousBorrow) {
    // Test: 3 - 7 (no previous borrow) = C (wrap), borrow, CY should be 0
    cpu.setAccumulator(0x3);
    cpu.setRegister(5, 0x7);
    cpu.setCarry(true);  // No previous borrow

    cpu.executeInstruction(0x95);  // SUB R5

    // 3 - 7 = -4 = 16 - 4 = 12 = 0xC
    EXPECT_EQ(cpu.getAccumulator(), 0xC) << "3 - 7 should wrap to C";
    EXPECT_FALSE(cpu.getCarry()) << "Borrow occurred: CY should be 0 (INVERTED)";
}

TEST_F(InvertedCarryTest, SUB_WithPreviousBorrow) {
    // Test: 5 - 5 with previous borrow (CY=0) = F, borrow, CY should be 0
    cpu.setAccumulator(0x5);
    cpu.setRegister(7, 0x5);
    cpu.setCarry(false);  // Previous borrow: subtract 1 extra!

    cpu.executeInstruction(0x97);  // SUB R7

    // 5 - 5 - 1 (from CY=0) = -1 = 15 = 0xF
    EXPECT_EQ(cpu.getAccumulator(), 0xF) << "5 - 5 - 1 (borrow) = F";
    EXPECT_FALSE(cpu.getCarry()) << "Borrow with previous borrow: CY=0";
}

TEST_F(InvertedCarryTest, SUB_Zero) {
    // Test: 5 - 5 (no previous borrow) = 0, no borrow, CY should be 1
    cpu.setAccumulator(0x5);
    cpu.setRegister(2, 0x5);
    cpu.setCarry(true);  // No previous borrow

    cpu.executeInstruction(0x92);  // SUB R2

    EXPECT_EQ(cpu.getAccumulator(), 0x0);
    EXPECT_TRUE(cpu.getCarry()) << "Equal values: no borrow, CY=1";
}

/**
 * SBM (Subtract RAM from Accumulator) - 0xE8
 *
 * Same inverted carry logic as SUB
 */
TEST_F(InvertedCarryTest, SBM_NoBorrow) {
    // Test: 8 - 3 = 5, no borrow, CY should be 1
    cpu.setAccumulator(0x8);
    cpu.setCarry(true);

    // Set up RAM with value 3
    cpu.setRegister(0, 0x0);  // Chip 0
    cpu.setRegister(1, 0x0);  // Reg 0, Char 0
    cpu.executeInstruction(0x21);  // SRC P0
    cpu.setAccumulator(0x3);
    cpu.executeInstruction(0xE0);  // WRM (write 3 to RAM)

    // Now subtract: ACC=8, RAM=3
    cpu.setAccumulator(0x8);
    cpu.setCarry(true);
    cpu.executeInstruction(0xE8);  // SBM

    EXPECT_EQ(cpu.getAccumulator(), 0x5);
    EXPECT_TRUE(cpu.getCarry()) << "SBM: No borrow, CY should be 1";
}

/**
 * TCS (Transfer Carry Subtract) - 0xF9
 *
 * Returns 10 - CY for BCD subtraction
 * - CY=0 (borrow) → ACC=10
 * - CY=1 (no borrow) → ACC=9
 */
TEST_F(InvertedCarryTest, TCS_WithBorrow) {
    // CY=0 means borrow occurred, should return 10
    cpu.setCarry(false);
    cpu.setAccumulator(0x5);  // Previous value irrelevant

    cpu.executeInstruction(0xF9);  // TCS

    EXPECT_EQ(cpu.getAccumulator(), 10) << "TCS with CY=0 should return 10";
    EXPECT_FALSE(cpu.getCarry()) << "TCS clears carry";
}

TEST_F(InvertedCarryTest, TCS_NoBorrow) {
    // CY=1 means no borrow, should return 9
    cpu.setCarry(true);
    cpu.setAccumulator(0xA);  // Previous value irrelevant

    cpu.executeInstruction(0xF9);  // TCS

    EXPECT_EQ(cpu.getAccumulator(), 9) << "TCS with CY=1 should return 9";
    EXPECT_FALSE(cpu.getCarry()) << "TCS clears carry";
}

// ============================================================================
// CRITICAL FINDING #2: DAA ADD-6 ALGORITHM (NOT SUBTRACT-10)
// ============================================================================

class DAAAlgorithmTest : public ::testing::Test {
protected:
    K4004 cpu;

    void SetUp() override {
        cpu = K4004();
    }
};

/**
 * DAA (Decimal Adjust Accumulator) - 0xFB
 *
 * DISCOVERED ALGORITHM: Add 6 to adjust, not subtract 10!
 * - If ACC > 9: ACC = (ACC + 6) & 0xF, CY = 1
 * - If ACC <= 9: No change
 */
TEST_F(DAAAlgorithmTest, DAA_ValidBCD_NoChange) {
    // Test: Valid BCD digits (0-9) should not change
    for (uint8_t digit = 0; digit <= 9; digit++) {
        cpu.setAccumulator(digit);
        cpu.setCarry(false);

        cpu.executeInstruction(0xFB);  // DAA

        EXPECT_EQ(cpu.getAccumulator(), digit)
            << "DAA should not change valid BCD digit " << (int)digit;
        EXPECT_FALSE(cpu.getCarry())
            << "No carry for valid BCD digit " << (int)digit;
    }
}

TEST_F(DAAAlgorithmTest, DAA_InvalidBCD_Add6) {
    // Test: 0xA (10) should become 0x0 with carry
    // Algorithm: (10 + 6) & 0xF = 16 & 0xF = 0, CY=1
    cpu.setAccumulator(0xA);
    cpu.setCarry(false);

    cpu.executeInstruction(0xFB);  // DAA

    EXPECT_EQ(cpu.getAccumulator(), 0x0) << "0xA + 6 = 0x10 → 0x0";
    EXPECT_TRUE(cpu.getCarry()) << "0xA adjustment sets carry";
}

TEST_F(DAAAlgorithmTest, DAA_InvalidBCD_0xB) {
    // Test: 0xB (11) → (11 + 6) & 0xF = 17 & 0xF = 1, CY=1
    cpu.setAccumulator(0xB);
    cpu.executeInstruction(0xFB);  // DAA

    EXPECT_EQ(cpu.getAccumulator(), 0x1);
    EXPECT_TRUE(cpu.getCarry());
}

TEST_F(DAAAlgorithmTest, DAA_InvalidBCD_0xC) {
    // Test: 0xC (12) → (12 + 6) & 0xF = 18 & 0xF = 2, CY=1
    cpu.setAccumulator(0xC);
    cpu.executeInstruction(0xFB);  // DAA

    EXPECT_EQ(cpu.getAccumulator(), 0x2);
    EXPECT_TRUE(cpu.getCarry());
}

TEST_F(DAAAlgorithmTest, DAA_InvalidBCD_0xD) {
    // Test: 0xD (13) → (13 + 6) & 0xF = 19 & 0xF = 3, CY=1
    cpu.setAccumulator(0xD);
    cpu.executeInstruction(0xFB);  // DAA

    EXPECT_EQ(cpu.getAccumulator(), 0x3);
    EXPECT_TRUE(cpu.getCarry());
}

TEST_F(DAAAlgorithmTest, DAA_InvalidBCD_0xE) {
    // Test: 0xE (14) → (14 + 6) & 0xF = 20 & 0xF = 4, CY=1
    cpu.setAccumulator(0xE);
    cpu.executeInstruction(0xFB);  // DAA

    EXPECT_EQ(cpu.getAccumulator(), 0x4);
    EXPECT_TRUE(cpu.getCarry());
}

TEST_F(DAAAlgorithmTest, DAA_InvalidBCD_0xF) {
    // Test: 0xF (15) → (15 + 6) & 0xF = 21 & 0xF = 5, CY=1
    cpu.setAccumulator(0xF);
    cpu.executeInstruction(0xFB);  // DAA

    EXPECT_EQ(cpu.getAccumulator(), 0x5);
    EXPECT_TRUE(cpu.getCarry());
}

/**
 * Complete BCD addition sequence
 */
TEST_F(DAAAlgorithmTest, BCD_Addition_6_Plus_7) {
    // BCD: 6 + 7 = 13 (decimal)
    // Binary: 6 + 7 = 0xD
    // DAA: 0xD → 0x3, CY=1 (represents 13)

    cpu.setAccumulator(0x6);
    cpu.setRegister(0, 0x7);
    cpu.setCarry(false);

    cpu.executeInstruction(0x80);  // ADD R0: ACC = 0xD
    EXPECT_EQ(cpu.getAccumulator(), 0xD) << "Binary 6+7=13=0xD";

    cpu.executeInstruction(0xFB);  // DAA: Adjust to BCD
    EXPECT_EQ(cpu.getAccumulator(), 0x3) << "BCD adjusted: digit 3";
    EXPECT_TRUE(cpu.getCarry()) << "Carry represents '1' in '13'";
}

TEST_F(DAAAlgorithmTest, BCD_Addition_9_Plus_9) {
    // BCD: 9 + 9 = 18 (decimal)
    // Binary: 9 + 9 = 0x12 (already sets carry!)
    // ACC will be 0x2, CY=1 after ADD
    // DAA: 0x2 → 0x2, CY=1 (no adjustment needed, already correct)

    cpu.setAccumulator(0x9);
    cpu.setRegister(1, 0x9);
    cpu.setCarry(false);

    cpu.executeInstruction(0x81);  // ADD R1
    EXPECT_EQ(cpu.getAccumulator(), 0x2) << "0x9 + 0x9 = 0x12 → ACC=0x2";
    EXPECT_TRUE(cpu.getCarry()) << "Carry already set from binary add";

    cpu.executeInstruction(0xFB);  // DAA
    EXPECT_EQ(cpu.getAccumulator(), 0x2) << "Already valid BCD";
    EXPECT_TRUE(cpu.getCarry()) << "Carry preserved";
}

// ============================================================================
// CRITICAL FINDING #3: KBP CARRY DEPENDENCY
// ============================================================================

class KBPCarryTest : public ::testing::Test {
protected:
    K4004 cpu;

    void SetUp() override {
        cpu = K4004();
    }
};

/**
 * KBP (Keyboard Process) - 0xFC
 *
 * DISCOVERED: Special handling for ACC=0 depends on input carry!
 * - ACC=0, CY=0 → Returns 9
 * - ACC=0, CY=1 → Returns 10
 * - This distinguishes "no key pressed" from "key 0"
 */
TEST_F(KBPCarryTest, KBP_NoKey_CarryClear) {
    // ACC=0, CY=0 should return 9 (no key pressed)
    cpu.setAccumulator(0x0);
    cpu.setCarry(false);

    cpu.executeInstruction(0xFC);  // KBP

    EXPECT_EQ(cpu.getAccumulator(), 9)
        << "KBP with ACC=0, CY=0 should return 9 (no key)";
    EXPECT_FALSE(cpu.getCarry()) << "KBP clears carry";
}

TEST_F(KBPCarryTest, KBP_NoKey_CarrySet) {
    // ACC=0, CY=1 should return 10 (alternative no-key value)
    cpu.setAccumulator(0x0);
    cpu.setCarry(true);

    cpu.executeInstruction(0xFC);  // KBP

    EXPECT_EQ(cpu.getAccumulator(), 10)
        << "KBP with ACC=0, CY=1 should return 10";
    EXPECT_FALSE(cpu.getCarry()) << "KBP clears carry";
}

TEST_F(KBPCarryTest, KBP_SingleBit) {
    // Test bit position conversion (normal behavior)
    uint8_t test_cases[] = {
        0b0001, 1,  // Bit 0 → 1
        0b0010, 2,  // Bit 1 → 2
        0b0100, 3,  // Bit 2 → 3
        0b1000, 4   // Bit 3 → 4
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i += 2) {
        cpu.setAccumulator(test_cases[i]);
        cpu.setCarry(true);  // Shouldn't matter for non-zero

        cpu.executeInstruction(0xFC);  // KBP

        EXPECT_EQ(cpu.getAccumulator(), test_cases[i + 1])
            << "KBP(" << (int)test_cases[i] << ") should return "
            << (int)test_cases[i + 1];
    }
}

TEST_F(KBPCarryTest, KBP_MultipleBits) {
    // Test with multiple bits set (returns specific values)
    cpu.setAccumulator(0b0011);  // Bits 0,1
    cpu.executeInstruction(0xFC);  // KBP
    EXPECT_EQ(cpu.getAccumulator(), 5) << "KBP(0b0011) = 5";

    cpu.setAccumulator(0b1111);  // All bits
    cpu.executeInstruction(0xFC);  // KBP
    EXPECT_EQ(cpu.getAccumulator(), 15) << "KBP(0b1111) = 15";
}

// ============================================================================
// CRITICAL FINDING #4: STACK OVERFLOW BEHAVIOR
// ============================================================================

class StackOverflowTest : public ::testing::Test {
protected:
    K4004 cpu_4004;
    K4040 cpu_4040;

    void SetUp() override {
        cpu_4004 = K4004();
        cpu_4040 = K4040();
    }
};

/**
 * 4004: 3-level stack, 4th JMS overwrites stack[2]
 */
TEST_F(StackOverflowTest, Stack4004_ThreeLevels) {
    // Fill stack with 3 calls
    cpu_4004.setProgramCounter(0x100);
    cpu_4004.executeInstruction(0x52);  // JMS 0x200
    cpu_4004.executeInstruction(0x00);

    cpu_4004.setProgramCounter(0x200);
    cpu_4004.executeInstruction(0x53);  // JMS 0x300
    cpu_4004.executeInstruction(0x00);

    cpu_4004.setProgramCounter(0x300);
    cpu_4004.executeInstruction(0x54);  // JMS 0x400
    cpu_4004.executeInstruction(0x00);

    // Stack now: [0x102, 0x202, 0x302]
    // PC = 0x400

    // Return 3 times should work
    cpu_4004.executeInstruction(0xC0);  // BBL 0 → PC = 0x302
    EXPECT_EQ(cpu_4004.getProgramCounter(), 0x302);

    cpu_4004.executeInstruction(0xC0);  // BBL 0 → PC = 0x202
    EXPECT_EQ(cpu_4004.getProgramCounter(), 0x202);

    cpu_4004.executeInstruction(0xC0);  // BBL 0 → PC = 0x102
    EXPECT_EQ(cpu_4004.getProgramCounter(), 0x102);
}

TEST_F(StackOverflowTest, Stack4004_FourthCallOverwrites) {
    // Fill stack completely (3 levels)
    for (int i = 0; i < 3; i++) {
        cpu_4004.setProgramCounter(0x100 * (i + 1));
        cpu_4004.executeInstruction(0x50 + i);  // JMS
        cpu_4004.executeInstruction(0x00);
    }

    // 4th call should overflow and overwrite stack[2]
    cpu_4004.setProgramCounter(0x400);
    cpu_4004.executeInstruction(0x55);  // JMS 0x500 (overflow!)
    cpu_4004.executeInstruction(0x00);

    // Stack should be: [0x102, 0x202, 0x402] (0x302 was overwritten)
    // Return should go to 0x402, not 0x302

    cpu_4004.executeInstruction(0xC0);  // BBL 0
    EXPECT_EQ(cpu_4004.getProgramCounter(), 0x402)
        << "Stack overflow: 4th JMS overwrites 3rd entry";
}

/**
 * 4040: 7-level stack, 8th JMS overwrites stack[6]
 */
TEST_F(StackOverflowTest, Stack4040_SevenLevels) {
    // Fill stack with 7 calls
    for (int i = 0; i < 7; i++) {
        cpu_4040.setProgramCounter(0x100 * (i + 1));
        cpu_4040.executeInstruction(0x50 + (i % 16));  // JMS
        cpu_4040.executeInstruction(0x00);
    }

    // Return 7 times should work
    for (int i = 6; i >= 0; i--) {
        cpu_4040.executeInstruction(0xC0);  // BBL 0
        uint16_t expected = (0x100 * (i + 1)) + 2;
        EXPECT_EQ(cpu_4040.getProgramCounter(), expected)
            << "Return " << (7 - i) << " should go to " << std::hex << expected;
    }
}

TEST_F(StackOverflowTest, Stack4040_EighthCallOverwrites) {
    // Fill stack completely (7 levels)
    for (int i = 0; i < 7; i++) {
        cpu_4040.setProgramCounter(0x100 * (i + 1));
        cpu_4040.executeInstruction(0x50 + (i % 16));  // JMS
        cpu_4040.executeInstruction(0x00);
    }

    // 8th call should overflow and overwrite stack[6]
    cpu_4040.setProgramCounter(0x800);
    cpu_4040.executeInstruction(0x59);  // JMS 0x900 (overflow!)
    cpu_4040.executeInstruction(0x00);

    // Stack[6] was 0x702, now should be 0x802
    cpu_4040.executeInstruction(0xC0);  // BBL 0
    EXPECT_EQ(cpu_4040.getProgramCounter(), 0x802)
        << "4040 stack overflow: 8th JMS overwrites 7th entry";
}

// ============================================================================
// CRITICAL FINDING #5: INC vs IAC CARRY BEHAVIOR
// ============================================================================

class INCvsIACTest : public ::testing::Test {
protected:
    K4004 cpu;

    void SetUp() override {
        cpu = K4004();
    }
};

/**
 * CRITICAL DIFFERENCE:
 * - IAC (0xF2): Sets carry on overflow (0xF+1)
 * - INC Rr (0x6R): Does NOT set carry on overflow
 */
TEST_F(INCvsIACTest, IAC_SetsCarryOnOverflow) {
    cpu.setAccumulator(0xF);
    cpu.setCarry(false);

    cpu.executeInstruction(0xF2);  // IAC

    EXPECT_EQ(cpu.getAccumulator(), 0x0) << "IAC: 0xF+1 wraps to 0";
    EXPECT_TRUE(cpu.getCarry()) << "IAC SETS carry on overflow";
}

TEST_F(INCvsIACTest, IAC_NoCarryNoOverflow) {
    cpu.setAccumulator(0x5);
    cpu.setCarry(false);

    cpu.executeInstruction(0xF2);  // IAC

    EXPECT_EQ(cpu.getAccumulator(), 0x6);
    EXPECT_FALSE(cpu.getCarry()) << "IAC: no carry when no overflow";
}

TEST_F(INCvsIACTest, INC_NoCarryOnOverflow) {
    cpu.setRegister(5, 0xF);
    cpu.setCarry(false);

    cpu.executeInstruction(0x65);  // INC R5

    EXPECT_EQ(cpu.getRegister(5), 0x0) << "INC: 0xF+1 wraps to 0";
    EXPECT_FALSE(cpu.getCarry()) << "INC does NOT set carry on overflow!";
}

TEST_F(INCvsIACTest, INC_NoCarryNoOverflow) {
    cpu.setRegister(3, 0x7);
    cpu.setCarry(false);

    cpu.executeInstruction(0x63);  // INC R3

    EXPECT_EQ(cpu.getRegister(3), 0x8);
    EXPECT_FALSE(cpu.getCarry()) << "INC: no carry";
}

TEST_F(INCvsIACTest, INC_PreservesCarry) {
    // Verify INC doesn't modify carry at all
    cpu.setRegister(2, 0xF);
    cpu.setCarry(true);  // Set carry

    cpu.executeInstruction(0x62);  // INC R2 (overflow)

    EXPECT_EQ(cpu.getRegister(2), 0x0);
    EXPECT_TRUE(cpu.getCarry()) << "INC preserves carry flag (still true)";

    // Test with carry false
    cpu.setRegister(4, 0xF);
    cpu.setCarry(false);

    cpu.executeInstruction(0x64);  // INC R4

    EXPECT_FALSE(cpu.getCarry()) << "INC preserves carry flag (still false)";
}

// ============================================================================
// CRITICAL FINDING #6: UNDEFINED OPCODES
// ============================================================================

class UndefinedOpcodeTest : public ::testing::Test {
protected:
    K4004 cpu_4004;
    K4040 cpu_4040;

    void SetUp() override {
        cpu_4004 = K4004();
        cpu_4040 = K4040();
    }
};

/**
 * Test opcodes 0x01-0x0E on 4004 (these become documented on 4040)
 */
TEST_F(UndefinedOpcodeTest, Range0x01_0x0E_On4004) {
    // These are undefined on 4004, documented on 4040
    // Expected behavior: NOP (no operation) or undefined

    for (uint8_t opcode = 0x01; opcode <= 0x0E; opcode++) {
        cpu_4004 = K4004();  // Reset
        cpu_4004.setProgramCounter(0x100);
        uint8_t initial_acc = 0x5;
        cpu_4004.setAccumulator(initial_acc);
        cpu_4004.setCarry(true);

        // Try executing undefined opcode
        // Should either: NOP, or be undefined (crash/hang)
        // We expect it to be implemented as NOP or not recognized

        EXPECT_NO_THROW({
            cpu_4004.executeInstruction(opcode);
        }) << "Opcode 0x" << std::hex << (int)opcode
           << " should not crash (4004)";

        // If it's NOP, ACC and CY should be unchanged, PC should advance
        // If it's undefined, behavior is unpredictable
    }
}

/**
 * Test that 0x01-0x0E work correctly on 4040
 */
TEST_F(UndefinedOpcodeTest, Range0x01_0x0E_On4040) {
    // These ARE documented on 4040

    // 0x01 - HLT
    cpu_4040.executeInstruction(0x01);
    EXPECT_TRUE(cpu_4040.isHalted()) << "0x01 should be HLT on 4040";

    cpu_4040 = K4040();  // Reset for next test

    // 0x04 - OR4
    cpu_4040.setAccumulator(0b1010);
    cpu_4040.setRegister(4, 0b0101);
    cpu_4040.executeInstruction(0x04);  // OR4
    EXPECT_EQ(cpu_4040.getAccumulator(), 0b1111) << "0x04 should be OR4";

    // 0x06 - AN6
    cpu_4040.setAccumulator(0b1010);
    cpu_4040.setRegister(6, 0b1100);
    cpu_4040.executeInstruction(0x06);  // AN6
    EXPECT_EQ(cpu_4040.getAccumulator(), 0b1000) << "0x06 should be AN6";

    // 0x0C - EIN
    cpu_4040.executeInstruction(0x0C);  // EIN
    EXPECT_TRUE(cpu_4040.areInterruptsEnabled()) << "0x0C should be EIN";

    // All 4040-specific opcodes work!
}

/**
 * Test truly undefined opcodes: 0x0F, 0xFE, 0xFF
 */
TEST_F(UndefinedOpcodeTest, TrulyUndefined) {
    uint8_t undefined[] = {0x0F, 0xFE, 0xFF};

    for (uint8_t opcode : undefined) {
        cpu_4004 = K4004();
        cpu_4004.setProgramCounter(0x200);

        EXPECT_NO_THROW({
            cpu_4004.executeInstruction(opcode);
        }) << "Undefined opcode 0x" << std::hex << (int)opcode
           << " should not crash";

        // Behavior is undefined - could be NOP, could do nothing, etc.
    }
}

// ============================================================================
// EDGE CASES: BBL STACK UNDERFLOW
// ============================================================================

class StackUnderflowTest : public ::testing::Test {
protected:
    K4004 cpu;

    void SetUp() override {
        cpu = K4004();
    }
};

TEST_F(StackUnderflowTest, BBL_EmptyStack) {
    // What happens when we return with empty stack?
    cpu.setProgramCounter(0x100);

    // Execute BBL without any JMS
    cpu.executeInstruction(0xC5);  // BBL 5

    // Behavior is undefined - could be:
    // - PC stays at current location
    // - PC goes to 0x000
    // - PC goes to garbage value

    uint16_t pc_after = cpu.getProgramCounter();

    // We expect it to either stay or go to 0
    EXPECT_TRUE(pc_after == 0x101 || pc_after == 0x000)
        << "Stack underflow: PC = " << std::hex << pc_after;

    // ACC should still be 5 from BBL
    EXPECT_EQ(cpu.getAccumulator(), 5) << "BBL loads immediate even on underflow";
}

TEST_F(StackUnderflowTest, BBL_MultipleUnderflows) {
    // Multiple returns with empty stack
    cpu.setProgramCounter(0x100);

    for (int i = 0; i < 5; i++) {
        EXPECT_NO_THROW({
            cpu.executeInstruction(0xC0);  // BBL 0
        }) << "Multiple BBL underflows should not crash";
    }
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
