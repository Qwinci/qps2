#include "bus.hpp"
#include <SDL.h>

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
	auto* window = SDL_CreateWindow(
		"qps2",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0);
	auto* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
	auto* tex = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH,
		SCREEN_HEIGHT
		);
	auto* backing = new uint32_t[SCREEN_HEIGHT * SCREEN_WIDTH];

	Bus bus {"../roms/bios.bin", backing};
	bool running = true;
	bool report = false;
	std::cerr << std::fixed;
	std::cerr.precision(64);
	auto freq = static_cast<double>(SDL_GetPerformanceFrequency());

	while (running) {
		auto frame_start = SDL_GetPerformanceCounter();
		for (size_t i = 0; i < EE_CYCLES_IN_NTSC_VBLANK; ++i) {
			bus.ee_cpu.clock();
		}
		auto cpu_frame_end = SDL_GetPerformanceCounter();

		// VBLANK start
		bus.ee_cpu.raise_int0(2);
		bus.gs.csr |= 1U << 3;
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
			else if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
				report = true;
			}
		}

		SDL_UpdateTexture(tex, nullptr, backing, SCREEN_WIDTH * 4);
		SDL_RenderCopy(renderer, tex, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		auto render_frame_end = SDL_GetPerformanceCounter();
		if (report) {
			auto cpu_time = static_cast<double>(cpu_frame_end - frame_start) / freq;
			auto render_time = static_cast<double>(render_frame_end - cpu_frame_end) / freq;
			std::cerr << "cpu frame took " << cpu_time << "s\n";
			std::cerr << "render frame took " << render_time << "s\n";
			report = false;
		}
	}

	delete[] backing;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
