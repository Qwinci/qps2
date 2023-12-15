#include <cassert>
#include "gs.hpp"
#include <SDL.h>

constexpr int VERTICES_IN_PRIM[] {
	// point
	1,
	// line
	2,
	// line strip
	2, // first 2 and others 1
	// triangle
	3,
	// triangle strip
	3, // first 3 and others 1
	// triangle fan
	3, // first 3 and others 1
	// sprite
	2,
	// reserved
	0
};

void Gs::write_reg(uint8_t reg, uint64_t data) {
	if (reg == 0x00) {
		prim = data & 0x7FF;
	}
	else if (reg == 0x01) {
		rgbaq.red = data & 0xFF;
		rgbaq.green = data >> 8 & 0xFF;
		rgbaq.blue = data >> 16 & 0xFF;
		rgbaq.alpha = data >> 24 & 0xFF;
		rgbaq.q = data >> 32;
	}
	else if (reg == 0x04) {
		uint16_t x = data & 0xFFFF;
		uint16_t y = data >> 16 & 0xFFFF;
		uint32_t z = data >> 32 & 0xFFFFFF;
		uint8_t f = data >> 56;

		const auto& ctx = contexts[(prim & 1 << 9) ? 1 : 0];

		vertex_queue[vertex_count++] = {
			.x = static_cast<uint16_t>(x - ctx.x_off),
			.y = static_cast<uint16_t>(y - ctx.y_off),
			.z = z,
			.r = rgbaq.red,
			.g = rgbaq.green,
			.b = rgbaq.blue,
			.a = rgbaq.alpha
		};

		auto needed = VERTICES_IN_PRIM[prim & 0b111];
		if (vertex_count == needed) {
			auto type = prim & 0b111;
			if (type == 3) {
				draw_triangle();
			}
			else if (type == 6) {
				draw_sprite();
			}
			else {
				assert(false);
			}
		}
	}
	else if (reg == 0x05) {
		uint16_t x = data & 0xFFFF;
		uint16_t y = data >> 16 & 0xFFFF;
		uint16_t z = data >> 32;

		const auto& ctx = contexts[(prim & 1 << 9) ? 1 : 0];

		vertex_queue[vertex_count++] = {
			.x = static_cast<uint16_t>(x - ctx.x_off),
			.y = static_cast<uint16_t>(y - ctx.y_off),
			.z = z,
			.r = rgbaq.red,
			.g = rgbaq.green,
			.b = rgbaq.blue,
			.a = rgbaq.alpha
		};

		auto needed = VERTICES_IN_PRIM[prim & 0b111];
		if (vertex_count == needed) {
			auto type = prim & 0b111;
			if (type == 6) {
				draw_sprite();
			}
			else {
				assert(false);
			}
		}
	}
	else if (reg == 0x06) {
		contexts[0].tex.base_ptr = data & 0x3FFF;
		contexts[0].tex.buf_width = data >> 14 & 0x3F;
		contexts[0].tex.fmt = data >> 20 & 0x3F;
		contexts[0].tex.width = data >> 26 & 0xF;
		contexts[0].tex.height = data >> 30 & 0xF;
		contexts[0].tex.alpha_ctrl = data >> 34 & 1;
		contexts[0].tex.color_func = data >> 35 & 0b11;
		contexts[0].tex.clut_base_ptr = data >> 37 & 0x3FFF;
		contexts[0].tex.clut_fmt = data >> 51 & 0xF;
		contexts[0].tex.clut_entry_off = data >> 56 & 0x1F;
		contexts[0].tex.clut_cache_ctrl = data >> 61;
	}
	else if (reg == 0x07) {
		contexts[1].tex.base_ptr = data & 0x3FFF;
		contexts[1].tex.buf_width = data >> 14 & 0x3F;
		contexts[1].tex.fmt = data >> 20 & 0x3F;
		contexts[1].tex.width = data >> 26 & 0xF;
		contexts[1].tex.height = data >> 30 & 0xF;
		contexts[1].tex.alpha_ctrl = data >> 34 & 1;
		contexts[1].tex.color_func = data >> 35 & 0b11;
		contexts[1].tex.clut_base_ptr = data >> 37 & 0x3FFF;
		contexts[1].tex.clut_fmt = data >> 51 & 0xF;
		contexts[1].tex.clut_entry_off = data >> 56 & 0x1F;
		contexts[1].tex.clut_cache_ctrl = data >> 61;
	}
	else if (reg == 0x08) {
		contexts[0].tex_wrap_mode.hz_wrap_mode = data & 0b11;
		contexts[0].tex_wrap_mode.vt_wrap_mode = data >> 2 & 0b11;
		contexts[0].tex_wrap_mode.u_clamp_min = data >> 4 & 0x3FF;
		contexts[0].tex_wrap_mode.u_clamp_max = data >> 14 & 0x3FF;
		contexts[0].tex_wrap_mode.v_clamp_min = data >> 24 & 0x3FF;
		contexts[0].tex_wrap_mode.v_clamp_max = data >> 34 & 0x3FF;
	}
	else if (reg == 0x09) {
		contexts[1].tex_wrap_mode.hz_wrap_mode = data & 0b11;
		contexts[1].tex_wrap_mode.vt_wrap_mode = data >> 2 & 0b11;
		contexts[1].tex_wrap_mode.u_clamp_min = data >> 4 & 0x3FF;
		contexts[1].tex_wrap_mode.u_clamp_max = data >> 14 & 0x3FF;
		contexts[1].tex_wrap_mode.v_clamp_min = data >> 24 & 0x3FF;
		contexts[1].tex_wrap_mode.v_clamp_max = data >> 34 & 0x3FF;
	}
	else if (reg == 0x18) {
		contexts[0].x_off = data & 0xFFFF;
		contexts[0].y_off = data >> 32 & 0xFFFF;
	}
	else if (reg == 0x1A) {
		prmodecont = data & 1;
	}
	else if (reg == 0x40) {
		contexts[0].scissor.x0 = data & 0x7FF;
		contexts[0].scissor.x1 = data >> 16 & 0x7FF;
		contexts[0].scissor.y0 = data >> 32 & 0x7FF;
		contexts[0].scissor.y1 = data >> 48 & 0x7FF;
	}
	else if (reg == 0x45) {
		dither = data & 1;
	}
	else if (reg == 0x46) {
		signed_clamp = data & 1;
	}
	else if (reg == 0x47) {
		contexts[0].test.alpha_test_enabled = data & 1;
		contexts[0].test.alpha_test_method = data >> 1 & 0b111;
		contexts[0].test.alpha_ref = data >> 4 & 0xFF;
		contexts[0].test.alpha_test_fail_processing = data >> 11 & 0b11;
		contexts[0].test.dest_alpha_test_enabled = data & 1U << 14;
		contexts[0].test.dest_alpha_test_method = data >> 15 & 1;
		contexts[0].test.depth_test_enabled = data & 1U << 16;
		contexts[0].test.depth_test_method = data >> 17 & 0b11;
	}
	else if (reg == 0x4C) {
		contexts[0].frame.base_ptr = data & 0x1FF;
		contexts[0].frame.buf_width = data >> 16 & 0x3F;
		contexts[0].frame.fmt = data >> 24 & 0x3F;
		contexts[0].frame.fb_mask = data >> 32;
	}
	else if (reg == 0x4E) {
		contexts[0].z_buf.base_ptr = data & 0x1FF;
		contexts[0].z_buf.fmt = data >> 24 & 0xF;
		contexts[0].z_buf.buf_mask = data >> 32 & 1;
	}
	else if (reg == 0x50) {
		transfer.src_base_ptr = data & 0x3FFF;
		transfer.src_buf_width = data >> 16 & 0x3F;
		transfer.src_fmt = data >> 24 & 0x3F;
		transfer.dest_base_ptr = data >> 32 & 0x3FFF;
		transfer.dest_buf_width = data >> 48 & 0x3F;
		transfer.dest_fmt = data >> 56 & 0x3F;
	}
	else if (reg == 0x51) {
		transfer.src_rect_x = data & 0x7FF;
		transfer.src_rect_y = data >> 16 & 0x7FF;
		transfer.dest_rect_x = data >> 32 & 0x7FF;
		transfer.dest_rect_y = data >> 48 & 0x7FF;
		transfer.transmission_order = data >> 59 & 0b11;
	}
	else if (reg == 0x52) {
		transfer.transfer_area_width = data & 0xFFF;
		transfer.transfer_area_height = data >> 32 & 0xFFF;
	}
	else if (reg == 0x53) {
		transfer.transfer_dir = data & 0b11;
		if (transfer.transfer_dir == 2) {
			transfer.transfer_dir = 3;
			transfer.cur_src_x = transfer.src_rect_x;
			transfer.cur_src_y = transfer.src_rect_y;
			transfer.cur_dest_x = transfer.dest_rect_x;
			transfer.cur_dest_y = transfer.dest_rect_y;

			while (true) {
				uint16_t src_base_ptr = transfer.src_base_ptr * 4 * 64;
				uint16_t src_buf_width_px = transfer.src_buf_width * 64;
				uint8_t src_fmt = transfer.src_fmt;
				uint16_t dest_base_ptr = transfer.dest_base_ptr * 4 * 64;
				uint16_t dest_buf_width_px = transfer.dest_buf_width * 64;
				uint8_t dest_fmt = transfer.dest_fmt;
				assert(src_fmt == 0);
				assert(dest_fmt == 0);

				auto width = transfer.transfer_area_width;
				auto height = transfer.transfer_area_height;

				uint8_t* dest_ptr_u8 = vram.data() + dest_base_ptr;
				dest_ptr_u8 += transfer.cur_dest_y * dest_buf_width_px * 4;
				dest_ptr_u8 += transfer.cur_dest_x * 4;
				auto* dest_ptr = reinterpret_cast<uint32_t*>(dest_ptr_u8);

				uint8_t* src_ptr_u8 = vram.data() + src_base_ptr;
				src_ptr_u8 += transfer.cur_src_y * src_buf_width_px * 4;
				src_ptr_u8 += transfer.cur_src_x * 4;
				auto* src_ptr = reinterpret_cast<uint32_t*>(src_ptr_u8);

				*dest_ptr = *src_ptr;
				transfer.cur_dest_x += 1;
				transfer.cur_src_x += 1;
				// dest
				if (transfer.cur_dest_x == 2048) {
					transfer.cur_dest_x = 0;
				}
				else if (transfer.cur_dest_x >= transfer.dest_rect_x + width) {
					transfer.cur_dest_x = transfer.dest_rect_x;
					transfer.cur_dest_y += 1;
					if (transfer.cur_dest_y == 2048) {
						transfer.cur_dest_y = 0;
					}
					else if (transfer.cur_dest_y >= transfer.dest_rect_y + height) {
						transfer.in_progress = false;
						return;
					}
				}
				// src
				if (transfer.cur_src_x == 2048) {
					transfer.cur_src_x = 0;
				}
				else if (transfer.cur_src_x >= transfer.src_rect_x + width) {
					transfer.cur_src_x = transfer.src_rect_x;
					transfer.cur_src_y += 1;
					if (transfer.cur_src_y == 2048) {
						transfer.cur_src_y = 0;
					}
					else if (transfer.cur_src_y >= transfer.src_rect_y + height) {
						return;
					}
				}
			}
		}
		else if (transfer.transfer_dir == 1) {
			assert(false);
		}

		if (transfer.transfer_dir != 3) {
			transfer.in_progress = true;
			transfer.cur_src_x = transfer.src_rect_x;
			transfer.cur_src_y = transfer.src_rect_y;
			transfer.cur_dest_x = transfer.dest_rect_x;
			transfer.cur_dest_y = transfer.dest_rect_y;
		}
		else {
			transfer.in_progress = false;
		}
	}
	else {
		assert(false);
	}
}

