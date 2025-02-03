.include "c64.inc"

.section zero_page {
    type: reserve_only
    address [
         : $02       ; Unused.
         : $19 - $21 ; Temporary string stack.
         : $26 - $2a ; Floating point result of multiplication or division.
         : $fb - $fe ; Unused.
    ]
}

.segment cartridge {
    ; type: read_only
    address: $8000 - $9fff
}

.section code {
    segment: cartridge
}

.section data {
    segment: cartridge
}

.section reserved {
    address: $0900 - $7fff
}

.include "commodore-cartridge.inc"
