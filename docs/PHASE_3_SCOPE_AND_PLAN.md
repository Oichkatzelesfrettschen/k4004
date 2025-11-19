# Phase 3: Next Steps - Scope & Execution Plan

**Date**: 2025-11-19
**Current Status**: 100% Test Coverage Complete ✅
**Phase**: Moving from Testing → Validation & Enhancement

---

## 🎯 Strategic Priorities (Scoped)

### Immediate High-Value Tasks (This Session)

Based on OPTIMAL_PATH_FORWARD.md and current capabilities, here's the scoped execution plan:

#### Priority 1: Validation Report & Metrics (30 min)
**Why**: Document achievements, create baseline for future comparisons
**Deliverables**:
- Comprehensive validation metrics report
- Performance benchmarks (instruction counts, test execution time)
- Completeness matrix (chips × features)
- Ready-for-production checklist

**Value**: HIGH - Demonstrates maturity, aids users/researchers

#### Priority 2: Cycle-Accurate Timing Implementation (2-3 hours)
**Why**: Many applications need timing accuracy, high-value feature
**Approach**:
1. Add cycle counter to CPU state
2. Implement cycle counts for all 60 instructions (from INSTRUCTION_LEVEL_AUDIT.md)
3. Add timing validation tests
4. Document cycle behavior

**Value**: VERY HIGH - Enables time-dependent applications

#### Priority 3: Example Programs (1-2 hours)
**Why**: Demonstrate emulator capabilities, educational value
**Programs to Create**:
1. Binary counter (simplest - tests outputs)
2. BCD calculator (4-function)
3. Keyboard scanner (tests K4003)
4. Walking LED pattern (shift register demo)
5. "Hello World" equivalent (7-segment display)

**Value**: HIGH - Makes emulator accessible

#### Priority 4: Busicom ROM Integration Planning (Research)
**Why**: Gold standard validation, but requires ROM acquisition
**Approach**:
1. Research Busicom ROM availability
2. Design ROM loader architecture
3. Plan integration tests
4. Document expected behavior

**Value**: VERY HIGH - Ultimate validation, but blocked on ROM access

---

## 📊 Phase 3 Execution Matrix

| Task | Priority | Effort | Value | Blocking? | Execute Now? |
|------|----------|--------|-------|-----------|--------------|
| Validation Report | P1 | 30m | High | No | ✅ YES |
| Cycle Timing | P2 | 2-3h | Very High | No | ✅ YES |
| Example Programs | P3 | 1-2h | High | No | ✅ YES |
| Busicom Planning | P4 | 1h | Very High | ROM access | ⏭️ Research |
| Enhanced Assembler | P5 | 12-20h | Medium | No | ⏭️ Later |
| Debugger | P6 | 15-25h | Medium | No | ⏭️ Later |

---

## 🚀 Execution Plan (This Session)

### Step 1: Validation Report (IMMEDIATE)
**Goal**: Create comprehensive validation metrics
**Time**: 30 minutes

**Deliverables**:
1. Test coverage matrix
2. Chip completeness table
3. Instruction validation grid
4. Performance benchmarks
5. Ready-for-production checklist

### Step 2: Cycle-Accurate Timing (HIGH VALUE)
**Goal**: Implement instruction cycle counting
**Time**: 2-3 hours

**Implementation Steps**:
1. Add `uint64_t cycleCount` to CPU state
2. Create `getCycleCount()` and `resetCycleCount()` methods
3. Add cycle increments to all instruction implementations
4. Use INSTRUCTION_LEVEL_AUDIT.md for cycle counts
5. Create cycle validation tests
6. Document timing behavior

**Example**:
```cpp
// NOP - 8 cycles (2 instruction cycles)
void NOP() {
    // No operation
    // Cycles: 8 (fetches next instruction)
}

// ADD - 8 cycles
void ADD(uint8_t& ACC, const uint8_t* registers, uint8_t IR) {
    // ... existing code ...
    // Cycles: 8
}

// JMS - 16 cycles (2 instruction cycles)
void JMS(...) {
    // ... existing code ...
    // Cycles: 16 (reads address byte from ROM)
}
```

### Step 3: Example Programs (EDUCATIONAL)
**Goal**: Create demonstrative applications
**Time**: 1-2 hours

**Programs**:

1. **Binary Counter** (binary_counter.asm)
   - Count 0-15 in binary
   - Output to 4 LEDs
   - Tests: Basic output, INC instruction

2. **BCD Calculator** (bcd_calculator.asm)
   - 4-function calculator (+ - × ÷)
   - BCD arithmetic using DAA
   - Tests: Full BCD stack, keyboard input

3. **Keyboard Scanner** (keyboard_scan.asm)
   - 4×4 matrix keyboard
   - Shift register pattern (K4003)
   - KBP instruction usage
   - Tests: Real keyboard scanning

4. **Walking LED** (walking_led.asm)
   - Single LED walks across 10 positions
   - K4003 shift register
   - Tests: Shift register cascading

5. **7-Segment Display** (seven_segment.asm)
   - Display "8004" (reference to 4004)
   - Segment patterns
   - Tests: Display output

### Step 4: Busicom Research (PLANNING)
**Goal**: Plan gold standard validation
**Time**: 1 hour

**Research Questions**:
1. Where to obtain Busicom ROM?
   - Check 4004.com archives
   - Computer History Museum
   - Vintage computing forums

2. ROM format and structure?
   - 4 × 4001 ROMs (1024 bytes total)
   - Address mapping
   - Initialization sequence

