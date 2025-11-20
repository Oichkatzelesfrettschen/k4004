#include <gtest/gtest.h>
#include "emulator_core/source/nibble.hpp"
#include "emulator_core/source/nibble_pair.hpp"
#include "emulator_core/source/address12.hpp"
#include "emulator_core/source/nibble_array.hpp"

// ============================================================================
// Nibble Tests
// ============================================================================

class NibbleTest : public ::testing::Test {
protected:
    Nibble n0{0};
    Nibble n5{5};
    Nibble nF{0xF};
    Nibble nA{0xA};
};

TEST_F(NibbleTest, Construction) {
    EXPECT_EQ(n0.value(), 0u);
    EXPECT_EQ(n5.value(), 5u);
    EXPECT_EQ(nF.value(), 0xFu);

    // Test overflow masking
    Nibble n_overflow{0x1F};
    EXPECT_EQ(n_overflow.value(), 0xFu);  // Should mask to 4 bits
}

TEST_F(NibbleTest, Arithmetic) {
    Nibble result = n5 + Nibble(3);
    EXPECT_EQ(result.value(), 8u);

    // Test wrapping
    Nibble wrap = nF + Nibble(1);
    EXPECT_EQ(wrap.value(), 0u);  // 0xF + 1 = 0x0 (wraps)

    // Subtraction
    result = n5 - Nibble(2);
    EXPECT_EQ(result.value(), 3u);

    // Subtraction with wrapping
    wrap = Nibble(0) - Nibble(1);
    EXPECT_EQ(wrap.value(), 0xFu);  // 0 - 1 = 0xF (wraps)
}

TEST_F(NibbleTest, IncrementDecrement) {
    Nibble n{5};

    // Pre-increment
    ++n;
    EXPECT_EQ(n.value(), 6u);

    // Post-increment
    Nibble old = n++;
    EXPECT_EQ(old.value(), 6u);
    EXPECT_EQ(n.value(), 7u);

    // Pre-decrement
    --n;
    EXPECT_EQ(n.value(), 6u);

    // Post-decrement
    old = n--;
    EXPECT_EQ(old.value(), 6u);
    EXPECT_EQ(n.value(), 5u);

    // Wrap on increment
    Nibble nmax{0xF};
    ++nmax;
    EXPECT_EQ(nmax.value(), 0u);

    // Wrap on decrement
    Nibble nzero{0};
    --nzero;
    EXPECT_EQ(nzero.value(), 0xFu);
}

TEST_F(NibbleTest, BitwiseOperations) {
    Nibble a{0xC};  // 1100
    Nibble b{0xA};  // 1010

    EXPECT_EQ((a & b).value(), 0x8u);  // 1000
    EXPECT_EQ((a | b).value(), 0xEu);  // 1110
    EXPECT_EQ((a ^ b).value(), 0x6u);  // 0110
    EXPECT_EQ((~a).value(), 0x3u);     // 0011
}

TEST_F(NibbleTest, ShiftOperations) {
    Nibble n{0xB};  // 1011

    EXPECT_EQ((n << 1).value(), 0x6u);  // 0110 (bit 3 shifted out)
    EXPECT_EQ((n >> 1).value(), 0x5u);  // 0101
    EXPECT_EQ((n << 2).value(), 0xCu);  // 1100
    EXPECT_EQ((n >> 2).value(), 0x2u);  // 0010
}

TEST_F(NibbleTest, Comparison) {
    EXPECT_TRUE(n5 == Nibble(5));
    EXPECT_FALSE(n5 == nF);
    EXPECT_TRUE(n5 != nF);
    EXPECT_TRUE(n5 < nF);
    EXPECT_TRUE(nF > n5);
    EXPECT_TRUE(n5 <= Nibble(5));
    EXPECT_TRUE(n5 >= Nibble(5));
}

TEST_F(NibbleTest, CarryDetection) {
    Nibble a{0xF};
    Nibble b{0x1};
    bool carry;

    a.addWithCarry(b, false, carry);
    EXPECT_TRUE(carry);
    EXPECT_EQ(a.value(), 0u);

    // No carry
    a = Nibble(0x5);
    b = Nibble(0x3);
    a.addWithCarry(b, false, carry);
    EXPECT_FALSE(carry);
    EXPECT_EQ(a.value(), 0x8u);

    // With carry in
    a = Nibble(0xE);
    b = Nibble(0);
    a.addWithCarry(b, true, carry);  // 0xE + 0 + 1 = 0xF, no carry
    EXPECT_FALSE(carry);
    EXPECT_EQ(a.value(), 0xFu);
}

