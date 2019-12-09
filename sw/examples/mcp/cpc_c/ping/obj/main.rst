                              1 ;--------------------------------------------------------
                              2 ; File Created by SDCC : free open source ANSI-C Compiler
                              3 ; Version 3.7.0 #10231 (Mac OS X x86_64)
                              4 ;--------------------------------------------------------
                              5 	.module main
                              6 	.optsdcc -mz80
                              7 	
                              8 ;--------------------------------------------------------
                              9 ; Public variables in this module
                             10 ;--------------------------------------------------------
                             11 	.globl _main
                             12 	.globl _fifo_out_byte
                             13 	.globl _fifo_in_byte
                             14 	.globl _fifo_reset
                             15 	.globl _scr_set_mode
                             16 	.globl _km_wait_char
                             17 	.globl _printf
                             18 	.globl _send_command_to_pi
                             19 	.globl _read_response_from_pi
                             20 ;--------------------------------------------------------
                             21 ; special function registers
                             22 ;--------------------------------------------------------
                             23 ;--------------------------------------------------------
                             24 ; ram data
                             25 ;--------------------------------------------------------
                             26 	.area _DATA
                             27 ;--------------------------------------------------------
                             28 ; ram data
                             29 ;--------------------------------------------------------
                             30 	.area _INITIALIZED
                             31 ;--------------------------------------------------------
                             32 ; absolute external ram data
                             33 ;--------------------------------------------------------
                             34 	.area _DABS (ABS)
                             35 ;--------------------------------------------------------
                             36 ; global & static initialisations
                             37 ;--------------------------------------------------------
                             38 	.area _HOME
                             39 	.area _GSINIT
                             40 	.area _GSFINAL
                             41 	.area _GSINIT
                             42 ;--------------------------------------------------------
                             43 ; Home
                             44 ;--------------------------------------------------------
                             45 	.area _HOME
                             46 	.area _HOME
                             47 ;--------------------------------------------------------
                             48 ; code
                             49 ;--------------------------------------------------------
                             50 	.area _CODE
                             51 ;src/main.c:31: void main ( void ) 
                             52 ;	---------------------------------
                             53 ; Function main
                             54 ; ---------------------------------
   4000                      55 _main::
   4000 DD E5         [15]   56 	push	ix
   4002 DD 21 00 00   [14]   57 	ld	ix,#0
   4006 DD 39         [15]   58 	add	ix,sp
   4008 21 E2 FF      [10]   59 	ld	hl, #-30
   400B 39            [11]   60 	add	hl, sp
   400C F9            [ 6]   61 	ld	sp, hl
                             62 ;src/main.c:33: uint8_t *command = "PING\n";
                             63 ;src/main.c:38: for(uint8_t index = 0; index < RESPONSE_LENGTH; index++)
   400D 21 00 00      [10]   64 	ld	hl, #0x0000
   4010 39            [11]   65 	add	hl, sp
   4011 EB            [ 4]   66 	ex	de, hl
   4012 0E 00         [ 7]   67 	ld	c, #0x00
   4014                      68 00103$:
   4014 79            [ 4]   69 	ld	a, c
   4015 D6 1E         [ 7]   70 	sub	a, #0x1e
   4017 30 09         [12]   71 	jr	NC,00101$
                             72 ;src/main.c:40: response[index] = 0;
   4019 69            [ 4]   73 	ld	l, c
   401A 26 00         [ 7]   74 	ld	h, #0x00
   401C 19            [11]   75 	add	hl, de
   401D 36 00         [10]   76 	ld	(hl), #0x00
                             77 ;src/main.c:38: for(uint8_t index = 0; index < RESPONSE_LENGTH; index++)
   401F 0C            [ 4]   78 	inc	c
   4020 18 F2         [12]   79 	jr	00103$
   4022                      80 00101$:
                             81 ;src/main.c:44: scr_set_mode(2);
   4022 D5            [11]   82 	push	de
   4023 21 02 00      [10]   83 	ld	hl, #0x0002
   4026 CD CD 42      [17]   84 	call	_scr_set_mode
   4029 CD FB 40      [17]   85 	call	_fifo_reset
   402C 21 60 40      [10]   86 	ld	hl, #___str_0
   402F E5            [11]   87 	push	hl
   4030 CD 87 40      [17]   88 	call	_send_command_to_pi
   4033 21 60 40      [10]   89 	ld	hl, #___str_0
   4036 E3            [19]   90 	ex	(sp),hl
   4037 21 66 40      [10]   91 	ld	hl, #___str_1
   403A E5            [11]   92 	push	hl
   403B CD 05 43      [17]   93 	call	_printf
   403E F1            [10]   94 	pop	af
   403F F1            [10]   95 	pop	af
   4040 D1            [10]   96 	pop	de
                             97 ;src/main.c:56: read_response_from_pi(response);
   4041 4B            [ 4]   98 	ld	c, e
   4042 42            [ 4]   99 	ld	b, d
   4043 D5            [11]  100 	push	de
   4044 C5            [11]  101 	push	bc
   4045 CD BB 40      [17]  102 	call	_read_response_from_pi
   4048 21 6B 40      [10]  103 	ld	hl, #___str_2
   404B E3            [19]  104 	ex	(sp),hl
   404C CD 05 43      [17]  105 	call	_printf
   404F F1            [10]  106 	pop	af
                            107 ;src/main.c:61: printf("Press any key to exit\r");
   4050 21 70 40      [10]  108 	ld	hl, #___str_3
   4053 E3            [19]  109 	ex	(sp),hl
   4054 CD 05 43      [17]  110 	call	_printf
   4057 F1            [10]  111 	pop	af
                            112 ;src/main.c:62: km_wait_char();
   4058 CD C8 42      [17]  113 	call	_km_wait_char
                            114 ;src/main.c:63: }
   405B DD F9         [10]  115 	ld	sp, ix
   405D DD E1         [14]  116 	pop	ix
   405F C9            [10]  117 	ret
   4060                     118 ___str_0:
   4060 50 49 4E 47         119 	.ascii "PING"
   4064 0A                  120 	.db 0x0a
   4065 00                  121 	.db 0x00
   4066                     122 ___str_1:
   4066 0D                  123 	.db 0x0d
   4067 25 73               124 	.ascii "%s"
   4069 0D                  125 	.db 0x0d
   406A 00                  126 	.db 0x00
   406B                     127 ___str_2:
   406B 25 73               128 	.ascii "%s"
   406D 0D                  129 	.db 0x0d
   406E 0D                  130 	.db 0x0d
   406F 00                  131 	.db 0x00
   4070                     132 ___str_3:
   4070 50 72 65 73 73 20   133 	.ascii "Press any key to exit"
        61 6E 79 20 6B 65
        79 20 74 6F 20 65
        78 69 74
   4085 0D                  134 	.db 0x0d
   4086 00                  135 	.db 0x00
                            136 ;src/main.c:65: void send_command_to_pi(uint8_t *command)
                            137 ;	---------------------------------
                            138 ; Function send_command_to_pi
                            139 ; ---------------------------------
   4087                     140 _send_command_to_pi::
                            141 ;src/main.c:70: while(command[counter] != 0)
   4087 0E 00         [ 7]  142 	ld	c, #0x00
   4089                     143 00101$:
   4089 21 02 00      [10]  144 	ld	hl, #2
   408C 39            [11]  145 	add	hl, sp
   408D 7E            [ 7]  146 	ld	a, (hl)
   408E 23            [ 6]  147 	inc	hl
   408F 66            [ 7]  148 	ld	h, (hl)
   4090 6F            [ 4]  149 	ld	l, a
   4091 06 00         [ 7]  150 	ld	b, #0x00
   4093 09            [11]  151 	add	hl, bc
   4094 7E            [ 7]  152 	ld	a, (hl)
   4095 B7            [ 4]  153 	or	a, a
   4096 28 03         [12]  154 	jr	Z,00114$
                            155 ;src/main.c:72: counter++;
   4098 0C            [ 4]  156 	inc	c
   4099 18 EE         [12]  157 	jr	00101$
   409B                     158 00114$:
                            159 ;src/main.c:76: for(uint8_t index = 0; index < counter; index++)
   409B 06 00         [ 7]  160 	ld	b, #0x00
   409D                     161 00106$:
   409D 78            [ 4]  162 	ld	a, b
   409E 91            [ 4]  163 	sub	a, c
   409F D0            [11]  164 	ret	NC
                            165 ;src/main.c:78: fifo_out_byte(command[ index ]);
   40A0 FD 21 02 00   [14]  166 	ld	iy, #2
   40A4 FD 39         [15]  167 	add	iy, sp
   40A6 FD 7E 00      [19]  168 	ld	a, 0 (iy)
   40A9 80            [ 4]  169 	add	a, b
   40AA 5F            [ 4]  170 	ld	e, a
   40AB FD 7E 01      [19]  171 	ld	a, 1 (iy)
   40AE CE 00         [ 7]  172 	adc	a, #0x00
   40B0 57            [ 4]  173 	ld	d, a
   40B1 1A            [ 7]  174 	ld	a, (de)
   40B2 6F            [ 4]  175 	ld	l, a
   40B3 C5            [11]  176 	push	bc
   40B4 CD 22 41      [17]  177 	call	_fifo_out_byte
   40B7 C1            [10]  178 	pop	bc
                            179 ;src/main.c:76: for(uint8_t index = 0; index < counter; index++)
   40B8 04            [ 4]  180 	inc	b
                            181 ;src/main.c:80: }
   40B9 18 E2         [12]  182 	jr	00106$
                            183 ;src/main.c:82: void read_response_from_pi(uint8_t *response)
                            184 ;	---------------------------------
                            185 ; Function read_response_from_pi
                            186 ; ---------------------------------
   40BB                     187 _read_response_from_pi::
   40BB DD E5         [15]  188 	push	ix
   40BD DD 21 00 00   [14]  189 	ld	ix,#0
   40C1 DD 39         [15]  190 	add	ix,sp
                            191 ;src/main.c:85: uint8_t did_we_get_byte = 0;
                            192 ;src/main.c:87: while((receive_byte_index == 0) || ((receive_byte_index > 0) && (response[receive_byte_index - 1] != '\n')))
   40C3 01 00 00      [10]  193 	ld	bc, #0x0000
   40C6                     194 00106$:
   40C6 79            [ 4]  195 	ld	a, c
   40C7 B7            [ 4]  196 	or	a, a
   40C8 28 14         [12]  197 	jr	Z,00101$
   40CA 79            [ 4]  198 	ld	a, c
   40CB B7            [ 4]  199 	or	a, a
   40CC 28 2A         [12]  200 	jr	Z,00109$
   40CE 59            [ 4]  201 	ld	e, c
   40CF 16 00         [ 7]  202 	ld	d, #0x00
   40D1 1B            [ 6]  203 	dec	de
   40D2 DD 6E 04      [19]  204 	ld	l, 4 (ix)
   40D5 DD 66 05      [19]  205 	ld	h, 5 (ix)
   40D8 19            [11]  206 	add	hl, de
   40D9 7E            [ 7]  207 	ld	a, (hl)
   40DA D6 0A         [ 7]  208 	sub	a, #0x0a
   40DC 28 1A         [12]  209 	jr	Z,00109$
                            210 ;src/main.c:89: while(did_we_get_byte == 0)
   40DE                     211 00101$:
   40DE 78            [ 4]  212 	ld	a, b
   40DF B7            [ 4]  213 	or	a, a
   40E0 20 11         [12]  214 	jr	NZ,00103$
                            215 ;src/main.c:92: did_we_get_byte = fifo_in_byte(&response[receive_byte_index]);
   40E2 DD 6E 04      [19]  216 	ld	l, 4 (ix)
   40E5 DD 66 05      [19]  217 	ld	h, 5 (ix)
   40E8 06 00         [ 7]  218 	ld	b, #0x00
   40EA 09            [11]  219 	add	hl, bc
   40EB C5            [11]  220 	push	bc
   40EC CD 11 41      [17]  221 	call	_fifo_in_byte
   40EF C1            [10]  222 	pop	bc
   40F0 45            [ 4]  223 	ld	b, l
   40F1 18 EB         [12]  224 	jr	00101$
   40F3                     225 00103$:
                            226 ;src/main.c:97: did_we_get_byte = 0;
   40F3 06 00         [ 7]  227 	ld	b, #0x00
                            228 ;src/main.c:100: receive_byte_index++;
   40F5 0C            [ 4]  229 	inc	c
   40F6 18 CE         [12]  230 	jr	00106$
   40F8                     231 00109$:
                            232 ;src/main.c:103: }
   40F8 DD E1         [14]  233 	pop	ix
   40FA C9            [10]  234 	ret
                            235 	.area _CODE
                            236 	.area _INITIALIZER
                            237 	.area _CABS (ABS)
