#include "bus.hpp"
#include "scheduler.hpp"
#include <SDL.h>
#include <cassert>

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
	auto* window = SDL_CreateWindow(
		"qps2",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
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
		bool frame_ready = false;
		bus.scheduler.schedule_event({
			.cycles = EE_CYCLES_BETWEEN_NTSC_VBLANK,
			.fn = [&]() {
				// VBLANK start
				bus.ee_cpu.raise_int0(2);
				bus.gs.csr |= 1U << 3;
				frame_ready = true;
			}
		});
		bus.scheduler.schedule_event({
			.cycles = EE_CYCLES_BETWEEN_NTSC_VBLANK + EE_CYCLES_IN_NTSC_VBLANK,
			.fn = [&]() {
				// VBLANK end
				bus.ee_cpu.raise_int0(3);
				bus.gs.csr &= ~(1U << 3);
			}
		});
		while (!frame_ready) {
			bus.scheduler.run();
		}
		auto cpu_frame_end = SDL_GetPerformanceCounter();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
			else if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
				report = true;
			}
		}

		const auto& ctx = bus.gs.contexts[(bus.gs.prim & 1 << 9) ? 1 : 0];
		assert(ctx.frame.fmt == 0);
		auto fb_base_ptr = ctx.frame.base_ptr * 4 * 2048;
		auto fb_width = ctx.frame.buf_width * 64;
		auto* vram = bus.gs.vram.data() + fb_base_ptr;

		for (uint16_t y = 0; y < SCREEN_HEIGHT; ++y) {
			for (uint16_t x = 0; x < SCREEN_WIDTH; ++x) {
				auto color = *(uint32_t*) &vram[y * fb_width * 4 + x * 4];
				uint32_t real_color = (color & 0xFF) << 24 | (color >> 8 & 0xFF) << 16 | (color >> 16 & 0xFF) << 8 | (color >> 24 & 0xFF);
				backing[y * SCREEN_WIDTH + x] = real_color;
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
