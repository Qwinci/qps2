#pragma once
#include <cstdint>

struct Cdvd {
	uint8_t cur_cmd;
	uint8_t cmd_status;
	uint8_t cmd_params;
	uint8_t error;
	uint8_t i_stat;
	uint8_t drive_status;
	uint8_t disk_type;
	uint8_t cur_s_cmd;
	uint8_t s_cmd_status;
	uint8_t s_cmd_params;

	uint8_t read(uint32_t addr);
	void write(uint32_t addr, uint8_t value);
};
