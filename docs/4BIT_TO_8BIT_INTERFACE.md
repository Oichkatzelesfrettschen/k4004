# 4-Bit to 8-Bit Interface Architecture
# Intel 4004/4040 Peripheral Interfacing Guide

**Version:** 1.0
**Date:** 2025-11-19
**Purpose:** Explain how 4-bit CPUs interface with 8-bit peripherals

---

## Table of Contents

1. [The 4-Bit Challenge](#the-4-bit-challenge)
2. [Historical Interface Solutions](#historical-interface-solutions)
3. [Intel's Official Interface Chips](#intels-official-interface-chips)
4. [Nibble-Wise Access Patterns](#nibble-wise-access-patterns)
5. [Modern Peripheral Compatibility](#modern-peripheral-compatibility)
6. [Implementation Strategy](#implementation-strategy)

---

## 1. The 4-Bit Challenge

### The Problem

The Intel 4004/4040 are **TRUE 4-bit processors**:
- **Data bus**: 4 bits wide (D0-D3)
- **ALU**: 4-bit operations
- **Registers**: 4-bit nibbles

But the world was moving to **8-bit peripherals**:
- Intel 8080 (8-bit CPU) released 1974
- 8-bit peripherals (8255, 8251, etc.) standard by 1975
- Memory chips: 8-bit wide (2102, 1702A)
- Industry standardized on bytes (8 bits)

### The Solution

**Nibble-Wise Computing**: Access 8-bit data as two sequential 4-bit operations

```
8-bit byte: 0xAB
├─ High nibble: 0xA (bits 7-4)
└─ Low nibble:  0xB (bits 3-0)

CPU reads/writes in two cycles:
Cycle 1: Transfer low nibble  (0xB)
Cycle 2: Transfer high nibble (0xA)
```

---

## 2. Historical Interface Solutions

### 2.1 Native 4-Bit Peripherals (1971-1973)

Intel's original MCS-4 chipset was purely 4-bit:

| Chip | Function | I/O Width | Era |
|------|----------|-----------|-----|
| **4001** | ROM | 4-bit I/O port | 1971 |
| **4002** | RAM | 4-bit output port | 1971 |
| **4003** | Shift Register | 10-bit parallel out | 1971 |

**No interface needed** - direct 4-bit connection!

### 2.2 Transition Era (1974-1976)

Mixed 4-bit and 8-bit peripherals. Intel created bridge chips:

| Chip | Function | Bridge Type |
|------|----------|-------------|
| **4008** | Address Latch | 4-bit → 8-bit address |
| **4009** | Data Converter | 4-bit ↔ 8-bit data |
| **4289** | Unified Interface | Combines 4008+4009 |

### 2.3 Modern Era (1976+)

Full 8-bit peripherals required interface logic:

| Peripheral | Width | Interface Method |
|------------|-------|------------------|
| **8255 PPI** | 8-bit × 3 | Via 4008/4009 or 4289 |
| **8251 USART** | 8-bit | Via 4008/4009 or 4289 |
| **2102 RAM** | 1-bit × 1024 | Via 4008/4009 (parallel) |
| **1702A EPROM** | 8-bit × 256 | Via 4008/4009 or 4289 |

---

## 3. Intel's Official Interface Chips

### 3.1 Intel 4008 - 8-Bit Address Latch

**Function**: Latch 8-bit addresses from two 4-bit cycles

```
┌─────────┐
│  4040   │ 4-bit data bus
│   CPU   ├──────────┐
└─────────┘          │
                     │ D0-D3 (4 bits)
                 ┌───▼────┐
                 │  4008  │
                 │ Latch  │
                 └───┬────┘
                     │ A0-A7 (8 bits)
                     ▼
              ┌──────────┐
              │ 8-bit    │
              │ Memory   │
              └──────────┘
```

**Operation**:
1. CPU sends low nibble (A0-A3) → 4008 latches bits 0-3
2. CPU sends high nibble (A4-A7) → 4008 latches bits 4-7
3. 4008 outputs stable 8-bit address to memory

**Also provides**:
- 4-bit chip select (CM0-CM3) → selects 1 of 16 memory chips
- 4-bit I/O port (like 4001 ROM)

### 3.2 Intel 4009 - 4-to-8 Bit Data Converter

**Function**: Convert between 4-bit CPU bus and 8-bit peripheral bus

```
Read Operation (8-bit → 4-bit):
┌──────────┐
│ 8-bit    │
│ Memory   │ 8 bits (D0-D7)
└────┬─────┘
     │
┌────▼────┐
│  4009   │ Converts 8→4
│ Convert │
└────┬────┘
     │ 4 bits (D0-D3)
┌────▼────┐
│  4040   │ Reads in 2 cycles
│   CPU   │
└─────────┘

Cycle 1: CPU reads low nibble
Cycle 2: CPU reads high nibble

Write Operation (4-bit → 8-bit):
┌─────────┐
│  4040   │
│   CPU   │ Writes in 2 cycles
└────┬────┘
     │ 4 bits (D0-D3)
┌────▼────┐
│  4009   │ Assembles 4→8
│ Convert │
└────┬────┘
     │ 8 bits (D0-D7)
┌────▼────┐
│ 8-bit   │
│ Periph  │
└─────────┘

Cycle 1: CPU writes low nibble → 4009 stores
Cycle 2: CPU writes high nibble → 4009 outputs complete byte
```

**Features**:
- Bidirectional (read and write)
- Two-cycle operation
- Mode control (program/I/O)
- Automatic nibble sequencing

### 3.3 Intel 4289 - Standard Memory Interface

**Function**: Unified 4008+4009 in single chip

```
┌─────────┐
│  4040   │ 4-bit bus
│   CPU   ├──────────┐
└─────────┘          │ D0-D3 (4 bits)
                     │
                 ┌───▼─────┐
                 │  4289   │
                 │ Unified │
                 │Interface│
                 └───┬─────┘
                     │ 8-bit address
                     │ 8-bit data
                     │ Control signals
                     ▼
         ┌───────────────────────┐
         │ Standard 8-bit        │
         │ Memory/Peripherals    │
         │ (2102, 1702A, etc.)   │
         └───────────────────────┘
```

**Advantages over 4008/4009 pair**:
- Single chip (saves board space)
- Integrated control logic
- Built-in 4001-compatible I/O port
- Simpler timing

**Specifications**:
- 12-bit address output (8-bit + 4-bit chip select)
- 8-bit bidirectional data bus
- Supports RAM as program memory
- Up to 8KB addressable

---

## 4. Nibble-Wise Access Patterns

### 4.1 Memory Read (8-bit)

```assembly
; Read byte from address 0x123 into register pair RP0

; Step 1: Load address into 4008/4289
FIM RP1, 0x23        ; Load low byte of address (0x23)
SRC RP1              ; Send to 4008 (A0-A7)
FIM RP2, 0x01        ; Load high nibble (0x1)
; 4008 now has complete address 0x123

; Step 2: Read data via 4009/4289 (two cycles)
RDM                  ; Read memory → low nibble to accumulator
XCH R0               ; Store low nibble in R0
RDM                  ; Read memory → high nibble to accumulator
XCH R1               ; Store high nibble in R1
; RP0 now contains complete byte
```

**Timing**:
- Address setup: 2 instruction cycles
- Data read: 2 instruction cycles (one per nibble)
- Total: 4 instruction cycles = ~32 CPU cycles

### 4.2 Memory Write (8-bit)

```assembly
; Write byte 0xAB to address 0x123

; Step 1: Set address (same as read)
FIM RP1, 0x23
SRC RP1
FIM RP2, 0x01

; Step 2: Write data (two cycles)
LDM 0x0B             ; Load low nibble
WRM                  ; Write to memory (4009 buffers)
LDM 0x0A             ; Load high nibble
WRM                  ; Write to memory (4009 completes write)
```

### 4.3 8255 PPI Access (8-bit peripheral)

```assembly
; Configure 8255: Write 0x80 to control register
; Control register address via 4008: A1=1, A0=1

; Set address (Port select via A1, A0)
FIM RP1, 0x03        ; A1=1, A0=1 (control register)
SRC RP1              ; Send to 4008

; Write control word 0x80 (all outputs)
LDM 0x00             ; Low nibble of 0x80
WRM                  ; Write via 4009
LDM 0x08             ; High nibble of 0x80
WRM                  ; Complete write

; Now write to Port A (address 00)
FIM RP1, 0x00        ; Port A address
SRC RP1
LDM 0x0A             ; Low nibble
WRM
LDM 0x0B             ; High nibble → Port A outputs 0xAB
WRM
```

---

## 5. Modern Peripheral Compatibility

### 5.1 Era-Appropriate Peripherals (1970s)

These work directly with 4-bit interface:

| Peripheral | Width | Interface | Notes |
|------------|-------|-----------|-------|
| **4001 ROM** | 4-bit I/O | Direct | Native 4-bit |
| **4002 RAM** | 4-bit out | Direct | Native 4-bit |
| **4003 Shift** | 10-bit | Direct | Serial 4-bit in |
| **4101 SRAM** | 4-bit data | Direct | True 4-bit memory |
| **4308 ROM** | 8-bit | Via 4289 | Needs converter |
| **4702 EPROM** | 8-bit | Via 4289 | Needs converter |
| **8255 PPI** | 8-bit × 3 | Via 4289 | Needs converter |
| **8251 USART** | 8-bit | Via 4289 | Needs converter |

### 5.2 Modern Peripherals (1980s+)

Can be interfaced with proper adapters:

| Peripheral | Era | Interface Method |
|------------|-----|------------------|
| **27C256 EPROM** | 1980s | 4289 + custom logic |
| **62256 SRAM** | 1980s | 4289 + custom logic |
| **16550 UART** | 1987 | 4289 + address decode |
| **HD44780 LCD** | 1987 | 4-bit mode native! |
| **SPI devices** | 1990s+ | Via bit-banging on 4-bit I/O |
| **I2C devices** | 1990s+ | Via bit-banging on 4-bit I/O |

**Important**: Many modern chips have 4-bit modes!
- HD44780 LCD: Has native 4-bit mode (perfect for 4004!)
- SD cards: SPI mode works with bit-banging
- Serial EEPROMs: I2C bit-banging on 2 I/O pins

### 5.3 Interface Adapter Design

For modern peripherals, create custom interface chips:

```
┌──────────────────────────────────────┐
│  4-Bit to 8-Bit Universal Adapter   │
├──────────────────────────────────────┤
│ Input: 4-bit data bus                │
│ Output: 8-bit peripheral bus         │
│                                       │
│ Features:                             │
│ - Dual nibble latch                  │
│ - Bidirectional conversion           │
│ - Address decode (A0-A1)             │
│ - Chip select generation             │
│ - Read/Write strobes                 │
└──────────────────────────────────────┘
```

**Implementation in our emulator**:
```cpp
class NibbleToByteAdapter {
public:
    // CPU writes nibble
    void writeNibble(bool highNibble, uint8_t nibble);

    // CPU reads nibble
    uint8_t readNibble(bool highNibble);

    // Get complete byte for peripheral
    uint8_t getByteForPeripheral();

    // Set byte from peripheral
    void setByteFromPeripheral(uint8_t byte);
};
```

---

## 6. Implementation Strategy

### 6.1 Emulator Architecture

```
┌─────────────────────────────────────────────┐
│              4-Bit CPU Layer                │
│  (Intel 4004/4040 with nibble operations)   │
└──────────────────┬──────────────────────────┘
                   │ 4-bit data bus
                   │
        ┌──────────▼──────────┐
        │  Interface Layer    │
        │ (4008/4009/4289)    │
        │  NibbleToByteAdapter│
        └──────────┬──────────┘
                   │ 8-bit data bus
                   │
    ┌──────────────┴──────────────┐
    │                             │
┌───▼────┐  ┌────────┐  ┌────────▼─────┐
│ Native │  │ 8-bit  │  │   Modern     │
│ 4-bit  │  │ Periph │  │  Peripherals │
│ Chips  │  │(8255)  │  │ (LCD, SPI)   │
└────────┘  └────────┘  └──────────────┘
```

### 6.2 Access Pattern Implementation

**Option 1: Transparent Adapter** (Automatic)
```cpp
// CPU doesn't know about nibble conversion
cpu.writeByte(address, 0xAB);
// Adapter automatically splits: 0xB, then 0xA

uint8_t value = cpu.readByte(address);
// Adapter automatically combines two nibble reads
```

**Option 2: Explicit Nibble** (Historical)
```cpp
// CPU explicitly does nibble operations
cpu.writeNibble(address, 0xB, false);  // Low nibble
cpu.writeNibble(address, 0xA, true);   // High nibble

uint8_t low = cpu.readNibble(address, false);
uint8_t high = cpu.readNibble(address, true);
uint8_t byte = low | (high << 4);
```

**Our Implementation**: Support both!

### 6.3 Peripheral Categories

**Category A: Native 4-Bit** (No adapter needed)
- 4001, 4002, 4003, 4101
- Direct connection to 4-bit bus
- Access via nibble operations

**Category B: 8-Bit with 4-Bit Mode** (Minimal adapter)
- HD44780 LCD (has 4-bit mode)
- Some modern peripherals with nibble modes
- Simple mode configuration

**Category C: 8-Bit Only** (Full adapter required)
- 8255, 8251, memory chips
- Require 4008/4009/4289 interface
- Two-cycle access pattern

---

## 7. Practical Examples

### 7.1 Calculator System (All 4-Bit)

```
4004 CPU
├─ 4001 ROM (program)
├─ 4002 RAM (data)
├─ 4003 Shift Register → 7-segment displays
└─ 4003 Shift Register → keyboard scan

No interface chips needed!
All native 4-bit peripherals.
```

### 7.2 Printer Interface System (Mixed)

```
4040 CPU
├─ 4001 ROM (program)
├─ 4002 RAM (data)
├─ 4289 Interface
│   └─ 8255 PPI
│       ├─ Port A: Printer data (8 bits)
│       ├─ Port B: Status input (8 bits)
│       └─ Port C: Control signals
└─ 4003 → Status LEDs

Interface chip required for 8-bit 8255.
```

### 7.3 Modern LCD System (Hybrid)

```
4040 CPU
├─ 4308 ROM (program, via 4289)
├─ 4101 RAM (data, native 4-bit)
├─ HD44780 LCD (4-bit mode, direct!)
└─ 4-bit I/O → Buttons

LCD has native 4-bit mode!
No 4289 needed for display.
```

---

## 8. Conclusion

### Key Takeaways

1. **4-bit CPUs CAN access 8-bit peripherals**
   - Via interface chips (4008/4009/4289)
   - Using two-cycle nibble-wise access
   - Standard practice in the era

2. **Not all peripherals need interfaces**
   - Native 4-bit chips work directly
   - Many modern chips have 4-bit modes
   - Bit-banging works for serial protocols

3. **Emulator must support both**
   - Direct 4-bit peripheral access
   - 4-to-8 bit adapter layer
   - Transparent or explicit modes

4. **Historical accuracy maintained**
   - Real systems used these exact techniques
   - Same timing and patterns
   - Era-appropriate solutions

### Implementation Priority

Our emulator implements:
1. ✅ Native 4-bit types (Nibble, NibblePair)
2. ✅ Native 4-bit peripherals (displays, keyboards)
3. ✅ 8-bit peripherals with adapter layer (8255)
4. 🔄 Next: More 8-bit peripherals (8251 USART)
5. 🔄 Next: Explicit 4289 interface chip emulation
6. 🔄 Future: Modern peripheral adapters

---

**AD ASTRA PER MATHEMATICA ET SCIENTIAM**

*Bridging 4-bit and 8-bit worlds, just as the pioneers did in 1974.*

**Document Version:** 1.0
**Last Updated:** 2025-11-19
**Status:** Complete Reference
