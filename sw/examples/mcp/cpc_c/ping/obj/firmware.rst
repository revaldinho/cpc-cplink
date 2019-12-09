ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 1.
Hexadecimal [16-Bits]



                              1 	;; Amstrad CPC Firmware routines
                              2 	;;
                              3 	;; (c) Revaldinho 2019
                              4 	;;
                              5 	;; All use standard SDCC stack based calling convention unless marked otherwise.
                              6 	;;
                              7 	;; Parameters are pushed onto the stack from right to left, and finally the
                              8 	;; return address is pushed.
                              9 	;;
                             10 	;;     +2N-> 16n Parameter N
                             11 	;;     +4 -> 16b Parameter 2
                             12 	;;     +2 -> 16b Parameter 1
                             13 	;;  SP +0 -> [Return Address]
                             14 	;;
                             15 	;; SDCC uses IX, IY for local register pointers, so need to preserve these
                             16 	;; in firmware calls. Registers which are used in these wrappers are usually
                             17 	;; spotted and preserved before and after the call, but any registers corrupted
                             18 	;; inside firmware calls will need to be preserved explictly in the wrappers.
                             19 
                             20 	;; ====================================
                             21 	;; Graphics Calls
                             22 	;; ====================================
                             23 
                             24         ;; -------------------------------------------------------
                             25 	;; gra_set_pen - Set graphics pen ink.  (__z88dk_fastcall)
                             26         ;; -------------------------------------------------------
                             27 	;;
                             28 	;; Entry:
                             29 	;; - l = pen
                             30 	;; Exit
                             31 	;; - af corrupt
                             32 	;; - all other registers preserved
   424B                      33 _gra_set_pen::
   424B 7D            [ 4]   34         ld      a, l
   424C CD DE BB      [17]   35         call    #0xbbde
   424F C9            [10]   36         ret
                             37         ;; -------------------------------------------------------
                             38         ;; gra_line_abs - draw a line to an absolute position
                             39         ;; -------------------------------------------------------
                             40 	;;
                             41 	;; Entry:
                             42 	;; - Param 1 = X
                             43 	;; - Param 2 = Y
                             44 	;;
                             45 	;; Exit:
                             46 	;; - af corrupt
                             47 	;; - de, hl used in wrapper
                             48 	;; - all other registers preserved
   4250                      49 _gra_line_abs::
   4250 DD E5         [15]   50 	push 	ix
   4252 DD 21 04 00   [14]   51         ld      ix,#4
   4256 DD 39         [15]   52         add     ix,sp
                             53 
   4258 DD 5E 00      [19]   54 	ld 	e, 0(ix)
   425B DD 56 01      [19]   55 	ld 	d, 1(ix)
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 2.
Hexadecimal [16-Bits]



   425E DD 6E 02      [19]   56 	ld 	l, 2(ix)
   4261 DD 66 03      [19]   57 	ld 	h, 3(ix)
   4264 C5            [11]   58 	push	bc
   4265 CD F6 BB      [17]   59         call    #0xbbf6
   4268 C1            [10]   60 	pop	bc
   4269 DD E1         [14]   61 	pop	ix
   426B C9            [10]   62         ret
                             63         ;; ---------------------------------------------------
                             64         ;; gra_move_abs - Move graphics cursor to an absolute position
                             65         ;; ---------------------------------------------------
                             66 	;;
                             67 	;; Entry:
                             68 	;; - Param 1 = X
                             69 	;; - Param 2 = Y
                             70 	;;
                             71 	;; Exit:
                             72 	;; - af corrupt
                             73 	;; - de, hl used in wrapper
                             74 	;; - all other registers preserved
   426C                      75 _gra_move_abs::
   426C DD E5         [15]   76 	push 	ix
   426E DD 21 04 00   [14]   77         ld      ix,#4
   4272 DD 39         [15]   78         add     ix,sp
                             79 
   4274 DD 5E 00      [19]   80 	ld 	e, 0(ix)
   4277 DD 56 01      [19]   81 	ld 	d, 1(ix)
   427A DD 6E 02      [19]   82 	ld 	l, 2(ix)
   427D DD 66 03      [19]   83 	ld 	h, 3(ix)
   4280 C5            [11]   84 	push	bc
   4281 CD C0 BB      [17]   85         call    #0xbbc0
   4284 C1            [10]   86 	pop	bc
   4285 DD E1         [14]   87 	pop	ix
   4287 C9            [10]   88         ret
                             89 
                             90         ;; ---------------------------------------------------
                             91         ;; gra_plot_abs - Plot a point at an absolute position
                             92         ;; ---------------------------------------------------
                             93 	;;
                             94 	;; Entry:
                             95 	;; - Param 1 = X
                             96 	;; - Param 2 = Y
                             97 	;;
                             98 	;; Exit:
                             99 	;; - af corrupt
                            100 	;; - de, hl used in wrapper
                            101 	;; - all other registers preserved
   4288                     102 _gra_plot_abs::
   4288 DD E5         [15]  103 	push 	ix
   428A DD 21 04 00   [14]  104         ld      ix,#4
   428E DD 39         [15]  105         add     ix,sp
                            106 
   4290 DD 5E 00      [19]  107 	ld 	e, 0(ix)
   4293 DD 56 01      [19]  108 	ld 	d, 1(ix)
   4296 DD 6E 02      [19]  109 	ld 	l, 2(ix)
   4299 DD 66 03      [19]  110 	ld 	h, 3(ix)
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 3.
Hexadecimal [16-Bits]



   429C C5            [11]  111 	push	bc
   429D CD EA BB      [17]  112         call    #0xbbea
   42A0 C1            [10]  113 	pop	bc
   42A1 DD E1         [14]  114 	pop	ix
   42A3 C9            [10]  115         ret
                            116 
                            117         ;; ---------------------------------------------------
                            118 	;; gra_set_origin  - set the origin of the user coordinates
                            119         ;; ---------------------------------------------------
                            120 	;;
                            121 	;; Entry:
                            122 	;; - Param 1 = X
                            123 	;; - Param 2 = Y
                            124 	;;
                            125 	;; Exit:
                            126 	;; - af corrupt
                            127 	;; - de, hl used in wrapper
                            128 	;; - all other registers preserved
   42A4                     129 _gra_set_origin::
   42A4 DD E5         [15]  130 	push 	ix
   42A6 DD 21 04 00   [14]  131         ld      ix,#4
   42AA DD 39         [15]  132         add     ix,sp
                            133 
   42AC DD 5E 00      [19]  134 	ld 	e, 0(ix)
   42AF DD 56 01      [19]  135 	ld 	d, 1(ix)
   42B2 DD 6E 02      [19]  136 	ld 	l, 2(ix)
   42B5 DD 66 03      [19]  137 	ld 	h, 3(ix)
   42B8 C5            [11]  138 	push	bc
   42B9 CD C9 BB      [17]  139         call    #0xbbc9
   42BC C1            [10]  140 	pop	bc
   42BD DD E1         [14]  141 	pop	ix
   42BF C9            [10]  142         ret
                            143 
                            144 	;; ====================================
                            145 	;; Kernel Block Calls
                            146 	;; ====================================
                            147 
                            148         ;; ---------------------------------------------------
                            149 	;; kl_time_please - Ask elapsed time
                            150         ;; ---------------------------------------------------
                            151 	;;
                            152 	;; Entry
                            153 	;; - no parameters
                            154 	;; Exit
                            155 	;; - <de,hl> - time in 1/300th sec (32bit long int)
                            156 	;; - all other registers preserved
   42C0                     157 _kl_time_please::
   42C0 CD 0D BD      [17]  158         call    #0xbd0d
   42C3 C9            [10]  159         ret
                            160 
                            161         ;; ---------------------------------------------------
                            162 	;; kl_time_set - set elapsed time (usu. to zero) (__z88dk_fastcall)
                            163         ;; ---------------------------------------------------
                            164 	;;
                            165 	;; Entry
