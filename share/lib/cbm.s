; CBM Kernal Routines

.target "6502"

.visibility public

; TODO: different address for PET
IRQ_VECTOR = $0314
NMI_VECTOR = $0318

ACPTR = $ffa5
CHKIN = $ffc6
CHKOUT = $ffc9
CHRIN = $ffcf
CHROUT = $ffd2
CIOUT = $ffa8
CINT = $ff81
CLAA = $ffe7
CLOSE = $ffc3
CLRCHN = $ffcc
GETIN = $ffe4
IOBASE = $fff3
LISTEN = $ffb1
LOAD = $ffd5
MEMBOT = $ff9c
MEMTOP = $ff99
OPEN = $ffc0
PLOT = $fff0
RAMTAS = $ff87
RDTIM = $ffde
READST = $ffb7
RESTOR = $ff8a
SAVE = $ffd8
SCNKEY = $ff9f
SCREEN = $ffed
SECOND = $ff93
SETLFS = $ffba
SETMSG = $ff90
SETNAM = $ffbd
SETTIM = $ffdb
SETTMO = $ffa2
STOP = $ffe1
TALK = $ffb4
TKSA = $ff96
UDTIM = $ffea
UNLSN = $ffae
UNTLK = $ffab
VECTOR = $ff8d
