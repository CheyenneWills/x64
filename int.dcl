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
;
    section .text

    extern  compsp
    extern  inf
    extern  neg1f
    extern  zeron
    extern  reg_cp ; code pointer (cp maintained in location reg_cp)

; Math functions
    extern atn_
    extern chp_
    extern cos_
    extern etx_
    extern lnf_
    extern sin_
    extern sqr_
    extern tan_

; Exported
    global  mxint
    global  start

;
;
;   table to recover type word from type ordinal
;

    section .data
    global  typet
typet:
    d_word  b_art   ; arblk type word - 0
    d_word  b_cdc   ; cdblk type word - 1
    d_word  b_exl   ; exblk type word - 2
    d_word  b_icl   ; icblk type word - 3
    d_word  b_nml   ; nmblk type word - 4
    d_word  p_aba   ; p0blk type word - 5
    d_word  p_alt   ; p1blk type word - 6
    d_word  p_any   ; p2blk type word - 7
; next needed only if support real arithmetic cnra
    d_word  b_rcl   ; rcblk type word - 8
    d_word  b_scl   ; scblk type word - 9
    d_word  b_sel   ; seblk type word - 10
    d_word  b_tbt   ; tbblk type word - 11
    d_word  b_vct   ; vcblk type word - 12
    d_word  b_xnt   ; xnblk type word - 13
    d_word  b_xrt   ; xrblk type word - 14
    d_word  b_bct   ; bcblk type word - 15
    d_word  b_pdt   ; pdblk type word - 16
    d_word  b_trt   ; trblk type word - 17
    d_word  b_bft   ; bfblk type word   18
    d_word  b_cct   ; ccblk type word - 19
    d_word  b_cmt   ; cmblk type word - 20
    d_word  b_ctt   ; ctblk type word - 21
    d_word  b_dfc   ; dfblk type word - 22
    d_word  b_efc   ; efblk type word - 23
    d_word  b_evt   ; evblk type word - 24
    d_word  b_ffc   ; ffblk type word - 25
    d_word  b_kvt   ; kvblk type word - 26
    d_word  b_pfc   ; pfblk type word - 27
    d_word  b_tet   ; teblk type word - 28
;
;   table of minimal entry points that can be dded from c
;   via the minimal function (see inter.asm).
;
;   note that the order of entries in this table must correspond
;   to the order of entries in the call enumeration in osint.h
;   and osint.inc.
;
    global calltab
calltab:
    d_word  relaj
    d_word  relcr
    d_word  reloc
    d_word  alloc
    d_word  alocs
    d_word  alost
    d_word  blkln
    d_word  insta
    d_word  rstrt
    d_word  start
    d_word  filnm
    d_word  dtype
;   d_word  enevs ;  engine words
;   d_word  engts ;   not used

    global  gbcnt
    global  headv
    global  mxlen
    global  stage
    global  timsx
    global  dnamb
    global  dnamp
    global  state
    global  b_efc
    global  b_icl
    global  b_scl
    global  b_vct
    global  b_xnt
    global  b_xrt
    global  stbas
    global  statb
;   global  polct   ; only if cpol is defined
    global  brint   ; only if cbrk is defined
    global  typet
    global  lowspmin
    global  flprt
    global  flptr
    global  gtcef
    global  hshtb
    global  pmhbs
    global  r_fcb
    global  c_aaa
    global  c_yyy
    global  g_aaa
    global  w_yyy
    global  s_aaa
    global  s_yyy
    global  r_cod
    global  kvstn
    global  kvdmp
    global  kvftr
    global  kvcom
    global  kvpfl
    global  cswfl
    global  stmcs
    global  stmct
    global  b_rcl
    global  end_min_data

;   integer arithmetic instructions


    section .data
    align   8
mxcsr       dd      0       ; used to test mxcsr exceptions
    section .text
; divide ia by r10 result in ia
; clobbers wc:w0
; set overflow
    align   16
do_dvi:
    test    r10,r10         ; check for divide by zero
    jz      do_dvi_over     ; set overflow
    mov     w0,ia
    cqo
    idiv    r10
    mov     ia,w0
    xor     w0_e,w0_e
    ret
do_dvi_over:
    xor     w0_e,w0_e
    cmp     al,-128
    ret

; remained of ia by r10 result in ia
; clobbers wc:w0
; set overflow
do_rmi:
    test    r10,r10         ; check for divide by zero
    jz      do_rmi_over     ; set overflow
    mov     w0,ia
    cqo
    idiv    r10
    mov     ia,wc
    xor     w0_e,w0_e
    ret
do_rmi_over:
    xor     w0_e,w0_e
    cmp     al,-128
    ret

%include "string_ops.asm"