ASxxxx Assembler V02.00 + NoICE + SDCC mods  (Zilog Z80 / Hitachi HD64180), page 4.
Hexadecimal [16-Bits]



                            166 	;; <dehl> - time in 1/300th sec (32b long int)
                            167 	;; Exit
                            168 	;; - af corrupt
                            169 	;; - all registers preserved
   42C4                     170 _kl_time_set::
   42C4 CD 10 BD      [17]  171         call    #0xbd10
   42C7 C9            [10]  172         ret
                            173 
                            174 	;; ====================================
                            175 	;; Keyboard Manager Calls
                            176 	;; ====================================
                            177 
                            178         ;; ---------------------------------------------------
                            179 	;; km_wait_char - Wait for a key press and return ASCII value (__z88dk_fastcall)
                            180         ;; ---------------------------------------------------
                            181 	;;
                            182 	;;
                            183 	;; Entry:
                            184 	;; - none
                            185 	;; Exit:
                            186 	;; - carry - True
                            187 	;; - l     - character
                            188 	;; - af corrupt
                            189 	;; - all other registers preserved
   42C8                     190 _km_wait_char::
   42C8 CD 06 BB      [17]  191 	call 	#0xbb06
   42CB 6F            [ 4]  192 	ld 	l,a
   42CC C9            [10]  193 	ret
                            194 
                            195 	;; ====================================
                            196 	;; Screen Pack Calls
                            197 	;; ====================================
                            198 
                            199         ;; ---------------------------------------------------
                            200         ;; scr_set_mode - set screen to new mode (__z88dk_fastcall)
                            201         ;; ---------------------------------------------------
                            202 	;;
                            203 	;; Entry:
                            204 	;; - l - new mode
                            205 	;; Exit:
                            206 	;; - af corrupt
                            207 	;; - all other registers preserved
   42CD                     208 _scr_set_mode::
   42CD 7D            [ 4]  209         ld      a,l
   42CE E5            [11]  210 	push	hl
   42CF C5            [11]  211 	push	bc
   42D0 D5            [11]  212 	push	de
   42D1 CD 0E BC      [17]  213         call    #0xbc0e
   42D4 D1            [10]  214 	pop	de
   42D5 C1            [10]  215 	pop	bc
   42D6 E1            [10]  216 	pop	hl
   42D7 C9            [10]  217         ret
