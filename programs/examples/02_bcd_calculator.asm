; =============================================================================
; BCD Calculator - Intel 4004 Example Program
; =============================================================================
; Description:
;   Demonstrates BCD (Binary Coded Decimal) arithmetic using the DAA
;   instruction. Performs multi-digit decimal addition.
;
; Hardware Requirements:
;   - Intel 4004 CPU
;   - Intel 4002 RAM (for storing BCD digits)
;
; Features Demonstrated:
;   - DAA (Decimal Adjust Accumulator) instruction
;   - BCD arithmetic (each 4-bit nibble = 0-9)
;   - Multi-digit addition with carry propagation
;   - RAM operations (reading/writing)
;   - Subroutines (JMS/BBL)
;
; Example Calculation:
;   Add two 3-digit BCD numbers: 387 + 256 = 643
;   Stored in RAM as: [3][8][7] + [2][5][6] = [6][4][3]
;
; Memory Map:
;   RAM Register 0:
;     Char 0: First number, ones digit
;     Char 1: First number, tens digit
;     Char 2: First number, hundreds digit
;     Char 3: Second number, ones digit
;     Char 4: Second number, tens digit
;     Char 5: Second number, hundreds digit
;     Char 6: Result, ones digit
;     Char 7: Result, tens digit
;     Char 8: Result, hundreds digit
;
; Register Usage:
;   R0R1 (P0): RAM addressing
;   R2: Loop counter for digits
;   R3: Temporary storage
; =============================================================================

; Initialize - Set up RAM addressing
INIT
    FIM P0, $00     ; P0 = chip 0, register 0
    SRC P0          ; Select RAM register 0

; Store first number: 387
    LDM $07         ; Load 7 (ones digit)
    WRM             ; Write to RAM char 0

    LDM $08         ; Load 8 (tens digit)
    WR1             ; Write to RAM char 1

    LDM $03         ; Load 3 (hundreds digit)
    WR2             ; Write to RAM char 2

; Store second number: 256
    LDM $06         ; Load 6 (ones digit)
    WR3             ; Write to RAM char 3

    FIM P0, $04     ; Move to char 4
    SRC P0

    LDM $05         ; Load 5 (tens digit)
    WRM             ; Write to RAM char 4

    LDM $02         ; Load 2 (hundreds digit)
    WR1             ; Write to RAM char 5

; Perform BCD addition: 387 + 256
    CLC             ; Clear carry flag
    FIM P0, $00     ; Reset to char 0
    SRC P0

; Add ones digit
    RDM             ; Read first number ones (7)
    XCH R3          ; Save in R3
    RD3             ; Read second number ones (6)
    ADD R3          ; Add: 7 + 6 = 13
    DAA             ; Decimal adjust: 13 → 3, carry set
    WR3             ; Store result ones digit (3) in char 6

; Add tens digit (with carry)
    RD1             ; Read first number tens (8)
    XCH R3          ; Save in R3

    FIM P0, $04     ; Move to char 4
    SRC P0
    RDM             ; Read second number tens (5)
    ADD R3          ; Add: 8 + 5 = 13 + carry(1) = 14
    DAA             ; Decimal adjust: 14 → 4, carry set
    WR3             ; Store result tens digit (4) in char 7

; Add hundreds digit (with carry)
    RD2             ; Read first number hundreds (3)
    XCH R3          ; Save in R3
    RD1             ; Read second number hundreds (2)
    ADD R3          ; Add: 3 + 2 = 5 + carry(1) = 6
    DAA             ; Decimal adjust: 6 → 6, carry clear

    FIM P0, $08     ; Move to char 8
    SRC P0
    WRM             ; Store result hundreds digit (6)

; Result is now stored: 643
; Read back and display result
DISPLAY
    FIM P0, $06     ; Point to result digits (char 6-8)
    SRC P0

    RDM             ; Read ones (3)
    XCH R0          ; Store in R0
    RD1             ; Read tens (4)
    XCH R1          ; Store in R1
    RD2             ; Read hundreds (6)
    XCH R2          ; Store in R2

; Infinite loop - result in R0R1R2 = 643
DONE
    JUN DONE

; =============================================================================
; Technical Notes - BCD Arithmetic
; =============================================================================
; The DAA (Decimal Adjust Accumulator) instruction corrects binary addition
; results to proper BCD format:
;
; Example 1: 7 + 6 = 13 (0x0D binary)
;   After ADD: ACC = 0x0D, CY = 0
;   After DAA: ACC = 0x03, CY = 1 (added 6 for adjustment)
;   Decimal: 13 = 1 carry + 3 remainder ✓
;
; Example 2: 8 + 5 + carry(1) = 14 (0x0E binary)
;   After ADD: ACC = 0x0E, CY = 0
;   After DAA: ACC = 0x04, CY = 1 (added 6 for adjustment)
;   Decimal: 14 = 1 carry + 4 remainder ✓
;
; Example 3: 3 + 2 + carry(1) = 6 (0x06 binary)
;   After ADD: ACC = 0x06, CY = 0
;   After DAA: ACC = 0x06, CY = 0 (no adjustment needed)
;   Decimal: 6 = 0 carry + 6 remainder ✓
;
; DAA Algorithm:
;   IF (ACC > 9) OR (carry from binary addition)
;       ACC = ACC + 6
;       Set carry flag
;
; This allows the 4004 to perform decimal arithmetic using only 4 bits per
; digit, essential for calculator applications like the Busicom 141-PF.
; =============================================================================

; =============================================================================
; Extension Ideas:
; =============================================================================
; 1. Add subtraction using SBM and DAS operations
; 2. Implement multiplication (repeated addition)
; 3. Add keyboard input for operands
; 4. Display results on 7-segment displays
; 5. Support negative numbers (sign-magnitude or complement)
; 6. Implement division (repeated subtraction)
; =============================================================================
