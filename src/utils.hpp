#pragma once
#include <cstdint>
#include <iostream>

#define TODO(name) do { \
	std::cerr << "todo " << name << '\n'; \
	abort(); \
} while (0)

#define UNREACHABLE(msg) do { \
	std::cerr << "reached unreachable code: " << msg << '\n'; \
	abort(); \
} while (0)

struct Uint128 {
	uint64_t low;
	uint64_t high;
};
