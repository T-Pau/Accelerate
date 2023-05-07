.section code

.local branches {
    lda #$00
    bne branches

    lda wherever
foo:
    inx
    bne foo
}
