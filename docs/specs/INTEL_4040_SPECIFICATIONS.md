# Intel 4040 Microprocessor - Complete Technical Specifications

## Document Information
- **Part Number**: Intel 4040
- **Description**: Enhanced 4-bit Central Processing Unit
- **Technology**: p-channel silicon-gate MOS (pMOS)
- **Introduction Date**: 1974
- **Production Period**: 1974-1981
- **Predecessor**: Intel 4004 (1971)

---

## 1. PACKAGE AND PINOUT

### Package Type
- **Package**: 24-pin Ceramic DIP (Dual In-line Package)
- **Die Size**: Larger than 4004 (exact dimensions not publicly specified)
- **Transistor Count**: 3,000 transistors (vs 2,250 in 4004)
- **Process**: 10 μm (10,000 nm) pMOS silicon-gate

### Pin Configuration (24-Pin DIP)

```
         Intel 4040
       ┌─────∪─────┐
 D3  1 │           │ 24  VDD
 D2  2 │           │ 23  VCC
 D1  3 │           │ 22  D0
VSS  4 │           │ 21  φ2(OUT)
φ2(IN)5│           │ 20  φ1(OUT)
φ1(IN)6│           │ 19  STP
SYNC 7 │  4040     │ 18  CM-ROM
CM-RAM08│          │ 17  CM-RAM3
CM-RAM19│          │ 16  CM-RAM2
CM-RAM210│         │ 15  INT
 TEST 11│          │ 14  EF2
  EF1 12│          │ 13  RESET
       └───────────┘
```

### Pin Descriptions

| Pin | Name      | Type   | Description |
|-----|-----------|--------|-------------|
| 1   | D3        | I/O    | Data bus bit 3 (MSB) |
| 2   | D2        | I/O    | Data bus bit 2 |
| 3   | D1        | I/O    | Data bus bit 1 |
| 4   | VSS       | Power  | Ground (0V) |
| 5   | φ2(IN)    | Input  | Clock phase 2 input |
| 6   | φ1(IN)    | Input  | Clock phase 1 input |
| 7   | SYNC      | Output | Synchronization pulse |
| 8   | CM-RAM0   | Output | Chip-select RAM bank 0 |
| 9   | CM-RAM1   | Output | Chip-select RAM bank 1 |
| 10  | CM-RAM2   | Output | Chip-select RAM bank 2 (NEW) |
| 11  | TEST      | Input  | Conditional test input |
| 12  | EF1       | Input  | External flag 1 (NEW) |
| 13  | RESET     | Input  | System reset |
| 14  | EF2       | Input  | External flag 2 (NEW) |
| 15  | INT       | Input  | Interrupt request (NEW) |
| 16  | CM-RAM2   | Output | Chip-select RAM bank 2 |
| 17  | CM-RAM3   | Output | Chip-select RAM bank 3 (NEW) |
| 18  | CM-ROM    | Output | Chip-select ROM |
| 19  | STP       | Output | Stop acknowledge (NEW) |
| 20  | φ1(OUT)   | Output | Clock phase 1 buffered output |
| 21  | φ2(OUT)   | Output | Clock phase 2 buffered output |
| 22  | D0        | I/O    | Data bus bit 0 (LSB) |
| 23  | VCC       | Power  | -10V supply (NEW) |
| 24  | VDD       | Power  | -15V supply |

### Key Differences from 4004

**Additional Pins**:
1. **INT (pin 15)**: Interrupt request input (enables interrupt-driven programming)
2. **STP (pin 19)**: Stop acknowledge output (indicates CPU halted state)
3. **EF1, EF2 (pins 12, 14)**: External flags for condition testing (more than TEST alone)
4. **VCC (pin 23)**: Additional power supply pin for dual-voltage operation

**Package Benefits**:
- 24 pins vs 16 pins (50% more I/O capacity)
- Separate interrupt and stop control
- Enhanced condition testing (3 inputs vs 1)
- Better power distribution (dual supply)

---

## 2. ELECTRICAL CHARACTERISTICS

### Absolute Maximum Ratings

| Parameter | Symbol | Min | Max | Unit |
|-----------|--------|-----|-----|------|
| VDD Supply Voltage | VDD | -0.5 | -17 | V |
| VCC Supply Voltage | VCC | -0.5 | -12 | V |
| Input Voltage | VIN | -0.5 | VDD-0.5 | V |
| Operating Temperature | TA | 0 | 70 | °C |
| Storage Temperature | TSTG | -55 | 125 | °C |

### DC Characteristics (TA = 0°C to 70°C, VDD = -15V ±5%, VCC = -10V ±5%)

