#include "bus.hpp"
#include "ee/cpu.hpp"
#include <fstream>
#include <cassert>
#include <iostream>

Bus::Bus(const std::string& bios_name) {
	bios.resize(1024 * 1024 * 4);
	main_ram.resize(1024 * 1024 * 32);
	iop_ram.resize(1024 * 1024 * 2);
	vu0_code.resize(0x1000);
	vu0_data.resize(0x1000);
	vu1_code.resize(1024 * 16);
	vu1_data.resize(1024 * 16);
	std::ifstream file {bios_name, std::ios::binary};
	file.read(reinterpret_cast<char*>(bios.data()), 1024 * 1024 * 4);
}

uint8_t Bus::read8(uint32_t addr) {
	if (addr < 0x2000000) {
		return main_ram[addr];
	}
	else if (addr >= 0x1C000000 && addr < 0x1C200000) {
		return iop_ram[addr - 0x1C000000];
	}
	else if (addr >= 0x1FC00000 && addr < 0x20000000) {
		return bios[addr - 0x1FC00000];
	}
	// unknown
	else if (addr == 0x1F803204 || (addr >= 0x1000F400 && addr < 0x1000F500) || addr == 0x1F80146E ||
		(addr >= 0x1A000000 && addr < 0x1B000000) ||
		(addr >= 0x1F803800 && addr < 0x1F803802)) {
		return 0;
	}
	else {
		std::cerr << "unimplemented read8 from "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}
}

uint16_t Bus::read16(uint32_t addr) {
	// unknown
	if (addr == 0x1A000006) {
		return 1;
	}

	return read8(addr) | read8(addr + 1) << 8;
}

uint32_t Bus::read32(uint32_t addr) {
	if (addr == 0x10000000) {
		return timers[0].counter;
	}
	// DMAC disable status
	else if (addr == 0x1000F520) {
		return dmac.enabler;
	}
	// EE INTC_STAT
	else if (addr == 0x1000F000) {
		return ee_cpu.intc_stat;
	}
	// EE INTC_MASK
	else if (addr == 0x1000F010) {
		return ee_cpu.intc_mask;
	}
	// IPU_CTRL
	else if (addr == 0x10002010) {
		return ipu.ctrl;
	}
	// GIF_STAT
	else if (addr == 0x10003020) {
		return gif.stat;
	}
	// SIF_MSCOM
	else if (addr == 0x1000F200) {
		return sif.mscom;
	}
	// SIF_MSFLG
	else if (addr == 0x1000F220) {
		return sif.msflg;
	}
	// SIF_SMFLG
	else if (addr == 0x1000F230) {
		return sif.smflg;
	}
	// unknown dram regs
	else if (addr == 0x1000F430) {
		return 0;
	}
	else if (addr == 0x1000F440) {
		uint8_t sop = mch_ricm >> 6 & 0xF;
		uint8_t sa = mch_ricm >> 16 & 0xFFF;
		if (!sop) {
			switch (sa) {
				case 0x21:
					if (rdram_dev_id < 2) {
						++rdram_dev_id;
						return 0x1F;
					}
					return 0;
				case 0x23:
					return 0x0D0D;
				case 0x24:
					return 0x90;
				case 0x40:
					return mch_ricm & 0x1F;
				default:
					break;
			}
		}
		return 0;
	}
	// DMAC SIF0
	else if (addr == 0x1000C000) {
		return dmac.sif0.chcr;
	}
	// unknown
	else if (addr == 0x1000F130 || (addr >= 0x1000F400 && addr < 0x1000F500)) {
		return 0;
	}
	// DMAC_CTRL
	else if (addr == 0x1000E000) {
		return dmac.ctrl;
	}
	// DMAC D_STAT
	else if (addr == 0x1000E010) {
		return dmac.stat;
	}
	// DMAC D_PCR
	else if (addr == 0x1000E020) {
		return dmac.pcr;
	}
	// DMAC SQWC
	else if (addr == 0x1000E030) {
		return dmac.sqwc;
	}
	// DMAC RBSR
	else if (addr == 0x1000E040) {
		return dmac.rbsr;
	}
	// DMAC RBOR
	else if (addr == 0x1000E050) {
		return dmac.rbor;
	}

	return read16(addr) | read16(addr + 2) << 16;
}

uint64_t Bus::read64(uint32_t addr) {
	return read32(addr) | static_cast<uint64_t>(read32(addr + 4)) << 32;
}

