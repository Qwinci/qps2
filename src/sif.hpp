#pragma once

struct Bus;

struct Sif {
	Bus& bus;
	uint32_t mscom;
	uint32_t msflg;
	uint32_t smcom;
	uint32_t smflg;
	uint32_t ctrl;
	uint8_t sif0_fifo_size;
	uint8_t sif0_fifo_ee_ptr;
	uint8_t sif0_fifo_iop_ptr;
	uint64_t sif0_fifo[16];
	uint8_t sif1_fifo_size;
	uint8_t sif1_fifo_ee_ptr;
	uint8_t sif1_fifo_iop_ptr;
	uint64_t sif1_fifo[16];
};
