	;; --------------------------------------------------------------
	;; mcplib.s
	;; --------------------------------------------------------------
	;;
	;; mcplib.s - an asm library for talking to mcp_pi via the CPC-CPLink board
	;; Copyright (C) 2019  Revaldinho && Shifters74
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

        org #8000
        ;;FAKE EQU 0x1                  ; For DEV testing on PC only - SHOULD BE COMMENTED OUT
        PRINT_BIN EQU 0x1               ; Comment out to save memory but no printing for
                                        ; BinaryCommand or BinaryData in hex
                                        ; TextCommand printing still works though for ASCII
        PACKET_OVERHEAD EQU 0x3         ; the over head of the packet header
        DELIMITER_SIZE  EQU 0x3         ; the size of the delimiter at either end of packet
        FRONT_PACKET_OVERHEAD EQU 0x6   ; size of delimiter and packet over head
        WHOLE_PACKET_OVERHEAD EQU 0x9   ; the overhead of the delimiters and front packet over head
        TEXTCOMMAND     EQU 0x1         ; define packet types constants
        BINARYCOMMAND   EQU 0x2
        BINARYDATA      EQU 0x3
        FW_PrintChar    EQU 0xBB5A

        FIFO_STATUS     EQU     0xFD81
        FIFO_DATA       EQU     0xFD80
        FIFO_DIR        EQU     0x2
        FIFO_DOR        EQU     0x1
        KL_LOG_EXT      EQU     0xbcd1

        ;; ------------------------------------------------------------------
        ;; install RSX
        ;; ------------------------------------------------------------------
        LD      hl,WORK_SPACE   ;address of a 4 byte workspace useable by Kernel
        LD      bc,JUMP_TABLE   ;address of command name table and routine handlers
        JP      KL_LOG_EXT      ;Install RSX's
WORK_SPACE:                     ;Space for kernel to use
        DS      4
JUMP_TABLE:
        DW      NAME_TABLE                   ; address pointing to RSX commands
        jp      FIFORST                   ; (was &8000) Initialise the cplink interface
        jp      SendTextCommand              ; (was &8003) send a text command (wrapped in a packet) via cplink to the pi
        jp      ReceiveTextCommandResponse   ; (was &8006) receive a response from a text command (wrapped in a packet) via cplink from the pi
        jp      SendBinaryCommand            ; (was &8009) send a binary command (wrapped in a packet) via cplink to the pi
        jp      ReceiveBinaryCommandResponse ; (was &800C) receive a response from a binary command (wrapped in a packet) via cplink from the pi
        jp      SendBinaryData               ; (was &800F) send a BinaryData payload (wrapped in a packet) via cplink to the pi
        jp      ReceiveBinaryData            ; (was &8012) receive a BinaryData payload (wrapped in a packet) via cplink from the pi
NAME_TABLE:
        ;; NB the last letter of each RSX name must have bit 7 set to 1.
        DB      "FIFORS","T"+0x80
        DB      "SENDTXTCM","D"+0x80
        DB      "GETTXTCMDREPL","Y"+0x80
        DB      "SENDBINCM","D"+0x80
        DB      "GETBINCMDREPL","Y"+0x80
        DB      "SENDBINDAT","A"+0x80
        DB      "GETBINDAT","A"+0x80
        DB      0 ;end of name table marker
	;; **********************************************************
	;; IMPORTANT IMPORTANT IMPORTANT IMPORTANT
	;; **********************************************************
	;;
	;; It is a requirement of the user calling the functions to
	;; ensure that the buffer is large enough to hold any data.
	;; The receive packet routines will discard packets that are
	;; too large and return an error.  The following define is used to
	;; indicate the buffer size (with basic files its 0x7000 - 0x7FFF)
	;; change to what you want but ensure you dont over write this
	;; library code!! (currently from 0x8000 onwards)
        BUFFER_MAX_SIZE_HI EQU 0x10     ; this is 0x10 x 256 bytes
                                        ; i.e. 0x1000 (4K)
                                        ; minimum value is 0x01

	;; --------------------------------------------------------------
	;; RSX: |FIFORST
	;; --------------------------------------------------------------
	;; Initialise the CP-LINK interface
	;;
	;; Entry
	;; - none
	;;
	;; Exit
	;; - AF corrupt (from _fifo_reset call)
	;; - return to basic

