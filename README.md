# cpc-cplink

(n CPC-Ker-Plink)

A co-processor link card for Amstrad CPC Computers

![Board Front Image](https://raw.githubusercontent.com/revaldinho/cpc-cplink/master/doc/CPCCPLINKFront.jpg)

### Outline 
This is a card which allows you to connect an external co-processor, like a RaspberryPi,  directly to any Amstrad CPC Computer. 

The card provides an easy to use parallel interface between the processors using FIFO hardware and a simple message passing software protocol. Voltage level shifting components are included so that both 3V3 and 5V co-processors can be connected directly. Most likely the co-processor will be a RaspberryPi, but the First-In First-Out (FIFO) interface is suitable for other processors and systems too including Arduinos, Teensy and PIC MCUs.

FIFO based operation ensures all key timing requirements are handled by the board, so that
any co-processor can be easily interfaced without needing to run bare-metal, timing critical code.
e.g. A Raspberry Pi running a full Rasbian (Linux) distribution and a relatively low performance
language like Python is perfectly suitable. This kind of co-processor could make some of its own
OS resources available to the CPC, for example an SD card interface, timers, serial links etc.

The board has a convenient connector to allow a RaspberryPi Zero to be plugged directly into the
back of the card and powered from the FIFO card itself. Other Pis will fit (although access to the HDMI
port is awkward at the bottom of the card), but can more easily be connected using a short 50W ribbon 
cable. Many other CPUs including Arduino, Teensy, ChipKit, PIC MPUs can be connected with a 
suitable cable adapter. The board is compatible with both 3V3 and 5V signalling (3V3 coprocessors need
to supply a 3V3 reference voltage to the card via a dedicated pin on the connector).

### Key Features

Here is a list of the key features:

  - suitable for all CPC models
    - uses a 50W box connector for compatibility with popular expansion boards
  - compatible with external processors using either 5V or 3.3V IO signalling
  - RaspberryPi compatible socket on rear
    - plug in a Pi Zero and power directly from the FIFO card
    - other (larger) Pi models or other CPUs incl. Arduino, Teensy can connect with a cable adapter
  - Simple access protocol
    - Byte wide, 16 byte deep FIFO in both directions with status flags
    - separate status register and data register mapped into CPC IO port space
    - byte wide data bus and flags/control signals driven by coprocessor GPIO
  - Documentation and support files provided
    - programming and construction notes
    - Several examples of CPC and RaspberryPi code for simple loopback testing
      + Python and C coprocessor examples for Raspberry Pi
      + BASIC, BCPL/assembler and C examples for Amstrad CPC
    - Assember coded libraries are provided for both BCPL and C
    - Loopback examples demonstrate data rates of
      + ~50KBytes/s with the CPC checking FIFO flags byte by byte 
      + \>100KBytes/s using block transfers (coprocessor checking for overrun only)
    - Higher rates are possible with more loop unrolling/burst transfer protocols
  - Simple construction using cheap and easily available parts
    - all through-hole, dual in line package, 74 Series logic design
      - no programmable CPlD,FPGA,GAL,MPU or other parts
  - Fully open source project licensed under the GNU GPL3
    - includes all PCB layout (Eagle and gerber) and logical netlist files

### Status

The project has been fully built and tested.

An initial prototype (v0.2) was built using a Xilinx XC9536 CPLD to implement all logic. This is fully functional, but the CPLD is sadly now obsolete.

A second version of the card with all logic implemented in standard 74 series gates has been built and tested now. This is fully working and totally
software compatible with the original prototype.

For more detailed information refer to the [project wiki pages](https://github.com/revaldinho/cpc-cplink/wiki/Home).

## License

All programs and data files in this project are made available under the terms of the [GNU General Public License v3.](https://github.com/revaldinho/cpc_ram_expansion/blob/master/LICENSE)
