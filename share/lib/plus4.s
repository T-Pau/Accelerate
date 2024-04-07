.target "6502"

.visibility public

ACIA = $fd00
ACIA_DATA = ACIA
ACIA_STATUS = ACIA + $1
ACIA_COMMAND = ACIA + $2
ACIA_CONTROL = ACIA + $3

PIO_1 = $fd10
PIO_2 = $fd30

; 1551 as drive 9
TIA1 = $fec0
TIA1_DATA = TIA1
TIA1_STATUS = TIA1 + $1
TIA1_HANDSHAKE = TIA1 + $2
TIA1_DATA_DIRECTION = TIA1 + $3
TIA1_STATUS_DIRECTION = TIA1 + $4
TIA1_HANDSHAKE_DIRECTION = TIA1 + $5

; 1551 as drive 8
TIA2 = $fee0
TIA2_DATA = TIA2
TIA2_STATUS = TIA2 + $1
TIA2_HANDSHAKE = TIA2 + $2
TIA2_DATA_DIRECTION = TIA2 + $3
TIA2_STATUS_DIRECTION = TIA2 + $4
TIA2_HANDSHAKE_DIRECTION = TIA2 + $5

TED = $ff00
TED_TIMER1 = TED
TED_TIMER2 = TED + $2
TED_TIMER3 = TED + $4
TED_CONTROL_1 = TED + $6
    TED_SCROLL_MASK = $7
    TED_SCROLL(n) = (n & TED_SCROLL_MASK)
    TED_SCREEN_HEIGHT = $8
    TED_SCREEN_HEIGHT_24 = $0
    TED_SCREEN_HEIGHT_25 = TED_SCREEN_HEIGHT
    TED_SCREEN_ENABLE = $10
    TED_BITMAP = $20
    TED_EXTENDED_COLOR = $40
    TED_TEST = $80
TED_CONTROL_2 = TED + $7
    TED_SCREEN_WIDTH = $8
    TED_SCREEN_WIDTH_38 = $0
    TED_SCREEN_WIDTH_40 = TED_SCREEN_WIDTH
    TED_MULTICOLOR = $10
    TED_FREEZE = $20
    TED_TV_STANDARD = $40
    TED_TV_STANDARD_PAL = 0
    TED_TV_STANDARD_NTSC = TED_TV_STANDARD
    TED_CHARSET_MODE = $80
        TED_CHARSET_MODE_INVERTED = 0
        TED_CHARSET_MODE_FULL = TED_CHARSET_MODE
TED_KEYBOARD = TED + $8
TED_INTERUPT_REQUEST = TED + $9
    TED_INTERRUPT_RASTER = $2
    TED_INTERRUPT_LIGHT_PEN = $4
    TED_INTERRUPT_TIMER_1 = $8
    TED_INTERRUPT_TIMER_2 = $10
    TED_INTERRUPT_TIMER_3 = $40
    TED_INTERRUPT_INTERRUPT = $80
TED_INTERRUPT_MASK = TED + $0a
    TED_RASTER_BIT_8 = $1
TED_INTERRUPT_RASTER_LINE = TED + $0b
TED_CURSOR_HIGH = TED + $0c
TED_CURSOR_LOW = TED + $0d
TED_VOICE_1_FREQUENCY_LOW = TED + $0e
TED_VOICE_2_FREQUENCY = TED + $0f
TED_SOUND_CONTROL = TED + $11
    TED_VOLUME_MASK = $f
    TED_VOLUME(n) = (n & TED_VOLUME_MASK)
    TED_VOICE_1 = $10
    TED_VOICE_2_TONE = $20
    TED_VOICE_2_NOISE = $40
TED_CONTROL_3 = TED + $12
    TED_VOICE_1_FREQUENCY_HIGH_MASK = $3
    TED_VOICE_1_FREQUENCY_HIGH(n) = n & TED_VOICE_1_FREQUENCY_HIGH_MASK
    TED_CHARSET_SOURCE = $4
    TED_CHARSET_SOURCE_ROM = TED_CHARSET_SOURCE
    TED_CHARSET_SOURCE_RAM = 0
    TED_BITMAP_ADDRESS_MASK = $38
    TED_BITMAP_ADDRESS(n) = (n & $e000) >> 10
TED_CONTROL_4 = TED + $13
    TED_ROM_ENABLED = $1
    TED_SINGLE_CLOCK = $2
    TED_CHARACTER_ADDRESS_MASK = $fc
    TED_CHARACTER_ADDRESS(n) = (n & $fc00) >> 8
TED_SCREEN_ADDRESS = TED + $14
    TED_SCREEN_ADDRESS_MASK = $f8
    TED_SCREEN_ADDRESS(n) = (n & $f800) >> 8
TED_BACKGROUND_COLOR = TED + $15
TED_COLOR_1 = TED + $16
TED_COLOR_2 = TED + $17
TED_COLOR_3 = TED + $18
TED_BORDER_COLOR = TED + $19
TED_CURRENT_CHARACTER_HIGH = TED + $1a
TED_CURRENT_CHARACTER_LOW = TED + $1b
TED_CURRENT_RASTER_HIGH = TED + $1c
TED_CURRENT_RASTER_LOW = TED + $1d
TED_CURRENT_PIXEL = TED + $1e
TED_CONTROL_5 = TED + $1f
    TED_CHARACTER_LINE_MASK = $7
    TED_CHARACTER_LINE(n) = (n & TED_CHARACTER_LINE_MASK)
    TED_FLASH_MASK = $f
    TED_CONTROL_5_FLASH(n) = (n << 3) & TED_FLASH_MASK
TED_PAGE_ROM = TED + $3e
TED_PAGE_RAM = TED + $3f
