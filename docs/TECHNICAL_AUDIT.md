# Intel 4004/4040 Emulator - Comprehensive Technical Audit

**Date:** 2025-11-19
**Status:** Deep Analysis and Expansion Phase
**Goal:** Documentation-accurate, functionally accurate MCS-4/MCS-40 emulation

---

## Executive Summary

This repository contains a partially implemented Intel 4004 and 4040 emulator with assembler and test infrastructure. The implementation demonstrates good architectural foundation but has **critical accuracy issues** and **missing functionality** that must be resolved for documentation-accurate emulation.

### Critical Findings

1. **STACK SIZE ERRORS** - Both CPUs have incorrect stack depths
2. **MISSING 4040 INSTRUCTIONS** - All 14 new 4040 instructions are declared but not implemented
3. **MISSING HARDWARE FEATURES** - No interrupt support, no halt mode, no bank switching
4. **CODE QUALITY** - Multiple TODOs indicate incomplete implementation and missing bounds checks

---

## 1. Intel 4004 (MCS-4) Specifications vs Implementation

### 1.1 CPU Core Specifications

| Feature | Intel Specification | Current Implementation | Status |
|---------|-------------------|----------------------|--------|
| Clock Speed | 740 kHz max | Not timing-accurate | ⚠️ PARTIAL |
| Instruction Set | 46 instructions | 46 instructions | ✅ COMPLETE |
| Registers | 16 × 4-bit (8 pairs) | 8 registers (correct) | ✅ CORRECT |
| Stack Depth | **3 levels** | **4 levels** | ❌ **BUG** |
| Accumulator | 4-bit + carry | 5-bit (4+1 carry) | ✅ CORRECT |
| ROM Address Space | 12-bit (4KB) | 0x03FF mask (1KB) | ⚠️ INCOMPLETE |
| Program Counter | 12-bit | 10-bit (0x03FF) | ❌ **BUG** |
| Interrupts | None | None | ✅ CORRECT |
| Instruction Timing | 10.8 or 21.6 μs | Returns cycle count | ⚠️ PARTIAL |

**Critical Bug:** `K4004.hpp:11` defines `STACK_SIZE = 4u` but should be **3** per Intel specifications.

**Critical Bug:** PC mask is `0x03FFu` (10-bit = 1KB) but should be `0x0FFFu` (12-bit = 4KB).

### 1.2 4004 Instruction Set (46 Instructions)

All 46 instructions are implemented in `instructions.cpp`. Implementation appears functionally correct for most instructions, but has identified TODOs requiring attention.

### 1.3 MCS-4 Support Chips

| Chip | Specification | Current Implementation | Status |
|------|--------------|----------------------|--------|
| 4001 ROM | 256B ROM + 4-bit I/O port | 4KB ROM, basic I/O ports | ⚠️ PARTIAL |
| 4002 RAM | 40B RAM + 4-bit output | 1024 nibbles, status, output | ⚠️ PARTIAL |
| 4003 I/O | 10-bit shift register | Not implemented | ❌ MISSING |

---

## 2. Intel 4040 (MCS-40) Specifications vs Implementation

### 2.1 CPU Core Specifications

| Feature | Intel Specification | Current Implementation | Status |
|---------|-------------------|----------------------|--------|
| Clock Speed | 740 kHz max | Not timing-accurate | ⚠️ PARTIAL |
| Instruction Set | 60 instructions (46+14) | 46 implemented, 14 declared | ❌ **INCOMPLETE** |
| Registers | 24 × 4-bit (12 pairs × 2 banks) | 12 registers (1 bank) | ❌ **MISSING BANKS** |
| Stack Depth | **7 levels** | **8 levels** | ❌ **BUG** |
| ROM Address Space | 13-bit (8KB, dual bank) | 10-bit (1KB) | ❌ **BUG** |
| Interrupts | INT pin, EIN/DIN instructions | Not implemented | ❌ MISSING |
| Halt Mode | STP pin, HLT instruction | Not implemented | ❌ MISSING |
| Package | 24-pin DIP | N/A (software) | N/A |

**Critical Bug:** `K4040.hpp:10` defines `STACK_SIZE = 8u` but should be **7** per Intel specifications.

