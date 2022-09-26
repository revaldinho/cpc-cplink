	;; --------------------------------------------------------------
	;; fiforsx.s
	;; --------------------------------------------------------------
	;;
	;; fiforsx.s - an RSX library for the CPC-CPLink board
	;; Copyright (C) 2019,2022  Revaldinho
	;;
	;; This program is free software: you can redistribute it and/or modify it
        ;; under the terms of the GNU Lesser General Public License as published by the Free
        ;; Software Foundation, either version 3 of the License, or (at your option)
        ;; any later version.
        ;;
        ;; This program is distributed in the hope that it will be useful, but
        ;; WITHOUT ANY WARRANTY without even the implied warranty of MERCHANTABILITY
        ;; or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
        ;; for more details.
        ;;
        ;; You should have received a copy of the GNU Lesser General Public License along
        ;; with this program. If not, see <https://www.gnu.org/licenses/>.
        ;;
        ;; Notes:-
        ;;
        ;; 1. Generic parameter Handling for RSXes
        ;;  a      - number of parameters
        ;;  (ix)   - points to parameter table
        ;;    (ix+3) -> high byte of param n-1
        ;;    (ix+2) -> low byte of param n-1
        ;;    (ix+1) -> high byte of last param
        ;;    (ix)   -> low byte of last param
        ;;
        ;; 2. BASIC strings are stored as
        ;;   <addr> -> Handle:<Len byte, addr2> --> Bytes ....
        ;;
        ;; 3. To load RSXes in BASIC use:
        ;;  MEMORY &9C39: LOAD "FIFORSX.BIN", &9C40: CALL &9C40
        ;; OR
        ;;   Assemble to ROM (setenv ROMflag -DROM before running make)
        FIFO_STATUS     EQU     0xFD81
        FIFO_DATA       EQU     0xFD80
        FIFO_DIR        EQU     0x2
        FIFO_DOR        EQU     0x1
        KL_LOG_EXT      EQU     0xbcd1
        TXT_OUTPUT      EQU     0xbb5a
        VER             EQU     0x1
        SVER            EQU     0x0
        SSVER           EQU     0x0

ifdef ROM
        ORG             0xC000
FIFORSXSTART:
        DB              0x1     ; 1=background ROM
        DB              VER     ; Version.subversion.subsubversion
        DB              SVER
        DB              SSVER
else
        ORG             0x9800
FIFORSXSTART:
        ;; ------------------------------------------------------------------
        ;; install RSX
        LD      hl,work_space   ;address of a 4 byte workspace useable by Kernel
        LD      bc,jump_table   ;address of command name table and routine handlers
        JP      kl_log_ext      ;Install RSXes
WORK_SPACE:                     ;Space for kernel to use
        DS      4
endif
JUMP_TABLE:
        DW      name_table      ;address pointing to RSX commands
ifdef ROM
        JP      FIFOROMINIT
        JP      FIFODIR
        JP      FIFODOR
        JP      FIFOFLUSH
        JP      FIFOHELP
endif
        JP      ORIGIN
        JP      PLOT
        JP      VDU
        JP      FIFOGETC
        JP      FIFOGETS
        JP      FIFOINC
        JP      FIFOOUTC
        JP      FIFOPUTC
        JP      FIFOPUTS
        JP      FIFORST
NAME_TABLE:
        ;; NB the last letter of each RSX name must have bit 7 set to 1.
ifdef ROM
        DB      "CPC-CPLIN","K"+0x80 ; illegal name for init command so that it cant be called again
        DB      "FIFODI","R"+0x80
        DB      "FIFODO","R"+0x80
        DB      "FIFOFLUS","H"+0x80
        DB      "FIFOHEL","P"+0x80
endif
        DB      "ORIGI","N"+0x80
        DB      "PLO","T"+0x80
        DB      "VD","U"+0x80
        DB      "FIFOGET","C"+0x80
        DB      "FIFOGET","S"+0x80
        DB      "FIFOIN","C"+0x80
        DB      "FIFOOUT","C"+0x80
        DB      "FIFOPUT","C"+0x80
        DB      "FIFOPUT","S"+0x80
        DB      "FIFORS","T"+0x80

        DB      0 ;end of name table marker
	;; --------------------------------------------------------------
        ;; RSX: |FIFOPUTC, c
	;; --------------------------------------------------------------
        ;; Blocking write of character c to FIFO.
        ;;
        ;; Entry
        ;;   a    - num params
        ;;   (ix) - low byte of BASIC parameter c
        ;; Exit
        ;;  assume all registers corrupt
