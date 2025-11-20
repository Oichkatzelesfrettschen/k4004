#include "emulator_core/source/K4003.hpp"

K4003::K4003() :
    m_shiftRegister(0u),
    m_outputEnabled(true)
{
}

void K4003::reset()
{
    m_shiftRegister = 0u;
    m_outputEnabled = true;
}

uint8_t K4003::shiftIn(uint8_t dataBit)
{
    // Capture bit 9 (MSB) before shifting - this gets shifted out for cascading
    uint8_t shiftedOut = (m_shiftRegister >> 9) & 1u;

    // Shift left by 1 position (all bits move up)
    m_shiftRegister = (m_shiftRegister << 1) & 0x3FFu;  // Mask to 10 bits (0x3FF = 0b1111111111)

    // Insert new bit at position 0 (LSB)
    if (dataBit & 1u) {
        m_shiftRegister |= 1u;
    }

    return shiftedOut;
}

uint8_t K4003::getOutputBit(uint8_t index) const
{
    if (index >= NUM_OUTPUTS)
        return 0u;

    if (!m_outputEnabled)
        return 0u;

    return (m_shiftRegister >> index) & 1u;
}
