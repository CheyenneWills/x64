; copyright 1987-2012 robert b. k. dewar and mark emmer.

; copyright 2012-2015 david shields
;
; this file is part of macro spitbol.
;
;     macro spitbol is free software: you can redistribute it and/or modify
;     it under the terms of the gnu general public license as published by
;     the free software foundation, either version 2 of the license, or
;     (at your option) any later version.
;
;     macro spitbol is distributed in the hope that it will be useful,
;     but without any warranty; without even the implied warranty of
;     merchantability or fitness for a particular purpose.  see the
;     gnu general public license for more details.
;
;     you should have received a copy of the gnu general public license
;     along with macro spitbol.  if not, see <http://www.gnu.org/licenses/>.

        BITS 64
        DEFAULT REL

        %define m_char  byte    ; reference to byte in memory
        %define d_char  db      ; define value of byte

        %define m_word  qword
        %define d_word  dq

        %define m_real  qword   ; reference to floating point value in memory
        %define d_real  dq      ; define value for floating point
        %define m_reall oword   ; 128-bit
; N INCLUDE
        %define xl      rsi
        %define xl_e    esi
        %define xr      rdi
        %define xr_e    edi
        %define xt      rsi
        %define xt_e    esi
        %define xs      rsp

        %define w0      rax
        %define w0_e    eax
        %define w0_s    ax
        %define w0_l    al

        %define wa      rcx
        %define wa_e    ecx
        %define wa_s    cx
        %define wa_l    cl

        %define wb      rbx
        %define wb_e    ebx
        %define wb_s    bx
        %define wb_l    bl

        %define wc      rdx
        %define wc_e    edx
        %define wc_s    dx
        %define wc_l    dl

        %define r7      r7
        %define r7_e    r7d
        %define r7_s    r7w
        %define r7_l    r7b
        %define r8      r8
        %define r8_e    r8d
        %define r8_s    r8w
        %define r8_l    r8b
        %define r9      r9
        %define r9_e    r9d
        %define r9_s    r9w
        %define r9_l    r9b
        %define r10     r10
        %define r10_e   r10d
        %define r10_s   r10w
        %define r10_l   r10b
        %define r11     r11
        %define r11_e   r11d
        %define r11_s   r11w
        %define r11_l   r11b
        %define r12     r12
        %define r12_e   r12d
        %define r12_s   r12w
        %define r12_l   r12b
        %define r13     r13
        %define r13_e   r13d
        %define r13_s   r13w
        %define r13_l   r13b
        %define r14     r14
        %define r14_e   r14d
        %define r14_s   r14w
        %define r14_l   r14b
        %define r15     r15
        %define r15_e   r15d
        %define r15_s   r15w
        %define r15_l   r15b

        %define ra      xmm12
        %define ia      r12
        %define ia_e    r12d
        %define ia_s    r12w
        %define ia_l    r12b

;       %define cfp_b   8

        %define log_cfp_b 3
        %define log_cfp_c 3
        %define d_real  dq
        %define cfp_c_val       8
        %define log_cfp_c 3
        %define cfp_m_  9223372036854775807
;       %define cfp_n_  64

        %define lods_w  lodsq
        %define lods_b  lodsb
        %define movs_b  movsb
        %define movs_w  movsq
        %define stos_b  stosb
        %define stos_w  stosq
        %define cmps_b  cmpsb

        %define cdq     cqo     ; sign extend (64 bits)

;       flags
        %define flag_of 0x80
        %define flag_cf 0x01
        %define flag_ca 0x40
