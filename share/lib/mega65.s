.define MEGA65

.include "c64.s"
.include "mega65-common.inc"

.macro set_vic_bank {
}

.macro set_vic_text screen, charset {
    lda #<screen
    sta VIC_SCREEN_POINTER
    lda #>screen
    sta VIC_SCREEN_POINTER + 1
    lda #<charset
    sta VIC_CHARSET_POINTER
    lda #>charset
    sta VIC_CHARSET_POINTER + 1
}

; TODO: Is this correct for VIC-IV?
.macro set_vic_text_mode {
    lda VIC_CONTROL_1
    and #$ff - VIC_MODE
    sta VIC_CONTROL_1
}
