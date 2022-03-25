      INSTALL @lib$ + "gpiolib"
      INSTALL @lib$ + "stringlib"

      QUEUE_SZ%=1024
      DIM queue%(QUEUE_SZ%)
      DIM PIN_DATA%(8)
      DIM PMASK_DATA%(8)

      PIN_DIR% = 17
      PIN_SI%  = 18
      PIN_SOB% = 22
      PIN_DOR% = 23
      PIN_WNR% = 24

      REM GPIO Allocation (Data bits match PiTubeDirect)
      PIN_DATA%()   = 2,3,4,7,8,9,10,11 : REM index 0..7
      PMASK_DIR%    = 1 << PIN_DIR%    : ALL_PINS%=ALL_PINS% + PMASK_DIR%
      PMASK_SI%     = 1 << PIN_SI%     : ALL_PINS%=ALL_PINS% + PMASK_SI%
      PMASK_SOB%    = 1 << PIN_SOB%    : ALL_PINS%=ALL_PINS% + PMASK_SOB%
      PMASK_DOR%    = 1 << PIN_DOR%    : ALL_PINS%=ALL_PINS% + PMASK_DOR%
      PMASK_WNR%    = 1 << PIN_WNR%    : ALL_PINS%=ALL_PINS% + PMASK_WNR%
      PMASK_DATA%(0)= 1 << PIN_DATA%(0): ALL_PINS%=ALL_PINS% + PMASK_DATA%(0)
      PMASK_DATA%(1)= 1 << PIN_DATA%(1): ALL_PINS%=ALL_PINS% + PMASK_DATA%(1)
      PMASK_DATA%(2)= 1 << PIN_DATA%(2): ALL_PINS%=ALL_PINS% + PMASK_DATA%(2)
      PMASK_DATA%(3)= 1 << PIN_DATA%(3): ALL_PINS%=ALL_PINS% + PMASK_DATA%(3)
      PMASK_DATA%(4)= 1 << PIN_DATA%(4): ALL_PINS%=ALL_PINS% + PMASK_DATA%(4)
      PMASK_DATA%(5)= 1 << PIN_DATA%(5): ALL_PINS%=ALL_PINS% + PMASK_DATA%(5)
      PMASK_DATA%(6)= 1 << PIN_DATA%(6): ALL_PINS%=ALL_PINS% + PMASK_DATA%(6)
      PMASK_DATA%(7)= 1 << PIN_DATA%(7): ALL_PINS%=ALL_PINS% + PMASK_DATA%(7)

      rptr%=0
      wptr%=0
      ctr%=0
      gpio%= FN_gpio_setup
      PROC_setup_fifo_pins

      REM Simple byte-at-a-time VDU processing
      REPEAT
        IF FN_gpio_get(gpio%, PIN_DOR%) THEN PRINT CHR$(FN_read_fifo_byte);
        ctr%=ctr%+1
        IF ctr% AND 7 THEN WAIT 0
      UNTIL 0
      END

      DEF PROC_setup_fifo_pins
      REM Need to setup all pins as inputs first before setting any as outputs
      LOCAL p%
      FOR p%=0 TO 24
        PROC_gpio_inp(gpio%,p%)
      NEXT p%
      REM now setup selected pins to output mode and drive low initially
      PROC_gpio_out(gpio%,PIN_SI%)
      PROC_gpio_out(gpio%,PIN_SOB%)
      PROC_gpio_out(gpio%,PIN_WNR%)
      REM Default state is all outputs driving low, databus tristated
      PROC_gpio_clr(gpio%,ALL_PINS%)
      ENDPROC

      DEF PROC_write_fifo_byte( txdata% )
      REM           _________
      REM   WNR  __/         \____
      REM            _______
      REM   DATA ---X_______X-----
      REM               __
      REM   SI  _______/  \______
      REM

      LOCAL mask%, b%, data%
      PROC_gpio_set(gpio%,PMASK_WNR%)
      mask%=1
      data%=0
      FOR b%=0 TO 7
        REM build output data pattern and set data bus bits in output mode
        IF (txdata% AND mask%)<>0 THEN data%=data%+PMASK_DATA%(b%)
        mask%= mask%<<1
        PROC_gpio_out(gpio%,PIN_DATA%(b%))
      NEXT b%

      PROC_gpio_set(gpio%,data%+PMASK_WNR%)
      PROC_gpio_set(gpio%,data%+PMASK_WNR%+PMASK_SI%)
      PROC_gpio_clr(gpio%,PMASK_SI%)

      REM tristate data bus again
      FOR b%=0 TO 7
        PROC_gpio_inp(gpio%, PIN_DATA%(b%))
      NEXT b%

      REM Set all pin data low (idle state)
      PROC_gpio_clr(gpio%,ALL_PINS%)
      ENDPROC

      DEF FN_read_fifo_byte
      REM
      REM   WNR  _________________
      REM            _______
      REM   DATA ---X___@___X-----  @=strobe point
      REM               __
      REM   SOB  ______/  \______
      REM
      LOCAL rslt%, mask%, b%

      rslt%=0
      mask%=1
      PROC_gpio_set(gpio%, PMASK_SOB%)
      FOR b%=0 TO 7
        bit%=FN_gpio_get(gpio%, PIN_DATA%(b%))
        IF bit%<>0 THEN rslt%=rslt%+mask%
        mask%=mask%<<1
      NEXT b%
      PROC_gpio_clr(gpio%, PMASK_SOB%)
      =rslt%
