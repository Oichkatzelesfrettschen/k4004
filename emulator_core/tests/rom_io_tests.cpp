#include <gtest/gtest.h>
#include "emulator_core/source/rom.hpp"

class RomIOTest : public ::testing::Test {
protected:
    ROM rom;

    // Helper to create a minimal valid ROM image
    std::vector<uint8_t> createRomImage(const std::vector<std::pair<uint8_t, uint8_t>>& ioMasks,
                                        const std::vector<uint8_t>& code) {
        std::vector<uint8_t> image;

        // I/O metal mask section header
        image.push_back(0xFE);

        // Add I/O port masks
        for (const auto& mask : ioMasks) {
            image.push_back(mask.first);   // Chip number
            image.push_back(mask.second);  // Mask value
        }

        // End of I/O mask section
        image.push_back(0xFF);

        // Add ROM code
        image.insert(image.end(), code.begin(), code.end());

        return image;
    }
};

// Test I/O port mask loading
TEST_F(RomIOTest, LoadIOMasks) {
    auto image = createRomImage(
        {{0, 0x00}, {1, 0x0F}, {2, 0x05}},  // Chip 0: all output, Chip 1: all input, Chip 2: mixed
        {0x10, 0x20, 0x30}  // Some ROM code
    );

    EXPECT_TRUE(rom.load(image.data(), image.size()));

    EXPECT_EQ(rom.getIOPortMask(0), 0x00);
    EXPECT_EQ(rom.getIOPortMask(1), 0x0F);
    EXPECT_EQ(rom.getIOPortMask(2), 0x05);
}

// Test I/O port write with all output mask (0x00)
TEST_F(RomIOTest, WriteAllOutputPort) {
    auto image = createRomImage({{0, 0x00}}, {0x00});
    rom.load(image.data(), image.size());

    rom.writeSrcAddress(0x00);  // Select chip 0
    rom.writeIOPort(0xA);

    EXPECT_EQ(rom.readIOPort(), 0xA);
    EXPECT_EQ(rom.getIOPort(0), 0xA);
}

// Test I/O port write with all input mask (0x0F)
TEST_F(RomIOTest, WriteAllInputPort) {
    auto image = createRomImage({{0, 0x0F}}, {0x00});
    rom.load(image.data(), image.size());

    rom.writeSrcAddress(0x00);
    rom.writeIOPort(0xA);  // Try to write

    // CPU writes should be ignored for input pins
    EXPECT_EQ(rom.getIOPort(0), 0x0);  // Should remain 0
}

// Test I/O port write with mixed mask
TEST_F(RomIOTest, WriteMixedPort) {
    // Mask 0x05 = 0b0101 = bits 0 and 2 are input, bits 1 and 3 are output
    auto image = createRomImage({{0, 0x05}}, {0x00});
    rom.load(image.data(), image.size());

    rom.writeSrcAddress(0x00);
    rom.writeIOPort(0xF);  // Try to write 0b1111

    // Only bits 1 and 3 should be written (outputs)
    uint8_t value = rom.readIOPort();
    EXPECT_EQ(value & 0x02, 0x02);  // Bit 1 should be set (output)
    EXPECT_EQ(value & 0x08, 0x08);  // Bit 3 should be set (output)
    EXPECT_EQ(value & 0x01, 0x00);  // Bit 0 should be clear (input)
    EXPECT_EQ(value & 0x04, 0x00);  // Bit 2 should be clear (input)
}

// Test external device setting input pins
TEST_F(RomIOTest, ExternalInputPins) {
    // Mask 0x0F = all pins are inputs
    auto image = createRomImage({{0, 0x0F}}, {0x00});
    rom.load(image.data(), image.size());

    // External device sets input value
    rom.setExternalIOPort(0, 0xC);

    rom.writeSrcAddress(0x00);
    EXPECT_EQ(rom.readIOPort(), 0xC);
}

// Test external input doesn't affect output pins
TEST_F(RomIOTest, ExternalInputPreservesOutput) {
    // Mask 0x05 = bits 0,2 are input, bits 1,3 are output
    auto image = createRomImage({{0, 0x05}}, {0x00});
    rom.load(image.data(), image.size());

    rom.writeSrcAddress(0x00);

    // CPU writes to output pins
    rom.writeIOPort(0xA);  // 0b1010

    // External device tries to set all pins
    rom.setExternalIOPort(0, 0xF);

    // Output pins should preserve CPU-written values
    uint8_t value = rom.readIOPort();
    EXPECT_EQ(value & 0x02, 0x02);  // Bit 1 (output) - from CPU
    EXPECT_EQ(value & 0x08, 0x08);  // Bit 3 (output) - from CPU
    EXPECT_EQ(value & 0x01, 0x01);  // Bit 0 (input) - from external
    EXPECT_EQ(value & 0x04, 0x04);  // Bit 2 (input) - from external
}

