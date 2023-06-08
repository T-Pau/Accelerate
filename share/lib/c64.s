.target "6502"

.global COLOR_BLACK = 0
.global COLOR_WHITE = 1
.global COLOR_RED = 2
.global COLOR_CYAN = 3
.global COLOR_PURPLE = 4
.global COLOR_GREEN = 5
.global COLOR_BLUE = 6
.global COLOR_YELLOW = 7
.global COLOR_ORANGE = 8
.global COLOR_BROWN = 9
.global COLOR_LIGHT_RED = 10
.global COLOR_GREY_1 = 11
.global COLOR_GREY_2 = 12
.global COLOR_LIGHT_GREEN = 13
.global COLOR_LIGHT_BLUE = 14
.global COLOR_GREY_3 = 15

.global VIC = $d000
.global VIC_SPRITE_0_X = VIC + $00
.global VIC_SPRITE_0_Y = VIC + $01
.global VIC_SPRITE_1_X = VIC + $02
.global VIC_SPRITE_1_Y = VIC + $03
.global VIC_SPRITE_2_X = VIC + $04
.global VIC_SPRITE_2_Y = VIC + $05
.global VIC_SPRITE_3_X = VIC + $06
.global VIC_SPRITE_3_Y = VIC + $07
.global VIC_SPRITE_4_X = VIC + $08
.global VIC_SPRITE_4_Y = VIC + $09
.global VIC_SPRITE_5_X = VIC + $0a
.global VIC_SPRITE_5_Y = VIC + $0b
.global VIC_SPRITE_6_X = VIC + $0c
.global VIC_SPRITE_6_Y = VIC + $0d
.global VIC_SPRITE_7_X = VIC + $0e
.global VIC_SPRITE_7_Y = VIC + $0f
.global VIC_SPRITE_X_MSB = VIC + $10
.global VIC_CONTROL_1 = VIC + $11
.global VIC_RASTER = VIC + $12
.global VIC_LIGHTPEN_X = VIC + $13
.global VIC_LIGHTPEN_Y = VIC + $14
.global VIC_SPRITE_ENABLE = VIC + $15
.global VIC_CONTROL_2 = VIC + $16
.global VIC_SPRITE_EXPAND_Y = VIC + $17
.global VIC_VIDEO_ADDRESS = VIC + $18
.global VIC_INTERRUPT_REQUEST = VIC + $19
.global VIC_INTERRUPT_MASK = VIC + $1a
.global VIC_SPRITE_PRIORITY = VIC + $1b
.global VIC_SPRITE_MULTICOLOR = VIC + $1c
.global VIC_SPRITE_EXPAND_X = VIC + $1d
.global VIC_SPRITE_SPRITE_COLLISION = VIC + $1e
.global VIC_SPRITE_BACKGROUND_COLLISION = VIC + $1f
.global VIC_BORDER_COLOR = VIC + $20
.global VIC_BACKGROUND_COLOR = VIC + $21
.global VIC_BACKGROUND_COLOR_1 = VIC + $22
.global VIC_BACKGROUND_COLOR_2 = VIC + $23
.global VIC_BACKGROUND_COLOR_3 = VIC + $24
.global VIC_SPRITE_MULTICOLOR_0 = VIC + $25
.global VIC_SPRITE_MULTICOLOR_1 = VIC + $26
.global VIC_SPRITE_0_COLOR = VIC + $27
.global VIC_SPRITE_1_COLOR = VIC + $28
.global VIC_SPRITE_2_COLOR = VIC + $29
.global VIC_SPRITE_3_COLOR = VIC + $2a
.global VIC_SPRITE_4_COLOR = VIC + $2b
.global VIC_SPRITE_5_COLOR = VIC + $2c
.global VIC_SPRITE_6_COLOR = VIC + $2d
.global VIC_SPRITE_7_COLOR = VIC + $2e

.global CIA1 = $dc00
.global CIA1_PRA = CIA1 + $00
.global CIA1_PRB = CIA1 + $01
.global CIA1_DDRA = CIA1 + $02
.global CIA1_DDRB = CIA1 + $03
.global CIA1_TIMER_A = CIA1 + $04
.global CIA1_TIMER_B = CIA1 + $06
.global CIA1_TOD_SUB_SECOND = CIA1 + $07
.global CIA1_TOD_SECONDS = CIA1 + $08
.global CIA1_TOD_MINUTES = CIA1 + $09
.global CIA1_TOD_HOURS = CIA1 + $0a
; ...
.global CIA1_TIMER_A_CONTROL = CIA1 + $0e
; ...

.global CIA2 = $dd00
.global CIA2_PRA = CIA2 + $00
.global CIA2_PRB = CIA2 + $01
.global CIA2_DDRA = CIA2 + $02
.global CIA2_DDRB = CIA2 + $03
.global CIA2_TIMER_A = CIA2 + $04
.global CIA2_TIMER_B = CIA2 + $06
.global CIA2_TOD_SUB_SECOND = CIA2 + $07
.global CIA2_TOD_SECONDS = CIA2 + $08
.global CIA2_TOD_MINUTES = CIA2 + $09
.global CIA2_TOD_HOURS = CIA2 + $0a


.global CINV = $3014 ; IRQ vector