**Critical Bug:** `K4040.hpp:9` defines `REGISTERS_SIZE = 12u` but with bank switching should support **24 registers** (2 banks of 12).

### 2.2 The 14 New 4040 Instructions - ALL MISSING IMPLEMENTATION

| Mnemonic | Opcode | Function | Implementation Status |
|----------|--------|----------|---------------------|
| HLT | 0x01 | Halt until interrupt or STP | ❌ Declared, not implemented |
| BBS | 0x02 | Branch back from interrupt, restore SRC | ❌ Declared, not implemented |
| LCR | 0x03 | Load command register to accumulator | ❌ Declared, not implemented |
| OR4 | 0x04 | OR index register 4 with accumulator | ❌ Declared, not implemented |
| OR5 | 0x05 | OR index register 5 with accumulator | ❌ Declared, not implemented |
| AN6 | 0x06 | AND index register 6 with accumulator | ❌ Declared, not implemented |
| AN7 | 0x07 | AND index register 7 with accumulator | ❌ Declared, not implemented |
| DB0 | 0x08 | Designate ROM bank 0 | ❌ Declared, not implemented |
| DB1 | 0x09 | Designate ROM bank 1 | ❌ Declared, not implemented |
| SB0 | 0x0A | Select index register bank 0 | ❌ Declared, not implemented |
| SB1 | 0x0B | Select index register bank 1 | ❌ Declared, not implemented |
| EIN | 0x0C | Enable interrupt system | ❌ Declared, not implemented |
| DIN | 0x0D | Disable interrupt system | ❌ Declared, not implemented |
| RPM | 0x0E | Read program memory to accumulator | ❌ Declared, not implemented |

**Function stubs exist in `instructions.hpp:10-25` but have NO implementations in `instructions.cpp`.**

### 2.3 Required Hardware State for 4040 Features

To implement the 14 new instructions, the following hardware state must be added to K4040:

```cpp
// Required additions to K4040 class
uint8_t m_registers_bank0[REGISTERS_SIZE];  // First bank (current)
uint8_t m_registers_bank1[REGISTERS_SIZE];  // Second bank (new)
uint8_t m_currentRegisterBank;               // 0 or 1
uint8_t m_currentROMBank;                    // 0 or 1 (13-bit addressing)
uint8_t m_commandRegister;                   // Command register for LCR
uint8_t m_srcBackup;                         // SRC backup for BBS
bool m_interruptEnabled;                     // Interrupt enable flag
bool m_halted;                               // Halt state
bool m_interruptPending;                     // INT pin state
```

---

## 3. Code Quality Issues

### 3.1 TODO Items Requiring Resolution

| File | Line | Issue | Priority |
|------|------|-------|----------|
| instructions.cpp | 93 | `// TODO: !! Add bounds check !!` (JMS stack) | **HIGH** |
| instructions.cpp | 152 | `// TODO: !! Add bounds check !!` (BBL stack) | **HIGH** |
| instructions.cpp | 157 | `// TODO: fix this, should load lsb 4bits to acc directly` | MEDIUM |
| emulator_core/source/emulator.hpp | 10 | `// TODO: add load from binary` | LOW |
| instructions_tests.cpp | 391, 512, 565, 662 | `// TODO: add means to edit rom` | MEDIUM |
| instructions_tests.cpp | 678 | `// TODO: Make tests for instructions that operates on page boundary` | MEDIUM |

### 3.2 Stack Bounds Checking

**CRITICAL SAFETY ISSUE:** Both JMS and BBL manipulate the stack pointer without bounds checking.

```cpp
// instructions.cpp:92-94 - JMS has no overflow check
++SP;
// TODO: !! Add bounds check !!
stack[SP] = address & 0x03FFu;

// instructions.cpp:151-155 - BBL has no underflow check
if (SP > 0) {
    // TODO: !! Add bounds check !!
    stack[SP] = 0u;
    --SP;
}
```

**Required Fix:** Add proper bounds checking against `STACK_SIZE` limits.

---

## 4. Memory Architecture Analysis

### 4.1 ROM Organization

