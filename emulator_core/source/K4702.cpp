#include "emulator_core/source/K4702.hpp"
#include <cstring>

K4702::K4702() :
    m_programMode(false),
    m_programCount(0u),
    m_eraseCount(0u)
{
    // Initialize to erased state (all 0xFF)
    erase();
}

void K4702::reset()
{
    // Reset to erased state
    erase();
    m_programMode = false;
    m_programCount = 0u;
    m_eraseCount = 0u;
}

void K4702::erase()
{
    // Simulate UV erase: set all bytes to 0xFF (erased state)
    std::memset(m_eprom, ERASED_VALUE, EPROM_SIZE);
    m_eraseCount++;
    m_programCount = 0u;
}

bool K4702::program(uint8_t address, uint8_t data)
{
    // Programming can only occur in program mode
    if (!m_programMode)
        return false;

    // EPROM programming: can only change 1s to 0s
    // Cannot change 0s to 1s (requires UV erase first)
    uint8_t currentValue = m_eprom[address];

    // Programming 0xFF is always valid (no-op: doesn't clear any bits)
    if (data == 0xFF) {
        m_programCount++;
        return true;
    }

    // Check if the programming would require changing 0 to 1
    // This is not possible without erasing
    if ((currentValue & data) != data) {
        return false;  // Cannot program - would require erase
    }

    // Program the byte (AND operation: can only clear bits)
    m_eprom[address] &= data;
    m_programCount++;

    return true;
}

uint8_t K4702::readByte(uint8_t address) const
{
    // Read operation is always available (no mode required)
    return m_eprom[address];
}

bool K4702::isErased() const
{
    // Check if entire EPROM is in erased state (all 0xFF)
    for (uint16_t i = 0; i < EPROM_SIZE; ++i) {
        if (m_eprom[i] != ERASED_VALUE)
            return false;
    }
    return true;
}

bool K4702::isByteErased(uint8_t address) const
{
    return m_eprom[address] == ERASED_VALUE;
}
