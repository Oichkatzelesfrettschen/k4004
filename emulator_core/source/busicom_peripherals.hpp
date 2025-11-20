#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <queue>

// Minimal Busicom 141-PF Peripheral Simulation
//
// This class provides just enough peripheral simulation to enable
// calculator functionality testing without full hardware emulation.
//
// Architecture:
// - Keyboard: Inject scan codes when shift register scans active column
// - Printer: Capture shift register output patterns
// - Status Lamps: Monitor RAM1 output port
//
// Based on Busicom 141-PF disassembly analysis:
// - 10 column × 4 row keyboard matrix
// - i4003 shift registers for scanning
// - ROM0 controls shifter (bit0=kbd clock, bit1=data, bit2=printer clock)
// - ROM1 reads keyboard rows (4 bits)
// - RAM0 controls printer (bit0=color, bit1=fire, bit3=advance)
// - RAM1 controls status lamps (bit0=memory, bit1=overflow, bit2=minus, bit3=round)

class BusicomPeripherals
{
public:
    BusicomPeripherals();

    // Keyboard interface
    void pressKey(uint8_t scanCode);        // Press key (scan codes 0x81-0xa0)
    void releaseKey();                       // Release currently pressed key
    bool isKeyPressed() const;

    // Shift register simulation (called by emulator when ROM0 changes)
    void updateShiftRegister(uint8_t rom0Output);

    // Get keyboard matrix row status for ROM1 input
    uint8_t getKeyboardRows() const;

    // Printer output capture
    struct PrinterCapture {
        std::string digits;       // Captured numeric digits
        std::string symbols;      // Captured symbols (+, -, ×, /, =, etc.)
        bool hasDecimalPoint;
        int decimalPosition;
    };

    const PrinterCapture& getPrinterOutput() const { return m_printerOutput; }
    void clearPrinterOutput();

    // Status lamps (from RAM1)
    void updateStatusLamps(uint8_t ram1Output);
    bool isMemoryLampOn() const { return m_memoryLamp; }
    bool isOverflowLampOn() const { return m_overflowLamp; }
    bool isMinusLampOn() const { return m_minusLamp; }
    bool isRoundLampOn() const { return m_roundLamp; }

    // Printer control (from RAM0)
    void updatePrinterControl(uint8_t ram0Output);

    // Debug/diagnostics
    std::string getKeyboardState() const;
    std::string getShiftRegisterState() const;

private:
    // Keyboard state
    uint8_t m_pressedKey;           // Currently pressed scan code (0 = none)
    bool m_keyActive;

    // Shift register state (keyboard scanner)
    uint32_t m_keyboardShifter;     // 10-bit shift register for keyboard columns
    uint8_t m_shiftColumn;          // Current column being scanned (0-9)
    uint8_t m_lastRom0Output;       // Previous ROM0 value (for edge detection)

    // Shift register state (printer)
    uint32_t m_printerShifter;      // 20-bit shift register for printer columns

    // Keyboard matrix mapping (10 columns × 4 rows)
    // Based on Busicom disassembly scan code table
    static const uint8_t KEYBOARD_MATRIX[10][4];

    // Printer output
    PrinterCapture m_printerOutput;

    // Status lamps
    bool m_memoryLamp;
    bool m_overflowLamp;
    bool m_minusLamp;
    bool m_roundLamp;

    // Printer control
    bool m_printerColor;       // 0=black, 1=red
    bool m_printerFire;        // Fire hammers
    bool m_paperAdvance;       // Advance paper

    // Helper methods
    void shiftKeyboardColumn();
    void decodeAndCapturePrinter();
};