| Parameter | Symbol | Min | Typ | Max | Unit | Conditions |
|-----------|--------|-----|-----|-----|------|------------|
| VDD Supply Voltage | VDD | -14.25 | -15 | -15.75 | V | ±5% |
| VCC Supply Voltage | VCC | -9.5 | -10 | -10.5 | V | ±5% |
| VDD Supply Current | IDD | - | 25 | 50 | mA | Operating |
| VCC Supply Current | ICC | - | 10 | 20 | mA | Operating |
| Input High Voltage | VIH | -10 | - | VSS | V | |
| Input Low Voltage | VIL | VDD | - | -3 | V | |
| Output High Voltage | VOH | -10 | -12 | VSS | V | IOH = -1mA |
| Output Low Voltage | VOL | VDD | -14 | -13 | V | IOL = 2mA |
| Input Leakage Current | IIL | - | - | ±10 | μA | |
| Power Dissipation | PD | - | 630 | 850 | mW | Operating |

### AC Characteristics (VDD = -15V, VCC = -10V, TA = 25°C, CL = 50pF)

| Parameter | Symbol | Min | Typ | Max | Unit |
|-----------|--------|-----|-----|-----|------|
| Clock Frequency | fCLK | 0.5 | 0.74 | 0.75 | MHz |
| Clock Period | tCY | 1.35 | 1.35 | 2.0 | μs |
| Clock Pulse Width | tPW | 300 | - | - | ns |
| Clock Non-overlap | tNO | 30 | - | - | ns |
| Data Setup Time | tDS | 150 | - | - | ns |
| Data Hold Time | tDH | 100 | - | - | ns |
| Output Delay | tOD | - | 200 | 400 | ns |
| Interrupt Setup Time | tINT | 500 | - | - | ns |
| SYNC Pulse Width | tSYNC | 1 | - | - | clock periods |

**Performance Improvement**:
- Power dissipation: 630 mW (typical) vs 1000 mW in 4004 (-37%)
- Lower power despite more transistors (dual voltage supplies)

---

## 3. FUNCTIONAL SPECIFICATIONS

### 3.1 Enhanced CPU Architecture

**Internal Registers** (Enhanced from 4004):
- **Accumulator**: 4-bit with carry/borrow flag (same as 4004)
- **Program Counter**: 13-bit (8192 address space) - **EXPANDED**
- **Stack**: 7 levels deep × 13 bits - **EXPANDED** from 3 levels
- **Index Registers**: 24 × 4-bit registers in 2 banks - **EXPANDED**
  - Bank 0: 12 registers (24 nibbles)
  - Bank 1: 12 registers (24 nibbles)
  - Total: 24 registers vs 16 in 4004
- **Command Register**: 4-bit (for LCR instruction) - **NEW**
- **SRC Backup**: 8-bit (for interrupt handling, BBS) - **NEW**
- **Instruction Register**: 8-bit (same as 4004)
- **Interrupt Enable**: 1-bit flag (EIN/DIN control) - **NEW**
- **Interrupt Pending**: 1-bit flag (latches INT pin) - **NEW**

**Memory Addressing** (Enhanced):
- **ROM Address Space**: 8192 bytes (13-bit addressing) - **DOUBLED**
  - Dual ROM banks (0 and 1)
  - DB0/DB1 instructions select bank
  - Each bank: 4096 bytes (16 × 256-byte chips)
  - Maximum: 32 × 4001 ROM chips = 8KB total
- **RAM Address Space**: Same as 4004 (1280 nibbles)
  - 4 banks × 4 chips × 4 registers × 20 characters
  - Supports same 4002 RAM chips

**I/O Addressing**: Same as 4004
- ROM ports: 16 × 4-bit per bank (32 total with dual banks)
- RAM ports: 16 × 4-bit output ports
- Expandable via 4003 shift registers

### 3.2 Instruction Set Architecture

**Total Instructions**: 60 (46 original + 14 new)

**Instruction Format**: Same as 4004
- Single-word: 8 bits (1 cycle = 10.8 μs @ 740 kHz)
- Double-word: 16 bits (2 cycles = 21.6 μs)

**New Instructions** (14 additions):

1. **HLT** (0x01) - Halt
   - Stop execution until interrupt or RESET
   - Sets STP output pin high
   - Resumes at interrupt vector or current PC+1 on reset
   - Power-saving mode

2. **BBS** (0x02) - Branch Back from Interrupt (Subroutine)
   - Restore SRC from backup
   - Pop return address from stack
   - Used to exit interrupt service routine
   - Automatically restores context

3. **LCR** (0x03) - Load Command Register to Accumulator
   - ACC ← CMD_REG
   - Read current command register value
   - Used for status checking

4. **OR4** (0x04) - OR Register 4 with Accumulator
   - ACC ← ACC | R[4]
   - Logical OR operation (NEW capability)
   - Expands logical operations beyond CMA

5. **OR5** (0x05) - OR Register 5 with Accumulator
   - ACC ← ACC | R[5]
   - Common register for OR operations

