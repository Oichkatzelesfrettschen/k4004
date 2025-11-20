#pragma once
#include <cstdint>
#include <cstddef>

// Intel 4702 - 2K EPROM (256×8)
// Package: 24-pin DIP (ceramic with UV-erasable window)
// Function: Erasable Programmable Read-Only Memory for development
//
// Specifications:
// - Capacity: 256 × 8 bits = 2048 bits = 256 bytes
// - Organization: 256 words × 8 bits
// - 8-bit addressing (A0-A7)
// - Access Time: 1 μs typical
// - Technology: Floating-gate UV-erasable
// - Programming: 25V pulses (100μs per byte)
// - Erase: UV light at 253.7 nm for 15-20 minutes
// - Data Retention: 10-20+ years
// - Programming Cycles: ~100 erase/program cycles
//
// Advantages:
// - Reusable (UV-erasable)
// - Perfect for prototyping
// - Field-programmable
// - No mask costs
// - Faster development iteration
//
// Typical Use:
// - Program development
// - Prototyping systems
// - Field-updatable firmware
// - Small production runs
// - Intellec 4/40 development system

class K4702
{
public:
    static constexpr uint16_t EPROM_SIZE = 256u;     // 256 bytes
    static constexpr uint8_t ADDRESS_BITS = 8u;      // 8-bit addressing
    static constexpr uint16_t ACCESS_TIME_NS = 1000u; // 1 μs typical
    static constexpr uint8_t ERASED_VALUE = 0xFFu;   // Erased state (all 1s)

    K4702();

    // Reset to erased state
    void reset();

    // Erase the EPROM (simulate UV exposure)
    void erase();

    // Program a byte at specified address
    // Returns true if programming successful, false if already programmed
    bool program(uint8_t address, uint8_t data);

    // Read a byte from EPROM
    uint8_t readByte(uint8_t address) const;

    // Check if EPROM is in erased state
    bool isErased() const;

    // Check if a specific byte is erased
    bool isByteErased(uint8_t address) const;

    // Program mode control (enables programming voltage)
    void setProgramMode(bool enable) { m_programMode = enable; }
    bool isProgramMode() const { return m_programMode; }

    // Direct memory access (for debugging/testing)
    const uint8_t* getEpromContents() const { return m_eprom; }

    // Get programming statistics
    uint16_t getProgramCount() const { return m_programCount; }
    uint16_t getEraseCount() const { return m_eraseCount; }

    K4702(const K4702&) = delete;
    K4702& operator=(const K4702&) = delete;

private:
    uint8_t m_eprom[EPROM_SIZE];    // 256 × 8-bit EPROM storage
    bool m_programMode;             // Programming mode enabled
    uint16_t m_programCount;        // Number of bytes programmed
    uint16_t m_eraseCount;          // Number of erase cycles
};
