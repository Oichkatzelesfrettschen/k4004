#pragma once
#include <cstdint>
#include <cstddef>

// Intel 8255 - Programmable Peripheral Interface (PPI)
// =====================================================
//
// The Intel 8255 PPI provides 24 programmable I/O lines organized as
// three 8-bit ports (A, B, C). It's one of the most important peripheral
// chips for expanding I/O capabilities of microprocessor systems.
//
// Package: 40-pin DIP
// Technology: NMOS
// Power: +5V single supply
// Era: 1975+
//
// Features:
// - 24 I/O lines (3 × 8-bit ports)
// - Port C can split into 4-bit upper/lower
// - Three operating modes:
//   * Mode 0: Basic input/output
//   * Mode 1: Strobed input/output (handshaking)
//   * Mode 2: Bidirectional bus
// - Programmable via control word
//
// Historical Context:
// - Used with 8080, 8085, Z80, and adapted for 4004/4040
// - Common in parallel printer interfaces
// - I/O expansion for early microcomputers
// - Still used in educational settings today
//
// Pin Assignment (40-pin DIP):
// PA0-PA7: Port A (8 bits)
// PB0-PB7: Port B (8 bits)
// PC0-PC7: Port C (8 bits)
// D0-D7:   Data bus
// A0-A1:   Address (port select)
// RD:      Read strobe
// WR:      Write strobe
// RESET:   Reset input
// CS:      Chip select

class Intel8255
{
public:
    // Port identifiers
    static constexpr uint8_t PORT_A = 0;
    static constexpr uint8_t PORT_B = 1;
    static constexpr uint8_t PORT_C = 2;
    static constexpr uint8_t CONTROL = 3;

    // Operating modes
    enum class Mode {
        MODE_0 = 0,  // Basic I/O
        MODE_1 = 1,  // Strobed I/O
        MODE_2 = 2   // Bidirectional bus
    };

    // Port groups
    enum class Group {
        GROUP_A,  // Port A + Port C upper
        GROUP_B   // Port B + Port C lower
    };

    // Construction
    Intel8255();

    // Reset
    void reset();

    // Register access (via address lines A1, A0)
    void write(uint8_t address, uint8_t data);
    uint8_t read(uint8_t address);

    // Direct port access (for internal use/testing)
    void writePortA(uint8_t value);
    void writePortB(uint8_t value);
    void writePortC(uint8_t value);
    uint8_t readPortA() const;
    uint8_t readPortB() const;
    uint8_t readPortC() const;

    // External device interface (simulates pins)
    void setExternalPortA(uint8_t value);
    void setExternalPortB(uint8_t value);
    void setExternalPortC(uint8_t value);
    uint8_t getExternalPortA() const;
    uint8_t getExternalPortB() const;
    uint8_t getExternalPortC() const;

    // Port C bit set/reset (special control word)
    void setBitPortC(uint8_t bit, bool value);

    // Control
    void setChipSelect(bool selected);
    bool isChipSelected() const { return m_chipSelect; }

    // Configuration queries
    bool isPortAInput() const { return m_portA_isInput; }
    bool isPortBInput() const { return m_portB_isInput; }
    bool isPortCUpperInput() const { return m_portCU_isInput; }
    bool isPortCLowerInput() const { return m_portCL_isInput; }

    Mode getGroupAMode() const { return m_groupA_mode; }
    Mode getGroupBMode() const { return m_groupB_mode; }

    uint8_t getControlWord() const { return m_controlWord; }

private:
    // Control word parsing
    void writeControlWord(uint8_t control);
    void writeBitSetReset(uint8_t control);
    void applyModeConfiguration();

    // Port data update based on direction
    void updatePortOutputs();

    // Port registers
    uint8_t m_portA;              // Port A latch
    uint8_t m_portB;              // Port B latch
    uint8_t m_portC;              // Port C latch

    // External pin values (from devices)
    uint8_t m_portA_external;     // External pins driving Port A
    uint8_t m_portB_external;     // External pins driving Port B
    uint8_t m_portC_external;     // External pins driving Port C

    // Control word
    uint8_t m_controlWord;

    // Direction flags
    bool m_portA_isInput;         // Port A: true=input, false=output
    bool m_portB_isInput;         // Port B: true=input, false=output
    bool m_portCU_isInput;        // Port C upper (bits 4-7)
    bool m_portCL_isInput;        // Port C lower (bits 0-3)

    // Operating modes
    Mode m_groupA_mode;           // Group A (Port A + Port C upper)
    Mode m_groupB_mode;           // Group B (Port B + Port C lower)

    // Chip select
    bool m_chipSelect;
};
