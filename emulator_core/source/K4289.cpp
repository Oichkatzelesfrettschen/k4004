#include "emulator_core/source/K4289.hpp"

K4289::K4289() :
    m_address12bit(0u),
    m_data8bit(0u),
    m_ioPort(0u),
    m_ioMask(0u),
    m_readMode(true),
    m_chipEnabled(true),
    m_programMemoryMode(false)
{
}

void K4289::reset()
{
    m_address12bit = 0u;
    m_data8bit = 0u;
    m_ioPort = 0u;
    m_ioMask = 0u;
    m_readMode = true;
    m_chipEnabled = true;
    m_programMemoryMode = false;
}

void K4289::setAddress(uint16_t address)
{
    // Store 12-bit address (8-bit address + 4-bit chip select)
    m_address12bit = address & 0x0FFFu;
}

void K4289::writeData(uint8_t data)
{
    // Only write if chip is enabled and in write mode
    if (m_chipEnabled && !m_readMode) {
        m_data8bit = data;
    }
}

void K4289::setControl(bool read, bool enable)
{
    m_readMode = read;
    m_chipEnabled = enable;
}

void K4289::writeIOPort(uint8_t value)
{
    // Intel 4289 I/O Port Write Logic (4001-compatible)
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

uint8_t K4289::readIOPort() const
{
    // Return the current I/O port value
    return m_ioPort & 0x0Fu;
}
