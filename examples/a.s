.include "c64.inc"

.section code

.local TEST = $8000

.global test {
    lda #$00
    sta VIC + $20

    adc address + 2

    asl a
    lsr
    sei

loop:
    beq loop
}

.include "a2.s"

.section bss

.local tmp .reserve 2
