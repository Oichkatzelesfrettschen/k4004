#include "emulator_core/source/K4101.hpp"
#include <cstring>

K4101::K4101() :
    m_chipEnabled(true)
{
    reset();
}

void K4101::reset()
{
    std::memset(m_ram, 0, RAM_SIZE);
    m_chipEnabled = true;
}

void K4101::write(uint8_t address, uint8_t value)
{
    // Only write if chip is enabled
    if (!m_chipEnabled)
        return;

    // Store only lower 4 bits
    m_ram[address] = value & 0x0Fu;
}

uint8_t K4101::read(uint8_t address) const
{
    // In real hardware, chip enable controls bus access (tri-state)
    // In emulation, we still return the data for debugging/testing
    // The chip enable primarily controls write access
    return m_ram[address] & 0x0Fu;
}
