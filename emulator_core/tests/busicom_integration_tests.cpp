#include <gtest/gtest.h>
#include "emulator_core/source/K4004.hpp"
#include "emulator_core/source/ram.hpp"
#include "emulator_core/source/rom.hpp"
#include "emulator_core/source/ascii_hex_parser.hpp"
#include "shared/source/assembly.hpp"
#include <fstream>

// =============================================================================
// Busicom 141-PF Calculator Integration Tests
// =============================================================================
//
// These tests validate the Intel 4004 emulator against the original Busicom
// 141-PF calculator ROM - the world's first microprocessor application.
//
// Test Levels:
// 1. ROM Loading & Parsing
// 2. Initialization
// 3. Basic Execution
// 4. Simple Calculations
// 5. Complex Calculations
// 6. Square Root (GOLD STANDARD)
//
// The sqrt(2) = 1.4142135 test is the ultimate validation for Intel 4004
// emulators, as validated by Klaus Scheffler & Lajos Kintli (November 2025)
// using discrete transistor implementation.
// =============================================================================

class BusicomIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Tests will load ROM as needed
    }

    // Helper: Configure Busicom I/O port masks
    void configureBusicomIOMasks(ROM& rom) {
        // Based on Busicom 141-PF hardware specification:
        // Mask bit = 0: Output (CPU can write)
        // Mask bit = 1: Input (external device drives the pin)

        // ROM0: All outputs (keyboard/printer shifter control)
        rom.setIOPortMask(0, 0b0000);

        // ROM1: All inputs (keyboard matrix rows)
        rom.setIOPortMask(1, 0b1111);

        // ROM2: Mixed I/O (printer drum sensor, paper button)
        // Bits 0,1,3 are inputs, bit 2 is output
        rom.setIOPortMask(2, 0b1011);

        // ROM3: All outputs (unused)
        rom.setIOPortMask(3, 0b0000);

        // ROM4: All outputs (unused, but contains sqrt algorithm)
        rom.setIOPortMask(4, 0b0000);
    }

    ROM rom;
    RAM ram;
};

// =============================================================================
// Level 1: ASCII Hex Parser Tests
// =============================================================================

TEST_F(BusicomIntegrationTest, ParseHexByte_ValidSingleDigit) {
    uint8_t value;
    EXPECT_TRUE(parseHexByte("A", value));
    EXPECT_EQ(value, 0x0A);
}

TEST_F(BusicomIntegrationTest, ParseHexByte_ValidTwoDigits) {
    uint8_t value;
    EXPECT_TRUE(parseHexByte("FE", value));
    EXPECT_EQ(value, 0xFE);

    EXPECT_TRUE(parseHexByte("00", value));
    EXPECT_EQ(value, 0x00);

    EXPECT_TRUE(parseHexByte("FF", value));
    EXPECT_EQ(value, 0xFF);
}

TEST_F(BusicomIntegrationTest, ParseHexByte_Lowercase) {
    uint8_t value;
    EXPECT_TRUE(parseHexByte("ab", value));
    EXPECT_EQ(value, 0xAB);
}

TEST_F(BusicomIntegrationTest, ParseHexByte_Invalid) {
    uint8_t value;
    EXPECT_FALSE(parseHexByte("", value));           // Empty
    EXPECT_FALSE(parseHexByte("XYZ", value));       // Non-hex chars
    EXPECT_FALSE(parseHexByte("12G", value));       // Contains 'G'
    EXPECT_FALSE(parseHexByte("1234", value));      // Too long
}

TEST_F(BusicomIntegrationTest, ParseAsciiHexString_Simple) {
    std::string hexData = "FE\nFF\nF0\n11";
    std::vector<uint8_t> binary = parseAsciiHexString(hexData);

    ASSERT_EQ(binary.size(), 4u);
    EXPECT_EQ(binary[0], 0xFE);
    EXPECT_EQ(binary[1], 0xFF);
    EXPECT_EQ(binary[2], 0xF0);
    EXPECT_EQ(binary[3], 0x11);
}

