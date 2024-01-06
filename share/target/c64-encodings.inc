.string_encoding screen {
    ranges [
        : $00 - $ff : $ee00
        : $00: "@" - "["
        : $01: "a" - "z"
        : $20: " " - "?"
    ]
    singletons {
        $1c: "£"
        $1d: "]"
        $1e: "↑"
        $1f: "←"
        $40: "━" "─"
        $41: "♠"
        $42: "|" "┃" "│"
        ; $43: "━" "─"
        $49: "╮"
        $4a: "╰"
        $4b: "╯"
        $4d: "╲"
        $4e: "╱"
        $51: "•" "●"
        $53: "♥" "❤"
        $55: "╭"
        $56: "╳"
        $57: "○" "◦" "◯"
        $58: "♣"
        $5a: "♦" "◆"
        $5b: "╋" "┼"
        $5e: "π"
        $5f: "◥"
        $61: "▌"
        $62: "▄"
        $63: "▔"
        $64: "_" "▁"
        $65: "▎"
        $66: "▒"
        $69: "◤"
        $6b: "┣" "├"
        $6c: "▗"
        $6d: "┗" "└"
        $6e: "┓" "┐"
        $6f: "▂"
        $70: "┏" "┌"
        $71: "┻" "┴"
        $72: "┳" "┬"
        $73: "┫" "┤"
        ; $74: "▎"
        $75: "▍"
        $79: "▃"
        $7b: "▖"
        $7c: "▝"
        $7d: "┛" "┘"
        $7e: "▘"
        $7f: "▚"

        $a0: "█"
        $d1: "◘"
        $d7: "◙"
        $df: "◣"
        $e1: "▐"
        $e2: "▀"
        $e3: "▇"
        $e7: "▊"
        $ec: "▛"
        $f6: "▋"
        $f7: "▆"
        $f8: "▅"
        $fb: "▜"
        $fc: "▙"
        $fe: "▟"
        $ff: "▞"
    }
    name_delimiters: "{" "}"
    named_ranges {
        "shift-" [
            : $40: "*"
            : $41: "a" - "z"
            : $5b: "+"
            : $5d: "-"
            : $60: " "
            : $69: "£"
            : $7a: "@"
        ]
        "cbm-" [
            : $5c: "-"
            : $5f: "*"
            : $61: "k"
            : $62: "i"
            : $63: "t"
            : $64: "@"
            : $65: "g"
            : $66: "+"
            : $67: "m"
            : $68: "£"
            : $6a: "n"
            : $6b: "q"
            : $6c: "d"
            : $6d: "z"
            : $6e: "s"
            : $6f: "p"
            : $70: "a"
            : $71: "e"
            : $72: "r"
            : $73: "w"
            : $74: "h"
            : $75: "j"
            : $76: "l"
            : $77: "y"
            : $78: "u"
            : $79: "o"
            : $7b: "f"
            : $7c: "c"
            : $7d: "x"
            : $7e: "v"
            : $7f: "b"
        ]
    }
}

.string_encoding screen_inverted {
    base {
        screen {
            use [
                : $00 - $7f: $80
            ]
        }
    }
}

.string_encoding petscii {
    base {
        screen {
            use [
                : $00 - $1f: $40
                : $20 - $3f
                : $40 - $5f: $60
                : $60 - $7f: $a0
            ]
        }
    }
    named {
        $03: "stop"
        $05: "white" "wht"
        $08: "up/lo lock on"
        $09: "up/lo lock off"
        $0d: "return"
        $0e: "lower case"
        $11: "down"
        $12: "reverse on" "rvs on"
        $13: "home"
        $14: "delete" "del"
        $1c: "red"
        $1d: "right"
        $1e: "green" "grn"
        $1f: "blue" "blu"
        $81: "orange"
        $83: "run"
        $85: "f1"
        $86: "f3"
        $87: "f5"
        $88: "f7"
        $89: "f2"
        $8a: "f4"
        $8b: "f6"
        $8c: "f8"
        $8d: "shift return"
        $8e: "upper case"
        $90: "black" "blk"
        $91: "up"
        $92: "reverse off" "rvs off"
        $93: "clear" "clr"
        $94: "insert" "inst"
        $95: "brown"
        $96: "light red"
        $97: "dark grey" "grey 1"
        $98: "grey" "medium grey" "grey 2"
        $99: "light green"
        $9a: "light blue"
        $9b: "light grey" "grey 3"
        $9c: "purple" "pur"
        $9d: "left"
        $9e: "yellow" "yel"
        $9f: "cyan" "cyn"
        $a0: "shift space"
    }
}

.default_string_encoding petscii
