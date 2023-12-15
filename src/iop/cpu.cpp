#include "cpu.hpp"

IopCpu::IopCpu(Bus& bus) : bus {bus}, iop_bus {bus} {
	co0.get_reg(IopCop0Reg::PrId) = 0x10;
}

void IopCpu::clock() {
	auto byte = read32(pc);
	pc += 4;

	if (in_branch_delay) {
		inst_normal(byte);
		in_branch_delay = false;
		pc = new_pc;
	}
	else {
		inst_normal(byte);
	}
}

uint8_t IopCpu::read8(uint32_t addr) {
	return iop_bus.read8(virt_to_phys(addr));
}

uint16_t IopCpu::read16(uint32_t addr) {
	return iop_bus.read16(virt_to_phys(addr));
}

uint32_t IopCpu::read32(uint32_t addr) {
	return iop_bus.read32(virt_to_phys(addr));
}

void IopCpu::write8(uint32_t addr, uint8_t value) {
	// isolate cache
	if (co0.get_reg(IopCop0Reg::Sr) & 1 << 16) {
		return;
	}
	iop_bus.write8(virt_to_phys(addr), value);
}

void IopCpu::write16(uint32_t addr, uint16_t value) {
	// isolate cache
	if (co0.get_reg(IopCop0Reg::Sr) & 1 << 16) {
		return;
	}
	iop_bus.write16(virt_to_phys(addr), value);
}

void IopCpu::write32(uint32_t addr, uint32_t value) {
	// isolate cache
	if (co0.get_reg(IopCop0Reg::Sr) & 1 << 16) {
		return;
	}
	iop_bus.write32(virt_to_phys(addr), value);
}

uint32_t IopCpu::virt_to_phys(uint32_t virt) {
	return virt & 0x1FFFFFFF;
}

void IopCpu::raise_level1_exception(uint32_t vector, uint8_t cause) {
	// BEV, use bootstrap vectors
	if (co0.get_reg(IopCop0Reg::Sr) & 1 << 22) {
		if (vector != 0xBFC00000) {
			vector &= 0xFFF;
			vector |= 0xBFC00200;
		}
	}

	auto& sr = co0.get_reg(IopCop0Reg::Sr);
	// update status
	uint8_t int_status = sr & 0b111111;
	sr &= ~0b111111;
	sr |= (int_status << 2) & 0b111111;

	auto& cause_reg = co0.get_reg(IopCop0Reg::Cause);
	cause_reg &= ~(0b11111 << 2);
	cause_reg |= cause << 2;
	if (in_branch_delay) {
		co0.get_reg(IopCop0Reg::Epc) = pc - 8;
		// BD
		cause_reg |= 1U << 31;
		in_branch_delay = false;
	}
	else {
		co0.get_reg(IopCop0Reg::Epc) = pc - 4;
		cause_reg &= ~(1U << 31);
	}
	pc = vector;
}
