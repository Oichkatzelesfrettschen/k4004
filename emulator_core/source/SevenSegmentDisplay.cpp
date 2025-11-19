#include "emulator_core/source/SevenSegmentDisplay.hpp"
#include <sstream>
#include <iomanip>

// Hex digit patterns (0-F)
// Bit pattern: DP G F E D C B A
const uint8_t SevenSegmentDisplay::HEX_PATTERNS[16] = {
    0b00111111,  // 0: A B C D E F
    0b00000110,  // 1: B C
    0b01011011,  // 2: A B D E G
    0b01001111,  // 3: A B C D G
    0b01100110,  // 4: B C F G
    0b01101101,  // 5: A C D F G
    0b01111101,  // 6: A C D E F G
    0b00000111,  // 7: A B C
    0b01111111,  // 8: A B C D E F G
    0b01101111,  // 9: A B C D F G
    0b01110111,  // A: A B C E F G
    0b01111100,  // b: C D E F G
    0b00111001,  // C: A D E F
    0b01011110,  // d: B C D E G
    0b01111001,  // E: A D E F G
    0b01110001   // F: A E F G
};

// BCD digit patterns (0-9, same as hex 0-9)
const uint8_t SevenSegmentDisplay::BCD_PATTERNS[10] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
};

// Character patterns (limited ASCII support)
const uint8_t SevenSegmentDisplay::CHAR_PATTERNS[128] = {
    // Most control characters: blank
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x00-0x0F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x10-0x1F
    0,                         // 0x20 ' ' space
    0,                         // 0x21 '!' (not displayable)
    0b00100010,                // 0x22 '"' (approximation)
    0,                         // 0x23 '#'
    0,                         // 0x24 '$'
    0,                         // 0x25 '%'
    0,                         // 0x26 '&'
    0b00000010,                // 0x27 '\'' (approximation)
    0b00111001,                // 0x28 '(' (like C)
    0b00001111,                // 0x29 ')' (like C reversed)
    0,                         // 0x2A '*'
    0,                         // 0x2B '+'
    0,                         // 0x2C ','
    0b01000000,                // 0x2D '-' (G segment)
    0b10000000,                // 0x2E '.' (DP)
    0,                         // 0x2F '/'
    0b00111111,                // 0x30 '0'
    0b00000110,                // 0x31 '1'
    0b01011011,                // 0x32 '2'
    0b01001111,                // 0x33 '3'
    0b01100110,                // 0x34 '4'
    0b01101101,                // 0x35 '5'
    0b01111101,                // 0x36 '6'
    0b00000111,                // 0x37 '7'
    0b01111111,                // 0x38 '8'
    0b01101111,                // 0x39 '9'
    0,                         // 0x3A ':'
    0,                         // 0x3B ';'
    0,                         // 0x3C '<'
    0b01001000,                // 0x3D '=' (approximation)
    0,                         // 0x3E '>'
    0,                         // 0x3F '?'
    0,                         // 0x40 '@'
    0b01110111,                // 0x41 'A'
    0b01111100,                // 0x42 'B' (lowercase b)
    0b00111001,                // 0x43 'C'
    0b01011110,                // 0x44 'D' (lowercase d)
    0b01111001,                // 0x45 'E'
    0b01110001,                // 0x46 'F'
    0b00111101,                // 0x47 'G'
    0b01110110,                // 0x48 'H'
    0b00000110,                // 0x49 'I' (like 1)
    0b00001110,                // 0x4A 'J'
    0,                         // 0x4B 'K' (not displayable)
    0b00111000,                // 0x4C 'L'
    0,                         // 0x4D 'M' (not displayable)
    0b01010100,                // 0x4E 'N' (lowercase n)
    0b00111111,                // 0x4F 'O' (like 0)
    0b01110011,                // 0x50 'P'
    0b01100111,                // 0x51 'Q'
    0b01010000,                // 0x52 'R' (lowercase r)
    0b01101101,                // 0x53 'S' (like 5)
    0b01111000,                // 0x54 'T' (lowercase t)
    0b00111110,                // 0x55 'U'
    0b00111110,                // 0x56 'V' (like U)
    0,                         // 0x57 'W' (not displayable)
    0,                         // 0x58 'X' (not displayable)
    0b01101110,                // 0x59 'Y'
    0,                         // 0x5A 'Z' (not displayable)
    0b00111001,                // 0x5B '[' (like C)
    0,                         // 0x5C '\'
    0b00001111,                // 0x5D ']'
    0,                         // 0x5E '^'
    0b00001000,                // 0x5F '_' (bottom segment)
    0,                         // 0x60 '`'
    0b01110111,                // 0x61 'a'
    0b01111100,                // 0x62 'b'
    0b01011000,                // 0x63 'c'
    0b01011110,                // 0x64 'd'
    0b01111001,                // 0x65 'e'
    0b01110001,                // 0x66 'f'
    0b01101111,                // 0x67 'g' (like 9)
    0b01110100,                // 0x68 'h'
    0b00000100,                // 0x69 'i'
    0b00001110,                // 0x6A 'j'
    0,                         // 0x6B 'k'
    0b00110000,                // 0x6C 'l'
    0,                         // 0x6D 'm'
    0b01010100,                // 0x6E 'n'
    0b01011100,                // 0x6F 'o'
    0b01110011,                // 0x70 'p'
    0b01100111,                // 0x71 'q'
    0b01010000,                // 0x72 'r'
    0b01101101,                // 0x73 's' (like 5)
    0b01111000,                // 0x74 't'
    0b00011100,                // 0x75 'u'
    0b00011100,                // 0x76 'v' (like u)
    0,                         // 0x77 'w'
    0,                         // 0x78 'x'
    0b01101110,                // 0x79 'y'
    0,                         // 0x7A 'z'
    0,                         // 0x7B '{'
    0b00110000,                // 0x7C '|' (like l)
    0,                         // 0x7D '}'
    0,                         // 0x7E '~'
    0                          // 0x7F DEL
};