FIFORST:
ifndef FAKE        
        ld   a,0xfd
        out  (0x81),a        	; write to status register resets FIFO
endif        
        ret

	;; --------------------------------------------------------------
	;; RSX: |SENDTXTCMD,buffer$,@error%
	;; --------------------------------------------------------------
	;; Send a TextCommand via CP-LINK  to pi
	;;
	;; Entry (BASIC)
	;; -  buffer$ is a BASIC string holding the command (IX 2,3)
	;;    error is an integer address so error values can be returned (IX 0,1)
	;;
	;; - A will have number of parameters
	;;
	;; Exit
	;; - AF corrupt
	;; - return to basic, error% will hold any errors
	;; - error% == 0x1 - invalid numbers of parameters
SendTextCommand
        cp 2                    ; we are expecting three parameters
        jp nz,ParamError        ; return error for invalid number of parameters        
        push ix
        push de
        push hl
        ld   h, (ix+3)          ; string descriptor pointer
        ld   l, (ix+2)
        ld   b, 0
        ld   c, (hl)            ; string len
        inc     hl
        ld   e, (hl)            ; de = bytes pointer
        inc     hl
        ld   d, (hl)
        ld   h, (ix+1)          ; hl = error num pointer
        ld   l, (ix)
        ;; create a new stack frame and point to last entry with ix        
        push de                 ; buffer address
        push bc                 ; buffer length
        push hl                 ; error number
        ld   ix,0
        add  ix,sp        
        ld e,TEXTCOMMAND        ; set e as parameter to SendPacket packet type
        call SendPacket         ; we have three params - send packet
        ;;  undo stack frame
        pop  hl                 ; discard
        pop  bc                 ; discard
        pop  de                 ; discard
        pop  hl                 ; restore hl from entry to proc
        pop  de                 ; restore de from entry to proc
        pop  ix                 ; restore ix from entry to proc
        ret                     ; return to basic
	;; --------------------------------------------------------------
	;; RSX: |GETTXTCMDREPLY,&buffer,@bufferlength%,printval,%@error%
	;; --------------------------------------------------------------
	;; Receive a TextCommand response via CP-LINK from pi and optionally print it
	;;
	;; Entry (BASIC)
	;; - buffer is a memory address holding the command and its params to send (IX 6,7)
	;;   bufferlength is the integer size of the packet body received (IX 4,5)
	;;   printval is 0 or 1.  1 will print arriving characters in the response (IX 2,3)
	;;   error is an integer address so error values can be returned (IX 0,1)
	;;
	;; - A will have number of parameters
	;;
	;; Exit
	;; - AF corrupt
	;; - return to basic, error% will hold any errors
	;; - error% == 0x1 - invalid numbers of parameters
	;; - error% == 0x2 - unexpected packet type received
	;; - error% == 0x3 - bad delimiter - malformed packet receieved
ReceiveTextCommandResponse:
        cp 4                    ; we are expecting four parameters
        jp nz,ParamError        ; return error for invalid number of parameters
        ld e,TEXTCOMMAND        ; set e as parameter of packet type we expect to receive
        call ReceivePacket      ; Receive a packet
        call CheckPrintReq      ; check if we need to print response
        ret                     ; return to basic

	;; --------------------------------------------------------------
	;; RSX: |SENDBINCMD,&buffer,bufferlength%,@error%
	;; --------------------------------------------------------------
	;; Send a BinaryCommand via CP-LINK  to pi
	;;
	;; Entry (BASIC)
	;; -  buffer is a memory address holding the command and its params to send (IX 4,5)
	;;    bufferlength is the integer size of the buffer to send (IX 2,3)
	;;    error is an integer address so error values can be returned (IX 0,1)
	;;
	;; - A will have number of parameters
	;;
	;; Exit
	;; - AF corrupt
	;; - return to basic, error% will hold any errors
	;; - error% == 0x1 - invalid numbers of parameters
