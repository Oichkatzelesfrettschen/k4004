#pragma once
#include "emulator_core/source/nibble.hpp"

// Intel 4004/4040 12-Bit Address (three nibbles)
// ===============================================
//
// The Intel 4004 uses 12-bit addresses for ROM access:
// - 4096 bytes of ROM address space (0x000 - 0xFFF)
// - Address represented as 3 nibbles (4 bits each)
// - Nibble 0: Low (bits 0-3)
// - Nibble 1: Mid (bits 4-7)
// - Nibble 2: High (bits 8-11)
//
// The 4040 can address up to 8KB with banking, but still uses
// 12-bit addresses within each bank.

class Address12
{
public:
    // Construction
    Address12() : m_nibbles{Nibble(0), Nibble(0), Nibble(0)} {}

    explicit Address12(uint16_t addr) {
        m_nibbles[0] = Nibble((addr >> 0) & 0x0F);  // Low nibble
        m_nibbles[1] = Nibble((addr >> 4) & 0x0F);  // Mid nibble
        m_nibbles[2] = Nibble((addr >> 8) & 0x0F);  // High nibble
    }

    Address12(Nibble n0, Nibble n1, Nibble n2)
        : m_nibbles{n0, n1, n2} {}

    // Access individual nibbles
    Nibble nibble0() const { return m_nibbles[0]; }  // Low (bits 0-3)
    Nibble nibble1() const { return m_nibbles[1]; }  // Mid (bits 4-7)
    Nibble nibble2() const { return m_nibbles[2]; }  // High (bits 8-11)

    void setNibble0(Nibble n) { m_nibbles[0] = n; }
    void setNibble1(Nibble n) { m_nibbles[1] = n; }
    void setNibble2(Nibble n) { m_nibbles[2] = n; }

    // Access by index (0-2)
    Nibble nibble(uint8_t index) const {
        if (index >= 3) return Nibble(0);
        return m_nibbles[index];
    }

    void setNibble(uint8_t index, Nibble n) {
        if (index < 3) {
            m_nibbles[index] = n;
        }
    }

    // Convert to uint16_t
    uint16_t toUint16() const {
        return static_cast<uint16_t>(m_nibbles[0].value())
             | (static_cast<uint16_t>(m_nibbles[1].value()) << 4)
             | (static_cast<uint16_t>(m_nibbles[2].value()) << 8);
    }

    // Convert from uint16_t
    void fromUint16(uint16_t addr) {
        m_nibbles[0] = Nibble((addr >> 0) & 0x0F);
        m_nibbles[1] = Nibble((addr >> 4) & 0x0F);
        m_nibbles[2] = Nibble((addr >> 8) & 0x0F);
    }

    // Get low byte (nibbles 0-1)
    uint8_t lowByte() const {
        return m_nibbles[0].value() | (m_nibbles[1].value() << 4);
    }

    // Get high nibble (nibble 2)
    Nibble highNibble() const {
        return m_nibbles[2];
    }

    // Set from low byte and high nibble
    void setFromBytes(uint8_t lowByte, Nibble highNibble) {
        m_nibbles[0] = Nibble(lowByte & 0x0F);
        m_nibbles[1] = Nibble((lowByte >> 4) & 0x0F);
        m_nibbles[2] = highNibble;
    }

    // Arithmetic operations
    Address12 operator+(uint16_t offset) const {
        return Address12((toUint16() + offset) & 0x0FFF);
    }

    Address12 operator-(uint16_t offset) const {
        return Address12((toUint16() - offset) & 0x0FFF);
    }

    Address12& operator+=(uint16_t offset) {
        fromUint16((toUint16() + offset) & 0x0FFF);
        return *this;
    }

    Address12& operator-=(uint16_t offset) {
        fromUint16((toUint16() - offset) & 0x0FFF);
        return *this;
    }

    // Increment/Decrement with 12-bit wrapping
    Address12& operator++() {
        bool carry0, carry1;
        m_nibbles[0].addWithCarry(Nibble(1), false, carry0);
        if (carry0) {
            m_nibbles[1].addWithCarry(Nibble(1), false, carry1);
            if (carry1) {
                m_nibbles[2].addWithCarry(Nibble(1), false, carry0);
            }
        }
        return *this;
    }

    Address12 operator++(int) {
        Address12 temp = *this;
        ++(*this);
        return temp;
    }

    Address12& operator--() {
        bool borrow0, borrow1;
        m_nibbles[0].subWithBorrow(Nibble(1), false, borrow0);
        if (borrow0) {
            m_nibbles[1].subWithBorrow(Nibble(1), false, borrow1);
            if (borrow1) {
                m_nibbles[2].subWithBorrow(Nibble(1), false, borrow0);
            }
        }
        return *this;
    }

    Address12 operator--(int) {
        Address12 temp = *this;
        --(*this);
        return temp;
    }

    // Comparison
    bool operator==(const Address12& other) const {
        return m_nibbles[0] == other.m_nibbles[0]
            && m_nibbles[1] == other.m_nibbles[1]
            && m_nibbles[2] == other.m_nibbles[2];
    }

    bool operator!=(const Address12& other) const {
        return !(*this == other);
    }

    bool operator<(const Address12& other) const {
        return toUint16() < other.toUint16();
    }

    bool operator<=(const Address12& other) const {
        return toUint16() <= other.toUint16();
    }

    bool operator>(const Address12& other) const {
        return toUint16() > other.toUint16();
    }

    bool operator>=(const Address12& other) const {
        return toUint16() >= other.toUint16();
    }

    // Utility
    bool isZero() const {
        return m_nibbles[0].isZero() && m_nibbles[1].isZero() && m_nibbles[2].isZero();
    }

    bool isMax() const {  // 0xFFF
        return m_nibbles[0].isMax() && m_nibbles[1].isMax() && m_nibbles[2].isMax();
    }

    // Page operations (4004 has 256-byte pages, 16 pages total)
    uint8_t getPage() const {  // Returns 0-15 (page number)
        return (m_nibbles[2].value() << 4) | m_nibbles[1].value();
    }

    uint8_t getPageOffset() const {  // Returns 0-255 (offset within page)
        return m_nibbles[0].value();
    }

    void setPage(uint8_t page) {
        m_nibbles[1] = Nibble((page >> 0) & 0x0F);
        m_nibbles[2] = Nibble((page >> 4) & 0x0F);
    }

    void setPageOffset(uint8_t offset) {
        m_nibbles[0] = Nibble(offset & 0x0F);
    }

private:
    Nibble m_nibbles[3];  // [0] = low, [1] = mid, [2] = high
};

// Common address constants
namespace Address12Constants {
    constexpr uint16_t MIN_ADDR = 0x000;
    constexpr uint16_t MAX_ADDR = 0xFFF;
    constexpr uint16_t ROM_SIZE_4004 = 4096;  // 4KB = 12-bit address space
}
