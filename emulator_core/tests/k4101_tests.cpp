#include <gtest/gtest.h>
#include "emulator_core/source/K4101.hpp"

class K4101Test : public ::testing::Test {
protected:
    K4101 ram;
};

// Test basic construction and initialization
TEST_F(K4101Test, Construction) {
    // All memory should be initialized to 0
    const uint8_t* memory = ram.getMemoryContents();
    for (uint16_t i = 0; i < K4101::RAM_SIZE; ++i) {
        EXPECT_EQ(memory[i], 0u);
    }
    EXPECT_TRUE(ram.isChipEnabled());
}

// Test reset functionality
TEST_F(K4101Test, Reset) {
    // Write some data
    ram.write(0, 0xF);
    ram.write(100, 0xA);
    ram.write(255, 0x5);

    // Reset
    ram.reset();

    // All memory should be 0
    const uint8_t* memory = ram.getMemoryContents();
    for (uint16_t i = 0; i < K4101::RAM_SIZE; ++i) {
        EXPECT_EQ(memory[i], 0u);
    }
}

// Test basic write and read
TEST_F(K4101Test, WriteRead) {
    ram.write(0, 0xA);
    EXPECT_EQ(ram.read(0), 0xA);

    ram.write(100, 0x5);
    EXPECT_EQ(ram.read(100), 0x5);

    ram.write(255, 0xF);
    EXPECT_EQ(ram.read(255), 0xF);
}

// Test 4-bit value masking
TEST_F(K4101Test, FourBitMasking) {
    // Write values with upper bits set - should be masked to 4 bits
    ram.write(0, 0xFF);
    EXPECT_EQ(ram.read(0), 0x0F);

    ram.write(10, 0xAB);
    EXPECT_EQ(ram.read(10), 0x0B);

    ram.write(20, 0x34);
    EXPECT_EQ(ram.read(20), 0x04);
}

// Test all addresses
TEST_F(K4101Test, AllAddresses) {
    // Write unique values to all 256 addresses
    for (uint16_t addr = 0; addr < K4101::RAM_SIZE; ++addr) {
        uint8_t value = (addr & 0x0F);  // Use lower 4 bits of address
        ram.write(addr, value);
    }

    // Read back and verify
    for (uint16_t addr = 0; addr < K4101::RAM_SIZE; ++addr) {
        uint8_t expected = (addr & 0x0F);
        EXPECT_EQ(ram.read(addr), expected);
    }
}

// Test chip enable functionality
TEST_F(K4101Test, ChipEnable) {
    // Write with chip enabled
    ram.write(50, 0xC);
    EXPECT_EQ(ram.read(50), 0xC);

    // Disable chip
    ram.setChipEnable(false);
    EXPECT_FALSE(ram.isChipEnabled());

    // Try to write - should fail
    ram.write(50, 0x3);
    EXPECT_EQ(ram.read(50), 0xC);  // Should still be old value

    // Read should return 0 when disabled
    ram.write(60, 0x7);  // This shouldn't work
    EXPECT_EQ(ram.read(60), 0u);   // Disabled read returns 0

    // Re-enable chip
    ram.setChipEnable(true);
    EXPECT_TRUE(ram.isChipEnabled());

    // Original data should be preserved
    EXPECT_EQ(ram.read(50), 0xC);

    // New writes should work
    ram.write(50, 0x3);
    EXPECT_EQ(ram.read(50), 0x3);
}

// Test data independence (write to one address doesn't affect others)
TEST_F(K4101Test, DataIndependence) {
    // Write to multiple addresses
    ram.write(0, 0x1);
    ram.write(1, 0x2);
    ram.write(2, 0x3);
    ram.write(100, 0xA);
    ram.write(200, 0xB);

    // Verify each independently
    EXPECT_EQ(ram.read(0), 0x1);
    EXPECT_EQ(ram.read(1), 0x2);
    EXPECT_EQ(ram.read(2), 0x3);
    EXPECT_EQ(ram.read(100), 0xA);
    EXPECT_EQ(ram.read(200), 0xB);

    // Unwritten addresses should still be 0
    EXPECT_EQ(ram.read(3), 0x0);
    EXPECT_EQ(ram.read(50), 0x0);
    EXPECT_EQ(ram.read(150), 0x0);
}

// Test overwrite functionality
TEST_F(K4101Test, Overwrite) {
    // Write initial value
    ram.write(42, 0x7);
    EXPECT_EQ(ram.read(42), 0x7);

    // Overwrite with new value
    ram.write(42, 0xE);
    EXPECT_EQ(ram.read(42), 0xE);

    // Overwrite again
    ram.write(42, 0x0);
    EXPECT_EQ(ram.read(42), 0x0);
}

// Test typical usage pattern (sequential access)
TEST_F(K4101Test, SequentialAccess) {
    // Write sequential pattern
    for (uint8_t i = 0; i < 16; ++i) {
        ram.write(i, i);
    }

    // Read sequential pattern
    for (uint8_t i = 0; i < 16; ++i) {
        EXPECT_EQ(ram.read(i), i);
    }
}

// Test typical usage pattern (lookup table)
TEST_F(K4101Test, LookupTable) {
    // Store a lookup table (e.g., BCD to 7-segment conversion)
    const uint8_t segmentTable[16] = {
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
        0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
    };

    for (uint8_t i = 0; i < 16; ++i) {
        ram.write(i, segmentTable[i]);
    }

    // Lookup values
    for (uint8_t i = 0; i < 16; ++i) {
        EXPECT_EQ(ram.read(i), segmentTable[i]);
    }
}

// Test comparison with 4002 RAM capacity
TEST_F(K4101Test, CapacityComparison) {
    // 4101: 256 × 4-bit = 1024 bits
    // 4002: 40 × 4-bit = 160 bits (data only)
    // 4101 is 6.4x larger

    EXPECT_EQ(K4101::RAM_SIZE, 256u);
    EXPECT_EQ(K4101::WORD_SIZE, 4u);

    // 4002 equivalent would be 40 nibbles
    // We can store 6+ times that amount
    const uint16_t ram4002_size = 40u;
    EXPECT_GT(K4101::RAM_SIZE, ram4002_size * 6u);
}

// Test access time specification
TEST_F(K4101Test, AccessTimeSpec) {
    // 4101 spec: 500 ns typical access time
    // 4002 spec: 1.2 μs typical access time
    // 4101 is 2.4x faster

    EXPECT_EQ(K4101::ACCESS_TIME_NS, 500u);

    // For reference: 4002 would be 1200 ns
    const uint16_t ram4002_access_time = 1200u;
    EXPECT_LT(K4101::ACCESS_TIME_NS, ram4002_access_time);
}
