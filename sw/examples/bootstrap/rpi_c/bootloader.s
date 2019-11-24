	org 116
	        
	call gethl ; payload length
        ex   de, hl 
        call gethl ; destination address

onemore:
        call getbyte
        ld (hl),a
        inc hl
        dec de
        ld a, d
        or e
        jr nz,onemore
        call gethl ; execution address
        push hl ; nop here to suppress auto-run
        ret

gethl:
        call getbyte
        ld l,a
        call getbyte
        ld h,a
        ret

getbyte:
        ld bc, $fd81
checkstatus:
        in a,(c)
        rra
        jr nc, checkstatus
        dec bc
        in a,(c)
        ret
        nop
        nop
        
       