FIFOPUTC:
        CP      1 ; check parameter count and return if not 1
        JR      nz,PERROR
        LD      L,(IX)
        CALL    _FIFO_PUT_BYTE
        RET

        ;; --------------------------------------------------------------
        ;; RSX: |FIFOGETC, @c
	;; --------------------------------------------------------------
        ;; Blocking read of FIFO into BASIC parameter c
        ;;
        ;; Entry
        ;;   a      - num params
        ;;   (ix+1) - high byte of BASIC parameter c address
        ;;   (ix)   - low byte of BASIC parameter c address
        ;; Exit
        ;;   BASIC var c holds character read
        ;;   assume bc,af,hl all corrupt
FIFOGETC:
        CP      1       ; check parameter count and return if not 1
        JR      nz,PERROR
        CALL    _FIFO_GET_BYTE
        LD      A,L
        LD      H,(IX+1)
        LD      L,(IX)
        LD      (HL),A
        RET
        ;; --------------------------------------------------------------
        ;; RSX: |FIFOPUTS, @A$
	;; --------------------------------------------------------------
        ;; Blocking write of bytes from BASIC string A$ to FIFO
        ;;
        ;; Entry
        ;;  a - number of parameters
        ;;  ix - points to parameter table
        ;;  ix+1 -> high byte of a$ string location
        ;;  ix   -> low byte of a$ string location
        ;;
        ;; (ix) -> ( len, (addr)-> "BYTES")
        ;; Exit
        ;;  assume hl,de,af,bc all corrupt
FIFOPUTS:
        CP      1       ; check parameter count and return if not 1
        JR      nz,PERROR
        LD      h,(ix+1); hl points to string location: <len:addr>
        LD      l,(ix)
        LD      a,(hl)  ; Max bytes (0..255)
        AND     0xff    ; check for zero and exit early
        RET     Z
        INC     hl      ; skip over length byte
        LD      e,(hl)  ; get low byte of string bytes
        INC     hl
        LD      d,(hl)  ; get high byte of string bytes
        EX      de,hl   ; hl now points to string bytes area
        LD      e,a     ; e = max bytes
        LD      bc,FIFO_STATUS
FPS1:
        IN      a,(c)   ; get DIR status flag
        AND     FIFO_DIR
        JR      z,FPS1  ; wait for data ready
        DEC     c       ; point to data reg
        INC     b       ; pre-increment B
        OUTI            ; B--; (HL)<-IN(bc), HL++
        INC     c       ; point to status reg for next check
        DEC     e
        JR      nz,FPS1 ; if not loop again
        RET
PERROR: CALL    SPRINT
PMSG:   DB      "RSX PARAMETER ERROR",13,10,0
        RET
        ;; --------------------------------------------------------------
        ;; RSX: |FIFOGETS, @A$, n
	;; --------------------------------------------------------------
        ;; Blocking read of n bytes from FIFO into BASIC string A$
        ;;
        ;; NB a$ must exist and have enough space to store n characters.
        ;;
        ;; Entry
        ;;  a - number of parameters
        ;;  ix - points to parameter table
        ;;  ix+3 -> high byte of a$ string location
        ;;  ix+2 -> low byte of a$ string location
        ;;  ix+1 -> high byte of string length (ignored)
        ;;  ix   -> low byte of string length
        ;;
        ;; Exit
        ;;  a$ holds string of exactly n bytes
        ;;  assume hl,de,af,bc all corrupt
FIFOGETS:
        CP      2       ; check parameter count and return if not 2
        JR      nz,PERROR
        LD      a,(ix)  ; Max bytes (0..255)
        AND     0xff    ; check for zero and exit early
        RET     Z
        LD      h,(ix+3); hl points to string location: <len:addr>
        LD      l,(ix+2)
        LD      (HL),a  ; store length byte n in A$
        INC     hl      ; skip over length byte
        LD      e,(hl)  ; get low byte of string bytes
        INC     hl
        LD      d,(hl)  ; get hugh byte of string bytes
        EX      de,hl   ; hl now points to string bytes area
        LD      e,a     ; e = max bytes
        LD      bc,FIFO_STATUS
