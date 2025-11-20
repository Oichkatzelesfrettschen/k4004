#include "emulator_core/source/busicom_peripherals.hpp"
#include <sstream>
#include <iomanip>

// Keyboard matrix mapping based on Busicom 141-PF disassembly
// Format: KEYBOARD_MATRIX[column][row] = scan_code
// Columns 0-7 are scanned for buttons, columns 8-9 are switches
const uint8_t BusicomPeripherals::KEYBOARD_MATRIX[10][4] = {
    // Column 0 (shifter bit0)
    {0x81, 0x82, 0x83, 0x84},  // CM, RM, M-, M+

    // Column 1 (shifter bit1)
    {0x85, 0x86, 0x87, 0x88},  // SQRT, %, M=-, M=+

    // Column 2 (shifter bit2)
    {0x89, 0x8a, 0x8b, 0x8c},  // diamond, /, *, =

    // Column 3 (shifter bit3)
    {0x8d, 0x8e, 0x8f, 0x90},  // -, +, diamond2, 000

    // Column 4 (shifter bit4)
    {0x91, 0x92, 0x93, 0x94},  // 9, 6, 3, .

    // Column 5 (shifter bit5)
    {0x95, 0x96, 0x97, 0x98},  // 8, 5, 2, 00

    // Column 6 (shifter bit6)
    {0x99, 0x9a, 0x9b, 0x9c},  // 7, 4, 1, 0

    // Column 7 (shifter bit7)
    {0x9d, 0x9e, 0x9f, 0xa0},  // Sign, EX, CE, C

    // Column 8 (shifter bit8) - decimal point switch
    {0x00, 0x00, 0x00, 0x00},  // Not scanned for keys

    // Column 9 (shifter bit9) - rounding switch
    {0x00, 0x00, 0x00, 0x00}   // Not scanned for keys
};

BusicomPeripherals::BusicomPeripherals()
    : m_pressedKey(0)
    , m_keyActive(false)
    , m_keyboardShifter(0x000)  // Start empty, will be loaded by shifter initialization
    , m_shiftColumn(0)
    , m_lastRom0Output(0)
    , m_printerShifter(0)
    , m_memoryLamp(false)
    , m_overflowLamp(false)
    , m_minusLamp(false)
    , m_roundLamp(false)
    , m_printerColor(false)
    , m_printerFire(false)
    , m_paperAdvance(false)
{
    m_printerOutput.hasDecimalPoint = false;
    m_printerOutput.decimalPosition = 0;
}

void BusicomPeripherals::pressKey(uint8_t scanCode)
{
    m_pressedKey = scanCode;
    m_keyActive = true;
}

void BusicomPeripherals::releaseKey()
{
    m_keyActive = false;
    m_pressedKey = 0;
}

bool BusicomPeripherals::isKeyPressed() const
{
    return m_keyActive;
}

void BusicomPeripherals::updateShiftRegister(uint8_t rom0Output)
{
    // ROM0 bit assignments (from Busicom disassembly):
    // bit0 = keyboard shifter clock
    // bit1 = shifter data (shared for keyboard and printer)
    // bit2 = printer shifter clock
    // bit3 = not used

    bool kbdClock = (rom0Output & 0x01) != 0;
    bool data = (rom0Output & 0x02) != 0;
    bool printerClock = (rom0Output & 0x04) != 0;

    bool lastKbdClock = (m_lastRom0Output & 0x01) != 0;
    bool lastPrinterClock = (m_lastRom0Output & 0x04) != 0;

    // Detect rising edge on keyboard clock (shift on 0->1 transition)
    if (kbdClock && !lastKbdClock) {
        // Shift keyboard register: shift LEFT (i4003 shifts towards MSB)
        // New data bit comes in at LSB (bit 0), MSB (bit 9) is shifted out
        m_keyboardShifter <<= 1;
        m_keyboardShifter &= 0x3FE;  // Mask to 10 bits, clear bit 0
        if (data) {
            m_keyboardShifter |= 0x001;  // Set bit 0 if data=1
        }
        // If data=0, bit 0 stays 0 (already cleared by mask)

        // Update current column (find which bit is set)
        // Scan from MSB to LSB to find the active column
        m_shiftColumn = 0;
        for (uint8_t col = 0; col < 10; col++) {
            if (m_keyboardShifter & (1 << col)) {
                m_shiftColumn = col;
                // Don't break - find the highest set bit
            }
        }
    }

    // Detect rising edge on printer clock
    if (printerClock && !lastPrinterClock) {
        // Shift printer register: shift LEFT (i4003 shifts towards MSB)
        // Two cascaded i4003 = 20 bits total
        m_printerShifter <<= 1;
        m_printerShifter &= 0xFFFFE;  // Mask to 20 bits, clear bit 0
        if (data) {
            m_printerShifter |= 0x001;  // Set bit 0 if data=1
        }
        // If data=0, bit 0 stays 0 (already cleared by mask)
    }

    m_lastRom0Output = rom0Output;
}

