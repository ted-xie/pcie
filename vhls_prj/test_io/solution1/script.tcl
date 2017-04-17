############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2016 Xilinx, Inc. All Rights Reserved.
############################################################
open_project test_io
set_top bandwidth
add_files ../copy_kernel.c
add_files ../copy_kernel.h
open_solution "solution1"
set_part {xcku060-ffva1156-2-e} -tool vivado
create_clock -period 4 -name default
config_interface -m_axi_addr64 -m_axi_offset off -register_io off
#source "./test_io/solution1/directives.tcl"
#csim_design
csynth_design
#cosim_design
export_design -evaluate verilog -format ip_catalog