SevenSegmentDisplay::SevenSegmentDisplay(size_t numDigits)
    : m_digits(numDigits, 0)
{
}

void SevenSegmentDisplay::setSegments(size_t digit, uint8_t segments)
{
    if (digit < m_digits.size()) {
        m_digits[digit] = segments;
    }
}

uint8_t SevenSegmentDisplay::getSegments(size_t digit) const
{
    if (digit < m_digits.size()) {
        return m_digits[digit];
    }
    return 0;
}

void SevenSegmentDisplay::setHexDigit(size_t digit, uint8_t value)
{
    if (digit < m_digits.size() && value < 16) {
        m_digits[digit] = HEX_PATTERNS[value];
    }
}

void SevenSegmentDisplay::setBCDDigit(size_t digit, uint8_t value)
{
    if (digit < m_digits.size() && value < 10) {
        m_digits[digit] = BCD_PATTERNS[value];
    }
}

void SevenSegmentDisplay::setSegment(size_t digit, char segment, bool on)
{
    if (digit >= m_digits.size()) return;

    uint8_t bit = charToSegmentBit(segment);
    if (bit < 8) {
        if (on) {
            m_digits[digit] |= (1u << bit);
        } else {
            m_digits[digit] &= ~(1u << bit);
        }
    }
}

bool SevenSegmentDisplay::getSegment(size_t digit, char segment) const
{
    if (digit >= m_digits.size()) return false;

    uint8_t bit = charToSegmentBit(segment);
    if (bit < 8) {
        return (m_digits[digit] >> bit) & 1;
    }
    return false;
}

void SevenSegmentDisplay::setDecimalPoint(size_t digit, bool on)
{
    setSegment(digit, '.', on);
}

bool SevenSegmentDisplay::getDecimalPoint(size_t digit) const
{
    return getSegment(digit, '.');
}

void SevenSegmentDisplay::clear()
{
    for (size_t i = 0; i < m_digits.size(); ++i) {
        m_digits[i] = 0;
    }
}

void SevenSegmentDisplay::clearDigit(size_t digit)
{
    if (digit < m_digits.size()) {
        m_digits[digit] = 0;
    }
}

std::string SevenSegmentDisplay::toASCII(size_t digit) const
{
    if (digit >= m_digits.size()) {
        return "       \n       \n       ";
    }

    uint8_t seg = m_digits[digit];
    std::ostringstream oss;

    // Line 1: top
    oss << " ";
    oss << (seg & MASK_A ? "_" : " ");
    oss << " \n";

    // Line 2: middle
    oss << (seg & MASK_F ? "|" : " ");
    oss << (seg & MASK_G ? "_" : " ");
    oss << (seg & MASK_B ? "|" : " ");
    oss << (seg & MASK_DP ? "." : " ");
    oss << "\n";

    // Line 3: bottom
    oss << (seg & MASK_E ? "|" : " ");
    oss << (seg & MASK_D ? "_" : " ");
    oss << (seg & MASK_C ? "|" : " ");

    return oss.str();
}

std::string SevenSegmentDisplay::toASCIIAll() const
{
    std::ostringstream oss;

    for (size_t i = 0; i < m_digits.size(); ++i) {
        if (i > 0) oss << "  ";
        oss << toASCII(i);
    }

    return oss.str();
}

void SevenSegmentDisplay::setHexValue(uint32_t value)
{
    size_t numDigits = m_digits.size();

    for (size_t i = 0; i < numDigits; ++i) {
        size_t digitIndex = numDigits - 1 - i;  // Right to left
        uint8_t hexDigit = (value >> (i * 4)) & 0x0F;
        setHexDigit(digitIndex, hexDigit);
    }
}

void SevenSegmentDisplay::setDecimalValue(uint32_t value)
{
    size_t numDigits = m_digits.size();
    clear();

    if (value == 0) {
        setHexDigit(numDigits - 1, 0);
        return;
    }

    size_t digitPos = numDigits - 1;
    while (value > 0 && digitPos < numDigits) {
        uint8_t digit = value % 10;
        setBCDDigit(digitPos, digit);
        value /= 10;
        if (digitPos == 0) break;
        --digitPos;
    }
}

void SevenSegmentDisplay::setString(const char* str)
{
    if (!str) return;

    clear();

    size_t len = 0;
    while (str[len] && len < m_digits.size()) {
        ++len;
    }

    for (size_t i = 0; i < len && i < m_digits.size(); ++i) {
        char c = str[i];
        if (c >= 0 && c < 128) {
            m_digits[i] = CHAR_PATTERNS[static_cast<uint8_t>(c)];
        }
    }
}

uint8_t SevenSegmentDisplay::charToSegmentBit(char segment)
{
    switch (segment) {
        case 'a': case 'A': return SEG_A;
        case 'b': case 'B': return SEG_B;
        case 'c': case 'C': return SEG_C;
        case 'd': case 'D': return SEG_D;
        case 'e': case 'E': return SEG_E;
        case 'f': case 'F': return SEG_F;
        case 'g': case 'G': return SEG_G;
        case '.': case 'p': case 'P': return SEG_DP;
        default: return 0xFF;  // Invalid
    }
}
