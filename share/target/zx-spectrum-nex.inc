.default .public NEX_BORDER_COLOR = 7
.default .public NEX_LOAD_BANK_DELAY = 0
.default .public NEX_LOAD_START_DELAY = 0

bank_start(bank) = bank * $4000
bank_end(bank) = bank * $4000 + $3fff
bank(address) = (address) >> 14

.macro bank bank, data_start, data_end {
    .if bank(data_start) <= bank && bank(data_end) >= bank {
        .memory bank_start(bank), bank_end(bank)
    }
}

.output {
    ; Header
    .data "NextV1.2" ; signature and version
    .data 0 ; 768k RAM is enough
    .data bank(.data_end) - bank(.data_start) + 1 ; number of 16k banks
    .data 0 ; TODO: loading screens
    .data NEX_BORDER_COLOR
    .data $ff58 ; stack pointer
    .data start:2 ; entry point
    .data 0:2 ; number of extra files
    .repeat j, 112 { ; included 16k banks
        .if bank(.data_start) <= j && bank(.data_end) >= j {
            .data 1
        }
        .else {
            .data 0
        }
    }
    .data 0, 0 ; loading bar enable and color
    .data NEX_LOAD_BANK_DELAY, NEX_LOAD_START_DELAY ; loading delays in frames
    .data 0 ; reset machine state
    .data 0, 0, 0 ; TODO: required core version
    .data 0 ; Timex Hires mode color
    .data 0 ; entry bank
    .data 0:2 ; close file
    .data .fill(370, 0) ; reserved
    ; TODO: loading screen
    bank 5, .data_start, .data_end
    bank 2, .data_start, .data_end
    bank 0, .data_start, .data_end
    bank 1, .data_start, .data_end
    bank 3, .data_start, .data_end
    bank 4, .data_start, .data_end
    ; TODO: banks 6 - 111
}