uint8_t BusicomPeripherals::getKeyboardRows() const
{
    // ROM1 reads the keyboard matrix rows (4 bits)
    // If a key is pressed and we're scanning its column, activate the corresponding row

    if (!m_keyActive || m_shiftColumn >= 8) {
        // No key pressed, or scanning switch columns
        return 0x00;
    }

    // Check if pressed key is in the current column
    for (uint8_t row = 0; row < 4; row++) {
        if (KEYBOARD_MATRIX[m_shiftColumn][row] == m_pressedKey) {
            // Key found! Return row bit pattern
            return (1 << row);
        }
    }

    // Pressed key not in current column
    return 0x00;
}

void BusicomPeripherals::clearPrinterOutput()
{
    m_printerOutput.digits.clear();
    m_printerOutput.symbols.clear();
    m_printerOutput.hasDecimalPoint = false;
    m_printerOutput.decimalPosition = 0;
}

void BusicomPeripherals::updateStatusLamps(uint8_t ram1Output)
{
    // RAM1 port bit assignments (from Busicom disassembly):
    // bit0 = memory lamp
    // bit1 = overflow lamp
    // bit2 = minus lamp
    // bit3 = rounding lamp

    m_memoryLamp = (ram1Output & 0x01) != 0;
    m_overflowLamp = (ram1Output & 0x02) != 0;
    m_minusLamp = (ram1Output & 0x04) != 0;
    m_roundLamp = (ram1Output & 0x08) != 0;
}

void BusicomPeripherals::updatePrinterControl(uint8_t ram0Output)
{
    // RAM0 port bit assignments (from Busicom disassembly):
    // bit0 = color (0=black, 1=red)
    // bit1 = fire hammers
    // bit2 = not used
    // bit3 = advance paper

    m_printerColor = (ram0Output & 0x01) != 0;
    bool fire = (ram0Output & 0x02) != 0;
    m_paperAdvance = (ram0Output & 0x08) != 0;

    // Capture printer output when hammers fire
    if (fire && !m_printerFire) {
        // Rising edge on fire signal - capture current shift register state
        decodeAndCapturePrinter();
    }

    m_printerFire = fire;
}

void BusicomPeripherals::decodeAndCapturePrinter()
{
    // Printer shift register mapping (from Busicom disassembly):
    // bit00-01: column 17-18 (special characters)
    // bit02: unused
    // bit03-17: columns 1-15 (digits and decimal points)
    // bit18-19: unused

    // For now, just capture the pattern as a hex string
    // Full character decoding would require drum sector timing
    // which we can implement when needed for display

    std::stringstream ss;
    ss << std::hex << std::setw(5) << std::setfill('0') << m_printerShifter;

    // Simple digit extraction: if bit 3-17 are set, likely printing a digit
    if (m_printerShifter & 0x0003FFF8) {
        // Extract which columns are active
        for (int col = 3; col <= 17; col++) {
            if (m_printerShifter & (1 << col)) {
                m_printerOutput.digits += 'X';  // Placeholder for actual digit
            }
        }
    }
}

std::string BusicomPeripherals::getKeyboardState() const
{
    std::stringstream ss;
    ss << "Keyboard: ";
    if (m_keyActive) {
        ss << "Key 0x" << std::hex << std::setw(2) << std::setfill('0')
           << (int)m_pressedKey << " pressed";
    } else {
        ss << "No key pressed";
    }
    ss << ", Column=" << std::dec << (int)m_shiftColumn;
    ss << ", Shifter=0x" << std::hex << std::setw(3) << std::setfill('0')
       << m_keyboardShifter;
    return ss.str();
}

std::string BusicomPeripherals::getShiftRegisterState() const
{
    std::stringstream ss;
    ss << "Shifters: KB=0x" << std::hex << std::setw(3) << std::setfill('0')
       << m_keyboardShifter;
    ss << ", Printer=0x" << std::setw(5) << std::setfill('0')
       << m_printerShifter;
    return ss.str();
}
