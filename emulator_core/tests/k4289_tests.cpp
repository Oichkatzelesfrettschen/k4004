#include <gtest/gtest.h>
#include "emulator_core/source/K4289.hpp"

class K4289Test : public ::testing::Test {
protected:
    K4289 memInterface;
};

// Test basic construction and initialization
TEST_F(K4289Test, Construction) {
    EXPECT_EQ(memInterface.getAddress(), 0u);
    EXPECT_EQ(memInterface.readData(), 0u);
    EXPECT_TRUE(memInterface.isChipEnabled());
    EXPECT_TRUE(memInterface.isReadMode());
    EXPECT_FALSE(memInterface.isProgramMemoryMode());
}

// Test reset functionality
TEST_F(K4289Test, Reset) {
    memInterface.setAddress(0x0ABC);
    memInterface.writeData(0x55);
    memInterface.setControl(false, false);

    memInterface.reset();

    EXPECT_EQ(memInterface.getAddress(), 0u);
    EXPECT_EQ(memInterface.readData(), 0u);
    EXPECT_TRUE(memInterface.isChipEnabled());
    EXPECT_TRUE(memInterface.isReadMode());
}

// Test 12-bit address generation
TEST_F(K4289Test, AddressGeneration12Bit) {
    // Set full 12-bit address
    memInterface.setAddress(0x0ABC);  // 1010 1011 1100
    EXPECT_EQ(memInterface.getAddress(), 0x0ABCu);

    // Check 8-bit address portion
    EXPECT_EQ(memInterface.getAddress8bit(), 0xBCu);

    // Check 4-bit chip select portion
    EXPECT_EQ(memInterface.getChipSelect(), 0x0Au);
}

// Test address masking to 12 bits
TEST_F(K4289Test, AddressMasking) {
    memInterface.setAddress(0xFFFF);  // Try to set more than 12 bits
    EXPECT_EQ(memInterface.getAddress(), 0x0FFFu);  // Should be masked to 12 bits
}

// Test 8-bit data bus write
TEST_F(K4289Test, DataWrite) {
    memInterface.setControl(false, true);  // Write mode, enabled
    memInterface.writeData(0xA5);
    EXPECT_EQ(memInterface.readData(), 0xA5u);
}

// Test data write only works when enabled and in write mode
TEST_F(K4289Test, DataWriteControl) {
    // Write in read mode - should not work
    memInterface.setControl(true, true);   // Read mode
    memInterface.writeData(0x55);
    EXPECT_EQ(memInterface.readData(), 0u);  // Should be 0

    // Write when disabled - should not work
    memInterface.setControl(false, false); // Write mode but disabled
    memInterface.writeData(0xAA);
    EXPECT_EQ(memInterface.readData(), 0u);  // Should still be 0

    // Write when enabled and write mode - should work
    memInterface.setControl(false, true);  // Write mode, enabled
    memInterface.writeData(0xCC);
    EXPECT_EQ(memInterface.readData(), 0xCCu);
}

// Test I/O port operations (4001-compatible)
TEST_F(K4289Test, IOPort) {
    // All output mask (default)
    memInterface.writeIOPort(0xF);
    EXPECT_EQ(memInterface.readIOPort(), 0xFu);

    memInterface.writeIOPort(0x0);
    EXPECT_EQ(memInterface.readIOPort(), 0x0u);
}

// Test I/O port with mixed input/output mask
TEST_F(K4289Test, IOPortMasking) {
    // Mask: 0b0101 = bits 0,2 input, bits 1,3 output
    memInterface.setIOMask(0x05);

    memInterface.writeIOPort(0xF);  // Try to write all 1s
    uint8_t value = memInterface.readIOPort();

    // Bits 1 and 3 should be 1 (outputs written)
    EXPECT_EQ(value & 0x02, 0x02);
    EXPECT_EQ(value & 0x08, 0x08);

    // Bits 0 and 2 should be 0 (inputs not affected by write)
    EXPECT_EQ(value & 0x01, 0x00);
    EXPECT_EQ(value & 0x04, 0x00);
}

// Test control signals
TEST_F(K4289Test, ControlSignals) {
    memInterface.setControl(true, true);
    EXPECT_TRUE(memInterface.isReadMode());
    EXPECT_TRUE(memInterface.isChipEnabled());

    memInterface.setControl(false, false);
    EXPECT_FALSE(memInterface.isReadMode());
    EXPECT_FALSE(memInterface.isChipEnabled());
}

// Test program memory mode
TEST_F(K4289Test, ProgramMemoryMode) {
    EXPECT_FALSE(memInterface.isProgramMemoryMode());

    memInterface.setProgramMemoryMode(true);
    EXPECT_TRUE(memInterface.isProgramMemoryMode());

    memInterface.setProgramMemoryMode(false);
    EXPECT_FALSE(memInterface.isProgramMemoryMode());
}

// Test typical address combinations
TEST_F(K4289Test, TypicalAddresses) {
    // Test various chip select + address combinations

    // Chip 0, address 0x00
    memInterface.setAddress(0x0000);
    EXPECT_EQ(memInterface.getChipSelect(), 0x00u);
    EXPECT_EQ(memInterface.getAddress8bit(), 0x00u);

    // Chip 0, address 0xFF
    memInterface.setAddress(0x00FF);
    EXPECT_EQ(memInterface.getChipSelect(), 0x00u);
    EXPECT_EQ(memInterface.getAddress8bit(), 0xFFu);

    // Chip 15, address 0xFF
    memInterface.setAddress(0x0FFF);
    EXPECT_EQ(memInterface.getChipSelect(), 0x0Fu);
    EXPECT_EQ(memInterface.getAddress8bit(), 0xFFu);

    // Chip 7, address 0x80
    memInterface.setAddress(0x0780);
    EXPECT_EQ(memInterface.getChipSelect(), 0x07u);
    EXPECT_EQ(memInterface.getAddress8bit(), 0x80u);
}

// Test sequential address access
TEST_F(K4289Test, SequentialAccess) {
    for (uint16_t addr = 0; addr < 256; ++addr) {
        memInterface.setAddress(addr);
        EXPECT_EQ(memInterface.getAddress8bit(), static_cast<uint8_t>(addr));
        EXPECT_EQ(memInterface.getChipSelect(), 0x00u);
    }
}

// Test full 4KB address space
TEST_F(K4289Test, FullAddressSpace) {
    // 4KB = 4096 bytes = 0x000 to 0xFFF (12-bit)
    memInterface.setAddress(0x0000);  // Start
    EXPECT_EQ(memInterface.getAddress(), 0x0000u);

    memInterface.setAddress(0x0FFF);  // End
    EXPECT_EQ(memInterface.getAddress(), 0x0FFFu);

    // Each chip has 256 bytes
    for (uint8_t chip = 0; chip < 16; ++chip) {
        memInterface.setAddress((chip << 8) | 0x00);
        EXPECT_EQ(memInterface.getChipSelect(), chip);
        EXPECT_EQ(memInterface.getAddress8bit(), 0x00u);

        memInterface.setAddress((chip << 8) | 0xFF);
        EXPECT_EQ(memInterface.getChipSelect(), chip);
        EXPECT_EQ(memInterface.getAddress8bit(), 0xFFu);
    }
}
