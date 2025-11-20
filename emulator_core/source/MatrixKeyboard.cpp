#include "emulator_core/source/MatrixKeyboard.hpp"

MatrixKeyboard::MatrixKeyboard(size_t rows, size_t cols)
    : m_rows(rows),
      m_cols(cols),
      m_keyState(rows, std::vector<bool>(cols, false)),
      m_debounceCounter(rows, std::vector<uint8_t>(cols, 0)),
      m_currentRowMask(0xFF)
{
}

void MatrixKeyboard::setRowOutput(uint8_t rowMask)
{
    m_currentRowMask = rowMask;
}

uint8_t MatrixKeyboard::readColumnInput() const
{
    // Start with all columns HIGH (no keys pressed)
    uint8_t columnMask = 0xFF;

    // For each row that is driven LOW
    for (size_t row = 0; row < m_rows && row < 8; ++row) {
        if (!isRowActive(row)) {
            continue;  // Row not being scanned
        }

        // Check each column for pressed keys
        for (size_t col = 0; col < m_cols && col < 8; ++col) {
            if (m_keyState[row][col]) {
                // Key is pressed - pull column LOW
                columnMask &= ~(1u << col);
            }
        }
    }

    return columnMask;
}

void MatrixKeyboard::pressKey(size_t row, size_t col)
{
    if (row < m_rows && col < m_cols) {
        m_keyState[row][col] = true;
        m_debounceCounter[row][col] = 0;
    }
}

void MatrixKeyboard::releaseKey(size_t row, size_t col)
{
    if (row < m_rows && col < m_cols) {
        m_keyState[row][col] = false;
        m_debounceCounter[row][col] = 0;
    }
}

void MatrixKeyboard::releaseAll()
{
    for (size_t row = 0; row < m_rows; ++row) {
        for (size_t col = 0; col < m_cols; ++col) {
            m_keyState[row][col] = false;
            m_debounceCounter[row][col] = 0;
        }
    }
}

bool MatrixKeyboard::isKeyPressed(size_t row, size_t col) const
{
    if (row < m_rows && col < m_cols) {
        return m_keyState[row][col];
    }
    return false;
}

MatrixKeyboard::ScanResult MatrixKeyboard::scan()
{
    ScanResult result = {false, 0, 0};

    // Scan all rows
    for (size_t row = 0; row < m_rows && row < 8; ++row) {
        // Set row LOW, others HIGH
        uint8_t rowMask = 0xFF & ~(1u << row);
        setRowOutput(rowMask);

        // Read columns
        uint8_t columns = readColumnInput();

        // Check for pressed keys
        for (size_t col = 0; col < m_cols && col < 8; ++col) {
            if (!(columns & (1u << col))) {
                // Key pressed
                result.keyPressed = true;
                result.row = row;
                result.col = col;
                return result;  // Return first pressed key
            }
        }
    }

    return result;
}

void MatrixKeyboard::tick()
{
    // Update debounce counters for all keys
    for (size_t row = 0; row < m_rows; ++row) {
        for (size_t col = 0; col < m_cols; ++col) {
            if (m_keyState[row][col]) {
                // Key is pressed - increment debounce counter
                if (m_debounceCounter[row][col] < DEBOUNCE_TICKS) {
                    ++m_debounceCounter[row][col];
                }
            } else {
                // Key is released - reset counter
                m_debounceCounter[row][col] = 0;
            }
        }
    }
}

bool MatrixKeyboard::getStableKey(size_t& row, size_t& col)
{
    // Find first key that has been stable for DEBOUNCE_TICKS
    for (size_t r = 0; r < m_rows; ++r) {
        for (size_t c = 0; c < m_cols; ++c) {
            if (m_keyState[r][c] && m_debounceCounter[r][c] >= DEBOUNCE_TICKS) {
                row = r;
                col = c;
                return true;
            }
        }
    }

    return false;
}

void MatrixKeyboard::reset()
{
    releaseAll();
    m_currentRowMask = 0xFF;
}

bool MatrixKeyboard::isRowActive(size_t row) const
{
    if (row >= 8) return false;

    // Row is active if its bit is LOW (0)
    return !(m_currentRowMask & (1u << row));
}
