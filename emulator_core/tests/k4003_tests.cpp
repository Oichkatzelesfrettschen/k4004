#include <gtest/gtest.h>
#include "emulator_core/source/K4003.hpp"

class K4003Test : public ::testing::Test {
protected:
    K4003 shiftReg;
};

// Test basic construction and initialization
TEST_F(K4003Test, Construction) {
    EXPECT_EQ(shiftReg.getShiftRegisterValue(), 0u);
    EXPECT_TRUE(shiftReg.isOutputEnabled());
    EXPECT_EQ(shiftReg.getParallelOutput(), 0u);
}

// Test reset functionality
TEST_F(K4003Test, Reset) {
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(1);
    EXPECT_NE(shiftReg.getShiftRegisterValue(), 0u);

    shiftReg.reset();
    EXPECT_EQ(shiftReg.getShiftRegisterValue(), 0u);
    EXPECT_TRUE(shiftReg.isOutputEnabled());
}

// Test shifting in single bit
TEST_F(K4003Test, ShiftInSingleBit) {
    uint8_t shiftedOut = shiftReg.shiftIn(1);
    EXPECT_EQ(shiftedOut, 0u);  // Nothing shifted out yet
    EXPECT_EQ(shiftReg.getShiftRegisterValue(), 0b0000000001u);
}

// Test shifting in multiple bits
TEST_F(K4003Test, ShiftInMultipleBits) {
    // Shift in pattern: 1, 0, 1, 0, 1, 0, 1, 0, 1, 0
    // With left-shift: first bit (1) ends up at bit 9 after 9 more shifts
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(0);
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(0);
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(0);
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(0);
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(0);

    // Expected: 0b1010101010 (correct for left-shift behavior)
    // Bit 9: 1st bit (1), Bit 8: 2nd bit (0), ..., Bit 0: 10th bit (0)
    EXPECT_EQ(shiftReg.getShiftRegisterValue(), 0b1010101010u);
}

// Test 10-bit overflow (bit shifts out)
TEST_F(K4003Test, TenBitOverflow) {
    // Fill with all 1s
    for (int i = 0; i < 10; ++i) {
        shiftReg.shiftIn(1);
    }
    EXPECT_EQ(shiftReg.getShiftRegisterValue(), 0b1111111111u);

    // Shift in another bit - should shift out a 1
    uint8_t shiftedOut = shiftReg.shiftIn(0);
    EXPECT_EQ(shiftedOut, 1u);
    EXPECT_EQ(shiftReg.getShiftRegisterValue(), 0b1111111110u);
}

// Test cascading (shifted out bit)
TEST_F(K4003Test, Cascading) {
    // Fill register with pattern: 0,1,0,1,0,1,0,1,0,1
    // Result: 0b0101010101 (bit 9=0, bit 8=1, alternating)
    for (int i = 0; i < 10; ++i) {
        shiftReg.shiftIn(i % 2);
    }

    // Shift in new bits and capture what shifts out
    uint8_t shiftedOut1 = shiftReg.shiftIn(1);
    EXPECT_EQ(shiftedOut1, 0u);  // Bit 9 was 0

    uint8_t shiftedOut2 = shiftReg.shiftIn(0);
    EXPECT_EQ(shiftedOut2, 1u);  // New bit 9 was 1 (from previous shift)

    uint8_t shiftedOut3 = shiftReg.shiftIn(1);
    EXPECT_EQ(shiftedOut3, 0u);  // New bit 9 was 0 (from previous shift)
}

// Test individual output bits
TEST_F(K4003Test, OutputBits) {
    // Shift in pattern: 0,1,0,1,0,1,0,1,0,1
    // Result: 0b0101010101 (with left-shift)
    for (int i = 0; i < 10; ++i) {
        shiftReg.shiftIn(i % 2 == 0 ? 0 : 1);
    }

    // Check individual bits (first bit ends up at bit 9, last at bit 0)
    EXPECT_EQ(shiftReg.getOutputBit(0), 1u);  // 10th shift: 1
    EXPECT_EQ(shiftReg.getOutputBit(1), 0u);  // 9th shift: 0
    EXPECT_EQ(shiftReg.getOutputBit(2), 1u);  // 8th shift: 1
    EXPECT_EQ(shiftReg.getOutputBit(3), 0u);  // 7th shift: 0
    EXPECT_EQ(shiftReg.getOutputBit(4), 1u);  // 6th shift: 1
    EXPECT_EQ(shiftReg.getOutputBit(5), 0u);  // 5th shift: 0
    EXPECT_EQ(shiftReg.getOutputBit(6), 1u);  // 4th shift: 1
    EXPECT_EQ(shiftReg.getOutputBit(7), 0u);  // 3rd shift: 0
    EXPECT_EQ(shiftReg.getOutputBit(8), 1u);  // 2nd shift: 1
    EXPECT_EQ(shiftReg.getOutputBit(9), 0u);  // 1st shift: 0
}

// Test output enable functionality
TEST_F(K4003Test, OutputEnable) {
    // Shift in some data
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(1);
    shiftReg.shiftIn(1);

    // With output enabled
    EXPECT_TRUE(shiftReg.isOutputEnabled());
    EXPECT_EQ(shiftReg.getParallelOutput(), 0b0000000111u);

    // Disable output
    shiftReg.setOutputEnable(false);
    EXPECT_FALSE(shiftReg.isOutputEnabled());
    EXPECT_EQ(shiftReg.getParallelOutput(), 0u);  // Outputs should be 0
    EXPECT_EQ(shiftReg.getOutputBit(0), 0u);      // Individual bits also 0

    // Re-enable output
    shiftReg.setOutputEnable(true);
    EXPECT_EQ(shiftReg.getParallelOutput(), 0b0000000111u);  // Data restored
}

// Test out-of-range bit access
TEST_F(K4003Test, OutOfRangeBitAccess) {
    shiftReg.shiftIn(1);
    EXPECT_EQ(shiftReg.getOutputBit(10), 0u);  // Beyond valid range
    EXPECT_EQ(shiftReg.getOutputBit(255), 0u); // Way beyond range
}

// Test typical keyboard scanning pattern
TEST_F(K4003Test, KeyboardScanPattern) {
    // Simulate keyboard scanning: shift in a single 1 to select a row
    shiftReg.reset();
    shiftReg.shiftIn(1);
    for (int i = 0; i < 9; ++i) {
        shiftReg.shiftIn(0);
    }

    // Should have 1 in bit 9, rest 0s
    EXPECT_EQ(shiftReg.getShiftRegisterValue(), 0b1000000000u);
    EXPECT_EQ(shiftReg.getOutputBit(9), 1u);
    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(shiftReg.getOutputBit(i), 0u);
    }
}

// Test 7-segment display pattern
TEST_F(K4003Test, SevenSegmentPattern) {
    // Shift in pattern for digit "8" (all segments on: 8 ones)
    // Plus 2 extra bits for decimal point and other control (both 0)
    // Pattern: 1,1,1,1,1,1,1,1,0,0
    shiftReg.reset();
    for (int i = 0; i < 8; ++i) {
        shiftReg.shiftIn(1);
    }
    shiftReg.shiftIn(0);  // Decimal point off
    shiftReg.shiftIn(0);  // Extra control

    uint16_t output = shiftReg.getParallelOutput();
    // With left-shift: first 8 ones end up in bits 9-2, last 2 zeros in bits 1-0
    EXPECT_EQ(output, 0b1111111100u);  // Bits 9-2: 11111111, bits 1-0: 00
}
