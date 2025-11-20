#pragma once
#include <cstdint>

// Intel 4101 - 1024-bit Static RAM (256 × 4)
// Package: 16-pin DIP
// Function: Static RAM with standard memory interface
//
// Specifications:
// - Capacity: 1024 bits (256 words × 4 bits)
// - Organization: 256 × 4-bit
// - Access Time: Typical 500 ns
// - Technology: NMOS static RAM
// - Power: +5V, -12V
// - I/O: Separate data input and output
//
// Advantages over 4002:
// - Much larger capacity (256 × 4 vs 40 × 4)
// - Faster access time (500 ns vs 1.2 μs)
// - No status character complexity
// - Standard memory interface (works with 4289)
// - True static RAM (no refresh needed)
//
// Pin Functions:
// - 8-bit address input (A0-A7)
// - 4-bit data input (D0-D3)
// - 4-bit data output (O0-O3)
// - Chip Enable (CE, active low)
// - Write Enable (WE, active low)

class K4101
{
public:
    static constexpr uint16_t RAM_SIZE = 256u;      // 256 words
    static constexpr uint8_t WORD_SIZE = 4u;        // 4 bits per word
    static constexpr uint16_t ACCESS_TIME_NS = 500u; // Typical access time

    K4101();

    // Reset the RAM to all zeros
    void reset();

    // Write a 4-bit value to the specified address
    void write(uint8_t address, uint8_t value);

    // Read a 4-bit value from the specified address
    uint8_t read(uint8_t address) const;

    // Chip enable control
    void setChipEnable(bool enable) { m_chipEnabled = enable; }
    bool isChipEnabled() const { return m_chipEnabled; }

    // Direct memory access (for debugging/testing)
    const uint8_t* getMemoryContents() const { return m_ram; }

    K4101(const K4101&) = delete;
    K4101& operator=(const K4101&) = delete;

private:
    uint8_t m_ram[RAM_SIZE];    // 256 × 4-bit storage
    bool m_chipEnabled;         // Chip enable state
};