void Gs::draw_pixel(uint16_t x, uint16_t y, uint32_t z, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	x = std::min<uint16_t>(x, SCREEN_WIDTH - 1);
	y = std::min<uint16_t>(y, SCREEN_HEIGHT - 1);

	const auto& ctx = contexts[(prim & 1 << 9) ? 1 : 0];

	assert(ctx.frame.fmt == 0);
	auto fb_base_ptr = ctx.frame.base_ptr * 4 * 2048;
	auto fb_width = ctx.frame.buf_width * 64;

	auto z_buf_base_ptr = ctx.z_buf.base_ptr * 4 * 2048;
	uint32_t z_value;
	if (ctx.z_buf.fmt == 0) {
		z_value = *(uint32_t*) &vram[z_buf_base_ptr + y * fb_width * 4 + x * 4];
	}
	else if (ctx.z_buf.fmt == 1) {
		z_value = *(uint32_t*) &vram[z_buf_base_ptr + y * fb_width * 4 + x * 4];
		z_value &= 0xFFFFFF;
	}
	else {
		assert(false);
	}

	if (ctx.test.depth_test_method == 0) {
		return;
	}
	else if (ctx.test.depth_test_method == 2 || ctx.test.depth_test_method == 3) {
		if ((ctx.test.depth_test_method == 2 && z < z_value) ||
			(ctx.test.depth_test_method == 3 && z <= z_value)) {
			return;
		}
	}

	if (!ctx.z_buf.buf_mask) {
		*(uint32_t*) &vram[z_buf_base_ptr + y * fb_width * 4 + x * 4] = z;
	}

	uint32_t rgba = *(uint32_t*) &vram[fb_base_ptr + y * fb_width * 4 + x * 4];
	uint32_t fb_value = rgba | a << 24 | b << 16 | g << 8 | r;
	uint32_t value = (fb_value & 0xFF) << 24 |
		(fb_value >> 8 & 0xFF) << 16 |
		(fb_value >> 16 & 0xFF) << 8 |
		(fb_value >> 24);

	*(uint32_t*) &vram[fb_base_ptr + y * fb_width * 4 + x * 4] = fb_value;
	target[y * SCREEN_WIDTH + x] = value;
}