SendBinaryCommand:
        cp 3                    ; we are expecting three parameters
        jp nz,ParamError        ; return error for invalid number of parameters
        ld e,BINARYCOMMAND      ; set e as parameter to SendPacket packet type
        call SendPacket         ; we have three params - send packet
        ret                     ; return to basic

	;; --------------------------------------------------------------
	;; RSX: |GETBINCMDREPLY,&buffer,@bufferlength%,printval,@error%
	;; --------------------------------------------------------------
	;; Receive a BinaryCommand response via CP-LINK from pi and optionally print it
	;;
	;; Entry (BASIC)
	;; - buffer is a memory address holding the command and its params to send (IX 6,7)
	;;   bufferlength is the integer size of the packet body received (IX 4,5)
	;;   printval is 0 or 1.  1 will print arriving characters in the response (IX 2,3)
	;;   error is an integer address so error values can be returned (IX 0,1)
	;;
	;; - A will have number of parameters
	;;
	;; Exit
	;; - AF corrupt
	;; - return to basic, error% will hold any errors
	;; - error% == 0x1 - invalid numbers of parameters
	;; - error% == 0x2 - unexpected packet type received
	;; - error% == 0x3 - bad delimiter - malformed packet receieved
ReceiveBinaryCommandResponse:
        cp 3                    ; we are expecting three parameters
        jp nz,ParamError        ; return error for invalid number of parameters
        ld e,BINARYCOMMAND      ; set e as parameter of packet type we expect to receive
        call ReceivePacket      ; Receive a packet
ifdef PRINT_BIN
        call CheckPrintReq      ; check if we need to print response
endif
        ret                     ; return to basic

	;; --------------------------------------------------------------
	;; RSX: |SENDBINDATA,&buffer,bufferlength%,@error%
	;; --------------------------------------------------------------
	;; Send BinaryData via CP-LINK  to pi
	;;
	;; Entry (BASIC)
	;; - buffer is a memory address holding the command and its params to send (IX 4,5)
	;;   bufferlength is the integer size of the buffer to send (IX 2,3)
	;;   error is an integer address so error values can be returned (IX 0,1)
	;;
	;; - A will have number of parameters
	;;
	;; Exit
	;; - AF corrupt
	;; - return to basic, error% will hold any errors
	;; - error% == 0x1 - invalid numbers of parameters
SendBinaryData:
        cp 3                    ; we are expecting three parameters
        jp nz,ParamError        ; return error for invalid number of parameters
        ld e,BINARYDATA         ; set e as parameter to SendPacket packet type
        call SendPacket         ; we have three params - send packet
        ret                     ; return to basic

	;; --------------------------------------------------------------
	;; RSX: |GETBINDATA,&buffer,@bufferlength%,printval,@error%
	;; --------------------------------------------------------------
	;; Receive a Binary data via CP-LINK from pi and optionally print it
	;;
	;; Entry
	;; - buffer is a memory address holding the command and its params to send (IX 6,7)
	;;   bufferlength is the integer size of the packet body received (IX 4,5)
	;;   printval is 0 or 1.  1 will print arriving characters in the response (IX 2,3)
	;;   error is an integer address so error values can be returned (IX 0,1)
	;;
	;; - A will have number of parameters
	;;
	;; Exit
	;; - AF corrupt
	;; - return to basic, error% will hold any errors
	;; - error% == 0x1 - invalid numbers of parameters
	;; - error% == 0x2 - unexpected packet type received
	;; - error% == 0x3 - bad delimiter - malformed packet receieved
ReceiveBinaryData:
        cp 3                    ; we are expecting three parameters
        jp nz,ParamError        ; return error for invalid number of parameters
        ld e,BINARYDATA         ; set e as parameter of packet type we expect to receive
        call ReceivePacket      ; Receive a packet
ifdef PRINT_BIN
        call CheckPrintReq      ; check if we need to print response
endif
        ret                     ; return to basic

	;; *************************************
	;; Support functions
	;; *************************************
	;; Send a formatted packet of data to pi
	;;
	;; Entry
	;; - E containing packet type
	;; - IX + 4 contains buffer address
	;; - IX + 2 contains buffer length
	;; - IX + 0 contains address of error%

SendPacket:
        ;; write +++
        ld d,'+'                ; load the byte to send in d
        call SendByte           ; send the delimiter '-' byte
        call SendByte           ; send the delimiter '-' byte
        call SendByte           ; send the delimiter '-' byte

        ;; write packet size (lo/high)
        ld l,(IX+2)             ; load hl with length of buffer
        ld h,(IX+3)             ; load hl with length of buffer
        ld a,PACKET_OVERHEAD    ; packet over head is 3 bytes
