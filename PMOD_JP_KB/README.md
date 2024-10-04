# PMOD_JP_KB
Compact PCB that lets you connect a NES joypad or PS/2 keyboard to a PMOD port.  

This board is designed to be used with 3.3v PMOD ports like the ones often found on FPGA boards. It has an integrated boost converter and level shifters for interfacing with 5v devices.  
There is a signle jumper on this board, it enables a strong pull-up resistor on the data line. It is needed for PS/2 devices but may prevent NES joypads from working.  
To use this with NES joypads you must remove the original NES connector and replace it with a mini DIN6 connector. See the schematic for the pinout.  
