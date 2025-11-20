; =============================================================================
; 7-Segment Display Demo - Intel 4004 Example Program
; =============================================================================
; Description:
;   Demonstrates driving a 7-segment LED display by counting from 0-9 and
;   displaying hexadecimal digits 0-F. Shows how to convert binary values
;   to segment patterns.
;
; Hardware Requirements:
;   - Intel 4004 CPU
;   - Intel 4001 ROM (with I/O port)
;   - Common-cathode 7-segment LED display
;   - Current-limiting resistors (typically 220Ω-470Ω)
;
; 7-Segment Layout:
;          AAA
;         F   B
;         F   B
;          GGG
;         E   C
;         E   C
;          DDD   (DP)
;
; Segment Mapping to ROM Port Bits:
;   Bit 0 = Segment A (top)
;   Bit 1 = Segment B (top right)
;   Bit 2 = Segment C (bottom right)
;   Bit 3 = Segment D (bottom)
;   (If using full 8-bit port on external interface:
;    Bit 4 = Segment E (bottom left)
;    Bit 5 = Segment F (top left)
;    Bit 6 = Segment G (middle)
;    Bit 7 = Decimal Point)
;
; Features Demonstrated:
;   - Lookup tables
;   - Memory addressing and indexing
;   - 7-segment encoding
;   - Display multiplexing concept
;
; Register Usage:
;   R0R1 (P0): ROM addressing
;   R2: Current digit to display (0-15)
;   R3: Segment pattern
;   R4R5: Table base address
; =============================================================================

INIT
    FIM P0, $00     ; Initialize ROM chip 0
    SRC P0          ; Select ROM for I/O
    LDM $00         ; Start with digit 0
    XCH R2          ; Store in R2

; =============================================================================
; Main Display Loop
; =============================================================================
MAIN_LOOP
    LD R2           ; Load current digit
    JMS GET_PATTERN ; Get 7-segment pattern for this digit
    WRR             ; Display on 7-segment LED
    JMS DELAY       ; Delay for visibility

    INC R2          ; Next digit
    LD R2           ; Load digit
    XCH R3          ; Save in R3
    SUB R3          ; Check if < 16
    JCN Z, INIT     ; If wrapped to 0 (after 15), restart

    JUN MAIN_LOOP

; =============================================================================
; Get 7-Segment Pattern Subroutine
; =============================================================================
; Input: ACC = digit (0-15)
; Output: ACC = 7-segment pattern (only 4 bits, simplified)
; Uses: R4, R5 as temporary
;
; For 4-bit output (segments A,B,C,D only):
GET_PATTERN
    XCH R4          ; Save digit in R4

; Use simple conditional logic (no table lookup due to limited addressing)
; This is simplified version using direct encoding

    JCN Z, PATTERN_0    ; If digit = 0
    DAC                 ; Decrement
    JCN Z, PATTERN_1    ; If digit = 1
    DAC
    JCN Z, PATTERN_2    ; If digit = 2
    DAC
    JCN Z, PATTERN_3    ; If digit = 3
    DAC
    JCN Z, PATTERN_4    ; If digit = 4
    DAC
    JCN Z, PATTERN_5    ; If digit = 5
    DAC
    JCN Z, PATTERN_6    ; If digit = 6
    DAC
    JCN Z, PATTERN_7    ; If digit = 7
    DAC
    JCN Z, PATTERN_8    ; If digit = 8
    DAC
    JCN Z, PATTERN_9    ; If digit = 9
    DAC
    JCN Z, PATTERN_A    ; If digit = A
    DAC
    JCN Z, PATTERN_B    ; If digit = B
    DAC
    JCN Z, PATTERN_C    ; If digit = C
    DAC
    JCN Z, PATTERN_D    ; If digit = D
    DAC
    JCN Z, PATTERN_E    ; If digit = E
    JUN PATTERN_F       ; Must be F

; Simplified 4-bit patterns (DCBA segments only)
; For full 7-segment, would need 8 bits (GFEDCBA + DP)

PATTERN_0   ; "0" = segments ABCD lit = 1111 = 0xF
    LDM $0F
    BBL $00

PATTERN_1   ; "1" = segments BC lit = 0110 = 0x6
    LDM $06
    BBL $00

PATTERN_2   ; "2" = segments ABD lit (+ G,E in full) = approx 1101 = 0xD
    LDM $0D
    BBL $00

PATTERN_3   ; "3" = segments ABCD lit (+ G in full) = 1111 = 0xF
    LDM $0F
    BBL $00

PATTERN_4   ; "4" = segments BC lit (+ F,G in full) = 0110 = 0x6
    LDM $06
    BBL $00

PATTERN_5   ; "5" = segments ACD lit (+ F,G in full) = 1011 = 0xB
    LDM $0B
    BBL $00

PATTERN_6   ; "6" = segments ACD lit (+ E,F,G in full) = 1011 = 0xB
    LDM $0B
    BBL $00

