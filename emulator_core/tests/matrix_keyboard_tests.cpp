#include <gtest/gtest.h>
#include "emulator_core/source/MatrixKeyboard.hpp"

class MatrixKeyboardTest : public ::testing::Test {
protected:
    MatrixKeyboard kb4x4{4, 4};
    MatrixKeyboard kb8x8{8, 8};
};

TEST_F(MatrixKeyboardTest, Construction) {
    EXPECT_EQ(kb4x4.numRows(), 4u);
    EXPECT_EQ(kb4x4.numCols(), 4u);
    EXPECT_EQ(kb8x8.numRows(), 8u);
    EXPECT_EQ(kb8x8.numCols(), 8u);

    // All keys should be released
    for (size_t r = 0; r < 4; ++r) {
        for (size_t c = 0; c < 4; ++c) {
            EXPECT_FALSE(kb4x4.isKeyPressed(r, c));
        }
    }
}

TEST_F(MatrixKeyboardTest, SingleKeyPress) {
    kb4x4.pressKey(2, 3);
    EXPECT_TRUE(kb4x4.isKeyPressed(2, 3));

    kb4x4.releaseKey(2, 3);
    EXPECT_FALSE(kb4x4.isKeyPressed(2, 3));
}

TEST_F(MatrixKeyboardTest, MultipleKeyPress) {
    kb4x4.pressKey(0, 0);
    kb4x4.pressKey(1, 1);
    kb4x4.pressKey(2, 2);

    EXPECT_TRUE(kb4x4.isKeyPressed(0, 0));
    EXPECT_TRUE(kb4x4.isKeyPressed(1, 1));
    EXPECT_TRUE(kb4x4.isKeyPressed(2, 2));
    EXPECT_FALSE(kb4x4.isKeyPressed(3, 3));
}

TEST_F(MatrixKeyboardTest, ReleaseAll) {
    kb4x4.pressKey(0, 0);
    kb4x4.pressKey(1, 1);
    kb4x4.pressKey(2, 2);

    kb4x4.releaseAll();

    for (size_t r = 0; r < 4; ++r) {
        for (size_t c = 0; c < 4; ++c) {
            EXPECT_FALSE(kb4x4.isKeyPressed(r, c));
        }
    }
}

TEST_F(MatrixKeyboardTest, ScanSingleRow) {
    kb4x4.pressKey(1, 2);

    // Scan row 1 (drive row 1 LOW, others HIGH)
    kb4x4.setRowOutput(0xFF & ~(1u << 1));

    uint8_t cols = kb4x4.readColumnInput();

    // Column 2 should be LOW (key pressed)
    EXPECT_FALSE(cols & (1u << 2));

    // Other columns should be HIGH
    EXPECT_TRUE(cols & (1u << 0));
    EXPECT_TRUE(cols & (1u << 1));
    EXPECT_TRUE(cols & (1u << 3));
}

TEST_F(MatrixKeyboardTest, ScanMultipleRows) {
    kb4x4.pressKey(0, 1);
    kb4x4.pressKey(2, 3);

    // Scan row 0
    kb4x4.setRowOutput(0xFF & ~(1u << 0));
    uint8_t cols0 = kb4x4.readColumnInput();
    EXPECT_FALSE(cols0 & (1u << 1));  // Key at (0,1)

    // Scan row 2
    kb4x4.setRowOutput(0xFF & ~(1u << 2));
    uint8_t cols2 = kb4x4.readColumnInput();
    EXPECT_FALSE(cols2 & (1u << 3));  // Key at (2,3)
}

TEST_F(MatrixKeyboardTest, NoKeyPressed) {
    // All rows HIGH (no row selected)
    kb4x4.setRowOutput(0xFF);

    uint8_t cols = kb4x4.readColumnInput();

    // All columns should be HIGH (no keys)
    EXPECT_EQ(cols, 0xFFu);
}

TEST_F(MatrixKeyboardTest, FullScan) {
    kb4x4.pressKey(2, 1);

    auto result = kb4x4.scan();

    EXPECT_TRUE(result.keyPressed);
    EXPECT_EQ(result.row, 2u);
    EXPECT_EQ(result.col, 1u);
}

TEST_F(MatrixKeyboardTest, FullScanNoKey) {
    auto result = kb4x4.scan();

    EXPECT_FALSE(result.keyPressed);
}

TEST_F(MatrixKeyboardTest, Debouncing) {
    kb4x4.pressKey(1, 1);

    size_t row, col;

    // Before debounce ticks, key not stable
    EXPECT_FALSE(kb4x4.getStableKey(row, col));

    // Tick once
    kb4x4.tick();
    EXPECT_FALSE(kb4x4.getStableKey(row, col));

    // Tick twice
    kb4x4.tick();
    EXPECT_FALSE(kb4x4.getStableKey(row, col));

    // Tick three times - now stable
    kb4x4.tick();
    EXPECT_TRUE(kb4x4.getStableKey(row, col));
    EXPECT_EQ(row, 1u);
    EXPECT_EQ(col, 1u);
}

TEST_F(MatrixKeyboardTest, DebounceReset) {
    kb4x4.pressKey(0, 0);

    kb4x4.tick();
    kb4x4.tick();

    // Release key before it stabilizes
    kb4x4.releaseKey(0, 0);

    kb4x4.tick();

    size_t row, col;
    EXPECT_FALSE(kb4x4.getStableKey(row, col));
}

TEST_F(MatrixKeyboardTest, Reset) {
    kb4x4.pressKey(1, 1);
    kb4x4.pressKey(2, 2);

    kb4x4.reset();

    // All keys should be released
    for (size_t r = 0; r < 4; ++r) {
        for (size_t c = 0; c < 4; ++c) {
            EXPECT_FALSE(kb4x4.isKeyPressed(r, c));
        }
    }

    // Row mask should be reset
    uint8_t cols = kb4x4.readColumnInput();
    EXPECT_EQ(cols, 0xFFu);
}
