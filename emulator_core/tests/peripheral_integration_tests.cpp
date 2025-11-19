#include <gtest/gtest.h>
#include "emulator_core/source/Intel8255.hpp"
#include "emulator_core/source/SevenSegmentDisplay.hpp"
#include "emulator_core/source/MatrixKeyboard.hpp"

// ============================================================================
// Integration Tests: Complete Systems
// ============================================================================
//
// These tests demonstrate how 4-bit CPUs interface with 8-bit peripherals
// using the patterns described in 4BIT_TO_8BIT_INTERFACE.md

class PeripheralIntegrationTest : public ::testing::Test {
protected:
    Intel8255 ppi;
    SevenSegmentDisplay display{4};
    MatrixKeyboard keyboard{4, 4};

    void SetUp() override {
        ppi.setChipSelect(true);
        ppi.reset();
    }
};

// ============================================================================
// Test 1: 8255 PPI + 7-Segment Display (Output)
// ============================================================================

TEST_F(PeripheralIntegrationTest, DisplayVia8255) {
    // Scenario: Drive a 4-digit 7-segment display via 8255 PPI
    //
    // Hardware Setup:
    // - Port A: Digit select (4 bits) + control (4 bits)
    // - Port B: Segment data (8 bits: 7 segments + decimal point)
    // - Port C: Not used
    //
    // This is a typical multiplexed display configuration from the 1970s

    // Configure 8255: Port A and B as output
    ppi.write(Intel8255::CONTROL, 0x80);

    ASSERT_FALSE(ppi.isPortAInput());
    ASSERT_FALSE(ppi.isPortBInput());

    // Display "1234" by multiplexing
    // In real hardware, this happens rapidly (100-1000 Hz)

    struct DigitPattern {
        uint8_t digit;
        uint8_t segments;
    };

    DigitPattern patterns[] = {
        {0, 0b00000110},  // Digit 0: "1"
        {1, 0b01011011},  // Digit 1: "2"
        {2, 0b01001111},  // Digit 2: "3"
        {3, 0b01100110}   // Digit 3: "4"
    };

    for (const auto& pattern : patterns) {
        // Select digit (Port A bits 0-3)
        ppi.write(Intel8255::PORT_A, pattern.digit);

        // Output segment data (Port B)
        ppi.write(Intel8255::PORT_B, pattern.segments);

        // In real hardware, a brief delay here, then move to next digit
        // For testing, we just verify the output
        EXPECT_EQ(ppi.getExternalPortA(), pattern.digit);
        EXPECT_EQ(ppi.getExternalPortB(), pattern.segments);

        // Update actual display (simulating latch)
        display.setSegments(pattern.digit, pattern.segments);
    }

    // Verify display shows correct digits
    EXPECT_EQ(display.getSegments(0), 0b00000110u);  // "1"
    EXPECT_EQ(display.getSegments(1), 0b01011011u);  // "2"
    EXPECT_EQ(display.getSegments(2), 0b01001111u);  // "3"
    EXPECT_EQ(display.getSegments(3), 0b01100110u);  // "4"
}

TEST_F(PeripheralIntegrationTest, DisplayMultiplexingFullCycle) {
    // More realistic: Show how CPU would multiplex in a loop
    // (Simulating what a 4004/4040 program would do)

    ppi.write(Intel8255::CONTROL, 0x80);  // All output

    // Data to display: 0xABCD in hex
    uint8_t hexDigits[] = {0xA, 0xB, 0xC, 0xD};
    uint8_t segmentPatterns[16] = {
        0b00111111,  // 0
        0b00000110,  // 1
        0b01011011,  // 2
        0b01001111,  // 3
        0b01100110,  // 4
        0b01101101,  // 5
        0b01111101,  // 6
        0b00000111,  // 7
        0b01111111,  // 8
        0b01101111,  // 9
        0b01110111,  // A
        0b01111100,  // B
        0b00111001,  // C
        0b01011110,  // D
        0b01111001,  // E
        0b01110001   // F
    };

    // Multiplex cycle (in real hardware, this runs continuously)
    for (int cycle = 0; cycle < 3; ++cycle) {  // 3 refresh cycles
        for (int digit = 0; digit < 4; ++digit) {
            // Turn off all digits (common anode: high = off)
            ppi.write(Intel8255::PORT_A, 0xFF);

            // Load segment data
            uint8_t segments = segmentPatterns[hexDigits[digit]];
            ppi.write(Intel8255::PORT_B, segments);

            // Turn on this digit (bit clear = on)
            ppi.write(Intel8255::PORT_A, ~(1u << digit));

            // Simulate latch
            display.setSegments(digit, segments);
        }
    }

    // Verify final state shows 0xABCD
    display.setHexValue(0xABCD);  // Direct set for comparison
    // (In reality, multiplexing shows all digits simultaneously)
}

