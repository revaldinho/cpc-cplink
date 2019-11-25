	org 116
        ld bc, $fd81 	        ; point to status reg
	call gethl              ; payload length
        ex   de, hl 
        call gethl              ; destination address

onemore:
        call getbyte
        ld (hl),a
        inc hl
        dec de
        ld a, d
        or e
        jr nz,onemore
        call gethl              ; execution address
        push hl                 ; nop here to suppress auto-run
        ret

gethl:
        call getbyte
        ld l,a
        call getbyte
        ld h,a
        ret

getbyte:
        in a,(c)
        rra
        jr nc, getbyte
        dec c                   ; point to data reg
        in a,(c)
        inc c                   ; point to status reg
        ret
        nop
        nop
        
       