FGS1:
        IN      a,(c)   ; get DOR status flag
        RRA
        JR      nc,FGS1 ; wait for data
        DEC     c       ; point to data reg
        INI             ; (HL)<-IN(bc), HL++, B--
        INC     b       ; restore B
        INC     c       ; point to status reg for next check
        DEC     e
        JR      nz,FGS1 ; if not loop again
        RET
        ;; --------------------------------------------------------------
        ;; RSX: |FIFOOUTC, c%, @n%
        ;; RSX: |FIFOINC, @c%, @n%
        ;; --------------------------------------------------------------
	;; Non-Blocking write (OUTC) or read (INC) of integer c to/from FIFO.
	;;
	;; Entry:
	;;  a    -  number of parameters
	;;  ix+3 -> high byte of char to write/read (ignored)
	;;  ix+2 -> low byte of char to write/read
	;;  ix+1 -> high byte of address of n
	;;  ix   -> low byte of address of n
	;;
	;; Exit
	;;  BASIC var n holds 1 if successful or 0 if not
	;;  BASIC var c% holds character read for FIFOINC if successful
FIFOOUTC:
        CP      2 ; check parameter
        JR      nz,PERROR
        LD      bc,FIFO_STATUS
        IN      a,(c)
        AND     FIFO_DIR
        JR      z,FIOC
        DEC     c
        LD      a,(ix+2)
        OUT     (c),a
        JR      FIOC1
FIFOINC:
        CP      2 ; check parameter
        JR      nz,PERROR
        LD      bc,FIFO_STATUS
        IN      a,(c)
        AND     FIFO_DOR
        JR      z,FIOC
        DEC     c
        LD      h,(ix+3)
        LD      l,(ix+2)
        INI      ; (HL) <- IN (BC) ; HL++, B--
FIOC1:  LD      a,1 ; wrote one char
FIOC:   LD      h,(ix+1)
        LD      l,(ix)
        LD      (hl),a ; writes 0 if unsuccessful or 1 otherwise
        RET

	;; --------------------------------------------------------------
	;; RSX: |FIFORST
	;; --------------------------------------------------------------
	;;
	;; Reset the FIFO
	;;
	;; Entry
	;; - None
	;;
	;; Exit
	;; - af corrupt
FIFORST:
        ld   a,0xfd
        out  (0x81),a        	; write to status register resets FIFO
        ret
	;; --------------------------------------------------------------
	;; SPRINT - string print
	;; --------------------------------------------------------------
	;;
        ;; CALL sprint
        ;; DB "string", 0
	;;
	;; Entry
	;; - None
	;;
	;; Exit
	;; - HL,AF Corrupt
SPRINT:
        pop hl                  ;get string address
SPLOOP:
        ld a,(hl)               ; get next char
        or a
        jr z,SPEND              ; end of loop ?
        call TXT_OUTPUT
        inc hl
        jr SPLOOP
SPEND:  inc hl                  ; point to next addr after end of string
        jp (hl)                 ; and jump to it to return
     	;; --------------------------------------------------------------
	;; fifo_put_byte    (__z88dk_fastcall)
	;; --------------------------------------------------------------
	;;
	;; Wait until FIFO is ready to receive a byte, then write a single
        ;; byte
	;;
	;; Entry
	;; - L holds byte to be written
	;;
	;; Exit
	;; - AF, BC corrupt
_FIFO_PUT_BYTE::
        LD   BC, FIFO_STATUS
FPB_LOOP:
        IN   A,(C)         	; get dir status flag
        AND  0X2
        JR   Z,FPB_LOOP         ; loop again if not yet ready
        DEC C			; point to data reg
        OUT (C), L              ; write the byte
        RET
	;; --------------------------------------------------------------
	;; fifo_get_byte    (__z88dk_fastcall)
	;; --------------------------------------------------------------
	;;
	;; Wait until the FIFO has a byte to read and then read and return it.
	;;
	;; Entry
	;; - none
	;;
	;; Exit
	;; - L holds byte to be returned
	;; - AF, BC corrupt
_FIFO_GET_BYTE::
        LD   BC, FIFO_STATUS
FGB_LOOP:
        IN   A,(C)
        AND  0X1
        JR   Z,FGB_LOOP
        DEC  C
        IN   L,(C)
        RET
	;; --------------------------------------------------------------
	;; RSX: |VDU, N%, P1%, P2%, ..., Pn%
	;; RSX: |VDU25, K%, X%, Y%
	;; RSX: |VDU29, X%, Y%
	;; --------------------------------------------------------------
	;;
	;; Send a BBC Micro VDU command byte and parameters to the FIFO for
        ;; interpretation by an external BBC VDU processor. 16b values need
        ;; to have their byte order reversed
	;;
	;; Entry (Generic RSX handling)
        ;;     a     - number of parameters
        ;;     (ix)   - points to parameter table
        ;;     (ix+5) -> high byte of K%
        ;;     (ix+4) -> low byte of K%
        ;;     (ix+3) -> high byte of X%
        ;;     (ix+2) -> low byte of X%
        ;;     (ix+1) -> high byte of Y%
        ;;     (ix)   -> low byte of Y%
	;; - None
	;;
	;; Exit
	;; - All registers preserved

        ;; Expected Parameters per VDU command _not_ including the VDU number itself
