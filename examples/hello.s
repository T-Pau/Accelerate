.include "c64.inc"

.section code

.global start {
    lda #<message
    sta ptr
    lda #>message
    sta ptr + 1
    ldy #0
loop:
    lda (ptr),y
    beq end
    jsr CHROUT
    iny
    bne loop
end:
    beq end
}

.section data

.local message {
    .data $41, $53, $53, $45, $4d, $42, $4c, $45, $44, $20, $42, $59, $20, $41, $43, $43, $45, $4c, $45, $52, $41, $54, $45, $21, $00
}

.section zero_page

.local ptr .reserve 2