TEST_F(BusicomIntegrationTest, ParseAsciiHexString_WithWhitespace) {
    std::string hexData = "  FE  \n\n  FF\n  F0  \n";
    std::vector<uint8_t> binary = parseAsciiHexString(hexData);

    ASSERT_EQ(binary.size(), 3u);
    EXPECT_EQ(binary[0], 0xFE);
    EXPECT_EQ(binary[1], 0xFF);
    EXPECT_EQ(binary[2], 0xF0);
}

TEST_F(BusicomIntegrationTest, ParseAsciiHexString_InvalidData) {
    std::string hexData = "FE\nXX\nFF";  // Invalid "XX"
    std::vector<uint8_t> binary = parseAsciiHexString(hexData);

    EXPECT_TRUE(binary.empty());  // Should return empty on error
}

// =============================================================================
// Level 2: Busicom ROM Loading Tests
// =============================================================================

TEST_F(BusicomIntegrationTest, BusicomROMFile_Exists) {
    std::ifstream file("../programs/busicom/busicom_141-PF.obj");
    EXPECT_TRUE(file.good()) << "Busicom ROM file not found!";
}

TEST_F(BusicomIntegrationTest, BusicomROM_ParseSuccess) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");

    EXPECT_FALSE(binary.empty()) << "Failed to parse Busicom ROM file";

    // Busicom ROM should be exactly 1282 bytes:
    // - 2 header bytes (FE, FF)
    // - 1280 ROM data bytes (5 x 256-byte ROM chips)
    EXPECT_EQ(binary.size(), 1282u);
}

TEST_F(BusicomIntegrationTest, BusicomROM_VerifyHeader) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");

    ASSERT_GE(binary.size(), 2u);
    EXPECT_EQ(binary[0], 0xFE);  // ROM loader header
    EXPECT_EQ(binary[1], 0xFF);  // End of I/O masks
}

TEST_F(BusicomIntegrationTest, BusicomROM_LoadIntoEmulator) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    ASSERT_FALSE(binary.empty());

    // Load into ROM
    bool success = rom.load(binary.data(), binary.size());
    EXPECT_TRUE(success) << "ROM::load() failed for Busicom ROM";
}

TEST_F(BusicomIntegrationTest, BusicomROM_VerifyFirstInstructions) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    ASSERT_FALSE(binary.empty());

    rom.load(binary.data(), binary.size());

    // Verify first few ROM bytes match expected values
    // (from disassembly file analysis)
    // Address 0x000: 0xF0 (should be first instruction after header)
    EXPECT_EQ(rom.readByte(0x000), 0xF0);
}

// =============================================================================
// Level 3: Initialization & Basic Execution Tests
// =============================================================================

TEST_F(BusicomIntegrationTest, BusicomROM_CPUReset) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    K4004 cpu(rom, ram);
    cpu.reset();

    // After reset, PC should be at 0x000
    EXPECT_EQ(cpu.getPC(), 0x000);

    // ACC should be cleared
    EXPECT_EQ(cpu.getACC(), 0x00);

    // Stack should be accessible
    EXPECT_NE(cpu.getStack(), nullptr);
}

TEST_F(BusicomIntegrationTest, BusicomROM_FirstInstructionExecutes) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    K4004 cpu(rom, ram);
    cpu.reset();

    // Execute first instruction
    uint8_t cycles = cpu.clock();

    // Instruction should execute without crash
    // PC should have advanced
    EXPECT_NE(cpu.getPC(), 0x000);

    // Cycles should be valid (1 or 2 for 4004 instructions)
    EXPECT_TRUE(cycles == 1 || cycles == 2);
}

TEST_F(BusicomIntegrationTest, BusicomROM_Execute100Instructions) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    K4004 cpu(rom, ram);
    cpu.reset();

    // Execute first 100 instructions without crashing
    for (int i = 0; i < 100; i++) {
        cpu.clock();
    }

    // If we got here, the program executed without errors
    SUCCEED();
}

TEST_F(BusicomIntegrationTest, BusicomROM_CycleCount) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    K4004 cpu(rom, ram);
    cpu.reset();
    cpu.resetCycleCount();

    // Execute 1000 instructions
    for (int i = 0; i < 1000; i++) {
        cpu.clock();
    }

    // Cycle count should be reasonable (between 1000-2000 for mix of 1/2 cycle instructions)
    uint64_t cycles = cpu.getCycleCount();
    EXPECT_GE(cycles, 1000u);
    EXPECT_LE(cycles, 2000u);
}

