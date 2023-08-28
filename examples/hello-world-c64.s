.target "c64-basic"

.section zero_page

ptr .reserve 2


.section code

.public start {
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
    rts
}


.section data

message {
    .data "{clear}hello world!{return}", 0
}