PATTERN_7   ; "7" = segments ABC lit = 0111 = 0x7
    LDM $07
    BBL $00

PATTERN_8   ; "8" = all segments lit = 1111 = 0xF
    LDM $0F
    BBL $00

PATTERN_9   ; "9" = segments ABCD lit (+ F,G in full) = 1111 = 0xF
    LDM $0F
    BBL $00

PATTERN_A   ; "A" = segments ABC lit (+ E,F,G in full) = 0111 = 0x7
    LDM $07
    BBL $00

PATTERN_B   ; "b" = segments CD lit (+ E,F,G in full) = 1100 = 0xC
    LDM $0C
    BBL $00

PATTERN_C   ; "C" = segments AD lit (+ E,F in full) = 1001 = 0x9
    LDM $09
    BBL $00

PATTERN_D   ; "d" = segments BCD lit (+ E,G in full) = 1110 = 0xE
    LDM $0E
    BBL $00

PATTERN_E   ; "E" = segments AD lit (+ E,F,G in full) = 1001 = 0x9
    LDM $09
    BBL $00

PATTERN_F   ; "F" = segments A lit (+ E,F,G in full) = 0001 = 0x1
    LDM $01
    BBL $00

; =============================================================================
; Delay Subroutine
; =============================================================================
DELAY
    LDM $0F
    XCH R6
DELAY_OUTER2
    LDM $0F
    XCH R7
DELAY_INNER2
    NOP
    NOP
    NOP
    INC R7
    JCN NZ, DELAY_INNER2
    INC R6
    JCN NZ, DELAY_OUTER2
    BBL $00

; =============================================================================
; Full 7-Segment Truth Table (for reference)
; =============================================================================
; For a complete implementation with all 7 segments + DP (8 bits total):
;
; Bit:     7   6   5   4   3   2   1   0
; Segment: DP  G   F   E   D   C   B   A
;
; Digit  HEX   DP G F E D C B A
;   0    0x3F   0 0 1 1 1 1 1 1  (all except G)
;   1    0x06   0 0 0 0 0 1 1 0  (B,C only)
;   2    0x5B   0 1 0 1 1 0 1 1  (A,B,D,E,G)
;   3    0x4F   0 1 0 0 1 1 1 1  (A,B,C,D,G)
;   4    0x66   0 1 1 0 0 1 1 0  (B,C,F,G)
;   5    0x6D   0 1 1 0 1 1 0 1  (A,C,D,F,G)
;   6    0x7D   0 1 1 1 1 1 0 1  (A,C,D,E,F,G)
;   7    0x07   0 0 0 0 0 1 1 1  (A,B,C)
;   8    0x7F   0 1 1 1 1 1 1 1  (all segments)
;   9    0x6F   0 1 1 0 1 1 1 1  (A,B,C,D,F,G)
;   A    0x77   0 1 1 1 0 1 1 1  (A,B,C,E,F,G)
;   B    0x7C   0 1 1 1 1 1 0 0  (C,D,E,F,G)
;   C    0x39   0 0 1 1 1 0 0 1  (A,D,E,F)
;   D    0x5E   0 1 0 1 1 1 1 0  (B,C,D,E,G)
;   E    0x79   0 1 1 1 1 0 0 1  (A,D,E,F,G)
;   F    0x71   0 1 1 1 0 0 0 1  (A,E,F,G)
;
; To implement full patterns, would need to store table in RAM or ROM
; and use indirect addressing to fetch patterns.
;
; =============================================================================
; Extension Ideas:
; =============================================================================
; 1. Add decimal point for decimal numbers
; 2. Display multi-digit numbers with multiplexing
; 3. Show scrolling text (Hello, 4004, etc.)
; 4. Implement clock display (HH:MM format)
; 5. Add brightness control (PWM)
; 6. Display temperature or other sensor data
; 7. Create calculator display (like Busicom 141-PF)
; 8. Implement hexadecimal calculator
; 9. Show error messages ("E", "F", etc.)
; 10. Animated patterns (counting, chase effects)
;
; =============================================================================
; Multiplexing for Multiple Digits:
; =============================================================================
; To drive multiple 7-segment displays (e.g., 4-digit clock):
;
; 1. Display digit 1, enable transistor 1
; 2. Delay briefly (~2ms)
; 3. Disable transistor 1
; 4. Display digit 2, enable transistor 2
; 5. Delay briefly
; 6. Disable transistor 2
; 7. Repeat for digits 3-4
; 8. Loop back to digit 1
;
; If refreshed fast enough (>60Hz total), appears as steady display
; due to persistence of vision. 4 digits @ 2ms each = 120Hz refresh rate.
;
; Advantages:
; - Saves I/O pins (7 segments + 4 digits = 11 pins vs 28 pins)
; - Saves power (only one digit on at a time)
; - Reduces component cost
;
; Used in nearly all digital clocks, calculators, and displays.
; =============================================================================
