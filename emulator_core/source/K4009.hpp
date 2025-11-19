#pragma once
#include <cstdint>

// Intel 4009 - Program and I/O Access Converter
// Package: 16-pin DIP
// Function: Converts 4-bit bus to 8-bit parallel for standard memory
//
// The 4009 works in conjunction with the 4008 to provide a complete
// interface between the 4004/4040 and standard 8-bit memory devices.
//
// Specifications:
// - 4-to-8 bit bus conversion
// - 8-to-4 bit bus conversion
// - Bidirectional data flow
// - Program/I/O mode selection
// - Read/Write mode selection
// - Technology: P-MOS
// - Power: +5V, -15V
//
// Operation:
// - Two 4-bit cycles assemble into one 8-bit word
// - First cycle: low nibble (bits 0-3)
// - Second cycle: high nibble (bits 4-7)
//
// Typical Use:
// - Interface with standard 8-bit memory
// - Works with C2102A (1K×1 RAM)
// - Works with C1702A (256×8 EPROM)
// - Data buffering and conversion
//
// Used with:
// - 4008 (required partner chip)
// - Standard RAM/ROM chips
// - TTL memory devices

class K4009
{
public:
    K4009();

    // Reset the chip
    void reset();

    // Write 4-bit nibble (builds 8-bit word over 2 cycles)
    void write4Bit(uint8_t nibble, bool highNibble);

    // Read 4-bit nibble (returns part of 8-bit word)
    uint8_t read4Bit(bool highNibble) const;

    // Get/Set full 8-bit data (for memory interface)
    uint8_t get8BitData() const { return m_dataBuffer; }
    void set8BitData(uint8_t data);

    // Mode control
    void setMode(bool programMode, bool readMode);
    bool isProgramMode() const { return m_programMode; }
    bool isReadMode() const { return m_readMode; }

    // Cycle tracking
    void advanceCycle();
    bool isLowNibbleCycle() const { return m_cycle == 0; }
    bool isHighNibbleCycle() const { return m_cycle == 1; }
    void resetCycle() { m_cycle = 0; }

    K4009(const K4009&) = delete;
    K4009& operator=(const K4009&) = delete;

private:
    uint8_t m_dataBuffer;       // 8-bit data buffer
    bool m_programMode;         // true=program memory, false=I/O
    bool m_readMode;            // true=read, false=write
    uint8_t m_cycle;            // Current cycle (0=low nibble, 1=high nibble)
};
