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
;     along with macro spitbol.      if not, see <http://www.gnu.org/licenses/>.


;     ws is bits per word, cfp_b is bytes per word, cfp_c is characters per word
      BITS 64
      DEFAULT REL

      %define     cfp_b 8
      %define     cfp_c 8

      %include "int.h"

      global      reg_block

      global      reg_size

      global      minimal

      extern      calltab
      extern      stacksiz
      extern      lowsp
      extern      highsp
      extern      basemem

;     values below must agree with calltab defined in x64.hdr and also in osint/osint.h

minimal_relaj     equ   0
minimal_relcr     equ   1
minimal_reloc     equ   2
minimal_alloc     equ   3
minimal_alocs     equ   4
minimal_alost     equ   5
minimal_blkln     equ   6
minimal_insta     equ   7
minimal_rstrt     equ   8
minimal_start     equ   9
minimal_filnm     equ   10
minimal_dtype     equ   11
minimal_enevs     equ   12
minimal_engts     equ   13

%define globals 1             ;asm globals defined here


;     ---------------------------------------

;     this file contains the assembly language routines that interface
;     the macro spitbol compiler written in 80386 assembly language to its
;     operating system interface functions written in c.

;     contents:

;     o overview
;     o global variables accessed by osint functions
;     o interface routines between compiler and osint functions
;     o c callable function startup
;     o c callable function restart
;     o c callable function makeexec
;     o routines for minimal opcodes chk and cvd
;     o math functions for integer multiply, divide, and remainder
;     o math functions for real operation

;     overview

;     the macro spitbol compiler relies on a set of operating system
;     interface functions to provide all interaction with the host
;     operating system.  these functions are referred to as osint
;     functions.  a typical call to one of these osint functions takes
;     the following form in the 80386 version of the compiler:

;           ...code to put arguments in registers...
;           call  sysxx       # call osint function
;           d_word      exit_1            # address of exit point 1
;           d_word      exit_2            # address of exit point 2
;           ...   ...         # ...
;           d_word      exit_n            # address of exit point n
;           ...instruction following call...

;     the osint function 'sysxx' can then return in one of n+1 ways:
;     to one of the n exit points or to the instruction following the
;     last exit.  this is not really very complicated - the call places
;     the return address on the stack, so all the interface function has
;     to do is add the appropriate offset to the return address and then
;     pick up the exit address and jump to it or do a normal return via
;     an ret instruction.

;     unfortunately, a c function cannot handle this scheme.      so, an
;     intermediary set of routines have been established to allow the
;     interfacing of c functions.  the mechanism is as follows:

;     (1) the compiler calls osint functions as described above.

;     (2) a set of assembly language interface routines is established,
;         one per osint function, named accordingly.  each interface
;         routine ...

;         (a) saves all compiler registers in global variables
;           accessible by c functions
;         (b) calls the osint function written in c
;         (c) restores all compiler registers from the global variables
;         (d) inspects the osint function's return value to determine
;           which of the n+1 returns should be taken and does so

;     (3) a set of c language osint functions is established, one per
;         osint function, named differently than the interface routines.
;         each osint function can access compiler registers via global
;         variables.    no arguments are passed via the call.

;         when an osint function returns, it must return a value indicating
;         which of the n+1 exits should be taken.  these return values are
;         defined in header file 'inter.h'.

;     note:  in the actual implementation below, the saving and restoring
;     of registers is actually done in one common routine accessed by all
;     interface routines.

;     other notes:

;     some c ompilers transform "internal" global names to
;     "external" global names by adding a leading underscore at the front
;     of the internal name.  thus, the function name 'osopen' becomes
;     '_osopen'.  however, not all c compilers follow this convention.

;     global variables

      segment     .data

      align 8
;                          0x8000          0x1000           0x0800           0x0400        | 0x200               |  0x0100        | 0x0040
mxcsr_minimal:   dd  0x9fc0  ; Flush to zero | Precision mask | Underflow mask | Overflow mask | Divide by Zero mask | Denormal mask | Denormals are zero
;
mxcsr_osint:  dd   0           ; Preserved mxcsr (restore when returning to C -- except for math functions)

      align 8
