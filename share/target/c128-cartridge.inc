.fill_byte $ff

.public CARTRIDGE_AUTOSTART_NONE = 0
.public CARTRIDGE_AUTOSTART_EARLY = 1
.public CARTRIDGE_AUTOSTART_BASIC = $ff

.default CARTRIDGE_AUTOSTART = CARTRIDGE_AUTOSTART_BASIC

.section data

cartridge_header .address $8000 .used {
    jmp start
    jmp start
    .data CARTRIDGE_AUTOSTART
    .data "cbm"
}
