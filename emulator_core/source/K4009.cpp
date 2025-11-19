#include "emulator_core/source/K4009.hpp"

K4009::K4009() :
    m_dataBuffer(0u),
    m_programMode(true),
    m_readMode(true),
    m_cycle(0u)
{
}

void K4009::reset()
{
    m_dataBuffer = 0u;
    m_programMode = true;
    m_readMode = true;
    m_cycle = 0u;
}

void K4009::write4Bit(uint8_t nibble, bool highNibble)
{
    // Write a 4-bit nibble to the appropriate position
    nibble &= 0x0Fu;

    if (highNibble) {
        // High nibble (bits 4-7)
        m_dataBuffer = (m_dataBuffer & 0x0Fu) | (nibble << 4);
    } else {
        // Low nibble (bits 0-3)
        m_dataBuffer = (m_dataBuffer & 0xF0u) | nibble;
    }
}

uint8_t K4009::read4Bit(bool highNibble) const
{
    // Read a 4-bit nibble from the appropriate position
    if (highNibble) {
        // High nibble (bits 4-7)
        return (m_dataBuffer >> 4) & 0x0Fu;
    } else {
        // Low nibble (bits 0-3)
        return m_dataBuffer & 0x0Fu;
    }
}

void K4009::set8BitData(uint8_t data)
{
    // Set the full 8-bit data buffer (from memory read)
    m_dataBuffer = data;
}

void K4009::setMode(bool programMode, bool readMode)
{
    m_programMode = programMode;
    m_readMode = readMode;
}

void K4009::advanceCycle()
{
    // Advance to next cycle (0 -> 1 -> 0 -> 1 ...)
    m_cycle = (m_cycle + 1) % 2;
}