void Gs::draw_sprite() {
	auto first = vertex_queue[0];
	auto second = vertex_queue[1];
	vertex_count = 0;

	assert(second.x > first.x);
	assert(second.y > first.y);

	first.x /= 16;
	first.y /= 16;
	second.x /= 16;
	second.y /= 16;

	for (uint16_t y = first.y; y < second.y; ++y) {
		for (uint16_t x = first.x; x < second.x; ++x) {
			draw_pixel(x, y, first.z, first.r, first.g, first.b, first.a);
		}
	}
}

static bool edge_function(const Gs::Vertex& a, const Gs::Vertex& b, uint32_t point_x, uint32_t point_y) {
	return (point_x - a.x) * (b.y - a.y) - (point_y - a.y) * (b.x - a.x) >= 0;
}

void Gs::draw_triangle() {
	auto first = vertex_queue[0];
	auto second = vertex_queue[1];
	auto third = vertex_queue[2];
	vertex_count = 0;

	first.x /= 16;
	first.y /= 16;
	second.x /= 16;
	second.y /= 16;
	third.x /= 16;
	third.y /= 16;

	auto left_bottom_x = first.x;
	if (second.x < left_bottom_x) left_bottom_x = second.x;
	if (third.x < left_bottom_x) left_bottom_x = third.x;

	auto left_bottom_y = first.y;
	if (second.y < left_bottom_y) left_bottom_y = second.y;
	if (third.y < left_bottom_y) left_bottom_y = third.y;

	auto right_top_x = second.x;
	if (first.x > right_top_x) right_top_x = first.x;
	if (third.x > right_top_x) right_top_x = third.x;

	auto right_top_y = second.y;
	if (first.y > right_top_y) right_top_y = first.y;
	if (third.y > right_top_y) right_top_y = third.y;

	for (uint16_t y = left_bottom_y; y < right_top_y; ++y) {
		for (uint16_t x = left_bottom_x; x < right_top_x; ++x) {
			bool inside_tri = true;
			inside_tri &= edge_function(first, second, x, y);
			inside_tri &= edge_function(second, third, x, y);
			inside_tri &= edge_function(third, first, x, y);

			if (inside_tri) {
				draw_pixel(x, y, first.z, first.r, first.g, first.b, first.a);
			}
		}
	}
}

