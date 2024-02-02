.define CRT

.public CRT_CHIP_TYPE_ROM = 0
.public CRT_CHIP_TYPE_RAM = 1
.public CRT_CHIP_TYPE_FLASH = 2
.public CRT_CHIP_TYPE_EEPROM = 3

.pre_if .defined(C64)
.public CRT_TYPE_GENERIC = 0
.public CRT_TYPE_ACTION_REPLAY = 1
.public CRT_TYPE_KCS_POWER = 2
.public CRT_TYPE_FINAL_III = 3
.public CRT_TYPE_SIMONS_BASIC = 4
.public CRT_TYPE_OCEAN = 5
.public CRT_TYPE_EXPERT = 6
.public CRT_TYPE_FUNPLAY = 7
.public CRT_TYPE_SUPER_GAMES = 8
.public CRT_TYPE_ATOMIC_POWER = 9
.public CRT_TYPE_EPYX_FASTLOAD = 10
.public CRT_TYPE_WESTERMANN = 11
.public CRT_TYPE_REX = 12
.public CRT_TYPE_FINAL_I = 13
.public CRT_TYPE_MAGIC_FORMEL = 14
.public CRT_TYPE_GS = 15
.public CRT_TYPE_WARPSPEED = 16
.public CRT_TYPE_DINAMIC = 17
.public CRT_TYPE_ZAXXON = 18
.public CRT_TYPE_MAGIC_DESK = 19
.public CRT_TYPE_SUPER_SNAPSHOT_V5 = 20
.public CRT_TYPE_COMAL80 = 21
.public CRT_TYPE_STRUCTURED_BASIC = 22
.public CRT_TYPE_ROSS = 23
.public CRT_TYPE_DELA_EP64 = 24
.public CRT_TYPE_DELA_EP7x8 = 25
.public CRT_TYPE_DELA_EP256 = 26
.public CRT_TYPE_REX_EP256 = 27
.public CRT_TYPE_MIKRO_ASSEMBLER = 28
.public CRT_TYPE_FINAL_PLUS = 29
.public CRT_TYPE_ACTION_REPLAY4 = 30
.public CRT_TYPE_STARDOS = 31
.public CRT_TYPE_EASYFLASH = 32
.public CRT_TYPE_EASYFLASH_XBANK = 33
.public CRT_TYPE_CAPTURE = 34
.public CRT_TYPE_ACTION_REPLAY3 = 35
.public CRT_TYPE_RETRO_REPLAY = 36
.public CRT_TYPE_MMC64 = 37
.public CRT_TYPE_MMC_REPLAY = 38
.public CRT_TYPE_IDE64 = 39
.public CRT_TYPE_SUPER_SNAPSHOT = 40
.public CRT_TYPE_IEEE488 = 41
.public CRT_TYPE_GAME_KILLER = 42
.public CRT_TYPE_P64 = 43
.public CRT_TYPE_EXOS = 44
.public CRT_TYPE_FREEZE_FRAME = 45
.public CRT_TYPE_FREEZE_MACHINE = 46
.public CRT_TYPE_SNAPSHOT64 = 47
.public CRT_TYPE_SUPER_EXPLODE_V5 = 48
.public CRT_TYPE_MAGIC_VOICE = 49
.public CRT_TYPE_ACTION_REPLAY2 = 50
.public CRT_TYPE_MACH5 = 51
.public CRT_TYPE_DIASHOW_MAKER = 52
.public CRT_TYPE_PAGEFOX = 53
.public CRT_TYPE_KINGSOFT = 54
.public CRT_TYPE_SILVERROCK_128 = 55
.public CRT_TYPE_FORMEL64 = 56
.public CRT_TYPE_RGCD = 57
.public CRT_TYPE_RRNETMK3 = 58
.public CRT_TYPE_EASYCALC = 59
.public CRT_TYPE_GMOD2 = 60
.public CRT_TYPE_MAX_BASIC = 61
.public CRT_TYPE_GMOD3 = 62
.public CRT_TYPE_ZIPPCODE48 = 63
.public CRT_TYPE_BLACKBOX8 = 64
.public CRT_TYPE_BLACKBOX3 = 65
.public CRT_TYPE_BLACKBOX4 = 66
.public CRT_TYPE_REX_RAMFLOPPY = 67
.public CRT_TYPE_BISPLUS = 68
.public CRT_TYPE_SDBOX = 69
.public CRT_TYPE_MULTIMAX = 70
.public CRT_TYPE_BLACKBOX9 = 71
.public CRT_TYPE_LT_KERNAL = 72
.public CRT_TYPE_RAMLINK = 73
.public CRT_TYPE_DREAN = 74
.public CRT_TYPE_IEEEFLASH64 = 75
.public CRT_TYPE_TURTLE_GRAPHICS_II = 76
.public CRT_TYPE_FREEZE_FRAME_MK2 = 77

CRT_SIGNATURE = "C64 CARTRIDGE"
.pre_end

.macro crt_header type, exrom, game, name {
    .data CRT_SIGNATURE:ascii(16) ; signature
    .data $40:big_endian(4) ; header length
    .data $01, $00 ; version
    .data type:big_endian(2) ; cartridge type
    .data exrom:1 ; EXROM
    .data game:1 ; GAME
    .data $00, $00, $00, $00, $00, $00 ; TODO: ignored?! .fill(6, $00) ; reserved
    .data name:ascii(32) ; TODO: padded with NUL, not space
}

.macro crt_chip_header type, bank, address, size {
    .data "CHIP":ascii
    .data size+$10:big_endian(4) ; length of section
    .data type:big_endian(2) ; type ROM
    .data bank:big_endian(2) ; bank
    .data address:big_endian(2) ; load address
    .data size:big_endian(2) ; data size
}
