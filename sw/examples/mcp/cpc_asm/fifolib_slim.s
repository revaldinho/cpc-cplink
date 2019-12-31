	;; --------------------------------------------------------------
	;; fifolib.s
	;; --------------------------------------------------------------
	;;
	;; fifolib.s - an SDCC library for the CPC-CPLink board
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


	;; --------------------------------------------------------------
	;; fifo_reset
	;; --------------------------------------------------------------
	;;
	;; Reset the FIFO
	;;
	;; Entry
	;; - None
	;;
	;; Exit
	;; - AF Corrupt
_fifo_reset:
        ld   a,0xfd
        out  (0x81),a        	; write to status register resets FIFO
        ret

	;; --------------------------------------------------------------
	;; fifo_in_byte    (__z88dk_fastcall)
	;; --------------------------------------------------------------
	;;
	;; Read a single byte and store it in the location pointed to by
	;; a parameter
	;;
	;; Entry
	;; - HL points to byte location for received byte
	;;
	;; Exit
	;; - A = 1 if successful, otherwise zero
	;; - AF corrupt
	;; - byte at (HL) updated
_fifo_in_byte:
        ld   a,0xfd
        in   a,(0x81)         	; get status
        and  0x1              	; test DOR flag
        ret  z       			; return if no data available

        ld   a,0xfd
        in   a,(0x80)         	; get byte
		ld   (hl),a

		;; debug
		;;push af
		;;ld a,13
		;;call FW_PrintChar
		;;ld a,10
		;;call FW_PrintChar
		;;ld a,h
		;;call PrintHexChar
		;;ld a,l
		;;call PrintHexChar
		;;pop af
	    ;;call PrintHexChar

        ld   a,1               ; success=1
        ret

	;; --------------------------------------------------------------
	;; fifo_out_byte    (__z88dk_fastcall)
	;; --------------------------------------------------------------
	;;
	;; Write a single byte and store it in the location pointed to by
	;; a parameter.
	;;
	;; Entry
	;; - D holds byte to be written
	;;
	;; Exit
	;; - A = 1 if successful, otherwise zero
	;; - AF, BC corrupt

_fifo_out_byte:
        ld   bc, 0xfd81
        in   a,(c)         	; get dir status flag
        and  0x2
        ret  z			    ; return if no data available
        dec c				; point to data reg
        out (c), d          ; write the byte
        ld   a, 0x1         ; success
        ret
