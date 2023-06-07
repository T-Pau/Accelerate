Select section for following symbols. Remains valid until next `.section` directive:

```asm
.section code ; 
```

Define global constant. Visibility selector (`.global` or default `.local`) is only valid for this symbol.

```asm
.global VIC_BACKGROUD_COLOR = $d020 ; Define global constant
```

Define code object:

```asm
init {
    sei
    lda #$34
    sta $01
}
```