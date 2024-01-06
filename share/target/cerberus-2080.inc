PROGRAM_START = $0202
PROGRAM_END = $efff

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
    segment: program
}

.pin start PROGRAM_START

.extension "bin"

.output {
    .memory .data_start, .data_end
}
