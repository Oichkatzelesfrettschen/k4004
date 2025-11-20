# Intel 4004 Example Programs

**Collection of Educational and Demonstrative Programs**

This directory contains carefully crafted example programs that demonstrate various features and capabilities of the Intel 4004/4040 microprocessor emulator. Each program is extensively commented and designed for learning.

---

## 📚 Program Index

### Quick Start (Easiest First)

| # | Program | Difficulty | Key Concepts | Lines |
|---|---------|------------|--------------|-------|
| 1 | [Binary Counter](#1-binary-counter) | ⭐ Beginner | Output, Looping, Increment | ~60 |
| 2 | [BCD Calculator](#2-bcd-calculator) | ⭐⭐ Intermediate | BCD Arithmetic, DAA, RAM | ~200 |
| 3 | [Keyboard Scanner](#3-keyboard-scanner) | ⭐⭐⭐ Advanced | KBP, I/O, Matrix Scanning | ~200 |
| 4 | [LED Patterns](#4-led-patterns) | ⭐⭐ Intermediate | Rotation, Patterns, Delays | ~250 |
| 5 | [7-Segment Display](#5-7-segment-display) | ⭐⭐ Intermediate | Lookup Tables, Encoding | ~280 |

**Total**: ~990 lines of well-documented assembly code

---

## Program Details

### 1. Binary Counter
**File**: `01_binary_counter.asm`

**What it does**:
Counts from 0 to 15 in binary, displaying the result on 4 LEDs connected to a ROM output port. Demonstrates the most fundamental operations.

**Key Learning Points**:
- ✅ Basic I/O operations (WRR - Write ROM port)
- ✅ Register operations (LD, INC)
- ✅ Looping structures (JUN)
- ✅ Auto-wrap behavior of 4-bit arithmetic
- ✅ ROM chip selection (SRC)

**Hardware**:
- Intel 4001 ROM with 4-bit output port
- 4 LEDs (red, green, yellow, blue recommended)
- Current-limiting resistors (220Ω-470Ω)

**Expected Output**:
```
Binary:  0000 → 0001 → 0010 → 0011 → ... → 1111 → 0000 (repeats)
Decimal:   0  →   1  →   2  →   3  → ... →  15  →   0
```

**Performance**:
- Cycles per iteration: 6 (LD + WRR + INC + JUN + SRC)
- Iterations per second: ~123,000 at 740kHz
- Too fast to see! (Add delays for visible counting)

**Extensions**:
- Add delay loops for visible counting
- Count down instead of up (use DAC)
- Count in different patterns (even numbers, powers of 2)
- Use carry flag to extend to 8-bit counting

---

### 2. BCD Calculator
**File**: `02_bcd_calculator.asm`

**What it does**:
Performs Binary Coded Decimal (BCD) addition of multi-digit decimal numbers. Example: 387 + 256 = 643. Shows how the Intel 4004 was used in calculators like the famous Busicom 141-PF.

**Key Learning Points**:
- ✅ DAA (Decimal Adjust Accumulator) instruction
- ✅ BCD arithmetic principles
- ✅ Carry propagation between digits
- ✅ RAM operations (WRM, RDM, WR0-WR3, RD0-RD3)
- ✅ Multi-digit number handling
- ✅ Memory organization

**Hardware**:
- Intel 4002 RAM (at least 1 chip)
- No display required (results stored in registers/RAM)

**Example Calculation**:
```
     387  (3 in hundreds, 8 in tens, 7 in ones)
   + 256  (2 in hundreds, 5 in tens, 6 in ones)
   -----
     643  (6 in hundreds, 4 in tens, 3 in ones)

Ones:      7 + 6 = 13 → DAA → 3 with carry 1
Tens:      8 + 5 + 1 = 14 → DAA → 4 with carry 1
Hundreds:  3 + 2 + 1 = 6 → DAA → 6 with carry 0
```

**DAA Instruction**:
The magical DAA (Decimal Adjust Accumulator) converts binary addition results to proper BCD:
```
Binary: 0x07 + 0x06 = 0x0D (13)
After DAA: 0x03 with carry = 1 (correct: 1 ten + 3 ones)
```

**Extensions**:
- Implement subtraction (use SBM)
- Add multiplication (repeated addition)
- Add division (repeated subtraction)
- Support negative numbers
- Create full 4-function calculator

---

### 3. Keyboard Scanner
**File**: `03_keyboard_scanner.asm`

**What it does**:
Scans a 4×4 matrix keyboard (16 keys total) and identifies which key is pressed. Uses the KBP (Keyboard Process) instruction for efficient key detection.

**Key Learning Points**:
- ✅ KBP (Keyboard Process) instruction
- ✅ Matrix keyboard scanning technique
- ✅ Column-row addressing
- ✅ Bit pattern detection
- ✅ Debouncing strategies
- ✅ I/O port operations (RDR/WRR)

**Hardware**:
- Intel 4001 ROM with bidirectional I/O
- 4×4 matrix keyboard (or individual buttons)
- Pull-up/pull-down resistors

**Keyboard Layout**:
```
      Col0  Col1  Col2  Col3
Row0  [ 1]  [ 2]  [ 3]  [ A]
Row1  [ 4]  [ 5]  [ 6]  [ B]
Row2  [ 7]  [ 8]  [ 9]  [ C]
Row3  [ *]  [ 0]  [ #]  [ D]
```

**Scanning Algorithm**:
1. Output column select pattern (0001, 0010, 0100, 1000)
2. Read row response from input port
3. If any row bit is set, key detected in that column
4. Use KBP to convert row pattern to row number
5. Calculate key code: (row × 4) + column

**KBP Magic**:
```
Input (row pattern) → KBP → Output (row number)
0001 → 1
0010 → 2
0100 → 3
1000 → 4
0000 → 15 (no key pressed)
```

**Extensions**:
- Add proper debouncing (read twice, compare)
- Support multiple simultaneous keys (chord input)
- Add key repeat functionality
- Implement keyboard buffer/queue
- Add shift/ctrl/alt modifier keys

---

### 4. LED Patterns
**File**: `04_led_pattern.asm`

**What it does**:
Creates mesmerizing animated LED patterns including:
- Knight Rider (walking single LED)
- Expanding/contracting patterns
- Alternating bits
- Binary counting
- Pseudo-random sequences (LFSR)

**Key Learning Points**:
- ✅ RAL/RAR (Rotate Accumulator Left/Right)
- ✅ Bit manipulation techniques
- ✅ Animation through delays
- ✅ Pattern generation algorithms
- ✅ LFSR (Linear Feedback Shift Register)
- ✅ Subroutines (JMS/BBL)

**Hardware**:
- Intel 4001 ROM with output port
- 4-8 LEDs in a row
- Current-limiting resistors

**Pattern Gallery**:
```
Knight Rider:
●○○○ → ○●○○ → ○○●○ → ○○○● → ○○●○ → ○●○○ (repeats)

Expanding:
○●●○ → ●●●● → ●○○● → ○○○○ (repeats)

Alternating:
●○●○ → ○●○● → ●○●○ (repeats)

Binary Counter:
○○○○ (0) → ○○○● (1) → ○○●○ (2) → ... → ●●●● (15)

LFSR Pseudo-Random:
●○○● → ○●○○ → ●●○○ → ○●●○ → ... (15-step sequence)
```

**Timing**:
- Delay loop: ~1536 cycles = ~2ms at 740kHz
- Adjust nested loop counts for desired animation speed
- Human eye persistence: ~20-50ms for smooth animation

**Extensions**:
- Add more patterns (sparkle, chase, fade)
- Implement brightness control (PWM)
- Add music sync (rhythm visualization)
- Create interactive games (Simon Says, reaction timer)
- Drive more LEDs using shift registers

---

### 5. 7-Segment Display
**File**: `05_seven_segment_display.asm`

**What it does**:
Displays hexadecimal digits 0-F on a 7-segment LED display by converting binary values to segment patterns. Counts through all hex digits.

**Key Learning Points**:
- ✅ 7-segment encoding
- ✅ Pattern lookup/generation
- ✅ Conditional branching
- ✅ Display multiplexing concepts
- ✅ Visual output formatting

**Hardware**:
- Intel 4001 ROM with output port (or external interface)
- Common-cathode 7-segment LED display
- Current-limiting resistors (7× 220Ω-470Ω)

**7-Segment Layout**:
```
     AAA
    F   B
    F   B
     GGG
    E   C
    E   C
     DDD  DP
```

**Segment Patterns** (simplified 4-bit DCBA):
```
Digit  Pattern  Segments Lit
  0     1111    A,B,C,D (+ E,F in full)
  1     0110    B,C
  2     1101    A,B,D (+ G,E in full)
  3     1111    A,B,C,D (+ G in full)
  4     0110    B,C (+ F,G in full)
  5     1011    A,C,D (+ F,G in full)
  6     1011    A,C,D (+ E,F,G in full)
  7     0111    A,B,C
  8     1111    All segments
  9     1111    A,B,C,D (+ F,G in full)
  A     0111    A,B,C (+ E,F,G in full)
  b     1100    C,D (+ E,F,G in full)
  C     1001    A,D (+ E,F in full)
  d     1110    B,C,D (+ E,G in full)
  E     1001    A,D (+ E,F,G in full)
  F     0001    A (+ E,F,G in full)
```

**Note**: This simplified version uses only 4 bits (DCBA). For full 7-segment displays, use 8 bits (DP,G,F,E,D,C,B,A) as shown in the comments.

**Multiplexing** (for multiple digits):
To drive 4 digits with only 11 pins instead of 28:
1. Display digit 1 for 2ms
2. Display digit 2 for 2ms
3. Display digit 3 for 2ms
4. Display digit 4 for 2ms
5. Repeat (120Hz refresh = flicker-free)

**Extensions**:
- Implement full 8-bit segment patterns
- Add decimal point support
- Display multi-digit numbers (clock, calculator)
- Show scrolling text ("HELLO", "4004", etc.)
- Add brightness control (PWM)
- Display temperature/sensor data

---

## 🛠️ How to Use These Examples

### Prerequisites
1. K4004 Emulator built and working
2. K4004 Assembler compiled
3. Basic understanding of Intel 4004 architecture

### Assembling a Program

```bash
# Navigate to project root
cd /path/to/k4004

# Assemble an example
./output/bin/assembler programs/examples/01_binary_counter.asm -o binary_counter.bin

# The output is an object file ready to load into the emulator
```

### Running in Emulator

```bash
# Run the emulator with the assembled program
./output/bin/mcs4_emulator binary_counter.bin

# Or integrate into your own application using the emulator library
```

### Modifying Examples

All programs are designed to be modified and experimented with:

1. **Change delay times**: Adjust loop counts in DELAY subroutines
2. **Add debug output**: Insert additional WRR instructions
3. **Combine patterns**: Mix and match code from different examples
4. **Create variations**: Use as templates for your own programs

---

## 🎓 Learning Path

**Recommended Order**:

1. **Start**: Binary Counter
   - Simplest possible program
   - Learn basic I/O and looping
   - ~10 minutes to understand

2. **Next**: LED Patterns
   - Fun visual feedback
   - Learn bit manipulation
   - ~30 minutes to master

3. **Then**: 7-Segment Display
   - Practical output device
   - Learn pattern encoding
   - ~30 minutes

4. **Advanced**: BCD Calculator
   - Real-world application
   - Learn BCD arithmetic
   - ~60 minutes

5. **Expert**: Keyboard Scanner
   - Complex I/O interaction
   - Learn scanning techniques
   - ~60 minutes

**Total Learning Time**: ~3-4 hours for complete mastery

---

## 📊 Technical Specifications

### Instruction Usage Coverage

| Instruction | Used In | Purpose |
|-------------|---------|---------|
| **I/O Operations** |
| WRR | All | Write ROM output port |
| RDR | Keyboard | Read ROM input port |
| WRM | Calculator | Write RAM main char |
| RDM | Calculator | Read RAM main char |
| SRC | All | Send register control (chip select) |
| **Arithmetic** |
| ADD | Calculator, Keyboard | Addition |
| SUB | Keyboard | Subtraction |
| INC | Counter, Patterns | Increment |
| DAC | Calculator, Patterns | Decrement |
| DAA | Calculator | Decimal adjust (BCD) |
| **Data Movement** |
| LD | All | Load accumulator from register |
| LDM | All | Load accumulator immediate |
| XCH | All | Exchange accumulator and register |
| FIM | All | Fetch immediate to register pair |
| **Bit Operations** |
| RAL | Patterns | Rotate accumulator left |
| RAR | Patterns | Rotate accumulator right |
| CLC | Calculator, Patterns | Clear carry |
| **Control Flow** |
| JUN | All | Jump unconditional |
| JCN | Keyboard, Display | Jump conditional |
| JMS | Patterns, Display | Jump to subroutine |
| BBL | Patterns, Display | Branch back and load |
| **Special** |
| KBP | Keyboard | Keyboard process |
| NOP | Patterns | Delay timing |

**Coverage**: 20+ different instructions demonstrated across 5 programs

### Performance Characteristics

| Program | Cycles/Iteration | Iterations/sec @ 740kHz | Typical Use |
|---------|------------------|-------------------------|-------------|
| Counter | 6 | ~123,000 | Learning |
| Calculator | N/A | One-shot | Demonstration |
| Keyboard | ~50-100 | ~10,000 scans/sec | Interactive |
| Patterns | ~1536 (with delay) | ~480/sec | Visual |
| Display | ~100-200 | ~5,000/sec | Output |

---

## 🔧 Hardware Connection Guide

### Minimal Test Setup

**Required**:
- Intel 4004 CPU
- Intel 4201A Clock Generator
- Intel 4001 ROM (256 bytes, one chip minimum)
- Power supply (15V, 5V, -10V for original; modern: 5V for CMOS versions)

**For Examples**:
1. **Counter & Patterns**: + 4 LEDs + resistors
2. **Calculator**: + Intel 4002 RAM
3. **Keyboard**: + 4×4 matrix keyboard
4. **Display**: + 7-segment LED + resistors

### Pin Connections (4001 ROM I/O)

```
Intel 4001 I/O Port:
  Pin 17: I/O 0 (bit 0)  →  LED 0 / Segment A
  Pin 18: I/O 1 (bit 1)  →  LED 1 / Segment B
  Pin 19: I/O 2 (bit 2)  →  LED 2 / Segment C
  Pin 20: I/O 3 (bit 3)  →  LED 3 / Segment D

For Input (Keyboard):
  Same pins configured as inputs via mask
```

**LED Connection**:
```
4001 I/O Pin → 220Ω Resistor → LED Anode → LED Cathode → GND
```

**7-Segment Connection**:
```
4001 I/O 0 → 330Ω → Segment A
4001 I/O 1 → 330Ω → Segment B
4001 I/O 2 → 330Ω → Segment C
4001 I/O 3 → 330Ω → Segment D
Common cathode → GND
```

---

## 📖 Additional Resources

### Intel 4004 Documentation
- [MCS-4 User Manual](../docs/specs/) - Official Intel documentation
- [Instruction Set Reference](../docs/INSTRUCTION_LEVEL_AUDIT.md) - Complete instruction details
- [Cycle Timing Specification](../docs/CYCLE_TIMING_SPECIFICATION.md) - Timing accuracy

### Online Resources
- **4004.com** - Comprehensive 4004 history and technical details
- **CPU Shack Museum** - Vintage microprocessor information
- **Bitsavers** - Historical computer documentation archive

### Related Projects
- **Busicom 141-PF** - Original calculator design
- **MCS-4 Evaluation Kit** - Historical development kit
- **Intel Museum** - Silicon history

---

## 🤝 Contributing

### Adding New Examples

We welcome contributions! To add a new example:

1. **Choose a clear, educational topic**
2. **Write extensively commented code** (50%+ should be comments)
3. **Follow the existing format**:
   - Header block with description
   - Hardware requirements
   - Features demonstrated
   - Register usage map
   - Extensive technical notes
   - Extension ideas
4. **Test thoroughly** on the emulator
5. **Update this README** with your program details

### Example Template

```assembly
; =============================================================================
; Program Name - Intel 4004 Example Program
; =============================================================================
; Description:
;   One paragraph describing what the program does and why it's useful
;
; Hardware Requirements:
;   - List all required chips and components
;
; Features Demonstrated:
;   - ✅ Feature 1
;   - ✅ Feature 2
;
; Register Usage:
;   R0: Purpose
;   R1: Purpose
; =============================================================================

; Your well-commented code here
```

---

## 📝 License

These example programs are provided as educational resources. Feel free to use, modify, and distribute for learning purposes.

**Original Author**: Intel 4004 Emulator Project
**Created**: 2025-11-19
**Status**: Production Ready ✅

---

## 🎯 Quick Reference

### Assembly Syntax Cheat Sheet

```assembly
; Comments start with semicolon

LABEL               ; Define a label (can be used as jump target)
    NOP             ; No operation
    LDM $05         ; Load immediate: ACC = 5
    LD R0           ; Load register: ACC = R0
    XCH R1          ; Exchange: R1 ↔ ACC
    ADD R2          ; Add: ACC = ACC + R2
    SUB R3          ; Subtract: ACC = ACC - R3
    INC R4          ; Increment: R4 = R4 + 1
    FIM P0, $AB     ; Fetch immediate pair: R0=0xB, R1=0xA
    JUN LABEL       ; Jump unconditional
    JCN Z, LABEL    ; Jump if zero
    JCN NZ, LABEL   ; Jump if not zero
    JCN C, LABEL    ; Jump if carry
    JMS SUBR        ; Jump to subroutine
    BBL $00         ; Return from subroutine
    WRR             ; Write ROM port
    RDR             ; Read ROM port
    WRM             ; Write RAM
    RDM             ; Read RAM
    DAA             ; Decimal adjust accumulator
    KBP             ; Keyboard process
```

### Conditional Jump Codes

| Code | Condition | Description |
|------|-----------|-------------|
| Z | Zero | Jump if ACC = 0 |
| NZ | Not Zero | Jump if ACC ≠ 0 |
| C | Carry | Jump if CY = 1 |
| NC | Not Carry | Jump if CY = 0 |
| AZ | ACC Zero | Jump if ACC = 0 |
| AN | ACC Not Zero | Jump if ACC ≠ 0 |
| TZ | Test Zero | Jump if TEST = 0 |
| TN | Test Not Zero | Jump if TEST ≠ 0 |

---

**Happy Programming on the World's First Microprocessor! 🎉**