;  constants

      global      ten
ten:  d_word      10              ; constant 10
; Real constants

      align 16
; double precision
;    0        1        2         3        4        5       6        7
; seeeeeee eeeeffff ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff
;
      global      zeron
zeron: dq         0x8000000000000000     ; Negative zero
      global      inf
inf:   dq         0x7ff0000000000000     ; double precision infinity
      global neg1f
neg1f: dq         0x7fffffffffffffff

      global      sav_block
;sav_block: times r_size db 0       ; save minimal registers during push/pop reg
sav_block: times 44 db 0            ; save minimal registers during push/pop reg

      align cfp_b
      global      ppoff
ppoff:      d_word          0             ; offset for ppm exits
      global      compsp
compsp: d_word        0             ; compiler's stack pointer
      global      lowspmin
lowspmin: d_word      0             ; Bottom of stack (r8 in minimal)
      global      lowspminx
lowspminx: d_word     0             ; Lowest stack used (r9 in minimal)

      global      osint_sp                   ; CCW not global
osint_sp:      d_word          0             ; osint's stack pointer

      align cfp_b

      global      minimal_id
minimal_id: d_word      0           ; id for call to minimal from c. see proc minimal below.

;
;     save and restore minimal and interface registers on stack.
;     used by any routine that needs to call back into the minimal
;     code in such a way that the minimal code might trigger another
;     sysxx call before returning.
;
;     note 1:      pushregs returns a collectable value in xl, safe
;     for subsequent call to memory allocation routine.
;
;     note 2:      these are not recursive routines.  only reg_xl is
;     saved on the stack, where it is accessible to the garbage
;     collector.  other registers are just moved to a temp area.
;
;     note 3:      popregs does not restore reg_cp, because it may have
;     been modified by the minimal routine called between pushregs
;     and popregs as a result of a garbage collection.  calling of
;     another sysxx routine in between is not a problem, because
;     cp will have been preserved by minimal.
;
;     note 4:      if there isn't a compiler stack yet, we don't bother
;     saving xl.  this only happens in call of nextef from sysxi when
;     reloading a save file.
;
;
      segment     .text

; ;
; ;     startup( char *dummy1, char *dummy2) - startup compiler
; ;
; ;     an osint c function calls startup to transfer control
; ;     to the compiler.
; ;
; ;     (rdi) = basemem
; ;     (rsi) = topmem - sizeof(word)
; ;
; ;   note: this function never returns.
; ;
;
      global      startup
;   ordinals for minimal calls from assembly language.

;   the order of entries here must correspond to the order of
;   calltab entries in the inter assembly language module.

calltab_relaj equ   0
calltab_relcr equ   1
calltab_reloc equ   2
calltab_alloc equ   3
calltab_alocs equ   4
calltab_alost equ   5
calltab_blkln equ   6
calltab_insta equ   7
calltab_rstrt equ   8
calltab_start equ   9
calltab_filnm equ   10
calltab_dtype equ   11
calltab_enevs equ   12
calltab_engts equ   13


; rdi points to a minimal_regs structure
startup:
      pop   rax               ; discard return
      cld                     ; default to up direction for string ops
      stmxcsr [mxcsr_osint]    ; Remember default mxcsr
; Set initial registers
      mov   r8, m_word [lowspmin]   ; internal registers
      mov   r9, m_word [lowspminx]  ; internal registers
      call  minimal           ; load regs, switch stack, start compiler

;     stackinit  -- initialize lowspmin from sp.

;     input:      sp - current c stack
;           stacksiz - size of desired minimal stack in bytes

;     uses: rax

;     output: register wa, sp, lowspmin, compsp, osint_sp set up per diagram:

;  C stack - (used when executing C code)
;  +--------------------+
;  |  C stack -         |
;  |    //////          |   o osint_sp set to RSP when entering minimal
;  |                    |   o RSP set to osint_sp when leaving minimal
;  +--------------------+
;
;  Minimal stack (static size, not relocated)
;  +--------------------+ <- highsp (stack goes down) <- initial compsp
;  |           ^        |   o compsp set to RSP when leaving minimal
;  |           |        |   o RSP set to compsp when entering minimal
;  / stacksiz bytes     /
;  |           |        | }-- lowspminx (lowest used stack)
;  |           |        |
;  |---------- | -------| <-- resultant lowspmin (stack overflow raised if RSP
;  | 400 guard v bytes  |                         less than lowspmin)
;  +--------------------+ <- lowsp
;
;  Minimal heap (dynamic size, relocatable)
;  +----------------------+ <-- maxmem (will expand until heap_alloced_size
;  |                    ^ |  ^          reaches heap_max)
;  |                    | |  |
;  / heap_alloced_size -| /  } - available for heap expansion without realloc
;  |                    | |  v
;  |--------------------|-| <- topmem (will grow to maxmem in heap_inc chunks)
;  / heap_inc           | /  ^
;  |--------------------|-|  } - Active minimal heap storage
;  / heap_initial_size  v /  v
;  +----------------------+ <- basemem  (address must be creater than maxlen)
;


;     mimimal -- call minimal function from c

;     usage:      extern void minimal(word callno)

;     where:
;       callno is an ordinal defined in osint.h, osint.inc, and calltab.

;     minimal registers wa, wb, wc, xr, and xl are loaded and
;     saved from/to the register block.

;     note that before restart is called, we do not yet have compiler
;     stack to switch to.  in that case, just make the call on the
;     the osint stack.

; rdi points to a minimal_regs structure
minimal:
      mov   r11,rsi                 ; Arg 2 - calltap entry
      mov   r10,rdi                 ; Arg 1 - minimal_regs
      push  rbx
      push  rbp
      push  r12
      push  r13
      push  r14
      push  r15
      push  r10                     ; Save for return
      mov   m_word [osint_sp],rsp      ; save osint stack pointer
; Load minimal registers from the passed parameter
      mov   r12, m_word [r10 + (0 * 8)]   ; IA
      movsd xmm12, m_real [r10 + (1 * 8)] ; RA
      mov   rdi, m_word [r10 + (2 * 8)]  ; XR
      mov   rsi, m_word [r10 + (3 * 8)]  ; XL
      mov   rcx, m_word [r10 + (4 * 8)]  ; WA
      mov   rbx, m_word [r10 + (5 * 8)]  ; WB
      mov   rdx, m_word [r10 + (6 * 8)]  ; WC
      mov   r13, m_word [r10 + (7 * 8)]  ; CP
;                m_word [r10 + (8 * 8)]  ; PC Not used in this context
      mov   rsp, m_word [r10 + (9 * 8)]  ; XS

      cmp   m_word [compsp],0       ; is there a compiler stack?
      je    min1                    ; jump if none yet
      mov   rsp,m_word [compsp]     ; switch to compiler stack
      mov   r8, m_word [lowspmin]
      mov   r9, m_word [lowspminx]
      ldmxcsr [mxcsr_minimal]

 min1:
      call   m_word [calltab+r11*cfp_b]    ; off to the minimal code
      ldmxcsr [mxcsr_osint]
; Return from Minimal
      mov   r11,rsp
      mov   rsp,m_word [osint_sp]      ; switch back to osint stack
; Update minimal_regs
      pop   r10
      mov   m_word [r10 + (0 * 8)], r12   ; IA
      movsd m_real [r10 + (1 * 8)], xmm12 ; RA
      mov   m_word [r10 + (2 * 8)], rdi  ; XR
      mov   m_word [r10 + (3 * 8)], rsi  ; XL
      mov   m_word [r10 + (4 * 8)], rcx  ; WA
      mov   m_word [r10 + (5 * 8)], rbx  ; WB
      mov   m_word [r10 + (6 * 8)], rdx  ; WC
      mov   m_word [r10 + (7 * 8)], r13  ; CP
;           m_word [r10 + (8 * 8)]  ; PC Not used in this context
      mov   m_word [r10 + (9 * 8)], r11  ; XS
      mov   m_word [lowspmin], r8
      mov   m_word [lowspminx], r9
      pop   r15
      pop   r14
      pop   r13
      pop   r12
      pop   rbp
      pop   rbx
      ret

      section           .data
      align       cfp_b
      global      cprtmsg