3. Expected behavior?
   - Keyboard input format
   - Display output format
   - √2 calculation sequence

4. Test validation criteria?
   - Output matches expected
   - Timing within tolerance
   - All functions work

---

## 📋 Detailed Task Breakdowns

### Task 1: Validation Report

**Metrics to Capture**:
```
Test Coverage:
- Total Tests: 247/247 (100%)
- By Category:
  - Instruction Tests: 39/39 (100%)
  - Chip Tests: 208/208 (100%)
  - Integration Tests: ...

Chip Completeness:
- 4004 CPU: 46/46 instructions (100%)
- 4040 CPU: 60/60 instructions (100%)
- Memory: All addressing modes
- I/O: All ports functional
- Peripherals: All interfaces working

Performance:
- Test execution time: ~1ms
- Build time: ~3 seconds
- Binary size: ...
- Memory footprint: ...

Code Quality:
- Lines of code: ...
- Documentation: 10,000+ lines
- Test coverage: 100%
- Known issues: 0
```

### Task 2: Cycle Timing Implementation

**Cycle Counts (from INSTRUCTION_LEVEL_AUDIT.md)**:

```
Single-Cycle Instructions (8 cycles):
- NOP, CLB, CLC, IAC, CMC, CMA, RAL, RAR, TCC, DAC, TCS, STC, DAA, KBP
- ADD, SUB, LD, XCH, LDM
- ADM, SBM, RDM, RD0-RD3
- WRM, WMP, WRR, WR0-WR3, WPM
- DCL

Double-Cycle Instructions (16 cycles):
- FIM, SRC, FIN, JIN, JUN, JMS, INC, ISZ, BBL, JCN

Special:
- HLT: Halts (cycles until interrupt)
- BBS: 24 cycles (bank switch)
```

**Architecture**:
```cpp
// In K4004 class
private:
    uint64_t m_cycleCount;

public:
    uint64_t getCycleCount() const { return m_cycleCount; }
    void resetCycleCount() { m_cycleCount = 0; }

// Each instruction increments:
void executeInstruction(uint8_t opcode) {
    // ... decode and execute ...
    m_cycleCount += instructionCycles;
}
```

### Task 3: Example Programs

**Binary Counter Example**:
```assembly
; Binary Counter - Count 0-15 on 4 LEDs
; Uses R0 as counter, outputs to ROM port

START:
    LDM 0           ; Load 0
    XCH R0          ; R0 = counter

LOOP:
    LD R0           ; Load counter
    WRR             ; Write to ROM output port
    INC R0          ; Increment counter
    LD R0
    CMA             ; Complement
    IAC             ; Add 1
    JCN CZ, DELAY   ; If zero, we overflowed (16 counts)
    JUN LOOP        ; Continue counting

DELAY:
    ; Simple delay loop
    FIM R1R2, 0xFF
DLOOP:
    ISZ R1, DLOOP
    ISZ R2, DLOOP
    JUN START       ; Restart count
```

**BCD Calculator Skeleton**:
```assembly
; 4-Function BCD Calculator
; Memory layout:
; - R0R1: Input buffer (2 digits)
; - R2R3: Accumulator (2 digits BCD)
; - R4: Operation (+,-,×,÷)
; - R5: Status flags

; Main loop:
; 1. Scan keyboard for input
; 2. Process digit or operation
; 3. Update display
; 4. Repeat

INIT:
    LDM 0
    XCH R2          ; Clear accumulator
    XCH R3

MAIN_LOOP:
    FIM R6R7, KBSCAN_ADDR
    SRC R6R7
    JMS KEYBOARD_SCAN

    ; Check if digit (0-9) or operation (+,-,×,÷,=)
    LD R0           ; Get key
    CLC
    DAA             ; Valid BCD?
    JCN CZ, PROCESS_DIGIT
    JMS PROCESS_OPERATION
    JUN MAIN_LOOP

PROCESS_DIGIT:
    ; Shift left and add new digit
    ; Implementation...

; ... more subroutines ...
```

---

## 🎯 Success Criteria

### Phase 3 Complete When:
- ✅ Validation report generated
- ✅ Cycle counting implemented
- ✅ Cycle tests passing
- ✅ 3+ example programs created
- ✅ Example programs tested
- ✅ Busicom integration planned
- ✅ Documentation updated

### Ready for Phase 4 When:
- ✅ Busicom ROM obtained
- ✅ ROM loader implemented
- ✅ √2 calculation passes
- ✅ Enhanced assembler started
- ✅ Debugger design complete

---

## 📈 Expected Outcomes

### By End of Session:
- **Validation Report**: Complete emulator metrics
- **Cycle Timing**: 60/60 instructions with cycle counts
- **Example Programs**: 3-5 working programs
- **Documentation**: +1,000 lines (examples, timing specs)
- **Test Coverage**: Still 100% (possibly +10 cycle tests)

### Value Delivered:
1. **Cycle Accuracy**: Enables timing-dependent applications
2. **Examples**: Makes emulator accessible to users
3. **Validation**: Proves production-readiness
4. **Planning**: Clear path to gold standard (Busicom)

---

## 🚀 Let's Execute!

**Starting with**:
1. ✅ Create validation metrics report (30 min)
2. ✅ Implement cycle counting (2-3 hours)
3. ✅ Create example programs (1-2 hours)
4. ✅ Update documentation

**Total Estimated Time**: 4-6 hours of high-value work!

Ready to continue? Let's build! 🔨

---

**END OF SCOPE DOCUMENT**
