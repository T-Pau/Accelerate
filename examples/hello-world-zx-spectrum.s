.section code

.public start {
    call CLS
    ld hl, string
:   ld a,(hl)
    cp a, 0
    ret z
    rst $10
    inc hl
    jr :-
}

.section data

string {
  .data "Hello World!{{enter}}", 0
}
