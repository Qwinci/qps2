#include "bus.hpp"

int main() {
	Bus bus {"../roms/bios.bin"};
	while (true) {
		bus.cpu.clock();
	}
}
