#pragma once
#include <vector>
#include <functional>

struct Bus;

class Scheduler {
public:
	explicit Scheduler(Bus& bus) : bus {bus} {}

	struct Event {
		size_t cycles;
		std::function<void()> fn;

		constexpr bool operator<(const Event& other) const {
			return cycles > other.cycles;
		}
	};

	void schedule_event(Event event);
	void run();
private:
	std::vector<Event> events;
	Bus& bus;
	size_t cycles {};
	size_t bus_cycles_remaining {};
	size_t iop_cycles_remaining {};
};
