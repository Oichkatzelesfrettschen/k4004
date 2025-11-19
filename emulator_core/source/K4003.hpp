#pragma once
#include <cstdint>

// Intel 4003 - 10-bit Parallel Output Shift Register
// Package: 16-pin DIP
// Function: Serial-in, parallel-out shift register for I/O expansion
// Typical Use: Keyboard scanning, LED/7-segment display control
//
// Pin Configuration:
// - Serial Data Input (from 4001/4002 I/O port)
// - Clock Input (synchronized with CPU)
// - 10 Parallel Outputs (Q0-Q9)
// - Output Enable (active low)
// - Reset (active low)
//
// Operation:
// - Data shifts in serially from ROM/RAM I/O ports
// - Each clock pulse shifts data one position
// - Parallel outputs update continuously
// - Multiple 4003 chips can be cascaded for more outputs

class K4003
{
public:
    static constexpr uint8_t NUM_OUTPUTS = 10u;

    K4003();

    // Reset the shift register to all zeros
    void reset();

    // Shift in one bit from serial data input
    // Returns the bit shifted out (for cascading)
    uint8_t shiftIn(uint8_t dataBit);

    // Get the current parallel output state
    uint16_t getParallelOutput() const { return m_outputEnabled ? m_shiftRegister : 0u; }

    // Get a specific output bit (0-9)
    uint8_t getOutputBit(uint8_t index) const;

    // Enable/disable parallel outputs
    void setOutputEnable(bool enable) { m_outputEnabled = enable; }
    bool isOutputEnabled() const { return m_outputEnabled; }

    // Get the full shift register value (for debugging/testing)
    uint16_t getShiftRegisterValue() const { return m_shiftRegister; }

    K4003(const K4003&) = delete;
    K4003& operator=(const K4003&) = delete;

private:
    uint16_t m_shiftRegister;   // 10-bit shift register (bits 0-9 used)
    bool m_outputEnabled;       // Output enable state
    uint8_t m_bitPosition;      // Current bit position for loading (0-9)
};
