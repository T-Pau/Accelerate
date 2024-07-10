.section zero_page {
    type: reserve_only
    address [
         : $03 - $06 ; Storage for RENUMBER.
         : $19 - $21 ; Temporary string stack.
         : $26 - $2a ; Floating point result of multiplication or division.
         : $d8 - $e8 ; Unused.
    ]
}

basic_start = $1001

.include "commodore-basic.inc"
