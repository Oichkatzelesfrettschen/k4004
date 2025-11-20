#pragma once
#include <cstdint>
#include <cstddef>

// Intel 4004/4040 Native 4-Bit Architecture
// ==========================================
//
// This file implements the fundamental 4-bit data type (Nibble) that forms
// the basis of the Intel 4004/4040 microprocessor architecture.
//
// Historical Context:
// The Intel 4004 (1971) was a TRUE 4-bit processor:
// - 4-bit data bus
// - 4-bit ALU operations
// - 4-bit registers (16 registers, pairwise for 8-bit ops)
// - All arithmetic operates on nibbles
// - BCD (Binary Coded Decimal) primary use case
//
// This implementation provides type-safe nibble operations that enforce
// 4-bit semantics at compile time, ensuring hardware-accurate emulation.

class Nibble
{
public:
    // Construction
    Nibble() : m_value(0) {}
    explicit Nibble(uint8_t val) : m_value(val & 0x0Fu) {}

    // Access
    uint8_t value() const { return m_value; }
    void setValue(uint8_t val) { m_value = val & 0x0Fu; }

    // Arithmetic operations (4-bit, wrapping)
    Nibble operator+(const Nibble& other) const {
        return Nibble((m_value + other.m_value) & 0x0Fu);
    }

    Nibble operator-(const Nibble& other) const {
        return Nibble((m_value - other.m_value) & 0x0Fu);
    }

    Nibble& operator+=(const Nibble& other) {
        m_value = (m_value + other.m_value) & 0x0Fu;
        return *this;
    }

    Nibble& operator-=(const Nibble& other) {
        m_value = (m_value - other.m_value) & 0x0Fu;
        return *this;
    }

    // Increment/Decrement
    Nibble& operator++() {  // Pre-increment
        m_value = (m_value + 1) & 0x0Fu;
        return *this;
    }

    Nibble operator++(int) {  // Post-increment
        Nibble temp = *this;
        m_value = (m_value + 1) & 0x0Fu;
        return temp;
    }

    Nibble& operator--() {  // Pre-decrement
        m_value = (m_value - 1) & 0x0Fu;
        return *this;
    }

    Nibble operator--(int) {  // Post-decrement
        Nibble temp = *this;
        m_value = (m_value - 1) & 0x0Fu;
        return temp;
    }

    // Bitwise operations
    Nibble operator&(const Nibble& other) const {
        return Nibble(m_value & other.m_value);
    }

    Nibble operator|(const Nibble& other) const {
        return Nibble(m_value | other.m_value);
    }

    Nibble operator^(const Nibble& other) const {
        return Nibble(m_value ^ other.m_value);
    }

    Nibble operator~() const {
        return Nibble((~m_value) & 0x0Fu);
    }

    Nibble& operator&=(const Nibble& other) {
        m_value &= other.m_value;
        return *this;
    }

    Nibble& operator|=(const Nibble& other) {
        m_value |= other.m_value;
        return *this;
    }

    Nibble& operator^=(const Nibble& other) {
        m_value ^= other.m_value;
        return *this;
    }

    // Shift operations (within 4-bit boundary)
    Nibble operator<<(uint8_t shift) const {
        return Nibble((m_value << shift) & 0x0Fu);
    }

    Nibble operator>>(uint8_t shift) const {
        return Nibble((m_value >> shift) & 0x0Fu);
    }

    Nibble& operator<<=(uint8_t shift) {
        m_value = (m_value << shift) & 0x0Fu;
        return *this;
    }

    Nibble& operator>>=(uint8_t shift) {
        m_value = (m_value >> shift) & 0x0Fu;
        return *this;
    }

    // Comparison operations
    bool operator==(const Nibble& other) const {
        return m_value == other.m_value;
    }

    bool operator!=(const Nibble& other) const {
        return m_value != other.m_value;
    }

    bool operator<(const Nibble& other) const {
        return m_value < other.m_value;
    }

    bool operator<=(const Nibble& other) const {
        return m_value <= other.m_value;
    }

    bool operator>(const Nibble& other) const {
        return m_value > other.m_value;
    }

    bool operator>=(const Nibble& other) const {
        return m_value >= other.m_value;
    }

    // Carry/Borrow detection (critical for multi-nibble arithmetic)
    bool addWithCarry(const Nibble& other, bool carry_in, bool& carry_out) {
        uint8_t sum = m_value + other.m_value + (carry_in ? 1 : 0);
        carry_out = (sum > 0x0F);
        m_value = sum & 0x0Fu;
        return carry_out;
    }

    bool subWithBorrow(const Nibble& other, bool borrow_in, bool& borrow_out) {
        int16_t diff = static_cast<int16_t>(m_value) - static_cast<int16_t>(other.m_value) - (borrow_in ? 1 : 0);
        borrow_out = (diff < 0);
        m_value = static_cast<uint8_t>(diff) & 0x0Fu;
        return borrow_out;
    }

    // Static helpers for carry detection without modifying value
    static bool hasAddCarry(const Nibble& a, const Nibble& b, bool carry_in = false) {
        uint8_t sum = a.m_value + b.m_value + (carry_in ? 1 : 0);
        return sum > 0x0F;
    }

    static bool hasSubBorrow(const Nibble& a, const Nibble& b, bool borrow_in = false) {
        int16_t diff = static_cast<int16_t>(a.m_value) - static_cast<int16_t>(b.m_value) - (borrow_in ? 1 : 0);
        return diff < 0;
    }

    // BCD (Binary Coded Decimal) operations
    // BCD uses only 0-9 (0x0-0x9), values 0xA-0xF are invalid
    bool isValidBCD() const {
        return m_value <= 9;
    }

    // Decimal Adjust After Addition (DAA equivalent for nibbles)
    void decimalAdjust() {
        if (m_value > 9) {
            m_value = 0x0F;  // Saturate to max nibble value
        }
    }

    // Convert to BCD (clamp to 0-9)
    Nibble toBCD() const {
        return Nibble(m_value <= 9 ? m_value : 9);
    }

    // BCD addition with carry
    bool addBCD(const Nibble& other, bool carry_in, bool& carry_out) {
        uint8_t sum = m_value + other.m_value + (carry_in ? 1 : 0);

        if (sum > 9) {
            carry_out = true;
            m_value = (sum - 10) & 0x0Fu;
        } else {
            carry_out = false;
            m_value = sum;
        }

        return carry_out;
    }

    // Bit operations
    bool getBit(uint8_t bit) const {
        if (bit >= 4) return false;
        return (m_value >> bit) & 1;
    }

    void setBit(uint8_t bit, bool val) {
        if (bit >= 4) return;

        if (val) {
            m_value |= (1u << bit);
        } else {
            m_value &= ~(1u << bit);
        }
    }

    void toggleBit(uint8_t bit) {
        if (bit >= 4) return;
        m_value ^= (1u << bit);
    }

    // Rotate operations (4-bit circular)
    Nibble rotateLeft(uint8_t count = 1) const {
        count &= 3;  // Modulo 4
        return Nibble(((m_value << count) | (m_value >> (4 - count))) & 0x0Fu);
    }

    Nibble rotateRight(uint8_t count = 1) const {
        count &= 3;  // Modulo 4
        return Nibble(((m_value >> count) | (m_value << (4 - count))) & 0x0Fu);
    }

    // Utility
    bool isZero() const { return m_value == 0; }
    bool isMax() const { return m_value == 0x0F; }

    // Conversion to/from bool (for conditional logic)
    explicit operator bool() const { return m_value != 0; }

private:
    uint8_t m_value;  // Only bits 0-3 are used (0x00-0x0F)
};
