#pragma once
#include <cstdint>

// Intel 4201A - Two-Phase Clock Generator
// Package: 16-pin DIP
// Function: Generates two-phase non-overlapping clocks for 4004/4040 CPU
//
// Specifications:
// - Input: 4-7 MHz crystal oscillator
// - Output: Two-phase clocks (φ1 and φ2)
// - Divide Ratio: ÷7 or ÷8 (jumper selectable)
// - Output Frequency: 500-740 kHz (typical 740 kHz for 5.185 MHz crystal)
// - Technology: CMOS
// - Non-overlapping: φ1 and φ2 never high simultaneously
//
// Typical Crystal Frequencies:
// - 5.185 MHz ÷ 7 = 740.7 kHz (standard for 4004)
// - 5.000 MHz ÷ 8 = 625.0 kHz
// - 4.000 MHz ÷ 8 = 500.0 kHz
//
// Clock Phases (per instruction cycle):
// - A1, A2, A3 (address/control)
// - M1, M2 (data transfer)
// - X1, X2, X3 (execution)
// Each phase = one CPU clock period (φ1 + φ2)

class K4201A
{
public:
    enum class DivideRatio : uint8_t {
        DIVIDE_7 = 7u,
        DIVIDE_8 = 8u
    };

    K4201A();

    // Set the input crystal frequency (in Hz)
    void setCrystalFrequency(uint32_t frequency);

    // Set the divide ratio (÷7 or ÷8)
    void setDivideRatio(DivideRatio ratio);

    // Get the output CPU clock frequency (in Hz)
    uint32_t getOutputFrequency() const;

    // Get the clock period in nanoseconds
    uint32_t getClockPeriodNs() const;

    // Get the instruction cycle time in microseconds
    // (8 clock periods per instruction cycle)
    float getInstructionCycleTimeUs() const;

    // Simulate clock tick (for accurate timing simulation)
    // Returns true when a full CPU clock cycle completes (φ1 + φ2)
    bool tick();

    // Get current phase (0 = φ1, 1 = φ2)
    uint8_t getCurrentPhase() const { return m_currentPhase; }

    // Reset the clock generator
    void reset();

    K4201A(const K4201A&) = delete;
    K4201A& operator=(const K4201A&) = delete;

private:
    uint32_t m_crystalFrequency;    // Input crystal frequency (Hz)
    DivideRatio m_divideRatio;      // Divide ratio (÷7 or ÷8)
    uint8_t m_divider;              // Current divider count
    uint8_t m_currentPhase;         // Current clock phase (0=φ1, 1=φ2)
};