// Test multiple chips with different masks
TEST_F(RomIOTest, MultipleChipsWithDifferentMasks) {
    auto image = createRomImage(
        {{0, 0x00}, {1, 0x0F}, {2, 0x0A}},
        {0x00}
    );
    rom.load(image.data(), image.size());

    // Chip 0: all output (mask 0x00)
    rom.writeSrcAddress(0x00);
    rom.writeIOPort(0x5);
    EXPECT_EQ(rom.readIOPort(), 0x5);

    // Chip 1: all input (mask 0x0F)
    rom.setExternalIOPort(1, 0x7);
    rom.writeSrcAddress(0x10);
    EXPECT_EQ(rom.readIOPort(), 0x7);

    // Chip 2: mixed (mask 0x0A = 0b1010, bits 1,3 input, bits 0,2 output)
    rom.writeSrcAddress(0x20);
    rom.writeIOPort(0xC);        // CPU writes 0b1100
    rom.setExternalIOPort(2, 0x3); // External sets 0b0011

    uint8_t value = rom.readIOPort();
    EXPECT_EQ(value & 0x01, 0x00);  // Bit 0 (output) - from CPU, masked
    EXPECT_EQ(value & 0x02, 0x02);  // Bit 1 (input) - from external
    EXPECT_EQ(value & 0x04, 0x04);  // Bit 2 (output) - from CPU
    EXPECT_EQ(value & 0x08, 0x00);  // Bit 3 (input) - from external, masked
}

// Test I/O port masking preserves only 4 bits
TEST_F(RomIOTest, FourBitMasking) {
    auto image = createRomImage({{0, 0x00}}, {0x00});
    rom.load(image.data(), image.size());

    rom.writeSrcAddress(0x00);
    rom.writeIOPort(0xFF);  // Write all bits

    EXPECT_EQ(rom.readIOPort(), 0x0F);  // Only lower 4 bits
}

// Test SRC address chip selection
TEST_F(RomIOTest, SrcAddressChipSelection) {
    auto image = createRomImage(
        {{0, 0x00}, {1, 0x00}, {15, 0x00}},
        {0x00}
    );
    rom.load(image.data(), image.size());

    // Write different values to different chips
    rom.writeSrcAddress(0x00);  // Chip 0
    rom.writeIOPort(0x1);

    rom.writeSrcAddress(0x10);  // Chip 1
    rom.writeIOPort(0x2);

    rom.writeSrcAddress(0xF0);  // Chip 15
    rom.writeIOPort(0xF);

    // Read back
    rom.writeSrcAddress(0x00);
    EXPECT_EQ(rom.readIOPort(), 0x1);

    rom.writeSrcAddress(0x10);
    EXPECT_EQ(rom.readIOPort(), 0x2);

    rom.writeSrcAddress(0xF0);
    EXPECT_EQ(rom.readIOPort(), 0xF);
}

// Test typical keyboard input scenario
TEST_F(RomIOTest, KeyboardInputScenario) {
    // All pins configured as inputs for keyboard
    auto image = createRomImage({{0, 0x0F}}, {0x00});
    rom.load(image.data(), image.size());

    rom.writeSrcAddress(0x00);

    // No keys pressed
    rom.setExternalIOPort(0, 0x0);
    EXPECT_EQ(rom.readIOPort(), 0x0);

    // Key 1 pressed (bit 0)
    rom.setExternalIOPort(0, 0x1);
    EXPECT_EQ(rom.readIOPort(), 0x1);

    // Multiple keys pressed
    rom.setExternalIOPort(0, 0x5);  // Bits 0 and 2
    EXPECT_EQ(rom.readIOPort(), 0x5);
}

// Test typical LED output scenario
TEST_F(RomIOTest, LedOutputScenario) {
    // All pins configured as outputs for LEDs
    auto image = createRomImage({{0, 0x00}}, {0x00});
    rom.load(image.data(), image.size());

    rom.writeSrcAddress(0x00);

    // Turn on LEDs in pattern
    rom.writeIOPort(0x5);  // 0b0101
    EXPECT_EQ(rom.readIOPort(), 0x5);

    // Change pattern
    rom.writeIOPort(0xA);  // 0b1010
    EXPECT_EQ(rom.readIOPort(), 0xA);

    // All off
    rom.writeIOPort(0x0);
    EXPECT_EQ(rom.readIOPort(), 0x0);

    // All on
    rom.writeIOPort(0xF);
    EXPECT_EQ(rom.readIOPort(), 0xF);
}
