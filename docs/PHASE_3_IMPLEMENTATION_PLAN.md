# Phase 3: Memory Expansion - Detailed Implementation Plan

**Date:** 2025-11-19
**Priority:** MEDIUM
**Status:** In Progress

---

## Overview

Phase 3 focuses on expanding memory capabilities beyond the basic 4001 ROM and 4002 RAM chips, enabling connection to standard 8-bit memory devices and larger capacity storage.

---

## Chips to Implement (5 Total)

### 1. Intel 4008 - 8-Bit Address Latch ⚡ START HERE

**Package:** 16-pin DIP
**Function:** Address latch for standard memory interface

**Specifications:**
- 8-bit address output (A0-A7)
- 4-bit binary chip select (CM0-CM3)
- Built-in 4-bit I/O port
- Latches address from 4004/4040 bus
- Synchronizes with CPU timing

**Implementation Requirements:**
```cpp
class K4008 {
    - uint8_t m_addressLatch;       // 8-bit latched address
    - uint8_t m_chipSelect;         // 4-bit chip select
    - uint8_t m_ioPort;             // 4-bit I/O port
    - uint8_t m_ioMask;             // I/O direction mask

    - void latchAddress(uint8_t addr);
    - void setChipSelect(uint8_t cs);
    - uint8_t getAddress();
    - uint8_t getChipSelect();
    - void writeIOPort(uint8_t value);
    - uint8_t readIOPort();
};
```

**Test Cases:**
- Address latching (8-bit)
- Chip select decoding (4-bit)
- I/O port read/write
- Multiple chip selection

**LOC Estimate:** ~150 lines + 100 lines tests

---

### 2. Intel 4009 - Program/I/O Access Converter

**Package:** 16-pin DIP
**Function:** Converts 4-bit bus to 8-bit parallel for standard memory

**Specifications:**
- 4-to-8 bit bus conversion
- Interfaces with C2102A (1K×1 RAM)
- Interfaces with C1702A (256×8 EPROM)
- Manages read/write timing
- Bidirectional data flow

**Implementation Requirements:**
```cpp
class K4009 {
    - uint8_t m_dataBuffer;         // 8-bit data buffer
    - bool m_readMode;              // Read vs write mode
    - bool m_programMode;           // Program vs I/O mode

    - void write4Bit(uint8_t nibble, bool highNibble);
    - uint8_t read4Bit(bool highNibble);
    - uint8_t get8BitData();
    - void set8BitData(uint8_t data);
    - void setMode(bool program, bool read);
};
```

**Test Cases:**
- 4-to-8 bit conversion (2 cycles)
- 8-to-4 bit conversion (2 cycles)
- Program vs I/O mode
- Read vs write operations

**LOC Estimate:** ~120 lines + 80 lines tests

---

### 3. Intel 4289 - Standard Memory Interface ⭐ HIGH VALUE

**Package:** 24-pin DIP
**Function:** Combines 4008 + 4009 in single chip

**Specifications:**
- 12-bit address generation (8-bit address + 4-bit chip select)
- 8-bit bidirectional data bus
- Supports standard RAM/ROM chips
- Allows RAM as program memory
- Emulates 4001 ROM interface to CPU

**Implementation Requirements:**
```cpp
class K4289 {
    - uint16_t m_address12bit;      // 12-bit address (8+4 CS)
    - uint8_t m_data8bit;           // 8-bit data buffer
    - uint8_t m_ioPort;             // 4-bit I/O (4001 compatible)
    - bool m_readWrite;             // R/W control
    - bool m_chipEnable;            // CE control

    - void setAddress(uint16_t addr);
    - uint16_t getAddress();
    - void writeData(uint8_t data);
    - uint8_t readData();
    - void writeIO(uint8_t value);
    - uint8_t readIO();
    - void setControl(bool read, bool enable);
};
```

**Test Cases:**
- 12-bit address generation
- 8-bit data bus operations
- I/O port compatibility with 4001
- RAM as program memory
- Chip enable/disable
- Read/write control

**LOC Estimate:** ~200 lines + 120 lines tests

---

### 4. Intel 4308 - 8K ROM with I/O

**Package:** 24-pin DIP
**Function:** 1KB ROM with built-in I/O port

**Specifications:**
- Capacity: 1024 × 8 bits = 8192 bits = 1KB
- 10-bit addressing (A0-A9)
- 4-bit I/O port (like 4001)
- Access time: 800 ns typical
- Program storage for larger applications

**Implementation Requirements:**
```cpp
class K4308 {
    static constexpr size_t ROM_SIZE = 1024;  // 1024 bytes

    - uint8_t m_rom[ROM_SIZE];      // 1024 × 8-bit ROM
    - uint8_t m_ioPort;             // 4-bit I/O port
    - uint8_t m_ioMask;             // I/O direction mask

    - bool load(const uint8_t* data, size_t size);
    - uint8_t readByte(uint16_t address);  // 10-bit addr
    - void writeIOPort(uint8_t value);
    - uint8_t readIOPort();
    - void setIOMask(uint8_t mask);
};
```

