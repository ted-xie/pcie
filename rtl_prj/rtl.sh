#!/bin/bash
set -e

cd ../vv_prj
sh ip.sh
sh xo.sh
cd ../rtl_prj
sdaccel run_ku3_2ddr.tcl
