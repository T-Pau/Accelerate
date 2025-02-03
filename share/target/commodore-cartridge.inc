.fill_byte $ff

.section code

.default nmi {
    jmp $fe5e ; Kernal NMI routine when no cartridge is present
}

.section data

cartridge_header .address $8000 .used {
    .data start:2, nmi:2
    .data $c3, $c2, $cd, "80"
}
