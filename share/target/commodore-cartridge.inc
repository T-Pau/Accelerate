.section data

cartridge_header .address $8000 .used {
    .data start:2, nmi:2
    .data $c3, $c2, $cd, "80"
}
