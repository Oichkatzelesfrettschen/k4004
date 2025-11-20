#include <gtest/gtest.h>
#include "emulator_core/source/Intel8255.hpp"

class Intel8255Test : public ::testing::Test {
protected:
    Intel8255 ppi;

    void SetUp() override {
        ppi.setChipSelect(true);  // Enable chip for tests
    }
};

// ============================================================================
// Construction and Reset Tests
// ============================================================================

TEST_F(Intel8255Test, Construction) {
    Intel8255 newPpi;
    newPpi.setChipSelect(true);

    // Default: all ports input
    EXPECT_TRUE(newPpi.isPortAInput());
    EXPECT_TRUE(newPpi.isPortBInput());
    EXPECT_TRUE(newPpi.isPortCUpperInput());
    EXPECT_TRUE(newPpi.isPortCLowerInput());

    // Default mode 0
    EXPECT_EQ(newPpi.getGroupAMode(), Intel8255::Mode::MODE_0);
    EXPECT_EQ(newPpi.getGroupBMode(), Intel8255::Mode::MODE_0);
}

TEST_F(Intel8255Test, Reset) {
    // Configure something
    ppi.write(Intel8255::CONTROL, 0x80);  // All output
    ppi.write(Intel8255::PORT_A, 0xAA);

    // Reset
    ppi.reset();
    ppi.setChipSelect(true);

    // Should be back to default (all input)
    EXPECT_TRUE(ppi.isPortAInput());
    EXPECT_TRUE(ppi.isPortBInput());
}

TEST_F(Intel8255Test, ChipSelectControl) {
    ppi.setChipSelect(false);

    // Writes should be ignored when CS is false
    ppi.write(Intel8255::PORT_A, 0xFF);

    ppi.setChipSelect(true);
    ppi.write(Intel8255::CONTROL, 0x80);  // All output

    EXPECT_EQ(ppi.read(Intel8255::PORT_A), 0x00u);  // Not 0xFF
}

// ============================================================================
// Mode 0 Basic I/O Tests
// ============================================================================

TEST_F(Intel8255Test, Mode0AllOutput) {
    // Control word: Mode 0, all output
    // Bit pattern: 1000 0000
    ppi.write(Intel8255::CONTROL, 0x80);

    EXPECT_FALSE(ppi.isPortAInput());
    EXPECT_FALSE(ppi.isPortBInput());
    EXPECT_FALSE(ppi.isPortCUpperInput());
    EXPECT_FALSE(ppi.isPortCLowerInput());

    // Write to ports
    ppi.write(Intel8255::PORT_A, 0xAA);
    ppi.write(Intel8255::PORT_B, 0x55);
    ppi.write(Intel8255::PORT_C, 0xF0);

    // Read back (output mode returns latched values)
    EXPECT_EQ(ppi.read(Intel8255::PORT_A), 0xAAu);
    EXPECT_EQ(ppi.read(Intel8255::PORT_B), 0x55u);
    EXPECT_EQ(ppi.read(Intel8255::PORT_C), 0xF0u);
}

TEST_F(Intel8255Test, Mode0AllInput) {
    // Control word: Mode 0, all input
    // Bit pattern: 1001 1011
    ppi.write(Intel8255::CONTROL, 0x9B);

    EXPECT_TRUE(ppi.isPortAInput());
    EXPECT_TRUE(ppi.isPortBInput());
    EXPECT_TRUE(ppi.isPortCUpperInput());
    EXPECT_TRUE(ppi.isPortCLowerInput());

    // Set external values
    ppi.setExternalPortA(0x12);
    ppi.setExternalPortB(0x34);
    ppi.setExternalPortC(0x56);

    // Read should return external values
    EXPECT_EQ(ppi.read(Intel8255::PORT_A), 0x12u);
    EXPECT_EQ(ppi.read(Intel8255::PORT_B), 0x34u);
    EXPECT_EQ(ppi.read(Intel8255::PORT_C), 0x56u);
}

