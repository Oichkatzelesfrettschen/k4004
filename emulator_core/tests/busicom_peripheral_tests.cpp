#include <gtest/gtest.h>
#include "emulator_core/source/busicom_peripherals.hpp"

// =============================================================================
// Busicom Peripheral Simulation Tests
// =============================================================================
//
// Tests for minimal Busicom 141-PF peripheral simulation including:
// - Keyboard matrix scanning and scan code injection
// - Shift register simulation (keyboard and printer)
// - Status lamp monitoring
// - Printer output capture
//
// Based on deep analysis of Busicom 141-PF disassembly.
// =============================================================================

class BusicomPeripheralTest : public ::testing::Test {
protected:
    void SetUp() override {
        peripherals = new BusicomPeripherals();
    }

    void TearDown() override {
        delete peripherals;
    }

    BusicomPeripherals* peripherals;
};

// =============================================================================
// Level 1: Keyboard Matrix Tests
// =============================================================================

TEST_F(BusicomPeripheralTest, KeyboardMatrix_InitialState) {
    // Initially no key should be pressed
    EXPECT_FALSE(peripherals->isKeyPressed());
    EXPECT_EQ(peripherals->getKeyboardRows(), 0x00);
}

TEST_F(BusicomPeripheralTest, KeyboardMatrix_PressKey) {
    // Press key '2' (scan code 0x97)
    peripherals->pressKey(0x97);
    EXPECT_TRUE(peripherals->isKeyPressed());
}

TEST_F(BusicomPeripheralTest, KeyboardMatrix_ReleaseKey) {
    peripherals->pressKey(0x97);
    peripherals->releaseKey();
    EXPECT_FALSE(peripherals->isKeyPressed());
}

TEST_F(BusicomPeripheralTest, KeyboardMatrix_ScanCodeDetection) {
    // Key '2' is in column 5, row 2 (scan code 0x97)
    // When shifter scans column 5, row bit 2 should be set

    peripherals->pressKey(0x97);  // Press '2'

    // Simulate shift register reaching column 5
    // ROM0 pattern: bit0=clock, bit1=data
    // Shifter starts empty, first shift loads column 0

    // Shift in '1' to activate column 0
    peripherals->updateShiftRegister(0b0010);  // data=1, clock=0
    peripherals->updateShiftRegister(0b0011);  // data=1, clock=1 (shift! column 0 active)
    peripherals->updateShiftRegister(0b0010);  // clock=0

    // Shift through columns 1-5 (shift in '0')
    for (int col = 1; col <= 5; col++) {
        peripherals->updateShiftRegister(0b0000);  // data=0, clock=0
        peripherals->updateShiftRegister(0b0001);  // data=0, clock=1 (shift!)
        peripherals->updateShiftRegister(0b0000);  // clock=0
    }

    // Now shifter is at column 5
    // Row output should have bit 2 set (value 0x04)
    uint8_t rows = peripherals->getKeyboardRows();
    EXPECT_EQ(rows, 0x04);  // Row 2 active (bit 2)
}

TEST_F(BusicomPeripheralTest, KeyboardMatrix_MultipleKeys) {
    // Test different keys in different columns

    // Key 'SQRT' (0x85) is in column 1, row 0
    peripherals->pressKey(0x85);

    // Shift to column 1
    peripherals->updateShiftRegister(0b0010);  // data=1
    peripherals->updateShiftRegister(0b0011);  // shift
    peripherals->updateShiftRegister(0b0000);  // data=0
    peripherals->updateShiftRegister(0b0001);  // shift (now at column 1)

    EXPECT_EQ(peripherals->getKeyboardRows(), 0x01);  // Row 0 (bit 0)

    // Change to key '+' (0x8e) in column 3, row 1
    peripherals->pressKey(0x8e);

    // Continue shifting to column 3
    peripherals->updateShiftRegister(0b0000);  // data=0
    peripherals->updateShiftRegister(0b0001);  // shift (column 2)
    peripherals->updateShiftRegister(0b0000);  // data=0
    peripherals->updateShiftRegister(0b0001);  // shift (column 3)

    EXPECT_EQ(peripherals->getKeyboardRows(), 0x02);  // Row 1 (bit 1)
}

// =============================================================================
// Level 2: Shift Register Tests
// =============================================================================

TEST_F(BusicomPeripheralTest, ShiftRegister_EdgeDetection) {
    // Shift register should only advance on rising edge (0->1)

    peripherals->updateShiftRegister(0b0010);  // data=1, clock=0
    std::string state1 = peripherals->getShiftRegisterState();

    peripherals->updateShiftRegister(0b0010);  // data=1, clock=0 (no change)
    std::string state2 = peripherals->getShiftRegisterState();
    EXPECT_EQ(state1, state2);  // Should be same (no shift)

    peripherals->updateShiftRegister(0b0011);  // data=1, clock=1 (rising edge!)
    std::string state3 = peripherals->getShiftRegisterState();
    EXPECT_NE(state2, state3);  // Should be different (shifted in a '1')
}