6. **AN6** (0x06) - AND Register 6 with Accumulator
   - ACC ← ACC & R[6]
   - Logical AND operation (NEW capability)
   - Bit masking support

7. **AN7** (0x07) - AND Register 7 with Accumulator
   - ACC ← ACC & R[7]
   - Common register for AND operations

8. **DB0** (0x08) - Designate Bank 0
   - Select ROM bank 0 (lower 4KB)
   - Affects ROM reads and JUN/JMS addressing
   - Default on reset

9. **DB1** (0x09) - Designate Bank 1
   - Select ROM bank 1 (upper 4KB)
   - Addresses 0x1000-0x1FFF
   - Enables 8KB program space

10. **SB0** (0x0A) - Select Register Bank 0
    - Switch to register bank 0 (default)
    - R0-R11 accessible
    - Used for context switching

11. **SB1** (0x0B) - Select Register Bank 1
    - Switch to register bank 1
    - R12-R23 accessible
    - Doubles register capacity

12. **EIN** (0x0C) - Enable Interrupts
    - Set interrupt enable flag
    - INT pin can now trigger interrupts
    - Critical section exit

13. **DIN** (0x0D) - Disable Interrupts
    - Clear interrupt enable flag
    - INT pin ignored
    - Critical section protection

14. **RPM** (0x0E) - Read Program Memory
    - ACC ← ROM[current_PC]
    - Read current instruction byte to accumulator
    - Table lookup / self-modifying code inspection

**All 4004 Instructions**: Fully compatible
- All 46 original instructions work identically
- Binary code compatible with 4004 (within 4KB space)
- Instruction opcodes unchanged

### 3.3 Interrupt System (NEW Feature)

**Interrupt Sources**:
- **INT Pin**: External interrupt request (active high)
- **Conditions**: Interrupt enabled (EIN), not already pending, not in HLT

**Interrupt Handling Sequence**:
1. INT pin sampled at end of each instruction
2. If enabled and high: Interrupt pending flag set
3. Current SRC register saved to backup
4. Stack: PC+1 pushed (7-level stack)
5. PC ← 0x003 or 0x1003 (depends on current ROM bank)
6. Interrupts automatically disabled (DIN implicit)
7. Execute interrupt service routine (ISR)
8. ISR ends with BBS instruction:
   - Restore SRC from backup
   - Pop return address
   - Return to interrupted code
9. Re-enable interrupts with EIN (in main code, not ISR)

**Interrupt Vector Address**:
- **If DB0 active**: Vector = 0x003 (bank 0)
- **If DB1 active**: Vector = 0x1003 (bank 1)
- ISR can switch banks if needed

**Interrupt Latency**:
- Maximum: Time of longest instruction (16 clocks = 21.6 μs)
- Typical: ~11 clocks average (14.8 μs)

**Stack Requirement**:
- ISR can nest up to 6 levels deep (7-level stack - 1 for interrupt)
- No re-entrant interrupts (disabled during ISR)

### 3.4 Halt Mode (NEW Feature)

**HLT Instruction** (0x01):
- Stops CPU execution
- STP pin goes high (signals external hardware)
- Clock continues running (for memory refresh)
- Ultra-low power state

**Wake-up Conditions**:
1. **INT pin asserted** (if interrupts enabled):
   - Jump to interrupt vector
   - STP goes low
   - Execute ISR

2. **RESET pin asserted**:
   - Full CPU reset
   - PC ← 0x000
   - STP goes low
   - Resume execution at reset vector

**Power Savings**:
- Estimated 50-70% power reduction in HLT state
- Only clock buffers and refresh circuits active
- Used in battery-powered designs

### 3.5 Register Banking (NEW Feature)

**Two Register Banks**:
- **Bank 0**: Registers R0-R11 (12 registers = 24 nibbles)
- **Bank 1**: Registers R12-R23 (12 registers = 24 nibbles)
- **Selection**: SB0/SB1 instructions
- **Default**: Bank 0 on reset

**Register Pair Organization**:
- Bank 0: Pairs 0-5 (R0R1, R2R3, R4R5, R6R7, R8R9, R10R11)
- Bank 1: Pairs 0-5 (R12R13, R14R15, R16R17, R18R19, R20R21, R22R23)
- FIM, SRC, INC, etc. operate on current bank

**Use Cases**:
- Context switching (multitasking)
- Separate register sets for ISR vs main code
- Parameter passing to subroutines
- Temporary storage expansion

**Important**: SRC is shared between banks (only one SRC register)

### 3.6 ROM Banking (NEW Feature)

**Dual ROM Banks**:
- **Bank 0**: Address 0x0000-0x0FFF (4KB)
- **Bank 1**: Address 0x1000-0x1FFF (4KB)
- **Selection**: DB0/DB1 instructions
- **Default**: Bank 0 on reset

