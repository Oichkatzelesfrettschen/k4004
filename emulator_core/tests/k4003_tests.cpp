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

    // Expected: 0b0101010101 (alternating pattern)
    EXPECT_EQ(shiftReg.getShiftRegisterValue(), 0b0101010101u);
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
    // Fill register with pattern
    for (int i = 0; i < 10; ++i) {
        shiftReg.shiftIn(i % 2);
    }

    // Shift in new bit and capture shifted out
    uint8_t shiftedOut1 = shiftReg.shiftIn(1);
    EXPECT_EQ(shiftedOut1, 0u);  // MSB was 0

    uint8_t shiftedOut2 = shiftReg.shiftIn(0);
    EXPECT_EQ(shiftedOut2, 0u);  // MSB was 0

    uint8_t shiftedOut3 = shiftReg.shiftIn(1);
    EXPECT_EQ(shiftedOut3, 1u);  // MSB was 1
}

// Test individual output bits
TEST_F(K4003Test, OutputBits) {
    // Shift in pattern: 0b1010101010
    for (int i = 0; i < 10; ++i) {
        shiftReg.shiftIn(i % 2 == 0 ? 0 : 1);
    }

    // Check individual bits
    EXPECT_EQ(shiftReg.getOutputBit(0), 0u);
    EXPECT_EQ(shiftReg.getOutputBit(1), 1u);
    EXPECT_EQ(shiftReg.getOutputBit(2), 0u);
    EXPECT_EQ(shiftReg.getOutputBit(3), 1u);
    EXPECT_EQ(shiftReg.getOutputBit(4), 0u);
    EXPECT_EQ(shiftReg.getOutputBit(5), 1u);
    EXPECT_EQ(shiftReg.getOutputBit(6), 0u);
    EXPECT_EQ(shiftReg.getOutputBit(7), 1u);
    EXPECT_EQ(shiftReg.getOutputBit(8), 0u);
    EXPECT_EQ(shiftReg.getOutputBit(9), 1u);
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
    // Shift in pattern for digit "8" (all segments on: 0b11111111)
    // Plus 2 extra bits for decimal point and other control
    shiftReg.reset();
    for (int i = 0; i < 8; ++i) {
        shiftReg.shiftIn(1);
    }
    shiftReg.shiftIn(0);  // Decimal point off
    shiftReg.shiftIn(0);  // Extra control

    uint16_t output = shiftReg.getParallelOutput();
    EXPECT_EQ(output, 0b0011111111u);
}
