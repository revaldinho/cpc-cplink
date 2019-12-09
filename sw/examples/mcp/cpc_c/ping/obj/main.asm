;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.7.0 #10231 (Mac OS X x86_64)
;--------------------------------------------------------
	.module main
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _fifo_out_byte
	.globl _fifo_in_byte
	.globl _fifo_reset
	.globl _scr_set_mode
	.globl _km_wait_char
	.globl _printf
	.globl _send_command_to_pi
	.globl _read_response_from_pi
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;src/main.c:31: void main ( void ) 
;	---------------------------------
; Function main
; ---------------------------------
_main::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl, #-30
	add	hl, sp
	ld	sp, hl
;src/main.c:33: uint8_t *command = "PING\n";
;src/main.c:38: for(uint8_t index = 0; index < RESPONSE_LENGTH; index++)
	ld	hl, #0x0000
	add	hl, sp
	ex	de, hl
	ld	c, #0x00
00103$:
	ld	a, c
	sub	a, #0x1e
	jr	NC,00101$
;src/main.c:40: response[index] = 0;
	ld	l, c
	ld	h, #0x00
	add	hl, de
	ld	(hl), #0x00
;src/main.c:38: for(uint8_t index = 0; index < RESPONSE_LENGTH; index++)
	inc	c
	jr	00103$
00101$:
;src/main.c:44: scr_set_mode(2);
	push	de
	ld	hl, #0x0002
	call	_scr_set_mode
	call	_fifo_reset
	ld	hl, #___str_0
	push	hl
	call	_send_command_to_pi
	ld	hl, #___str_0
	ex	(sp),hl
	ld	hl, #___str_1
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	de
;src/main.c:56: read_response_from_pi(response);
	ld	c, e
	ld	b, d
	push	de
	push	bc
	call	_read_response_from_pi
	ld	hl, #___str_2
	ex	(sp),hl
	call	_printf
	pop	af
;src/main.c:61: printf("Press any key to exit\r");
	ld	hl, #___str_3
	ex	(sp),hl
	call	_printf
	pop	af
;src/main.c:62: km_wait_char();
	call	_km_wait_char
;src/main.c:63: }
	ld	sp, ix
	pop	ix
	ret
___str_0:
	.ascii "PING"
	.db 0x0a
	.db 0x00
___str_1:
	.db 0x0d
	.ascii "%s"
	.db 0x0d
	.db 0x00
___str_2:
	.ascii "%s"
	.db 0x0d
	.db 0x0d
	.db 0x00
___str_3:
	.ascii "Press any key to exit"
	.db 0x0d
	.db 0x00
;src/main.c:65: void send_command_to_pi(uint8_t *command)
;	---------------------------------
; Function send_command_to_pi
; ---------------------------------
_send_command_to_pi::
;src/main.c:70: while(command[counter] != 0)
	ld	c, #0x00
00101$:
	ld	hl, #2
	add	hl, sp
	ld	a, (hl)
	inc	hl
	ld	h, (hl)
	ld	l, a
	ld	b, #0x00
	add	hl, bc
	ld	a, (hl)
	or	a, a
	jr	Z,00114$
;src/main.c:72: counter++;
	inc	c
	jr	00101$
00114$:
;src/main.c:76: for(uint8_t index = 0; index < counter; index++)
	ld	b, #0x00
00106$:
	ld	a, b
	sub	a, c
	ret	NC
;src/main.c:78: fifo_out_byte(command[ index ]);
	ld	iy, #2
	add	iy, sp
	ld	a, 0 (iy)
	add	a, b
	ld	e, a
	ld	a, 1 (iy)
	adc	a, #0x00
	ld	d, a
	ld	a, (de)
	ld	l, a
	push	bc
	call	_fifo_out_byte
	pop	bc
;src/main.c:76: for(uint8_t index = 0; index < counter; index++)
	inc	b
;src/main.c:80: }
	jr	00106$
;src/main.c:82: void read_response_from_pi(uint8_t *response)
;	---------------------------------
; Function read_response_from_pi
; ---------------------------------
_read_response_from_pi::
	push	ix
	ld	ix,#0
	add	ix,sp
;src/main.c:85: uint8_t did_we_get_byte = 0;
;src/main.c:87: while((receive_byte_index == 0) || ((receive_byte_index > 0) && (response[receive_byte_index - 1] != '\n')))
	ld	bc, #0x0000
00106$:
	ld	a, c
	or	a, a
	jr	Z,00101$
	ld	a, c
	or	a, a
	jr	Z,00109$
	ld	e, c
	ld	d, #0x00
	dec	de
	ld	l, 4 (ix)
	ld	h, 5 (ix)
	add	hl, de
	ld	a, (hl)
	sub	a, #0x0a
	jr	Z,00109$
;src/main.c:89: while(did_we_get_byte == 0)
00101$:
	ld	a, b
	or	a, a
	jr	NZ,00103$
;src/main.c:92: did_we_get_byte = fifo_in_byte(&response[receive_byte_index]);
	ld	l, 4 (ix)
	ld	h, 5 (ix)
	ld	b, #0x00
	add	hl, bc
	push	bc
	call	_fifo_in_byte
	pop	bc
	ld	b, l
	jr	00101$
00103$:
;src/main.c:97: did_we_get_byte = 0;
	ld	b, #0x00
;src/main.c:100: receive_byte_index++;
	inc	c
	jr	00106$
00109$:
;src/main.c:103: }
	pop	ix
	ret
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