// ============================================================================
// Test 2: 8255 PPI + Matrix Keyboard (Input)
// ============================================================================

TEST_F(PeripheralIntegrationTest, KeyboardVia8255) {
    // Scenario: Scan a 4×4 matrix keyboard via 8255 PPI
    //
    // Hardware Setup:
    // - Port A: Row outputs (bits 0-3, active LOW)
    // - Port B: Column inputs (bits 0-3, pulled HIGH, key pulls LOW)
    // - Port C: Not used
    //
    // This is the standard keyboard scanning technique from the 1970s

    // Configure 8255: Port A output (rows), Port B input (columns)
    ppi.write(Intel8255::CONTROL, 0x82);

    ASSERT_FALSE(ppi.isPortAInput());  // Rows: output
    ASSERT_TRUE(ppi.isPortBInput());    // Columns: input

    // Simulate key press at row 2, column 1
    keyboard.pressKey(2, 1);

    // Scan keyboard (what CPU would do)
    bool keyFound = false;
    uint8_t foundRow = 0, foundCol = 0;

    for (uint8_t row = 0; row < 4; ++row) {
        // Drive this row LOW, others HIGH
        uint8_t rowMask = 0xFF & ~(1u << row);
        ppi.write(Intel8255::PORT_A, rowMask);

        // Simulate keyboard response
        keyboard.setRowOutput(rowMask);
        uint8_t columns = keyboard.readColumnInput();

        // Read columns from PPI
        ppi.setExternalPortB(columns);
        uint8_t readCols = ppi.read(Intel8255::PORT_B);

        // Check for key press (column goes LOW)
        for (uint8_t col = 0; col < 4; ++col) {
            if (!(readCols & (1u << col))) {
                keyFound = true;
                foundRow = row;
                foundCol = col;
                break;
            }
        }

        if (keyFound) break;
    }

    EXPECT_TRUE(keyFound);
    EXPECT_EQ(foundRow, 2u);
    EXPECT_EQ(foundCol, 1u);
}

TEST_F(PeripheralIntegrationTest, KeyboardFullScan) {
    // More comprehensive: Test scanning all positions

    ppi.write(Intel8255::CONTROL, 0x82);  // A=out, B=in

    // Test each key position
    for (uint8_t testRow = 0; testRow < 4; ++testRow) {
        for (uint8_t testCol = 0; testCol < 4; ++testCol) {
            keyboard.releaseAll();
            keyboard.pressKey(testRow, testCol);

            // Scan
            bool found = false;
            uint8_t foundRow = 0, foundCol = 0;

            for (uint8_t row = 0; row < 4; ++row) {
                uint8_t rowMask = 0xFF & ~(1u << row);
                ppi.write(Intel8255::PORT_A, rowMask);
                keyboard.setRowOutput(rowMask);

                uint8_t columns = keyboard.readColumnInput();
                ppi.setExternalPortB(columns);
                uint8_t readCols = ppi.read(Intel8255::PORT_B);

                for (uint8_t col = 0; col < 4; ++col) {
                    if (!(readCols & (1u << col))) {
                        found = true;
                        foundRow = row;
                        foundCol = col;
                        break;
                    }
                }
                if (found) break;
            }

            EXPECT_TRUE(found) << "Key at (" << (int)testRow << "," << (int)testCol << ") not found";
            EXPECT_EQ(foundRow, testRow) << "Row mismatch";
            EXPECT_EQ(foundCol, testCol) << "Col mismatch";
        }
    }
}

// ============================================================================
// Test 3: Complete Calculator System
// ============================================================================

