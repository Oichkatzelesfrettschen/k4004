#pragma once
#include "emulator_core/source/nibble.hpp"
#include <array>
#include <cstddef>
#include <cstring>

// Intel 4004/4040 Nibble Array
// =============================
//
// Fixed-size array of nibbles for representing memory and registers.
//
// Usage:
// - 4002 RAM: NibbleArray<80> (40 bytes = 80 nibbles)
// - 4001 ROM: NibbleArray<512> (256 bytes = 512 nibbles for data)
// - 4101 SRAM: NibbleArray<512> (256×4 bits = 512 nibbles)
// - Registers: NibbleArray<16> (16 4-bit registers)
//
// This maintains hardware accuracy by storing data as discrete nibbles
// rather than bytes, matching the true 4-bit architecture.

template<size_t N>
class NibbleArray
{
public:
    // Construction
    NibbleArray() {
        fill(Nibble(0));
    }

    explicit NibbleArray(Nibble fillValue) {
        fill(fillValue);
    }

    // Element access
    Nibble& operator[](size_t index) {
        return m_data[index];
    }

    const Nibble& operator[](size_t index) const {
        return m_data[index];
    }

    Nibble& at(size_t index) {
        if (index >= N) {
            // In a real implementation, throw std::out_of_range
            // For now, return first element as safe fallback
            static Nibble dummy(0);
            return dummy;
        }
        return m_data[index];
    }

    const Nibble& at(size_t index) const {
        if (index >= N) {
            static const Nibble dummy(0);
            return dummy;
        }
        return m_data[index];
    }

    // Size
    constexpr size_t size() const { return N; }
    constexpr size_t sizeInBytes() const { return (N + 1) / 2; }  // Rounded up

    // Fill operations
    void fill(Nibble value) {
        for (size_t i = 0; i < N; ++i) {
            m_data[i] = value;
        }
    }

    void fillRange(size_t start, size_t count, Nibble value) {
        size_t end = (start + count < N) ? (start + count) : N;
        for (size_t i = start; i < end; ++i) {
            m_data[i] = value;
        }
    }

    void clear() {
        fill(Nibble(0));
    }

    // Byte operations (convert pairs of nibbles to/from bytes)
    // Nibbles are stored as [low, high, low, high, ...]
    uint8_t readByte(size_t byteIndex) const {
        size_t nibbleIndex = byteIndex * 2;
        if (nibbleIndex + 1 >= N) return 0;

        return m_data[nibbleIndex].value()
             | (m_data[nibbleIndex + 1].value() << 4);
    }

    void writeByte(size_t byteIndex, uint8_t byte) {
        size_t nibbleIndex = byteIndex * 2;
        if (nibbleIndex + 1 >= N) return;

        m_data[nibbleIndex] = Nibble(byte & 0x0F);
        m_data[nibbleIndex + 1] = Nibble((byte >> 4) & 0x0F);
    }

    // Load from byte array (for ROM/RAM initialization)
    void loadFromBytes(const uint8_t* bytes, size_t byteCount) {
        size_t count = (byteCount * 2 < N) ? (byteCount * 2) : N;

        for (size_t i = 0; i < count; i += 2) {
            size_t byteIndex = i / 2;
            m_data[i] = Nibble(bytes[byteIndex] & 0x0F);
            if (i + 1 < N) {
                m_data[i + 1] = Nibble((bytes[byteIndex] >> 4) & 0x0F);
            }
        }
    }

    // Save to byte array
    void saveToBytes(uint8_t* bytes, size_t byteCount) const {
        size_t count = (byteCount * 2 < N) ? (byteCount * 2) : N;

        for (size_t i = 0; i < count; i += 2) {
            size_t byteIndex = i / 2;
            uint8_t byte = m_data[i].value();
            if (i + 1 < N) {
                byte |= (m_data[i + 1].value() << 4);
            }
            bytes[byteIndex] = byte;
        }
    }

    // Copy operations
    void copyFrom(const NibbleArray<N>& other) {
        for (size_t i = 0; i < N; ++i) {
            m_data[i] = other.m_data[i];
        }
    }

    void copyRange(const NibbleArray<N>& other, size_t srcStart, size_t dstStart, size_t count) {
        for (size_t i = 0; i < count && (srcStart + i) < N && (dstStart + i) < N; ++i) {
            m_data[dstStart + i] = other.m_data[srcStart + i];
        }
    }

    // Comparison
    bool operator==(const NibbleArray<N>& other) const {
        for (size_t i = 0; i < N; ++i) {
            if (m_data[i] != other.m_data[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const NibbleArray<N>& other) const {
        return !(*this == other);
    }

    // Check if all nibbles are zero
    bool isAllZero() const {
        for (size_t i = 0; i < N; ++i) {
            if (!m_data[i].isZero()) {
                return false;
            }
        }
        return true;
    }

    // Check if all nibbles match a value
    bool isAllValue(Nibble value) const {
        for (size_t i = 0; i < N; ++i) {
            if (m_data[i] != value) {
                return false;
            }
        }
        return true;
    }

    // Count matching nibbles
    size_t countValue(Nibble value) const {
        size_t count = 0;
        for (size_t i = 0; i < N; ++i) {
            if (m_data[i] == value) {
                ++count;
            }
        }
        return count;
    }

    // Iterators (for range-based for loops)
    Nibble* begin() { return m_data.data(); }
    const Nibble* begin() const { return m_data.data(); }
    Nibble* end() { return m_data.data() + N; }
    const Nibble* end() const { return m_data.data() + N; }

    // Raw data access (for advanced use)
    Nibble* data() { return m_data.data(); }
    const Nibble* data() const { return m_data.data(); }

private:
    std::array<Nibble, N> m_data;
};

// Common nibble array sizes for Intel MCS-4/40 chips
namespace NibbleArraySizes {
    // 4001 ROM: 256 bytes = 512 nibbles
    using ROM_4001 = NibbleArray<512>;

    // 4002 RAM: 40 bytes (80 nibbles)
    // Organized as 4 registers × (16 main + 4 status) characters
    using RAM_4002 = NibbleArray<80>;

    // 4101 SRAM: 256×4 bits = 256 nibbles
    using SRAM_4101 = NibbleArray<256>;

    // CPU Registers: 16 × 4-bit registers
    using CPU_REGISTERS = NibbleArray<16>;

    // Stack (4004): 3 levels × 12 bits = 9 nibbles
    using STACK_4004 = NibbleArray<9>;

    // Stack (4040): 7 levels × 12 bits = 21 nibbles
    using STACK_4040 = NibbleArray<21>;
}