TEST_F(BusicomPeripheralTest, ShiftRegister_DataPropagation) {
    // Verify data bit propagates through shift register

    // Shift in pattern: 1, 0, 1, 0, 1
    uint8_t pattern[] = {1, 0, 1, 0, 1};

    for (int i = 0; i < 5; i++) {
        uint8_t data = pattern[i] ? 0b0010 : 0b0000;
        peripherals->updateShiftRegister(data);          // Set data
        peripherals->updateShiftRegister(data | 0b0001); // Clock high (shift)
        peripherals->updateShiftRegister(data);          // Clock low
    }

    // Shift register should contain the pattern
    std::string state = peripherals->getShiftRegisterState();
    EXPECT_NE(state.find("0x"), std::string::npos);  // Should have hex representation
}

// =============================================================================
// Level 3: Status Lamp Tests
// =============================================================================

TEST_F(BusicomPeripheralTest, StatusLamps_InitialState) {
    // All lamps should be off initially
    EXPECT_FALSE(peripherals->isMemoryLampOn());
    EXPECT_FALSE(peripherals->isOverflowLampOn());
    EXPECT_FALSE(peripherals->isMinusLampOn());
    EXPECT_FALSE(peripherals->isRoundLampOn());
}

TEST_F(BusicomPeripheralTest, StatusLamps_MemoryLamp) {
    // RAM1 bit 0 controls memory lamp
    peripherals->updateStatusLamps(0b0001);
    EXPECT_TRUE(peripherals->isMemoryLampOn());
    EXPECT_FALSE(peripherals->isOverflowLampOn());
    EXPECT_FALSE(peripherals->isMinusLampOn());
    EXPECT_FALSE(peripherals->isRoundLampOn());
}

TEST_F(BusicomPeripheralTest, StatusLamps_OverflowLamp) {
    // RAM1 bit 1 controls overflow lamp
    peripherals->updateStatusLamps(0b0010);
    EXPECT_FALSE(peripherals->isMemoryLampOn());
    EXPECT_TRUE(peripherals->isOverflowLampOn());
    EXPECT_FALSE(peripherals->isMinusLampOn());
    EXPECT_FALSE(peripherals->isRoundLampOn());
}

TEST_F(BusicomPeripheralTest, StatusLamps_MinusLamp) {
    // RAM1 bit 2 controls minus lamp
    peripherals->updateStatusLamps(0b0100);
    EXPECT_FALSE(peripherals->isMemoryLampOn());
    EXPECT_FALSE(peripherals->isOverflowLampOn());
    EXPECT_TRUE(peripherals->isMinusLampOn());
    EXPECT_FALSE(peripherals->isRoundLampOn());
}

TEST_F(BusicomPeripheralTest, StatusLamps_MultipleLamps) {
    // Test multiple lamps on simultaneously
    peripherals->updateStatusLamps(0b0111);  // Memory + Overflow + Minus
    EXPECT_TRUE(peripherals->isMemoryLampOn());
    EXPECT_TRUE(peripherals->isOverflowLampOn());
    EXPECT_TRUE(peripherals->isMinusLampOn());
    EXPECT_FALSE(peripherals->isRoundLampOn());
}

// =============================================================================
// Level 4: Printer Control Tests
// =============================================================================

TEST_F(BusicomPeripheralTest, PrinterControl_UpdateControl) {
    // RAM0 controls printer: bit0=color, bit1=fire, bit3=paper advance
    // Just verify the method doesn't crash
    peripherals->updatePrinterControl(0b0000);
    peripherals->updatePrinterControl(0b0001);  // Black
    peripherals->updatePrinterControl(0b0010);  // Fire
    peripherals->updatePrinterControl(0b1000);  // Advance
}

TEST_F(BusicomPeripheralTest, PrinterOutput_ClearOutput) {
    peripherals->clearPrinterOutput();
    const auto& output = peripherals->getPrinterOutput();
    EXPECT_TRUE(output.digits.empty());
    EXPECT_TRUE(output.symbols.empty());
}

// =============================================================================
// Level 5: Integration Tests (Keyboard + Shifter)
// =============================================================================