cprtmsg:
      db        ' copyright 1987-2012 robert b. k. dewar and mark emmer.',0,0


;     interface routines

;     each interface routine takes the following form:

;           sysxx call  ccaller ; call common interface
;                 d_word      zysxx ; dd    of c osint function
;                 db    n     ; offset to instruction after
;                             ;   last procedure exit

;     in an effort to achieve portability of c osint functions, we
;     do not take take advantage of any "internal" to "external"
;     transformation of names by c compilers.    so, a c osint function
;     representing sysxx is named _zysxx.  this renaming should satisfy
;     all c compilers.

;     important  one interface routine, sysfc, is passed arguments on
;     the stack.  these items are removed from the stack before calling
;     ccaller, as they are not needed by this implementation.

;     ccaller is called by the os interface routines to call the
;     real c os interface function.

;     general calling sequence is

;           call  ccaller
;           d_word      address_of_c_function
;           db    2*number_of_exit_points

;     control is never returned to a interface routine.  instead, control
;     is returned to the compiler (the caller of the interface routine).

;     the c function that is called must always return an integer
;     indicating the procedure exit to take or that a normal return
;     is to be performed.

;           c function  interpretation
;           return value
;           ------------      -------------------------------------------
;                <0           do normal return to instruction past
;                       last procedure exit (distance passed
;                       in by dummy routine and saved on stack)
;                 0           take procedure exit 1
;                 4           take procedure exit 2
;                 8           take procedure exit 3
;                ...    ...


      segment     .text

do_syscall:
;     save registers in global variables

      pop   rax                      ; Pop return address
      mov   r10,rsp                  ; Save
      mov   m_word [compsp],rsp
      mov   rsp,m_word [osint_sp]       ; Switch to C stack
      mov   rbp,rsp
      sub   rsp,(8*10)+8             ; space for 10 registers + alignment
      and   rsp,-16                  ; 16byte alignment
;     Now save the registers - register layout must match
;                              minimal_regs in osint.h
      mov     [rsp + (9 * 8)], r10        ; XS (Minimal Stack)
      mov     [rsp + (8 * 8)], rax        ; PC (Return Addr)
      mov     [rsp + (7 * 8)], r13        ; CP
      mov     [rsp + (6 * 8)], rdx        ; WC
      mov     [rsp + (5 * 8)], rbx        ; WB
      mov     [rsp + (4 * 8)], rcx        ; WA
      mov     [rsp + (3 * 8)], rsi        ; XL
      mov     [rsp + (2 * 8)], rdi        ; XR
      vmovq   [rsp + (1 * 8)], xmm12      ; RA (Direct Store!)
      mov     [rsp + (0 * 8)], r12        ; IA
; End of minimal_regs structure
      mov   rdi,rsp                  ; Set address of MINIMAL_REGS structure
      push  r8                       ; Save lowspmin
      push  r9                       ; and lowpminx
      push  rbp
      push  rax                      ; Keep proper stack alignment
; rsp at this point must be aligned on a 16 byte alignment +8
      mov   m_word [lowspmin],r8
      mov   m_word [lowspminx],r9
      ldmxcsr [mxcsr_osint]           ; Restore C rtl mxcsr register
      call  r15

syscall_exit:
      ldmxcsr [mxcsr_minimal]           ; restore minimal's mxcsr
      mov   r15,rax                 ; Set return code
      pop   rax                     ; throwaway - padding
      pop   rbp
      pop   r9                      ; lowspminx
      pop   r8                      ; lowspmin
      mov   r8, m_word [lowspmin]
      mov   r9, m_word [lowspminx]

      mov   r12, [rsp + (0 * 8)]      ; IA
      vmovq xmm12, [rsp + (1 * 8)]    ; RA (Direct Load)
      mov   rdi, [rsp + (2 * 8)]      ; XR
      mov   rsi, [rsp + (3 * 8)]      ; XL
      mov   rcx, [rsp + (4 * 8)]      ; WA
      mov   rbx, [rsp + (5 * 8)]      ; WB
      mov   rdx, [rsp + (6 * 8)]      ; WC
      mov   r13, [rsp + (7 * 8)]      ; CP
      mov   r14, [rsp + (8 * 8)]      ; PC (Return Addr)
      mov   r10, [rsp + (9 * 8)]      ; XS (Minimal Stack)

      mov   m_word [osint_sp],rbp      ; Save C stack
      mov   rsp,r10                 ; Back to minimal
      cld
      jmp   r14

      %macro      syscall     2
      global %1
      extern %2
