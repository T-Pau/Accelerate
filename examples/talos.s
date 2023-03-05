.section code

.global test {
    nop
    ah = $55
    al = $aa
    ar1++
    ar2++
    ah = (ar1)
    al = (ar2)
    bh = (ar1), ar2++
    bl = (ar2), ar1++
    (ar1) = rl
}