TEST_F(NibbleTest, BorrowDetection) {
    Nibble a{0};
    Nibble b{1};
    bool borrow;

    a.subWithBorrow(b, false, borrow);
    EXPECT_TRUE(borrow);
    EXPECT_EQ(a.value(), 0xFu);

    // No borrow
    a = Nibble(0x8);
    b = Nibble(0x3);
    a.subWithBorrow(b, false, borrow);
    EXPECT_FALSE(borrow);
    EXPECT_EQ(a.value(), 0x5u);
}

TEST_F(NibbleTest, BCDOperations) {
    EXPECT_TRUE(Nibble(0).isValidBCD());
    EXPECT_TRUE(Nibble(9).isValidBCD());
    EXPECT_FALSE(Nibble(0xA).isValidBCD());
    EXPECT_FALSE(Nibble(0xF).isValidBCD());

    // BCD addition
    Nibble a{5};
    Nibble b{7};
    bool carry;

    a.addBCD(b, false, carry);
    EXPECT_TRUE(carry);
    EXPECT_EQ(a.value(), 2u);  // 5 + 7 = 12 BCD = carry with 2

    // BCD no carry
    a = Nibble(3);
    b = Nibble(4);
    a.addBCD(b, false, carry);
    EXPECT_FALSE(carry);
    EXPECT_EQ(a.value(), 7u);
}

TEST_F(NibbleTest, BitOperations) {
    Nibble n{0xA};  // 1010

    EXPECT_TRUE(n.getBit(1));
    EXPECT_FALSE(n.getBit(0));
    EXPECT_TRUE(n.getBit(3));
    EXPECT_FALSE(n.getBit(2));

    n.setBit(0, true);
    EXPECT_EQ(n.value(), 0xBu);  // 1011

    n.setBit(1, false);
    EXPECT_EQ(n.value(), 0x9u);  // 1001

    n.toggleBit(0);
    EXPECT_EQ(n.value(), 0x8u);  // 1000
}

TEST_F(NibbleTest, RotateOperations) {
    Nibble n{0x9};  // 1001

    EXPECT_EQ(n.rotateLeft(1).value(), 0x3u);   // 0011 (bit 3 wraps to bit 0)
    EXPECT_EQ(n.rotateRight(1).value(), 0xCu);  // 1100 (bit 0 wraps to bit 3)
    EXPECT_EQ(n.rotateLeft(2).value(), 0x6u);   // 0110
    EXPECT_EQ(n.rotateRight(2).value(), 0x6u);  // 0110
}

// ============================================================================
// NibblePair Tests
// ============================================================================

class NibblePairTest : public ::testing::Test {
protected:
    NibblePair p0{Nibble(0), Nibble(0)};
    NibblePair p_5A{Nibble(0xA), Nibble(0x5)};
    NibblePair p_byte{0xAB};
};

TEST_F(NibblePairTest, Construction) {
    EXPECT_EQ(p0.low().value(), 0u);
    EXPECT_EQ(p0.high().value(), 0u);

    EXPECT_EQ(p_5A.low().value(), 0xAu);
    EXPECT_EQ(p_5A.high().value(), 0x5u);

    EXPECT_EQ(p_byte.low().value(), 0xBu);
    EXPECT_EQ(p_byte.high().value(), 0xAu);
}

TEST_F(NibblePairTest, ByteConversion) {
    EXPECT_EQ(p_5A.toByte(), 0x5Au);
    EXPECT_EQ(p_byte.toByte(), 0xABu);

    NibblePair p;
    p.fromByte(0xCD);
    EXPECT_EQ(p.low().value(), 0xDu);
    EXPECT_EQ(p.high().value(), 0xCu);
    EXPECT_EQ(p.toByte(), 0xCDu);
}

TEST_F(NibblePairTest, Arithmetic) {
    NibblePair a{0x50};
    NibblePair b{0x30};

    NibblePair result = a + b;
    EXPECT_EQ(result.toByte(), 0x80u);

    result = a - b;
    EXPECT_EQ(result.toByte(), 0x20u);

    // With overflow
    a = NibblePair{0xFF};
    b = NibblePair{0x01};
    result = a + b;
    EXPECT_EQ(result.toByte(), 0x00u);  // Wraps
}

