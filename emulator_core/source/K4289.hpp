#pragma once
#include <cstdint>
#include <cstddef>

// Intel 4289 - Standard Memory Interface
// Package: 24-pin DIP
// Function: Unified memory interface combining 4008 + 4009 functionality
//
// The 4289 allows the 4004/4040 CPU to interface with standard 8-bit memory
// devices (RAM, ROM, EPROM) while appearing as a 4001 ROM to the CPU.
//
// Key Features:
// - 12-bit address generation (8-bit address + 4-bit chip select)
// - 8-bit bidirectional data bus
// - Supports standard RAM and ROM chips
// - Enables RAM as program memory
// - 4-bit I/O port (4001-compatible)
// - Timing synchronized with CPU
//
// Typical Use:
// - Interface with C2102A (1K×1 RAM)
// - Interface with 1702A (256×8 EPROM)
// - Interface with 4101 (256×4 SRAM)
// - Interface with 4308 (1K×8 ROM)
// - Mixed ROM/RAM systems up to 8KB

class K4289
{
public:
    static constexpr uint8_t ADDRESS_BITS = 8u;
    static constexpr uint8_t CHIPSEL_BITS = 4u;
    static constexpr uint16_t TOTAL_ADDRESS_BITS = 12u;  // 8 + 4

    K4289();

    // Reset the interface
    void reset();

    // Address management (12-bit total: 8-bit address + 4-bit chip select)
    void setAddress(uint16_t address);
    uint16_t getAddress() const { return m_address12bit; }
    uint8_t getAddress8bit() const { return m_address12bit & 0xFFu; }
    uint8_t getChipSelect() const { return (m_address12bit >> 8) & 0x0Fu; }

    // 8-bit data bus operations
    void writeData(uint8_t data);
    uint8_t readData() const { return m_data8bit; }

    // 4-bit I/O port (emulates 4001 ROM I/O)
    void writeIOPort(uint8_t value);
    uint8_t readIOPort() const;
    void setIOMask(uint8_t mask) { m_ioMask = mask & 0x0Fu; }
    uint8_t getIOMask() const { return m_ioMask; }

    // Control signals
    void setControl(bool read, bool enable);
    bool isReadMode() const { return m_readMode; }
    bool isChipEnabled() const { return m_chipEnabled; }

    // Program memory mode (RAM as ROM)
    void setProgramMemoryMode(bool enable) { m_programMemoryMode = enable; }
    bool isProgramMemoryMode() const { return m_programMemoryMode; }

    K4289(const K4289&) = delete;
    K4289& operator=(const K4289&) = delete;

private:
    // Address and data
    uint16_t m_address12bit;        // 12-bit address (bits 0-7: address, bits 8-11: chip select)
    uint8_t m_data8bit;             // 8-bit data buffer

    // I/O port (4001-compatible)
    uint8_t m_ioPort;               // 4-bit I/O port
    uint8_t m_ioMask;               // I/O direction mask (0=output, 1=input)

    // Control signals
    bool m_readMode;                // true=read, false=write
    bool m_chipEnabled;             // Chip enable state
    bool m_programMemoryMode;       // RAM as program memory mode
};