VDUPARMS:
        DB      0,1,0,0,0,0,0,0
        DB      0,0,0,0,0,0,0,0
        DB      0,1,2,5,0,0,1,9
        DB      8,5,0,0,4,4,0,2

VDU:
        ;;  Bail out if no parameters at all, ie missing VDU command num!
        CP      0
        JP      Z, PERROR

        ;; Find VDU number, 1st RSX parameter at loc IX + (N-1)*2
        LD      E, A            ; save num RSX params in E
        PUSH    IX              ; Move IX into HL
        POP     HL
        DEC     A               ; A=N-1
        SLA     A               ; A=(N-1)*2
        LD      B,0             ; BC=(N-1)*2 for 16bit addition
        LD      C,A
        ADD     HL,BC           ; Pointer to 1st RSX param, N%, is HL+2*(N-1)
        LD      A, (HL)         ; Get N% (VDU number)

        ;; Special handling for VDU 25,29 (16 bit args) and VDU 127
        CP      25
        JR      NZ,VDU_L1       ; Not VDU25, move on
        LD      A,E             ; restore num params
        DEC     A               ; -1 to remove VDU number itself
        JP      VDU25           ; call VDU25
VDU_L1:
        CP      29
        JR      NZ,VDU_L2       ; Not VDU29, move on
        LD      A,E             ; restore num params
        DEC     A               ; -1 to remove VDU number itself
        JP      VDU29           ; call VDU29
VDU_L2:
        CP      127             ;
        JR      NZ,VDU_L3       ; Not VDU127, move on
        LD      L, A            ; write out code 127 and return
        CALL    _FIFO_PUT_BYTE
        RET
VDU_L3:
        CP      32
        JR      C,VDU_L4       ; reject all codes > 31
        CALL    SPRINT
        DB      "ERROR - Only VDU commands 0-31 and 127 are valid",13,10,0
        RET
VDU_L4:
        PUSH    HL              ; save pointer to 1st parameter
        ;;  Check VDU call parameters
        LD      B,0
        LD      C,A             ; C holds VDU num and is also offset to VDU param check table
        LD      HL,VDUPARMS
        ADD     HL,BC
        LD      A,(HL)          ; get required params for VDU command
        INC     A               ; add 1 to it to include the VDU number
        CP      E               ; compare with actual number of RSX params
        JP      Z, VDU_L5       ; continue if match
        POP     HL              ; else discard top of stack (pointer to 1st param)
        JP      PERROR          ; exit with error message
VDU_L5:
        POP     HL              ; restore HL as pointer to first param, E still holds num params >=1
VDU_L6:
        LD  	BC, FIFO_STATUS
        IN  	A,(C)
        AND  	FIFO_DIR
        JR  	Z,VDU_L6
        DEC 	C               ; Point to DATA reg
        LD  	A,(HL)          ; Get Param
        OUT 	(C),A           ; Write it
        DEC 	HL              ; Next Param
        DEC 	HL
        DEC 	E               ; dec counter
        JR  	NZ,VDU_L6       ; more params ?
        RET

PLOT:
VDU25:                          ; |PLOT (VDU25),K%,X%,Y%
        CP      3
        JP      NZ,PERROR
        LD      L,25            ; Command 25
        CALL    _FIFO_PUT_BYTE
        LD      L,(IX+4)        ; Low byte of K% (VDU25)
        JR      VDU_2INT_PARAMS
ORIGIN:
VDU29:                          ; |ORIGIN (VDU29), X%, Y%
        CP      2
        JP      NZ,PERROR
        LD      L,29            ; Command 29
VDU_2INT_PARAMS:
        CALL    _FIFO_PUT_BYTE
        LD      L,(IX+2)        ; Low byte of X%
        CALL    _FIFO_PUT_BYTE
        LD      L,(IX+3)        ; High byte of X%
        CALL    _FIFO_PUT_BYTE
        LD      L,(IX)          ; Low byte of Y%
        CALL    _FIFO_PUT_BYTE
        LD      L,(IX+1)        ; High byte of Y%
        CALL    _FIFO_PUT_BYTE
        RET


ifdef ROM
	; --------------------------------------------------------------
	; RSX: |FIFODOR, @f%
	; --------------------------------------------------------------
	; Return state of FIFO Data Output Ready flag in BASIC var f%
	;
	; Entry
	;   a      - num params
	;   (ix+1) - high byte of BASIC parameter c address
	;   (ix)   - low byte of BASIC parameter c address
	; Exit
	;   BASIC var f% holds value of DOR flag
	;   assume bc,af,hl all corrupt
