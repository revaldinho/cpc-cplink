	;; --------------------------------------------------------------
	;; fiforsx.s
	;; --------------------------------------------------------------
	;;
	;; fiforsx.s - an RSX library for the CPC-CPLink board
	;; Copyright (C) 2019  Revaldinho
	;;
	;; This program is free software: you can redistribute it and/or modify
	;; it under the terms of the GNU General Public License as published by
	;; the Free Software Foundation, either version 3 of the License, or
	;; (at your option) any later version.
	;;
	;; This program is distributed in the hope that it will be useful,
	;; but WITHOUT ANY WARRANTY; without even the implied warranty of
	;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	;; GNU General Public License for more details.
	;;
	;; You should have received a copy of the GNU General Public License
	;; along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
        ORG             0x9C40
        FIFO_STATUS     EQU     0xFD81
        FIFO_DATA       EQU     0xFD80
        FIFO_DIR        EQU     0x2
        FIFO_DOR        EQU     0x1
        KL_LOG_EXT      EQU     0xbcd1
        ;; ------------------------------------------------------------------
        ;; install RSX
        LD      hl,work_space   ;address of a 4 byte workspace useable by Kernel
        LD      bc,jump_table   ;address of command name table and routine handlers
        JP      kl_log_ext      ;Install RSX's
WORK_SPACE:                     ;Space for kernel to use
        DS      4
JUMP_TABLE:
        DW      name_table      ;address pointing to RSX commands
        JP      FIFOGETC
        JP      FIFOGETS
        JP      FIFOINC
        JP      FIFOOUTC
        JP      FIFOPUTC
        JP      FIFOPUTS
        JP      FIFORST
NAME_TABLE:
        ;; NB the last letter of each RSX name must have bit 7 set to 1.
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
        ;;  all registers preserved via stack
FIFOPUTC:
        CP      1 ; check parameter count and return if not 1
        JR      nz,PERROR         
        PUSH    AF
        PUSH    BC
        PUSH    DE
        PUSH    HL
        LD      bc,FIFO_STATUS
FPC1:
        IN      a,(c)
        AND     FIFO_DIR
        JR      z,FPC1
        DEC     c
        LD      a,(ix)
        OUT     (c),a
        JR      RESTOREANDRETURN

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
        ;;   bc,af,hl all preserved via stack
FIFOGETC:
        CP      1       ; check parameter count and return if not 1
        JR      nz,PERROR 
        PUSH    AF
        PUSH    BC
        PUSH    DE
        PUSH    HL

        LD      bc,FIFO_STATUS
FPC2:
        IN      a,(c)
        RRA
        JR      nc,FPC2
        DEC     c       ; point to FIFO DATA reg
        LD      l,(ix)
        LD      h,(ix+1)
        INI             ; (HL) <-IN(BC); HL++; B--
        JR      RESTOREANDRETURN
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
        ;;  hl,de,af,bc all preserved via stack
FIFOPUTS:
        CP      2       ; check parameter count and return if not 1
        JR      nz,PERROR
        PUSH    AF
        PUSH    BC
        PUSH    DE
        PUSH    HL
        LD      h,(ix+1); hl points to string location: <len:addr>
        LD      l,(ix)
        LD      a,(hl)  ; Max bytes (0..255)
        AND     0xff    ; check for zero and exit early
        JR      Z, RESTOREANDRETURN
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
RESTOREANDRETURN:
        POP     HL
        POP     DE
        POP     BC
        POP     AF
        RET
PMSG:   DB      "RSX PARAMETER ERROR",12,0 
PERROR: LD      HL,PMSG 
PLOOP:  LD      A,(HL) 
        OR      A ; check for zero byte
        RET     Z 
        CALL    0xBB5A 
        INC     HL 
        JR      PLOOP         
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
        ;;  hl,de,af,bc all preserved via stack
FIFOGETS:
        CP      2       ; check parameter count and return if not 2
        JR      nz,PERROR        
        PUSH    AF
        PUSH    BC
        PUSH    DE
        PUSH    HL
        LD      a,(ix)  ; Max bytes (0..255)
        AND     0xff    ; check for zero and exit early
        JR      Z,RESTOREANDRETURN
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
        JR      RESTOREANDRETURN
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
        ;;  All registers preserved via the stack
FIFOOUTC:            
        CP      2 ; check parameter
        JR      nz,PERROR
        PUSH    AF
        PUSH    BC
        PUSH    DE
        PUSH    HL
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
        PUSH    AF
        PUSH    BC
        PUSH    DE
        PUSH    HL
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
        JP      RESTOREANDRETURN        

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
	;; - AF Corrupt
FIFORST:
        ld   a,0xfd
        out  (0x81),a        	; write to status register resets FIFO
        ret
