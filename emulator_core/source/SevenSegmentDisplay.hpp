#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

// 7-Segment LED Display Emulation
// ================================
//
// Emulates classic 7-segment LED displays common in 1970s microcomputer systems.
// Used with Intel MCS-4/MCS-40 for calculators, instruments, and displays.
//
// Segment Layout:
//      _a_
//    f|   |b
//     |_g_|
//    e|   |c
//     |___|.dp
//       d
//
// Each segment can be individually controlled. Common configurations:
// - Single digit displays
// - Multi-digit displays (1-8 digits typical)
// - Common anode or common cathode
//
// Historical Context:
// - TIL311 (Texas Instruments): Single digit with built-in latch/decoder
// - HP 5082-7340: Common anode 7-segment display
// - Used in calculators (HP-35, HP-65), instruments, early computers

class SevenSegmentDisplay
{
public:
    // Segment bit positions in byte
    static constexpr uint8_t SEG_A  = 0;  // Top
    static constexpr uint8_t SEG_B  = 1;  // Top-right
    static constexpr uint8_t SEG_C  = 2;  // Bottom-right
    static constexpr uint8_t SEG_D  = 3;  // Bottom
    static constexpr uint8_t SEG_E  = 4;  // Bottom-left
    static constexpr uint8_t SEG_F  = 5;  // Top-left
    static constexpr uint8_t SEG_G  = 6;  // Middle
    static constexpr uint8_t SEG_DP = 7;  // Decimal point

    // Segment masks
    static constexpr uint8_t MASK_A  = (1u << SEG_A);
    static constexpr uint8_t MASK_B  = (1u << SEG_B);
    static constexpr uint8_t MASK_C  = (1u << SEG_C);
    static constexpr uint8_t MASK_D  = (1u << SEG_D);
    static constexpr uint8_t MASK_E  = (1u << SEG_E);
    static constexpr uint8_t MASK_F  = (1u << SEG_F);
    static constexpr uint8_t MASK_G  = (1u << SEG_G);
    static constexpr uint8_t MASK_DP = (1u << SEG_DP);

    // Construction
    explicit SevenSegmentDisplay(size_t numDigits = 1);

    // Direct segment control
    void setSegments(size_t digit, uint8_t segments);
    uint8_t getSegments(size_t digit) const;

    // Set from hex value (0-F)
    void setHexDigit(size_t digit, uint8_t value);

    // Set from BCD value (0-9)
    void setBCDDigit(size_t digit, uint8_t value);

    // Individual segment control
    void setSegment(size_t digit, char segment, bool on);
    bool getSegment(size_t digit, char segment) const;

    // Decimal point control
    void setDecimalPoint(size_t digit, bool on);
    bool getDecimalPoint(size_t digit) const;

    // Clear display
    void clear();
    void clearDigit(size_t digit);

    // Display properties
    size_t numDigits() const { return m_digits.size(); }

    // ASCII representation (for testing/debugging)
    std::string toASCII(size_t digit) const;
    std::string toASCIIAll() const;

    // Multi-digit display helpers
    void setHexValue(uint32_t value);      // Display as hex
    void setDecimalValue(uint32_t value);  // Display as decimal
    void setString(const char* str);       // Display string (limited chars)

private:
    std::vector<uint8_t> m_digits;  // Segment data for each digit

    // Hex digit patterns (0-F)
    static const uint8_t HEX_PATTERNS[16];

    // BCD digit patterns (0-9)
    static const uint8_t BCD_PATTERNS[10];

    // Character to segment mapping (limited)
    static const uint8_t CHAR_PATTERNS[128];

    // Helper to get segment bit from character
    static uint8_t charToSegmentBit(char segment);
};