FIFODOR:
	CP      1 ; check parameter count and return if not 1
	JP      nz,PERROR
	LD      BC,FIFO_STATUS
	IN      A,(C)
	JR      FIFODOIR
	; --------------------------------------------------------------
	; RSX: |FIFODIR, @f%
	; --------------------------------------------------------------
	; Return state of FIFO Data Input Ready flag in BASIC var f%
	;
	; Entry
	;   a      - num params
	;   (ix+1) - high byte of BASIC parameter c address
	;   (ix)   - low byte of BASIC parameter c address
	; Exit
	;   BASIC var f% holds value of DIR flag
	;   bc,af,hl all preserved via stack
FIFODIR:
	CP      1 ; check parameter count and return if not 1
	JP      nz,PERROR
	LD      BC,FIFO_STATUS
	IN      A,(C)
	RRA
FIFODOIR:
	AND     0x1
        LD      L,(IX)
        LD      H,(IX+1)
        LD      (HL),A
        INC     HL
        LD      (HL),0
	RET
	;; --------------------------------------------------------------
	;; RSX: |FIFOFLUSH
	;; --------------------------------------------------------------
	;; Reset the FIFO and flush any new input until no data remains
	;;
	;; Entry
        ;; - none
        ;; Exit
	;;  - assume bc,af,de, hl all corrupt
FIFOFLUSH:
FLOOP:
        CALL    FIFORST ; reset the FIFO (clear all current input/output)
        LD      BC,FIFO_STATUS
        IN      A,(C)
        AND     0x1 ; any input data ?
        JR      NZ, FLOOP ; new data so reset FIFO again
FLEXIT:
        RET
	;; --------------------------------------------------------------
	;; RSX: |FIFOHELP
	;; --------------------------------------------------------------
	;; Print HELP message
	;;
	;; Entry
        ;; - none
        ;; Exit
	;;  - assume bc,af,de, hl all corrupt
FIFOHELP:
        CALL SPRINT
        DB 4,2                  ; Change to Mode 2 for Help output
        DB 13,10,"CPC-CPLINK V",VER+'0',".",SVER+'0',SSVER+'0'," ",164," Revaldinho 2022",13,10
        DB 13,10,"|FIFODIR,@f%    - get FIFO data input ready flag in lsb of f%"
        DB 13,10,"|FIFODOR,@f%    - get FIFO data output ready flag in lsb of f%"
        DB 13,10,"|FIFOGETC,@a%   - get byte from FIFO into int a% (blocking)"
        DB 13,10,"|FIFOGETS,@a$,n%- get n% bytes from FIFO into string a$ (blocking)"
        DB 13,10,"|FIFOPUTC,a%    - write int a% to FIFO (blocking)"
        DB 13,10,"|FIFOPUTS,@a$   - write string a$ to FIFO (blocking)"
        DB 13,10,"|FIFOOUTC,a%,@f%- write int a% to FIFO, f%=1 if successful (non-blocking)"
        DB 13,10,"|FIFOINC,@a%,@f%- get int a% from FIFO, f%=1 if successful (non-blocking)"
        DB 13,10,"|FIFORST        - reset FIFO"
        DB 13,10,"|FIFOFLUSH      - reset FIFO and flush any new input data"
        DB 13,10,"|FIFOHELP       - show this help message"
        DB 13,10,"|VDU,N%,P%,..P% - Send a generic BBC VDU command to external GPU"
        DB 13,10,"|PLOT,K%,X%,Y%  - Send a BBC PLOT/VDU25 command to external GPU"
        DB 13,10,"|ORIGIN,X%,Y%   - Send a BBC ORIGIN/VDU29 command to external GPU"
        DB 13,10,0
        RET
FIFOROMINIT:
	;; --------------------------------------------------------------
	;; RSX: |FIFOROMINIT
	;; --------------------------------------------------------------
	;;
	;; initialise the FIFO ROM and print the banner on startup
	;;
	;; Entry
	;; - None
	;;
	;; Exit
	;; - AF Corrupt
        PUSH DE
        PUSH HL
        CALL SPRINT
        DB 13,10, " CPC-CPLINK V", VER+'0', '.',SVER+'0',SSVER+'0',13,10,10,0
        POP HL ;restore hl/de (optionally subtract any workspace area from hl before returning)
        POP DE
        SCF
        RET
endif

FIFORSXEND:
