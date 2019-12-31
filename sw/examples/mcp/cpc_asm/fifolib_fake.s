	;; --------------------------------------------------------------
	;; fifolib_fake.s
	;; --------------------------------------------------------------
	;;
	;; fifolib.s that does nothing to help with dev on no hardware


	;; --------------------------------------------------------------
	;; fifo_reset
	;; --------------------------------------------------------------
	;;
_fifo_reset:
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
_fifo_in_byte:

		push bc

			ld bc,demo_message
			ld a,l
			cp 0
	dec_again2:
			jr z,get_demo_byte
			inc bc
			dec a
			jr nz,dec_again2
	get_demo_byte:
			ld a,(bc)
			ld (hl),a

		;; call PrintHexChar		; print the character

		pop bc
		
		ld a,0xff				; just corrupting the registers as the real call would
		ld h,0xff
		ld l,0xff

        ld l,0x1               	; success=1
        ret

demo_message:
		defb "+++",9,0,1,"PONG!",10,"---"

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
_fifo_out_byte:
		ld a,l
		call PrintHexChar		; print the character

		ld a,0xff				; just corrupting the registers as the real call would
		ld b,0xff
		ld c,0xff

        ld l, 0x1           	; success
        ret
