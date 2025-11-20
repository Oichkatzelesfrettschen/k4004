#include "emulator_core/source/K4308.hpp"
#include <cstring>

K4308::K4308() :
    m_ioPort(0u),
    m_ioMask(0u)
{
    std::memset(m_rom, 0, ROM_SIZE);
}

void K4308::reset()
{
    std::memset(m_rom, 0, ROM_SIZE);
    m_ioPort = 0u;
    m_ioMask = 0u;
}

bool K4308::load(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0)
        return false;

    if (size > ROM_SIZE)
        size = ROM_SIZE;  // Truncate if too large

    std::memcpy(m_rom, data, size);
    return true;
}

uint8_t K4308::readByte(uint16_t address) const
{
    // Mask to 10-bit address (0-1023)
    return m_rom[address & 0x3FFu];
}

void K4308::writeIOPort(uint8_t value)
{
    // Intel 4308 I/O Port Write Logic (same as 4001)
    // - Mask bit = 0: Output (CPU can write)
    // - Mask bit = 1: Input (value preserved)

    uint8_t oldValue = m_ioPort;
    uint8_t mask = m_ioMask;
    uint8_t newValue = 0u;

    // For each of the 4 I/O bits
    for (uint8_t i = 0; i < 4; ++i) {
        bool isInput = (mask >> i) & 1;
        uint8_t bitValue = isInput ? ((oldValue >> i) & 1) : ((value >> i) & 1);
        newValue |= (bitValue << i);
    }

    m_ioPort = newValue & 0x0Fu;
}

uint8_t K4308::readIOPort() const
{
    // Return the current I/O port value
    return m_ioPort & 0x0Fu;
}

void K4308::setExternalIOPort(uint8_t value)
{
    // Allow external devices (keyboard, switches, etc.) to set I/O port values
    // Only affects pins configured as inputs (mask bit = 1)

    uint8_t mask = m_ioMask;
    uint8_t oldValue = m_ioPort;
    uint8_t newValue = 0u;

    // For each of the 4 I/O bits:
    // If mask bit is 1 (input), use new external value
    // If mask bit is 0 (output), preserve CPU-written value
    for (uint8_t i = 0; i < 4; ++i) {
        bool isInput = (mask >> i) & 1;
        uint8_t bitValue = isInput ? ((value >> i) & 1) : ((oldValue >> i) & 1);
        newValue |= (bitValue << i);
    }

    m_ioPort = newValue & 0x0Fu;
}