void Bus::write8(uint32_t addr, uint8_t value) {
	if (addr < 0x2000000) {
		main_ram[addr] = value;
	}
	else if (addr >= 0x1C000000 && addr < 0x1C200000) {
		iop_ram[addr - 0x1C000000] = value;
	}
	// KPUTCHAR
	else if (addr == 0x1000F180) {
		std::cout << value;
		std::cout.flush();
	}
	// unknown
	else if ((addr >= 0x1F801470 && addr < 0x1F801480) || (addr >= 0x1A000000 && addr < 0x1A100000)) {
		return;
	}
	else if (addr >= 0x11000000 && addr < 0x11001000) {
		vu0_code[addr - 0x11000000] = value;
	}
	else if (addr >= 0x11004000 && addr < 0x11005000) {
		vu0_data[addr - 0x11004000] = value;
	}
	else if (addr >= 0x11008000 && addr < 0x1100C000) {
		vu1_code[addr - 0x11008000] = value;
	}
	else if (addr >= 0x1100C000 && addr < 0x11010000) {
		vu1_data[addr - 0x1100C000] = value;
	}
	else {
		std::cerr << "unimplemented write8 to "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}
}

void Bus::write16(uint32_t addr, uint16_t value) {
	write8(addr, value);
	write8(addr + 1, value >> 8);
}

