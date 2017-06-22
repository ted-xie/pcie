Steps:
1) Generate IO rtl from HLS C code.
	a) Navigate to the vhls_prj directory.
	b) vivado_hls -f test_io/solution1/script.tcl
2) Open generated project in Vivado
	a) Navigate to vv_prj directory.
	b) Open Vivado (vivado &)
	c) Open project -> ../vhls_prj/test_io/solution1/impl/verilog/project.xpr
3) Make RTL modifications
	a) Search for the "signed" operator in bandwidth.v
	b) There will be two signal assignments with the dummy addition.
	c) Both will be of the form "assign outbuffer(...) = load(AB|CD)(...) ;"
	d) loadAB is buffer[i] and loadCD is buffer[i+1]
	e) Import your custom RTL file as well as ste_sim_2x.vhd into the project.
	f) data_in0 should be loadAB, data_in1 should be loadCD.
	g) Send the "reports" vector to outbuffer.
4) Compile
	a) Navigate to rtl_prj
	b) ./rtl.sh
	c) Executables will be in prj_hw_ku3_2ddr/pkg/pcie

Example for step 3:

//assign outbuffer_data_0_fu_1626_p2 = ($signed(loadAB_phi_fu_786_p64) + $signed(ap_const_lv8_FA));

//assign outbuffer_data_1_fu_1632_p2 = ($signed(loadCD_fu_1556_p66) + $signed(ap_const_lv8_FB));

wire [7:0] automata_reports;
Donut automata (.clock(ap_clk),
                .reset(~ap_rst_n),
                .run(1),
                .data_in(loadAB_phi_fu_786_p64),
                .data_in2(loadCD_fu_1556_p66),
                .reports(automata_reports));

                
assign outbuffer_data_0_fu_1626_p2 = automata_reports;
assign outbuffer_data_1_fu_1632_p2 = automata_reports;  
