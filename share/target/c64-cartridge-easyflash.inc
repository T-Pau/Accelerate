.include "c64.inc"

.visibility public

EASYFLASH_BANK = $de00
  EASYFLASH_BANK_MAX = $3f
EASYFLASH_CONTROL = $de02
  EASYFLASH_CONTROL_LED = $80
  EASYFLASH_CONTROL_GAME_OVERRIDE = $04
  EASYFLASH_CONTROL_EXROM = $02
  EASYFLASH_CONTROL_GAME = $01

  EASYFLASH_CONTROL_OFF = $04
  EASYFLASH_CONTROL_ULTIMAX = $05
  EASYFLASH_CONTROL_8K = $06
  EASYFLASH_CONTROL_16K = $07

.visibility private

.fill_byte $ff

; TODO: support for multiple banks

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
    address [
        : $8000 - $9fff
        : $e000 - $ffff
    ]
}

.section code {
    segment: cartridge
}

.section data {
    segment: cartridge
}

.section code_low {
    address: $8000 - $9fff
}

.section data_low {
    address: $8000 - $9fff
}

.section reserved {
    address: $0900 - $7fff
}
