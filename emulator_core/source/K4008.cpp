#include "emulator_core/source/K4008.hpp"

K4008::K4008() :
    m_addressLatch(0u),
    m_chipSelect(0u),
    m_ioPort(0u),
    m_ioMask(0u),
    m_chipEnabled(true)
{
}

void K4008::reset()
{
    m_addressLatch = 0u;
    m_chipSelect = 0u;
    m_ioPort = 0u;
    m_ioMask = 0u;
    m_chipEnabled = true;
}

void K4008::latchAddress(uint8_t address)
{
    // Latch the 8-bit address
    m_addressLatch = address;
}

void K4008::setChipSelect(uint8_t chipSelect)
{
    // Store 4-bit chip select value (CM0-CM3)
    m_chipSelect = chipSelect & 0x0Fu;
}

void K4008::writeIOPort(uint8_t value)
{
    // Intel 4008 I/O Port Write Logic
    // - Mask bit = 0: Output (CPU can write)
    // - Mask bit = 1: Input (value preserved)

    if (!m_chipEnabled)
        return;

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

uint8_t K4008::readIOPort() const
{
    if (!m_chipEnabled)
        return 0u;

    return m_ioPort & 0x0Fu;
}

void K4008::setExternalIOPort(uint8_t value)
{
    // Allow external devices to set I/O port values
    // Only affects pins configured as inputs (mask bit = 1)

    uint8_t mask = m_ioMask;
    uint8_t oldValue = m_ioPort;
    uint8_t newValue = 0u;

    for (uint8_t i = 0; i < 4; ++i) {
        bool isInput = (mask >> i) & 1;
        uint8_t bitValue = isInput ? ((value >> i) & 1) : ((oldValue >> i) & 1);
        newValue |= (bitValue << i);
    }

    m_ioPort = newValue & 0x0Fu;
}
