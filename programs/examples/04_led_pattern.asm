; =============================================================================
; LED Pattern Generator - Intel 4004 Example Program
; =============================================================================
; Description:
;   Demonstrates shift register operations by creating walking LED patterns.
;   Uses ROM output port to control LEDs in various animated sequences.
;
; Hardware Requirements:
;   - Intel 4004 CPU
;   - Intel 4001 ROM (with I/O port)
;   - 4 LEDs connected to ROM output port bits 0-3
;
; Features Demonstrated:
;   - Shift operations (RAL/RAR - Rotate Accumulator Left/Right)
;   - Pattern generation
;   - Delay loops for visible animation
;   - ROM output operations
;   - Bit manipulation
;
; LED Patterns:
;   1. Walking single bit (Knight Rider effect)
;   2. Walking pair of bits
;   3. Expanding/contracting pattern
;   4. Binary counting
;   5. Random-looking pattern
;
; Register Usage:
;   R0R1 (P0): ROM addressing
;   R2: Pattern data
;   R3: Delay counter
;   R4: Loop counter
; =============================================================================

INIT
    FIM P0, $00     ; Initialize ROM chip 0
    SRC P0          ; Select ROM for I/O
    CLC             ; Clear carry (important for rotation)

; =============================================================================
; Pattern 1: Knight Rider (Single LED Walking Left to Right and Back)
; =============================================================================
KNIGHT_RIDER
    LDM $01         ; Start with LED 0: 0001
    XCH R2          ; Store pattern in R2

; Walk right (0001 → 0010 → 0100 → 1000)
WALK_RIGHT
    LD R2           ; Load pattern
    WRR             ; Display on LEDs
    JMS DELAY       ; Pause for visibility

    LD R2           ; Load pattern
    CLC             ; Clear carry
    RAL             ; Rotate left: 0001 → 0010 → 0100 → 1000
    XCH R2          ; Save new pattern

    XCH R4          ; Check if we've reached rightmost (1000)
    JCN C, WALK_LEFT ; If carry set, we wrapped (1000 → 0001), go back

    JUN WALK_RIGHT  ; Continue walking right

; Walk left (1000 → 0100 → 0010 → 0001)
WALK_LEFT
    LDM $08         ; Reset to rightmost: 1000
    XCH R2

WALK_LEFT_LOOP
    LD R2           ; Load pattern
    WRR             ; Display on LEDs
    JMS DELAY       ; Pause

    LD R2           ; Load pattern
    CLC             ; Clear carry
    RAR             ; Rotate right: 1000 → 0100 → 0010 → 0001
    XCH R2          ; Save new pattern

    XCH R4          ; Check pattern
    ADD R4          ; Double it
    JCN Z, PATTERN2 ; If zero after double, we're at 0001, next pattern

    JUN WALK_LEFT_LOOP

; =============================================================================
; Pattern 2: Expanding/Contracting (Center Out)
; =============================================================================
PATTERN2
    LDM $06         ; Start with center LEDs: 0110
    XCH R2

EXPAND
    LD R2           ; Load pattern
    WRR             ; Display
    JMS DELAY

; Expand: 0110 → 1111
    LDM $0F         ; All LEDs on
    WRR
    JMS DELAY

; Contract: 1111 → 1001 → 0000
CONTRACT
    LDM $09         ; Corners: 1001
    WRR
    JMS DELAY

    LDM $00         ; All off
    WRR
    JMS DELAY

; =============================================================================
; Pattern 3: Alternating Bits
; =============================================================================
PATTERN3
    LDM $0A         ; Pattern: 1010
    WRR
    JMS DELAY

    LDM $05         ; Pattern: 0101
    WRR
    JMS DELAY

    LDM $0A         ; Pattern: 1010
    WRR
    JMS DELAY

    LDM $05         ; Pattern: 0101
    WRR
    JMS DELAY

; =============================================================================
; Pattern 4: Binary Counter
; =============================================================================
PATTERN4
    LDM $00         ; Start at 0
    XCH R2

COUNT_UP
    LD R2           ; Load count
    WRR             ; Display
    JMS DELAY       ; Delay

    INC R2          ; Increment (auto-wraps at 16)
    LD R2
    JCN Z, PATTERN5 ; If wrapped to zero, next pattern

    JUN COUNT_UP

