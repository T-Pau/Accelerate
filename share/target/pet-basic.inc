.include "pet.inc"

; TODO: check free adddress
.section zero_page {
    type: reserve_only
    address [
        : $0f - $10 ; unused
        : $a2       ; unused
        : $b6       ; unused
        : $b8       ; unused
        : $ed - $f7 ; unused
        : $ff       ; unused
    ]
}

basic_start = $0401
.default basic_end = $7fff

.include "commodore-basic.inc"