**Address Mapping**:
```
DB0 active:
  PC 0x000-0xFFF → Physical ROM 0x0000-0x0FFF (bank 0)

DB1 active:
  PC 0x000-0xFFF → Physical ROM 0x1000-0x1FFF (bank 1)
```

**Cross-Bank Jumping**:
```assembly
; From bank 0 to bank 1:
DB1         ; Switch to bank 1
JUN 0x000   ; Jump to bank 1 address 0x000
            ; (actually physical 0x1000)

; From bank 1 back to bank 0:
DB0         ; Switch to bank 0
JUN 0x000   ; Jump to bank 0 address 0x000
```

**Bank Persistence**:
- Bank selection persists across instructions
- Subroutines can execute in different bank
- JMS/BBL work across banks
- Interrupt vector depends on current bank

### 3.7 Enhanced Condition Testing

**Three Conditional Inputs**:
1. **TEST** (pin 11): General-purpose test input (4004 compatible)
2. **EF1** (pin 12): External flag 1 (NEW)
3. **EF2** (pin 14): External flag 2 (NEW)

**JCN Instruction Enhancements**:
- Same opcode (0x1C), compatible with 4004
- Condition bits can test all three inputs:
  - Bit 0: Test TEST pin (same as 4004)
  - Additional logic can combine EF1/EF2 via external gates

**Use Cases**:
- TEST: Keyboard input
- EF1: Serial data ready
- EF2: Battery low warning
- Combined: Complex state machines

---

## 4. TIMING DIAGRAMS

### 4.1 Clock Timing (Same as 4004)

```
         _____       _____       _____       _____
φ1  ____|     |_____|     |_____|     |_____|     |_____
              _____       _____       _____       _____
φ2  _________|     |_____|     |_____|     |_____|     |

        |<-tCY->|
        |<tPW>|
             |<tNO>|

tCY   = 1.35 μs @ 740 kHz
tPW   ≥ 300 ns
tNO   ≥ 30 ns
```

### 4.2 Interrupt Timing (NEW)

```
         ___     ___     ___     ___     ___     ___     ___
φ1   ___|   |___|   |___|   |___|   |___|   |___|   |___|   |___
           ___     ___     ___     ___     ___     ___     ___
φ2   ____|   |___|   |___|   |___|   |___|   |___|   |___|   |____

INT  ____/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\_____
         |<------tINT setup------>|
                                  |
PC   <--Current Instruction------>|<--ISR starts at 0x003---->
                                  |
SYNC ____/‾‾\________________/‾‾\_|_/‾‾\______________________
     Normal instruction       End  | ISR instruction
                              Curr | fetch

Interrupt latency: Time from INT high to ISR execution
Maximum: 21.6 μs (if 16-bit instruction in progress)
```

### 4.3 Halt Mode Timing (NEW)

```
HLT instruction executed:

         ___     ___     ___     ___
φ1   ___|   |___|   |___|   |___|   |___ (continues)
           ___     ___     ___     ___
φ2   ____|   |___|   |___|   |___|   |____ (continues)

STP  _____________/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾ (stays high)

D0-3 <--HLT executed-->ZZZZZZZZZZZZZZZZZZZZZZZZ (high-Z)

PC   <--Frozen at PC+1---------------------------------->

Wake-up on interrupt:

INT  ________________/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\_____________

STP  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\_____________________________________

PC   <--Frozen-------->|<--Jump to ISR (0x003)--------->
```

### 4.4 Register Bank Switching (NEW)

```
SB0/SB1 instruction (single cycle, immediate effect):

         ___     ___     ___     ___     ___     ___
φ1   ___|   |___|   |___|   |___|   |___|   |___|   |___
           ___     ___     ___     ___     ___     ___
φ2   ____|   |___|   |___|   |___|   |___|   |___|   |____

D0-3 <--SB1 opcode-->|<--Next instruction using bank 1-->

Bank  [Bank 0]       |[Bank 1]<---------------------------->
                     ^
                     Bank switch takes effect immediately
                     next instruction uses bank 1 registers
```

### 4.5 ROM Bank Switching (NEW)

```
DB0/DB1 instruction:

         ___     ___     ___     ___     ___     ___
φ1   ___|   |___|   |___|   |___|   |___|   |___|   |___

D0-3 <--DB1-->|<--JUN-->|<--Fetch from bank 1 ROM---->

ROM_Bank [0]  |[1]<------------------------------------------->
              ^
              Bank switch effective for next memory access
```

---

## 5. SYSTEM INTERFACING

### 5.1 Maximum System Configuration (Enhanced)

**MCS-40 System (4040-based)**:
- 1 × 4040 CPU
- 32 × 4001 ROM (8192 bytes) - DOUBLED from 4004
  - Bank 0: 16 ROMs (4KB)
  - Bank 1: 16 ROMs (4KB)
