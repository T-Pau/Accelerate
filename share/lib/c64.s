.target "6502"

.visibility public

COLOR_BLACK = 0
COLOR_WHITE = 1
COLOR_RED = 2
COLOR_CYAN = 3
COLOR_PURPLE = 4
COLOR_GREEN = 5
COLOR_BLUE = 6
COLOR_YELLOW = 7
COLOR_ORANGE = 8
COLOR_BROWN = 9
COLOR_LIGHT_RED = 10
COLOR_GREY_1 = 11
COLOR_GREY_2 = 12
COLOR_LIGHT_GREEN = 13
COLOR_LIGHT_BLUE = 14
COLOR_GREY_3 = 15


VIC = $d000
VIC_SPRITE_0_X = VIC + $00
VIC_SPRITE_0_Y = VIC + $01
VIC_SPRITE_1_X = VIC + $02
VIC_SPRITE_1_Y = VIC + $03
VIC_SPRITE_2_X = VIC + $04
VIC_SPRITE_2_Y = VIC + $05
VIC_SPRITE_3_X = VIC + $06
VIC_SPRITE_3_Y = VIC + $07
VIC_SPRITE_4_X = VIC + $08
VIC_SPRITE_4_Y = VIC + $09
VIC_SPRITE_5_X = VIC + $0a
VIC_SPRITE_5_Y = VIC + $0b
VIC_SPRITE_6_X = VIC + $0c
VIC_SPRITE_6_Y = VIC + $0d
VIC_SPRITE_7_X = VIC + $0e
VIC_SPRITE_7_Y = VIC + $0f
VIC_SPRITE_X_MSB = VIC + $10
VIC_CONTROL_1 = VIC + $11
VIC_RASTER = VIC + $12
VIC_LIGHT_PEN_X = VIC + $13
VIC_LIGHT_PEN_Y = VIC + $14
VIC_SPRITE_ENABLE = VIC + $15
VIC_CONTROL_2 = VIC + $16
VIC_SPRITE_EXPAND_Y = VIC + $17
VIC_VIDEO_ADDRESS = VIC + $18
VIC_INTERRUPT_REQUEST = VIC + $19
VIC_INTERRUPT_MASK = VIC + $1a
VIC_SPRITE_PRIORITY = VIC + $1b
VIC_SPRITE_MULTICOLOR = VIC + $1c
VIC_SPRITE_EXPAND_X = VIC + $1d
VIC_SPRITE_SPRITE_COLLISION = VIC + $1e
VIC_SPRITE_BACKGROUND_COLLISION = VIC + $1f
VIC_BORDER_COLOR = VIC + $20
VIC_BACKGROUND_COLOR = VIC + $21
VIC_BACKGROUND_COLOR_1 = VIC + $22
VIC_BACKGROUND_COLOR_2 = VIC + $23
VIC_BACKGROUND_COLOR_3 = VIC + $24
VIC_SPRITE_MULTICOLOR_0 = VIC + $25
VIC_SPRITE_MULTICOLOR_1 = VIC + $26
VIC_SPRITE_0_COLOR = VIC + $27
VIC_SPRITE_1_COLOR = VIC + $28
VIC_SPRITE_2_COLOR = VIC + $29
VIC_SPRITE_3_COLOR = VIC + $2a
VIC_SPRITE_4_COLOR = VIC + $2b
VIC_SPRITE_5_COLOR = VIC + $2c
VIC_SPRITE_6_COLOR = VIC + $2d
VIC_SPRITE_7_COLOR = VIC + $2e

VIC_ADDRESS(screen, charset) = ((screen & $3c00) >> 6) | ((charset & $3800) >> 10)


SID = $d400
SID_VOICE1_FREQUENCY = SID + $00
SID_VOICE1_PULSE_WIDTH = SID + $02
SID_VOICE1_CONTROL = SID + $04
SID_VOICE1_ATTACK_DECAY = SID + $05
SID_VOICE1_SUSTAIN_RELEASE = SID + $06
SID_VOICE2_FREQUENCY = SID + $07
SID_VOICE2_PULSE_WIDTH = SID + $09
SID_VOICE2_CONTROL = SID + $0b
SID_VOICE2_ATTACK_DECAY = SID + $0c
SID_VOICE2_SUSTAIN_RELEASE = SID + $0d
SID_VOICE3_FREQUENCY = SID + $0e
SID_VOICE3_PULSE_WIDTH = SID + $10
SID_VOICE3_CONTROL = SID + $12
SID_VOICE3_ATTACK_DECAY = SID + $13
SID_VOICE3_SUSTAIN_RELEASE = SID + $14
SID_FILTER_CUTOFF = SID + $15
SID_FILTER_RESONANCE = SID + $17
SID_VOLUME_FILTER_MODE = SID + $18
SID_POT_X = SID + $19
SID_POT_Y = SID + $1a
SID_VOICE3_OSCILLATOR = SID + $1b
SID_VOICE3_ENVELOPE = SID + $1c


CIA1 = $dc00
CIA1_PRA = CIA1 + $00
CIA1_PRB = CIA1 + $01
CIA1_DDRA = CIA1 + $02
CIA1_DDRB = CIA1 + $03
CIA1_TIMER_A = CIA1 + $04
CIA1_TIMER_B = CIA1 + $06
CIA1_TOD_SUB_SECOND = CIA1 + $08
CIA1_TOD_SECONDS = CIA1 + $09
CIA1_TOD_MINUTES = CIA1 + $0a
CIA1_TOD_HOURS = CIA1 + $0b
CIA1_SERIAL_DATA = CIA1 + $0c
CIA1_INTERRUPT = CIA1 + $0d
CIA1_TIMER_A_CONTROL = CIA1 + $0e
CIA1_TIMER_B_CONTROL = CIA1 + $0f

CIA2 = $dd00
CIA2_PRA = CIA2 + $00
CIA2_PRB = CIA2 + $01
CIA2_DDRA = CIA2 + $02
CIA2_DDRB = CIA2 + $03
CIA2_TIMER_A = CIA2 + $04
CIA2_TIMER_B = CIA2 + $06
CIA2_TOD_SUB_SECOND = CIA2 + $08
CIA2_TOD_SECONDS = CIA2 + $09
CIA2_TOD_MINUTES = CIA2 + $0a
CIA2_TOD_HOURS = CIA2 + $0b
CIA2_SERIAL_DATA = CIA2 + $0c
CIA2_INTERRUPT = CIA2 + $0d
CIA2_TIMER_A_CONTROL = CIA2 + $0e
CIA2_TIMER_B_CONTROL = CIA2 + $0f


CINV = $3014 ; IRQ vector
