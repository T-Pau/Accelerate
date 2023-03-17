
START_PAGE = $10
END_PAGE = $80

.section bss

.local screen .reserve 2
.local address .reserve 2

.section code

.global start {
    ldx #00
    ldy #f80
    stx screen
    sty screen + 1
    ldy #$10
    stx address
    sty address + 1
    
    ldy #0    
    
loop:
    tya
    sta (address),y
    cmp (address),y
    beq ok
    jsr error
ok:
    iny
    beq loop
    inc address    
    lda address
    cmp #END_PAGE
    bcc loop
    lda #START_PAGE
    sta address
    bne loop
}

; 0123456789abcdef
; aaaa ss gg

.local error {
    sty should
    sta got
    lda address
    jsr output
    lda address + 1
    jsr output
    jsr skip
    lda should
    jsr output
    jsr skip
    lda got
    jsr output
    
    clc
    lda #9
    adc screen
    sta screen
    bcc end
    inc screen + 1
    lda screen + 1
    cmp #$fc
    bcc end
    lda #0
    sta screen
    lda #$f8
    sta screen + 1
end:
    rts    
}

.local output {
    sta tmp
    lsr
    lsr
    lsr
    jsr output_digit
    lda tmp
    and #$0f
    jmp output_digit
}

.local output_digit {
    tax
    lda hex_digit,x
    ldy #0
    sta (output),y
    jmp skip
}

.local skip {
    inc screen
    bne end
    inc screen + 1
end:    
    rts
}


.section data

.local hex_digit {
    .data $30, $31, $32, $33, $34, $35, $36, $37, $38, $39, $41, $42, $43, $44, $45, $46
}

.section bss

.local tmp .reserve 1
.local should .reserve 1
.local got .reserve 1