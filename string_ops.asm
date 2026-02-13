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

; move character backwards
; xr rdi - dest (end of buffer)
; xl rsi - src  (end of buffer)
; wa rcx - byte count  will always be 16 or greater

min_mcb:
    cmp    wa, 256
    jbe    min_mcb_tail

min_mcb_loop:
    sub    xl, 256
    sub    xr, 256
    sub    wa, 256
    vmovdqu ymm0, [xl]
    vmovdqu ymm1, [xl+32]
    vmovdqu ymm2, [xl+64]
    vmovdqu ymm3, [xl+96]
    vmovdqu ymm4, [xl+128]
    vmovdqu ymm5, [xl+160]
    vmovdqu ymm6, [xl+192]
    vmovdqu ymm7, [xl+224]
    vmovdqu [xr], ymm0
    vmovdqu [xr+32], ymm1
    vmovdqu [xr+64], ymm2
    vmovdqu [xr+96], ymm3
    vmovdqu [xr+128], ymm4
    vmovdqu [xr+160], ymm5
    vmovdqu [xr+192], ymm6
    vmovdqu [xr+224], ymm7
    cmp    wa, 256
    ja     min_mcb_loop

min_mcb_tail:
    sub    xl, wa
    sub    xr, wa
    call   min_memcpy
    vzeroupper
    ret

; ------------------------------------------

; move word backwards
; xr rdi - dest (end of buffer)
; xl rsi - src  (end of buffer)
; wa rcx - byte count  will always be 16 or greater

min_mwb:
    cmp    wa, 256
    jbe    min_mwb_tail

min_mwb_loop:
    sub    xl, 256
    sub    xr, 256
    sub    wa, 256
    vmovdqu ymm0, [xl]
    vmovdqu ymm1, [xl+32]
    vmovdqu ymm2, [xl+64]
    vmovdqu ymm3, [xl+96]
    vmovdqu ymm4, [xl+128]
    vmovdqu ymm5, [xl+160]
    vmovdqu ymm6, [xl+192]
    vmovdqu ymm7, [xl+224]
    vmovdqu [xr], ymm0
    vmovdqu [xr+32], ymm1
    vmovdqu [xr+64], ymm2
    vmovdqu [xr+96], ymm3
    vmovdqu [xr+128], ymm4
    vmovdqu [xr+160], ymm5
    vmovdqu [xr+192], ymm6
    vmovdqu [xr+224], ymm7
    cmp    wa, 256
    jae    min_mwb_loop

min_mwb_tail:
    sub    xl, wa
    sub    xr, wa
    call   min_memcpy
    vzeroupper
    ret

; ------------------------------------------

; min_memfill
; xr rdi - dest
; w0 rax - fill (word)
; wa rcx - count
; matches 'rep stosq'

; min_memfill: Pattern Fill
min_memfill:
    test   wa, wa
    jz     min_memfill_done
    ; beyond 2KB (256 words), hardware rep usually beats unrolled loops.
    cmp    wa, 256
    jae    min_memfill_repstos

    shl    wa, 3
    ; --- prepare AVX2 pattern ---
    vmovq        xmm0, w0
    vpbroadcastq ymm0, xmm0     ; splat 64-bit pattern into all YMM slots

    cmp    wa, 32
    jb     min_memfill_tiny

    ; --- AVX2 loop (32 bytes per step) ---
min_memfill_loop32:
    vmovdqu [xr], ymm0
    add    xr, 32
    sub    wa, 32
    cmp    wa, 32
    jae    min_memfill_loop32
    ; check if we have an 8, 16, or 24 byte remainder
    test   wa, wa
    jz     min_memfill_cleanup

    vmovdqu [xr + wa - 32], ymm0

min_memfill_cleanup:
    add    xr, wa
    vzeroupper                  ; return to SSE state (important for haswell)
    xor    wa, wa
    ret

min_memfill_tiny:
    cmp    wa, 16
    jae    min_memfill_16_24
    mov    [xr], w0
    add    xr, 8
    xor    wa, wa
    ret

min_memfill_16_24:
    vmovdqu [xr], xmm0
    vmovdqu [xr + wa - 16], xmm0

min_memfill_finish:
    add     xr, wa
    xor     wa, wa
    ret

min_memfill_repstos:
    rep    stosq
    xor    wa, wa
min_memfill_done:
    ret

; ------------------------------------------

; lexically compare strings (cmc)
; xr rdi - str1
; xl rsi - str2
; wa rcx - count
; Sets flags
; xl and xr zeroed

min_memcmc:

    cmp     wa, 32
    ja      min_cmc_repcmp

    ; --- Medium Path (17-32 Bytes) ---
    ; We only need to check 2 QWORDS to cover up to 24 bytes,
    ; or 3 QWORDS to cover up to 32 bytes (Head, Middle, Tail).

    ; Check First 8
    mov     w0, [xl]
    mov     r10, [xr]
    bswap   w0
    bswap   r10
    cmp     w0, r10
    jne     min_cmc_return

    ; Check Next 8 (Offset 8)
    mov     w0, [xl + 8]
    mov     r10, [xr + 8]
    bswap   w0
    bswap   r10
    cmp     w0, r10
    jne     min_cmc_return

    ; Check Tail 8 (Overlap covers the end)
    ; This safely handles 17-32 bytes.
    mov     w0, [xl + wa - 16]
    mov     r10, [xr + wa - 16]
    bswap   w0
    bswap   r10
    cmp     w0, r10
    jne     min_cmc_return

    ; If we are really paranoid about 24-32 range coverage:
    ; 17 bytes: [0..7], [8..15], [9..16] (Tail overlap). Covered.
    ; 32 bytes: [0..7], [8..15], [24..31]. GAP at 16-23!
    ; We need one more check for full 32 byte coverage.

    ; Check Third 8 (Offset 16)
    ; Safe because len > 16 implies we have at least 17 bytes.
    ; If len < 24, this overlaps with tail. Safe.
    mov     w0, [xl + wa - 8]
    mov     r10, [xr + wa - 8]
    bswap   w0
    bswap   r10
    cmp     w0, r10
min_cmc_return:
    ret

min_cmc_repcmp:
    ; > 32 bytes: use hardware
    repe    cmpsb
    ret

; ------------------------------------------
