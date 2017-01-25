#!/bin/bash
rm kernel.xo
vivado -mode batch -source gen_xo.tcl
cp kernel.xo ../rtl_prj
