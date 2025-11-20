# Phase 4 + Pure 4-Bit Architecture Refactoring Plan

**Version:** 1.0
**Date:** 2025-11-19
**Scope:** Phase 4 peripheral implementation + architectural refactoring to pure 4-bit operations

---

## Table of Contents

1. [Overview](#overview)
2. [4-Bit Architecture Fundamentals](#4-bit-architecture-fundamentals)
3. [Core 4-Bit Data Types](#core-4-bit-data-types)
4. [Refactoring Strategy](#refactoring-strategy)
5. [Phase 4 Chips](#phase-4-chips)
6. [Implementation Order](#implementation-order)
7. [Testing Strategy](#testing-strategy)

---

## 1. Overview

### Goals

This plan addresses two major objectives:

1. **Architectural Purity**: Refactor the entire codebase to use native 4-bit (nibble) operations, matching the actual Intel 4004/4040 hardware architecture
2. **Phase 4 Implementation**: Add essential peripheral chips (8255, 8251, displays, keyboards)

### Why 4-Bit Native Architecture?

The Intel 4004/4040 are TRUE 4-bit processors:
- **Data bus**: 4 bits wide
- **Registers**: 4 bits (16 registers can pair for 8-bit operations)
- **ALU**: 4-bit operations
- **Memory**: Organized in 4-bit nibbles (4002 RAM is 80 nibbles)

Current implementation uses uint8_t everywhere, which masks the hardware reality. We need to redesign using nibbles as the fundamental unit.

---

## 2. 4-Bit Architecture Fundamentals

### Historical Context

**Intel 4004 (1971)**:
- First commercial microprocessor
- 4-bit data bus
- 8-bit instructions (two 4-bit cycles)
- 12-bit addresses (three 4-bit nibbles)
- 740 kHz maximum clock
- BCD (Binary Coded Decimal) focused

**Key Architectural Points**:
- Data flows as nibbles (4 bits)
- Instructions span multiple nibbles
- Addresses are multi-nibble values
- Everything internally operates on nibbles
- Registers are 4-bit units

### Nibble-Centric Design

```
Fundamental Unit: NIBBLE (4 bits)
Values: 0x0 - 0xF (0-15 decimal)

Multi-Nibble Values:
- Byte: 2 nibbles (low/high)
- Address (12-bit): 3 nibbles
- Instruction: 1-2 nibbles

BCD Support:
- Each nibble = 1 decimal digit
- 0x0-0x9 valid BCD
- 0xA-0xF invalid BCD (but used in hex)
```

---

## 3. Core 4-Bit Data Types

### Design Philosophy

Create a type system that enforces nibble semantics at compile time:

```cpp
// Base nibble type - fundamental 4-bit unit
class Nibble {
public:
    Nibble() : m_value(0) {}
    explicit Nibble(uint8_t val) : m_value(val & 0x0F) {}

    uint8_t value() const { return m_value; }
    void setValue(uint8_t val) { m_value = val & 0x0F; }

    // Arithmetic operations
    Nibble operator+(const Nibble& other) const;
    Nibble operator-(const Nibble& other) const;
    Nibble operator&(const Nibble& other) const;
    Nibble operator|(const Nibble& other) const;
    Nibble operator^(const Nibble& other) const;
    Nibble operator~() const;

    // Comparison
    bool operator==(const Nibble& other) const;
    bool operator!=(const Nibble& other) const;

    // Carry/borrow detection
    bool addWithCarry(const Nibble& other, bool carry_in, bool& carry_out);
    bool subWithBorrow(const Nibble& other, bool borrow_in, bool& borrow_out);

    // BCD operations
    bool isValidBCD() const { return m_value <= 9; }
    Nibble toBCD() const;

    // Bit operations
    bool getBit(uint8_t bit) const { return (m_value >> bit) & 1; }
    void setBit(uint8_t bit, bool val);

private:
    uint8_t m_value; // Only bits 0-3 used
};

// Paired nibbles for 8-bit operations
class NibblePair {
public:
    NibblePair() = default;
    NibblePair(Nibble low, Nibble high) : m_low(low), m_high(high) {}
    explicit NibblePair(uint8_t byte)
        : m_low(byte & 0x0F), m_high((byte >> 4) & 0x0F) {}

    Nibble low() const { return m_low; }
    Nibble high() const { return m_high; }
    void setLow(Nibble n) { m_low = n; }
    void setHigh(Nibble n) { m_high = n; }

    uint8_t toByte() const { return m_low.value() | (m_high.value() << 4); }

private:
    Nibble m_low;
    Nibble m_high;
};

// Multi-nibble address (12-bit for 4004/4040)
class Address12 {
public:
    Address12() = default;
    explicit Address12(uint16_t addr);

    Nibble nibble0() const { return m_nibbles[0]; } // Low nibble
    Nibble nibble1() const { return m_nibbles[1]; } // Mid nibble
    Nibble nibble2() const { return m_nibbles[2]; } // High nibble

    uint16_t toUint16() const;

private:
    Nibble m_nibbles[3]; // 12 bits = 3 nibbles
};

// Nibble array for memory/registers
template<size_t N>
class NibbleArray {
public:
    Nibble& operator[](size_t index) { return m_data[index]; }
    const Nibble& operator[](size_t index) const { return m_data[index]; }

    size_t size() const { return N; }
    void fill(Nibble value);

private:
    std::array<Nibble, N> m_data;
};
```

---

## 4. Refactoring Strategy

### Phase 1: Core Data Types (Foundational)

**Tasks**:
1. Create `nibble.hpp` and `nibble.cpp` with Nibble class
2. Create `nibble_pair.hpp` for NibblePair
3. Create `address12.hpp` for Address12
4. Create `nibble_array.hpp` for NibbleArray<N>
5. Unit tests for all nibble operations (100+ tests)

**Dependencies**: None (foundational)

**Estimated Time**: 4-6 hours

**Success Criteria**:
- All nibble arithmetic works correctly
- Carry/borrow detection accurate
- BCD validation functional
- All tests pass

### Phase 2: CPU Refactoring (Critical Path)

**Tasks**:
1. Refactor 4004/4040 registers to use Nibble type
2. Refactor accumulator to Nibble
3. Refactor ALU operations to work with Nibbles
4. Refactor instruction decoder to emit nibble operations
5. Update all CPU tests

**Current Code Impact**:
- `emulator_core/source/cpu_4004.hpp`
- `emulator_core/source/cpu_4040.hpp`
- All CPU implementation files

**Dependencies**: Phase 1 complete

**Estimated Time**: 8-12 hours

**Success Criteria**:
- All existing CPU tests still pass
- Instruction execution uses nibble operations
- Register pairs work correctly

### Phase 3: Memory Refactoring (High Priority)

**Tasks**:
1. Refactor 4002 RAM to use NibbleArray<80>
2. Refactor 4001 ROM to use NibbleArray<512> (256 bytes = 512 nibbles)
3. Refactor 4101 SRAM to use NibbleArray<512>
4. Update memory addressing to use Address12
5. Update all memory tests

**Current Code Impact**:
- `emulator_core/source/ram.hpp/cpp`
- `emulator_core/source/rom.hpp/cpp`
- `emulator_core/source/K4101.hpp/cpp`

**Dependencies**: Phase 1 complete

**Estimated Time**: 6-8 hours

**Success Criteria**:
- Memory organized as nibbles
- All tests pass
- Addressing works correctly

### Phase 4: I/O Chips Refactoring (Medium Priority)

**Tasks**:
1. Refactor 4003 shift register to nibble operations
2. Refactor 4008 to use Nibble for I/O port
3. Refactor 4009 nibble converter
4. Refactor 4201A clock generator
5. Update all I/O chip tests

**Current Code Impact**:
- `emulator_core/source/K4003.hpp/cpp`
- `emulator_core/source/K4008.hpp/cpp`
- `emulator_core/source/K4009.hpp/cpp`
- `emulator_core/source/K4201A.hpp/cpp`

**Dependencies**: Phase 1 complete

**Estimated Time**: 6-8 hours

**Success Criteria**:
- All I/O operations use nibbles
- Tests pass
- No uint8_t in public interfaces

### Phase 5: Large Memory Chips (Lower Priority)

**Tasks**:
1. Refactor 4289 to use nibble/byte pairs
2. Refactor 4308 to use NibblePair arrays
3. Refactor 4702 EPROM
4. Update all large memory tests

**Current Code Impact**:
- `emulator_core/source/K4289.hpp/cpp`
- `emulator_core/source/K4308.hpp/cpp`
- `emulator_core/source/K4702.hpp/cpp`

**Dependencies**: Phase 1, 3 complete

**Estimated Time**: 4-6 hours

---

## 5. Phase 4 Chips

### Intel 8255 - Programmable Peripheral Interface

**Package**: 40-pin DIP
**Technology**: NMOS
**Power**: +5V single supply

**Specifications**:
- **3 Ports**: A (8-bit), B (8-bit), C (8-bit) = 24 total I/O lines
- **Port C Split**: Can be 4-bit upper + 4-bit lower
- **Modes**:
  - Mode 0: Simple I/O (basic input/output)
  - Mode 1: Strobed I/O (handshaking)
  - Mode 2: Bidirectional bus
- **Control**: 8-bit control register for configuration

**Pin Assignment**:
```
PA0-PA7: Port A (8 bits)
PB0-PB7: Port B (8 bits)
PC0-PC7: Port C (8 bits)
D0-D7:   Data bus
A0-A1:   Address (select port or control)
RD:      Read strobe
WR:      Write strobe
RESET:   Reset input
CS:      Chip select
```

**Register Map**:
```
A1 A0 | Register
------+----------
0  0  | Port A
0  1  | Port B
1  0  | Port C
1  1  | Control Word
```

**Control Word Format (Mode 0)**:
```
Bit 7: Mode set flag (1 = mode definition)
Bit 6-5: Group A mode (00 = mode 0)
Bit 4: Port A direction (1=input, 0=output)
Bit 3: Port C upper direction
Bit 2: Group B mode
Bit 1: Port B direction
Bit 0: Port C lower direction
```

**Implementation**:
```cpp
class Intel8255 {
public:
    static constexpr uint8_t NUM_PORTS = 3;
    static constexpr uint8_t PORT_A = 0;
    static constexpr uint8_t PORT_B = 1;
    static constexpr uint8_t PORT_C = 2;

    enum class Mode {
        MODE_0 = 0,  // Basic I/O
        MODE_1 = 1,  // Strobed I/O
        MODE_2 = 2   // Bidirectional bus
    };

    Intel8255();
    void reset();

    // Register access (via address lines A1, A0)
    void write(uint8_t address, uint8_t data);
    uint8_t read(uint8_t address);

    // Port direct access
    void writePortA(uint8_t value);
    void writePortB(uint8_t value);
    void writePortC(uint8_t value);
    uint8_t readPortA() const;
    uint8_t readPortB() const;
    uint8_t readPortC() const;

    // External device interface
    void setExternalPortA(uint8_t value);
    void setExternalPortB(uint8_t value);
    void setExternalPortC(uint8_t value);

    // Control
    void setChipSelect(bool selected);

private:
    void writeControlWord(uint8_t control);
    void applyModeConfiguration();

    uint8_t m_portA;
    uint8_t m_portB;
    uint8_t m_portC;
    uint8_t m_portA_external;
    uint8_t m_portB_external;
    uint8_t m_portC_external;
    uint8_t m_controlWord;

    bool m_portA_isInput;
    bool m_portB_isInput;
    bool m_portCU_isInput; // Upper 4 bits
    bool m_portCL_isInput; // Lower 4 bits

    Mode m_groupA_mode;
    Mode m_groupB_mode;
    bool m_chipSelect;
};
```

**Test Coverage**:
- Mode 0 operation (basic I/O)
- All port read/write operations
- Direction control
- Control word parsing
- External device interface
- Port C split configuration
- **Total**: ~20 tests

**Estimated Time**: 6-8 hours

---

### Intel 8251 - USART (Universal Synchronous/Asynchronous Receiver/Transmitter)

**Package**: 28-pin DIP
**Technology**: NMOS
**Power**: +5V single supply

**Specifications**:
- **Baud Rates**: Up to 64 kbps (sync), 19.2 kbps (async typical)
- **Data Bits**: 5, 6, 7, 8
- **Stop Bits**: 1, 1.5, 2
- **Parity**: None, Even, Odd
- **Modes**: Asynchronous (UART) and Synchronous
- **Buffers**: Transmit and receive buffers
- **Error Detection**: Parity, framing, overrun errors

**Pin Assignment**:
```
D0-D7:   Data bus
RD:      Read control
WR:      Write control
C/D:     Control/Data select
CS:      Chip select
RESET:   Reset
TxD:     Transmit data (serial out)
RxD:     Receive data (serial in)
TxC:     Transmit clock
RxC:     Receive clock
TxRDY:   Transmit ready
RxRDY:   Receive ready
SYNDET:  Sync detect (sync mode)
```

**Register Map**:
```
C/D | RD | WR | Operation
----+----+----+-----------
 0  | 1  | 0  | Write data to transmit buffer
 0  | 0  | 1  | Read data from receive buffer
 1  | 1  | 0  | Write mode/command
 1  | 0  | 1  | Read status
```

**Mode Instruction (Asynchronous)**:
```
Bit 7-6: Baud rate factor (00=sync, 01=x1, 10=x16, 11=x64)
Bit 5-4: Character length (00=5, 01=6, 10=7, 11=8)
Bit 3:   Parity enable
Bit 2:   Parity type (0=odd, 1=even)
Bit 1-0: Stop bits (00=invalid, 01=1, 10=1.5, 11=2)
```

**Command Instruction**:
```
Bit 7: Enter hunt mode (sync)
Bit 6: Internal reset
Bit 5: Request to send (RTS)
Bit 4: Error reset
Bit 3: Send break character
Bit 2: Receive enable
Bit 1: Data terminal ready (DTR)
Bit 0: Transmit enable
```

**Status Register**:
```
Bit 7: DSR (data set ready)
Bit 6: SYNDET/BD (sync detect / break detect)
Bit 5: FE (framing error)
Bit 4: OE (overrun error)
Bit 3: PE (parity error)
Bit 2: TxEMPTY (transmit empty)
Bit 1: RxRDY (receiver ready)
Bit 0: TxRDY (transmitter ready)
```

**Implementation**:
```cpp
class Intel8251 {
public:
    enum class Mode {
        SYNCHRONOUS,
        ASYNC_X1,
        ASYNC_X16,
        ASYNC_X64
    };

    enum class CharLength {
        BITS_5 = 5,
        BITS_6 = 6,
        BITS_7 = 7,
        BITS_8 = 8
    };

    enum class Parity {
        NONE,
        ODD,
        EVEN
    };

    enum class StopBits {
        BITS_1,
        BITS_1_5,
        BITS_2
    };

    Intel8251();
    void reset();

    // Register access
    void write(bool controlData, uint8_t data);
    uint8_t read(bool controlData);

    // Serial interface
    void receiveSerialBit(bool bit);
    bool transmitSerialBit();

    // Status
    bool isTransmitReady() const { return m_txReady; }
    bool isReceiveReady() const { return m_rxReady; }
    bool hasParityError() const { return m_parityError; }
    bool hasFramingError() const { return m_framingError; }
    bool hasOverrunError() const { return m_overrunError; }

    // Clock
    void transmitClock();
    void receiveClock();

private:
    void writeModeInstruction(uint8_t mode);
    void writeCommandInstruction(uint8_t cmd);
    uint8_t readStatus() const;
    uint8_t readData();
    void writeData(uint8_t data);

    // Configuration
    Mode m_mode;
    CharLength m_charLength;
    Parity m_parity;
    StopBits m_stopBits;

    // Buffers
    uint8_t m_txBuffer;
    uint8_t m_rxBuffer;

    // Status flags
    bool m_txReady;
    bool m_rxReady;
    bool m_txEmpty;
    bool m_parityError;
    bool m_framingError;
    bool m_overrunError;

    // Control
    bool m_txEnable;
    bool m_rxEnable;
    bool m_rts;
    bool m_dtr;

    // State machine
    uint8_t m_txShiftReg;
    uint8_t m_rxShiftReg;
    uint8_t m_txBitCount;
    uint8_t m_rxBitCount;

    bool m_modeInstructionWritten;
};
```

**Test Coverage**:
- Mode configuration (async modes)
- Character transmission
- Character reception
- Parity checking
- Error detection (framing, overrun, parity)
- Status register
- Buffer management
- **Total**: ~25 tests

**Estimated Time**: 8-12 hours

---

### 7-Segment LED Display Emulation

**Specifications**:
- **Segments**: a, b, c, d, e, f, g, dp (decimal point)
- **Common Types**: Common Anode or Common Cathode
- **Interface**: 7/8-bit parallel
- **Typical Chips**: TIL311, HP 5082-7340
- **Digits**: 1-8 digit displays

**Segment Layout**:
```
     _a_
   f|   |b
    |_g_|
   e|   |c
    |___|  .dp
      d
```

**Encoding**:
```
Bit 7: dp (decimal point)
Bit 6: g (middle)
Bit 5: f (top-left)
Bit 4: e (bottom-left)
Bit 3: d (bottom)
Bit 2: c (bottom-right)
Bit 1: b (top-right)
Bit 0: a (top)
```

**Implementation**:
```cpp
class SevenSegmentDisplay {
public:
    SevenSegmentDisplay(size_t numDigits = 1);

    // Set segments directly
    void setSegments(size_t digit, uint8_t segments);
    uint8_t getSegments(size_t digit) const;

    // Set from hex/BCD
    void setHexDigit(size_t digit, uint8_t value);
    void setBCDDigit(size_t digit, uint8_t value);

    // Segment control
    void setSegment(size_t digit, char segment, bool on);
    bool getSegment(size_t digit, char segment) const;

    // Display as text (for testing/debugging)
    std::string toASCII(size_t digit) const;

    size_t numDigits() const { return m_digits.size(); }

private:
    std::vector<uint8_t> m_digits;

    static const uint8_t HEX_PATTERNS[16];
    static const uint8_t BCD_PATTERNS[10];
};
```

**Test Coverage**:
- Hex digit display (0-F)
- BCD digit display (0-9)
- Individual segment control
- Multiple digit displays
- ASCII representation
- **Total**: ~15 tests

**Estimated Time**: 3-4 hours

---

### Matrix Keyboard Interface

**Specifications**:
- **Common Sizes**: 4×4, 8×8
- **Scanning**: Row/column scanning
- **Debouncing**: Software debounce required
- **Interface**: Parallel I/O

**4×4 Keypad Layout**:
```
     COL0  COL1  COL2  COL3
ROW0  [1]   [2]   [3]   [A]
ROW1  [4]   [5]   [6]   [B]
ROW2  [7]   [8]   [9]   [C]
ROW3  [*]   [0]   [#]   [D]
```

**Scanning Algorithm**:
1. Drive one row LOW, others HIGH
2. Read column inputs
3. If column is LOW, key at (row, col) is pressed
4. Repeat for all rows

**Implementation**:
```cpp
class MatrixKeyboard {
public:
    MatrixKeyboard(size_t rows = 4, size_t cols = 4);

    // Scanning interface
    void setRowOutput(uint8_t rowMask);
    uint8_t readColumnInput() const;

    // Key state (for simulation)
    void pressKey(size_t row, size_t col);
    void releaseKey(size_t row, size_t col);
    bool isKeyPressed(size_t row, size_t col) const;

    // Full scan
    struct ScanResult {
        bool keyPressed;
        size_t row;
        size_t col;
    };
    ScanResult scan();

    // Debouncing
    void tick(); // Call periodically
    bool getStableKey(size_t& row, size_t& col);

private:
    size_t m_rows;
    size_t m_cols;
    std::vector<std::vector<bool>> m_keyState;
    std::vector<std::vector<uint8_t>> m_debounceCounter;

    uint8_t m_currentRowMask;

    static constexpr uint8_t DEBOUNCE_TICKS = 3;
};
```

**Test Coverage**:
- Single key press/release
- Multiple simultaneous keys
- Scanning all keys
- Debouncing logic
- Row/column mapping
- **Total**: ~12 tests

**Estimated Time**: 4-5 hours

---

## 6. Implementation Order

### Step-by-Step Execution Plan

#### Week 1: Core 4-Bit Architecture

**Day 1-2: Nibble Foundation**
- [ ] Create `nibble.hpp` with Nibble class
- [ ] Implement all nibble arithmetic operations
- [ ] Implement carry/borrow detection
- [ ] Implement BCD operations
- [ ] Write 50+ nibble unit tests
- [ ] All tests passing

**Day 3: Multi-Nibble Types**
- [ ] Create `nibble_pair.hpp` with NibblePair
- [ ] Create `address12.hpp` with Address12
- [ ] Create `nibble_array.hpp` with NibbleArray<N>
- [ ] Write 30+ tests for multi-nibble types
- [ ] All tests passing

**Day 4-5: CPU Refactoring**
- [ ] Refactor CPU registers to use Nibble
- [ ] Refactor accumulator to Nibble
- [ ] Refactor ALU operations
- [ ] Update instruction execution
- [ ] Run all existing CPU tests
- [ ] Fix any failures
- [ ] All CPU tests passing

**Day 6-7: Memory Refactoring**
- [ ] Refactor 4002 RAM to NibbleArray
- [ ] Refactor 4001 ROM to NibbleArray
- [ ] Refactor 4101 SRAM to NibbleArray
- [ ] Update addressing logic
- [ ] Run all memory tests
- [ ] Fix any failures
- [ ] All memory tests passing

**End of Week 1: Commit "Core 4-Bit Architecture"**

---

#### Week 2: Complete Refactoring + Start Phase 4

**Day 8-9: I/O Chips Refactoring**
- [ ] Refactor 4003 shift register
- [ ] Refactor 4008 address latch
- [ ] Refactor 4009 converter
- [ ] Refactor 4201A clock
- [ ] Run all I/O tests
- [ ] All tests passing

**Day 10: Large Memory Chips**
- [ ] Refactor 4289
- [ ] Refactor 4308
- [ ] Refactor 4702
- [ ] Run all tests
- [ ] All tests passing

**End of Day 10: Commit "Complete 4-Bit Architecture Refactoring"**

**Day 11-12: Intel 8255 PPI**
- [ ] Create K8255.hpp/cpp
- [ ] Implement all modes (Mode 0 initially)
- [ ] Implement port operations
- [ ] Implement control word parsing
- [ ] Write 20 tests
- [ ] All tests passing

**Day 13-14: Intel 8251 USART**
- [ ] Create K8251.hpp/cpp
- [ ] Implement asynchronous mode
- [ ] Implement serial transmission
- [ ] Implement serial reception
- [ ] Implement error detection
- [ ] Write 25 tests
- [ ] All tests passing

**End of Week 2: Commit "Phase 4: Essential Peripherals Part 1"**

---

#### Week 3: Complete Phase 4

**Day 15: 7-Segment Display**
- [ ] Create SevenSegmentDisplay.hpp/cpp
- [ ] Implement segment control
- [ ] Implement hex/BCD encoding
- [ ] Implement ASCII representation
- [ ] Write 15 tests
- [ ] All tests passing

**Day 16: Matrix Keyboard**
- [ ] Create MatrixKeyboard.hpp/cpp
- [ ] Implement scanning logic
- [ ] Implement debouncing
- [ ] Implement key state management
- [ ] Write 12 tests
- [ ] All tests passing

**Day 17: Integration Testing**
- [ ] Create 8255 + Display integration test
- [ ] Create 8255 + Keyboard integration test
- [ ] Create 8251 + Serial loopback test
- [ ] Create complete system test
- [ ] Write 10 integration tests
- [ ] All tests passing

**Day 18: Documentation**
- [ ] Update all chip documentation
- [ ] Create Phase 4 user guide
- [ ] Create 4-bit architecture guide
- [ ] Update main README
- [ ] Create example programs

**Day 19: Build System**
- [ ] Update CMakeLists.txt
- [ ] Ensure clean build
- [ ] Run full test suite (200+ tests)
- [ ] Fix any issues
- [ ] All tests passing

**Day 20: Final Commit and Review**
- [ ] Commit "Phase 4: Complete"
- [ ] Push all changes
- [ ] Create detailed commit message
- [ ] Update roadmap status
- [ ] Celebrate! 🎉

**End of Week 3: Phase 4 Complete + Pure 4-Bit Architecture**

---

## 7. Testing Strategy

### Test Categories

**1. Unit Tests (Per Component)**
- Individual function testing
- Boundary conditions
- Error cases
- Edge cases

**2. Integration Tests**
- Multi-chip interaction
- Bus protocol testing
- Real-world scenarios

**3. Architecture Tests**
- Nibble operation correctness
- Carry/borrow propagation
- BCD validation
- Multi-nibble operations

**4. Regression Tests**
- Ensure refactoring doesn't break existing functionality
- All previous tests must pass

### Test Metrics

**Target Coverage**:
- Nibble operations: 100% coverage
- All chips: >95% coverage
- Integration: >85% coverage

**Expected Test Count**:
- Nibble tests: 80+
- Refactored chip tests: 127+ (existing)
- New Phase 4 tests: 82+
- Integration tests: 15+
- **Total**: 300+ tests

---

## 8. Success Criteria

### Architecture Refactoring

✅ **Complete** when:
- All data uses Nibble/NibblePair/NibbleArray
- No uint8_t in public chip interfaces (except for 8-bit peripherals)
- All arithmetic uses nibble operations
- All existing tests pass
- Nibble tests have 100% coverage

### Phase 4 Implementation

✅ **Complete** when:
- 8255 PPI fully functional (Mode 0 minimum)
- 8251 USART functional (async mode)
- 7-segment display working
- Matrix keyboard working
- All tests pass (82+ new tests)
- Documentation complete
- Build system updated

---

## 9. Risk Mitigation

### Potential Issues

**1. Refactoring Breaking Tests**
- **Mitigation**: Incremental refactoring with continuous testing
- **Backup**: Git commits after each successful phase

**2. Performance Degradation**
- **Mitigation**: Profile code, optimize hot paths
- **Backup**: Keep uint8_t internally if needed, expose Nibble in API

**3. Complex Debugging**
- **Mitigation**: Excellent test coverage catches issues early
- **Backup**: Nibble::value() allows inspecting raw values

**4. Time Overrun**
- **Mitigation**: Granular task breakdown with time estimates
- **Backup**: Can defer Mode 1/2 of 8255, sync mode of 8251

---

## 10. Future Enhancements (Post-Phase 4)

### Phase 5 Candidates
- 8279 Keyboard/Display Controller
- 8253 Timer/Counter
- Cassette interface (Kansas City Standard)
- Character LCD display
- Enhanced 8255 modes (Mode 1, Mode 2)
- Enhanced 8251 modes (synchronous)

### Phase 6+ Ideas
- Paper tape emulation
- Floppy disk interface
- VGA display output
- Full system simulator with UI
- Cycle-accurate timing
- Historical program library

---

**AD ASTRA PER MATHEMATICA ET SCIENTIAM**

*Building the most accurate 4-bit Intel hardware emulation, one nibble at a time.*

**Document Status:** Ready for Implementation
**Granularity Level:** Step-by-step with daily tasks
**Test Coverage Goal:** 300+ tests
**Timeline:** 3 weeks for complete implementation