- 16 × 4002 RAM (640 bytes / 1280 nibbles) - same as 4004
- Unlimited 4003 shift registers
- 1 × 4201A Clock Generator
- 4008/4009/4289 interface chips (for 8-bit peripherals)

### 5.2 Interrupt Interface Design

**Typical Interrupt Circuit**:
```
                    +5V
                     |
                    [ ] 10kΩ pull-up
                     |
External Device ---->|-----> INT (pin 15)
(active high)        |
                    GND (when inactive)

Software:
  EIN         ; Enable interrupts
  ...         ; Main code
  ; Interrupt occurs here
  ; => PC = 0x003 (ISR entry)

ISR:
  ; Save context to RAM if needed
  ; Handle interrupt
  BBS         ; Return from interrupt
```

**Multiple Interrupt Sources**:
```
Device1 ----+
            |
Device2 ----|--[OR gate]--> INT (pin 15)
            |
Device3 ----+

ISR must poll devices to determine source:
  RDR       ; Read ROM port 0 (device status)
  JCN ...   ; Jump based on device ID
```

### 5.3 Halt Mode Application

**Battery-Powered Design**:
```assembly
MAIN_LOOP:
    ; Check if work to do
    RDR         ; Read peripheral status
    JCN NZ, DO_WORK

    ; No work, sleep to save power
    HLT         ; Wait for interrupt

    ; (Wakes here on INT or RESET)
    JUN MAIN_LOOP

DO_WORK:
    ; Process data
    ...
    JUN MAIN_LOOP
```

**STP Pin Usage**:
```
4040 STP (pin 19) ---> External power control
                       (disable peripherals when CPU halted)
```

### 5.4 8KB ROM Organization

**Bank Configuration Example**:
```
Bank 0 (DB0):
  ROM 0-15 at 0x000-0xFFF
  - Operating system
  - Common subroutines
  - Interrupt vectors (0x000, 0x003)

Bank 1 (DB1):
  ROM 16-31 at 0x000-0xFFF (physical 0x1000-0x1FFF)
  - Application code
  - Data tables
  - Extended functions

Cross-bank call:
BANK0_ROUTINE:
    DB1           ; Switch to bank 1
    JMS BANK1_SUB ; Call subroutine in bank 1
    DB0           ; Switch back to bank 0
    ; (BBL returns here)
```

---

## 6. PERFORMANCE CHARACTERISTICS

### 6.1 Instruction Execution Times (Same as 4004)

| Instruction Type | Cycles | Time @ 740 kHz | Example |
|-----------------|--------|----------------|---------|
| Single-word (8-bit) | 8 | 10.8 μs | HLT, EIN, DIN, SB0, DB0 |
| Double-word (16-bit) | 16 | 21.6 μs | JCN, FIM, JUN |

### 6.2 New Instruction Performance

| Instruction | Cycles | Time @ 740 kHz | Notes |
|-------------|--------|----------------|-------|
| HLT | 8 | 10.8 μs | Then stops until INT/RESET |
| BBS | 8 | 10.8 μs | Interrupt return |
| EIN, DIN | 8 | 10.8 μs | Interrupt control |
| SB0, SB1 | 8 | 10.8 μs | Bank switch immediate |
| DB0, DB1 | 8 | 10.8 μs | ROM bank switch |
| OR4, OR5, AN6, AN7 | 8 | 10.8 μs | Logical operations |
| LCR | 8 | 10.8 μs | Read command register |
| RPM | 8 | 10.8 μs | Read program memory |

### 6.3 Interrupt Overhead

**Interrupt Service Time**:
```
Event:                          Time:
INT pin asserted                0 μs
Current instruction completes   0-21.6 μs (worst case)
Hardware saves SRC, pushes PC   ~1 cycle (internal)
Jump to 0x003                   ~1 cycle
ISR instruction fetch           10.8 μs
Total latency:                  12-35 μs

ISR execution:                  Variable (user code)

BBS return instruction:         10.8 μs
Return to main code:            0 μs
Total overhead:                 23-46 μs + ISR time
```

**Context Save/Restore** (if needed):
```assembly
; ISR with full context save:
ISR_ENTRY:          ; @ 0x003
    SRC P0          ; Select RAM location (10.8 μs)
    WRM             ; Save ACC (10.8 μs)
    LD R1           ; (10.8 μs)
    WRM             ; Save R1 (10.8 μs)
    ; ... handle interrupt ...
    SRC P0          ; (10.8 μs)
    RDM             ; Restore ACC (10.8 μs)
    BBS             ; Return (10.8 μs)
; Overhead: ~75 μs + handler code
```

### 6.4 System Throughput

**Instruction Throughput**:
- Same as 4004: ~92,500 instructions/second @ 740 kHz
- New instructions execute in same time as originals
- No performance penalty for enhanced features

**Effective Performance Gains**:
1. **7-level stack**: Deeper subroutine nesting (vs 3 on 4004)
   - More modular code possible
   - Reduced stack management overhead

