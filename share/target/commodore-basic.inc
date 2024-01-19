.segment program {
    address: basic_start - basic_end
}

.section code {
    segment: program
}

.section data {
    segment: program
}

.section reserved {
    type: reserve_only
    segment: program
}

.default .macro extra_setup {
}

.section data

basic_program .address basic_start .used {
    .data next_line, 0:2
    extra_setup
    .data $9e, start:string(5), 0  ; BASIC: SYS start
next_line:
    .data $0000 ; end of BASIC program
}

.include "commodore-prg.inc"