%1:   lea   r15,%2
      jmp   do_syscall
      %endmacro

      syscall sysax,zysax     ; after execution, before printing stats/dump
      syscall sysbs,zysbs     ; backspace (previous record)
      syscall sysbx,zysbx     ; after compilation, before execution
      syscall sysdm,zysdm     ; dump(v)/&dump = v and v >= 4
      syscall sysdt,zysdt     ; obtain current date as a string (date() function)
      syscall sysea,zysea     ; error action
      syscall sysef,zysef     ; page eject on a file
      syscall sysej,zysej     ; end job - terminate run, set exit code
      syscall sysem,zysem     ; obtain text of err
      syscall sysen,zysen     ; endfile - close the file
      syscall sysep,zysep     ; page eject on standard printer output file (CCW - check ?? .lst)
      syscall sysex,zysex     ; call external function that was loaded via sysld
      syscall sysfc,zysfc     ; process arg2 and arg3 of input() and output() functions
      syscall sysgc,zysgc     ; called before and after garbage collection
      syscall syshs,zyshs     ; host() backend
      syscall sysid,zysid     ; additional info for standard printer output headers (CCW -- add git describe info)
      syscall sysif,zysif     ; include file processing
      syscall sysil,zysil     ; length of nfext input record
      syscall sysin,zysin     ; called after sysil to actually provide the data
      syscall sysio,zysio     ; follows sysfc call if input()/output() args1 and 2 are not null (opens file)
      syscall sysld,zysld     ; load() backend - load an external function
      syscall sysmm,zysmm     ; expand memory region
      syscall sysmx,zysmx     ; maximum object length (provides value from -m). value must be smaller than dnamb
      syscall sysou,zysou     ; write a record to file defined via sysio
      syscall syspi,zyspi     ; print to interactive terminal (terminal =)
      syscall syspl,zyspl     ; interrupt execution (CCW - check where this is called)
      syscall syspp,zyspp     ; obtain print format parameters
      syscall syspr,zyspr     ; print to standard output (output = )
      syscall sysrd,zysrd     ; read from standard input (x = input)
      syscall sysri,zysri     ; read from terminal - x = teriminal)
      syscall sysrw,zysrw     ; rewind file (close file and reopen on next read)
      syscall sysst,zysst     ; seek to position in file
      syscall systm,zystm     ; system execution time (in microseconds)
      syscall systt,zystt     ; called when trace() with no arguments is called
      syscall sysul,zysul     ; unload external function
      syscall sysxi,zysxi     ; exit to produce a save file

; math functions

      syscall  atn_,f_atn
      syscall  chp_,f_chp
      syscall  cos_,f_cos
      syscall  etx_,f_etx
      syscall  lnf_,f_lnf
      syscall  sin_,f_sin
      syscall  sqr_,f_sqr
      syscall  tan_,f_tan

      extern      rereloc

      global      restart
      extern      stbas
      extern      statb
      extern      stage
      extern      gbcnt
      extern      lmodstk
      extern      startbrk
      extern      outptr
      extern      swcoup
;     scstro is offset to start of string in scblk, or two words
scstro equ   cfp_c+cfp_c

;
restart:  ;; BROKEN ;;
      pop   rax                ; discard return
      pop   rax                     ; discard dummy
      pop   rax                     ; get lowest legal stack value

      add   rax,m_word [stacksiz]   ; top of compiler's stack
      mov   rsp,rax                       ; switch to this stack
;      call  stackinit         ; initialize minimal stack

                              ; set up for stack relocation
