.section code

.global test {
    (ar1) = rl
    (ar2) = rl
    bh = (ar1), ar2++
    ah = (ar1)
    nop
    jp
    jim
    ah = $55
    al = $aa
    ar1++
    ar2++
    al = (ar2)
    bl = (ar2), ar1++
}