inc_again:
        inc hl
        dec a
        jr nz,inc_again
        ld a,e                  ; check if packet type e is a TEXTCOMMAND
        cp TEXTCOMMAND
        jr nz,SkipAddingNewLine ; not a TEXTCOMMAND so skip the inc hl (as the message has no \n)
        inc hl                  ; add 1 as we need to allow for \n terminator below for a TEXTCOMMAND

SkipAddingNewLine
	ld d,l                  ; get low byte
	call SendByte           ; send low byte of length
	ld d,h                  ; get high byte
	call SendByte           ; send high byte

	;; write packet type
	ld d,e                  ; load d with packet type value from e
	call SendByte

	;; write packet body
	ld l,(IX+4)             ; load the buffer address into hl
	ld h,(IX+5)             ; load the buffer address into hl
	ld c,(IX+2)             ; get the number of bytes of the message
	ld b,(IX+3)             ; get the number of bytes of the message

LoopSendNextByte:
        ld d,(hl)               ; load the byte of message into d

        push bc                 ; preserve bc
        call SendByte           ; send the byte
        pop bc                  ; get bc back

	inc hl
	dec bc                  ; dec loop counter (number bytes to send)
	ld a,c                  ; check if bc is 0x0000
	or a
	jr nz,LoopSendNextByte
	ld a,b
	or a
	jr nz,LoopSendNextByte  ; not zero, send next byte
	ld a,e                  ; check if packet type e is a TEXTCOMMAND
	cp TEXTCOMMAND
	jr nz,WriteTerminator   ; not a TEXTCOMMAND so skip output of \n
	ld d,0x0A               ; load \n to send to terminate string
	call SendByte           ; send the byte

        ;; write ---
WriteTerminator:
        ld d,'-'                ; load the byte to send in d
        call SendByte           ; send the delimiter '-' byte
        call SendByte           ; send the delimiter '-' byte
        call SendByte           ; send the delimiter '-' byte
        ret

	;; ******************************************
	;; Receive a formatted packet of data from pi
	;;
	;; Entry
	;; - E containing packet type we expect
	;; - IX + 6 contains buffer address
	;; - IX + 4 contains buffer length
	;; - IX + 2 contains print character received
	;; - IX + 0 contains address of error%
	;;
	;; Exit
	;; - AF, D, HL corrupt
	;; - return to basic

ReceivePacket:
        ld l,(IX+6)                     ; load the buffer address into hl
        ld h,(IX+7)                     ; load the buffer address into hl
        ld b,FRONT_PACKET_OVERHEAD      ; set up loop counter to read 6 bytes '+++' and packet header
        ;; read +++, size(lo/high) and packet_type
LoopReceiveNextByte:
        call ReceiveByte            	; get a byte from the CP-LINK interface
        inc hl                      	; increment HL to point at next space to receive a byte
        djnz LoopReceiveNextByte
	;; read packet type
	dec hl                      	; move hl back on to last byte received
	ld a,(hl)                   	; get packet_type byte received
	cp e                        	; check if it was the correct type packet
	jp nz,PacketTypeError       	; got a packet of the wrong type, so exit with error
	;; read packet size (lo/high) into bc
	dec hl                      	; move hl back to get packet size high byte
	ld b,(hl)                   	; load b with high byte size
	dec hl                      	; move hl back to get packet size low byte
	ld c,(hl)                   	; load c with low byte size
	;; reduce the packet size in bc by PACKET_OVERHEAD bytes
	ld a,PACKET_OVERHEAD
dec_again:
        dec bc
        dec a
        jr nz,dec_again

	;; check if bc bytes < (BUFFER_MAX_SIZE_HI << 8)
	;; if it is not then drain packet and return an error
	ld a,b                      	; (no need to check c as long as b < BUFFER_MAX_SIZE_HI)
	cp BUFFER_MAX_SIZE_HI       	; compare with size - high byte
	jr c,BufferCheckOK          	; if it is less than BUFFER_MAX_SIZE_HI then continue receive packet

	call DrainPacketFromQueue   	; drain the packet out of the fifo as we cant process it - its too big
	ld b,0x0                    	; set BC to 0x0000 i.e. there is no valid packet data in buffer
	ld c,0x0
	call SetBufferLengthParam   	; output 0x0000 as packet body size
	jp PacketSizeTooLargeError  	; return the error