**Test Cases:**
- ROM loading (1KB)
- 10-bit address decoding
- Byte read operations
- I/O port read/write
- I/O mask configuration
- Out-of-bounds access

**LOC Estimate:** ~180 lines + 100 lines tests

---

### 5. Intel 4702 - 2K EPROM (256×8)

**Package:** 24-pin DIP (ceramic with UV window)
**Function:** Erasable Programmable ROM for development

**Specifications:**
- Capacity: 256 × 8 bits = 2048 bits = 256 bytes
- 8-bit addressing (A0-A7)
- UV-erasable (optional simulation)
- Programming: 25V pulses (optional simulation)
- Access time: 1 μs typical
- Data retention: 10-20 years

**Implementation Requirements:**
```cpp
class K4702 {
    static constexpr size_t EPROM_SIZE = 256;  // 256 bytes

    - uint8_t m_eprom[EPROM_SIZE];  // 256 × 8-bit EPROM
    - bool m_erased;                // Erase state (all 0xFF)
    - bool m_programMode;           // Programming mode

    - void erase();                 // Simulate UV erase
    - bool program(uint8_t address, uint8_t data);
    - uint8_t readByte(uint8_t address);
    - bool isErased();
    - void setProgramMode(bool enable);
};
```

**Test Cases:**
- EPROM erase (all 0xFF)
- Byte programming
- Byte reading
- Program mode enable/disable
- Erase verification
- Data retention (read after program)
- Write-once behavior

**LOC Estimate:** ~150 lines + 90 lines tests

---

## Implementation Order (Priority Ranked)

1. **K4289** (⭐ Highest value - combines 4008/4009, most useful)
2. **K4308** (Large ROM - direct CPU interface)
3. **K4702** (EPROM - development tool)
4. **K4008** (Address latch - needed for 4009)
5. **K4009** (Converter - works with 4008)

---

## Total Estimates

**Implementation:**
- Total LOC: ~800 lines of implementation
- Total Test LOC: ~490 lines of tests
- Total Files: 10 new files (5 .cpp + 5 .hpp)

**Time Estimate:**
- Implementation: 2-3 hours
- Testing: 1-2 hours
- Documentation: 30 minutes
- **Total: 3.5-5.5 hours**

---

## Integration Points

### With Existing Code

1. **4289** can interface with existing 4001/4002 emulation
2. **4308** extends ROM capabilities beyond 4001 (256 → 1024 bytes)
3. **4702** enables prototyping/development workflows
4. **4008/4009** enable use of standard memory chips

### CMakeLists.txt Updates

```cmake
# Add to emulator_core/source/CMakeLists.txt
${CMAKE_CURRENT_SOURCE_DIR}/K4008.cpp
${CMAKE_CURRENT_SOURCE_DIR}/K4008.hpp
${CMAKE_CURRENT_SOURCE_DIR}/K4009.cpp
${CMAKE_CURRENT_SOURCE_DIR}/K4009.hpp
${CMAKE_CURRENT_SOURCE_DIR}/K4289.cpp
${CMAKE_CURRENT_SOURCE_DIR}/K4289.hpp
${CMAKE_CURRENT_SOURCE_DIR}/K4308.cpp
${CMAKE_CURRENT_SOURCE_DIR}/K4308.hpp
${CMAKE_CURRENT_SOURCE_DIR}/K4702.cpp
${CMAKE_CURRENT_SOURCE_DIR}/K4702.hpp

# Add to emulator_core/tests/CMakeLists.txt
${CMAKE_CURRENT_SOURCE_DIR}/k4008_tests.cpp
${CMAKE_CURRENT_SOURCE_DIR}/k4009_tests.cpp
${CMAKE_CURRENT_SOURCE_DIR}/k4289_tests.cpp
${CMAKE_CURRENT_SOURCE_DIR}/k4308_tests.cpp
${CMAKE_CURRENT_SOURCE_DIR}/k4702_tests.cpp
```

---

## Success Criteria

✅ All 5 chips implemented with full functionality
✅ Comprehensive test coverage (>90% pass rate)
✅ Builds without errors or warnings
✅ Documentation complete
✅ Committed and pushed to branch

---

## Next Steps After Phase 3

**Phase 4: Essential Peripherals (MEDIUM-HIGH Priority)**
- 8255 PPI - Programmable Peripheral Interface
- 8251 USART - Serial communication
- 7-segment LED Display
- Matrix Keyboard (4×4 or 8×8)

---

## Notes

- Phase 3 expands memory from 4KB to potentially 8KB+ with standard memory
- Enables use of commercially available RAM/ROM chips
- Critical for larger applications and development systems
- 4289 is the most important chip (replaces 4008+4009 pair)
- EPROM support enables development/prototyping workflows
