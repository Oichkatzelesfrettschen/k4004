#pragma once
#include "emulator_core/source/nibble.hpp"

// Intel 4004/4040 Nibble Pair (8-bit value as two 4-bit nibbles)
// ================================================================
//
// The Intel 4004/4040 often operates on paired registers to handle 8-bit values:
// - Instructions span 8 bits (two nibbles)
// - Addresses can be 8-12 bits (multiple nibbles)
// - Some operations work with register pairs (RP0-RP7)
//
// NibblePair represents an 8-bit value as LOW and HIGH nibbles,
// maintaining the hardware-accurate nibble-oriented architecture.

class NibblePair
{
public:
    // Construction
    NibblePair() : m_low(0), m_high(0) {}
    NibblePair(Nibble low, Nibble high) : m_low(low), m_high(high) {}
    explicit NibblePair(uint8_t byte)
        : m_low(byte & 0x0F), m_high((byte >> 4) & 0x0F) {}

    // Access individual nibbles
    Nibble low() const { return m_low; }
    Nibble high() const { return m_high; }

    void setLow(Nibble n) { m_low = n; }
    void setHigh(Nibble n) { m_high = n; }

    // Convert to/from byte
    uint8_t toByte() const {
        return m_low.value() | (m_high.value() << 4);
    }

    void fromByte(uint8_t byte) {
        m_low = Nibble(byte & 0x0F);
        m_high = Nibble((byte >> 4) & 0x0F);
    }

    // Arithmetic operations (8-bit)
    NibblePair operator+(const NibblePair& other) const {
        uint8_t sum = toByte() + other.toByte();
        return NibblePair(sum);
    }

    NibblePair operator-(const NibblePair& other) const {
        uint8_t diff = toByte() - other.toByte();
        return NibblePair(diff);
    }

    NibblePair& operator+=(const NibblePair& other) {
        fromByte(toByte() + other.toByte());
        return *this;
    }

    NibblePair& operator-=(const NibblePair& other) {
        fromByte(toByte() - other.toByte());
        return *this;
    }

    // Increment/Decrement
    NibblePair& operator++() {
        bool carry;
        m_low.addWithCarry(Nibble(1), false, carry);
        if (carry) {
            m_high.addWithCarry(Nibble(1), false, carry);
        }
        return *this;
    }

    NibblePair operator++(int) {
        NibblePair temp = *this;
        ++(*this);
        return temp;
    }

    NibblePair& operator--() {
        bool borrow;
        m_low.subWithBorrow(Nibble(1), false, borrow);
        if (borrow) {
            m_high.subWithBorrow(Nibble(1), false, borrow);
        }
        return *this;
    }

    NibblePair operator--(int) {
        NibblePair temp = *this;
        --(*this);
        return temp;
    }

    // Bitwise operations
    NibblePair operator&(const NibblePair& other) const {
        return NibblePair(m_low & other.m_low, m_high & other.m_high);
    }

    NibblePair operator|(const NibblePair& other) const {
        return NibblePair(m_low | other.m_low, m_high | other.m_high);
    }

    NibblePair operator^(const NibblePair& other) const {
        return NibblePair(m_low ^ other.m_low, m_high ^ other.m_high);
    }

    NibblePair operator~() const {
        return NibblePair(~m_low, ~m_high);
    }

    NibblePair& operator&=(const NibblePair& other) {
        m_low &= other.m_low;
        m_high &= other.m_high;
        return *this;
    }

    NibblePair& operator|=(const NibblePair& other) {
        m_low |= other.m_low;
        m_high |= other.m_high;
        return *this;
    }

    NibblePair& operator^=(const NibblePair& other) {
        m_low ^= other.m_low;
        m_high ^= other.m_high;
        return *this;
    }

    // Comparison
    bool operator==(const NibblePair& other) const {
        return m_low == other.m_low && m_high == other.m_high;
    }

    bool operator!=(const NibblePair& other) const {
        return !(*this == other);
    }

    bool operator<(const NibblePair& other) const {
        return toByte() < other.toByte();
    }

    bool operator<=(const NibblePair& other) const {
        return toByte() <= other.toByte();
    }

    bool operator>(const NibblePair& other) const {
        return toByte() > other.toByte();
    }

    bool operator>=(const NibblePair& other) const {
        return toByte() >= other.toByte();
    }

    // Multi-nibble addition with carry propagation
    bool addWithCarry(const NibblePair& other, bool carry_in, bool& carry_out) {
        bool carry_mid;

        // Add low nibbles
        m_low.addWithCarry(other.m_low, carry_in, carry_mid);

        // Add high nibbles with carry from low
        m_high.addWithCarry(other.m_high, carry_mid, carry_out);

        return carry_out;
    }

    bool subWithBorrow(const NibblePair& other, bool borrow_in, bool& borrow_out) {
        bool borrow_mid;

        // Subtract low nibbles
        m_low.subWithBorrow(other.m_low, borrow_in, borrow_mid);

        // Subtract high nibbles with borrow from low
        m_high.subWithBorrow(other.m_high, borrow_mid, borrow_out);

        return borrow_out;
    }

    // BCD operations (each nibble is a BCD digit)
    bool isValidBCD() const {
        return m_low.isValidBCD() && m_high.isValidBCD();
    }

    // Add two BCD byte values (each nibble is 0-9)
    bool addBCD(const NibblePair& other, bool carry_in, bool& carry_out) {
        bool carry_mid;

        // Add low BCD nibbles
        m_low.addBCD(other.m_low, carry_in, carry_mid);

        // Add high BCD nibbles
        m_high.addBCD(other.m_high, carry_mid, carry_out);

        return carry_out;
    }

    // Utility
    bool isZero() const {
        return m_low.isZero() && m_high.isZero();
    }

    bool isMax() const {
        return m_low.isMax() && m_high.isMax();
    }

    // Swap nibbles
    void swap() {
        Nibble temp = m_low;
        m_low = m_high;
        m_high = temp;
    }

    NibblePair swapped() const {
        return NibblePair(m_high, m_low);
    }

private:
    Nibble m_low;   // Bits 0-3
    Nibble m_high;  // Bits 4-7
};