BufferCheckOK:  
        ;; check for +++ delimiter
        ld d,DELIMITER_SIZE
NextDelimiterByte:
        dec hl
	ld a,(hl)
	cp '+'
	jp nz,BadDelimiterError
	dec d
	jr nz,NextDelimiterByte
	
	;; write out the packet body length to the basic parameter
	call SetBufferLengthParam

        ;; read packet body
        ld l,(IX+6)             ; load the buffer address into hl afresh
        ld h,(IX+7)             ; load the buffer address into hl afresh
                                ; (as we are discarding everything written so far)
NextBodyByte:   
        call ReceiveByte        ; get a byte from the CP-LINK interface
        inc hl                  ; increment hl to next byte in buffer
        dec bc                  ; read a byte so decrease the packet_size counter (from above)
	ld a,c                  ; check if bc is 0x0000
	or a
	jr nz,NextBodyByte      ; not zero, loop for next byte
	ld a,b
	or a
	jr nz,NextBodyByte      ; not zero, loop for next byte	
	;; read ---
	ld b,DELIMITER_SIZE
NextEndDelimiter:       
        call ReceiveByte        ; get a byte from the CP-LINK interface
        djnz NextEndDelimiter
        ld (hl),0x0             ; clear the byte we have writing to (we dont want it)
        ret

	;; ******************************************
	;; read BC bytes from buffer + DELIMITER_SIZE
	;; which will clear the current packet from
	;; the fifo/mcp queues
	;;
	;; Entry
	;; - BC contains packet body length
	;;
	;; Exit
	;; - BC, HL corrupt
DrainPacketFromQueue
	ld l,(IX+6)             ; load the buffer address into hl afresh
	ld h,(IX+7)             ; load the buffer address into hl afresh
	                        ; as ReceiveByte needs some where to store
	                        ; incoming byte even if not going to be used

NextLongBodyByte
	call ReceiveByte        ; get a byte from the CP-LINK interface
	dec bc                  ; read a byte so decrease the packet_size counter (from above)
	
	ld a,c                  ; check if bc is 0x0000
	or a
	jr nz,NextLongBodyByte  ; not zero, loop for next byte
	ld a,b
	or a
	jr nz,NextLongBodyByte  ; not zero, loop for next byte
	
	;; read DELIMITER_SIZE bytes to end of packet
	ld b,DELIMITER_SIZE
NextLongBodyDelimiter
        call ReceiveByte          ; get a byte from the CP-LINK interface
        djnz NextLongBodyDelimiter
        ld (hl),0x0               ; clear the byte we have writing to (we dont want it)
        ret
        
	;; ******************************************
	;; write out the packet body length to the basic
	;; parameter
	;;
	;; Entry
	;; - BC contains packet body length
	;; - IX + 4 contains buffer length
	;; - IX + 5 contains buffer length
	;;
	;; Exit
	;; - HL corrupt
SetBufferLengthParam
        ld l,(IX+4)               ; get the address of the buffer_length% parameter
	ld h,(IX+5)               ; get the address of the buffer_length% parameter
	ld (hl),c                 ; output the buffer length to the parameter
	inc hl
	ld (hl),b
	ret

	;; **********************************************
	;; This function will check if printing is enabled.
	;; TextCommand will be packet body printed as text.
	;; BinaryCommand and BinaryData will be printed as
	;; HEX for the whole packet
	;;
	;; Entry
	;; - 'e' holds packet type
	;;
	;; Exit
	;; - AF, HL, BC corrupt

CheckPrintReq
        ;; are we asked to print the character
	ld a,(IX+2)               ; read basic parameter do we print received data
	cp 0x1                    ; is it set
	ret nz                    ; if its not set skip the call to print the packet
	
	;; read in the packet size value from the passed address
	ld l,(IX+4)               ; load the packet body size into bc from the address of the int
	ld h,(IX+5)
	ld c,(hl)
	inc hl
	ld b,(hl)
	
	;; check if BC is 0x0000 i.e. no data to print
	ld a,c                    ; check if bc is 0x0000
	or a
	jr nz,ContinuePrint       ; not zero, then print
	ld a,b
	or a
	ret z                     ; return on zero otherwise fall through

ContinuePrint
	;; read in the buffer address
	ld l,(IX+6)               ; load the buffer address into hl
	ld h,(IX+7)               ; load the buffer address into hl
	
	;; start the output on a new line
	ld a,13
	call FW_PrintChar
	ld a,10
	call FW_PrintChar

