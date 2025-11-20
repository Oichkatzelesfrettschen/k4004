#include <gtest/gtest.h>
#include "emulator_core/source/K4008.hpp"
#include "emulator_core/source/K4009.hpp"

// K4008 Tests
class K4008Test : public ::testing::Test {
protected:
    K4008 addrLatch;
};

TEST_F(K4008Test, Construction) {
    EXPECT_EQ(addrLatch.getAddress(), 0u);
    EXPECT_EQ(addrLatch.getChipSelect(), 0u);
    EXPECT_TRUE(addrLatch.isChipEnabled());
}

TEST_F(K4008Test, AddressLatching) {
    addrLatch.latchAddress(0xAB);
    EXPECT_EQ(addrLatch.getAddress(), 0xABu);

    addrLatch.latchAddress(0xFF);
    EXPECT_EQ(addrLatch.getAddress(), 0xFFu);
}

TEST_F(K4008Test, ChipSelect) {
    addrLatch.setChipSelect(0x0);
    EXPECT_EQ(addrLatch.getChipSelect(), 0x0u);

    addrLatch.setChipSelect(0xF);
    EXPECT_EQ(addrLatch.getChipSelect(), 0xFu);

    addrLatch.setChipSelect(0x7);
    EXPECT_EQ(addrLatch.getChipSelect(), 0x7u);
}

TEST_F(K4008Test, IOPort) {
    addrLatch.writeIOPort(0xA);
    EXPECT_EQ(addrLatch.readIOPort(), 0xAu);
}

TEST_F(K4008Test, ChipEnable) {
    addrLatch.setChipEnable(false);
    addrLatch.writeIOPort(0xF);
    EXPECT_EQ(addrLatch.readIOPort(), 0u);  // Disabled

    addrLatch.setChipEnable(true);
    addrLatch.writeIOPort(0xA);
    EXPECT_EQ(addrLatch.readIOPort(), 0xAu);  // Enabled
}

// K4009 Tests
class K4009Test : public ::testing::Test {
protected:
    K4009 converter;
};

TEST_F(K4009Test, Construction) {
    EXPECT_EQ(converter.get8BitData(), 0u);
    EXPECT_TRUE(converter.isProgramMode());
    EXPECT_TRUE(converter.isReadMode());
}

TEST_F(K4009Test, Write4BitLowNibble) {
    converter.write4Bit(0xA, false);  // Low nibble
    EXPECT_EQ(converter.get8BitData(), 0x0Au);
}

TEST_F(K4009Test, Write4BitHighNibble) {
    converter.write4Bit(0xB, true);   // High nibble
    EXPECT_EQ(converter.get8BitData(), 0xB0u);
}

TEST_F(K4009Test, Write8BitFromNibbles) {
    converter.write4Bit(0x5, false);  // Low = 0x5
    converter.write4Bit(0xA, true);   // High = 0xA
    EXPECT_EQ(converter.get8BitData(), 0xA5u);
}

TEST_F(K4009Test, Read4Bit) {
    converter.set8BitData(0xC3);
    EXPECT_EQ(converter.read4Bit(false), 0x3u);  // Low nibble
    EXPECT_EQ(converter.read4Bit(true), 0xCu);   // High nibble
}

TEST_F(K4009Test, ModeControl) {
    converter.setMode(true, true);
    EXPECT_TRUE(converter.isProgramMode());
    EXPECT_TRUE(converter.isReadMode());

    converter.setMode(false, false);
    EXPECT_FALSE(converter.isProgramMode());
    EXPECT_FALSE(converter.isReadMode());
}

TEST_F(K4009Test, CycleTracking) {
    EXPECT_TRUE(converter.isLowNibbleCycle());
    EXPECT_FALSE(converter.isHighNibbleCycle());

    converter.advanceCycle();
    EXPECT_FALSE(converter.isLowNibbleCycle());
    EXPECT_TRUE(converter.isHighNibbleCycle());

    converter.advanceCycle();
    EXPECT_TRUE(converter.isLowNibbleCycle());
}

// Combined K4008/K4009 Integration Tests
class K4008K4009IntegrationTest : public ::testing::Test {
protected:
    K4008 addrLatch;
    K4009 converter;
};

TEST_F(K4008K4009IntegrationTest, MemoryWriteCycle) {
    // Simulate writing 0xA5 to address 0x80 on chip 3

    // Cycle 1: Latch address
    addrLatch.latchAddress(0x80);
    addrLatch.setChipSelect(3);

    // Cycle 2-3: Write data (low then high nibble)
    converter.write4Bit(0x5, false);  // Low nibble
    converter.write4Bit(0xA, true);   // High nibble

    EXPECT_EQ(addrLatch.getAddress(), 0x80u);
    EXPECT_EQ(addrLatch.getChipSelect(), 3u);
    EXPECT_EQ(converter.get8BitData(), 0xA5u);
}

TEST_F(K4008K4009IntegrationTest, MemoryReadCycle) {
    // Simulate reading from address 0x40 on chip 1

    // Latch address
    addrLatch.latchAddress(0x40);
    addrLatch.setChipSelect(1);

    // Memory responds with data
    converter.set8BitData(0xC3);

    // Read low nibble
    uint8_t lowNibble = converter.read4Bit(false);
    EXPECT_EQ(lowNibble, 0x3u);

    // Read high nibble
    uint8_t highNibble = converter.read4Bit(true);
    EXPECT_EQ(highNibble, 0xCu);
}
