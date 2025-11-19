#include <gtest/gtest.h>
#include "emulator_core/source/K4201A.hpp"

class K4201ATest : public ::testing::Test {
protected:
    K4201A clockGen;
};

// Test default construction
TEST_F(K4201ATest, DefaultConstruction) {
    // Default should be 5.185 MHz ÷ 7 = 740.7 kHz
    EXPECT_EQ(clockGen.getOutputFrequency(), 740714u);  // 5185000 / 7
}

// Test divide by 7 (standard 4004 configuration)
TEST_F(K4201ATest, DivideBy7) {
    clockGen.setCrystalFrequency(5185000u);  // 5.185 MHz
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);

    EXPECT_EQ(clockGen.getOutputFrequency(), 740714u);
}

// Test divide by 8
TEST_F(K4201ATest, DivideBy8) {
    clockGen.setCrystalFrequency(5000000u);  // 5.0 MHz
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_8);

    EXPECT_EQ(clockGen.getOutputFrequency(), 625000u);  // 5000000 / 8
}

// Test various crystal frequencies
TEST_F(K4201ATest, VariousCrystalFrequencies) {
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);

    // 7 MHz crystal
    clockGen.setCrystalFrequency(7000000u);
    EXPECT_EQ(clockGen.getOutputFrequency(), 1000000u);

    // 4 MHz crystal
    clockGen.setCrystalFrequency(4000000u);
    EXPECT_EQ(clockGen.getOutputFrequency(), 571428u);  // 4000000 / 7
}

// Test clock period calculation
TEST_F(K4201ATest, ClockPeriod) {
    clockGen.setCrystalFrequency(5185000u);
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);

    // Period = 1 / 740714 Hz ≈ 1350 ns
    uint32_t period = clockGen.getClockPeriodNs();
    EXPECT_NEAR(period, 1350u, 10u);  // Allow ±10 ns tolerance
}

// Test instruction cycle time
TEST_F(K4201ATest, InstructionCycleTime) {
    clockGen.setCrystalFrequency(5185000u);
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);

    // Instruction cycle = 8 clock periods
    // 8 × 1.35 μs ≈ 10.8 μs
    float cycleTime = clockGen.getInstructionCycleTimeUs();
    EXPECT_NEAR(cycleTime, 10.8f, 0.2f);  // Allow ±0.2 μs tolerance
}

// Test clock tick and phase toggling
TEST_F(K4201ATest, ClockTick) {
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);
    clockGen.reset();

    // Initial phase should be φ1 (0)
    EXPECT_EQ(clockGen.getCurrentPhase(), 0u);

    // Tick 7 times (÷7 ratio) - should toggle to φ2
    for (int i = 0; i < 7; ++i) {
        bool cycleDone = clockGen.tick();
        if (i < 6) {
            EXPECT_FALSE(cycleDone);
        }
    }
    EXPECT_EQ(clockGen.getCurrentPhase(), 1u);  // Now in φ2

    // Tick 7 more times - should toggle back to φ1 and return true (cycle complete)
    bool cycleDone = false;
    for (int i = 0; i < 7; ++i) {
        cycleDone = clockGen.tick();
    }
    EXPECT_TRUE(cycleDone);
    EXPECT_EQ(clockGen.getCurrentPhase(), 0u);  // Back to φ1
}

// Test divide by 8 ticking
TEST_F(K4201ATest, DivideBy8Ticking) {
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_8);
    clockGen.reset();

    // Tick 8 times - should toggle phase
    for (int i = 0; i < 8; ++i) {
        clockGen.tick();
    }
    EXPECT_EQ(clockGen.getCurrentPhase(), 1u);

    // Tick 8 more times - complete cycle
    bool cycleDone = false;
    for (int i = 0; i < 8; ++i) {
        cycleDone = clockGen.tick();
    }
    EXPECT_TRUE(cycleDone);
    EXPECT_EQ(clockGen.getCurrentPhase(), 0u);
}

// Test reset
TEST_F(K4201ATest, Reset) {
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);

    // Tick a few times
    for (int i = 0; i < 5; ++i) {
        clockGen.tick();
    }

    // Reset
    clockGen.reset();
    EXPECT_EQ(clockGen.getCurrentPhase(), 0u);

    // Should now take full 7 ticks to toggle
    for (int i = 0; i < 6; ++i) {
        bool cycleDone = clockGen.tick();
        EXPECT_FALSE(cycleDone);
    }
    clockGen.tick();
    EXPECT_EQ(clockGen.getCurrentPhase(), 1u);
}

// Test standard 4004 timing (740 kHz)
TEST_F(K4201ATest, Standard4004Timing) {
    // Standard 4004: 5.185 MHz ÷ 7 = 740.7 kHz
    // Clock period: 1.35 μs
    // Instruction: 10.8 μs (8 clocks)

    clockGen.setCrystalFrequency(5185000u);
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);

    uint32_t freq = clockGen.getOutputFrequency();
    EXPECT_NEAR(freq, 740714u, 100u);

    uint32_t period = clockGen.getClockPeriodNs();
    EXPECT_NEAR(period, 1350u, 20u);

    float instrTime = clockGen.getInstructionCycleTimeUs();
    EXPECT_NEAR(instrTime, 10.8f, 0.2f);
}

// Test standard 4040 timing
TEST_F(K4201ATest, Standard4040Timing) {
    // 4040 can use same timing as 4004
    clockGen.setCrystalFrequency(5185000u);
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);

    EXPECT_EQ(clockGen.getOutputFrequency(), 740714u);
}

// Test frequency boundary conditions
TEST_F(K4201ATest, FrequencyBoundaries) {
    // Minimum frequency (4 MHz)
    clockGen.setCrystalFrequency(4000000u);
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_8);
    EXPECT_EQ(clockGen.getOutputFrequency(), 500000u);  // 500 kHz

    // Maximum frequency (7 MHz)
    clockGen.setCrystalFrequency(7000000u);
    clockGen.setDivideRatio(K4201A::DivideRatio::DIVIDE_7);
    EXPECT_EQ(clockGen.getOutputFrequency(), 1000000u);  // 1 MHz
}
