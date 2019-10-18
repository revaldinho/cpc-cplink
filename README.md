# cpc-cplink
A co-processor link card for Amstrad CPC Computers

![Board Front Image](https://raw.githubusercontent.com/revaldinho/cpc-cplink/master/doc/CPCCPLINKFront.jpg)

### Outline 
This is a generic coprocessor interface card for the CPC using a simple message passing protocol. Voltage level shifting components are included so that both 3V3 and 5V co-processors can be connected directly. Most likely the co-processor will be a RaspberryPi, but the First-In First-Out (FIFO) interface is suitable for other processors and systems too including Arduinos, Teensy and PIC MCUs.

The FIFO is 16 bytes deep in both directions for communications between host and coprocessor. Handshaking flags need to be polled to determine when there is space to write new data into the FIFO, or when new data is available from the FIFO. 

The project provides
- PCB layout and schematics
- Logic description in Verilog for the CPLD based prototype
- Documentation
  - Circuit description and programming information
  - Bill of materials for full construction details
- Example software to demonstrate and test the interface

All code and documentation for this project is made available under the GPL3 open source license.

### Status

A first prototype has been successfully build and tested, in which the logic is implemented using a small (but sadly now obsolete) Xilinx XC9536-PC44 CPLD. 

A second version, with all logic implemented in standard 74 series gates, has been designed and is now awaiting assembly and testing.

For more detailed information refer to the [project wiki pages](https://github.com/revaldinho/cpc-cplink/wiki/CPC-CPLink).
