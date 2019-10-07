/* 
 *  This code is part of the cpc-cplink project
 *  
 *  https://github.com/revaldinho/cpc-cplink
 *  
 *  Copyright (C) 2019 Revaldinho
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 *
 * WRITE OPERATION
 *         _____                                    _________
 * ioreq_q      \__________________________________/
 *         ________________                         _________
 * wr_b                    \_______________________/ 
 *                          _______________________
 * o_fifo_si  _____________/                       \_________
 *                    ______________________________
 * DATA    XXXXXXXXXXX_____:________________________XXXXXXXXX
 *                         :_ data written to FIFO here
 * 
 * 
 * READ OPERATION
 *         _____                                    _________
 * ioreq_q      \__________________________________/
 *         ________                                 _________
 * rd_b            \_______________________________/ 
 *                  _______________________________
 * o_fifo_sob  ____/                               \_________
 *             ____________________________________   ________
 * o_fifo_data ____________word0___________________XXX_Word 1_
 *                                                :_data sampled here
 * 
 * - SOB held low
 * - SOB goes HIGH on read starting shift of next word to output position
 * - shift is complete by end of the host read cycle and sampled
 * - SOB goes low again at end of read to complete shift operation
 * 
 * ie need to sample current data at end of host read cycle and then
 * move output data to next value.
 * 
 * IOs reserved  &FD80-&FD9F for 74 series version
 * 
 */              


// Define a NAND8 to make it easier to map logic to a 7430 later
`ifdef NAND8_D

module nand8(
             input [7:0] i,
             output o
             );
  assign o = !( &i);
endmodule
`endif


module cpld_fifo(
                 // Incomplete address bus to simplify decoding in 74 series implementation
                 input       a15,a14,a13,a12,a11,a10,a9,a8,a7,a6,a5,a4,a1,a0,
                 input       ioreq_b,
                 input       wr_b,
                 input       rd_b,
                 input       clk,
                 input       reset_b, 
                 input       fifo_host_dir,
                 input       fifo_host_dor,

                 inout [7:0] data,
                 output      wait_b, 
                 output      o_fifo_si ,
                 output      o_fifo_sob ,
                 output      o_fifo_oeb,
                 output      o_fifo_reset                
);
  // Omitting a4,a3,a2,a1 in decoding to minimise chip count
  wire[15:0]   address = { a15,a14,a13,a12,a11,a10,a9,a8,a7,a6,a5,1'b0,1'b0,1'b0,1'b0,a0} ;

`ifdef NAND8_D
  wire         decode_fd_b;
  nand8 nand8_u (
                 .i({address[15:10],address[8:7]}),
                 .o(decode_fd_b)
                 );
  wire         fifo_data_adr_b   = !( {decode_fd_b,address[9],address[6:0]} == 9'b0_0_000_0000);  // read/write data at same address
  wire         fifo_status_adr_b = !( {decode_fd_b,address[9],address[6:0]} == 9'b0_0_000_0001);  // status bits
`else
  wire         fifo_data_adr_b   = !( address == 16'hFD80);  // read/write data at same address
  wire         fifo_status_adr_b = !( address == 16'hFD81);  // status bits
`endif
  
  wire         fifo_status_oeb   =  (rd_b | ioreq_b | fifo_status_adr_b );

  assign       o_fifo_si   =  !(wr_b | ioreq_b | fifo_data_adr_b);   // FIFO WCLK   
  assign       o_fifo_sob  =  !(rd_b | ioreq_b | fifo_data_adr_b);   // FIFO RCLK
  
  assign       o_fifo_oeb  = ! o_fifo_sob;	   // Enable FIFO output onto host databus only for reads

  // Reset FIFO by any write to the status register
  assign       o_fifo_reset =  ! (wr_b | ioreq_b | fifo_status_adr_b);  

  // FIFO IC drives databus directly using OEB to control tristate so control status bits only
   assign    data = (fifo_status_oeb) ? 8'bz : {6'b0, fifo_host_dir, fifo_host_dor};
   
            
endmodule
