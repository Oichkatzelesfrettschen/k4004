# Intel 4004/4040 Emulator - System Architecture

**Version:** 2.0 (Expanded)
**Date:** 2025-11-19

---

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [CPU Core Architecture](#cpu-core-architecture)
4. [Memory Architecture](#memory-architecture)
5. [I/O Architecture](#io-architecture)
6. [Instruction Pipeline](#instruction-pipeline)
7. [Assembler Architecture](#assembler-architecture)
8. [Module Organization](#module-organization)
9. [Extension Architecture](#extension-architecture)

---

## 1. Overview

This emulator implements both the Intel 4004 (MCS-4) and Intel 4040 (MCS-40) microprocessor systems with a focus on:

1. **Documentation accuracy** - Faithful to Intel specifications
2. **Functional completeness** - All instructions and hardware features
3. **Testability** - Comprehensive test infrastructure
4. **Extensibility** - Support for enhanced configurations and peripherals

### Design Philosophy

The emulator supports **two operational modes**:

- **Accurate Mode:** Strict adherence to Intel specifications
- **Enhanced Mode:** "Cranked to eleven" with expanded memory and capabilities

---

## 2. System Architecture

### 2.1 High-Level System Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     MCS-4/MCS-40 System                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐      ┌──────────────┐                     │
│  │   CPU Core   │◄────►│     ROM      │                     │
│  │  (4004/4040) │      │  (4001×16)   │                     │
│  └──────┬───────┘      └──────────────┘                     │
│         │                                                     │
│         │              ┌──────────────┐                     │
│         ├─────────────►│     RAM      │                     │
│         │              │  (4002×16)   │                     │
│         │              └──────────────┘                     │
│         │                                                     │
│         │              ┌──────────────┐                     │
│         └─────────────►│     I/O      │                     │
│                        │  (4003, etc) │                     │
│                        └──────────────┘                     │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Component Interaction

```
   Application Layer (emulator_apps)
           │
           ▼
   ┌──────────────────┐
   │  Emulator Core   │  ← High-level emulation management
   └────────┬─────────┘
            │
     ┌──────┴──────┬──────────┬──────────┐
     ▼             ▼          ▼          ▼
  ┌─────┐      ┌─────┐    ┌─────┐    ┌─────┐
  │4004 │      │4040 │    │ ROM │    │ RAM │
  │ CPU │      │ CPU │    │Chip │    │Chip │
  └─────┘      └─────┘    └─────┘    └─────┘
     │            │           │          │
     └────────────┴───────────┴──────────┘
                  │
            ┌─────┴──────┐
            ▼            ▼
       Instructions   Assembly
       (execution)    (shared)
```

---

## 3. CPU Core Architecture

### 3.1 Intel 4004 Architecture

```
┌────────────────────────────────────────────────────────┐
│                    Intel 4004 CPU                       │
├────────────────────────────────────────────────────────┤
│                                                          │
│  Registers:                                             │
│  ┌──────────────────────────────────────────────────┐  │
│  │ R0R1 │ R2R3 │ R4R5 │ R6R7 │ R8R9 │ RARB │ RCRD │  │
│  │ RERF │  (16 × 4-bit registers, 8 pairs)         │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
│  Accumulator: [CY][A3][A2][A1][A0]  (4-bit + carry)    │
│                                                          │
│  Stack (3 levels):                                      │
│  ┌──────────┬──────────┬──────────┐                    │
│  │  Level 0 │  Level 1 │  Level 2 │  (12-bit each)     │
│  └──────────┴──────────┴──────────┘                    │
│  SP: Stack Pointer (0-2)                                │
│                                                          │
│  Program Counter: 12-bit (0x000 - 0xFFF)               │
│  Instruction Register: 8-bit                            │
│  Test Pin: 1-bit input                                  │
│                                                          │
│  Instruction Set: 46 instructions                       │
│  Timing: 10.8 μs (1-byte) or 21.6 μs (2-byte)         │
│                                                          │
└────────────────────────────────────────────────────────┘
```

### 3.2 Intel 4040 Architecture

```
┌────────────────────────────────────────────────────────┐
│                    Intel 4040 CPU                       │
├────────────────────────────────────────────────────────┤
│                                                          │
│  Register Banks (2 banks, selectable via SB0/SB1):     │
│  Bank 0:                                                │
│  ┌──────────────────────────────────────────────────┐  │
│  │ R0R1 │ R2R3 │ R4R5 │ R6R7 │ R8R9 │ RARB │ RCRD │  │
│  │ RERF │ R10R11 │ R12R13 │ R14R15 │ R16R17 │       │  │
│  │ R18R19 │ R20R21 │ R22R23 │ (24 × 4-bit)         │  │
│  └──────────────────────────────────────────────────┘  │
│  Bank 1: [Same layout]                                  │
│                                                          │
│  Accumulator: [CY][A3][A2][A1][A0]  (4-bit + carry)    │
│  Command Register: 8-bit (for LCR instruction)          │
│  SRC Backup: 8-bit (for BBS instruction)                │
│                                                          │
│  Stack (7 levels):                                      │
│  ┌──────────┬──────────┬──────────┬───────────┬──...   │
│  │  Level 0 │  Level 1 │  Level 2 │  Level 3  │  ...   │
│  └──────────┴──────────┴──────────┴───────────┴──...   │
│  SP: Stack Pointer (0-6)                                │
│                                                          │
│  Program Counter: 13-bit (0x0000 - 0x1FFF) dual ROM    │
│  ROM Bank Select: 1-bit (DB0/DB1)                       │
│  Instruction Register: 8-bit                            │
│  Test Pin: 1-bit input                                  │
│                                                          │
│  Interrupt System:                                      │
│  - Interrupt Enable Flag (EIN/DIN)                      │
│  - INT Pin (interrupt request input)                    │
│  - Interrupt pending flag                               │
│                                                          │
│  Halt System:                                           │
│  - Halt Flag (HLT instruction)                          │
│  - STP Pin (single-step/stop)                           │
│                                                          │
│  Instruction Set: 60 instructions (46 + 14 new)         │
│  Timing: 10.8 μs (1-byte) or 21.6 μs (2-byte)         │
│                                                          │
└────────────────────────────────────────────────────────┘
```

### 3.3 Instruction Categories

#### 4004 Instructions (46 total)

| Category | Count | Instructions |
|----------|-------|--------------|
| **Arithmetic** | 8 | ADD, SUB, INC, DAC, IAC, DAA |
| **Logical** | 7 | CMA, RAL, RAR, CLC, CMC, STC, TCC, TCS |
| **Transfer** | 4 | LD, XCH, LDM, BBL |
| **Branch** | 5 | JCN, JUN, JMS, JIN, ISZ |
| **Memory** | 17 | WRM, RDM, WR0-3, RD0-3, WMP, SBM, ADM, etc. |
| **Special** | 5 | NOP, KBP, DCL, SRC, FIM, FIN |

#### 4040 Additional Instructions (14 new)

| Category | Count | Instructions |
|----------|-------|--------------|
| **Logical** | 4 | OR4, OR5, AN6, AN7 |
| **Bank Select** | 4 | DB0, DB1, SB0, SB1 |
| **Interrupt** | 3 | EIN, DIN, BBS |
| **Special** | 3 | HLT, LCR, RPM |

---

## 4. Memory Architecture

### 4.1 ROM Architecture (4001 Chips)

```
MCS-4 Configuration (4004):
┌─────────────────────────────────────────────┐
│  ROM Address Space: 12-bit (4096 bytes)     │
├─────────────────────────────────────────────┤
│                                               │
│  Bank 0 (only bank):                         │
│  ┌───────────────────────────────────────┐  │
│  │ Chip 0:  0x000-0x0FF (256 bytes)      │  │
│  │ Chip 1:  0x100-0x1FF                  │  │
│  │ Chip 2:  0x200-0x2FF                  │  │
│  │ ...                                    │  │
│  │ Chip 15: 0xF00-0xFFF                  │  │
│  └───────────────────────────────────────┘  │
│                                               │
│  Each 4001 chip:                             │
│  - 256 bytes ROM                             │
│  - 4-bit I/O port                            │
│                                               │
└─────────────────────────────────────────────┘

MCS-40 Configuration (4040):
┌─────────────────────────────────────────────┐
│  ROM Address Space: 13-bit (8192 bytes)     │
├─────────────────────────────────────────────┤
│                                               │
│  Bank 0 (selected by DB0):                   │
│  ┌───────────────────────────────────────┐  │
│  │ 0x0000-0x0FFF (4096 bytes)            │  │
│  └───────────────────────────────────────┘  │
│                                               │
│  Bank 1 (selected by DB1):                   │
│  ┌───────────────────────────────────────┐  │
│  │ 0x1000-0x1FFF (4096 bytes)            │  │
│  └───────────────────────────────────────┘  │
│                                               │
└─────────────────────────────────────────────┘
```

### 4.2 RAM Architecture (4002 Chips)

```
RAM Organization:
┌─────────────────────────────────────────────────────┐
│  4 Banks × 4 Chips × 4 Registers × 16 Characters   │
│  = 1024 data nibbles                                │
├─────────────────────────────────────────────────────┤
│                                                       │
│  Each 4002 Chip:                                     │
│  ┌─────────────────────────────────────────────┐   │
│  │ Data RAM: 4 registers × 16 nibbles = 64    │   │
│  │           (32 main characters + 32 status)  │   │
│  │                                              │   │
│  │ Main Characters:  [0-15] × 4 registers      │   │
│  │ Status Characters: [0-3] × 4 registers      │   │
│  │                                              │   │
│  │ Output Port: 4-bit                          │   │
│  └─────────────────────────────────────────────┘   │
│                                                       │
│  Bank Selection: DCL instruction (3 bits)           │
│  ┌────────┬────────┬────────┬────────┐              │
│  │ Bank 0 │ Bank 1 │ Bank 2 │ Bank 3 │              │
│  └────────┴────────┴────────┴────────┘              │
│                                                       │
│  Addressing via SRC instruction:                     │
│  [CM3][CM2][CM1][CM0][MA3][MA2][MA1][MA0]          │
│   └─── Chip ────┘ └──── Register ─────┘            │
│                                                       │
└─────────────────────────────────────────────────────┘
```

### 4.3 I/O Architecture

```
I/O Port Organization:
┌──────────────────────────────────────────┐
│  ROM I/O Ports: 16 × 4-bit (from 4001)  │
│  ┌────────────────────────────────────┐ │
│  │ Port 0-15: Connected to ROM chips  │ │
│  │ Direction: Bidirectional            │ │
│  │ Control: WRR, RDR instructions     │ │
│  └────────────────────────────────────┘ │
│                                           │
│  RAM Output Ports: 16 × 4-bit (4002)    │
│  ┌────────────────────────────────────┐ │
│  │ Port 0-15: Connected to RAM chips  │ │
│  │ Direction: Output only              │ │
│  │ Control: WMP instruction            │ │
│  └────────────────────────────────────┘ │
│                                           │
│  4003 Shift Register (if present):      │
│  ┌────────────────────────────────────┐ │
│  │ 10-bit parallel output              │ │
│  │ Used for: keyboard scan, display   │ │
│  └────────────────────────────────────┘ │
└──────────────────────────────────────────┘
```

---

## 5. Instruction Pipeline

### 5.1 Instruction Fetch-Execute Cycle

```
┌─────────────────────────────────────────────────────┐
│             Instruction Execution Flow               │
├─────────────────────────────────────────────────────┤
│                                                       │
│  1. FETCH (A1-A2):                                   │
│     ┌──────────────────────────────────────┐        │
│     │ IR ← ROM[PC]                         │        │
│     │ PC ← PC + 1                          │        │
│     └──────────────────────────────────────┘        │
│                    │                                 │
│                    ▼                                 │
│  2. DECODE (M1):                                     │
│     ┌──────────────────────────────────────┐        │
│     │ Opcode ← IR[7:4]                     │        │
│     │ Operand ← IR[3:0]                    │        │
│     └──────────────────────────────────────┘        │
│                    │                                 │
│                    ▼                                 │
│  3. EXECUTE (M2, X1-X2):                             │
│     ┌──────────────────────────────────────┐        │
│     │ Perform operation                     │        │
│     │ Update registers                      │        │
│     │ If 2-byte: fetch second byte         │        │
│     └──────────────────────────────────────┘        │
│                    │                                 │
│                    ▼                                 │
│  4. WRITEBACK (X3):                                  │
│     ┌──────────────────────────────────────┐        │
│     │ Update memory/flags                   │        │
│     │ Prepare for next instruction         │        │
│     └──────────────────────────────────────┘        │
│                                                       │
└─────────────────────────────────────────────────────┘

Timing: 8 clock cycles per machine cycle
        1 or 2 machine cycles per instruction
```

### 5.2 Instruction Timing

| Instruction Type | Bytes | Cycles | Time @ 740kHz |
|------------------|-------|--------|---------------|
| 1-byte | 1 | 8 | 10.8 μs |
| 2-byte | 2 | 16 | 21.6 μs |
| FIN (special) | 1 | 16 | 21.6 μs |

---

## 6. Assembler Architecture

### 6.1 Assembler Pipeline

```
┌────────────────────────────────────────────────────┐
│              Assembler Processing Flow              │
├────────────────────────────────────────────────────┤
│                                                      │
│  Source Code (.asm)                                 │
│       │                                              │
│       ▼                                              │
│  ┌──────────┐                                       │
│  │Tokenizer │  Lexical Analysis                     │
│  └────┬─────┘                                       │
│       │ Token Stream                                 │
│       ▼                                              │
│  ┌──────────┐                                       │
│  │  Parser  │  Syntax Analysis                      │
│  └────┬─────┘                                       │
│       │ AST / IR                                     │
│       ▼                                              │
│  ┌──────────┐                                       │
│  │Assembler │  Code Generation                      │
│  └────┬─────┘                                       │
│       │ Object Code                                  │
│       ▼                                              │
│  Binary Output (.bin)                               │
│                                                      │
└────────────────────────────────────────────────────┘
```

### 6.2 Syntax Grammar

```
program = line* EOF

line = '*' '=' NUMBER '\n'              # Set address
     | LABEL ('=' NUMBER)? '\n'         # Define label
     | LABEL instruction '\n'           # Labeled instruction
     | pragma '\n'                      # Assembler directive
     | instruction '\n'                 # Plain instruction

pragma = ".BYTE" NUMBER                 # Embed raw byte

instruction = SIMPLE_MNEMONIC                    # NOP, WRM, etc.
            | COMPLEX_MNEMONIC argument          # ADD R0, LD R5
            | TWO_BYTE_MNEMONIC argument (, argument)?  # JUN LABEL

argument = LABEL | NUMBER | REGISTER | REGISTER_PAIR
```

---

## 7. Module Organization

### 7.1 Directory Structure

```
k4004/
├── assembler/              # Assembler and disassembler
│   ├── source/
│   │   ├── tokenizer.cpp/hpp
│   │   ├── assembler.cpp/hpp
│   │   ├── conversions.cpp/hpp
│   │   └── main.cpp
│   └── tests/
│       ├── tokenizer_tests.cpp
│       ├── assembler_tests.cpp
│       └── disassembler_tests.cpp
│
├── emulator_core/          # CPU emulation core
│   ├── source/
│   │   ├── K4004.cpp/hpp          # 4004 CPU
│   │   ├── K4040.cpp/hpp          # 4040 CPU
│   │   ├── instructions.cpp/hpp    # Instruction implementations
│   │   ├── rom.cpp/hpp             # ROM chip emulation
│   │   ├── ram.cpp/hpp             # RAM chip emulation
│   │   └── emulator.cpp/hpp        # High-level emulator
│   └── tests/
│       └── instructions_tests.cpp
│
├── emulator_apps/          # Application layer
│   └── main.cpp            # CLI emulator application
│
├── shared/                 # Shared components
│   └── source/
│       └── assembly.cpp/hpp        # Shared assembly definitions
│
├── programs/               # Sample programs
│   └── busicom/            # Busicom calculator ROM
│
├── docs/                   # Documentation
│   ├── MCS-4_UsersManual_Feb73.pdf
│   ├── MCS-40_UsersManual_Nov74.pdf
│   ├── TECHNICAL_AUDIT.md
│   └── ARCHITECTURE.md (this file)
│
├── third_party/            # External dependencies
│   └── gtest/              # Google Test framework
│
├── scripts/                # Build and utility scripts (planned)
├── logs/                   # Build logs (planned)
└── CMakeLists.txt          # Build configuration
```

### 7.2 Module Dependencies

```
                    emulator_apps
                         │
                         ▼
                   emulator_core
                    ┌────┴────┐
                    ▼         ▼
                  K4004     K4040
                    └────┬────┘
                         ▼
              ┌──────────┴──────────┐
              ▼          ▼          ▼
            ROM        RAM    instructions
              └──────────┬──────────┘
                         ▼
                      assembly
                      (shared)
```

---

## 8. Extension Architecture

### 8.1 Enhanced Mode Features

The emulator supports an "Enhanced Mode" that extends beyond Intel specifications:

```
┌────────────────────────────────────────────────┐
│          Enhanced Configuration                 │
├────────────────────────────────────────────────┤
│                                                  │
│  ROM: Expanded to 64KB (vs 4KB standard)       │
│  RAM: Expanded to 64KB (vs 1280 nibbles)       │
│  I/O: Additional virtual peripherals            │
│                                                  │
│  Virtual Hardware:                              │
│  - Serial/TTY interface                         │
│  - Keyboard controller                          │
│  - Display controller                           │
│  - Storage interface (tape/disk emulation)     │
│  - Real-time clock                              │
│                                                  │
└────────────────────────────────────────────────┘
```

### 8.2 Planned Software Ecosystem

```
┌─────────────────────────────────────────┐
│         Software Stack                   │
├─────────────────────────────────────────┤
│                                           │
│  User Applications                       │
│  ┌────────────────────────────────────┐ │
│  │ Calculator, Games, Utilities       │ │
│  └─────────────┬──────────────────────┘ │
│                ▼                         │
│  4-bit BASIC Interpreter                │
│  ┌────────────────────────────────────┐ │
│  │ - Variable storage                 │ │
│  │ - Expression evaluation            │ │
│  │ - Control flow                     │ │
│  │ - I/O routines                     │ │
│  └─────────────┬──────────────────────┘ │
│                ▼                         │
│  4-bit DOS/Monitor                      │
│  ┌────────────────────────────────────┐ │
│  │ - Command interpreter              │ │
│  │ - Memory management                │ │
│  │ - File system (simple)             │ │
│  │ - Device drivers                   │ │
│  └─────────────┬──────────────────────┘ │
│                ▼                         │
│  Hardware Abstraction Layer            │
│  ┌────────────────────────────────────┐ │
│  │ - ROM/RAM interface                │ │
│  │ - I/O routines                     │ │
│  │ - Interrupt handlers (4040)        │ │
│  └────────────────────────────────────┘ │
│                                           │
└─────────────────────────────────────────┘
```

### 8.3 Extensibility Points

The architecture provides several extension points:

1. **Custom Instructions** - Via instruction handler table
2. **Virtual Devices** - Via I/O port mapping
3. **Memory Extensions** - Via banking mechanisms
4. **Peripherals** - Via device abstraction layer

---

## 9. Implementation Details

### 9.1 Data Types and Sizes

```cpp
// 4-bit values stored in uint8_t with masking
using nibble_t = uint8_t;  // 0x0 - 0xF

// Register pair (8-bit)
using reg_pair_t = uint8_t;  // 0x00 - 0xFF

// Address types
using addr12_t = uint16_t;  // 12-bit address (4004)
using addr13_t = uint16_t;  // 13-bit address (4040)

// Instruction types
using opcode_t = uint8_t;   // 8-bit opcode
```

### 9.2 Register Encoding

Registers are stored as pairs in a single byte:
```
Register Pair Layout:
┌────────┬────────┐
│ R(n)   │ R(n+1) │
│ [7:4]  │ [3:0]  │
└────────┴────────┘

Example: R2R3 pair
Bit 7-4: R2 value
Bit 3-0: R3 value
```

### 9.3 Accumulator Encoding

```
Accumulator (5-bit):
┌────┬────┬────┬────┬────┐
│ CY │ A3 │ A2 │ A1 │ A0 │
│ [4]│ [3]│ [2]│ [1]│ [0]│
└────┴────┴────┴────┴────┘

Carry flag is stored in bit 4
Value is in bits [3:0]
```

---

## 10. Testing Architecture

### 10.1 Test Levels

```
Unit Tests
  ├── Instruction Tests (each instruction individually)
  ├── CPU Tests (state transitions)
  ├── Memory Tests (ROM/RAM operations)
  └── I/O Tests (port operations)

Integration Tests
  ├── Instruction Sequences
  ├── Subroutine Calls
  ├── Interrupt Handling (4040)
  └── Bank Switching (4040)

System Tests
  ├── Sample Programs (Busicom ROM)
  ├── BASIC Interpreter
  └── DOS/Monitor
```

### 10.2 Test Infrastructure

- **Framework:** Google Test (gtest)
- **Coverage Target:** >90% code coverage
- **Validation:** Against Intel specifications and known programs

---

## 11. Build System

### 11.1 CMake Configuration

```cmake
Options:
  BUILD_TESTS        ON/OFF  (default: ON)
  STATIC_ANALYSIS    ON/OFF  (default: OFF)
  BUILD_ENHANCED     ON/OFF  (default: OFF) - Enhanced mode features
  ENABLE_LOGGING     ON/OFF  (default: OFF) - Debug logging

Targets:
  assembler          - Assembler/disassembler tool
  mcs4_emulator      - Main emulator application
  emulator_tests     - Test suite
  assembler_tests    - Assembler test suite
```

---

## 12. Future Directions

### 12.1 Planned Enhancements

1. **Cycle-Accurate Timing** - Precise clock cycle emulation
2. **Debugger Interface** - Step, breakpoint, watch support
3. **GUI Interface** - Visual register/memory inspector
4. **FPGA Implementation** - Hardware acceleration
5. **Network Support** - Multi-system communication

### 12.2 Research Areas

1. **Optimizations** - Speed improvements while maintaining accuracy
2. **Historical Software** - Porting period software to emulator
3. **Educational Tools** - Teaching materials using emulator
4. **Preservation** - Archiving historical MCS-4/40 programs

---

**Document Version:** 2.0
**Last Updated:** 2025-11-19
**Maintained By:** K4004 Development Team

**AD ASTRA PER MATHEMATICA ET SCIENTIAM**
