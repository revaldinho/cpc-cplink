	;; Amstrad CPC Firmware routines
	;;
	;; (c) Revaldinho 2019
	;;
	;; All use standard SDCC stack based calling convention unless marked otherwise.
	;;
	;; Parameters are pushed onto the stack from right to left, and finally the
	;; return address is pushed.
	;;
	;;     +2N-> 16n Parameter N
	;;     +4 -> 16b Parameter 2
	;;     +2 -> 16b Parameter 1
	;;  SP +0 -> [Return Address]
	;;
	;; SDCC uses IX, IY for local register pointers, so need to preserve these
	;; in firmware calls. Registers which are used in these wrappers are usually
	;; spotted and preserved before and after the call, but any registers corrupted
	;; inside firmware calls will need to be preserved explictly in the wrappers.

	;; ====================================
	;; Graphics Calls
	;; ====================================

        ;; -------------------------------------------------------
	;; gra_set_pen - Set graphics pen ink.  (__z88dk_fastcall)
        ;; -------------------------------------------------------
	;;
	;; Entry:
	;; - l = pen
	;; Exit
	;; - af corrupt
	;; - all other registers preserved
_gra_set_pen::
        ld      a, l
        call    #0xbbde
        ret
        ;; -------------------------------------------------------
        ;; gra_line_abs - draw a line to an absolute position
        ;; -------------------------------------------------------
	;;
	;; Entry:
	;; - Param 1 = X
	;; - Param 2 = Y
	;;
	;; Exit:
	;; - af corrupt
	;; - de, hl used in wrapper
	;; - all other registers preserved
_gra_line_abs::
	push 	ix
        ld      ix,#4
        add     ix,sp

	ld 	e, 0(ix)
	ld 	d, 1(ix)
	ld 	l, 2(ix)
	ld 	h, 3(ix)
	push	bc
        call    #0xbbf6
	pop	bc
	pop	ix
        ret
        ;; ---------------------------------------------------
        ;; gra_move_abs - Move graphics cursor to an absolute position
        ;; ---------------------------------------------------
	;;
	;; Entry:
	;; - Param 1 = X
	;; - Param 2 = Y
	;;
	;; Exit:
	;; - af corrupt
	;; - de, hl used in wrapper
	;; - all other registers preserved
_gra_move_abs::
	push 	ix
        ld      ix,#4
        add     ix,sp

	ld 	e, 0(ix)
	ld 	d, 1(ix)
	ld 	l, 2(ix)
	ld 	h, 3(ix)
	push	bc
        call    #0xbbc0
	pop	bc
	pop	ix
        ret

        ;; ---------------------------------------------------
        ;; gra_plot_abs - Plot a point at an absolute position
        ;; ---------------------------------------------------
	;;
	;; Entry:
	;; - Param 1 = X
	;; - Param 2 = Y
	;;
	;; Exit:
	;; - af corrupt
	;; - de, hl used in wrapper
	;; - all other registers preserved
_gra_plot_abs::
	push 	ix
        ld      ix,#4
        add     ix,sp

	ld 	e, 0(ix)
	ld 	d, 1(ix)
	ld 	l, 2(ix)
	ld 	h, 3(ix)
	push	bc
        call    #0xbbea
	pop	bc
	pop	ix
        ret

        ;; ---------------------------------------------------
	;; gra_set_origin  - set the origin of the user coordinates
        ;; ---------------------------------------------------
	;;
	;; Entry:
	;; - Param 1 = X
	;; - Param 2 = Y
	;;
	;; Exit:
	;; - af corrupt
	;; - de, hl used in wrapper
	;; - all other registers preserved
_gra_set_origin::
	push 	ix
        ld      ix,#4
        add     ix,sp

	ld 	e, 0(ix)
	ld 	d, 1(ix)
	ld 	l, 2(ix)
	ld 	h, 3(ix)
	push	bc
        call    #0xbbc9
	pop	bc
	pop	ix
        ret

	;; ====================================
	;; Kernel Block Calls
	;; ====================================

        ;; ---------------------------------------------------
	;; kl_time_please - Ask elapsed time
        ;; ---------------------------------------------------
	;;
	;; Entry
	;; - no parameters
	;; Exit
	;; - <de,hl> - time in 1/300th sec (32bit long int)
	;; - all other registers preserved
_kl_time_please::
        call    #0xbd0d
        ret

        ;; ---------------------------------------------------
	;; kl_time_set - set elapsed time (usu. to zero) (__z88dk_fastcall)
        ;; ---------------------------------------------------
	;;
	;; Entry
	;; <dehl> - time in 1/300th sec (32b long int)
	;; Exit
	;; - af corrupt
	;; - all registers preserved
_kl_time_set::
        call    #0xbd10
        ret

	;; ====================================
	;; Keyboard Manager Calls
	;; ====================================

        ;; ---------------------------------------------------
	;; km_wait_char - Wait for a key press and return ASCII value (__z88dk_fastcall)
        ;; ---------------------------------------------------
	;;
	;;
	;; Entry:
	;; - none
	;; Exit:
	;; - carry - True
	;; - l     - character
	;; - af corrupt
	;; - all other registers preserved
_km_wait_char::
	call 	#0xbb06
	ld 	l,a
	ret

	;; ====================================
	;; Screen Pack Calls
	;; ====================================

        ;; ---------------------------------------------------
        ;; scr_set_mode - set screen to new mode (__z88dk_fastcall)
        ;; ---------------------------------------------------
	;;
	;; Entry:
	;; - l - new mode
	;; Exit:
	;; - af corrupt
	;; - all other registers preserved
_scr_set_mode::
        ld      a,l
	push	hl
	push	bc
	push	de
        call    #0xbc0e
	pop	de
	pop	bc
	pop	hl
        ret
