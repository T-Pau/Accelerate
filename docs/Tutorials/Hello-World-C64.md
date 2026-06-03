---
layout: page
title: Hello World for the Commodore 64
---
Place the following examlpe in a file called **hello-world-c64.s** and compile with the command


```sh
xlr8 --target c64-basic hello-world-c64.s cbm.lib
```

<table class="code highlight">
<tr><td><code><span class="kd">.section</span> zero_page</code></td><td>; Place following objects into the <strong>zero page</strong>.</td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td><code><span class="vg">ptr</span> <span class="kd">.reserve</span> <span class="m">2</span></code></td><td>; Reserve a 2-byte variable called <code class="vg">ptr</code> in the zero page.</td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td><code><span class="kd">.section</span> code</code></td><td>; Place the follwing symbols in the <strong>code</strong> section, which for this target is in the BASIC program area.</td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td><code><span class="kd">.public</span> <span class="vg">start</span> {</code></td><td>; Begin the code object named <code class="vg">start</code>, which will be called when starting the resulting program.</td></tr>
<tr><td><code>    lda #&lt;<span class="vg">message</span></code></td><td>; Copy the address of the object <code class="vg">message</code> to the variable <code class="vg">ptr</code>.</td></tr>
<tr><td><code>    sta <span class="vg">ptr</span></code></td></tr>
<tr><td><code>    lda #><span class="vg">message</span></code></td></tr>
<tr><td><code>    sta <span class="vg">ptr</span> + <span class="m">1</span></code></td></tr>
<tr><td><code>    ldy #<span class="m">0</span></code></td><td>; Print the 0-terminated string at <code>message</code> to the screen using the kernal routine <code>CHROUT</code>.</td></tr>
<tr><td><code><span class="nl">loop</span>:</code></td></tr>
<tr><td><code>    lda (<span class="vg">ptr</span>),y</code></td></tr>
<tr><td><code>    beq <span class="nl">end</span></code></td></tr>
<tr><td><code>    jsr CHROUT</code></td></tr>
<tr><td><code>    iny</code></td></tr>
<tr><td><code>    bne <span class="nl">loop</span></code></td></tr>
<tr><td><code><span class="nl">end</span>:</code></td></tr>
<tr><td><code>    rts</code></td><td>; Return to BASIC.</td></tr>
<tr><td><code>}</code></td><td>; End the code object <code>start</code>.</td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td><code><span class="kd">.section</span> data</code></td><td>; Place the following symbols in the <strong>data</strong> section, which for this target is also in the BASIC program area.</td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td><code><span class="vg">message</span> {</code></td><td>; Create the object <code class="vg">message</code> containing the string to print, using the default stirng encoding <strong>PETSCII</strong>.</td></tr>
<tr><td><code>    .data <span class="s">"{clear}hello world!{return}"</span>, <span class="m">0</span></code></td><td>; The named character <code class="s">{clear}</code> is the control code to clear the screen.</td></tr>
<tr><td><code>}</code></td></tr>
</table>