**Current:** `ROM.hpp` defines ROM_SIZE as `PAGE_SIZE (256) * NUM_ROM_CHIPS (16) = 4096 bytes`

**4004 Requirement:** 12-bit addressing = 4096 bytes (current implementation correct)

**4040 Requirement:** 13-bit addressing = 8192 bytes (needs dual bank support)

**Fix Required:** Implement ROM bank switching for 4040 via DB0/DB1 instructions.

### 4.2 RAM Organization

**Current Implementation (RAM.hpp):**
```cpp
NUM_RAM_BANKS = 4      // 4 banks
NUM_RAM_CHIPS = 4      // 4 chips per bank
NUM_RAM_REGS = 4       // 4 registers per chip
NUM_REG_CHARS = 16     // 16 characters per register
NUM_STATUS_CHARS = 4   // 4 status characters per register
```

**Calculated Sizes:**
- RAM: 4 × 4 × 4 × 16 = 1024 nibbles (correct for maximal configuration)
- Status: 4 × 4 × 4 × 4 = 256 nibbles
- Output ports: 4 × 4 = 16 ports

**Intel 4002 Spec:** Each chip has 40 nibbles (32 data + 8 status) + 1 output port

**Analysis:** Current implementation supports expanded configuration ("cranked to eleven") which exceeds original spec. This is acceptable per project goals.

---

## 5. Instruction Implementation Analysis

### 5.1 Implemented Instructions (46 for 4004)

All 46 4004 instructions have implementations. Spot-check analysis:

**Arithmetic Instructions:** ✅ ADD, SUB, INC, DAC, IAC appear correct
**Logical Instructions:** ✅ CMA, RAL, RAR, CLC, CMC, STC appear correct
**Transfer Instructions:** ✅ LD, XCH, LDM appear correct
**Branch Instructions:** ✅ JCN, JUN, JMS, JIN appear correct with noted TODOs
**Memory Instructions:** ✅ WRM, RDM, WR0-3, RD0-3 appear correct
**Special Instructions:** ✅ KBP, DAA, DCL appear correct

### 5.2 Missing Implementations (14 for 4040)

**ALL 14 new 4040 instructions have NO implementation.** They are declared in:
- `assembly.hpp:6-19` (opcode enums)
- `instructions.hpp:10-25` (function declarations)

But have no corresponding function definitions in `instructions.cpp`.

---

## 6. Test Infrastructure Analysis

### 6.1 Existing Tests

**Location:** `emulator_core/tests/instructions_tests.cpp` (678 lines)

**Coverage:** Tests exist for many 4004 instructions

