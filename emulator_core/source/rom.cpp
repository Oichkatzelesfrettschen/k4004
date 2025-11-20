#include "emulator_core/source/rom.hpp"

#include <cstring>

ROM::ROM()
{
    reset();
}

bool ROM::load(const uint8_t* objectCode, size_t objectCodeLength)
{   
    if (objectCode == nullptr || objectCodeLength == 0)
        return false;

     // Start of the I/O metal mask section
    if (objectCode[0] != 0xFEu)
        return false;

    size_t i = 1;
    uint8_t chipNumber;
    while (objectCode[i] != 0xFFu) {
        chipNumber = objectCode[i++];

        if (i >= objectCodeLength)
            return false; // Somethin went terribly wrong

        // Do not support more than NUM_ROM_CHIPS (32 in future) rom chips for now
        if (chipNumber >= NUM_ROM_CHIPS)
            return false;

        m_ioPortsMasks[chipNumber] = objectCode[i++];

        if (i >= objectCodeLength)
            return false; // Somethin went terribly wrong
    }
    ++i;

    if (objectCodeLength - i > ROM_SIZE)
        return false;

    for (size_t j = 0; i < objectCodeLength; ++i, ++j)
        m_rom[j] = objectCode[i];

    return true;
}

void ROM::reset()
{
    m_srcAddress = 0u;
    std::memset(m_rom, 0, ROM_SIZE);
    std::memset(m_ioPorts, 0, NUM_ROM_CHIPS);
    std::memset(m_ioPortsMasks, 0, NUM_ROM_CHIPS);
}

void ROM::writeIOPort(uint8_t value)
{
    // Intel 4001 I/O Port Write Logic:
    // - Each I/O bit can be configured as input or output via metal mask
    // - Mask bit = 0: Output (CPU can write)
    // - Mask bit = 1: Input (value preserved, CPU cannot write)

    uint8_t oldValue = m_ioPorts[m_srcAddress];
    uint8_t mask = m_ioPortsMasks[m_srcAddress];
    uint8_t newValue = 0u;

    // For each of the 4 I/O bits:
    // If mask bit is 0 (output), use new value from CPU
    // If mask bit is 1 (input), preserve old value
    for (uint8_t i = 0; i < 4; ++i) {
        bool isInput = (mask >> i) & 1;
        uint8_t bitValue = isInput ? ((oldValue >> i) & 1) : ((value >> i) & 1);
        newValue |= (bitValue << i);
    }

    m_ioPorts[m_srcAddress] = newValue & 0x0Fu;
}

uint8_t ROM::readIOPort() const
{
    // Intel 4001 I/O Port Read Logic:
    // - Mask bit = 0: Output (read back what CPU wrote)
    // - Mask bit = 1: Input (read from external device)
    // Simply return the current port value (external devices update via setExternalIOPort)

    return m_ioPorts[m_srcAddress] & 0x0Fu;
}

void ROM::setExternalIOPort(uint8_t chipIndex, uint8_t value)
{
    // Allow external devices (keyboard, switches, etc.) to set I/O port values
    // Only affects pins configured as inputs (mask bit = 1)

    if (chipIndex >= NUM_ROM_CHIPS)
        return;

    uint8_t mask = m_ioPortsMasks[chipIndex];
    uint8_t oldValue = m_ioPorts[chipIndex];
    uint8_t newValue = 0u;

    // For each of the 4 I/O bits:
    // If mask bit is 1 (input), use new external value
    // If mask bit is 0 (output), preserve CPU-written value
    for (uint8_t i = 0; i < 4; ++i) {
        bool isInput = (mask >> i) & 1;
        uint8_t bitValue = isInput ? ((value >> i) & 1) : ((oldValue >> i) & 1);
        newValue |= (bitValue << i);
    }

    m_ioPorts[chipIndex] = newValue & 0x0Fu;
}

void ROM::setIOPortMask(uint8_t chipIndex, uint8_t mask)
{
    // Configure I/O port mask programmatically
    // Used for systems like Busicom where ROM file doesn't contain mask data
    // Mask bit = 0: Output (CPU can write)
    // Mask bit = 1: Input (external device drives the pin)

    if (chipIndex >= NUM_ROM_CHIPS)
        return;

    m_ioPortsMasks[chipIndex] = mask & 0x0Fu;
}
