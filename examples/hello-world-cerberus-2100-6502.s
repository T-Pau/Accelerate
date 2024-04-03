SCREEN = $f800

.section zero_page

ptr .reserve 2


.section code

.public start {
    lda #<message
    sta ptr
    lda #>message
    sta ptr + 1
    ldy 0
:   lda (ptr),y
    beq end
    sta SCREEN,y
    iny
    bne :-
end:
    jmp end
}


.section data

message {
    .data "Hello world!", 0
}
