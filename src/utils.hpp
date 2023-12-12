#pragma once
#include <iostream>

#define TODO(name) do { \
	std::cerr << "todo " << name << '\n'; \
	abort(); \
} while (0)

#define UNREACHABLE(msg) do { \
	std::cerr << "reached unreachable code: " << msg << '\n'; \
	abort(); \
} while (0)
