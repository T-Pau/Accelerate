# Notes

```asm
; definition:

; function
multiply(arg1, arg2 = 1) = arg1 * arg2

; macro
copy source, destination = $0400 {
    lda source
    sta destination
}

; object
start .address $1000 {
    jmp init
    jmp play
}

; reservation

charset .align $800 .reserve $400    


; usage:

init {
    sei
    copy $1000, multiply($1000, 2)
}
```

# Questions

- `.global` only valid for symbol on same line or for all following symbols (like `.section`)
- `.global` and `.local` or `.public` and `.private` (like C++)
- Require `.macro` prefix for definition? Makes it easier to differentiate from object, and removes ambiguatity for macros without arguments. 

# Documentation

Select section for following symbols. Remains valid until next `.section` directive:

```asm
.section code
```

Define global constant. Visibility selector (`.global` or default `.local`) is only valid for this symbol.

```asm
.global VIC_BACKGROUD_COLOR = $d020
```

Define code object:

```asm
init {
    sei
    lda #$34
    sta $01
    ldx #0
    txa
:   sta screen,x
    dex
    bne :-
    rts
}
```

Reserve memory:

```asm
screen .reserve $400 .align $400
```


## Macros

Define a macro:

```asm
inc16 address {
    inc address
    bne :+
    inc address + 1
:
}
```

Use a macro:

```asm
    lda (source),y
    sta (destination),y
    inc16 source
    inc16 destination
```

## Functions

Define a function:

```asm
vic_address(screen, charset) = ((screen & $3c00) >> 6) | ((charset & $3800) >> 10)
```

Use a function:

```asm
    lda vic_address($400, $2800)
    sta VIC_VIDEO_ADDRESS
```