void Bus::write32(uint32_t addr, uint32_t value) {
	if (addr == 0x10000000) {
		timers[0].counter = value;
		return;
	}
	else if (addr == 0x10000010) {
		timers[0].mode = value;
		return;
	}
	else if (addr == 0x10000020) {
		timers[0].compare = value;
	}
	else if (addr == 0x10000030) {
		timers[0].hold = value;
	}
	else if (addr == 0x10000800) {
		timers[1].counter = value;
		return;
	}
	else if (addr == 0x10000810) {
		timers[1].mode = value;
		return;
	}
	else if (addr == 0x10000820) {
		timers[1].compare = value;
	}
	else if (addr == 0x10000830) {
		timers[1].hold = value;
	}
	else if (addr == 0x10001000) {
		timers[2].counter = value;
		return;
	}
	else if (addr == 0x10001010) {
		timers[2].mode = value;
		return;
	}
	else if (addr == 0x10001020) {
		timers[2].compare = value;
	}
	else if (addr == 0x10001030) {
		timers[2].hold = value;
	}
	else if (addr == 0x10001800) {
		timers[3].counter = value;
		return;
	}
	else if (addr == 0x10001810) {
		timers[3].mode = value;
		return;
	}
	else if (addr == 0x10001820) {
		timers[3].compare = value;
	}
	else if (addr == 0x10001830) {
		timers[3].hold = value;
	}
	// EE INTC_STAT
	else if (addr == 0x1000F000) {
		ee_cpu.intc_stat = value;
	}
	// EE INTC_MASK
	else if (addr == 0x1000F010) {
		ee_cpu.intc_mask = value;
	}
	// IPU_CMD
	else if (addr == 0x10002000) {
		ipu.cmd = value;
	}
	// IPU_CTRL
	else if (addr == 0x10002010) {
		ipu.ctrl = value;
	}
	// GIF_CTRL
	else if (addr == 0x10003000) {
		gif.ctrl = value;
	}
	// VIF0_FBRST
	else if (addr == 0x10003810) {
		vif0.fbrst = value;
	}
	// VIF0_ERR
	else if (addr == 0x10003820) {
		vif0.err = value;
	}
	// VIF0_MARK
	else if (addr == 0x10003830) {
		vif0.mark = value;
	}
	// VIF1_STAT
	else if (addr == 0x10003C00) {
		vif1.stat = value;
	}
	// VIF1_FBRST
	else if (addr == 0x10003C10) {
		vif1.fbrst = value;
	}
	// VIF0_FIFO
	else if (addr >= 0x10004000 && addr < 0x10005000) {
		// todo vif0 fifo
	}
	// VIF1_FIFO
	else if (addr >= 0x10005000 && addr < 0x10006000) {
		// todo vif1 fifo
	}
	// SIF_MSCOM
	else if (addr == 0x1000F200) {
		sif.mscom = value;
	}
	// SIF_MSFLG
	else if (addr == 0x1000F220) {
		sif.msflg = value;
	}
	// SIF_CTRL
	else if (addr == 0x1000F240) {
		sif.ctrl = value;
	}
	// DMAC VIF0
	else if (addr == 0x10008000) {
		dmac.vif0.chcr = value;
	}
	else if (addr == 0x10008010) {
		dmac.vif0.madr = value;
	}
	else if (addr == 0x10008020) {
		dmac.vif0.qwc = value;
	}
	else if (addr == 0x10008030) {
		dmac.vif0.tadr = value;
	}
	else if (addr == 0x10008040) {
		dmac.vif0.asr0 = value;
	}
	else if (addr == 0x10008050) {
		dmac.vif0.asr1 = value;
	}
	else if (addr == 0x10008080) {
		dmac.vif0.sadr = value;
	}
	// DMAC VIF1
	else if (addr == 0x10009000) {
		dmac.vif1.chcr = value;
	}
	else if (addr == 0x10009010) {
		dmac.vif1.madr = value;
	}
	else if (addr == 0x10009020) {
		dmac.vif1.qwc = value;
	}
	else if (addr == 0x10009030) {
		dmac.vif1.tadr = value;
	}
	else if (addr == 0x10009040) {
		dmac.vif1.asr0 = value;
	}
	else if (addr == 0x10009050) {
		dmac.vif1.asr1 = value;
	}
	else if (addr == 0x10009080) {
		dmac.vif1.sadr = value;
	}
	// DMAC GIF
	else if (addr == 0x1000A000) {
		dmac.gif.chcr = value;
	}
	else if (addr == 0x1000A010) {
		dmac.gif.madr = value;
	}
	else if (addr == 0x1000A020) {
		dmac.gif.qwc = value;
	}
	else if (addr == 0x1000A030) {
		dmac.gif.tadr = value;
	}
	else if (addr == 0x1000A040) {
		dmac.gif.asr0 = value;
	}
	else if (addr == 0x1000A050) {
		dmac.gif.asr1 = value;
	}
	else if (addr == 0x1000A080) {
		dmac.gif.sadr = value;
	}
	// DMAC IPU_FROM
	else if (addr == 0x1000B000) {
		dmac.ipu_from.chcr = value;
	}
	else if (addr == 0x1000B010) {
		dmac.ipu_from.madr = value;
	}
	else if (addr == 0x1000B020) {
		dmac.ipu_from.qwc = value;
	}
	else if (addr == 0x1000B030) {
		dmac.ipu_from.tadr = value;
	}
	else if (addr == 0x1000B040) {
		dmac.ipu_from.asr0 = value;
	}
	else if (addr == 0x1000B050) {
		dmac.ipu_from.asr1 = value;
	}
	else if (addr == 0x1000B080) {
		dmac.ipu_from.sadr = value;
	}
	// DMAC IPU_TO
	else if (addr == 0x1000B400) {
		dmac.ipu_to.chcr = value;
	}
	else if (addr == 0x1000B410) {
		dmac.ipu_to.madr = value;
	}
	else if (addr == 0x1000B420) {
		dmac.ipu_to.qwc = value;
	}
	else if (addr == 0x1000B430) {
		dmac.ipu_to.tadr = value;
	}
	else if (addr == 0x1000B440) {
		dmac.ipu_to.asr0 = value;
	}
	else if (addr == 0x1000B450) {
		dmac.ipu_to.asr1 = value;
	}
	else if (addr == 0x1000B480) {
		dmac.ipu_to.sadr = value;
	}
	// DMAC SIF0
	else if (addr == 0x1000C000) {
		dmac.sif0.chcr = value;
	}
	else if (addr == 0x1000C010) {
		dmac.sif0.madr = value;
	}
	else if (addr == 0x1000C020) {
		dmac.sif0.qwc = value;
	}
	else if (addr == 0x1000C030) {
		dmac.sif0.tadr = value;
	}
	else if (addr == 0x1000C040) {
		dmac.sif0.asr0 = value;
	}
	else if (addr == 0x1000C050) {
		dmac.sif0.asr1 = value;
	}
	else if (addr == 0x1000C080) {
		dmac.sif0.sadr = value;
	}
	// DMAC SIF1
	else if (addr == 0x1000C400) {
		dmac.sif1.chcr = value;
	}
	else if (addr == 0x1000C410) {
		dmac.sif1.madr = value;
	}
	else if (addr == 0x1000C420) {
		dmac.sif1.qwc = value;
	}
	else if (addr == 0x1000C430) {
		dmac.sif1.tadr = value;
	}
	else if (addr == 0x1000C440) {
		dmac.sif1.asr0 = value;
	}
	else if (addr == 0x1000C450) {
		dmac.sif1.asr1 = value;
	}
	else if (addr == 0x1000C480) {
		dmac.sif1.sadr = value;
	}
	// DMAC SIF2
	else if (addr == 0x1000C800) {
		dmac.sif2.chcr = value;
	}
	else if (addr == 0x1000C810) {
		dmac.sif2.madr = value;
	}
	else if (addr == 0x1000C820) {
		dmac.sif2.qwc = value;
	}
	else if (addr == 0x1000C830) {
		dmac.sif2.tadr = value;
	}
	else if (addr == 0x1000C840) {
		dmac.sif2.asr0 = value;
	}
	else if (addr == 0x1000C850) {
		dmac.sif2.asr1 = value;
	}
	else if (addr == 0x1000C880) {
		dmac.sif2.sadr = value;
	}
	// DMAC SPR_FROM
	else if (addr == 0x1000D000) {
		dmac.spr_from.chcr = value;
	}
	else if (addr == 0x1000D010) {
		dmac.spr_from.madr = value;
	}
	else if (addr == 0x1000D020) {
		dmac.spr_from.qwc = value;
	}
	else if (addr == 0x1000D030) {
		dmac.spr_from.tadr = value;
	}
	else if (addr == 0x1000D040) {
		dmac.spr_from.asr0 = value;
	}
	else if (addr == 0x1000D050) {
		dmac.spr_from.asr1 = value;
	}
	else if (addr == 0x1000D080) {
		dmac.spr_from.sadr = value;
	}
	// DMAC SPR_TO
	else if (addr == 0x1000D400) {
		dmac.spr_to.chcr = value;
	}
	else if (addr == 0x1000D410) {
		dmac.spr_to.madr = value;
	}
	else if (addr == 0x1000D420) {
		dmac.spr_to.qwc = value;
	}
	else if (addr == 0x1000D430) {
		dmac.spr_to.tadr = value;
	}
	else if (addr == 0x1000D440) {
		dmac.spr_to.asr0 = value;
	}
	else if (addr == 0x1000D450) {
		dmac.spr_to.asr1 = value;
	}
	else if (addr == 0x1000D480) {
		dmac.spr_to.sadr = value;
	}
	// DMAC D_CTRL
	else if (addr == 0x1000E000) {
		dmac.ctrl = value;
	}
	// DMAC D_STAT
	else if (addr == 0x1000E010) {
		dmac.stat = value;
	}
	// DMAC D_PCR
	else if (addr == 0x1000E020) {
		dmac.pcr = value;
	}
	// DMAC SQWC
	else if (addr == 0x1000E030) {
		dmac.sqwc = value;
	}
	// DMAC RBSR
	else if (addr == 0x1000E040) {
		dmac.rbsr = value;
	}
	// DMAC RBOR
	else if (addr == 0x1000E050) {
		dmac.rbor = value;
	}
	// unknown dram
	else if (addr == 0x1000F440) {
		mch_drd = value;
		return;
	}
	else if (addr == 0x1000F430) {
		uint8_t sa = value >> 16;
		uint8_t sbc = value >> 6 & 0xF;
		if (sa == 0x21 && sbc == 1 && (mch_drd & 1 << 7) == 0) {
			rdram_dev_id = 0;
		}
		mch_ricm = value & ~0x80000000;
		return;
	}
	// unknown
	else if ((addr >= 0x1000F100 && addr < 0x1000F180) ||
		addr == 0x1000F500 || addr == 0x1000F260 ||
		(addr >= 0x1000F400 && addr < 0x1000F520)) {

	}
	else {
		write16(addr, value);
		write16(addr + 2, value >> 16);
	}
}

void Bus::write64(uint32_t addr, uint64_t value) {
	// GS_CSR
	if (addr == 0x12001000) {
		gs.csr = value;
	}
	// GIF FIFO
	else if (addr == 0x10006000) {
		gif.fifo[0] = value;
	}
	else if (addr == 0x10006008) {
		gif.fifo[1] = value;
	}
	// IPU in FIFO
	else if (addr == 0x10007010) {
		ipu.fifo[0] = value;
	}
	else if (addr == 0x10007018) {
		ipu.fifo[1] = value;
	}
	// SMODE1, SMODE2, SRFSH, SYNCH1, SYNCH2, SYNCV
	else if (addr >= 0x12000010 && addr < 0x12000068) {

	}
	else {
		write32(addr, value);
		write32(addr + 4, value >> 32);
	}
}

void Bus::clock() {
	for (auto& timer : timers) {
		timer.clock();
	}
}