TEST_F(Intel8255Test, Mode0MixedDirection) {
    // Port A: output, Port B: input, Port C: mixed
    // Bit pattern: 1000 0010
    ppi.write(Intel8255::CONTROL, 0x82);

    EXPECT_FALSE(ppi.isPortAInput());   // Output
    EXPECT_TRUE(ppi.isPortBInput());     // Input
    EXPECT_FALSE(ppi.isPortCUpperInput()); // Output
    EXPECT_FALSE(ppi.isPortCLowerInput()); // Output

    // Write to Port A (output)
    ppi.write(Intel8255::PORT_A, 0xAB);
    EXPECT_EQ(ppi.read(Intel8255::PORT_A), 0xABu);

    // Port B is input - set external
    ppi.setExternalPortB(0xCD);
    EXPECT_EQ(ppi.read(Intel8255::PORT_B), 0xCDu);
}

TEST_F(Intel8255Test, PortCSplitDirection) {
    // Port C upper: input, Port C lower: output
    // Bit pattern: 1000 1000
    ppi.write(Intel8255::CONTROL, 0x88);

    EXPECT_TRUE(ppi.isPortCUpperInput());   // Input (bit 3 = 1)
    EXPECT_FALSE(ppi.isPortCLowerInput());  // Output (bit 0 = 0)

    // Write to Port C
    ppi.write(Intel8255::PORT_C, 0xAB);

    // Set external upper nibble
    ppi.setExternalPortC(0xF0);

    // Read: upper from external, lower from latch
    uint8_t result = ppi.read(Intel8255::PORT_C);
    EXPECT_EQ(result & 0xF0, 0xF0u);  // Upper from external
    EXPECT_EQ(result & 0x0F, 0x0Bu);  // Lower from write
}

// ============================================================================
// Control Word Tests
// ============================================================================

TEST_F(Intel8255Test, ControlWordParsing) {
    // Test various control words

    // All output: 0x80
    ppi.write(Intel8255::CONTROL, 0x80);
    EXPECT_EQ(ppi.getControlWord(), 0x80u);
    EXPECT_FALSE(ppi.isPortAInput());
    EXPECT_FALSE(ppi.isPortBInput());

    // All input: 0x9B
    ppi.write(Intel8255::CONTROL, 0x9B);
    EXPECT_EQ(ppi.getControlWord(), 0x9Bu);
    EXPECT_TRUE(ppi.isPortAInput());
    EXPECT_TRUE(ppi.isPortBInput());
}

TEST_F(Intel8255Test, ReadControlWord) {
    ppi.write(Intel8255::CONTROL, 0x90);
    EXPECT_EQ(ppi.read(Intel8255::CONTROL), 0x90u);
}

// ============================================================================
// Bit Set/Reset Tests
// ============================================================================

TEST_F(Intel8255Test, BitSetResetPortC) {
    // Configure all output
    ppi.write(Intel8255::CONTROL, 0x80);

    // Clear Port C
    ppi.write(Intel8255::PORT_C, 0x00);

    // Set bit 3 (control: 0000 0111 = bit 3, set)
    ppi.write(Intel8255::CONTROL, 0x07);
    EXPECT_EQ(ppi.read(Intel8255::PORT_C), 0x08u);

    // Set bit 7 (control: 0000 1111 = bit 7, set)
    ppi.write(Intel8255::CONTROL, 0x0F);
    EXPECT_EQ(ppi.read(Intel8255::PORT_C), 0x88u);

    // Reset bit 3 (control: 0000 0110 = bit 3, reset)
    ppi.write(Intel8255::CONTROL, 0x06);
    EXPECT_EQ(ppi.read(Intel8255::PORT_C), 0x80u);

    // Reset bit 7 (control: 0000 1110 = bit 7, reset)
    ppi.write(Intel8255::CONTROL, 0x0E);
    EXPECT_EQ(ppi.read(Intel8255::PORT_C), 0x00u);
}

TEST_F(Intel8255Test, BitSetResetIndividualBits) {
    ppi.write(Intel8255::CONTROL, 0x80);  // All output
    ppi.write(Intel8255::PORT_C, 0x00);

    // Set each bit individually
    for (uint8_t bit = 0; bit < 8; ++bit) {
        uint8_t setBits = (bit << 1) | 0x01;  // Bit select + set
        ppi.write(Intel8255::CONTROL, setBits);
        EXPECT_EQ(ppi.read(Intel8255::PORT_C), (1u << bit));

        // Reset the bit
        uint8_t resetBits = (bit << 1) | 0x00;  // Bit select + reset
        ppi.write(Intel8255::CONTROL, resetBits);
        EXPECT_EQ(ppi.read(Intel8255::PORT_C), 0u);
    }
}

