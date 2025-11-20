#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

// Matrix Keyboard Interface Emulation
// ====================================
//
// Emulates matrix-scanned keyboards common in 1970s microcomputer systems.
// Used with Intel MCS-4/MCS-40 via I/O ports or peripheral controllers.
//
// Scanning Method:
// 1. CPU drives one row LOW, others HIGH
// 2. CPU reads column inputs
// 3. If column is LOW, key at (row,col) is pressed
// 4. Repeat for all rows
//
// Common Configurations:
// - 4×4 matrix: Hex keypad (0-9, A-F)
// - 8×8 matrix: Full keyboard (64 keys)
// - Custom layouts for calculators, instruments
//
// Historical Context:
// - HP-35 calculator: 5×7 keyboard matrix
// - MITS Altair 8800: Front panel switches
// - Early terminals: Full ASCII keyboards
// - Debouncing required in software or hardware

class MatrixKeyboard
{
public:
    // Construction
    explicit MatrixKeyboard(size_t rows = 4, size_t cols = 4);

    // Scanning interface (hardware simulation)
    void setRowOutput(uint8_t rowMask);
    uint8_t readColumnInput() const;

    // Key state management (for testing/simulation)
    void pressKey(size_t row, size_t col);
    void releaseKey(size_t row, size_t col);
    void releaseAll();
    bool isKeyPressed(size_t row, size_t col) const;

    // Full keyboard scan
    struct ScanResult {
        bool keyPressed;
        size_t row;
        size_t col;
    };
    ScanResult scan();

    // Debouncing (call tick() periodically)
    void tick();
    bool getStableKey(size_t& row, size_t& col);

    // Properties
    size_t numRows() const { return m_rows; }
    size_t numCols() const { return m_cols; }

    // Reset
    void reset();

private:
    size_t m_rows;
    size_t m_cols;

    // Key state: true = pressed, false = released
    std::vector<std::vector<bool>> m_keyState;

    // Debounce counters for each key
    std::vector<std::vector<uint8_t>> m_debounceCounter;

    // Current row output mask
    uint8_t m_currentRowMask;

    // Debounce settings
    static constexpr uint8_t DEBOUNCE_TICKS = 3;

    // Helper to check if row is active (driven LOW)
    bool isRowActive(size_t row) const;
};
