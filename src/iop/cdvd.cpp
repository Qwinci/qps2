#include "cdvd.hpp"

uint8_t Cdvd::read(uint32_t addr) {
	if (addr == 0x1F402004) {
		return cur_cmd;
	}
	else if (addr == 0x1F402005) {
		return cmd_status;
	}
	else if (addr == 0x1F402006) {
		return error;
	}
	else if (addr == 0x1F402008) {
		return i_stat;
	}
	else if (addr == 0x1F40200A) {
		return drive_status;
	}
	else if (addr == 0x1F40200F) {
		return disk_type;
	}
	else if (addr == 0x1F402016) {
		return cur_s_cmd;
	}
	else if (addr == 0x1F402017) {
		return s_cmd_status;
	}
	else if (addr == 0x1F402018) {
		// todo s cmd result
		return 0;
	}
	return 0;
}

void Cdvd::write(uint32_t addr, uint8_t value) {
	if (addr == 0x1F402004) {
		cur_cmd = value;
	}
	else if (addr == 0x1F402007) {
		// todo break command
	}
	else if (addr == 0x1F402008) {
		i_stat = value;
	}
	else if (addr == 0x1F402016) {
		cur_s_cmd = value;
	}
	else if (addr == 0x1F402018) {
		s_cmd_params = value;
	}
}
