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
_fifo_reset::
        ld   a,#0xfd
        out  (#0x81),a        	; write to status register resets FIFO
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
	;; - L =1 if successful, otherwise zero
	;; - AF, HL corrupt
	;; - byte at (HL) updated
_fifo_in_byte::
        ld   a,#0xfd
        in   a,(#0x81)         	; get status
        and  #0x1              	; test DOR flag
        jr   z,fib_end       	; go to end if no data available
        ld   a,#0xfd
        in   a,(#0x80)         	; get byte
	ld   (hl),a
        ld   a,#1               ; success=1
fib_end:
        ld   l,a
        ret

	;; --------------------------------------------------------------
	;; fifo_out_byte    (__z88dk_fastcall)
	;; --------------------------------------------------------------
	;;
	;; Write a single byte and store it in the location pointed to by
	;; a parameter.
	;;
	;; Entry
	;; - L holds byte to be written
	;;
	;; Exit
	;; - L =1 if successful, otherwise zero
	;; - AF, BC corrupt
_fifo_out_byte::
        ld   bc, #0xfd81
        in   a,(c)         	; get dir status flag
        and  #0x2
        jr   z,fob_end          ; go to end if no data available
        dec c			; point to data reg
        out (c), l              ; write the byte
        ld   a, #0x1           	; success
fob_end:
        ld   l,a
        ret

	;; --------------------------------------------------------
	;; fifo_ext_param
	;; --------------------------------------------------------
        ;; Service routine to extract parameters from stack
        ;; and set up common start conditions for the fifo byte transfer
        ;; routines
        ;;
        ;; Entry
	;;  (SP+6) = Num bytes
	;;  (SP+4) = RX buffer
	;;  (SP+2) = Ext Ret addr
        ;;  (SP)   = Immediate ret addr
	;; Exit:
        ;;  bc     = 0xfd80 (point to the data register)
	;;  hl     = pointer to num bytes
	;;   a     = num of bytes to read= 0-255
        ;;   d     = num of bytes to read= 0-255 (duplicated)
        ;;   e     = num of bytes to read= 0-255 (duplicated)
	;;  all other register preserved
fifo_ext_param:
        push ix                 ; save IX (pushing every thing 2 bytes deeper)
        ld   ix,#6
        add  ix,sp
        ld   bc, #0xfd80        ; init FIFO register pointer
        ld   l,0(ix)
        ld   h,1(ix)
        ld   a,3(ix)            ; get high byte of count
        and  #0xff              ; check if non-zero
        jr   z,fep_get_lowb     ; get low byte if zero
        rla                     ; check sign bit
        jr   c,fep_end0         ; exit (with 0) if sign bit is set
        ld   a,#0xff            ; else, +ve, non-zero hi byte so set size to 255 max
        jr   fep_end
fep_get_lowb:
        ld   a,2(ix)            ; get low byte of max bytes param
fep_end:
        ld   d,a
        ld   e,a
        pop  ix
        ret
fep_end0:
        ld   de, #0x0000        ; no bytes to read
        ld   a,#0
        pop  ix
        ret

	;; --------------------------------------------------------
	;; fifo_in_bytes
	;; --------------------------------------------------------
	;;
	;; Read up to 255 bytes from the FIFO terminating when the
	;; FIFO signals EMTPY and returns the actual number read.
	;;
	;; Routine will return early if a negative 16b number or 0
	;; is passed.
	;;
	;; Entry:
	;;  (SP+4) = Num bytes
	;;  (SP+2) = RX buffer
	;;  (SP)   = Ret addr
	;; Exit:
	;;  hl     = number of bytes read
	;;  RX Buffer holds bytes read
	;;  IY,IX preserved
	;;  all other register corrupted
_fifo_in_bytes::
        call fifo_ext_param     ; get buffer pointer in hl, number of bytes in a
        and  #0xFF              ; check if A is zero
        jr   z, fibs_end
        inc  c                  ; point to status register first
        rra                     ; check if count is odd in which case start at top1
        jr   c, fibs_top1       ; else start at top2 to progress in twos
fibs_top2:
        in   a,(c)              ; get dor status flag
        rra
        jr   nc,fibs_end  	; go to end if no data available
        dec  c            	; point to data reg
        ini               	; (hl)<-in(bc), hl++, b--
        inc  b            	; restore b
        inc  c            	; point to status reg for next check
        dec  e            	; update counter (but no need to check for zero here)
fibs_top1:
        in   a,(c)        	; get dor status flag
        rra
        jr   nc,fibs_end  	; go to end if no data available
        dec  c            	; point to data reg
        ini               	; (hl)<-in(bc), hl++, b--
        inc  b            	; restore b
        inc  c            	; point to status reg for next check
        dec  e            	; decrement counter and check if done
        jr   nz,fibs_top2 	; if not loop again
fibs_end:
        ld   a,d          	; restore max count
        sub  e            	; subtract remaining bytes
        ld   l, a
        ret

	;; --------------------------------------------------------
	;; fifo_out_bytes
	;; --------------------------------------------------------
	;;
	;; Write up to 255 bytes from the FIFO terminating when the
	;; FIFO signals FULL and returns the actual number written.
	;;
	;; Routine will return early if a negative 16b number or 0
	;; is passed.
	;;
	;; Entry:
	;;  (SP+4) = Num bytes
	;;  (SP+2) = TX buffer
	;;  (SP)   = Ret addr
	;; Exit:
	;;  hl     = number of bytes read
	;;  RX Buffer holds bytes read
	;;  IY,IX preserved
	;;  all other register corrupted
_fifo_out_bytes::
        call fifo_ext_param     ; get buffer pointer in hl, number of bytes in a
        and  #0xFF
        jr   z, fobs_end
        inc  c                  ; point to status register first
        rra                     ; check if count is odd in which case start at top1
        jr   c, fobs_top1       ; else start at top2 to progress in twos
fobs_top2:
        in   a,(c)              ; get dir status flag
        and  #0x2
        jr   z,fobs_end  	; go to end if no data available
        dec  c            	; point to data reg
        inc  b                  ; pre-incr B
        outi               	; b-- ; OUT(BC) <- (hl) ; hl++
        inc  c            	; point to status reg for next check
        dec  e            	; update counter (but no need to check for zero here)
fobs_top1:
        in   a,(c)        	; get dir status flag
        and  #0x2
        jr   z,fobs_end  	; go to end if no data available
        dec  c            	; point to data reg
        inc  b                  ; pre-incr B
        outi               	; b-- ; OUT(BC) <- (hl) ; hl++
        inc  c            	; point to status reg for next check
        dec  e            	; decrement counter and check if done
        jr   nz,fobs_top2 	; if not loop again
fobs_end:
        ld   a,d          	; restore max count
        sub  e            	; subtract remaining bytes
        ld   l, a
        ret

	;; --------------------------------------------------------
	;; fifo_in_nc_bytes
	;; --------------------------------------------------------
	;;
	;; Read up to 255 bytes from the FIFO without looking at the
	;; status flags and terminating only when all bytes are trans-
        ;; -ferred. Routine will return early if a negative 16b number
        ;; or 0 is passed.
	;;
	;; Entry:
	;;  (SP+4) = Num bytes
	;;  (SP+2) = RX buffer
	;;  (SP)   = Ret addr
	;; Exit:
	;;  hl     = number of bytes read
	;;  RX Buffer holds bytes read
	;;  IY,IX preserved
	;;  all other register corrupted
_fifo_in_nc_bytes::
        call fifo_ext_param     ; get buffer pointer in hl, number of bytes in a
        and  #0xFF
        jr   z, finbs_end
        rra                     ; check if count is odd in which case start at top1
        jr   c, finbs_top1      ; else start at top2 to progress in twos
finbs_top2:
        ini               	; (hl)<-in(bc), hl++, b--
        inc  b            	; restore b
        dec  e            	; decrement counter but no need to check
finbs_top1:
        ini               	; (hl)<-in(bc), hl++, b--
        inc  b            	; restore b
        dec  e            	; decrement counter and check if done
        jr   nz,finbs_top2 	; if not loop again
finbs_end:
        ld   a,d          	; restore max count
        sub  e            	; subtract remaining bytes
        ld   l, a
        ret

	;; --------------------------------------------------------
	;; fifo_out_nc_bytes
	;; --------------------------------------------------------
	;;
	;; Write up to 255 bytes from the FIFO without looking at the
	;; status flags and terminating only when all bytes are trans-
        ;; -ferred. Routine will return early if a negative 16b number
        ;; or 0 is passed.
	;;
	;; Entry:
	;;  (SP+4) = Num bytes
	;;  (SP+2) = TX buffer
	;;  (SP)   = Ret addr
	;; Exit:
	;;  hl     = number of bytes read
	;;  RX Buffer holds bytes read
	;;  IY,IX preserved
	;;  all other register corrupted
_fifo_out_nc_bytes::
        call fifo_ext_param     ; get buffer pointer in hl, number of bytes in a
        and  #0xFF
        jr   z, fonbs_end
        rra                     ; check if count is odd in which case start at top1
        jr   c, fonbs_top1      ; else start at top2 to progress in twos
fonbs_top2:
        inc  b            	; pre-inc  b
        outi               	; b-- ; OUT(BC)<-(HL) ; hl++
        dec e
fonbs_top1:
        inc  b            	; pre-inc  b
        outi               	; b-- ; OUT(BC)<-(HL) ; hl++
        dec  e            	; decrement counter and check if done
        jr   nz,fonbs_top2 	; if not loop again
fonbs_end:
        ld   a,d                ; restore max count
        sub  e            	; subtract remaining bytes
        ld   l, a
        ret
