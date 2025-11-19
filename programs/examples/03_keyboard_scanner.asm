; =============================================================================
; Keyboard Scanner - Intel 4004 Example Program
; =============================================================================
; Description:
;   Demonstrates keyboard scanning using the KBP (Keyboard Process)
;   instruction and ROM I/O ports. Scans a 4x4 matrix keyboard and
;   identifies which key is pressed.
;
; Hardware Requirements:
;   - Intel 4004 CPU
;   - Intel 4001 ROM (with I/O ports)
;   - 4x4 matrix keyboard (16 keys)
;
; Keyboard Matrix Layout:
;   Columns (output on ROM port bits 0-3):
;       C0   C1   C2   C3
;   R0  [1]  [2]  [3]  [A]  ← Row 0 (input bit 0)
;   R1  [4]  [5]  [6]  [B]  ← Row 1 (input bit 1)
;   R2  [7]  [8]  [9]  [C]  ← Row 2 (input bit 2)
;   R3  [*]  [0]  [#]  [D]  ← Row 3 (input bit 3)
;
; Features Demonstrated:
;   - KBP (Keyboard Process) instruction
;   - ROM I/O operations (RDR/WRR)
;   - Column scanning technique
;   - Bit pattern detection
;   - Conditional branching
;
; Scanning Algorithm:
;   1. Output column select pattern (0001, 0010, 0100, 1000)
;   2. Read row response
;   3. If row bit set, key found
;   4. Use KBP to identify which bit
;   5. Calculate key code
;
; Register Usage:
;   R0R1 (P0): ROM addressing
;   R2: Column counter (0-3)
;   R3: Row data
;   R4: Key code result
; =============================================================================

INIT
    FIM P0, $00     ; Initialize ROM chip 0
    SRC P0          ; Select ROM for I/O

; Scan column 0 (pattern 0001)
SCAN_COL0
    LDM $01         ; Column 0 select: 0001
    WRR             ; Output to ROM port
    RDR             ; Read row inputs
    JCN Z, SCAN_COL1  ; If no key (zero), try next column
    XCH R3          ; Save row data
    LDM $00         ; Column number = 0
    XCH R2          ; Save column
    JUN PROCESS_KEY ; Process the key press

; Scan column 1 (pattern 0010)
SCAN_COL1
    LDM $02         ; Column 1 select: 0010
    WRR             ; Output to ROM port
    RDR             ; Read row inputs
    JCN Z, SCAN_COL2  ; If no key, try next column
    XCH R3          ; Save row data
    LDM $01         ; Column number = 1
    XCH R2          ; Save column
    JUN PROCESS_KEY

; Scan column 2 (pattern 0100)
SCAN_COL2
    LDM $04         ; Column 2 select: 0100
    WRR             ; Output to ROM port
    RDR             ; Read row inputs
    JCN Z, SCAN_COL3  ; If no key, try next column
    XCH R3          ; Save row data
    LDM $02         ; Column number = 2
    XCH R2          ; Save column
    JUN PROCESS_KEY

; Scan column 3 (pattern 1000)
SCAN_COL3
    LDM $08         ; Column 3 select: 1000
    WRR             ; Output to ROM port
    RDR             ; Read row inputs
    JCN Z, NO_KEY   ; If no key, none pressed
    XCH R3          ; Save row data
    LDM $03         ; Column number = 3
    XCH R2          ; Save column
    JUN PROCESS_KEY

; No key pressed
NO_KEY
    LDM $0F         ; Key code = 15 (no key)
    XCH R4          ; Store in R4
    JUN INIT        ; Restart scan

; Process detected key press
PROCESS_KEY
    LD R3           ; Load row data
    KBP             ; Keyboard process: convert bit pattern to number
                    ; 0001 → 1, 0010 → 2, 0100 → 3, 1000 → 4
                    ; 0000 → 15 (with carry check for disambiguation)

; Calculate key code: key = (row * 4) + column
; Row is in ACC (1-4 from KBP), column is in R2 (0-3)

    DAC             ; Decrement (KBP returns 1-4, we need 0-3)
    XCH R3          ; Save row number (0-3) in R3

; Multiply row by 4 (shift left 2 positions)
    ADD R3          ; ACC = row * 2
    ADD R3          ; ACC = row * 2 again
    ADD R3          ; Oops, this is row * 3
    ; Actually, let's use a different approach - lookup table would be better
    ; For now, simplified calculation:

; Simplified: Use row directly (1-4) and column (0-3)
    LD R3           ; Load row (0-3)
    ADD R3          ; row * 2
    ADD R3          ; row * 3
    ADD R3          ; row * 4
    ADD R2          ; Add column
    XCH R4          ; Store key code in R4

; Display the key code
DISPLAY
    LD R4           ; Load key code
    WRR             ; Output to ROM port (LEDs show key number)

; Debounce delay loop
DEBOUNCE
    LDM $0F         ; Delay counter
    XCH R5          ; Store in R5
DELAY_LOOP
    NOP             ; Waste cycles
    NOP
    NOP
    NOP
    INC R5          ; Increment (will wrap at 16)
    JCN NZ, DELAY_LOOP  ; Loop until zero

; Wait for key release
WAIT_RELEASE
    LDM $00         ; Output all zeros (no column select)
    WRR
    RDR             ; Read all rows
    JCN NZ, WAIT_RELEASE  ; If any bit set, key still pressed

; Key released, restart scan
    JUN INIT

; =============================================================================
; Key Code Mapping:
; =============================================================================
; Key [1]: Row 0, Col 0 → Code 0
; Key [2]: Row 0, Col 1 → Code 1
; Key [3]: Row 0, Col 2 → Code 2
; Key [A]: Row 0, Col 3 → Code 3
; Key [4]: Row 1, Col 0 → Code 4
; Key [5]: Row 1, Col 1 → Code 5
; Key [6]: Row 1, Col 2 → Code 6
; Key [B]: Row 1, Col 3 → Code 7
; Key [7]: Row 2, Col 0 → Code 8
; Key [8]: Row 2, Col 1 → Code 9
; Key [9]: Row 2, Col 2 → Code 10 (0x0A)
; Key [C]: Row 2, Col 3 → Code 11 (0x0B)
; Key [*]: Row 3, Col 0 → Code 12 (0x0C)
; Key [0]: Row 3, Col 1 → Code 13 (0x0D)
; Key [#]: Row 3, Col 2 → Code 14 (0x0E)
; Key [D]: Row 3, Col 3 → Code 15 (0x0F)
;
; =============================================================================
; KBP Instruction Details:
; =============================================================================
; The KBP (Keyboard Process) instruction converts a single-bit pattern in the
; accumulator into a binary number representing the bit position:
;
;   Input (ACC)  →  Output (ACC)
;   0000 + CY=0  →  15 (no key, special case)
;   0000 + CY=1  →  0  (key on bit 0, after masking)
;   0001         →  1  (bit 0 set)
;   0010         →  2  (bit 1 set)
;   0100         →  3  (bit 2 set)
;   1000         →  4  (bit 3 set)
;   Other        →  15 (multiple bits or invalid)
;
; This is perfect for keyboard scanning where only one key should be pressed
; at a time in each column.
;
; =============================================================================
; Debouncing:
; =============================================================================
; Mechanical switches "bounce" - they make and break contact multiple times
; when pressed. This causes multiple readings. Solutions:
;
; 1. Software delay (this example): Wait after detection
; 2. Read twice: Compare two readings separated by delay
; 3. Hysteresis: Require stable signal for N consecutive reads
; 4. Hardware: RC circuit or dedicated debounce IC
;
; For production code, implement more robust debouncing!
; =============================================================================