TEST_F(NibblePairTest, IncrementDecrement) {
    NibblePair p{0x0F};

    ++p;
    EXPECT_EQ(p.toByte(), 0x10u);  // Low nibble wraps, high nibble increments

    ++p;
    EXPECT_EQ(p.toByte(), 0x11u);

    --p;
    EXPECT_EQ(p.toByte(), 0x10u);

    --p;
    EXPECT_EQ(p.toByte(), 0x0Fu);  // High nibble decrements, low nibble wraps
}

TEST_F(NibblePairTest, CarryPropagation) {
    NibblePair a{Nibble(0xF), Nibble(0x5)};  // 0x5F
    NibblePair b{Nibble(0x1), Nibble(0x0)};  // 0x01
    bool carry;

    a.addWithCarry(b, false, carry);
    EXPECT_FALSE(carry);
    EXPECT_EQ(a.toByte(), 0x60u);  // 0x5F + 0x01 = 0x60

    // Test carry out
    a = NibblePair{0xFF};
    b = NibblePair{0x01};
    a.addWithCarry(b, false, carry);
    EXPECT_TRUE(carry);
    EXPECT_EQ(a.toByte(), 0x00u);
}

TEST_F(NibblePairTest, BCDOperations) {
    EXPECT_TRUE(NibblePair{0x99}.isValidBCD());
    EXPECT_FALSE(NibblePair{0x9A}.isValidBCD());
    EXPECT_FALSE(NibblePair{0xA9}.isValidBCD());

    // BCD addition
    NibblePair a{0x25};
    NibblePair b{0x38};
    bool carry;

    a.addBCD(b, false, carry);
    EXPECT_EQ(a.toByte(), 0x63u);  // 25 + 38 = 63 BCD
    EXPECT_FALSE(carry);

    // BCD with carry
    a = NibblePair{0x95};
    b = NibblePair{0x07};
    a.addBCD(b, false, carry);
    EXPECT_TRUE(carry);  // 95 + 7 = 102, carry with 02
    EXPECT_EQ(a.toByte(), 0x02u);
}

TEST_F(NibblePairTest, Swap) {
    NibblePair p{Nibble(0xA), Nibble(0x5)};  // 0x5A

    p.swap();
    EXPECT_EQ(p.toByte(), 0xA5u);

    NibblePair swapped = p.swapped();
    EXPECT_EQ(swapped.toByte(), 0x5Au);
}

// ============================================================================
// Address12 Tests
// ============================================================================

class Address12Test : public ::testing::Test {
protected:
    Address12 a0{0};
    Address12 a_FFF{0xFFF};
    Address12 a_ABC{0xABC};
};

TEST_F(Address12Test, Construction) {
    EXPECT_EQ(a0.toUint16(), 0u);
    EXPECT_EQ(a_FFF.toUint16(), 0xFFFu);
    EXPECT_EQ(a_ABC.toUint16(), 0xABCu);

    Address12 a{Nibble(0xC), Nibble(0xB), Nibble(0xA)};
    EXPECT_EQ(a.toUint16(), 0xABCu);
}

TEST_F(Address12Test, NibbleAccess) {
    EXPECT_EQ(a_ABC.nibble0().value(), 0xCu);
    EXPECT_EQ(a_ABC.nibble1().value(), 0xBu);
    EXPECT_EQ(a_ABC.nibble2().value(), 0xAu);

    Address12 a;
    a.setNibble0(Nibble(0x3));
    a.setNibble1(Nibble(0x2));
    a.setNibble2(Nibble(0x1));
    EXPECT_EQ(a.toUint16(), 0x123u);
}

TEST_F(Address12Test, ByteAccess) {
    EXPECT_EQ(a_ABC.lowByte(), 0xBCu);
    EXPECT_EQ(a_ABC.highNibble().value(), 0xAu);

    Address12 a;
    a.setFromBytes(0x56, Nibble(0x4));
    EXPECT_EQ(a.toUint16(), 0x456u);
}