ifdef PRINT_BIN
        ld a,e
        cp 0x1
        jp nz,PrintHexPacket      ; if its not a TEXTCOMMAND jp to PrintHexPacket (for handling binary)
endif

PrintTextPacket
        ;; get the byte and print it
LoopPrintNextByte
        ld a,(hl)                   ; get the character from the buffer
	cp a,13                     ; is the character \r (as we will add \n if it is)
	jr nz,SkipNL
	call FW_PrintChar           ; print the character
	ld a,10                     ; set up next char as \n
	jr SkipCR
SkipNL
	cp a,10
	jr nz,SkipCR
	call FW_PrintChar           ; print the character
	ld a,13                     ; set up next char as \n
SkipCR
	call FW_PrintChar           ; print the character
	inc hl
	dec bc
	
	ld a,c                      ; check if bc is 0x0000
	cp 0x0
	jr nz,LoopPrintNextByte
	ld a,b
	cp 0x0
	jr nz,LoopPrintNextByte     ; not zero, send next byte
	ret

ifdef PRINT_BIN

PrintHexPacket
        ;; handle Binary Packet
LoopPrintNextHexByte
	ld a,(hl)                   ; get the character from the buffer
	call PrintHexChar           ; print the character
	inc hl
	dec bc	
	ld a,c                      ; check if bc is 0x0000
	cp 0x0
	jr nz,LoopPrintNextHexByte  ; not zero, print next byte
	ld a,b
	cp 0x0
	jr nz,LoopPrintNextHexByte  ; not zero, print next byte
	ret

	;; **********************************************
	;; This function will print the character in 'a'
	;; to the screen as a HEX byte pair i.e. 00 to FF
	;;
	;; Entry
	;; - 'a' holds byte to print
	;;
	;; Exit
	;; - AF corrupt

PrintHexChar
        push af                     ; preserve a
        rra                         ; 4x rra == a/16
        rra
        rra
        rra
        and 0xF                     ; ignore bits from carry
        call PrintHexDigit          ; print hex char first digit
	pop af	
	and 0xF
	call PrintHexDigit          ; print hex char first digit	
	ld a,0x20                   ; a space character
	call FW_PrintChar
	ret
PrintHexDigit:
        cp 10
        jr c,PrintDigit
        add 7                       ; convert values above 9 to A - F
PrintDigit:
        add 48
        call FW_PrintChar
        ret
endif

	;; **********************************************
	;; This function will keep trying to send
	;; a byte via CP-LINK to pi until it succeeds
	;;
	;; Entry
	;; - d holds byte to send
	;;
	;; Exit
	;; - AF corrupt
	;; - BC corrupt (from _fifo_out_byte call)
SendByte
        call _fifo_out_byte    ; send the byte in register d
        dec a
        jr nz,SendByte         ; no a is not 1, so loop trying to SendByte
        ret

        ;; **********************************************
	;; This function will keep trying to receive
	;; a byte via CP-LINK to pi until it succeeds
	;;
	;; Entry
	;; - HL points to byte location for received byte
	;;
	;; Exit
	;; - AF corrupt

ReceiveByte
	call _fifo_in_byte     ; send the byte in register l
	dec a
	jr nz,ReceiveByte      ; no a is not 1, so loop trying to receive byte
	ret
	;; **********************************************
	;; Sets the error basic variable in call
	;;
	;; Entry
	;; - IX holds reference to basic call parameters
	;;
	;; Exit
	;; - HL corrupt
ParamError
        ld a,0x1
        jr SetError
PacketTypeError
        ld a,0x2
        jr SetError
BadDelimiterError
	ld a,0x3
	jr SetError
PacketSizeTooLargeError
        ld a,0x4
        jr SetError
SetError
	ld l,(IX+0)             ; get the address of the error%
	ld h,(IX+1)             ; get the address of the error%
	ld (hl),a               ; return error in low byte
	inc hl
	ld (hl),0x0             ; return 0x0 in high byte
	ret

ifdef FAKE
    include "fifolib_fake.s"                    ;; include a fake interface to hardware for testing on pc
else
    include "fifolib_slim.s"                    ;; include the standard routines for reading/writing bytes to the interface
endif