// ============================================================================
// External Device Interface Tests
// ============================================================================

TEST_F(Intel8255Test, ExternalInputDevices) {
    // All input mode
    ppi.write(Intel8255::CONTROL, 0x9B);

    // Simulate external devices driving pins
    ppi.setExternalPortA(0xDE);
    ppi.setExternalPortB(0xAD);
    ppi.setExternalPortC(0xBE);

    // CPU reads should see external values
    EXPECT_EQ(ppi.read(Intel8255::PORT_A), 0xDEu);
    EXPECT_EQ(ppi.read(Intel8255::PORT_B), 0xADu);
    EXPECT_EQ(ppi.read(Intel8255::PORT_C), 0xBEu);
}

TEST_F(Intel8255Test, ExternalOutputDevices) {
    // All output mode
    ppi.write(Intel8255::CONTROL, 0x80);

    // CPU writes to ports
    ppi.write(Intel8255::PORT_A, 0xCA);
    ppi.write(Intel8255::PORT_B, 0xFE);
    ppi.write(Intel8255::PORT_C, 0xBA);

    // External devices should see these values
    EXPECT_EQ(ppi.getExternalPortA(), 0xCAu);
    EXPECT_EQ(ppi.getExternalPortB(), 0xFEu);
    EXPECT_EQ(ppi.getExternalPortC(), 0xBAu);
}

TEST_F(Intel8255Test, MixedInputOutput) {
    // Port A: output, Port B: input
    ppi.write(Intel8255::CONTROL, 0x82);

    // CPU writes to Port A
    ppi.write(Intel8255::PORT_A, 0x12);

    // External device drives Port B
    ppi.setExternalPortB(0x34);

    // Verify
    EXPECT_EQ(ppi.getExternalPortA(), 0x12u);  // Output from CPU
    EXPECT_EQ(ppi.read(Intel8255::PORT_B), 0x34u);  // Input to CPU
}

// ============================================================================
// Address Decoding Tests
// ============================================================================

TEST_F(Intel8255Test, AddressDecoding) {
    ppi.write(Intel8255::CONTROL, 0x80);  // All output

    // Address 0: Port A
    ppi.write(0, 0x11);
    EXPECT_EQ(ppi.read(0), 0x11u);

    // Address 1: Port B
    ppi.write(1, 0x22);
    EXPECT_EQ(ppi.read(1), 0x22u);

    // Address 2: Port C
    ppi.write(2, 0x33);
    EXPECT_EQ(ppi.read(2), 0x33u);

    // Address 3: Control
    uint8_t control = ppi.read(3);
    EXPECT_EQ(control, 0x80u);
}

TEST_F(Intel8255Test, WriteIgnoredOnInput) {
    // Port A as input
    ppi.write(Intel8255::CONTROL, 0x90);

    // Try to write to Port A
    ppi.write(Intel8255::PORT_A, 0xFF);

    // Set external value
    ppi.setExternalPortA(0x42);

    // Read should return external value, not written value
    EXPECT_EQ(ppi.read(Intel8255::PORT_A), 0x42u);
}

TEST_F(Intel8255Test, TypicalParallelPrinter) {
    // Typical printer interface:
    // Port A: data output (8 bits)
    // Port B: status input (8 bits)
    // Port C: control signals (mixed)

    ppi.write(Intel8255::CONTROL, 0x82);  // A=out, B=in, C=out

    // Send data byte to printer
    ppi.write(Intel8255::PORT_A, 0x41);  // 'A'

    // Simulate printer status
    ppi.setExternalPortB(0x80);  // Bit 7 = busy

    // Read status
    uint8_t status = ppi.read(Intel8255::PORT_B);
    EXPECT_EQ(status, 0x80u);

    // Assert strobe on Port C
    ppi.write(Intel8255::PORT_C, 0x01);  // Strobe bit
    EXPECT_EQ(ppi.getExternalPortC() & 0x01, 0x01u);
}
