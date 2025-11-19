#include <gtest/gtest.h>
#include "emulator_core/source/SevenSegmentDisplay.hpp"

class SevenSegmentDisplayTest : public ::testing::Test {
protected:
    SevenSegmentDisplay single{1};
    SevenSegmentDisplay multi{4};
};

TEST_F(SevenSegmentDisplayTest, Construction) {
    EXPECT_EQ(single.numDigits(), 1u);
    EXPECT_EQ(multi.numDigits(), 4u);

    // All digits should be cleared initially
    EXPECT_EQ(single.getSegments(0), 0u);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(multi.getSegments(i), 0u);
    }
}

TEST_F(SevenSegmentDisplayTest, DirectSegmentControl) {
    single.setSegments(0, 0xFF);
    EXPECT_EQ(single.getSegments(0), 0xFFu);

    single.setSegments(0, 0x55);
    EXPECT_EQ(single.getSegments(0), 0x55u);

    // Out of bounds should be ignored
    single.setSegments(10, 0xFF);
    EXPECT_EQ(single.getSegments(10), 0u);
}

TEST_F(SevenSegmentDisplayTest, HexDigitDisplay) {
    // Test all hex digits 0-F
    single.setHexDigit(0, 0);
    EXPECT_EQ(single.getSegments(0), 0b00111111u);  // 0

    single.setHexDigit(0, 1);
    EXPECT_EQ(single.getSegments(0), 0b00000110u);  // 1

    single.setHexDigit(0, 5);
    EXPECT_EQ(single.getSegments(0), 0b01101101u);  // 5

    single.setHexDigit(0, 0xA);
    EXPECT_EQ(single.getSegments(0), 0b01110111u);  // A

    single.setHexDigit(0, 0xF);
    EXPECT_EQ(single.getSegments(0), 0b01110001u);  // F
}

TEST_F(SevenSegmentDisplayTest, BCDDigitDisplay) {
    // Test all BCD digits 0-9
    single.setBCDDigit(0, 0);
    EXPECT_EQ(single.getSegments(0), 0b00111111u);  // 0

    single.setBCDDigit(0, 7);
    EXPECT_EQ(single.getSegments(0), 0b00000111u);  // 7

    single.setBCDDigit(0, 9);
    EXPECT_EQ(single.getSegments(0), 0b01101111u);  // 9

    // Invalid BCD should be ignored
    uint8_t before = single.getSegments(0);
    single.setBCDDigit(0, 15);
    EXPECT_EQ(single.getSegments(0), before);  // Unchanged
}

TEST_F(SevenSegmentDisplayTest, IndividualSegmentControl) {
    single.clear();

    // Turn on segment A (top)
    single.setSegment(0, 'a', true);
    EXPECT_TRUE(single.getSegment(0, 'a'));
    EXPECT_EQ(single.getSegments(0), 0b00000001u);

    // Turn on segment G (middle)
    single.setSegment(0, 'g', true);
    EXPECT_TRUE(single.getSegment(0, 'g'));
    EXPECT_EQ(single.getSegments(0), 0b01000001u);

    // Turn off segment A
    single.setSegment(0, 'a', false);
    EXPECT_FALSE(single.getSegment(0, 'a'));
    EXPECT_EQ(single.getSegments(0), 0b01000000u);
}

TEST_F(SevenSegmentDisplayTest, DecimalPointControl) {
    single.clear();

    single.setDecimalPoint(0, true);
    EXPECT_TRUE(single.getDecimalPoint(0));
    EXPECT_EQ(single.getSegments(0), 0b10000000u);

    single.setDecimalPoint(0, false);
    EXPECT_FALSE(single.getDecimalPoint(0));
    EXPECT_EQ(single.getSegments(0), 0u);
}

TEST_F(SevenSegmentDisplayTest, ClearOperations) {
    multi.setSegments(0, 0xFF);
    multi.setSegments(1, 0xFF);
    multi.setSegments(2, 0xFF);
    multi.setSegments(3, 0xFF);

    multi.clearDigit(1);
    EXPECT_EQ(multi.getSegments(0), 0xFFu);
    EXPECT_EQ(multi.getSegments(1), 0u);
    EXPECT_EQ(multi.getSegments(2), 0xFFu);
    EXPECT_EQ(multi.getSegments(3), 0xFFu);

    multi.clear();
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(multi.getSegments(i), 0u);
    }
}

TEST_F(SevenSegmentDisplayTest, HexValueDisplay) {
    multi.setHexValue(0xABCD);

    // Right-most digit should be D
    EXPECT_EQ(multi.getSegments(3), 0b01011110u);  // D
    // Next digit should be C
    EXPECT_EQ(multi.getSegments(2), 0b00111001u);  // C
    // Next digit should be B
    EXPECT_EQ(multi.getSegments(1), 0b01111100u);  // B
    // Left-most digit should be A
    EXPECT_EQ(multi.getSegments(0), 0b01110111u);  // A
}

TEST_F(SevenSegmentDisplayTest, DecimalValueDisplay) {
    multi.setDecimalValue(1234);

    // Right-most digit should be 4
    EXPECT_EQ(multi.getSegments(3), 0b01100110u);  // 4
    // Next should be 3
    EXPECT_EQ(multi.getSegments(2), 0b01001111u);  // 3
    // Next should be 2
    EXPECT_EQ(multi.getSegments(1), 0b01011011u);  // 2
    // Left-most should be 1
    EXPECT_EQ(multi.getSegments(0), 0b00000110u);  // 1
}

TEST_F(SevenSegmentDisplayTest, DecimalValueZero) {
    multi.setDecimalValue(0);

    // Right-most digit should be 0
    EXPECT_EQ(multi.getSegments(3), 0b00111111u);  // 0
    // Others should be blank
    EXPECT_EQ(multi.getSegments(2), 0u);
    EXPECT_EQ(multi.getSegments(1), 0u);
    EXPECT_EQ(multi.getSegments(0), 0u);
}

TEST_F(SevenSegmentDisplayTest, StringDisplay) {
    multi.setString("HELL");

    // Should display "HELL"
    EXPECT_NE(multi.getSegments(0), 0u);  // H
    EXPECT_NE(multi.getSegments(1), 0u);  // E
    EXPECT_NE(multi.getSegments(2), 0u);  // L
    EXPECT_NE(multi.getSegments(3), 0u);  // L
}

TEST_F(SevenSegmentDisplayTest, ASCIIRepresentation) {
    single.setHexDigit(0, 8);
    std::string ascii = single.toASCII(0);

    // Should contain visual representation
    EXPECT_FALSE(ascii.empty());
    EXPECT_TRUE(ascii.find("_") != std::string::npos);
    EXPECT_TRUE(ascii.find("|") != std::string::npos);
}

TEST_F(SevenSegmentDisplayTest, MultiDigitValue) {
    multi.setHexValue(0x1F);

    // Should show 001F
    EXPECT_EQ(multi.getSegments(0), 0b00111111u);  // 0
    EXPECT_EQ(multi.getSegments(1), 0b00111111u);  // 0
    EXPECT_EQ(multi.getSegments(2), 0b00000110u);  // 1
    EXPECT_EQ(multi.getSegments(3), 0b01110001u);  // F
}