;      lea   rax,[tscblk+scstro]       ; top of saved stack
      mov   rbx,m_word [lmodstk]          ; bottom of saved stack
      mov   rcx,m_word [stbas]      ; rcx = stbas from exit() time
      sub   rbx,rax                       ; wb = size of saved stack
      mov   rdx,rcx
      sub   rdx,rbx                       ; rdx = stack bottom from exit() time
      mov   rbx,rcx
      sub   rbx,rsp                       ; rbx =      stbas - new stbas

      mov   m_word [stbas],rsp       ; save initial sp
;      getoff      rax,dffnc         ; get address of ppm offset
      mov   m_word [ppoff],rax       ; save for use later
;
;     restore stack from tscblk.
;
      mov   rsi,m_word [lmodstk]          ; -> bottom word of stack in tscblk
;      lea   rdi,[tscblk+scstro]            ; -> top word of stack
      cmp   rsi,rdi                       ; any stack to transfer?
      je    re3               ;  skip if not
      sub   rsi,4
      std
re1:  lodsd                   ; get old stack word to rax
      cmp   rax,rdx                       ; below old stack bottom?
      jb    re2               ;   j. if rax < rdx
      cmp   rax,rcx                       ; above old stack top?
      ja    re2               ;   j. if rax > rcx
      sub   rax,rbx                       ; within old stack, perform relocation
re2:  push  rax                     ; transfer word of stack
      cmp   rsi,rdi                       ; if not at end of relocation then
      jae   re1               ;    loop back

re3:  cld
      mov   m_word [compsp],rsp           ; save compiler's stack pointer
      mov   rsp,m_word [osint_sp]            ; back to osint's stack pointer
      and   rsp,0xfffffffffffffff0  ; 16byte alignment
      call   rereloc                ; relocate compiler pointers into stack
      mov   rax,m_word [statb]            ; start of static region to rdi
;      mov   m_word [reg_xr],rax
      mov   rax,minimal_insta
      jmp   minimal                 ; initialize static region
                              ; was a call, but there is nothing to return to.  This was probably for
                              ; debugging purposes.

;
;     now pretend that we're executing the following c statement from
;     function zysxi:
;
;           return      normal_return;
;
;     if the load module was invoked by exit(), the return path is
;     as follows:  back to ccaller, back to s$ext following sysxi call,
;     back to user program following exit() call.
;
;     alternately, the user specified -w as a command line option, and
;     sysbx called makeexec, which in turn called sysxi.  the return path
;     should be:  back to ccaller, back to makeexec following sysxi call,
;     back to sysbx, back to minimal code.  if we allowed this to happen,
;     then it would require that stacked return address to sysbx still be
;     valid, which may not be true if some of the c programs have changed
;     size.  instead, we clear the stack and execute the restart code that
;     simulates resumption just past the sysbx call in the minimal code.
;     we distinguish this case by noting the variable stage is 4.
;
      call   startbrk               ; start control-c logic

      mov   rax,m_word [stage]            ; is this a -w call?
      cmp   rax,4
      je          re4         ; yes, do a complete fudge

;
;     jump back with return value = normal_return
      xor   rax,rax                 ; set to zero to indicate normal return
      call  syscall_exit
      ret

;     here if -w produced load module.  simulate all the code that
;     would occur if we naively returned to sysbx.  clear the stack and
;     go for it.
;
re4: ;  mov   rax,m_word [stbas]
      mov   m_word [compsp],rax           ; empty the stack

;     code that would be executed if we had returned to makeexec:
;
      mov   m_word [gbcnt],0  ; reset garbage collect count
      call  zystm             ; fetch execution time to reg_ia
;      mov   rax,m_word [reg_ia]           ; set time into compiler
      extern      timsx
      mov   m_word [timsx],rax

;     code that would be executed if we returned to sysbx:
;
      push  m_word [outptr]         ; swcoup(outptr)
      extern      swcoup
      call  swcoup
      add   rsp,cfp_b

;     jump to minimal code to restart a save file.

      mov   rax,minimal_rstrt
      mov   m_word [minimal_id],rax
      call  minimal                 ; no return

section .note.GNU-stack noalloc noexec nowrite progbits
