; String optimization routines
; Unrolled rep {op} routines.
; If adding new routines, create a new test to verify
; the generated code and the code added here.

; The rep {op} has a high start up cost in clock cycles.
; There are 3 things to consider.
; 1 - The start up cost of the rep {op}
; 2 - The cost of calling a routine to handle part or all
;     of the operation.
; 3 - The code bloat for unrolling some of the rep {op}
;
; Handle any lengths 16 and under inline for most operations
; Lengths greater than 32 (or some special cases), use the
;    rep {op}

; xr rdi - dest
; xl rsi - src
; wa rcx - byte count  will always be 16 or greater
; destroys w0 rax
; must mimic:
;   rep movsb
; -or- if called from mvw support
;   shr wa,3
;   rep movsq
min_memcpy:
    cmp     wa, 16
    jb      min_memcpy_repmov  ; Tiny copy -> Let hardware handle it

    ; If dest > Src and dest < src + count, let hardware handle it.
    mov     r10, xr
    sub     r10, xl
    cmp     r10, wa
    jb      min_memcpy_repmov

    cmp     wa, 32
    jb      min_memcpy_cpy16_31

min_memcpy_repmov:
    rep     movsb
    ret

min_memcpy_cpy16_31:
    vmovdqu xmm0, [xl]
    vmovdqu xmm1, [xl + wa - 16]
    vmovdqu [xr], xmm0
    vmovdqu [xr + wa - 16], xmm1
    add     xl, wa
    add     xr, wa
    xor     wa, wa
    ret

; ------------------------------------------