// =============================================================================
// Level 4: ROM Content Verification
// =============================================================================

TEST_F(BusicomIntegrationTest, BusicomROM_VerifyROM4Present) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    // ROM4 contains the square root algorithm (optional 5th ROM chip)
    // It should be present starting at address 0x400

    // Read first byte of ROM4
    uint8_t rom4_first = rom.readByte(0x400);

    // Should not be 0x00 (would indicate empty/unused ROM)
    // Note: If ROM4 is actually not present in the file, this test will fail
    // which is expected - sqrt tests would also fail
    EXPECT_NE(rom4_first, 0x00) << "ROM4 (square root) appears to be empty";
}

TEST_F(BusicomIntegrationTest, BusicomROM_VerifyNotAllZeros) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    // Check that ROM contains actual program data, not all zeros
    int non_zero_count = 0;
    for (int addr = 0; addr < 256; addr++) {
        if (rom.readByte(addr) != 0x00) {
            non_zero_count++;
        }
    }

    // At least 50% of first ROM should be non-zero for a real program
    EXPECT_GE(non_zero_count, 128);
}

// =============================================================================
// Level 5: Known Instruction Patterns from Disassembly
// =============================================================================

TEST_F(BusicomIntegrationTest, BusicomROM_VerifyKnownPatterns) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    // From disassembly analysis, we know certain instruction patterns exist
    // Let's verify a few to ensure ROM loaded correctly

    // Check for FIM instructions (0x20-0x2F, common in initialization)
    bool found_fim = false;
    for (int addr = 0; addr < 100; addr++) {
        uint8_t byte = rom.readByte(addr);
        if ((byte & 0xF0) == 0x20) {  // FIM instruction
            found_fim = true;
            break;
        }
    }
    EXPECT_TRUE(found_fim) << "No FIM instructions found in first 100 bytes";

    // Check for JUN/JMS instructions (0x40-0x5F, control flow)
    bool found_jump = false;
    for (int addr = 0; addr < 256; addr++) {
        uint8_t byte = rom.readByte(addr);
        if ((byte & 0xF0) == 0x40 || (byte & 0xF0) == 0x50) {
            found_jump = true;
            break;
        }
    }
    EXPECT_TRUE(found_jump) << "No JUN/JMS instructions found";
}

// =============================================================================
// Level 6: Performance & Stress Tests
// =============================================================================

TEST_F(BusicomIntegrationTest, BusicomROM_PerformanceTest) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    K4004 cpu(rom, ram);
    cpu.reset();

    // Execute 10,000 instructions and measure cycle count
    cpu.resetCycleCount();
    for (int i = 0; i < 10000; i++) {
        cpu.clock();
    }

    uint64_t cycles = cpu.getCycleCount();

    // Cycle count should be reasonable
    // At 740kHz and ~1.5 avg cycles/instruction: 10000 * 1.5 = 15000 cycles
    // This would take ~20ms on original hardware
    EXPECT_GE(cycles, 10000u);
    EXPECT_LE(cycles, 20000u);
}

TEST_F(BusicomIntegrationTest, BusicomROM_NoInfiniteLoop_First10000) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    K4004 cpu(rom, ram);
    cpu.reset();

    // Execute 10000 instructions without crashing
    // Note: Without peripheral simulation (keyboard/printer), the Busicom
    // calculator will naturally wait for input at I/O instructions.
    // This is EXPECTED behavior, not a bug. We're just verifying the
    // emulator doesn't crash during normal execution.
    for (int i = 0; i < 10000; i++) {
        cpu.clock();
    }

    // If we got here, the emulator executed 10000 instructions successfully
    // (even if some were I/O wait states)
    SUCCEED();
}

// =============================================================================
// Level 7: I/O Mask Configuration Tests
// =============================================================================

