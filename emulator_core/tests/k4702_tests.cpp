#include <gtest/gtest.h>
#include "emulator_core/source/K4702.hpp"

class K4702Test : public ::testing::Test {
protected:
    K4702 eprom;
};

// Test construction (should be erased)
TEST_F(K4702Test, Construction) {
    EXPECT_TRUE(eprom.isErased());
    for (uint16_t i = 0; i < K4702::EPROM_SIZE; ++i) {
        EXPECT_EQ(eprom.readByte(i), K4702::ERASED_VALUE);
    }
}

// Test erase functionality
TEST_F(K4702Test, Erase) {
    eprom.setProgramMode(true);
    eprom.program(0, 0x00);
    EXPECT_FALSE(eprom.isErased());

    eprom.erase();
    EXPECT_TRUE(eprom.isErased());
    EXPECT_EQ(eprom.getEraseCount(), 2u);  // Constructor + explicit erase
}

// Test programming
TEST_F(K4702Test, Programming) {
    eprom.setProgramMode(true);

    EXPECT_TRUE(eprom.program(0, 0xAA));
    EXPECT_EQ(eprom.readByte(0), 0xAAu);

    EXPECT_TRUE(eprom.program(255, 0x55));
    EXPECT_EQ(eprom.readByte(255), 0x55u);
}

// Test programming requires program mode
TEST_F(K4702Test, ProgramModeRequired) {
    eprom.setProgramMode(false);
    EXPECT_FALSE(eprom.program(0, 0xAA));
    EXPECT_EQ(eprom.readByte(0), K4702::ERASED_VALUE);

    eprom.setProgramMode(true);
    EXPECT_TRUE(eprom.program(0, 0xAA));
    EXPECT_EQ(eprom.readByte(0), 0xAAu);
}

// Test EPROM can only change 1s to 0s
TEST_F(K4702Test, OnlyChangeonestozeros) {
    eprom.setProgramMode(true);

    // Program 0xAA (1010 1010)
    eprom.program(0, 0xAA);
    EXPECT_EQ(eprom.readByte(0), 0xAAu);

    // Try to program 0xFF (would require changing 0s to 1s) - should succeed (no change)
    EXPECT_TRUE(eprom.program(0, 0xFF));
    EXPECT_EQ(eprom.readByte(0), 0xAAu);  // Should still be 0xAA

    // Program 0x88 (1000 1000) - can clear more bits
    EXPECT_TRUE(eprom.program(0, 0x88));
    EXPECT_EQ(eprom.readByte(0), 0x88u);  // 0xAA AND 0x88 = 0x88

    // Try to program 0xCC (1100 1100) - would require setting bits 2 and 6
    EXPECT_FALSE(eprom.program(0, 0xCC));
    EXPECT_EQ(eprom.readByte(0), 0x88u);  // Should still be 0x88
}

// Test byte erased check
TEST_F(K4702Test, ByteErased) {
    eprom.setProgramMode(true);

    EXPECT_TRUE(eprom.isByteErased(0));
    eprom.program(0, 0x00);
    EXPECT_FALSE(eprom.isByteErased(0));
    EXPECT_TRUE(eprom.isByteErased(1));  // Other bytes still erased
}

// Test reset
TEST_F(K4702Test, Reset) {
    eprom.setProgramMode(true);
    eprom.program(0, 0x55);
    eprom.program(10, 0xAA);

    eprom.reset();

    EXPECT_TRUE(eprom.isErased());
    EXPECT_EQ(eprom.getProgramCount(), 0u);
    EXPECT_EQ(eprom.getEraseCount(), 0u);
}

// Test program counting
TEST_F(K4702Test, ProgramCounting) {
    eprom.setProgramMode(true);

    EXPECT_EQ(eprom.getProgramCount(), 0u);

    eprom.program(0, 0xFF);
    EXPECT_EQ(eprom.getProgramCount(), 1u);

    eprom.program(1, 0xAA);
    EXPECT_EQ(eprom.getProgramCount(), 2u);
}

// Test typical use case: program then read
TEST_F(K4702Test, TypicalUseCaseProgramAndRead) {
    eprom.setProgramMode(true);

    // Program a sequence
    for (uint8_t i = 0; i < 16; ++i) {
        EXPECT_TRUE(eprom.program(i, i));
    }

    // Read it back
    for (uint8_t i = 0; i < 16; ++i) {
        EXPECT_EQ(eprom.readByte(i), i);
    }
}
