.macro tap_file_program name, start, end, line_number {
    tap_file TAP_FILE_PROGRAM, name, start, end, line_number, end - start + 1
}

.macro tap_file_code name, start, end, address = .none {
    .if address == .none {
        tap_file TAP_FILE_CODE, name, start, end, start
    }
    .else {
        tap_file TAP_FILE_CODE, name, start, end, address
    }
}

TAP_FILE_PROGRAM = 0
TAP_FILE_NUMBER_ARRAY = 1
TAP_FILE_CHARACTER_ARRAY = 1
TAP_FILE_CODE = 3

.macro tap_file type, name, start, end, parameter_1, paramter_2 = 0 {
    ; header block
    .data $13:2 ; size
header_start:
    .data 0, type:1 ; block type, file type
    .data name:string(10) ; file name
    .data end - start + 1:2, parameter_1:2, parameter_2:2 ; length, parameters
header_end:
    ; .checksum "xor", header_start, header_end - 1
    .data $00 ; TODO

    ; data block
    .data end - start + 3:2; size
data_start:
    .data $ff ; block type
    .memory start, end
.data_end:
    ; .checksum "xor", data_start, data_end - 1
    .data $00 ; TODO: checksum
}

.macro tap_file_program name, start, end, line_number {
    tap_file TAP_FILE_PROGRAM, name, start, end, line_number, end
}

.macro tap_file_code name, start, end, address = .none {
    .if address == .none {
        tap_file TAP_FILE_CODE, name, start, end, start
    }
    .else {
        tap_file TAP_FILE_CODE, name, start, end, address
    }
}