TEST_F(Address12Test, Arithmetic) {
    Address12 a{0x100};

    Address12 result = a + 0x050;
    EXPECT_EQ(result.toUint16(), 0x150u);

    result = a - 0x050;
    EXPECT_EQ(result.toUint16(), 0x0B0u);

    // Test wrapping
    Address12 max{0xFFF};
    result = max + 1;
    EXPECT_EQ(result.toUint16(), 0x000u);
}

TEST_F(Address12Test, IncrementDecrement) {
    Address12 a{0x0FF};

    ++a;
    EXPECT_EQ(a.toUint16(), 0x100u);

    --a;
    EXPECT_EQ(a.toUint16(), 0x0FFu);

    // Wrap around
    Address12 max{0xFFF};
    ++max;
    EXPECT_EQ(max.toUint16(), 0x000u);

    Address12 zero{0};
    --zero;
    EXPECT_EQ(zero.toUint16(), 0xFFFu);
}

TEST_F(Address12Test, Comparison) {
    Address12 a{0x100};
    Address12 b{0x200};

    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a == Address12{0x100});
    EXPECT_TRUE(a != b);
}

// ============================================================================
// NibbleArray Tests
// ============================================================================

class NibbleArrayTest : public ::testing::Test {
protected:
    NibbleArray<16> arr;
};

TEST_F(NibbleArrayTest, Construction) {
    EXPECT_EQ(arr.size(), 16u);
    EXPECT_TRUE(arr.isAllZero());
}

TEST_F(NibbleArrayTest, ElementAccess) {
    arr[0] = Nibble(0x5);
    arr[15] = Nibble(0xA);

    EXPECT_EQ(arr[0].value(), 0x5u);
    EXPECT_EQ(arr[15].value(), 0xAu);
}

TEST_F(NibbleArrayTest, Fill) {
    arr.fill(Nibble(0x7));

    for (size_t i = 0; i < arr.size(); ++i) {
        EXPECT_EQ(arr[i].value(), 0x7u);
    }
    EXPECT_TRUE(arr.isAllValue(Nibble(0x7)));
}

TEST_F(NibbleArrayTest, ByteOperations) {
    arr.writeByte(0, 0xAB);
    arr.writeByte(1, 0xCD);

    EXPECT_EQ(arr[0].value(), 0xBu);  // Low nibble of 0xAB
    EXPECT_EQ(arr[1].value(), 0xAu);  // High nibble of 0xAB
    EXPECT_EQ(arr[2].value(), 0xDu);  // Low nibble of 0xCD
    EXPECT_EQ(arr[3].value(), 0xCu);  // High nibble of 0xCD

    EXPECT_EQ(arr.readByte(0), 0xABu);
    EXPECT_EQ(arr.readByte(1), 0xCDu);
}

TEST_F(NibbleArrayTest, LoadFromBytes) {
    uint8_t bytes[] = {0x12, 0x34, 0x56, 0x78};

    arr.loadFromBytes(bytes, 4);

    EXPECT_EQ(arr.readByte(0), 0x12u);
    EXPECT_EQ(arr.readByte(1), 0x34u);
    EXPECT_EQ(arr.readByte(2), 0x56u);
    EXPECT_EQ(arr.readByte(3), 0x78u);
}

TEST_F(NibbleArrayTest, SaveToBytes) {
    arr.writeByte(0, 0xAB);
    arr.writeByte(1, 0xCD);

    uint8_t bytes[2];
    arr.saveToBytes(bytes, 2);

    EXPECT_EQ(bytes[0], 0xABu);
    EXPECT_EQ(bytes[1], 0xCDu);
}

TEST_F(NibbleArrayTest, Comparison) {
    NibbleArray<16> arr2;

    EXPECT_TRUE(arr == arr2);

    arr[0] = Nibble(0x5);
    EXPECT_FALSE(arr == arr2);
    EXPECT_TRUE(arr != arr2);

    arr2[0] = Nibble(0x5);
    EXPECT_TRUE(arr == arr2);
}

TEST_F(NibbleArrayTest, CountValue) {
    arr.fill(Nibble(0x5));
    arr[0] = Nibble(0x3);
    arr[5] = Nibble(0x3);

    EXPECT_EQ(arr.countValue(Nibble(0x5)), 14u);
    EXPECT_EQ(arr.countValue(Nibble(0x3)), 2u);
    EXPECT_EQ(arr.countValue(Nibble(0xF)), 0u);
}
