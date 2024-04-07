.target "6502"

.visibility public

VIC = $9000
VIC_CONTROL_1 = VIC
    VIC_HORIZONTAL_ORIGIN_MASK = $7f
    VIC_HORIZONTAL_ORIGIN(n) = n & VIC_HORIZONTAL_ORIGIN_MASK
    VIC_INTERLACE = $80
        VIC_INTERLACE_ON = VIC_INTERLACE
        VIC_INTERLACE_OFF = $00
VIC_VERTICAL_ORIGIN = VIC + $1
VIC_CONTROL_2 = VIC + $2
    VIC_SCREEN_ADDRESS_BIT8(n) = (n & $8000) >> 8
    VIC_COLUMNS_MASK = $7f
    VIC_COLUMNS(n) = n & VIC_COLUMNS_MASK
VIC_CONTROL_3 = VIC + $3
    VIC_CHARACTER_HEIGHT = $1
        VIC_CHARACTER_HEIGHT_16 = VIC_CHARACTER_HEIGHT
        VIC_CHARACTER_HEIGHT_8 = $0
    VIC_ROWS_MASK = $7e
    VIC_ROWS(n) = (n << 1) & VIC_ROWS_MASK
    VIC_RASTER_BIT_8 = $80
VIC_RASTER = VIC + $4
VIC_ADDRESS = VIC + $5
    VIC_ADDRESS(screen, charset) = ((screen & $7800) >> (11 - 4)) | ((charset & $f000) >> 12)
VIC_LIGHT_PEN_X = VIC + $6
VIC_LIGHT_PEN_Y = VIC + $7
VIC_POT_X = VIC + $8
VIC_POT_Y = VIC + $9
VIC_BASS = VIC + $a
  VIC_VOICE_FREQUENCY_MASK = $7f
  VIC_VOICE_FREQUENCY(n) = n & VIC_VOICE_FREQUENCY_MASK
  VIC_VOICE_ENABLE = $80
VIC_ALTO = VIC + $b
VIC_SOPRANO = VIC + $c
VIC_NOISE = VIC + $d
VIC_CONTROL_4 = VIC + $e
    VIC_LOUDNESS_MASK = $0f
    VIC_LOUDNESS(n) = n & VIC_LOUDNESS_MASK
    VIC_AUX_COLOR_MASK = $f0
    VIC_AUX_COLOR(n) = (n << 4) & VIC_AUX_COLOR_MASK
VIC_COLOR = VIC + $f
    VIC_BACKGROUND_COLOR_MASK = $7
    VIC_BACKGROUND_COLOR(n) = n & VIC_BACKGROUND_COLOR_MASK
    VIC_INVERSE = $8
        VIC_INVERSE_ON = $0
        VIC_INVERSE_OFF = VIC_INVERSE
    VIC_BORDER_COLOR_MASK = $f0
    VIC_BORDER_COLOR(n) = (n << 4) & VIC_BORDER_COLOR_MASK


VIA1 = $9110
VIA1_PRB = VIA1
VIA1_PRA = VIA1 + $1
VIA1_DDRB = VIA1 + $2
VIA1_DDRA = VIA1 + $3
VIA1_TIMER_1 = VIA1 + $4
VIA1_TIMER_1_LOAD = VIA1 + $6
VIA1_TIMER_2 = VIA1 + $8
VIA1_SHIFT = VIA1 + $a
VIA1_CONTROL_1 = VIA1 + $b ; LLMNNNOP
    VIA_PORT_A_LATCH = $1
    VIA_PORT_B_LATCH = $2
    VIA_SHIFT_REGISTER_MASK = $7
    VIA_SHIFT_REGISTER(n) = (n << 2) & VIA_SHIFT_REGISTER_MASK
    VIA_TIMER_2_MODE = $20
        VIA_TIMER_2_MODE_INTERVAL = $0
        VIA_TIMER_2_MODE_COUNT = VIA_TIMER_2_MODE
    VIA_TIMER_1_MODE = $40
        VIA_TIMER_1_MODE_INTERVAL = $0
        VIA_TIMER_1_MODE_COUNT = VIA_TIMER_1_MODE
    VIA_TIMER_1_PB7 = $80
VIA1_CONTROL_2 = VIA1 + $c ; QQQRSSST
    VIA_CA1 = $01
        VIA_CA1_NEGATIVE = $0
        VIA_CA1_POSITIVE = VIA_CA1
    VIA_CA2_MASK = $7
    VIA_CA2(n) = (n << 1) & VIA_CA2_MASK
    VIA_CB1 = $10
        VIA_CB1_NEGATIVE = $0
        VIA_CB1_POSITIVE = VIA_CB1
    VIA_CB2_MASK = $7
    VIA_CB2(n) = (n << 5) & VIA_CB2_MASK
VIA1_INTERRUPT_REQUEST = VIA1 + $d
  VIA_INTERRUPT_CA2 = $01
  VIA_INTERRUPT_CA1 = $02
  VIA_INTERRUPT_SHIFT = $04
  VIA_INTERRUPT_CB2 = $08
  VIA_INTERRUPT_CB1 = $10
  VIA_INTERRUPT_TIMER_2 = $20
  VIA_INTERRUPT_TIMER_1 = $40
  VIA_INTERRUPT_STATUS = $80
VIA1_INTERRUPT_MASK = VIA1 + $e
VIA1_PRA_RAW = VIA1 + $f


VIA2 = $9120
VIA2_PRB = VIA2
VIA2_PRA = VIA2 + $1
VIA2_DDRB = VIA2 + $2
VIA2_DDRA = VIA2 + $3
VIA2_TIMER_1 = VIA2 + $4
VIA2_TIMER_1_LOAD = VIA2 + $4
VIA2_TIMER_2 = VIA2 + $8
VIA2_SHIFT = VIA2 + $a
VIA2_CONTROL_1 = VIA2 + $b
VIA2_CONTROL_2 = VIA2 + $c
VIA2_INTERRUPT_REQUEST = VIA2 + $d
VIA2_INTERRUPT_MASK = VIA2 + $e
VIA2_PRA_RAW = VIA2 + $f
