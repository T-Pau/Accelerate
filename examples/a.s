.include "c64.inc"

PTR = $fb

.section code

.local TEST = $8000

.global start {
    lda #<message
    sta $fb
    lda #>message
    sta $fb + 1
    ldy #0
loop:
    lda (PTR),y
    beq end
    jsr CHROUT
    iny
    bne loop
end:
    beq end
}

.global test {
    lda #$00
    sta VIC + $20

    adc message + 2

    asl a
    lsr
    sei

loop:
    beq loop
}

.include "a2.s"

.section reserved

.local tmp .reserve 2
