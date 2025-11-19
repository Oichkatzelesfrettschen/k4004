#pragma once
#include <cstdint>
#include <cstddef>

// Intel 4308 - 8K ROM with I/O Port
// Package: 24-pin DIP
// Function: Large program storage with built-in I/O
//
// Specifications:
// - Capacity: 1024 × 8 bits = 8192 bits = 1KB
// - Organization: 1024 words × 8 bits
// - Access Time: 800 ns typical
// - 10-bit addressing (A0-A9)
// - 4-bit I/O port (like 4001 ROM)
// - Technology: Mask-programmed ROM
// - Power: +5V, -12V
//
// Advantages over 4001:
// - 4× larger capacity (1024 bytes vs 256 bytes)
// - 8-bit data width (more efficient storage)
// - Still includes I/O port
// - Better for complex programs
//
// Typical Use:
// - Large program storage
// - Calculator firmware
// - Control systems
// - Combined with 4289 for system expansion

class K4308
{
public:
    static constexpr uint16_t ROM_SIZE = 1024u;      // 1024 bytes
    static constexpr uint16_t ADDRESS_BITS = 10u;    // 10-bit addressing
    static constexpr uint16_t ACCESS_TIME_NS = 800u; // Typical access time

    K4308();

    // Reset the ROM
    void reset();

    // Load ROM image from memory
    bool load(const uint8_t* data, size_t size);

    // Read a byte from ROM (10-bit address)
    uint8_t readByte(uint16_t address) const;

    // I/O Port operations (4-bit, like 4001)
    void writeIOPort(uint8_t value);
    uint8_t readIOPort() const;
    void setIOMask(uint8_t mask) { m_ioMask = mask & 0x0Fu; }
    uint8_t getIOMask() const { return m_ioMask; }

    // Allow external devices to set I/O port input pins
    void setExternalIOPort(uint8_t value);

    // Direct memory access (for debugging/testing)
    const uint8_t* getRomContents() const { return m_rom; }
    uint8_t getIOPort() const { return m_ioPort; }

    K4308(const K4308&) = delete;
    K4308& operator=(const K4308&) = delete;

private:
    uint8_t m_rom[ROM_SIZE];    // 1024 × 8-bit ROM storage
    uint8_t m_ioPort;           // 4-bit I/O port
    uint8_t m_ioMask;           // I/O direction mask (0=output, 1=input)
};
