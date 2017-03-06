# PFC
TMS320F28377F code for TENS applications with close-loop stabilization (TENS: Transcutaneous Electrical Nerve Stimulation)

This code is only a basic software architecture made for TMS320F28377F whose objetive is to support various nerve-stimulation-signal adjustment algorythms using raw data from any kind of sensor (i.e. skin impedance).

It is needed to include some libraries and compiler/linker configuration. To do so, follow these instructions:

1) Do as it is told in the document:
  - F2837xS Firmware Development Package
    -> http://www.ti.com/lit/ug/spruhy0/spruhy0.pdf
  Chapter 2.2, from point 1 to 7 (of 8)
  
2) Afterwards, copy both folders 'src' and 'include' in the project

3) Lastly, use the linker command files '2837xS_RAM_CLA_lnk_cpu1_2.cmd' and '2837xS_FLASH_CLA_lnk_cpu1_2.cmd'
