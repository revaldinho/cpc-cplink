/*
 *  CPC/Copro interface
 * 
 */              

module cpld_fifo(
                 // Incomplete address bus is simplify decoding in 74 series implementation
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

  wire[15:0]   address = { a15,a14,a13,a12,a11,a10,a9,a8,a7,a6,a5,a4,1'b0,1'b0,a1,a0} ;    
  wire         fifo_data_adr   = ( address == 16'hFD80);  // read/write data at same address
  wire         fifo_status_adr = ( address == 16'hFD81);  // status bits
  
  assign    o_fifo_sob  = !(wr_b | !( !ioreq_b && fifo_data_adr ));  // FIFO WCLK (acts on falling edge of sob)
  assign    o_fifo_si   = rd_b | !( !ioreq_b && fifo_data_adr );     // FIFO RCLK rising edge (also oeb while low)
  assign    o_fifo_oeb  = o_fifo_si;	    // FIFO output onto host databus only - other FIFO always enabled

  // Reset FIFO by any write to the status register
  assign    o_fifo_reset =  (!wr_b && !ioreq_b && fifo_status_adr );
  
  wire    fifo_status_oeb = !(!rd_b && !ioreq_b && fifo_status_adr );

  // FIFO IC drives databus directly using OEB to control tristate so control status bits only
  assign    data = (!fifo_status_oeb) ?  {6'b0, fifo_host_dir, fifo_host_dor}: 8'bz;
            
endmodule
