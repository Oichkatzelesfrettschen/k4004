#include "emulator_core/source/K4003.hpp"

K4003::K4003() :
    m_shiftRegister(0u),
    m_outputEnabled(true),
    m_bitPosition(0u)
{
}

void K4003::reset()
{
    m_shiftRegister = 0u;
    m_outputEnabled = true;
    m_bitPosition = 0u;
}

uint8_t K4003::shiftIn(uint8_t dataBit)
{
    // Set the bit at the current position
    if (dataBit & 1u) {
        m_shiftRegister |= (1u << m_bitPosition);
    } else {
        m_shiftRegister &= ~(1u << m_bitPosition);
    }

    // Capture bit that will be shifted out (bit that was at position 9)
    uint8_t shiftedOut = (m_shiftRegister >> NUM_OUTPUTS) & 1u;

    // Increment position and wrap around
    m_bitPosition = (m_bitPosition + 1) % NUM_OUTPUTS;

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