**Issues:**
- No tests for 14 new 4040 instructions (they don't exist yet)
- TODOs indicate missing ROM editing capability for some tests
- Missing page boundary condition tests

### 6.2 Test Infrastructure Quality

**Positive:**
- Uses Google Test framework (gtest)
- Tests are well-structured
- Good separation of concerns

**Needs Improvement:**
- Cycle timing validation
- Comprehensive edge case coverage
- 4040-specific test suite

---

## 7. Documentation Analysis

### 7.1 Existing Documentation

**Present in `docs/` directory:**
- ✅ MCS-4_UsersManual_Feb73.pdf (5.2 MB)
- ✅ MCS-4_AssemblyProgrammingManual_Dec73.pdf (7.4 MB)
- ✅ MCS-4_Intellec4AndMicroComputerModules_Jan74.pdf (4.5 MB)
- ✅ MCS-40_UsersManual_Nov74.pdf (18.3 MB)
- ✅ MCS-40_MicrocomputerSetAdvanceSpecifications_Sep74.pdf (2.7 MB)

**Analysis:** Excellent primary source documentation! All official Intel manuals are present.

### 7.2 Missing Documentation

❌ `requirements.md` - Build requirements and dependencies
❌ `ARCHITECTURE.md` - System architecture overview
❌ `INSTRUCTION_REFERENCE.md` - Quick instruction reference
❌ `DEVELOPMENT_ROADMAP.md` - Future development plans
❌ `TESTING.md` - Test strategy and coverage
❌ Build/installation instructions in README are minimal

---

## 8. Repository Organization Analysis

### 8.1 Current Structure

```
k4004/
├── assembler/          ✅ Well organized
│   ├── source/
│   └── tests/
├── emulator_core/      ✅ Well organized
│   ├── source/
│   └── tests/
├── emulator_apps/      ✅ Exists
├── docs/               ✅ Has PDFs
├── programs/           ✅ Has sample programs
├── shared/             ✅ Shared code
└── third_party/        ✅ Dependencies (gtest)
```

### 8.2 Missing Directories (Per Requirements)

❌ `scripts/` - Build, test, and utility scripts
❌ `logs/` - Build logs, test output
❌ `build/` - Out-of-tree build directory (usually gitignored)
❌ `archive/` - Deprecated/legacy code

---

## 9. Assembler Analysis

### 9.1 Assembler Capabilities

**Present:**
- ✅ Full 4004/4040 syntax support (grammar defined in README)
- ✅ Tokenizer (344 lines)
- ✅ Assembler (478 lines)
- ✅ Labels, numeric literals (decimal, hex, binary)
- ✅ Register and register pair syntax
- ✅ Pragma support (`.BYTE`)
- ✅ Comprehensive tests (assembler_tests, tokenizer_tests, disassembler_tests)

**Issues:**
- Multiple TODOs in assembler.cpp suggest incomplete implementation
- Error handling could be improved (TODO at tokenizer.cpp:72)

---

## 10. Critical Path to Accuracy

### Phase 1: Fix Critical Bugs (HIGH PRIORITY)
1. **Fix K4004 stack size:** 4 → 3 levels
2. **Fix K4040 stack size:** 8 → 7 levels
3. **Fix PC mask:** 0x03FF → 0x0FFF (4004), 0x1FFF (4040 with banks)
4. **Add stack bounds checks** in JMS and BBL

### Phase 2: Implement 4040 Hardware Features (HIGH PRIORITY)
1. Add register bank switching (SB0/SB1)
2. Add ROM bank switching (DB0/DB1)
3. Add interrupt system (EIN/DIN, interrupt flag, INT pin)
4. Add halt mode (HLT instruction, halted flag, STP pin)
5. Add command register and SRC backup for BBS

### Phase 3: Implement 14 Missing 4040 Instructions (HIGH PRIORITY)
1. Implement logical operations: OR4, OR5, AN6, AN7
2. Implement bank selection: DB0, DB1, SB0, SB1
3. Implement interrupt control: EIN, DIN, BBS
4. Implement special: HLT, LCR, RPM

### Phase 4: Expand Support Chips (MEDIUM PRIORITY)
1. Proper 4001 ROM chip emulation (256B + I/O per chip)
2. Proper 4002 RAM chip emulation (40B + output per chip)
3. Implement 4003 shift register (10-bit parallel output)

### Phase 5: Advanced Features (MEDIUM PRIORITY)
1. 4-bit BASIC interpreter
2. 4-bit DOS/monitor system
3. Serial/TTY interface
4. Era-appropriate peripherals (1970-1980)

### Phase 6: Quality and Testing (ONGOING)
1. Comprehensive test suite with cycle-accurate timing
2. Resolve all TODOs
3. Add page boundary tests
4. Performance benchmarking

---

## 11. Compatibility Validation

### 11.1 4004 Instruction Compatibility Test

To validate 4004 compatibility, we need to:
1. Test against known 4004 programs (e.g., Busicom calculator)
2. Verify all 46 instructions produce correct results
3. Validate timing (cycle counts)
4. Test edge cases (page boundaries, stack overflow)

### 11.2 4040 Backward Compatibility

The 4040 is **fully compatible** with 4004 code. Test requirements:
1. All 4004 programs must run identically on 4040 emulator
2. The 14 new instructions must be ignored/invalid on 4004 emulator
3. Stack depth difference (3 vs 7) must not break compatibility

---

## 12. Build System Analysis

### 12.1 CMake Configuration

**Present:**
- ✅ Top-level CMakeLists.txt with options
- ✅ BUILD_TESTS option (ON by default, requires gtest)
- ✅ STATIC_ANALYSIS option (OFF by default, requires clang-tidy)
- ✅ Modular CMakeLists for assembler, emulator_core, emulator_apps

**Quality:** Build system appears well-structured and modern.

### 12.2 Missing Build Features

❌ Installation targets (`make install`)
❌ Package generation (CPack)
❌ Cross-compilation support
❌ Continuous integration beyond basic CI workflow
❌ Automated documentation generation (Doxygen)

---

## 13. Project Scope Validation

### 13.1 Original Project Goal (per README)

> "Emulator tries to 'crank everything up to eleven', meaning amount of ROM and RAM, number of I/O lines and available peripherals."
>
> "The ultimate goal is to make something fun (like simple BASIC interpreter maybe?) running on the system."

**Analysis:** This is an ambitious and clear goal that goes beyond pure emulation to explore "what if" scenarios with enhanced hardware.

### 13.2 Expanded Scope (per Current Requirements)

Current requirements add:
- 4-bit BASIC interpreter ✅ Aligns with original goal
- 4-bit DOS/monitor system ✅ Logical extension
- Serial/TTY for interaction ✅ Necessary for BASIC
- Era-appropriate hardware ✅ Interesting addition
- Full documentation accuracy ⚠️ May conflict with "crank to eleven" goal

**Recommendation:** Maintain TWO modes:
1. **Accurate Mode:** Strict Intel spec compliance
2. **Enhanced Mode:** "Cranked to eleven" with expanded memory and peripherals

---

## 14. Recommendations and Roadmap

### 14.1 Immediate Actions (This Session)

1. ✅ Complete this technical audit document
2. Create ARCHITECTURE.md with system design
3. Fix critical bugs (stack sizes, PC mask, bounds checks)
4. Implement 4040 hardware state extensions
5. Implement the 14 missing 4040 instructions
6. Create requirements.md

### 14.2 Short-Term Goals (Next Sprint)

1. Comprehensive test suite for all 60 instructions
2. Cycle-accurate timing validation
3. Page boundary condition tests
4. ROM editing capability for tests
5. Enhanced documentation

### 14.3 Medium-Term Goals

1. 4003 shift register implementation
2. Proper chip-level emulation (4001, 4002, 4003)
3. Serial/TTY interface design
4. Initial BASIC interpreter design
5. DOS/monitor system architecture

### 14.4 Long-Term Vision

1. Full 4-bit BASIC interpreter
2. Interactive development environment
3. Era-appropriate peripheral emulation
4. Performance optimization
5. Hardware acceleration (FPGA?)

---

## 15. Conclusion

This emulator project has a **solid foundation** but requires **significant accuracy fixes** and **feature completion** before achieving the goal of documentation-accurate MCS-4/MCS-40 emulation.

### Strengths
- ✅ Well-structured codebase
- ✅ Excellent official documentation
- ✅ Good test infrastructure foundation
- ✅ All 60 instructions defined
- ✅ Modular architecture

### Critical Weaknesses
- ❌ Stack size bugs in both CPUs
- ❌ PC addressing bugs
- ❌ 14 missing instruction implementations
- ❌ No interrupt system
- ❌ No bank switching
- ❌ Missing stack bounds checks

### Priority Assessment

**MUST FIX IMMEDIATELY:**
1. Stack sizes (3 for 4004, 7 for 4040)
2. PC addressing (12-bit for 4004, 13-bit for 4040)
3. Stack bounds checking

**MUST IMPLEMENT SOON:**
1. All 14 4040 instructions
2. Interrupt system
3. Bank switching
4. Halt mode

**SHOULD ADD:**
1. Comprehensive tests
2. Better documentation
3. 4003 shift register

**NICE TO HAVE:**
1. BASIC interpreter
2. DOS/monitor
3. Serial interface

---

**Assessment:** With focused effort on the critical path, this project can achieve its goals of creating an accurate, well-documented, and functionally complete Intel 4004/4040 emulator with enhanced capabilities.

**Next Step:** Begin implementation of fixes and missing features following the critical path outlined above.

---

*Document prepared by: Claude (Sonnet 4.5)*
*Analysis based on: Intel official documentation + full codebase audit*
*Confidence Level: HIGH (based on authoritative sources)*

**AD ASTRA PER MATHEMATICA ET SCIENTIAM**
