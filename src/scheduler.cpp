#include "scheduler.hpp"
#include <algorithm>
#include "bus.hpp"

void Scheduler::schedule_event(Scheduler::Event event) {
	event.cycles += cycles;
	events.push_back(std::move(event));
	std::push_heap(events.begin(), events.end());
}

void Scheduler::run() {
	size_t run_cycles = 512;
	if (!events.empty() && events.front().cycles - cycles < run_cycles) {
		run_cycles = events.front().cycles - cycles;
	}

	for (size_t i = 0; i < run_cycles; ++i) {
		bus.ee_cpu.clock();
	}

	size_t bus_cycles = run_cycles / 2;
	bus_cycles_remaining += run_cycles % 2;
	if (bus_cycles_remaining == 2) {
		bus_cycles_remaining = 0;
		++bus_cycles;
	}

	for (size_t i = 0; i < bus_cycles; ++i) {
		bus.clock();
	}

	size_t iop_cycles = run_cycles / 8;
	iop_cycles_remaining += run_cycles % 8;
	if (iop_cycles_remaining >= 8) {
		iop_cycles_remaining -= 8;
		++iop_cycles;
	}
	for (size_t i = 0; i < iop_cycles; ++i) {
		bus.iop_cpu.clock();
	}

	cycles += run_cycles;

	while (!events.empty() && events.front().cycles <= cycles) {
		auto event = events.front();
		std::pop_heap(events.begin(), events.end());
		events.pop_back();
		event.fn();
	}
}
