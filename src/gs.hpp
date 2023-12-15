#pragma once
#include <cstdint>
#include "utils.hpp"
#include <vector>

struct Bus;

struct Gs {
	Bus& bus;
	uint32_t* target;
	uint64_t pmode;
	uint64_t dispfb2;
	uint64_t display2;
	uint64_t csr;
	uint64_t imr;

	uint16_t prim;

	struct {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha;
		uint32_t q;
	} rgbaq;

	bool prmodecont;
	bool signed_clamp;
	bool dither;

	struct Context {
		uint16_t x_off;
		uint16_t y_off;

		struct {
			uint16_t base_ptr;
			uint8_t buf_width;
			uint8_t fmt;
			uint32_t fb_mask;
		} frame;

		struct {
			uint16_t base_ptr;
			uint8_t fmt;
			bool buf_mask;
		} z_buf;

		struct {
			uint16_t x0;
			uint16_t x1;
			uint16_t y0;
			uint16_t y1;
		} scissor;

		struct {
			bool alpha_test_enabled;
			uint8_t alpha_test_method;
			uint8_t alpha_ref;
			uint8_t alpha_test_fail_processing;
			bool dest_alpha_test_enabled;
			uint8_t dest_alpha_test_method;
			bool depth_test_enabled;
			uint8_t depth_test_method;
		} test;

		struct {
			uint8_t hz_wrap_mode;
			uint8_t vt_wrap_mode;
			uint16_t u_clamp_min;
			uint16_t u_clamp_max;
			uint16_t v_clamp_min;
			uint16_t v_clamp_max;
		} tex_wrap_mode;

		struct {
			uint16_t base_ptr;
			uint8_t buf_width;
			uint8_t fmt;
			uint8_t width;
			uint8_t height;
			uint8_t alpha_ctrl;
			uint8_t color_func;
			uint16_t clut_base_ptr;
			uint8_t clut_fmt;
			bool clut_csm2;
			uint8_t clut_entry_off;
			uint8_t clut_cache_ctrl;
		} tex;
	} contexts[2];

	struct {
		uint16_t src_base_ptr;
		uint8_t src_buf_width;
		uint8_t src_fmt;
		uint16_t dest_base_ptr;
		uint8_t dest_buf_width;
		uint8_t dest_fmt;

		uint16_t src_rect_x;
		uint16_t src_rect_y;
		uint16_t dest_rect_x;
		uint16_t dest_rect_y;
		uint8_t transmission_order;

		uint16_t transfer_area_width;
		uint16_t transfer_area_height;
		uint8_t transfer_dir;

		uint16_t cur_src_x;
		uint16_t cur_src_y;
		uint16_t cur_dest_x;
		uint16_t cur_dest_y;
		bool in_progress;
	} transfer;

	void write_reg(uint8_t reg, uint64_t data);
	void write_hw_reg(uint64_t data);

	struct Vertex {
		uint16_t x;
		uint16_t y;
		uint32_t z;
		uint8_t fog;
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	Vertex vertex_queue[3];
	uint8_t vertex_count;
	std::vector<uint8_t> vram;

	void draw_pixel(uint16_t x, uint16_t y, uint32_t z, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	void draw_sprite();
	void draw_triangle();
};

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
