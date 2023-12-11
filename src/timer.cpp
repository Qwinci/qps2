#include "timer.hpp"

void Timer::clock() {
	if (!(mode & 1 << 7)) {
		return;
	}

	auto current = cycle++;

	auto clock_mode = mode & 0b11;
	constexpr int dividers[] {1, 16, 256};

	// todo proper hblank
	if (clock_mode == 3) {
		counter += 1;
	}
	else {
		if ((current & (dividers[clock_mode] - 1)) == 0) {
			counter += 1;
		}
	}
}
