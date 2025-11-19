#include "emulator_core/source/Intel8255.hpp"

Intel8255::Intel8255()
    : m_portA(0),
      m_portB(0),
      m_portC(0),
      m_portA_external(0),
      m_portB_external(0),
      m_portC_external(0),
      m_controlWord(0x9B),  // Default: all inputs
      m_portA_isInput(true),
      m_portB_isInput(true),
      m_portCU_isInput(true),
      m_portCL_isInput(true),
      m_groupA_mode(Mode::MODE_0),
      m_groupB_mode(Mode::MODE_0),
      m_chipSelect(false)
{
}

void Intel8255::reset()
{
    m_portA = 0;
    m_portB = 0;
    m_portC = 0;
    m_portA_external = 0;
    m_portB_external = 0;
    m_portC_external = 0;
    m_controlWord = 0x9B;  // All inputs
    m_portA_isInput = true;
    m_portB_isInput = true;
    m_portCU_isInput = true;
    m_portCL_isInput = true;
    m_groupA_mode = Mode::MODE_0;
    m_groupB_mode = Mode::MODE_0;
    // Note: m_chipSelect is NOT reset - it's a pin signal controlled externally
}

void Intel8255::write(uint8_t address, uint8_t data)
{
    if (!m_chipSelect) return;

    address &= 0x03;  // Only 2 address bits

    switch (address) {
        case PORT_A:
            writePortA(data);
            break;

        case PORT_B:
            writePortB(data);
            break;

        case PORT_C:
            writePortC(data);
            break;

        case CONTROL:
            // Check bit 7 to determine control word type
            if (data & 0x80) {
                // Mode set: bit 7 = 1
                writeControlWord(data);
            } else {
                // Bit set/reset: bit 7 = 0
                writeBitSetReset(data);
            }
            break;
    }
}

uint8_t Intel8255::read(uint8_t address)
{
    if (!m_chipSelect) return 0xFF;

    address &= 0x03;

    switch (address) {
        case PORT_A:
            return readPortA();

        case PORT_B:
            return readPortB();

        case PORT_C:
            return readPortC();

        case CONTROL:
            // Reading control word returns current configuration
            return m_controlWord;

        default:
            return 0xFF;
    }
}

void Intel8255::writePortA(uint8_t value)
{
    if (!m_portA_isInput) {
        m_portA = value;
    }
    // If input mode, writes are ignored
}

void Intel8255::writePortB(uint8_t value)
{
    if (!m_portB_isInput) {
        m_portB = value;
    }
}

void Intel8255::writePortC(uint8_t value)
{
    // Port C can be split - respect upper/lower direction
    uint8_t newValue = m_portC;

    // Upper nibble (bits 4-7)
    if (!m_portCU_isInput) {
        newValue = (newValue & 0x0F) | (value & 0xF0);
    }

    // Lower nibble (bits 0-3)
    if (!m_portCL_isInput) {
        newValue = (newValue & 0xF0) | (value & 0x0F);
    }

    m_portC = newValue;
}

uint8_t Intel8255::readPortA() const
{
    if (m_portA_isInput) {
        // Input mode: return external pin values
        return m_portA_external;
    } else {
        // Output mode: return latched value
        return m_portA;
    }
}

uint8_t Intel8255::readPortB() const
{
    if (m_portB_isInput) {
        return m_portB_external;
    } else {
        return m_portB;
    }
}

uint8_t Intel8255::readPortC() const
{
    uint8_t value = 0;

    // Upper nibble
    if (m_portCU_isInput) {
        value |= m_portC_external & 0xF0;
    } else {
        value |= m_portC & 0xF0;
    }

    // Lower nibble
    if (m_portCL_isInput) {
        value |= m_portC_external & 0x0F;
    } else {
        value |= m_portC & 0x0F;
    }

    return value;
}

void Intel8255::setExternalPortA(uint8_t value)
{
    m_portA_external = value;
}

void Intel8255::setExternalPortB(uint8_t value)
{
    m_portB_external = value;
}

void Intel8255::setExternalPortC(uint8_t value)
{
    m_portC_external = value;
}

uint8_t Intel8255::getExternalPortA() const
{
    return m_portA_isInput ? 0 : m_portA;
}

uint8_t Intel8255::getExternalPortB() const
{
    return m_portB_isInput ? 0 : m_portB;
}

uint8_t Intel8255::getExternalPortC() const
{
    uint8_t value = 0;
    if (!m_portCU_isInput) value |= m_portC & 0xF0;
    if (!m_portCL_isInput) value |= m_portC & 0x0F;
    return value;
}

void Intel8255::setBitPortC(uint8_t bit, bool value)
{
    if (bit >= 8) return;

    // Bit set/reset only works in Mode 0
    if (m_groupA_mode != Mode::MODE_0 || m_groupB_mode != Mode::MODE_0) {
        return;
    }

    if (value) {
        m_portC |= (1u << bit);
    } else {
        m_portC &= ~(1u << bit);
    }
}

void Intel8255::setChipSelect(bool selected)
{
    m_chipSelect = selected;
}

void Intel8255::writeControlWord(uint8_t control)
{
    m_controlWord = control;
    applyModeConfiguration();
}

void Intel8255::writeBitSetReset(uint8_t control)
{
    // Bit 7 = 0 for bit set/reset
    // Bit 3-1: bit select (0-7)
    // Bit 0: 1=set, 0=reset

    uint8_t bit = (control >> 1) & 0x07;
    bool set = control & 0x01;

    setBitPortC(bit, set);
}

void Intel8255::applyModeConfiguration()
{
    // Control Word Format (Mode Set):
    // Bit 7:   1 (mode set flag)
    // Bit 6-5: Group A mode (00=Mode 0, 01=Mode 1, 1x=Mode 2)
    // Bit 4:   Port A direction (1=input, 0=output)
    // Bit 3:   Port C upper direction (1=input, 0=output)
    // Bit 2:   Group B mode (0=Mode 0, 1=Mode 1)
    // Bit 1:   Port B direction (1=input, 0=output)
    // Bit 0:   Port C lower direction (1=input, 0=output)

    // Group A mode
    uint8_t groupA_modeBits = (m_controlWord >> 5) & 0x03;
    if (groupA_modeBits == 0) {
        m_groupA_mode = Mode::MODE_0;
    } else if (groupA_modeBits == 1) {
        m_groupA_mode = Mode::MODE_1;
    } else {
        m_groupA_mode = Mode::MODE_2;
    }

    // Group B mode
    uint8_t groupB_modeBit = (m_controlWord >> 2) & 0x01;
    m_groupB_mode = groupB_modeBit ? Mode::MODE_1 : Mode::MODE_0;

    // Port directions
    m_portA_isInput = (m_controlWord >> 4) & 0x01;
    m_portB_isInput = (m_controlWord >> 1) & 0x01;
    m_portCU_isInput = (m_controlWord >> 3) & 0x01;
    m_portCL_isInput = (m_controlWord >> 0) & 0x01;

    // Clear ports on mode change
    if (!m_portA_isInput) m_portA = 0;
    if (!m_portB_isInput) m_portB = 0;
    if (!m_portCU_isInput && !m_portCL_isInput) {
        m_portC = 0;
    } else if (!m_portCU_isInput) {
        m_portC &= 0x0F;
    } else if (!m_portCL_isInput) {
        m_portC &= 0xF0;
    }
}

void Intel8255::updatePortOutputs()
{
    // This would be called after configuration changes
    // to update external pin states
    // (Currently outputs are always updated immediately)
}
