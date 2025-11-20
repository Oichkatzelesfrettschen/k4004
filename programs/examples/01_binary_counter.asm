; =============================================================================
; Binary Counter - Intel 4004 Example Program
; =============================================================================
; Description:
;   Demonstrates basic output operations by counting from 0 to 15 in binary.
;   The count value is continuously displayed on a ROM output port.
;
; Hardware Requirements:
;   - Intel 4004 CPU
;   - Intel 4001 ROM (at least one chip)
;   - 4 LEDs connected to ROM output port bits 0-3
;
; Features Demonstrated:
;   - Basic arithmetic (increment)
;   - Output operations (WRR - Write ROM port)
;   - Looping structures
;   - Register operations
;   - Overflow handling (0xF → 0x0)
;
; Expected Behavior:
;   The LEDs will count in binary: 0000, 0001, 0010, 0011, ..., 1111, 0000, ...
;   Each count is visible for a brief moment (no delay implemented).
;
; Register Usage:
;   R0: Counter value (0-15)
;   ACC: Temporary for output operations
;
; Cycle Count:
;   Loop iteration: 6 cycles (LD=1, WRR=1, INC=1, JUN=2, SRC=1)
;   ~162,000 iterations/second at 740kHz
; =============================================================================

; Initialize ROM chip select (SRC sends to ROM port 0)
INIT
    FIM P0, $00     ; R0R1 = 0x00 (chip 0, register 0)
    SRC P0          ; Select ROM chip 0

; Main counting loop
LOOP
    LD R0           ; Load counter into accumulator
    WRR             ; Write accumulator to ROM output port
    INC R0          ; Increment counter (auto-wraps at 16)
    JUN LOOP        ; Repeat forever

; =============================================================================
; Technical Notes:
; =============================================================================
; 1. INC instruction automatically wraps: 15 + 1 = 0 (4-bit operation)
; 2. WRR writes to the ROM port selected by the most recent SRC
; 3. The counter runs at maximum speed (no delays)
; 4. To add delays, insert NOP instructions or delay loops
; 5. ROM output format: bit 3 = MSB, bit 0 = LSB
;
; Example Output Sequence:
;   0000 (0)  →  0001 (1)  →  0010 (2)  →  0011 (3)  →  0100 (4)  →  ...
;   1111 (15) →  0000 (0)  →  0001 (1)  →  ... (repeats forever)
; =============================================================================
