.string_encoding screen {
    ranges [
        : $20: " " - "~"
    ]
    singletons {
        $00: "£"
        $f2: "▗"
        $f3: "▖"
        $f4: "▝"
        $f5: "▘"
        $f6: "▚"
        $f7: "▞"
        $f8: "▼"
        $f9: "◀"
        $fa: "▶"
        $fb: "▲"
        $fc: "♥"
        $fd: "♦"
        $fe: "♠"
        $ff: "♣"
    }
}

.default_string_encoding "screen"
