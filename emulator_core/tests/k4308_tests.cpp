#include <gtest/gtest.h>
#include "emulator_core/source/K4308.hpp"
#include <vector>

class K4308Test : public ::testing::Test {
protected:
    K4308 rom;
};

// Test construction
TEST_F(K4308Test, Construction) {
    const uint8_t* contents = rom.getRomContents();
    for (uint16_t i = 0; i < K4308::ROM_SIZE; ++i) {
        EXPECT_EQ(contents[i], 0u);
    }
}

// Test ROM loading
TEST_F(K4308Test, LoadROM) {
    std::vector<uint8_t> data(1024);
    for (uint16_t i = 0; i < 1024; ++i) {
        data[i] = i & 0xFF;
    }

    EXPECT_TRUE(rom.load(data.data(), data.size()));

    for (uint16_t i = 0; i < 1024; ++i) {
        EXPECT_EQ(rom.readByte(i), static_cast<uint8_t>(i & 0xFF));
    }
}

// Test partial ROM loading
TEST_F(K4308Test, PartialLoad) {
    std::vector<uint8_t> data(512, 0xAA);
    EXPECT_TRUE(rom.load(data.data(), data.size()));

    for (uint16_t i = 0; i < 512; ++i) {
        EXPECT_EQ(rom.readByte(i), 0xAAu);
    }
    for (uint16_t i = 512; i < 1024; ++i) {
        EXPECT_EQ(rom.readByte(i), 0x00u);  // Unloaded portion
    }
}

// Test 10-bit address masking
TEST_F(K4308Test, AddressMasking) {
    std::vector<uint8_t> data(1024, 0x55);
    rom.load(data.data(), data.size());

    // Address 0x3FF = 1023 (last valid address)
    EXPECT_EQ(rom.readByte(0x3FF), 0x55u);

    // Address beyond 10 bits should wrap/mask
    EXPECT_EQ(rom.readByte(0x7FF), rom.readByte(0x3FF));  // Should be same
}

// Test I/O port write/read
TEST_F(K4308Test, IOPort) {
    rom.writeIOPort(0xA);
    EXPECT_EQ(rom.readIOPort(), 0xAu);

    rom.writeIOPort(0x5);
    EXPECT_EQ(rom.readIOPort(), 0x5u);
}

// Test I/O port masking
TEST_F(K4308Test, IOPortMasking) {
    rom.setIOMask(0x0C);  // 0b1100 - bits 2,3 input, bits 0,1 output

    rom.writeIOPort(0xF);
    uint8_t value = rom.readIOPort();

    // Bits 0,1 should be 1 (outputs)
    EXPECT_EQ(value & 0x03, 0x03);

    // Bits 2,3 should be 0 (inputs, not set by write)
    EXPECT_EQ(value & 0x0C, 0x00);
}

// Test external I/O port setting
TEST_F(K4308Test, ExternalIOPort) {
    rom.setIOMask(0x0F);  // All inputs
    rom.setExternalIOPort(0xC);
    EXPECT_EQ(rom.readIOPort(), 0xCu);
}

// Test reset
TEST_F(K4308Test, Reset) {
    std::vector<uint8_t> data(1024, 0xFF);
    rom.load(data.data(), data.size());
    rom.writeIOPort(0xF);

    rom.reset();

    for (uint16_t i = 0; i < K4308::ROM_SIZE; ++i) {
        EXPECT_EQ(rom.readByte(i), 0u);
    }
    EXPECT_EQ(rom.readIOPort(), 0u);
}