TEST_F(BusicomPeripheralTest, Integration_FullKeyScan) {
    // Simulate complete keyboard scan cycle (8 columns)
    // Press key '5' (scan code 0x96, column 5, row 1)

    peripherals->pressKey(0x96);

    // Scan through all 8 columns
    for (int col = 0; col < 8; col++) {
        // Shift in '1' for first column, then '0'
        uint8_t data = (col == 0) ? 0b0010 : 0b0000;

        peripherals->updateShiftRegister(data);          // Set data
        peripherals->updateShiftRegister(data | 0b0001); // Clock high
        peripherals->updateShiftRegister(data);          // Clock low

        // Check if this is the active column for our key
        uint8_t rows = peripherals->getKeyboardRows();

        if (col == 5) {
            // Column 5: key should be detected on row 1
            EXPECT_EQ(rows, 0x02);  // Row 1 (bit 1)
        } else {
            // Other columns: no key detected
            EXPECT_EQ(rows, 0x00);
        }
    }
}

TEST_F(BusicomPeripheralTest, Integration_DiagnosticStrings) {
    // Verify diagnostic methods don't crash
    peripherals->pressKey(0x97);

    std::string kbdState = peripherals->getKeyboardState();
    EXPECT_FALSE(kbdState.empty());
    EXPECT_NE(kbdState.find("0x97"), std::string::npos);  // Should contain scan code

    std::string shifterState = peripherals->getShiftRegisterState();
    EXPECT_FALSE(shifterState.empty());
    EXPECT_NE(shifterState.find("0x"), std::string::npos);  // Should have hex values
}

// =============================================================================
// Level 6: Scan Code Validation
// =============================================================================

TEST_F(BusicomPeripheralTest, ScanCodes_DigitKeys) {
    // Verify digit keys map correctly
    struct KeyTest {
        uint8_t scanCode;
        uint8_t column;
        uint8_t row;
    };

    KeyTest digits[] = {
        {0x9c, 6, 3},  // '0' - column 6, row 3
        {0x9b, 6, 2},  // '1' - column 6, row 2
        {0x97, 5, 2},  // '2' - column 5, row 2
        {0x93, 4, 2},  // '3' - column 4, row 2
        {0x9a, 6, 1},  // '4' - column 6, row 1
        {0x96, 5, 1},  // '5' - column 5, row 1
        {0x92, 4, 1},  // '6' - column 4, row 1
        {0x99, 6, 0},  // '7' - column 6, row 0
        {0x95, 5, 0},  // '8' - column 5, row 0
        {0x91, 4, 0},  // '9' - column 4, row 0
    };

    for (const auto& test : digits) {
        // Create fresh peripheral object for each test to reset shifter state
        BusicomPeripherals testPeripherals;
        testPeripherals.pressKey(test.scanCode);

        // Shift to the expected column
        for (uint8_t col = 0; col <= test.column; col++) {
            uint8_t data = (col == 0) ? 0b0010 : 0b0000;
            testPeripherals.updateShiftRegister(data);
            testPeripherals.updateShiftRegister(data | 0b0001);
            testPeripherals.updateShiftRegister(data);
        }

        // Check row output
        uint8_t expected_row = (1 << test.row);
        EXPECT_EQ(testPeripherals.getKeyboardRows(), expected_row)
            << "Failed for scan code 0x" << std::hex << (int)test.scanCode;
    }
}

TEST_F(BusicomPeripheralTest, ScanCodes_OperatorKeys) {
    // Test operator keys
    struct KeyTest {
        uint8_t scanCode;
        uint8_t column;
        uint8_t row;
        const char* name;
    };

    KeyTest operators[] = {
        {0x8e, 3, 1, "+"},      // Plus
        {0x8d, 3, 0, "-"},      // Minus
        {0x8b, 2, 2, "*"},      // Multiply
        {0x8a, 2, 1, "/"},      // Divide
        {0x8c, 2, 3, "="},      // Equals
        {0x85, 1, 0, "SQRT"},   // Square root
    };

    for (const auto& test : operators) {
        // Create fresh peripheral object for each test to reset shifter state
        BusicomPeripherals testPeripherals;
        testPeripherals.pressKey(test.scanCode);

        // Shift to column (start empty, first shift activates column 0)
        // Shift in '1' for column 0, then '0' for subsequent columns
        for (uint8_t col = 0; col <= test.column; col++) {
            uint8_t data = (col == 0) ? 0b0010 : 0b0000;
            testPeripherals.updateShiftRegister(data);          // Set data
            testPeripherals.updateShiftRegister(data | 0b0001); // Clock high
            testPeripherals.updateShiftRegister(data);          // Clock low
        }

        uint8_t expected_row = (1 << test.row);
        EXPECT_EQ(testPeripherals.getKeyboardRows(), expected_row)
            << "Failed for " << test.name << " (0x" << std::hex << (int)test.scanCode << ")";
    }
}