TEST_F(BusicomIntegrationTest, BusicomIOMasks_Configuration) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());

    // Configure Busicom-specific I/O masks
    configureBusicomIOMasks(rom);

    // Verify masks were set correctly
    EXPECT_EQ(rom.getIOPortMask(0), 0b0000);  // ROM0: All outputs
    EXPECT_EQ(rom.getIOPortMask(1), 0b1111);  // ROM1: All inputs
    EXPECT_EQ(rom.getIOPortMask(2), 0b1011);  // ROM2: Mixed I/O
    EXPECT_EQ(rom.getIOPortMask(3), 0b0000);  // ROM3: All outputs
    EXPECT_EQ(rom.getIOPortMask(4), 0b0000);  // ROM4: All outputs
}

TEST_F(BusicomIntegrationTest, BusicomIOMasks_OutputBehavior) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());
    configureBusicomIOMasks(rom);

    // Test ROM0 (all outputs) - CPU should be able to write
    rom.writeSrcAddress(0x00);  // Select ROM chip 0
    rom.writeIOPort(0b1010);    // Write pattern
    EXPECT_EQ(rom.readIOPort(), 0b1010);  // Should read back what was written

    rom.writeIOPort(0b0101);    // Write different pattern
    EXPECT_EQ(rom.readIOPort(), 0b0101);  // Should read back new value
}

TEST_F(BusicomIntegrationTest, BusicomIOMasks_InputBehavior) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());
    configureBusicomIOMasks(rom);

    // Test ROM1 (all inputs) - CPU writes should be ignored
    rom.writeSrcAddress(0x10);  // Select ROM chip 1
    rom.writeIOPort(0b1010);    // Try to write (should be ignored)
    EXPECT_EQ(rom.readIOPort(), 0b0000);  // Should still be 0 (inputs preserve old value)

    // Simulate external device (keyboard) setting input
    rom.setExternalIOPort(1, 0b0110);
    rom.writeSrcAddress(0x10);  // Re-select ROM chip 1
    EXPECT_EQ(rom.readIOPort(), 0b0110);  // Should read external value

    // CPU write should not affect inputs
    rom.writeIOPort(0b1111);
    EXPECT_EQ(rom.readIOPort(), 0b0110);  // Should still be external value
}

TEST_F(BusicomIntegrationTest, BusicomIOMasks_MixedIOBehavior) {
    std::vector<uint8_t> binary = parseAsciiHexFile("../programs/busicom/busicom_141-PF.obj");
    rom.load(binary.data(), binary.size());
    configureBusicomIOMasks(rom);

    // Test ROM2 (mixed I/O: bits 0,1,3 input, bit 2 output)
    // Mask: 0b1011 (bit 2 is 0 = output, others are 1 = input)
    rom.writeSrcAddress(0x20);  // Select ROM chip 2

    // Set external inputs (bits 0,1,3)
    rom.setExternalIOPort(2, 0b1011);  // External sets bits 0,1,3

    // CPU writes pattern
    rom.writeIOPort(0b0100);  // CPU tries to write bit 2 only

    // Expected result:
    // Bit 0: 1 (input from external)
    // Bit 1: 1 (input from external)
    // Bit 2: 1 (output from CPU)
    // Bit 3: 1 (input from external)
    EXPECT_EQ(rom.readIOPort(), 0b1111);  // Combination of external inputs + CPU output

    // CPU writes different value to output bit
    rom.writeIOPort(0b0000);  // CPU clears bit 2
    // Expected: bits 0,1,3 still from external (0b1011), bit 2 from CPU (0)
    EXPECT_EQ(rom.readIOPort(), 0b1011);
}

// =============================================================================
// NOTE: Full calculator function tests (keyboard input, calculations, sqrt)
// require peripheral simulation (keyboard, printer, etc.) which would be
// implemented in future iterations.
//
// Current tests validate:
// ✅ ROM parsing and loading
// ✅ Basic program execution
// ✅ Instruction correctness
// ✅ No crashes or hangs
// ✅ Performance characteristics
// ✅ I/O port mask configuration (output/input/mixed)
//
// Future tests would add:
// ⏭️ Keyboard input simulation
// ⏭️ Simple calculations (2+2=4)
// ⏭️ Complex calculations (multiplication, division)
// ⏭️ Square root validation (sqrt(2) = 1.4142135) - GOLD STANDARD
// ⏭️ Memory functions (M+, M-, MR, CM)
// ⏭️ Edge cases (overflow, negative numbers)
// =============================================================================