2. **24 registers**: Less RAM access needed
   - Registers: 10.8 μs access
   - RAM: 10.8 μs + SRC setup = 21.6 μs
   - Savings: ~50% for frequent data access

3. **8KB ROM**: Larger programs without external banking hardware
   - DB0/DB1 instructions: 10.8 μs
   - vs external banking: 3-4 instructions + I/O

4. **Interrupts**: Event-driven programming
   - vs polling: Continuous CPU overhead
   - Savings: 50-90% CPU time in idle states

---

## 7. COMPARISON WITH 4004

### 7.1 Feature Comparison Table

| Feature | 4004 | 4040 | Improvement |
|---------|------|------|-------------|
| **Package** | 16-pin DIP | 24-pin DIP | +50% pins |
| **Instructions** | 46 | 60 | +14 instructions |
| **Stack Depth** | 3 levels | 7 levels | +133% |
| **ROM Space** | 4KB (12-bit) | 8KB (13-bit) | +100% |
| **RAM Space** | 640 bytes | 640 bytes | Same |
| **Registers** | 16 (8 pairs) | 24 (12 pairs, 2 banks) | +50% |
| **Interrupts** | None | Yes (INT pin) | NEW |
| **Halt Mode** | No | Yes (HLT, STP) | NEW |
| **Power** | 1.0 W | 0.63 W | -37% |
| **Test Inputs** | TEST (1) | TEST, EF1, EF2 (3) | +200% |
| **Logical Ops** | CMA only | CMA, OR, AND | NEW |
| **Transistors** | 2,250 | 3,000 | +33% |
| **Die Size** | 12 mm² | Larger | Unknown |

### 7.2 Instruction Set Comparison

**4004 Instructions**: All 46 retained, binary compatible
**New 4040 Instructions**: 14 additions

| Category | 4004 | 4040 | New in 4040 |
|----------|------|------|-------------|
| Arithmetic | 14 | 14 | - |
| Logical | 3 (CMA, CMC, CLC) | 7 | OR4, OR5, AN6, AN7 |
| Branch/Jump | 5 | 5 | - |
| Memory | 16 | 17 | RPM |
| I/O | 8 | 8 | - |
| Stack | 2 (JMS, BBL) | 3 | BBS |
| Control | 1 (NOP) | 7 | HLT, EIN, DIN, LCR |
| Banking | 0 | 4 | SB0, SB1, DB0, DB1 |
| **Total** | **46** | **60** | **+14** |

### 7.3 Software Compatibility

**Upward Compatible**:
- All 4004 programs run on 4040 without modification
- Same opcodes, same behavior (within 4KB space)
- Binary image from 4004 loads directly into 4040 ROM bank 0

**Migration Path**:
```
4004 Program:
  - Fits in 4KB → Load into 4040 bank 0, works perfectly
  - Uses 3-level stack → 4040 provides 7 levels (extra capacity)
  - No changes needed

Enhanced 4040 Program:
  - Use bank 1 for extended code (4-8KB programs)
  - Use interrupts for event handling (vs polling)
  - Use HLT for power saving
  - Use logical ops (OR/AND) for bit manipulation
  - Use register banking for context switching
```

**NOT Downward Compatible**:
- 4040 programs using new instructions won't run on 4004
- 4040 programs using >4KB ROM won't fit on 4004
- Interrupt-driven 4040 code must be rewritten for polling on 4004

---

## 8. TYPICAL APPLICATIONS

### 8.1 Enhanced Calculator Systems

**Improvements Over 4004**:
- Larger ROM: More functions (scientific, financial)
- Interrupts: Real-time clock integration
- Halt mode: Battery life extension
- More registers: Complex formulas without RAM

**Example: Scientific Calculator**:
- Bank 0: Basic operations, UI, interrupt handlers
- Bank 1: Scientific functions (sin, cos, log, exp)
- Interrupts: Auto power-off timer, keyboard debounce
- HLT: Sleep between key presses

### 8.2 Industrial Control Systems

**4040 Advantages**:
- Interrupts: Respond to sensors/alarms immediately
- Larger ROM: Complex control logic, lookup tables
- 7-level stack: Hierarchical state machines
- Halt: Power saving in idle periods

**Example: Process Controller**:
```
Main Loop:
  - Monitor sensors via polling (slow)
  - HLT when idle

Interrupt Sources:
  - Emergency stop button (INT)
  - High-priority sensor alarms (INT)

Interrupts disabled during critical section (DIN/EIN)
```

### 8.3 Communication Terminals

**4040 Features Used**:
- Interrupts: Serial data arrival (UART RxD)
- ROM banking: Separate protocol stacks
- Registers: FIFO buffering
- Logical ops: Bit stuffing, parity checking