void Gs::write_hw_reg(uint64_t data) {
	if (!transfer.in_progress) {
		assert(false && "write_hw_reg called while transfer was not in progress");
	}
	// GIF->VRAM
	if (transfer.transfer_dir == 0) {
		uint8_t src_fmt = transfer.src_fmt;
		uint16_t dest_base_ptr = transfer.dest_base_ptr * 4 * 64;
		uint16_t dest_buf_width_px = transfer.dest_buf_width * 64;
		uint8_t dest_fmt = transfer.dest_fmt;
		assert(src_fmt == 0);
		assert(dest_fmt == 0);

		auto width = transfer.transfer_area_width;
		auto height = transfer.transfer_area_height;

		uint8_t* dest_ptr = vram.data();
		dest_ptr += transfer.cur_dest_y * dest_buf_width_px * 4;
		dest_ptr += transfer.cur_dest_x * 4;
		auto* ptr = reinterpret_cast<uint32_t*>(dest_ptr);

		*ptr++ = data;
		transfer.cur_dest_x += 1;
		if (transfer.cur_dest_x == 2048) {
			transfer.cur_dest_x = 0;
		}
		else if (transfer.cur_dest_x >= transfer.dest_rect_x + width) {
			transfer.cur_dest_x = transfer.dest_rect_x;
			transfer.cur_dest_y += 1;
			if (transfer.cur_dest_y == 2048) {
				transfer.cur_dest_y = 0;
			}
			else if (transfer.cur_dest_y >= transfer.dest_rect_y + height) {
				transfer.in_progress = false;
				return;
			}
		}

		*ptr = data >> 32;
		transfer.cur_dest_x += 1;
		if (transfer.cur_dest_x == 2048) {
			transfer.cur_dest_x = 0;
		}
		else if (transfer.cur_dest_x >= transfer.dest_rect_x + width) {
			transfer.cur_dest_x = transfer.dest_rect_x;
			transfer.cur_dest_y += 1;
			if (transfer.cur_dest_y == 2048) {
				transfer.cur_dest_y = 0;
			}
			else if (transfer.cur_dest_y >= transfer.dest_rect_y + height) {
				transfer.in_progress = false;
				return;
			}
		}
	}
	else {
		assert(false);
	}
}
