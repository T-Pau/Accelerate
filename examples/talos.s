.section code

.global instructions {
    nop

    ah = immediate
    al = immediate
    bh = immediate
    bl = immediate
    jah = immediate
    jal = immediate

    ar1++
    ar2++

    ah = (ar1)
    al = (ar1)
    bh = (ar1)
    bl = (ar1)
    jah = (ar1)
    jal = (ar1)

    ah = (ar2)
    al = (ar2)
    bh = (ar2)
    bl = (ar2)
    jah = (ar2)
    jal = (ar2)

    ah = (ar1), ar2++
    al = (ar1), ar2++
    bh = (ar1), ar2++
    bl = (ar1), ar2++
    jah = (ar1), ar2++
    jal = (ar1), ar2++

    ah = (ar2), ar1++
    al = (ar2), ar1++
    bh = (ar2), ar1++
    bl = (ar2), ar1++
    jah = (ar2), ar1++
    jal = (ar2), ar1++

    (ar1) = ah
    (ar1) = al
    (ar1) = bh
    (ar1) = bl
    (ar1) = rh
    (ar1) = rl

    (ar2) = ah
    (ar2) = al
    (ar2) = bh
    (ar2) = bl
    (ar2) = rh
    (ar2) = rl

    (ar1) = ah, ar2++
    (ar1) = al, ar2++
    (ar1) = bh, ar2++
    (ar1) = bl, ar2++
    (ar1) = rh, ar2++
    (ar1) = rl, ar2++

    (ar2) = ah, ar1++
    (ar2) = al, ar1++
    (ar2) = bh, ar1++
    (ar2) = bl, ar1++
    (ar2) = rh, ar1++
    (ar2) = rl, ar1++

    a = b
    a = r
    b = a
    b = r
    ar1 = a
    ar1 = b
    ar1 = r
    ar2 = a
    ar2 = b
    ar2 = r
    ja = a
    ja = b
    ja = r

    ah = bh
    ah = rh
    bh = ah
    bh = rh
    jah = ah
    jah = bh
    jah = rh

    al = bl
    al = rl
    bl = al
    bl = rl
    jal = al
    jal = bl
    jal = rl

    ar1 = a, ar2++
    ar1 = b, ar2++
    ar1 = r, ar2++

    ar2 = a, ar1++
    ar2 = b, ar1++
    ar2 = r, ar1++
    ja = a, ar1++
    ja = b, ar1++
    ja = r, ar1++

    ah = bh, ar1++
    ah = rh, ar1++
    bh = ah, ar1++
    bh = rh, ar1++
    jah = ah, ar1++
    jah = bh, ar1++
    jah = rh, ar1++

    al = bl, ar1++
    al = rl, ar1++
    bl = al, ar1++
    bl = rl, ar1++
    jal = al, ar1++
    jal = bl, ar1++
    jal = rl, ar1++

    ja = a, ar2++
    ja = b, ar2++
    ja = r, ar2++

    ah = bh, ar2++
    ah = rh, ar2++
    bh = ah, ar2++
    bh = rh, ar2++
    jah = ah, ar2++
    jah = bh, ar2++
    jah = rh, ar2++

    al = bl, ar2++
    al = rl, ar2++
    bl = al, ar2++
    bl = rl, ar2++
    jal = al, ar2++
    jal = bl, ar2++
    jal = rl, ar2++
}

.global pseudo_instructions {
    rh = ah + bh, rl = al + bl
    rh = ah + bh
    rl = al + bl
}