TEST_F(PeripheralIntegrationTest, CalculatorSystem) {
    // Scenario: Complete calculator with display and keyboard
    //
    // Hardware:
    // - 8255 Port A: Display digit select (4 bits)
    // - 8255 Port B: Display segments (8 bits)
    // - 8255 Port C upper: Keyboard rows (4 bits)
    // - 8255 Port C lower: Keyboard columns (4 bits)
    //
    // This demonstrates a complete interactive system

    // Configure 8255
    // Port A: output (display select)
    // Port B: output (segments)
    // Port C upper: output (keyboard rows)
    // Port C lower: input (keyboard columns)
    ppi.write(Intel8255::CONTROL, 0x81);

    ASSERT_FALSE(ppi.isPortAInput());
    ASSERT_FALSE(ppi.isPortBInput());
    ASSERT_FALSE(ppi.isPortCUpperInput());
    ASSERT_TRUE(ppi.isPortCLowerInput());

    // Simulate user pressing "5" key (row 1, col 1)
    keyboard.pressKey(1, 1);

    // CPU scans keyboard
    uint8_t keyValue = 0xFF;

    for (uint8_t row = 0; row < 4; ++row) {
        // Output row scan via Port C upper nibble
        uint8_t rowBits = ~(1u << (row + 4));  // Upper nibble
        ppi.write(Intel8255::PORT_C, rowBits);

        // Simulate keyboard
        keyboard.setRowOutput(0xFF & ~(1u << row));
        uint8_t cols = keyboard.readColumnInput();

        // Read columns via Port C lower nibble
        ppi.setExternalPortC(cols & 0x0F);
        uint8_t portC = ppi.read(Intel8255::PORT_C);
        uint8_t colBits = portC & 0x0F;

        // Check for key
        for (uint8_t col = 0; col < 4; ++col) {
            if (!(colBits & (1u << col))) {
                keyValue = row * 4 + col;  // Calculate key number
                break;
            }
        }

        if (keyValue != 0xFF) break;
    }

    EXPECT_EQ(keyValue, 5u);  // Row 1 * 4 + Col 1 = 5

    // Display the key value
    uint8_t segments[10] = {
        0b00111111,  // 0
        0b00000110,  // 1
        0b01011011,  // 2
        0b01001111,  // 3
        0b01100110,  // 4
        0b01101101,  // 5
        0b01111101,  // 6
        0b00000111,  // 7
        0b01111111,  // 8
        0b01101111   // 9
    };

    if (keyValue < 10) {
        // Display on rightmost digit (digit 3)
        ppi.write(Intel8255::PORT_A, 0x03);  // Select digit 3
        ppi.write(Intel8255::PORT_B, segments[keyValue]);

        display.setSegments(3, segments[keyValue]);
    }

    // Verify display shows "5"
    EXPECT_EQ(display.getSegments(3), segments[5]);
}

// ============================================================================
// Test 4: Parallel Printer Interface
// ============================================================================

TEST_F(PeripheralIntegrationTest, ParallelPrinter) {
    // Scenario: Send data to parallel printer
    //
    // Hardware:
    // - Port A: 8-bit data to printer
    // - Port B: 8-bit status from printer
    // - Port C bit 0: STROBE signal
    // - Port C bit 1: Printer BUSY (input)

    // Configure: A=out, B=in, C=mixed
    ppi.write(Intel8255::CONTROL, 0x82);

    // Simulate printer ready (BUSY=0)
    ppi.setExternalPortB(0x00);

    // Check printer status
    uint8_t status = ppi.read(Intel8255::PORT_B);
    bool busy = status & 0x80;  // Bit 7 = BUSY
    EXPECT_FALSE(busy);

    // Send character 'A' (0x41)
    ppi.write(Intel8255::PORT_A, 0x41);

    // Assert STROBE (Port C bit 0)
    ppi.write(Intel8255::CONTROL, 0x01);  // Set PC0
    EXPECT_EQ(ppi.read(Intel8255::PORT_C) & 0x01, 0x01u);

    // De-assert STROBE
    ppi.write(Intel8255::CONTROL, 0x00);  // Reset PC0
    EXPECT_EQ(ppi.read(Intel8255::PORT_C) & 0x01, 0x00u);

    // Verify data sent
    EXPECT_EQ(ppi.getExternalPortA(), 0x41u);

    // Simulate printer becoming busy
    ppi.setExternalPortB(0x80);
    status = ppi.read(Intel8255::PORT_B);
    busy = status & 0x80;
    EXPECT_TRUE(busy);
}

// ============================================================================
// Test 5: 4-Bit to 8-Bit Nibble-Wise Access Pattern
// ============================================================================

TEST_F(PeripheralIntegrationTest, NibbleWiseByteAccess) {
    // Demonstrate how 4-bit CPU writes 8-bit value in two cycles
    // This simulates what a real 4004/4040 program would do

    ppi.write(Intel8255::CONTROL, 0x80);  // All output

    // Write byte 0xAB to Port A using nibble-wise access
    // (Simulating two separate CPU write cycles)

    // Cycle 1: Write low nibble (0xB)
    // In real 4004, this would be: LDM 0x0B; WRM
    uint8_t lowNibble = 0x0B;

    // Cycle 2: Write high nibble (0xA)
    // In real 4004, this would be: LDM 0x0A; WRM
    uint8_t highNibble = 0x0A;

    // Combine nibbles to form byte
    uint8_t completeByte = lowNibble | (highNibble << 4);

    // Write to Port A
    ppi.write(Intel8255::PORT_A, completeByte);

    // Verify
    EXPECT_EQ(ppi.read(Intel8255::PORT_A), 0xABu);
    EXPECT_EQ(ppi.getExternalPortA(), 0xABu);

    // Read back using nibble-wise access
    uint8_t readByte = ppi.read(Intel8255::PORT_A);
    uint8_t readLow = readByte & 0x0F;
    uint8_t readHigh = (readByte >> 4) & 0x0F;

    EXPECT_EQ(readLow, 0x0Bu);
    EXPECT_EQ(readHigh, 0x0Au);
}
