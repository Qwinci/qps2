#include "iop_bus.hpp"
#include "bus.hpp"
#include <iostream>

uint8_t IopBus::read8(uint32_t addr) {
	if (addr <= 0x200000) {
		return bus.iop_ram[addr];
	}
	else if (addr >= 0x1FC00000 && addr < 0x20000000) {
		return bus.read8(addr);
	}
	// cdvd
	else if (addr >= 0x1F402004 && addr <= 0x1F402018) {
		return cdvd.read(addr);
	}
	else {
		std::cerr << "unimplemented iop read8 from "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}
}

uint16_t IopBus::read16(uint32_t addr) {
	if (addr >= 0x1FC00000 && addr < 0x20000000) {
		return bus.read16(addr);
	}

	return read8(addr) | read8(addr + 1) << 8;
}

uint32_t IopBus::read32(uint32_t addr) {
	if (addr <= 0x200000) {
		return *(uint32_t*) &bus.iop_ram[addr];
	}

	if (addr >= 0x1FC00000 && addr < 0x20000000) {
		return bus.read32(addr);
	}
	// PS1 mode if (value & 8) != 0
	else if (addr == 0x1F801450) {
		return 0;
	}
	// unknown
	else if (addr == 0x1F801010 || addr == 0x1D000060) {
		return 0;
	}
	// DPCR
	else if (addr == 0x1F8010F0) {
		return dma.dpcr;
	}
	// DPCR2
	else if (addr == 0x1F801570) {
		return dma.dpcr2;
	}
	// DICR
	else if (addr == 0x1F8010F4) {
		return dma.dicr;
	}
	// DICR2
	else if (addr == 0x1F801574) {
		return dma.dicr2;
	}
	// DMACEN
	else if (addr == 0x1F801578) {
		return dma.dmacen;
	}
	// I_MASK
	else if (addr == 0x1F801074) {
		return i_mask;
	}
	// I_CTRL
	else if (addr == 0x1F801078) {
		return i_ctrl;
	}
	// SIF_SMCOM
	else if (addr == 0x1D000010) {
		return bus.sif.smcom;
	}
	// SIF_MSFLG
	else if (addr == 0x1D000020) {
		return bus.sif.msflg;
	}
	// SIF_SMFLG
	else if (addr == 0x1D000030) {
		return bus.sif.smflg;
	}
	// SIF_CTRL
	else if (addr == 0x1D000040) {
		return bus.sif.ctrl;
	}
	else if (addr == 0x1F8014A0) {
		return timers[4].count;
	}

	return read16(addr) | read16(addr + 2) << 16;
}

void IopBus::write8(uint32_t addr, uint8_t value) {
	if (addr <= 0x200000) {
		bus.iop_ram[addr] = value;
	}
	// cdvd
	else if (addr >= 0x1F402004 && addr <= 0x1F402018) {
		cdvd.write(addr, value);
	}
	// unknown
	else if (addr == 0x1F802070) {

	}
	else {
		std::cerr << "unimplemented iop write8 to "
		          << std::hex << std::uppercase << addr << std::dec << '\n';
		abort();
	}
}

void IopBus::write16(uint32_t addr, uint16_t value) {
	if (addr == 0x1F8014A4) {
		timers[4].mode = value;
	}
	// SIF1 D_BCR
	else if (addr == 0x1F801534) {
		dma.channels[10].bcr &= 0xFFFF0000;
		dma.channels[10].bcr |= value;
	}
	else {
		write8(addr, value);
		write8(addr + 1, value >> 8);
	}
}

void IopBus::write32(uint32_t addr, uint32_t value) {
	// unknown
	if ((addr >= 0x1F801000 && addr <= 0x1F801060) ||
		addr == 0x1F802070 || addr == 0x1FFE0130 ||
		(addr >= 0x1F801400 && addr <= 0x1F801440) ||
		(addr >= 0x1FFE0140 && addr <= 0x1FFE0144) ||
		addr == 0x1F801450 || addr == 0x1F8015F0 ||
		(addr >= 0x1F801560 && addr <= 0x1F801568)) {

	}
	// DPCR
	else if (addr == 0x1F8010F0) {
		dma.dpcr = value;
	}
	// DICR
	else if (addr == 0x1F8010F4) {
		dma.dicr = value;
	}
	// DPCR2
	else if (addr == 0x1F801570) {
		dma.dpcr = value;
	}
	// DICR2
	else if (addr == 0x1F801574) {
		dma.dicr2 = value;
	}
	// DMACEN
	else if (addr == 0x1F801578) {
		dma.dmacen = value;
	}
	// I_MASK
	else if (addr == 0x1F801074) {
		i_mask = value;
	}
	// I_CTRL
	else if (addr == 0x1F801078) {
		i_ctrl = value;
	}
	// DMA channels
	else if ((addr >= 0x1F801080 && addr <= 0x1F8010E8) ||
		(addr >= 0x1F801500 && addr <= 0x1F801558)) {
		dma.write(addr, value);
	}
	else if (addr == 0x1F8014A0) {
		timers[4].count = value;
	}
	else if (addr == 0x1F8014A8) {
		timers[4].target = value;
	}
	// SIF_SMCOM
	else if (addr == 0x1D000010) {
		bus.sif.smcom = value;
	}
	// SIF_SMFLG
	else if (addr == 0x1D000030) {
		bus.sif.smflg = value;
	}
	// SIF_CTRL
	else if (addr == 0x1D000040) {
		bus.sif.ctrl = value;
	}
	else {
		write16(addr, value);
		write16(addr + 2, value >> 16);
	}
}
