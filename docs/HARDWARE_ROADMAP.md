# Intel MCS-4/MCS-40 Hardware Emulation Roadmap

**Version:** 1.0
**Date:** 2025-11-19
**Purpose:** Comprehensive guide for implementing historically accurate hardware emulation

---

## Table of Contents

1. [Overview](#overview)
2. [Intel MCS-4 Chipset Family](#intel-mcs-4-chipset-family)
3. [Intel MCS-40 Chipset Family](#intel-mcs-40-chipset-family)
4. [Support and Interface Chips](#support-and-interface-chips)
5. [Clock Generation and Timing](#clock-generation-and-timing)
6. [Era-Appropriate Peripherals (1970-1980)](#era-appropriate-peripherals-1970-1980)
7. [Electrical Characteristics](#electrical-characteristics)
8. [Implementation Priority](#implementation-priority)
9. [Detailed Specifications](#detailed-specifications)
10. [Integration Architecture](#integration-architecture)

---

## 1. Overview

This document provides a granular, comprehensive list of ALL hardware components that should be emulated for a complete, historically accurate Intel MCS-4 and MCS-40 microcomputer system. This goes far beyond the basic CPU emulation to encompass the entire ecosystem of chips, peripherals, and support hardware available in the 1970-1980 era.

### Design Philosophy

- **Historical Accuracy**: Emulate chips as they actually worked
- **Modularity**: Each chip is a separate, reusable component
- **Scalability**: Support minimal to maximal system configurations
- **Era-Appropriate**: Focus on 1970-1980 technology
- **Educational Value**: Demonstrate early microcomputer architecture

---

## 2. Intel MCS-4 Chipset Family

The MCS-4 (Micro Computer Set - 4 bit) was Intel's first complete microcomputer system, introduced November 15, 1971.

### 2.1 Core Components

| Chip | Type | Function | Status | Priority |
|------|------|----------|--------|----------|
| **4004** | CPU | 4-bit microprocessor, 740 kHz | ✅ Complete | **DONE** |
| **4001** | ROM | 256-byte ROM + 4-bit I/O port | ⚠️ Partial | **HIGH** |
| **4002** | RAM | 40-byte RAM + 4-bit output port | ⚠️ Partial | **HIGH** |
| **4003** | I/O | 10-bit parallel shift register | ❌ Missing | **HIGH** |

#### 2.1.1 Intel 4001 ROM - Detailed Specifications

**Function:** Program memory and I/O

**Package:** 16-pin DIP

**Specifications:**
- **ROM Size:** 256 bytes (256 × 8-bit instructions)
- **Organization:** 256 words × 8 bits
- **I/O Port:** 4-bit programmable I/O
- **Address Range:** Chip-select via 4-bit address
- **Access Time:** Typ. 1.2 μs
- **Technology:** Silicon-gate P-MOS
- **Supply Voltage:** -15V, +5V (GND)

**Emulation Requirements:**
- [ ] ROM storage array (256 × 8 bits)
- [ ] 4-bit I/O port with direction control
- [ ] Chip selection logic (4-bit address decode)
- [ ] Read/write I/O port control
- [ ] Timing simulation (access delays)
- [ ] Multiple instance support (up to 16 chips)

**System Integration:**
- Maximum 16 × 4001 chips per system = 4KB program memory
- Cannot coexist with 4008/4009 in same system
- Directly controlled by 4004 CPU

#### 2.1.2 Intel 4002 RAM - Detailed Specifications

**Function:** Data memory and output port

**Package:** 16-pin DIP

**Specifications:**
- **RAM Size:** 40 nibbles (40 × 4-bit)
- **Organization:** 4 registers × 16 data characters + 4 status characters
- **Data Characters:** 16 × 4-bit per register (64 nibbles total)
- **Status Characters:** 4 × 4-bit per register (16 nibbles total)
- **Output Port:** 4-bit output-only
- **Access Time:** Typ. 1.2 μs
- **Technology:** Silicon-gate P-MOS
- **Supply Voltage:** -15V, +5V (GND)

**Memory Organization:**
```
Each 4002 chip contains:
├── Register 0
│   ├── Main Characters: [0-15] (16 × 4-bit)
│   └── Status Characters: [0-3] (4 × 4-bit)
├── Register 1
│   ├── Main Characters: [0-15]
│   └── Status Characters: [0-3]
├── Register 2
│   ├── Main Characters: [0-15]
│   └── Status Characters: [0-3]
└── Register 3
    ├── Main Characters: [0-15]
    └── Status Characters: [0-3]
```

**Emulation Requirements:**
- [x] Data RAM array (64 × 4-bit)
- [x] Status RAM array (16 × 4-bit)
- [ ] 4-bit output port
- [ ] Register addressing logic
- [ ] Character addressing within registers
- [ ] Bank selection (DCL command)
- [ ] Multiple instance support (up to 16 chips in 4 banks)

**System Integration:**
- Maximum 16 × 4002 chips = 640 nibbles data + 256 nibbles status
- Organized in 4 banks of 4 chips each
- Bank selected via DCL instruction
- Chip and register selected via SRC instruction

#### 2.1.3 Intel 4003 Shift Register - Detailed Specifications

**Function:** I/O expansion and peripheral control

**Package:** 16-pin DIP

**Specifications:**
- **Type:** 10-bit parallel output shift register
- **Inputs:** Serial data in, clock, enable
- **Outputs:** 10 parallel outputs
- **Technology:** Silicon-gate P-MOS
- **Supply Voltage:** -15V, +5V (GND)
- **Clock Rate:** Sync with CPU clock

**Typical Applications:**
- Keyboard matrix scanning
- LED/7-segment display control
- Printer control
- External device control

**Emulation Requirements:**
- [ ] 10-bit shift register
- [ ] Serial input (from 4001/4002 I/O)
- [ ] Parallel output latches
- [ ] Clock synchronization
- [ ] Output enable control
- [ ] Multiple instance support (theoretically unlimited)

**System Integration:**
- Connected to I/O ports of 4001 or 4002
- Cascadable for more outputs
- Common use: 2-3 chips for keyboard/display matrix

### 2.2 Extended Interface Components

| Chip | Type | Function | Status | Priority |
|------|------|----------|--------|----------|
| **4008** | Interface | 8-bit address latch + I/O | ❌ Missing | MEDIUM |
| **4009** | Interface | Memory access converter | ❌ Missing | MEDIUM |
| **4289** | Interface | Standard memory interface (replaces 4008/4009) | ❌ Missing | MEDIUM |

#### 2.2.1 Intel 4008/4009 Pair - Standard Memory Interface

**4008 - 8-Bit Address Latch:**

**Package:** 16-pin DIP

**Specifications:**
- **Function:** Address latch for standard memory
- **Outputs:** 8-bit address
- **Chip Select:** 4-bit binary chip select
- **I/O Port:** Built-in 4-bit I/O

**4009 - Program/I/O Access Converter:**

**Package:** 16-pin DIP

**Specifications:**
- **Function:** Convert 4-bit bus to 8-bit parallel
- **Interfaces:** Standard RAM and ROM chips
- **Supported Memory:** C2102A (RAM), C1702A (EPROM)

**Purpose:**
- Allow 4004 to interface with standard 8-bit memory chips
- Expand beyond 4001/4002 limitations
- Access commercially available memory

**Emulation Requirements:**
- [ ] 8-bit address latching
- [ ] 4-to-8 bit bus conversion
- [ ] Chip select decode logic
- [ ] Synchronization with CPU timing

#### 2.2.2 Intel 4289 - Standard Memory Interface

**Package:** 24-pin DIP

**Function:** Combines 4008/4009 functionality in single chip

**Specifications:**
- **Address Output:** 12-bit (8-bit + 4-bit chip select)
- **Data Bus:** 8-bit bidirectional
- **Memory Support:** Standard RAMs and ROMs
- **RAM as Program Memory:** Allows RAM-based programs

**Emulation Requirements:**
- [ ] 12-bit address generation
- [ ] 8-bit data bus interface
- [ ] Read/write control logic
- [ ] Chip select generation
- [ ] Timing compatibility with 4004/4040

---

## 3. Intel MCS-40 Chipset Family

The MCS-40 was the enhanced successor to MCS-4, introduced in 1974 with the 4040 CPU.

### 3.1 Core Components

| Chip | Type | Function | Status | Priority |
|------|------|----------|--------|----------|
| **4040** | CPU | Enhanced 4-bit CPU | ✅ Complete | **DONE** |
| **4001** | ROM | (Same as MCS-4) | ⚠️ Partial | **HIGH** |
| **4002** | RAM | (Same as MCS-4) | ⚠️ Partial | **HIGH** |
| **4003** | I/O | (Same as MCS-4) | ❌ Missing | **HIGH** |
| **4289** | Interface | Standard memory interface | ❌ Missing | MEDIUM |

### 3.2 Enhanced Memory Components

| Chip | Type | Function | Status | Priority |
|------|------|----------|--------|----------|
| **4101** | SRAM | 256 × 4-bit static RAM | ❌ Missing | MEDIUM |
| **4308** | ROM | 1024 × 8-bit ROM + 4-bit I/O | ❌ Missing | MEDIUM |
| **4316** | ROM | 2048 × 8-bit static ROM | ❌ Missing | LOW |
| **4702** | EPROM | 256 × 8-bit erasable PROM | ❌ Missing | LOW |

#### 3.2.1 Intel 4101 - 1K Static RAM

**Package:** 16-pin DIP

**Specifications:**
- **Capacity:** 1024 bits (256 × 4)
- **Organization:** 256 words × 4 bits
- **Access Time:** Typ. 500 ns
- **Technology:** NMOS static
- **Power:** +5V, -12V
- **I/O:** Separate input/output

**Advantages:**
- Much larger than 4002 (256 × 4 vs 40 × 4)
- Faster access time
- No status character complexity
- Standard memory interface

**Emulation Requirements:**
- [ ] 256 × 4-bit storage array
- [ ] Address decode (8-bit)
- [ ] Read/write control
- [ ] Chip select logic
- [ ] Timing accurate to 500ns access

#### 3.2.2 Intel 4308 - 8K ROM with I/O

**Package:** 24-pin DIP

**Specifications:**
- **Capacity:** 8192 bits (1024 × 8)
- **Organization:** 1024 words × 8 bits
- **I/O Ports:** 4-bit I/O capability
- **Access Time:** Typ. 800 ns
- **Address:** 10-bit addressing

**Use Case:**
- Larger program storage than 4001
- Built-in I/O like 4001
- Better for complex programs

**Emulation Requirements:**
- [ ] 1024 × 8-bit ROM storage
- [ ] 10-bit address decode
- [ ] 4-bit I/O port
- [ ] Timing simulation

#### 3.2.3 Intel 4316 - 16K Static ROM

**Package:** 24-pin DIP

**Specifications:**
- **Capacity:** 16384 bits (2048 × 8)
- **Organization:** 2048 words × 8 bits
- **Access Time:** Typ. 450 ns
- **Address:** 11-bit addressing

**Use Case:**
- Maximum program storage for era
- Fast access time
- Production ROMs

#### 3.2.4 Intel 4702 - 2K EPROM

**Package:** 24-pin DIP (ceramic, UV-erasable window)

**Specifications:**
- **Capacity:** 2048 bits (256 × 8)
- **Organization:** 256 words × 8 bits
- **Technology:** Floating-gate UV-erasable
- **Programming:** 25V pulses
- **Erase:** UV light exposure (15-20 minutes)
- **Access Time:** Typ. 1 μs

**Use Case:**
- Program development
- Prototyping
- Field-updateable systems

**Emulation Requirements:**
- [ ] 256 × 8-bit storage
- [ ] Erase simulation (optional)
- [ ] Programming simulation (optional)
- [ ] Standard ROM read interface

---

## 4. Support and Interface Chips

### 4.1 Clock Generation

| Chip | Type | Function | Status | Priority |
|------|------|----------|--------|----------|
| **4201/4201A** | Clock | Two-phase clock generator | ❌ Missing | **HIGH** |
| **Discrete** | Clock | External crystal + logic | ⚠️ Sim | MEDIUM |

#### 4.1.1 Intel 4201A Clock Generator

**Package:** 16-pin DIP

**Specifications:**
- **Input:** 4-7 MHz crystal
- **Output:** Two-phase non-overlapping clocks
- **Divide Ratio:** ÷7 or ÷8 (jumper selectable)
- **Output Frequency:** 500-740 kHz (CPU clock)
- **Technology:** CMOS
- **Outputs:**
  - φ1, φ2 (MOS levels: 0V / -10V)
  - TTL-compatible outputs
- **Typical Crystal:** 5.185 MHz → 740 kHz (÷7)

**Clock Phases:**
```
φ1: _____┐‾‾‾‾‾┐_____┐‾‾‾‾‾┐_____
φ2: ‾‾‾‾‾┐_____┐‾‾‾‾‾┐_____┐‾‾‾‾‾
        Non-overlapping phases
```

**Emulation Requirements:**
- [x] Simulated two-phase clock
- [ ] Programmable frequency (500-740 kHz)
- [ ] Phase timing accuracy
- [ ] Crystal frequency simulation
- [ ] TTL/MOS level outputs

### 4.2 Discrete Clock Alternative

**Components:**
- Crystal oscillator (5.185 MHz typical)
- 7490 or 74163 counter (÷7 divider)
- 9602 one-shots (phase generation)
- Transistor level shifters (TTL to MOS)

**Emulation:**
- Less historically accurate
- More educational (shows circuit design)
- Optionally implementable

---

## 5. Era-Appropriate Peripherals (1970-1980)

To create a complete system, we need to emulate the peripheral chips that would interface with the MCS-4/40 in real applications.

### 5.1 Intel 8000-Series Peripherals

| Chip | Type | Function | Era | Priority |
|------|------|----------|-----|----------|
| **8255** | PPI | Programmable Peripheral Interface (24-bit I/O) | 1975 | **HIGH** |
| **8251** | USART | Universal Sync/Async Serial (RS-232) | 1974 | **HIGH** |
| **8279** | KDC | Keyboard & Display Controller | 1976 | MEDIUM |
| **8253** | Timer | Programmable Interval Timer (3 channels) | 1974 | MEDIUM |
| **8257** | DMA | Direct Memory Access Controller | 1976 | LOW |
| **8259** | PIC | Programmable Interrupt Controller | 1976 | LOW |

#### 5.1.1 Intel 8255A - Programmable Peripheral Interface

**Package:** 40-pin DIP

**Specifications:**
- **I/O Ports:** 3 × 8-bit (Port A, B, C)
- **Total I/O:** 24 programmable lines
- **Modes:**
  - Mode 0: Basic I/O
  - Mode 1: Strobed I/O
  - Mode 2: Bidirectional bus
- **Handshaking:** Built-in
- **Speed:** Up to 2 MHz
- **Technology:** NMOS

**Configuration:**
```
Port A: 8-bit (input or output)
Port B: 8-bit (input or output)
Port C: 8-bit (can be split 4+4)
        Can provide control signals for A & B
```

**Use Cases with 4004/4040:**
- Parallel printer interface
- LED/relay control
- Sensor input
- General-purpose I/O expansion

**Emulation Requirements:**
- [ ] Three 8-bit ports (A, B, C)
- [ ] Mode control register
- [ ] Input/output direction control
- [ ] Handshaking logic
- [ ] Interrupt generation (mode 1/2)

#### 5.1.2 Intel 8251A - Universal Sync/Async Receiver/Transmitter

**Package:** 28-pin DIP

**Specifications:**
- **Data Rate:** Up to 19.2 kbps (async), 64 kbps (sync)
- **Data Bits:** 5, 6, 7, 8
- **Stop Bits:** 1, 1.5, 2
- **Parity:** Odd, even, none
- **Modem Control:** RTS, CTS, DTR, DSR
- **Status:** Framing error, parity error, overrun

**Modes:**
- **Asynchronous:** Standard UART (RS-232)
- **Synchronous:** Bisync, SDLC, etc.

**Use Cases:**
- Serial terminal connection
- Modem communication
- Computer-to-computer link
- Printer interface (serial)

**Emulation Requirements:**
- [ ] Serial-to-parallel conversion
- [ ] Baud rate generation
- [ ] Parity generation/checking
- [ ] Status register (error flags)
- [ ] Modem control signals
- [ ] Transmit/receive buffers

#### 5.1.3 Intel 8279 - Keyboard/Display Controller

**Package:** 40-pin DIP

**Specifications:**
- **Keyboard:** 8 × 8 matrix (64 keys)
- **Display:** 16 × 8-segment or 16 × 16-segment
- **Scan Rate:** Programmable
- **FIFO:** 8-character keyboard buffer
- **Modes:** Scanned keyboard, scanned sensor, strobed input

**Display Modes:**
- Left entry
- Right entry
- Auto-increment
- 8-segment or 16-segment

**Use Cases:**
- Calculator interface
- Terminal keyboard
- Front-panel display
- Instrument interface

**Emulation Requirements:**
- [ ] Keyboard matrix scanning
- [ ] 8-character FIFO buffer
- [ ] Display RAM (16 characters)
- [ ] Scan counter
- [ ] Debounce logic
- [ ] Display refresh

### 5.2 Display Technologies

| Type | Resolution | Interface | Era | Priority |
|------|-----------|-----------|-----|----------|
| **7-Segment LED** | 1-8 digits | Parallel | 1970s | **HIGH** |
| **16-Segment LED** | 1-8 alphanumeric | Parallel | 1970s | MEDIUM |
| **Character LCD** | 20×2, 40×2 | Parallel | Late 70s | MEDIUM |
| **Dot Matrix LED** | 5×7 per char | Multiplexed | 1970s | LOW |

#### 5.2.1 7-Segment LED Display

**Common Chips:**
- TIL311 (HP) - single digit with latch and decoder
- Common anode/cathode arrays

**Emulation:**
```
Segments: a,b,c,d,e,f,g,dp
         _a_
       f|   |b
        |_g_|
       e|   |c
        |___|  .dp
          d
```

**Requirements:**
- [ ] 7-segment decoder (BCD or hex)
- [ ] Visual representation
- [ ] Multiplexing support
- [ ] Brightness control (PWM simulation)

### 5.3 Input Devices

| Type | Interface | Typical Use | Priority |
|------|-----------|-------------|----------|
| **Matrix Keyboard** | Scanned (8×8) | Data entry | **HIGH** |
| **Hex Keypad** | 4×4 matrix | Calculator | **HIGH** |
| **Toggle Switches** | Parallel input | Front panel | MEDIUM |
| **Rotary Encoder** | Quadrature | Selection | LOW |

### 5.4 Storage Interfaces

| Type | Technology | Capacity | Era | Priority |
|------|-----------|----------|-----|----------|
| **Paper Tape** | Punched holes | ~100 bytes/foot | 1970s | MEDIUM |
| **Cassette Tape** | Audio FSK | ~1500 baud | 1975+ | MEDIUM |
| **Floppy Disk** | Magnetic | 80-250 KB | 1975+ | LOW |

#### 5.4.1 Cassette Tape Interface (Kansas City Standard)

**Specifications:**
- **Standard:** Kansas City (1200 baud) or faster
- **Encoding:** FSK (Frequency Shift Keying)
- **Mark (1):** 2400 Hz
- **Space (0):** 1200 Hz
- **Medium:** Standard audio cassette
- **Interface:** Via 8251 USART or bit-banging

**Emulation Requirements:**
- [ ] FSK modulation/demodulation
- [ ] Audio tone generation
- [ ] File-to-tape conversion
- [ ] Tape position tracking
- [ ] Simple file format

#### 5.4.2 Paper Tape

**Standards:**
- 5-bit (Baudot)
- 7-bit (ASCII)
- 8-bit (binary)

**Hardware:**
- Photo-electric reader
- Punch mechanism
- Sprocket holes for timing

**Emulation:**
- [ ] Character encoding
- [ ] File-based storage
- [ ] Virtual punch/reader

---

## 6. Electrical Characteristics

### 6.1 Power Supply Requirements

| Component | Vdd | Vss | Vcc | Notes |
|-----------|-----|-----|-----|-------|
| 4004, 4001-4003 | +5V | -15V | GND | P-MOS technology |
| 4040, 4289 | +5V | -12V | GND | Reduced voltage |
| 4101, 4201 | +5V | -12V | GND | Later CMOS/NMOS |
| 8255, 8251 | +5V | GND | — | TTL/NMOS compatible |

**Simulation Notes:**
- Emulator doesn't need actual voltages
- Important for understanding real hardware
- Affects pin assignment and interfacing

### 6.2 Timing Specifications

| Parameter | 4004 | 4040 | Units |
|-----------|------|------|-------|
| **Clock Frequency** | 500-740 | 500-740 | kHz |
| **Clock Period** | 1.35-2.0 | 1.35-2.0 | μs |
| **Instruction Time (1-byte)** | 10.8 | 10.8 | μs |
| **Instruction Time (2-byte)** | 21.6 | 21.6 | μs |
| **Instructions/Second** | 46-92K | 46-92K | IPS |

**Cycle Timing:**
- 8 clock periods = 1 instruction cycle (1-byte)
- 16 clock periods = 1 instruction cycle (2-byte)
- SYNC signal marks last cycle

### 6.3 Bus Timing

**4-Bit Data Bus:**
- Setup time: typ. 200 ns
- Hold time: typ. 150 ns
- Propagation delay: typ. 500 ns

**Control Signals:**
- SYNC: Low during last cycle
- TEST: Sampled during M2, X2
- Φ1, Φ2: Non-overlapping clocks

---

## 7. Implementation Priority

### Phase 1: Core MCS-4 (Current - Complete)
- [x] 4004 CPU
- [x] 4040 CPU
- [x] Basic 4001 ROM emulation
- [x] Basic 4002 RAM emulation

### Phase 2: Complete MCS-4/40 Chips (Next - High Priority)
- [ ] **4001 ROM** - Complete implementation with I/O
- [ ] **4002 RAM** - Complete with output ports
- [ ] **4003 Shift Register** - Full 10-bit implementation
- [ ] **4201A Clock Generator** - Programmable clock
- [ ] **4101 Static RAM** - Enhanced RAM option

### Phase 3: Memory Expansion (Medium Priority)
- [ ] **4008/4009** - Standard memory interface pair
- [ ] **4289** - Unified memory interface
- [ ] **4308 ROM** - Larger program storage
- [ ] **4702 EPROM** - Erasable program memory

### Phase 4: Essential Peripherals (Medium-High Priority)
- [ ] **8255 PPI** - Parallel I/O (critical for expansion)
- [ ] **8251 USART** - Serial communication (for terminal)
- [ ] **7-Segment Display** - Visual output
- [ ] **Matrix Keyboard** - User input

### Phase 5: Advanced Peripherals (Medium Priority)
- [ ] **8279** - Keyboard/display controller
- [ ] **8253** - Timer/counter
- [ ] **Cassette Interface** - Program storage
- [ ] **Character LCD** - Text display

### Phase 6: Optional Components (Low Priority)
- [ ] **4316 ROM** - Larger ROM
- [ ] **8257 DMA** - Direct memory access
- [ ] **8259 PIC** - Interrupt controller
- [ ] **Paper Tape** - Historical storage
- [ ] **Floppy Interface** - Disk storage

---

## 8. Detailed Specifications

### 8.1 Pin Assignments

#### 4004 CPU (16-pin DIP)
```
        ┌─────────┐
  SYNC ─┤1      16├─ Vss (-15V)
  TEST ─┤2      15├─ φ1 (clock phase 1)
    A0 ─┤3      14├─ φ2 (clock phase 2)
    A1 ─┤4      13├─ CM-RAM3
    A2 ─┤5      12├─ CM-RAM2
    A3 ─┤6      11├─ CM-RAM1
  DATA ─┤7      10├─ CM-RAM0
   Vdd ─┤8       9├─ CM-ROM
  +5V   └─────────┘   GND
```

#### 4040 CPU (24-pin DIP)
```
        ┌───────────┐
  SYNC ─┤1        24├─ Vss (-12V)
  TEST ─┤2        23├─ φ1
   INT ─┤3        22├─ φ2
   STP ─┤4        21├─ CM-RAM3
    A0 ─┤5        20├─ CM-RAM2
    A1 ─┤6        19├─ CM-RAM1
    A2 ─┤7        18├─ CM-RAM0
    A3 ─┤8        17├─ CM-ROM0
  DATA ─┤9        16├─ CM-ROM1
   ACK ─┤10       15├─ (Reserved)
   Vdd ─┤11       14├─ (Reserved)
  +5V   ├12       13├─ GND
        └───────────┘
```

#### 4001 ROM (16-pin DIP)
```
        ┌─────────┐
   I/O ─┤1      16├─ Vss
  DATA ─┤2      15├─ φ1
   CM- ─┤3      14├─ φ2
   Vdd ─┤4      13├─ D0
   D7  ─┤5      12├─ D1
   D6  ─┤6      11├─ D2
   D5  ─┤7      10├─ D3
   GND ─┤8       9├─ D4
        └─────────┘
```

### 8.2 Instruction Timing

**1-Byte Instructions (8 clock cycles):**
```
Cycle: A1  A2  A3  M1  M2  X1  X2  X3
φ1:    ‾┐_┐‾┐_┐‾┐_┐‾┐_┐‾┐_┐‾┐_┐‾┐_┐‾┐_
φ2:    _┐‾┐_┐‾┐_┐‾┐_┐‾┐_┐‾┐_┐‾┐_┐‾┐_┐‾

A1-A2: Address send
A3:    Opcode fetch
M1-M2: Opcode decode
X1-X3: Execute
```

**2-Byte Instructions (16 clock cycles):**
- First 8 cycles: Fetch first byte
- Second 8 cycles: Fetch second byte + execute

---

## 9. Integration Architecture

### 9.1 Component Hierarchy

```
┌─────────────────────────────────────────────────────────┐
│                     System Bus                          │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐         │
│  │   4004   │    │   4040   │    │  4201A   │         │
│  │   CPU    │    │   CPU    │    │  Clock   │         │
│  └────┬─────┘    └────┬─────┘    └────┬─────┘         │
│       │               │               │                 │
│       └───────────────┴───────────────┘                 │
│                       │                                 │
│       ┌───────────────┴───────────────┐                │
│       │                               │                 │
│  ┌────▼─────┐                   ┌────▼─────┐          │
│  │  Memory  │                   │   I/O    │          │
│  │  System  │                   │  System  │          │
│  └────┬─────┘                   └────┬─────┘          │
│       │                               │                 │
│  ┌────┴──────────┐            ┌──────┴──────────┐     │
│  │               │            │                  │     │
│ ┌▼───┐  ┌───┐  ┌▼───┐       ┌▼────┐  ┌────┐  ┌▼───┐ │
│ │4001│  │4002│  │4101│       │4003 │  │8255│  │8251│ │
│ │ROM │  │RAM │  │SRAM│       │Shift│  │PPI │  │UART│ │
│ └────┘  └────┘  └────┘       └─────┘  └────┘  └────┘ │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │           Memory Management / Banking            │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

### 9.2 Address Space Mapping

**4004/4040 Address Spaces:**
```
Program Memory (ROM):
  4004: 0x000-0xFFF (4KB, 12-bit address)
  4040: 0x0000-0x1FFF (8KB, 13-bit with banking)

Data Memory (RAM):
  4002: 640 nibbles (16 chips × 40 nibbles)
  Organized in banks and registers

I/O Space:
  4001: 16 × 4-bit ports (one per chip)
  4002: 16 × 4-bit output ports
  4003: Unlimited expansion
```

### 9.3 System Configurations

#### Minimal System
```
- 1 × 4004 CPU
- 1 × 4001 ROM (256 bytes)
- 1 × 4201A Clock
- External power supply
- Crystal (5.185 MHz)
```

#### Typical Development System
```
- 1 × 4004 or 4040 CPU
- 4 × 4001 ROM (1KB program)
- 4 × 4002 RAM (160 nibbles)
- 1 × 4003 Shift Register
- 1 × 4201A Clock
- 1 × 8251 USART (terminal)
- 1 × 8255 PPI (parallel I/O)
- 7-segment display
- Hex keypad
```

#### Maximal MCS-4 System
```
- 1 × 4004 CPU
- 16 × 4001 ROM (4KB program)
- 16 × 4002 RAM (640 nibbles)
- 3 × 4003 Shift Registers
- 1 × 4201A Clock
- Multiple peripherals via I/O expansion
```

#### Enhanced MCS-40 System
```
- 1 × 4040 CPU
- 8 × 4308 ROM (8KB program via banking)
- 16 × 4101 SRAM (4KB data)
- 1 × 4289 Memory Interface
- 1 × 4201A Clock
- 1 × 8255 PPI
- 1 × 8251 USART
- 1 × 8279 Keyboard/Display Controller
- Cassette tape interface
```

---

## 10. Implementation Guidelines

### 10.1 Module Structure

Each hardware component should be implemented as:

```cpp
class Chip4001 {
public:
    static constexpr size_t ROM_SIZE = 256;
    static constexpr size_t IO_BITS = 4;

    Chip4001();
    void reset();

    // ROM interface
    uint8_t readROM(uint8_t address);
    void loadROM(const uint8_t* data, size_t length);

    // I/O interface
    void writeIO(uint8_t value);
    uint8_t readIO();
    void setIODirection(uint8_t direction); // 4-bit mask

    // Control
    void chipSelect(bool selected);

private:
    uint8_t m_rom[ROM_SIZE];
    uint8_t m_ioPort;
    uint8_t m_ioDirection;
    bool m_selected;
};
```

### 10.2 Bus Architecture

Implement a system bus for component communication:

```cpp
class SystemBus {
public:
    void writeData(uint8_t data);
    uint8_t readData();

    void setAddress(uint16_t address);
    void setControl(ControlSignals signals);

    void registerChip(HardwareChip* chip);
    void clock(ClockPhase phase);

private:
    std::vector<HardwareChip*> m_chips;
    uint16_t m_address;
    uint8_t m_data;
    ControlSignals m_control;
};
```

### 10.3 Testing Strategy

Each chip implementation requires:

1. **Unit Tests:**
   - Individual function testing
   - Boundary condition validation
   - Timing verification

2. **Integration Tests:**
   - Multi-chip interaction
   - Bus protocol compliance
   - Real-world scenarios

3. **Validation Tests:**
   - Historical program execution
   - Known hardware behavior
   - Edge cases from documentation

---

## 11. Historical Accuracy Notes

### 11.1 Manufacturing Variations

- Early 4004s had variations in timing
- Different fab runs had different characteristics
- Documentation vs. actual behavior sometimes differed

**Emulation Approach:**
- Use documented specifications as baseline
- Add configuration options for variations
- Note discrepancies in documentation

### 11.2 Real-World Issues

Historical systems had to deal with:
- Noise and signal integrity
- Temperature effects on timing
- Component tolerance variations
- Power supply fluctuations

**Emulation:**
- Perfect digital simulation
- Optional: Add noise/error injection for realism
- Primarily educational, not cycle-exact

---

## 12. Future Expansion Possibilities

### 12.1 Hypothetical Enhanced Components

"What if" scenarios for educational purposes:

- **4004-E:** Enhanced 4004 with larger stack
- **4256:** Hypothetical 256-byte static RAM
- **4512:** Hypothetical 512-byte ROM
- **DMA Controller:** 4-bit DMA for 4004

### 12.2 Modern Interfacing

Bridge to modern hardware:

- USB-to-4004 bridge
- Ethernet interface
- SD card storage
- VGA display output
- Modern keyboard/mouse

---

## Conclusion

This roadmap provides a comprehensive guide to implementing a complete, historically accurate Intel MCS-4 and MCS-40 microcomputer system emulation. By following this granular specification, we can create an educational platform that demonstrates the full ecosystem of early microcomputer technology, from the bare CPU through memory, I/O, peripherals, and even storage systems.

**Implementation should proceed systematically:**
1. Complete core chips (4001, 4002, 4003, 4201)
2. Add memory expansion options (4101, 4289, 4308)
3. Implement essential peripherals (8255, 8251, display, keyboard)
4. Add storage interfaces (cassette, paper tape)
5. Explore hypothetical enhancements

This creates a powerful platform for education, preservation of computing history, and exploration of "what if" scenarios in early microcomputing.

---

**AD ASTRA PER MATHEMATICA ET SCIENTIAM**

*Preserving and expanding upon the dawn of the microcomputer revolution.*

**Document Version:** 1.0
**Last Updated:** 2025-11-19
**Status:** Comprehensive Research Complete
