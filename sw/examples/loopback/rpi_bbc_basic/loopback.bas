   10 INSTALL @lib$ + "gpiolib"
   20 INSTALL @lib$ + "stringlib"
   30
   40 QUEUE_SZ%=1024
   50 DIM queue%(QUEUE_SZ%)
   60 DIM PIN_DATA%(8)
   70 DIM PMASK_DATA%(8)
   80
   90 PIN_DIR% = 17
  100 PIN_SI%  = 18
  110 PIN_SOB% = 22
  120 PIN_DOR% = 23
  130 PIN_WNR% = 24
  140
  150 REM GPIO Allocation (Data bits match PiTubeDirect)
  160 PIN_DATA%()   = 2,3,4,7,8,9,10,11 : REM index 0..7
  170 PMASK_DIR%    = 1 << PIN_DIR%    : ALL_PINS%=ALL_PINS% + PMASK_DIR%
  180 PMASK_SI%     = 1 << PIN_SI%     : ALL_PINS%=ALL_PINS% + PMASK_SI%
  190 PMASK_SOB%    = 1 << PIN_SOB%    : ALL_PINS%=ALL_PINS% + PMASK_SOB%
  200 PMASK_DOR%    = 1 << PIN_DOR%    : ALL_PINS%=ALL_PINS% + PMASK_DOR%
  210 PMASK_WNR%    = 1 << PIN_WNR%    : ALL_PINS%=ALL_PINS% + PMASK_WNR%
  220 PMASK_DATA%(0)= 1 << PIN_DATA%(0): ALL_PINS%=ALL_PINS% + PMASK_DATA%(0)
  230 PMASK_DATA%(1)= 1 << PIN_DATA%(1): ALL_PINS%=ALL_PINS% + PMASK_DATA%(1)
  240 PMASK_DATA%(2)= 1 << PIN_DATA%(2): ALL_PINS%=ALL_PINS% + PMASK_DATA%(2)
  250 PMASK_DATA%(3)= 1 << PIN_DATA%(3): ALL_PINS%=ALL_PINS% + PMASK_DATA%(3)
  260 PMASK_DATA%(4)= 1 << PIN_DATA%(4): ALL_PINS%=ALL_PINS% + PMASK_DATA%(4)
  270 PMASK_DATA%(5)= 1 << PIN_DATA%(5): ALL_PINS%=ALL_PINS% + PMASK_DATA%(5)
  280 PMASK_DATA%(6)= 1 << PIN_DATA%(6): ALL_PINS%=ALL_PINS% + PMASK_DATA%(6)
  290 PMASK_DATA%(7)= 1 << PIN_DATA%(7): ALL_PINS%=ALL_PINS% + PMASK_DATA%(7)
  300
  310 rptr%=0
  320 wptr%=0
  330 gpio%= FN_gpio_setup
  340 PROC_setup_fifo_pins
  350
  360 REM Simple byte-at-a-time loopback
  370 REPEAT
  380   IF ((wptr%+1) MOD QUEUE_SZ% <> rptr%) AND FN_gpio_get(gpio%, PIN_DOR%)<>0 THEN
  390     queue%(wptr%)=FN_read_fifo_byte
  400     wptr%=(wptr%+1) MOD QUEUE_SZ%
  410   ENDIF
  420   IF (wptr%<>rptr% AND FN_gpio_get(gpio%, PIN_DIR%)<>0) THEN
  430     PROC_write_fifo_byte(queue%(rptr%))
  440     rptr%=(rptr%+1) MOD QUEUE_SZ%
  450   ENDIF
  460 UNTIL 0
  470 END
  480
  490 DEF PROC_setup_fifo_pins
  500 REM Need to setup all pins as inputs first before setting any as outputs
  510 LOCAL p%
  520 FOR p%=0 TO 24
  530   PROC_gpio_inp(gpio%,p%)
  540 NEXT p%
  550 REM now setup selected pins to output mode and drive low initially
  560 PROC_gpio_out(gpio%,PIN_SI%)
  570 PROC_gpio_out(gpio%,PIN_SOB%)
  580 PROC_gpio_out(gpio%,PIN_WNR%)
  590 REM Default state is all outputs driving low, databus tristated
  600 PROC_gpio_clr(gpio%,ALL_PINS%)
  610 ENDPROC
  620
  630 DEF PROC_write_fifo_byte( txdata% )
  640 REM           _________
  650 REM   WNR  __/         \____
  660 REM            _______
  670 REM   DATA ---X_______X-----
  680 REM               __
  690 REM   SI  _______/  \______
  700 REM
  710
  720 LOCAL mask%, b%, data%
  730 PROC_gpio_set(gpio%,PMASK_WNR%)
  740 mask%=1
  750 data%=0
  760 FOR b%=0 TO 7
  770   REM build output data pattern and set data bus bits in output mode
  780   IF (txdata% AND mask%)<>0 THEN data%=data%+PMASK_DATA%(b%)
  790   mask%= mask%<<1
  800   PROC_gpio_out(gpio%,PIN_DATA%(b%))
  810 NEXT b%
  820
  830 PROC_gpio_set(gpio%,data%+PMASK_WNR%)
  840 PROC_gpio_set(gpio%,data%+PMASK_WNR%+PMASK_SI%)
  850 PROC_gpio_clr(gpio%,PMASK_SI%)
  860
  870 REM tristate data bus again
  880 FOR b%=0 TO 7
  890   PROC_gpio_inp(gpio%, PIN_DATA%(b%))
  900 NEXT b%
  910
  920 REM Set all pin data low (idle state)
  930 PROC_gpio_clr(gpio%,ALL_PINS%)
  940 ENDPROC
  950
  960 DEF FN_read_fifo_byte
  970 REM
  980 REM   WNR  _________________
  990 REM            _______
 1000 REM   DATA ---X_@_____X-----  @=strobe point
 1010 REM               __
 1020 REM   SOB  ______/  \______
 1030 REM
 1040 LOCAL rslt%, mask%, b%
 1050
 1060 rslt%=0
 1070 mask%=1
 1080 FOR b%=0 TO 7
 1090   bit%=FN_gpio_get(gpio%, PIN_DATA%(b%))
 1100   IF bit%<>0 THEN rslt%=rslt%+mask%
 1110   mask%=mask%<<1
 1120 NEXT b%
 1130
 1140 PROC_gpio_set(gpio%, PMASK_SOB%)
 1150 PROC_gpio_clr(gpio%, PMASK_SOB%)
 1160
 1170 =rslt%
