#pragma once
#include <cstdint>

// Intel 4008 - 8-Bit Address Latch
// Package: 16-pin DIP
// Function: Address latch and chip select for standard memory interface
//
// The 4008 works in conjunction with the 4009 to allow the 4004/4040
// to interface with standard 8-bit memory devices.
//
// Specifications:
// - 8-bit address output (A0-A7)
// - 4-bit binary chip select (CM0-CM3)
// - Built-in 4-bit I/O port
// - Synchronizes with CPU timing
// - Technology: P-MOS
// - Power: +5V, -15V
//
// Typical Use:
// - Interface with standard RAM/ROM chips
// - Address latching for memory cycles
// - Chip selection (1 of 16)
// - I/O expansion
//
// Used with:
// - 4009 (required partner chip)
// - C2102A RAM (1K×1)
// - C1702A EPROM (256×8)
// - Standard TTL memory

class K4008
{
public:
    K4008();

    // Reset the chip
    void reset();

    // Address latching (8-bit address)
    void latchAddress(uint8_t address);
    uint8_t getAddress() const { return m_addressLatch; }

    // Chip select (4-bit, binary decoded)
    void setChipSelect(uint8_t chipSelect);
    uint8_t getChipSelect() const { return m_chipSelect; }

    // I/O Port operations (4-bit)
    void writeIOPort(uint8_t value);
    uint8_t readIOPort() const;
    void setIOMask(uint8_t mask) { m_ioMask = mask & 0x0Fu; }
    uint8_t getIOMask() const { return m_ioMask; }

    // Allow external devices to set I/O port input pins
    void setExternalIOPort(uint8_t value);

    // Chip enable
    void setChipEnable(bool enable) { m_chipEnabled = enable; }
    bool isChipEnabled() const { return m_chipEnabled; }

    K4008(const K4008&) = delete;
    K4008& operator=(const K4008&) = delete;

private:
    uint8_t m_addressLatch;     // 8-bit latched address (A0-A7)
    uint8_t m_chipSelect;       // 4-bit chip select (CM0-CM3)
    uint8_t m_ioPort;           // 4-bit I/O port
    uint8_t m_ioMask;           // I/O direction mask (0=output, 1=input)
    bool m_chipEnabled;         // Chip enable state
};
