.section basic

.local basic_start {
    .data $08, $0c, $e7, $07, $9e, $20, start / 1000 + $30 :1, start / 100 - (start / 1000) * 10 + $30 : 1, start / 10 - (start / 100) * 10 + $30 : 1, start - (start / 10) * 10 + $30 : 1, 0, 0, 0
}

.section data

.local message {
    .data $41, $53, $53, $45, $4d, $42, $4c, $45, $44, $20, $42, $59, $20, $41, $43, $43, $45, $4c, $45, $52, $41, $54, $45, $21, $00
}

