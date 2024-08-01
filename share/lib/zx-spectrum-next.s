.include "zx-spectrum.s"

.visibility public

NEXTREG_MACHINE_ID = $00
    NEXTREG_MACHINE_ID_EMULATOR = $08
    NEXTREG_MACHINE_ID_NEXT = $0a
    NEXTREG_MACHINE_ID_NEXT_ANTI_BRICK = $fa

NEXTREG_CORE_VERSION = $01
    NEXTREG_CORE_VERSION_MINOR_MASK = $0f
    NEXTREG_CORE_VERSION_MAJOR_MASK = $f0

NEXTREG_RESET = $02
    NEXTREG_RESET_SOFT = $01
    NEXTREG_RESET_HARD = $02
    NEXTREG_RESET_EXPANSION = $80

NEXTREG_MACHINE_TYPE = $03
    NEXTREG_MACHINE_TYPE_MASK = $07
        NEXTREG_MACHINE_TYPE_CONFIGURATION = $00
        NEXTREG_MACHINE_TYPE_48K = $01
        NEXTREG_MACHINE_TYPE_128K = $02
        NEXTREG_MACHINE_TYPE_PLUS3_NEXT = $03
        NEXTREG_MACHINE_TYPE_PENT = $04
    NEXTREG_MACHINE_TYPE_DISPLAY_TIMING_USER_LOCK = $08
    NEXTREG_MACHINE_TYPE_DISPLAY_TIMING_MASK = $70
    NEXTREG_MACHINE_TYPE_DISPLAY_TIMING(n) = (n & $7) << 4
        ; Uses the same values as NEXTREG_MACHINE_TYPE
    NEXTREG_MACHIEN_TYPE_DISPLAY_TIMING_CHANGE_ENABLE = $80

NEXTREG_CONFIGURATION_MAPPING = $04
    NEXTREG_CONFIGURATION_MAPPING_MASK = $1f

NEXTREG_PERIPHERAL_SETTINGS_1 = $05
    NEXTREG_PERIPHERAL_SETTINGS_1_SCANDOUBLER = $01
    NEXTREG_PERIPHERAL_SETTINGS_1_VERTICAL_FERQUENCY = $04
        NEXTREG_PERIPHERAL_SETTINGS_1_VERTICAL_FERQUENCY_50HZ = $0
        NEXTREG_PERIPHERAL_SETTINGS_1_VERTICAL_FERQUENCY_60HZ = NEXTREG_PERIPHERAL_SETTINGS_1_VERTICAL_FERQUENCY
    NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_1_MASK = $c8
    NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_2_MASK = $32
        NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_SINCLAIR_2 = $0
        NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_KEMPSTON_2 = $1
        NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_KEMPSTON_1 = $2
        NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_MEGADRIVE_1 = $3
        NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_CURSOR = $4
        NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_MEGADRIVE_2 = $5
        NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_SINCLAIR_1 = $6
        NEXTREG_PERIPHERAL_SETTINGS_1_JOYSTICK_KEYJOY = $7

NEXTREG_PERIPHERAL_SETTINGS_2 = $06
