#include "emulator_core/source/K4201A.hpp"

K4201A::K4201A() :
    m_crystalFrequency(5185000u),  // Default: 5.185 MHz
    m_divideRatio(DivideRatio::DIVIDE_7),
    m_divider(0u),
    m_currentPhase(0u)
{
}

void K4201A::setCrystalFrequency(uint32_t frequency)
{
    // Typical range: 4-7 MHz
    if (frequency >= 4000000u && frequency <= 7000000u) {
        m_crystalFrequency = frequency;
    }
}

void K4201A::setDivideRatio(DivideRatio ratio)
{
    m_divideRatio = ratio;
}

uint32_t K4201A::getOutputFrequency() const
{
    return m_crystalFrequency / static_cast<uint8_t>(m_divideRatio);
}

uint32_t K4201A::getClockPeriodNs() const
{
    // Clock period in nanoseconds
    // Period = 1 / Frequency
    uint32_t frequency = getOutputFrequency();
    if (frequency == 0u)
        return 0u;

    return (1000000000u / frequency);
}

float K4201A::getInstructionCycleTimeUs() const
{
    // Each instruction cycle = 8 clock periods
    // Standard 4004: 8 × 1.35 μs = 10.8 μs (for 740 kHz clock)
    uint32_t periodNs = getClockPeriodNs();
    return (periodNs * 8.0f) / 1000.0f;  // Convert to microseconds
}

bool K4201A::tick()
{
    // Increment the divider
    ++m_divider;

    // Check if we've reached the divide ratio
    if (m_divider >= static_cast<uint8_t>(m_divideRatio)) {
        m_divider = 0u;

        // Toggle the phase (φ1 ↔ φ2)
        m_currentPhase ^= 1u;

        // Return true when completing a full cycle (φ1 + φ2)
        return m_currentPhase == 0u;
    }

    return false;
}

void K4201A::reset()
{
    m_divider = 0u;
    m_currentPhase = 0u;
}
