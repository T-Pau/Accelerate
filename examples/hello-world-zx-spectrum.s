.section code

.public start {
    ld bc, string
:   ld a, (bc)
    cp a, 0
    jr z, end
    rst $10
    inc bc
    jr :-

end:
  ret
}


.section data

string {
  .data "Hello World!{{enter}}", 0
}