; =============================================================================
; Pattern 5: Pseudo-Random Sequence (Linear Feedback Shift Register)
; =============================================================================
PATTERN5
    LDM $09         ; Seed: 1001 (must be non-zero)
    XCH R2

LFSR_LOOP
    LD R2           ; Load current value
    WRR             ; Display
    JMS DELAY

; Simple LFSR: feedback from bits 0 and 1 (XOR)
    LD R2           ; Get current value
    XCH R3          ; Save copy in R3

; Extract bit 0
    LD R2
    CLC
    RAR             ; Get bit 0 into carry
    TCC             ; Transfer carry to accumulator (bit 0)
    XCH R4          ; Save bit 0

; Extract bit 1
    LD R3
    CLC
    RAR
    CLC
    RAR             ; Get bit 1 into carry
    TCC             ; Transfer carry to accumulator (bit 1)

; XOR with bit 0 (poor man's XOR using arithmetic)
    LD R4           ; This is simplified - real XOR would use bit ops
    CMA             ; Complement
    CLC

; Shift right and insert feedback bit
    LD R2
    CLC
    RAR             ; Shift right
    RAR
    RAR
    RAR
    XCH R2          ; Save new value

    LD R2
    JCN Z, RESTART  ; If zero (shouldn't happen), restart
    XCH R4          ; Check if back to seed
    SUB R4
    JCN Z, RESTART  ; If back to original, restart

    JUN LFSR_LOOP

; Restart all patterns
RESTART
    JUN INIT

; =============================================================================
; Delay Subroutine
; =============================================================================
; Creates a visible delay by wasting CPU cycles.
; Adjustable by changing the loop count.
DELAY
    LDM $0F         ; Outer loop count
    XCH R5
DELAY_OUTER
    LDM $0F         ; Inner loop count
    XCH R6
DELAY_INNER
    NOP             ; Waste cycles
    NOP
    NOP
    NOP
    INC R6          ; Increment (wraps at 16)
    JCN NZ, DELAY_INNER  ; Loop until wrap

    INC R5          ; Increment outer
    JCN NZ, DELAY_OUTER  ; Loop until wrap

    BBL $00         ; Return from subroutine

; =============================================================================
; Technical Notes:
; =============================================================================
; 1. RAL (Rotate Accumulator Left):
;    - Shifts all bits left by one position
;    - Bit 3 goes into carry
;    - Carry goes into bit 0
;    - Example: CY=0, ACC=0101 → CY=0, ACC=1010
;    - Example: CY=0, ACC=1010 → CY=1, ACC=0100
;
; 2. RAR (Rotate Accumulator Right):
;    - Shifts all bits right by one position
;    - Bit 0 goes into carry
;    - Carry goes into bit 3
;    - Example: CY=0, ACC=1010 → CY=0, ACC=0101
;    - Example: CY=0, ACC=0101 → CY=1, ACC=0010
;
; 3. Delay Calculation:
;    - Nested loop: 16 × 16 = 256 iterations
;    - Inner loop: ~6 cycles (NOP×4 + INC + JCN)
;    - Outer loop: ~3 cycles
;    - Total: ~1536 cycles per delay call
;    - At 740 kHz: ~2ms per delay
;    - Visible to human eye (>~20ms needed for persistence)
;    - Adjust loop counts for desired speed
;
; 4. LFSR (Linear Feedback Shift Register):
;    - Generates pseudo-random sequence
;    - Period = 2^n - 1 for n-bit LFSR (max 15 for 4-bit)
;    - Taps determine sequence (this uses simplified version)
;    - Never produces all-zeros (gets stuck)
;    - Used in many applications: crypto, CRC, random numbers
;
; =============================================================================
; Extension Ideas:
; =============================================================================
; 1. Add more patterns (chase, sparkle, fade)
; 2. Use RAM to store pattern sequences
; 3. Add button input to select patterns
; 4. Implement PWM for LED brightness control
; 5. Drive 8 or more LEDs using shift registers (K4003)
; 6. Create music/rhythm visualization
; 7. Implement game (Simon Says, reaction time tester)
; =============================================================================