**Example: Serial Terminal**:
- Bank 0: Terminal emulator, keyboard handler
- Bank 1: Communication protocols (XMODEM, etc.)
- INT: Serial receive interrupt
- EF1: CTS (Clear To Send) from modem
- EF2: Carrier detect

### 8.4 Gaming and Entertainment

**Enhanced Capabilities**:
- Larger ROM: More game content, graphics
- Interrupts: Real-time game loop (60 Hz interrupt)
- Better sound: Interrupt-driven tone generation
- Complex AI: More stack for game logic

**Example: Pinball Machine (Bally)**:
- INT: Periodic display refresh (multiplexing)
- EF1: Tilt sensor
- EF2: Ball detection
- HLT: Attract mode power saving

---

## 9. IMPLEMENTATION VALIDATION

### 9.1 Emulator Implementation Status

**Files**:
- `emulator_core/source/K4040.hpp`
- `emulator_core/source/K4040.cpp`
- `emulator_core/tests/instructions_tests.cpp`

**Compliance Checklist**:
- ✅ All 60 instructions implemented (46 + 14 new)
- ✅ 7-level stack (13-bit addresses)
- ✅ 24 index registers in 2 banks
- ✅ 13-bit program counter (8KB addressing)
- ✅ Dual ROM banks (DB0/DB1)
- ✅ Dual register banks (SB0/SB1)
- ✅ Interrupt system (INT, EIN, DIN, BBS)
- ✅ Halt mode (HLT, STP output)
- ✅ Command register (LCR)
- ✅ SRC backup for interrupts
- ✅ Logical operations (OR4, OR5, AN6, AN7)
- ✅ Read program memory (RPM)
- ✅ All 4004 instructions (backward compatible)

### 9.2 New Instruction Validation

**Interrupt System**:
```cpp
// Interrupt enable/disable
✅ EIN instruction sets interrupt_enable flag
✅ DIN instruction clears interrupt_enable flag
✅ INT pin checked at end of each instruction
✅ Interrupt pending flag latches INT signal
✅ Jump to vector 0x003 (or 0x1003 if bank 1)
✅ SRC saved to backup register
✅ PC pushed to stack (7 levels available)
✅ BBS restores SRC and pops PC
```

**Banking System**:
```cpp
// Register banking
✅ SB0 switches to bank 0 (registers 0-11)
✅ SB1 switches to bank 1 (registers 12-23)
✅ Register operations use current bank
✅ FIM, INC, ADD, etc. operate on correct bank

// ROM banking
✅ DB0 selects ROM bank 0 (addresses 0x0000-0x0FFF)
✅ DB1 selects ROM bank 1 (addresses 0x1000-0x1FFF)
✅ PC uses current bank for fetch
✅ JUN, JMS respect current bank
```

**Halt Mode**:
```cpp
✅ HLT instruction implemented
✅ CPU stops execution after HLT
✅ STP signal/flag set when halted
✅ INT wakes CPU from halt (jumps to ISR)
✅ RESET wakes CPU from halt (PC ← 0)
```

**Logical Operations**:
```cpp
✅ OR4: ACC ← ACC | R[4]
✅ OR5: ACC ← ACC | R[5]
✅ AN6: ACC ← ACC & R[6]
✅ AN7: ACC ← ACC & R[7]
```

**Other New Instructions**:
```cpp
✅ LCR: ACC ← command_register
✅ RPM: ACC ← current_instruction_byte
```

### 9.3 Test Coverage

**4040-Specific Tests**:
- Interrupt enable/disable: ✅ Tested
- Interrupt handling: ✅ Tested
- Halt mode: ✅ Tested
- Register bank switching: ✅ Tested
- ROM bank switching: ✅ Tested
- New logical ops: ✅ Tested
- 7-level stack: ✅ Tested
- BBS instruction: ✅ Tested
- LCR instruction: ✅ Tested
- RPM instruction: ✅ Tested

**Edge Cases**:
- Stack depth (7 levels): ✅ Overflow tested
- Interrupt during interrupt (blocked): ✅ Tested
- Bank switching mid-subroutine: ✅ Tested
- Halt wake-up timing: ✅ Tested

---

## 10. REFERENCES

### 10.1 Official Intel Documentation

1. **MCS-40 User's Manual** (November 1974)
   - Complete 4040 specification
   - New instructions documented
   - System design with 4040
   - Interrupt handling examples

2. **Intel 4040 Datasheet**
   - Electrical specifications
   - 24-pin pinout
   - Timing diagrams
   - AC/DC characteristics

3. **MCS-40 Family Databook**
   - Complete chip family
   - 4040 + 4289 + 4201A
   - Application notes

### 10.2 Historical Context

**Why the 4040?**
- Address 4004 limitations (3-level stack, no interrupts, 4KB ROM)
- Compete with emerging 8-bit CPUs (8008, 8080)
- Maintain 4-bit cost advantage while adding features
- Backward compatibility with existing 4004 designs

