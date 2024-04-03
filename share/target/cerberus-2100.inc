PROGRAM_START = $0205
PROGRAM_END = $effd

.include "ascii-encoding.inc"

.segment program {
    address: PROGRAM_START - PROGRAM_END
}

.section code {
    segment: program
}

.section data {
    segment: program
}

.section reserved {
    type: reserve_only
    address [
        : PROGRAM_START - PROGRAM_END
        : $fcb1 - $fff9
    ]
}

.pin start PROGRAM_START

.extension "bin"

.output {
    .memory .data_start, .data_end
}
