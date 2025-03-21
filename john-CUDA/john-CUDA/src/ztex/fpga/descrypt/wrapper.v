`timescale 1ns / 1ps
/*
 * This software is Copyright (c) 2016 Denis Burykin
 * [denis_burykin yahoo com], [denis-burykin2014 yandex ru]
 * and it is hereby released to the general public under the following terms:
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 */

`include "descrypt_core/descrypt.vh"

module wrapper #(
	parameter N_CORES = -1,
	parameter [N_CORES*16+15 : 0] CORES_CONF = 0,
	parameter NUM_CRYPT_INSTANCES = 16
	)(
	input CORE_CLK,
	input [`DIN_MSB:0] din,
	input [2:0] addr_in,
	input [`GLOBAL_SALT_MSB:`GLOBAL_SALT_LSB] global_salt,
	
	input [N_CORES-1 :0] wr_en,
	output [N_CORES-1 :0] crypt_ready,
	output [N_CORES-1 :0] core_idle,
	output [N_CORES-1 :0] err_core,
	
	input CMP_CLK,
	output [4*N_CORES-1:0] dout,
	input [N_CORES-1 :0] rd_en,
	output [N_CORES-1 :0] empty,
	output [N_CORES-1 :0] err_core_dout,
	output reg [N_CORES-1 :0] err_cmp = 0
	);

	// Input register stages (broadcast)
	(* SHREG_EXTRACT="NO", EQUIVALENT_REGISTER_REMOVAL="NO" *)
	reg [`DIN_MSB:0] din_r1, din_r2;
	(* SHREG_EXTRACT="NO", EQUIVALENT_REGISTER_REMOVAL="NO" *)
	reg [2:0] addr_in_r1, addr_in_r2;
	
	always @(posedge CORE_CLK) begin
		din_r1 <= din;
		addr_in_r1 <= addr_in;
		
		din_r2 <= din_r1;
		addr_in_r2 <= addr_in_r1;
	end
			
	

	genvar i;
	generate
	for (i=0; i < N_CORES; i=i+1) begin:core_gen

		// INPUT_R_STAGES: 0, 1 or 2
		// *in_r1*, *in_r2*
		// OUTPUT_R_STAGES: 1 or 2
		// *out_r2*
		localparam INPUT_R_STAGES = CORES_CONF[i*16+3 : i*16+2];
		localparam OUTPUT_R_STAGES = CORES_CONF[i*16+1 : i*16+0];

		// Input register stages
		(* SHREG_EXTRACT="NO" *) reg wr_en_in_r1 = 0, wr_en_in_r2 = 0;
		(* SHREG_EXTRACT="NO" *) reg crypt_ready_in_r1, crypt_ready_in_r2;
		(* SHREG_EXTRACT="NO" *) reg core_idle_in_r1, core_idle_in_r2;
		(* SHREG_EXTRACT="NO" *) reg err_core_in_r1 = 0, err_core_in_r2 = 0;

		always @(posedge CORE_CLK) begin
			wr_en_in_r1 <= wr_en[i];
			wr_en_in_r2 <= wr_en_in_r1;

			crypt_ready_in_r1 <= INPUT_R_STAGES==1 ? crypt_ready_in : crypt_ready_in_r2;
			core_idle_in_r1 <= INPUT_R_STAGES==1 ? core_idle_in : core_idle_in_r2;
			err_core_in_r1 <= INPUT_R_STAGES==1 ? core_error : err_core_in_r2;
			
			crypt_ready_in_r2 <= crypt_ready_in;
			core_idle_in_r2 <= core_idle_in;
			err_core_in_r2 <= core_error;
		end

		assign crypt_ready[i] = INPUT_R_STAGES==0 ? crypt_ready_in : crypt_ready_in_r1;
		assign core_idle[i] = INPUT_R_STAGES==0 ? core_idle_in : core_idle_in_r1;
		assign err_core[i] = INPUT_R_STAGES==0 ? core_error : err_core_in_r1;
		
		
		// Output from the core @ CMP_CLK
		wire [3:0] core_dout;

		// Extra register stage for output (CMP_CLK)
		(* SHREG_EXTRACT="NO" *) reg [3:0] core_dout_r1, core_dout_r2; // from core
		(* SHREG_EXTRACT="NO" *) reg err_cmp_out_r1 = 0, err_cmp_out_r2 = 0;
		(* SHREG_EXTRACT="NO" *) reg core_dout_ready_out_r1, core_dout_ready_out_r2; // to core

		always @(posedge CMP_CLK) begin
			core_dout_r2 <= core_dout;
			err_cmp_out_r2 <= cmp_error;
			
			core_dout_r1 <= OUTPUT_R_STAGES==2 ? core_dout_r2 : core_dout;
			err_cmp_out_r1 <= OUTPUT_R_STAGES==2 ? err_cmp_out_r2 : cmp_error;
			err_cmp[i] <= err_cmp_out_r1;

			core_dout_ready_out_r1 <= core_dout_ready; // towards the core
			core_dout_ready_out_r2 <= core_dout_ready_out_r1;
		end
		
		(* KEEP_HIERARCHY="true" *)
		descrypt_core core(
			.CORE_CLK(CORE_CLK),
			.din(INPUT_R_STAGES==0 ? din : INPUT_R_STAGES==1 ? din_r1 : din_r2),
			.addr_in(INPUT_R_STAGES==0 ? addr_in : INPUT_R_STAGES==1 ? addr_in_r1 : addr_in_r2),
			.wr_en(INPUT_R_STAGES==0 ? wr_en[i] : INPUT_R_STAGES==1 ? wr_en_in_r1 : wr_en_in_r2),
			.global_salt(global_salt),
			
			.crypt_ready(crypt_ready_in),
			.core_idle(core_idle_in),
			.core_error(core_error),
			
			.CMP_CLK(CMP_CLK),
			.dout(core_dout),
			.dout_ready(OUTPUT_R_STAGES==2 ? core_dout_ready_out_r2 : core_dout_ready_out_r1),
			.cmp_error(cmp_error)
		);
		
		core_dout_proc core_dout_proc(
			.CLK(CMP_CLK),
			.core_dout(core_dout_r1),
			.core_dout_ready(core_dout_ready),
			
			.dout(dout[4*(i+1)-1 : 4*i]), // to arbiter
			.empty(empty[i]), .rd_en(rd_en[i]), .err_core_dout(err_core_dout[i]) 
		);
		
	end
	endgenerate


endmodule