**Market Position**:
- 1974-1981 production
- Used in calculators, controllers, terminals
- Eventually replaced by 8-bit CPUs (8048, 8051)
- Legacy: Longest-lived 4-bit CPU family

### 10.3 Online Resources

- **WikiChip**: https://en.wikichip.org/wiki/intel/mcs-40/4040
- **CPU World**: https://www.cpu-world.com/CPUs/4040/
- **Pastraiser CPU Museum**: https://pastraiser.com/cpu/i4040/

---

## 11. APPENDIX A: 4040-Specific Opcode Map

**New Instructions Only**:

```
Opcode | Mnemonic | Description
-------|----------|---------------------------
0x01   | HLT      | Halt until interrupt
0x02   | BBS      | Branch back from interrupt
0x03   | LCR      | Load command register to ACC
0x04   | OR4      | OR R[4] with ACC
0x05   | OR5      | OR R[5] with ACC
0x06   | AN6      | AND R[6] with ACC
0x07   | AN7      | AND R[7] with ACC
0x08   | DB0      | Designate ROM bank 0
0x09   | DB1      | Designate ROM bank 1
0x0A   | SB0      | Select register bank 0
0x0B   | SB1      | Select register bank 1
0x0C   | EIN      | Enable interrupts
0x0D   | DIN      | Disable interrupts
0x0E   | RPM      | Read program memory to ACC
```

**Complete 4040 Opcode Map** (includes all 4004 + new):
```
      0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
0x0   NOP  HLT  BBS  LCR  OR4  OR5  AN6  AN7  DB0  DB1  SB0  SB1  EIN  DIN  RPM  -
0x1   JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN  JCN
0x2   FIM/SRC (register pair addressing)
0x3   FIN/JIN (register pair indirect)
0x4   JUN  (12-bit unconditional jump)
0x5   JMS  (12-bit subroutine call)
0x6   INC  (increment register 0-15)
0x7   ISZ  (increment and skip)
0x8   ADD  (add register to ACC)
0x9   SUB  (subtract register from ACC)
0xA   LD   (load register to ACC)
0xB   XCH  (exchange ACC and register)
0xC   BBL  (branch back and load)
0xD   LDM  (load immediate to ACC)
0xE   WRM, WMP, WRR, WPM, WR0-3, SBM, RDM, RDR, ADM, RD0-3
0xF   CLB, CLC, IAC, CMC, CMA, RAL, RAR, TCC, DAC, TCS, STC, DAA, KBP, DCL, -, -
```

---

## APPENDIX B: Interrupt Service Routine Template

```assembly
; Minimal ISR (no context save)
; Entry point: 0x003 (bank 0) or 0x1003 (bank 1)
ISR_MINIMAL:
    ; Handle interrupt (ACC can be used)
    RDR         ; Read device status (example)
    ; ... process interrupt ...
    BBS         ; Return from interrupt (restores SRC, pops PC)

; Full context-save ISR
ISR_FULL:
    ; Save ACC to RAM
    SRC P0      ; Point to save area (register pair 0)
    WRM         ; Save accumulator

    ; Save carry (via TCC)
    TCC         ; ACC ← CY, CY ← 0
    INC R1      ; R1 points to next RAM location
    SRC P0      ; Update SRC
    WRM         ; Save old carry value

    ; Save working registers (example: R2, R3)
    LD R2
    INC R1
    SRC P0
    WRM

    LD R3
    INC R1
    SRC P0
    WRM

    ; === Handle interrupt here ===
    ; (Can now use ACC, R2, R3 freely)
    RDR         ; Read interrupt source
    ; ... process ...

    ; === Restore context ===
    ; Restore R3
    FIM P0, SAVE_AREA + 3
    SRC P0
    RDM
    XCH R3

    ; Restore R2
    FIM P0, SAVE_AREA + 2
    SRC P0
    RDM
    XCH R2

    ; Restore carry
    FIM P0, SAVE_AREA + 1
    SRC P0
    RDM
    ; If ACC != 0, set carry
    JCN Z, NO_CARRY
    STC
NO_CARRY:

    ; Restore ACC
    FIM P0, SAVE_AREA
    SRC P0
    RDM

    ; Return (BBS restores SRC automatically)
    BBS
```

---

## APPENDIX C: Power Supply Circuit (Dual Voltage)

```
        +-------+
VDD o---|  0.1  |---o VSS    VDD = -15V ±5%
(-15V)  |  μF   |   (GND)
        +-------+
         Ceramic

        +-------+
VCC o---|  0.1  |---o VSS    VCC = -10V ±5%
(-10V)  |  μF   |   (GND)
        +-------+
         Ceramic

Both placed close to IC pins (within 0.5")

Additional bulk capacitance at power entry:
  10-22 μF tantalum on VDD
  4.7-10 μF tantalum on VCC
```

---

**END OF DOCUMENT**
