ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 1.
Hexadecimal [16-Bits]



                              1         ;; --------------------------------------------------------------
                              2         ;; fifolib.s
                              3         ;; --------------------------------------------------------------
                              4         ;;
                              5 	;; fifolib.s - an SDCC library for the CPC-CPLink board
                              6 	;; Copyright (C) 2019  Revaldinho
                              7 	;;
                              8 	;; This program is free software: you can redistribute it and/or modify
                              9 	;; it under the terms of the GNU General Public License as published by
                             10 	;; the Free Software Foundation, either version 3 of the License, or
                             11 	;; (at your option) any later version.
                             12 	;;
                             13 	;; This program is distributed in the hope that it will be useful,
                             14 	;; but WITHOUT ANY WARRANTY; without even the implied warranty of
                             15 	;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
                             16 	;; GNU General Public License for more details.
                             17 	;;
                             18 	;; You should have received a copy of the GNU General Public License
                             19 	;; along with this program.  If not, see <https://www.gnu.org/licenses/>.
                             20         ;;
                             21 
                             22 
                             23 	;; --------------------------------------------------------------
                             24 	;; fifo_reset
                             25 	;; --------------------------------------------------------------
                             26 	;;
                             27 	;; Reset the FIFO
                             28 	;;
                             29 	;; Entry
                             30 	;; - None
                             31 	;;
                             32 	;; Exit
                             33 	;; - AF Corrupt
   40FB                      34 _fifo_reset::
   40FB 3E FD         [ 7]   35         ld   a,#0xfd
   40FD D3 81         [11]   36         out  (#0x81),a        	; write to status register resets FIFO
   40FF C9            [10]   37         ret
                             38 
                             39 	;; --------------------------------------------------------------
                             40 	;; fifo_get_dor
                             41 	;; --------------------------------------------------------------
                             42 	;;
                             43 	;; Check FIFO Data Output Ready (DOR) flag and return 1 if set, or
                             44         ;; 0 otherwise
                             45 	;;
                             46 	;; Entry
                             47 	;; - None
                             48 	;;
                             49 	;; Exit
                             50 	;; - L = state of DOR
                             51 	;; - AF corrupt, all other registers preserved
                             52         ;; 
   4100                      53 _fifo_get_dor::
   4100 3E FD         [ 7]   54         ld   a,#0xfd
   4102 DB 81         [11]   55         in   a,(#0x81)         	; get status
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 2.
Hexadecimal [16-Bits]



   4104 E6 01         [ 7]   56         and  #0x1              	; isolate DOR flag
   4106 6F            [ 4]   57         ld   l,a                ; transfer to l as return value
   4107 C9            [10]   58         ret
                             59 
                             60 	;; --------------------------------------------------------------
                             61 	;; fifo_get_dir
                             62 	;; --------------------------------------------------------------
                             63 	;;
                             64 	;; Check FIFO Data Input Ready (DIR) flag and return 1 if set, or
                             65         ;; 0 otherwise
                             66 	;;
                             67 	;; Entry
                             68 	;; - None
                             69 	;;
                             70 	;; Exit
                             71 	;; - L = state of DIR
                             72 	;; - AF corrupt, all other registers preserved
                             73         ;; 
   4108                      74 _fifo_get_dir::
   4108 3E FD         [ 7]   75         ld   a,#0xfd
   410A DB 81         [11]   76         in   a,(#0x81)         	; get status
   410C E6 02         [ 7]   77         and  #0x2              	; isolate DIR flag (zero carry)
   410E 1F            [ 4]   78         rra                     ; quick shift to LSB
   410F 6F            [ 4]   79         ld   l,a                ; transfer to l as return value
   4110 C9            [10]   80         ret
                             81         
                             82         
                             83 	;; --------------------------------------------------------------
                             84 	;; fifo_in_byte    (__z88dk_fastcall)
                             85 	;; --------------------------------------------------------------
                             86 	;;
                             87 	;; Read a single byte and store it in the location pointed to by
                             88 	;; a parameter
                             89 	;;
                             90 	;; Entry
                             91 	;; - HL points to byte location for received byte
                             92 	;;
                             93 	;; Exit
                             94 	;; - L =1 if successful, otherwise zero
                             95 	;; - AF, HL corrupt
                             96 	;; - byte at (HL) updated
   4111                      97 _fifo_in_byte::
   4111 3E FD         [ 7]   98         ld   a,#0xfd
   4113 DB 81         [11]   99         in   a,(#0x81)         	; get status
   4115 E6 01         [ 7]  100         and  #0x1              	; test DOR flag
   4117 28 07         [12]  101         jr   z,fib_end       	; go to end if no data available
   4119 3E FD         [ 7]  102         ld   a,#0xfd
   411B DB 80         [11]  103         in   a,(#0x80)         	; get byte
   411D 77            [ 7]  104 	ld   (hl),a
   411E 3E 01         [ 7]  105         ld   a,#1               ; success=1
   4120                     106 fib_end:
   4120 6F            [ 4]  107         ld   l,a
   4121 C9            [10]  108         ret
                            109 
                            110 	;; --------------------------------------------------------------
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 3.
Hexadecimal [16-Bits]



                            111 	;; fifo_out_byte    (__z88dk_fastcall)
                            112 	;; --------------------------------------------------------------
                            113 	;;
                            114 	;; Write a single byte and store it in the location pointed to by
                            115 	;; a parameter.
                            116 	;;
                            117 	;; Entry
                            118 	;; - L holds byte to be written
                            119 	;;
                            120 	;; Exit
                            121 	;; - L =1 if successful, otherwise zero
                            122 	;; - AF, BC corrupt
   4122                     123 _fifo_out_byte::
   4122 01 81 FD      [10]  124         ld   bc, #0xfd81
   4125 ED 78         [12]  125         in   a,(c)         	; get dir status flag
   4127 E6 02         [ 7]  126         and  #0x2
   4129 28 05         [12]  127         jr   z,fob_end          ; go to end if no data available
   412B 0D            [ 4]  128         dec c			; point to data reg
   412C ED 69         [12]  129         out (c), l              ; write the byte
   412E 3E 01         [ 7]  130         ld   a, #0x1           	; success
   4130                     131 fob_end:
   4130 6F            [ 4]  132         ld   l,a
   4131 C9            [10]  133         ret
                            134 
                            135 	;; --------------------------------------------------------
                            136 	;; fifo_ext_param
                            137 	;; --------------------------------------------------------
                            138         ;; Service routine to extract parameters from stack
                            139         ;; and set up common start conditions for the fifo byte transfer
                            140         ;; routines
                            141         ;;
                            142         ;; Entry
                            143 	;;  (SP+6) = Num bytes
                            144 	;;  (SP+4) = RX buffer
                            145 	;;  (SP+2) = Ext Ret addr
                            146         ;;  (SP)   = Immediate ret addr
                            147 	;; Exit:
                            148         ;;  bc     = 0xfd80 (point to the data register)
                            149 	;;  hl     = pointer to num bytes
                            150 	;;   a     = num of bytes to read= 0-255
                            151         ;;   d     = num of bytes to read= 0-255 (duplicated)
                            152         ;;   e     = num of bytes to read= 0-255 (duplicated)
                            153 	;;  all other register preserved
   4132                     154 fifo_ext_param:
   4132 DD E5         [15]  155         push ix                 ; save IX (pushing every thing 2 bytes deeper)
   4134 DD 21 06 00   [14]  156         ld   ix,#6
   4138 DD 39         [15]  157         add  ix,sp
   413A 01 80 FD      [10]  158         ld   bc, #0xfd80        ; init FIFO register pointer
   413D DD 6E 00      [19]  159         ld   l,0(ix)
   4140 DD 66 01      [19]  160         ld   h,1(ix)
   4143 DD 7E 03      [19]  161         ld   a,3(ix)            ; get high byte of count
   4146 E6 FF         [ 7]  162         and  #0xff              ; check if non-zero
   4148 28 07         [12]  163         jr   z,fep_get_lowb     ; get low byte if zero
   414A 17            [ 4]  164         rla                     ; check sign bit
   414B 38 0C         [12]  165         jr   c,fep_end0         ; exit (with 0) if sign bit is set
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 4.
Hexadecimal [16-Bits]



   414D 3E FF         [ 7]  166         ld   a,#0xff            ; else, +ve, non-zero hi byte so set size to 255 max
   414F 18 03         [12]  167         jr   fep_end
   4151                     168 fep_get_lowb:
   4151 DD 7E 02      [19]  169         ld   a,2(ix)            ; get low byte of max bytes param
   4154                     170 fep_end:
   4154 57            [ 4]  171         ld   d,a
   4155 5F            [ 4]  172         ld   e,a
   4156 DD E1         [14]  173         pop  ix
   4158 C9            [10]  174         ret
   4159                     175 fep_end0:
   4159 11 00 00      [10]  176         ld   de, #0x0000        ; no bytes to read
   415C 3E 00         [ 7]  177         ld   a,#0
   415E DD E1         [14]  178         pop  ix
   4160 C9            [10]  179         ret
                            180 
                            181 	;; --------------------------------------------------------
                            182 	;; fifo_in_bytes
                            183 	;; --------------------------------------------------------
                            184 	;;
                            185 	;; Read up to 255 bytes from the FIFO terminating when the
                            186 	;; FIFO signals EMTPY and returns the actual number read.
                            187 	;;
                            188 	;; Routine will return early if a negative 16b number or 0
                            189 	;; is passed.
                            190 	;;
                            191 	;; Entry:
                            192 	;;  (SP+4) = Num bytes
                            193 	;;  (SP+2) = RX buffer
                            194 	;;  (SP)   = Ret addr
                            195 	;; Exit:
                            196 	;;  hl     = number of bytes read
                            197 	;;  RX Buffer holds bytes read
                            198 	;;  IY,IX preserved
                            199 	;;  all other register corrupted
   4161                     200 _fifo_in_bytes::
   4161 CD 32 41      [17]  201         call fifo_ext_param     ; get buffer pointer in hl, number of bytes in a
   4164 E6 FF         [ 7]  202         and  #0xFF              ; check if A is zero
   4166 28 3D         [12]  203         jr   z, fibs_end
   4168 0C            [ 4]  204         inc  c                  ; point to status register first
   4169 E6 03         [ 7]  205         and  #0x03              ; Take count modulus 4 to find entry point
   416B FE 01         [ 7]  206         cp   #1
   416D 28 29         [12]  207         jr   z, fibs_top1
   416F FE 02         [ 7]  208         cp   #2
   4171 28 1A         [12]  209         jr   z, fibs_top2
   4173 FE 03         [ 7]  210         cp   #3
   4175 28 0B         [12]  211         jr   z, fibs_top3
   4177                     212 fibs_top4:
   4177 ED 78         [12]  213         in   a,(c)              ; get dor status flag
   4179 1F            [ 4]  214         rra
   417A 30 29         [12]  215         jr   nc,fibs_end  	; go to end if no data available
   417C 0D            [ 4]  216         dec  c            	; point to data reg
   417D ED A2         [16]  217         ini               	; (hl)<-in(bc), hl++, b--
   417F 04            [ 4]  218         inc  b            	; restore b
   4180 0C            [ 4]  219         inc  c            	; point to status reg for next check
   4181 1D            [ 4]  220         dec  e            	; update counter (but no need to check for zero here)
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 5.
Hexadecimal [16-Bits]



   4182                     221 fibs_top3:
   4182 ED 78         [12]  222         in   a,(c)              ; get dor status flag
   4184 1F            [ 4]  223         rra
   4185 30 1E         [12]  224         jr   nc,fibs_end  	; go to end if no data available
   4187 0D            [ 4]  225         dec  c            	; point to data reg
   4188 ED A2         [16]  226         ini               	; (hl)<-in(bc), hl++, b--
   418A 04            [ 4]  227         inc  b            	; restore b
   418B 0C            [ 4]  228         inc  c            	; point to status reg for next check
   418C 1D            [ 4]  229         dec  e            	; update counter (but no need to check for zero here)
   418D                     230 fibs_top2:
   418D ED 78         [12]  231         in   a,(c)              ; get dor status flag
   418F 1F            [ 4]  232         rra
   4190 30 13         [12]  233         jr   nc,fibs_end  	; go to end if no data available
   4192 0D            [ 4]  234         dec  c            	; point to data reg
   4193 ED A2         [16]  235         ini               	; (hl)<-in(bc), hl++, b--
   4195 04            [ 4]  236         inc  b            	; restore b
   4196 0C            [ 4]  237         inc  c            	; point to status reg for next check
   4197 1D            [ 4]  238         dec  e            	; update counter (but no need to check for zero here)
   4198                     239 fibs_top1:
   4198 ED 78         [12]  240         in   a,(c)        	; get dor status flag
   419A 1F            [ 4]  241         rra
   419B 30 08         [12]  242         jr   nc,fibs_end  	; go to end if no data available
   419D 0D            [ 4]  243         dec  c            	; point to data reg
   419E ED A2         [16]  244         ini               	; (hl)<-in(bc), hl++, b--
   41A0 04            [ 4]  245         inc  b            	; restore b
   41A1 0C            [ 4]  246         inc  c            	; point to status reg for next check
   41A2 1D            [ 4]  247         dec  e            	; decrement counter and check if done
   41A3 20 D2         [12]  248         jr   nz,fibs_top4 	; if not loop again
   41A5                     249 fibs_end:
   41A5 7A            [ 4]  250         ld   a,d          	; restore max count
   41A6 93            [ 4]  251         sub  e            	; subtract remaining bytes
   41A7 6F            [ 4]  252         ld   l, a
   41A8 C9            [10]  253         ret
                            254 
                            255 	;; --------------------------------------------------------
                            256 	;; fifo_out_bytes
                            257 	;; --------------------------------------------------------
                            258 	;;
                            259 	;; Write up to 255 bytes from the FIFO terminating when the
                            260 	;; FIFO signals FULL and returns the actual number written.
                            261 	;;
                            262 	;; Routine will return early if a negative 16b number or 0
                            263 	;; is passed.
                            264 	;;
                            265 	;; Entry:
                            266 	;;  (SP+4) = Num bytes
                            267 	;;  (SP+2) = TX buffer
                            268 	;;  (SP)   = Ret addr
                            269 	;; Exit:
                            270 	;;  hl     = number of bytes read
                            271 	;;  RX Buffer holds bytes read
                            272 	;;  IY,IX preserved
                            273 	;;  all other register corrupted
   41A9                     274 _fifo_out_bytes::
   41A9 CD 32 41      [17]  275         call fifo_ext_param     ; get buffer pointer in hl, number of bytes in a
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 6.
Hexadecimal [16-Bits]



   41AC E6 FF         [ 7]  276         and  #0xFF
   41AE 28 41         [12]  277         jr   z, fobs_end
   41B0 0C            [ 4]  278         inc  c                  ; point to status register first
   41B1 E6 03         [ 7]  279         and  #0x03              ; Take count modulus 4 to find entry point
   41B3 FE 01         [ 7]  280         cp   #1
   41B5 28 2C         [12]  281         jr   z, fobs_top1
   41B7 FE 02         [ 7]  282         cp   #2
   41B9 28 1C         [12]  283         jr   z, fobs_top2
   41BB FE 03         [ 7]  284         cp   #3
   41BD 28 0C         [12]  285         jr   z, fobs_top3
   41BF                     286 fobs_top4:
   41BF ED 78         [12]  287         in   a,(c)              ; get dir status flag
   41C1 E6 02         [ 7]  288         and  #0x2
   41C3 28 2C         [12]  289         jr   z,fobs_end  	; go to end if no data available
   41C5 0D            [ 4]  290         dec  c            	; point to data reg
   41C6 04            [ 4]  291         inc  b                  ; pre-incr B
   41C7 ED A3         [16]  292         outi               	; b-- ; OUT(BC) <- (hl) ; hl++
   41C9 0C            [ 4]  293         inc  c            	; point to status reg for next check
   41CA 1D            [ 4]  294         dec  e            	; update counter (but no need to check for zero here)
   41CB                     295 fobs_top3:
   41CB ED 78         [12]  296         in   a,(c)              ; get dir status flag
   41CD E6 02         [ 7]  297         and  #0x2
   41CF 28 20         [12]  298         jr   z,fobs_end  	; go to end if no data available
   41D1 0D            [ 4]  299         dec  c            	; point to data reg
   41D2 04            [ 4]  300         inc  b                  ; pre-incr B
   41D3 ED A3         [16]  301         outi               	; b-- ; OUT(BC) <- (hl) ; hl++
   41D5 0C            [ 4]  302         inc  c            	; point to status reg for next check
   41D6 1D            [ 4]  303         dec  e            	; update counter (but no need to check for zero here)
   41D7                     304 fobs_top2:
   41D7 ED 78         [12]  305         in   a,(c)              ; get dir status flag
   41D9 E6 02         [ 7]  306         and  #0x2
   41DB 28 14         [12]  307         jr   z,fobs_end  	; go to end if no data available
   41DD 0D            [ 4]  308         dec  c            	; point to data reg
   41DE 04            [ 4]  309         inc  b                  ; pre-incr B
   41DF ED A3         [16]  310         outi               	; b-- ; OUT(BC) <- (hl) ; hl++
   41E1 0C            [ 4]  311         inc  c            	; point to status reg for next check
   41E2 1D            [ 4]  312         dec  e            	; update counter (but no need to check for zero here)
   41E3                     313 fobs_top1:
   41E3 ED 78         [12]  314         in   a,(c)        	; get dir status flag
   41E5 E6 02         [ 7]  315         and  #0x2
   41E7 28 08         [12]  316         jr   z,fobs_end  	; go to end if no data available
   41E9 0D            [ 4]  317         dec  c            	; point to data reg
   41EA 04            [ 4]  318         inc  b                  ; pre-incr B
   41EB ED A3         [16]  319         outi               	; b-- ; OUT(BC) <- (hl) ; hl++
   41ED 0C            [ 4]  320         inc  c            	; point to status reg for next check
   41EE 1D            [ 4]  321         dec  e            	; decrement counter and check if done
   41EF 20 CE         [12]  322         jr   nz,fobs_top4 	; if not loop again
   41F1                     323 fobs_end:
   41F1 7A            [ 4]  324         ld   a,d          	; restore max count
   41F2 93            [ 4]  325         sub  e            	; subtract remaining bytes
   41F3 6F            [ 4]  326         ld   l, a
   41F4 C9            [10]  327         ret
                            328 
                            329 	;; --------------------------------------------------------
                            330 	;; fifo_in_nc_bytes
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 7.
Hexadecimal [16-Bits]



                            331 	;; --------------------------------------------------------
                            332 	;;
                            333 	;; Read up to 255 bytes from the FIFO without looking at the
                            334 	;; status flags and terminating only when all bytes are trans-
                            335         ;; -ferred. Routine will return early if a negative 16b number
                            336         ;; or 0 is passed.
                            337 	;;
                            338 	;; Entry:
                            339 	;;  (SP+4) = Num bytes
                            340 	;;  (SP+2) = RX buffer
                            341 	;;  (SP)   = Ret addr
                            342 	;; Exit:
                            343 	;;  hl     = number of bytes read
                            344 	;;  RX Buffer holds bytes read
                            345 	;;  IY,IX preserved
                            346 	;;  all other register corrupted
   41F5                     347 _fifo_in_nc_bytes::
   41F5 CD 32 41      [17]  348         call fifo_ext_param     ; get buffer pointer in hl, number of bytes in a
   41F8 E6 FF         [ 7]  349         and  #0xFF
   41FA 28 20         [12]  350         jr   z, finbs_end
   41FC E6 03         [ 7]  351         and  #0x03              ; Take count modulus 4 to find entry point
   41FE FE 01         [ 7]  352         cp   #1
   4200 28 14         [12]  353         jr   z, finbs_top1
   4202 FE 02         [ 7]  354         cp   #2
   4204 28 0C         [12]  355         jr   z, finbs_top2
   4206 FE 03         [ 7]  356         cp   #3
   4208 28 04         [12]  357         jr   z, finbs_top3
   420A                     358 finbs_top4:
   420A ED A2         [16]  359         ini               	; (hl)<-in(bc), hl++, b--
   420C 04            [ 4]  360         inc  b            	; restore b
   420D 1D            [ 4]  361         dec  e            	; decrement counter but no need to check
   420E                     362 finbs_top3:
   420E ED A2         [16]  363         ini               	; (hl)<-in(bc), hl++, b--
   4210 04            [ 4]  364         inc  b            	; restore b
   4211 1D            [ 4]  365         dec  e            	; decrement counter but no need to check
   4212                     366 finbs_top2:
   4212 ED A2         [16]  367         ini               	; (hl)<-in(bc), hl++, b--
   4214 04            [ 4]  368         inc  b            	; restore b
   4215 1D            [ 4]  369         dec  e            	; decrement counter but no need to check
   4216                     370 finbs_top1:
   4216 ED A2         [16]  371         ini               	; (hl)<-in(bc), hl++, b--
   4218 04            [ 4]  372         inc  b            	; restore b
   4219 1D            [ 4]  373         dec  e            	; decrement counter and check if done
   421A 20 EE         [12]  374         jr   nz,finbs_top4 	; if not loop again
   421C                     375 finbs_end:
   421C 7A            [ 4]  376         ld   a,d          	; restore max count
   421D 93            [ 4]  377         sub  e            	; subtract remaining bytes
   421E 6F            [ 4]  378         ld   l, a
   421F C9            [10]  379         ret
                            380 
                            381 	;; --------------------------------------------------------
                            382 	;; fifo_out_nc_bytes
                            383 	;; --------------------------------------------------------
                            384 	;;
                            385 	;; Write up to 255 bytes from the FIFO without looking at the
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 8.
Hexadecimal [16-Bits]



                            386 	;; status flags and terminating only when all bytes are trans-
                            387         ;; -ferred. Routine will return early if a negative 16b number
                            388         ;; or 0 is passed.
                            389 	;;
                            390 	;; Entry:
                            391 	;;  (SP+4) = Num bytes
                            392 	;;  (SP+2) = TX buffer
                            393 	;;  (SP)   = Ret addr
                            394 	;; Exit:
                            395 	;;  hl     = number of bytes read
                            396 	;;  RX Buffer holds bytes read
                            397 	;;  IY,IX preserved
                            398 	;;  all other register corrupted
   4220                     399 _fifo_out_nc_bytes::
   4220 CD 32 41      [17]  400         call fifo_ext_param     ; get buffer pointer in hl, number of bytes in a
   4223 E6 FF         [ 7]  401         and  #0xFF
   4225 28 20         [12]  402         jr   z, fonbs_end
   4227 E6 03         [ 7]  403         and  #0x03              ; Take count modulus 4 to find entry point
   4229 FE 01         [ 7]  404         cp   #1
   422B 28 14         [12]  405         jr   z, fonbs_top1
   422D FE 02         [ 7]  406         cp   #2
   422F 28 0C         [12]  407         jr   z, fonbs_top2
   4231 FE 03         [ 7]  408         cp   #3
   4233 28 04         [12]  409         jr   z, fonbs_top3
   4235                     410 fonbs_top4:
   4235 04            [ 4]  411         inc  b            	; pre-inc  b
   4236 ED A3         [16]  412         outi               	; b-- ; OUT(BC)<-(HL) ; hl++
   4238 1D            [ 4]  413         dec e
   4239                     414 fonbs_top3:
   4239 04            [ 4]  415         inc  b            	; pre-inc  b
   423A ED A3         [16]  416         outi               	; b-- ; OUT(BC)<-(HL) ; hl++
   423C 1D            [ 4]  417         dec e
   423D                     418 fonbs_top2:
   423D 04            [ 4]  419         inc  b            	; pre-inc  b
   423E ED A3         [16]  420         outi               	; b-- ; OUT(BC)<-(HL) ; hl++
   4240 1D            [ 4]  421         dec e
   4241                     422 fonbs_top1:
   4241 04            [ 4]  423         inc  b            	; pre-inc  b
   4242 ED A3         [16]  424         outi               	; b-- ; OUT(BC)<-(HL) ; hl++
   4244 1D            [ 4]  425         dec  e            	; decrement counter and check if done
   4245 20 EE         [12]  426         jr   nz,fonbs_top4 	; if not loop again
   4247                     427 fonbs_end:
   4247 7A            [ 4]  428         ld   a,d                ; restore max count
   4248 93            [ 4]  429         sub  e            	; subtract remaining bytes
   4249 6F            [ 4]  430         ld   l, a
   424A C9            [10]  431         ret
