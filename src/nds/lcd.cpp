// GB Enhanced Copyright Daniel Baxter 2013
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : lcd.cpp
// Date : August 16, 2014
// Description : NDS LCD emulation
//
// Draws background, window, and sprites to screen
// Responsible for blitting pixel data and limiting frame rate

#include <cmath>

#include "lcd.h"
#include "common/util.h"

/****** LCD Constructor ******/
NTR_LCD::NTR_LCD()
{
	window = NULL;
	reset();
}

/****** LCD Destructor ******/
NTR_LCD::~NTR_LCD()
{
	screen_buffer.clear();

	scanline_buffer_a.clear();
	scanline_buffer_b.clear();

	render_buffer_a.clear();
	render_buffer_b.clear();

	SDL_DestroyWindow(window);

	std::cout<<"LCD::Shutdown\n";
}

/****** Reset LCD ******/
void NTR_LCD::reset()
{
	final_screen = NULL;
	original_screen = NULL;
	mem = NULL;

	if((window != NULL) && (config::sdl_render)) { SDL_DestroyWindow(window); }
	window = NULL;

	screen_buffer.clear();
	gx_screen_buffer.clear();

	scanline_buffer_a.clear();
	scanline_buffer_b.clear();

	render_buffer_a.clear();
	render_buffer_b.clear();
	gx_render_buffer.clear();
	gx_z_buffer.clear();

	sfx_buffer.clear();

	lcd_stat.lcd_clock = 0;
	lcd_stat.lcd_mode = 0;

	frame_start_time = 0;
	frame_current_time = 0;
	fps_count = 0;
	fps_time = 0;

	lcd_stat.current_scanline = 0;
	scanline_pixel_counter = 0;

	lcd_stat.lyc_nds9 = 0;
	lcd_stat.lyc_nds7 = 0;

	//Screen + render buffer initialization
	screen_buffer.resize(0x18000, 0);
	gx_screen_buffer.resize(2);
	gx_screen_buffer[0].resize(0xC000, 0);
	gx_screen_buffer[1].resize(0xC000, 0);

	scanline_buffer_a.resize(0x100, 0);
	scanline_buffer_b.resize(0x100, 0);

	render_buffer_a.resize(0x100, 0);
	render_buffer_b.resize(0x100, 0);
	gx_render_buffer.resize(0xC000, 0);
	gx_z_buffer.resize(0xC000, -4096);

	sfx_buffer.resize(0x100, 0);

	line_buffer.resize(4);
	for(u32 x = 0; x < 4; x++) { line_buffer[x].resize(0x100); }

	full_scanline_render_a = false;
	full_scanline_render_b = false;

	//BG palette initialization
	lcd_stat.bg_pal_update_a = true;
	lcd_stat.bg_pal_update_list_a.resize(0x100, true);

	lcd_stat.bg_pal_update_b = true;
	lcd_stat.bg_pal_update_list_b.resize(0x100, true);

	lcd_stat.bg_ext_pal_update_a = true;
	lcd_stat.bg_ext_pal_update_list_a.resize(0x4000, true);

	lcd_stat.bg_ext_pal_update_b = true;
	lcd_stat.bg_ext_pal_update_list_b.resize(0x4000, true);

	//OBJ palette initialization
	lcd_stat.obj_pal_update_a = true;
	lcd_stat.obj_pal_update_list_a.resize(0x100, true);

	lcd_stat.obj_pal_update_b = true;
	lcd_stat.obj_pal_update_list_b.resize(0x100, true);

	lcd_stat.obj_ext_pal_update_a = true;
	lcd_stat.obj_ext_pal_update_list_a.resize(0x1000, true);

	lcd_stat.obj_ext_pal_update_b = true;
	lcd_stat.obj_ext_pal_update_list_b.resize(0x1000, true);

	//SFX and Window initialization
	lcd_stat.current_sfx_type_a = NDS_NORMAL;
	lcd_stat.current_sfx_type_b = NDS_NORMAL;

	//OAM initialization
	lcd_stat.oam_update = true;
	lcd_stat.oam_update_list.resize(0x100, true);

	lcd_stat.update_bg_control_a = false;
	lcd_stat.update_bg_control_b = false;

	lcd_stat.display_mode_a = 0;
	lcd_stat.display_mode_b = 0;

	lcd_stat.obj_boundary_a = 32;
	lcd_stat.obj_boundary_b = 32;

	lcd_stat.ext_pal_a = 0;
	lcd_stat.ext_pal_b = 0;

	lcd_stat.display_stat_nds9 = 0;
	lcd_stat.display_stat_nds7 = 0;

	lcd_stat.bg_mode_a = 0;
	lcd_stat.bg_mode_b = 0;
	lcd_stat.hblank_interval_free = false;

	lcd_stat.master_bright = 0;
	lcd_stat.old_master_bright = 0;

	lcd_stat.forced_blank_a = false;
	lcd_stat.forced_blank_b = false;

	lcd_stat.vblank_irq_enable_a = false;
	lcd_stat.hblank_irq_enable_a = false;
	lcd_stat.vcount_irq_enable_a = false;

	lcd_stat.vblank_irq_enable_b = false;
	lcd_stat.hblank_irq_enable_b = false;
	lcd_stat.vcount_irq_enable_b = false;

	//Misc BG initialization
	for(int x = 0; x < 4; x++)
	{
		lcd_stat.bg_control_a[x] = 0;
		lcd_stat.bg_control_b[x] = 0;

		lcd_stat.bg_priority_a[x] = 0;
		lcd_stat.bg_priority_b[x] = 0;

		lcd_stat.bg_offset_x_a[x] = 0;
		lcd_stat.bg_offset_x_b[x] = 0;

		lcd_stat.bg_offset_y_a[x] = 0;
		lcd_stat.bg_offset_y_b[x] = 0;

		lcd_stat.bg_depth_a[x] = 4;
		lcd_stat.bg_depth_b[x] = 4;

		lcd_stat.bg_size_a[x] = 0;
		lcd_stat.bg_size_b[x] = 0;

		lcd_stat.text_width_a[x] = 0;
		lcd_stat.text_width_b[x] = 0;
		lcd_stat.text_height_a[x] = 0;
		lcd_stat.text_height_b[x] = 0;

		lcd_stat.bg_base_tile_addr_a[x] = 0x6000000;
		lcd_stat.bg_base_tile_addr_b[x] = 0x6000000;

		lcd_stat.bg_base_map_addr_a[x] = 0x6000000;
		lcd_stat.bg_base_map_addr_b[x] = 0x6000000;

		lcd_stat.bg_enable_a[x] = false;
		lcd_stat.bg_enable_b[x] = false;
	}

	//BG2/3 affine parameters + bitmap base addrs
	//Window coordinates
	for(int x = 0; x < 2; x++)
	{
		lcd_stat.bg_affine_a[x].overflow = false;
		lcd_stat.bg_affine_a[x].dmx = lcd_stat.bg_affine_a[x].dy = 0.0;
		lcd_stat.bg_affine_a[x].dx = lcd_stat.bg_affine_a[x].dmy = 1.0;
		lcd_stat.bg_affine_a[x].x_ref = lcd_stat.bg_affine_a[x].y_ref = 0.0;
		lcd_stat.bg_affine_a[x].x_pos = lcd_stat.bg_affine_a[x].y_pos = 0.0;

		lcd_stat.bg_affine_b[x].overflow = false;
		lcd_stat.bg_affine_b[x].dmx = lcd_stat.bg_affine_b[x].dy = 0.0;
		lcd_stat.bg_affine_b[x].dx = lcd_stat.bg_affine_b[x].dmy = 1.0;
		lcd_stat.bg_affine_a[x].x_ref = lcd_stat.bg_affine_b[x].y_ref = 0.0;
		lcd_stat.bg_affine_b[x].x_pos = lcd_stat.bg_affine_b[x].y_pos = 0.0;

		lcd_stat.bg_bitmap_base_addr_a[x] = 0x6000000;
		lcd_stat.bg_bitmap_base_addr_b[x] = 0x6000000;

		lcd_stat.window_x_a[0][x] = 0;
		lcd_stat.window_x_a[1][x] = 0;

		lcd_stat.window_x_b[0][x] = 0;
		lcd_stat.window_x_b[1][x] = 0;

		lcd_stat.window_y_a[0][x] = 0;
		lcd_stat.window_y_a[1][x] = 0;

		lcd_stat.window_y_b[0][x] = 0;
		lcd_stat.window_y_b[1][x] = 0;

		lcd_stat.window_enable_a[x] = false;
		lcd_stat.window_enable_b[x] = false;

		lcd_stat.obj_win_enable_a = false;
		lcd_stat.obj_win_enable_b = false;

		for(int y = 0; y < 6; y++)
		{
			lcd_stat.window_in_enable_a[y][x] = false;
			lcd_stat.window_in_enable_b[y][x] = false;

			lcd_stat.window_out_enable_a[y][x] = false;
			lcd_stat.window_out_enable_b[y][x] = false;
		}

		lcd_stat.current_window_a = 0;
		lcd_stat.current_window_b = 0;
	}

	//OBJ affine parameters
	for(int x = 0; x < 256; x++)
	{
		lcd_stat.obj_affine[x] = 0.0;
	}

	//VRAM blocks
	lcd_stat.vram_bank_addr[0] = 0x6800000;
	lcd_stat.vram_bank_addr[1] = 0x6820000;
	lcd_stat.vram_bank_addr[2] = 0x6840000;
	lcd_stat.vram_bank_addr[3] = 0x6860000;
	lcd_stat.vram_bank_addr[4] = 0x6880000;
	lcd_stat.vram_bank_addr[5] = 0x6890000;
	lcd_stat.vram_bank_addr[6] = 0x6894000;
	lcd_stat.vram_bank_addr[7] = 0x6898000;
	lcd_stat.vram_bank_addr[8] = 0x68A0000;

	lcd_stat.vram_bank_enable[0] = false;
	lcd_stat.vram_bank_enable[1] = false;
	lcd_stat.vram_bank_enable[2] = false;
	lcd_stat.vram_bank_enable[3] = false;
	lcd_stat.vram_bank_enable[4] = false;
	lcd_stat.vram_bank_enable[5] = false;
	lcd_stat.vram_bank_enable[6] = false;
	lcd_stat.vram_bank_enable[7] = false;
	lcd_stat.vram_bank_enable[8] = false;

	//Inverse LUT
	for(int x = 0, y = 7; x < 8; x++, y--) { inv_lut[x] = y; }

	//Screen offset LUT
	for(int x = 0; x < 512; x++)
	{
		screen_offset_lut[x] = (x > 255) ? 0x800 : 0x0;
	}

	lcd_3D_stat.display_control = 0;
	lcd_3D_stat.current_gx_command = 0;
	lcd_3D_stat.parameter_index = 0;
	lcd_3D_stat.buffer_id = 0;
	lcd_3D_stat.gx_state = 0;
	lcd_3D_stat.process_command = false;
	lcd_3D_stat.packed_command = false;
	lcd_3D_stat.render_polygon = false;

	lcd_3D_stat.view_port_x1 = 0;
	lcd_3D_stat.view_port_x2 = 0;
	lcd_3D_stat.view_port_y1 = 0;
	lcd_3D_stat.view_port_y2 = 0;

	lcd_3D_stat.matrix_mode = 0;
	lcd_3D_stat.vertex_mode = 0;
	lcd_3D_stat.vertex_list_index = 0;

	lcd_3D_stat.rear_plane_color = 0;
	lcd_3D_stat.rear_plane_alpha = 0;
	lcd_3D_stat.vertex_color = 0;
	lcd_3D_stat.clip_flags = 0;

	lcd_3D_stat.last_x = 0;
	lcd_3D_stat.last_y = 0;
	lcd_3D_stat.last_z = 0;

	//3D GFX command parameters
	for(int x = 0; x < 128; x++) { lcd_3D_stat.command_parameters[x] = 0; }
	
	//Vertex colors
	for(int x = 0; x < 4; x++) { vert_colors[x] = 0xFFFFFFFF; }

	//Polygon fill coordinates
	for(int x = 0; x < 256; x++) { lcd_3D_stat.hi_fill[x] = lcd_3D_stat.lo_fill[x] = 0xFF; }

	//Polygon fill Z coordinates
	for(int x = 0; x < 256; x++) { lcd_3D_stat.hi_line_z[x] = lcd_3D_stat.lo_line_z[x] = 0.0; }

	//Polygon vertices
	gx_triangles.clear();
	gx_quads.clear();

	//GX Matrices
	gx_projection_matrix.resize(4, 4);
	gx_position_matrix.resize(4, 4);
	gx_vector_matrix.resize(4, 4);
	gx_texture_matrix.resize(4, 4);

	//GX Matrix Stacks
	gx_projection_stack.resize(1);
	gx_position_stack.resize(31);
	gx_vector_stack.resize(31);
	gx_texture_stack.resize(1);

	//Matrix Stack Pointers
	position_sp = 0;
	vector_sp = 0;

	//Initialize system screen dimensions
	config::sys_width = 256;
	config::sys_height = 384;

	max_fullscreen_ratio = 2;
}

/****** Initialize LCD with SDL ******/
bool NTR_LCD::init()
{
	//Initialize with SDL rendering software or hardware
	if(config::sdl_render)
	{
		//Initialize all of SDL
		if(SDL_Init(SDL_INIT_VIDEO) == -1)
		{
			std::cout<<"LCD::Error - Could not initialize SDL video\n";
			return false;
		}

		//Setup OpenGL rendering
		if(config::use_opengl)
		{
			if(!opengl_init())
			{
				std::cout<<"LCD::Error - Could not initialize OpenGL\n";
				return false;
			}
		}

		//Set up software rendering
		else
		{
			window = SDL_CreateWindow("GBE+", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config::sys_width, config::sys_height, config::flags);
			SDL_GetWindowSize(window, &config::win_width, &config::win_height);
			final_screen = SDL_GetWindowSurface(window);
			original_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, config::sys_width, config::sys_height, 32, 0, 0, 0, 0);
			config::scaling_factor = 1;
		}

		if(final_screen == NULL) { return false; }

		SDL_SetWindowIcon(window, util::load_icon(config::data_path + "icons/gbe_plus.bmp"));
	}

	//Initialize with only a buffer for OpenGL (for external rendering)
	else if((!config::sdl_render) && (config::use_opengl))
	{
		final_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, config::sys_width, config::sys_height, 32, 0, 0, 0, 0);
	}

	std::cout<<"LCD::Initialized\n";

	return true;
}

/****** Updates OAM entries when values in memory change ******/
void NTR_LCD::update_oam()
{
	lcd_stat.oam_update = false;
	
	u32 oam_ptr = 0x7000000;
	u16 attribute = 0;

	u16 a_bound = (lcd_stat.display_control_a & 0x400000) ? 256 : 128;
	u8 bitmap_mask_a = (lcd_stat.display_control_a & 0x20) ? 0x1F : 0xF;
	u8 bitmap_mask_b = (lcd_stat.display_control_b & 0x20) ? 0x1F : 0xF; 

	for(int x = 0; x < 256; x++)
	{
		//Update if OAM entry has changed
		if(lcd_stat.oam_update_list[x])
		{
			lcd_stat.oam_update_list[x] = false;

			//Read and parse Attribute 0
			attribute = mem->read_u16_fast(oam_ptr);
			oam_ptr += 2;

			obj[x].y = (attribute & 0xFF);
			obj[x].affine_enable = (attribute & 0x100) ? 1 : 0;
			obj[x].type = (attribute & 0x200) ? 1 : 0;
			obj[x].mode = (attribute >> 10) & 0x3;
			obj[x].mosiac = (attribute >> 12) & 0x1;
			obj[x].bit_depth = (attribute & 0x2000) ? 8 : 4;
			obj[x].shape = (attribute >> 14);

			if((obj[x].affine_enable == 0) && (obj[x].type == 1)) { obj[x].visible = false; }
			else { obj[x].visible = true; }

			//Read and parse Attribute 1
			attribute = mem->read_u16_fast(oam_ptr);
			oam_ptr += 2;

			obj[x].x = (attribute & 0x1FF);
			obj[x].h_flip = (attribute & 0x1000) ? true : false;
			obj[x].v_flip = (attribute & 0x2000) ? true : false;
			obj[x].size = (attribute >> 14);

			if(obj[x].affine_enable) { obj[x].affine_group = (attribute >> 9) & 0x1F; }

			//Read and parse Attribute 2
			attribute = mem->read_u16_fast(oam_ptr);
			oam_ptr += 4;

			obj[x].tile_number = (attribute & 0x3FF);
			obj[x].bg_priority = ((attribute >> 10) & 0x3);
			obj[x].palette_number = ((attribute >> 12) & 0xF); 

			//Determine dimensions of the sprite
			switch(obj[x].size)
			{
				//Size 0 - 8x8, 16x8, 8x16
				case 0x0:
					if(obj[x].shape == 0) { obj[x].width = 8; obj[x].height = 8; }
					else if(obj[x].shape == 1) { obj[x].width = 16; obj[x].height = 8; }
					else if(obj[x].shape == 2) { obj[x].width = 8; obj[x].height = 16; }
					break;

				//Size 1 - 16x16, 32x8, 8x32
				case 0x1:
					if(obj[x].shape == 0) { obj[x].width = 16; obj[x].height = 16; }
					else if(obj[x].shape == 1) { obj[x].width = 32; obj[x].height = 8; }
					else if(obj[x].shape == 2) { obj[x].width = 8; obj[x].height = 32; }
					break;

				//Size 2 - 32x32, 32x16, 16x32
				case 0x2:
					if(obj[x].shape == 0) { obj[x].width = 32; obj[x].height = 32; }
					else if(obj[x].shape == 1) { obj[x].width = 32; obj[x].height = 16; }
					else if(obj[x].shape == 2) { obj[x].width = 16; obj[x].height = 32; }
					break;

				//Size 3 - 64x64, 64x32, 32x64
				case 0x3:
					if(obj[x].shape == 0) { obj[x].width = 64; obj[x].height = 64; }
					else if(obj[x].shape == 1) { obj[x].width = 64; obj[x].height = 32; }
					else if(obj[x].shape == 2) { obj[x].width = 32; obj[x].height = 64; }
					break;
			}

			//Precalulate OBJ boundaries
			obj[x].left = obj[x].x;
			obj[x].right = (obj[x].x + obj[x].width - 1) & 0x1FF;

			obj[x].top = obj[x].y;
			obj[x].bottom = (obj[x].y + obj[x].height - 1) & 0xFF;

			//Precalculate OBJ wrapping
			if(obj[x].left > obj[x].right) 
			{
				obj[x].x_wrap = true;
				obj[x].x_wrap_val = (obj[x].width - obj[x].right - 1);
			}

			else { obj[x].x_wrap = false; }

			if(obj[x].top > obj[x].bottom)
			{
				obj[x].y_wrap = true;
				obj[x].y_wrap_val = (obj[x].height - obj[x].bottom - 1);
			}

			else { obj[x].y_wrap = false; }

			//Precalculate OBJ base address
			u8 boundary = (x < 128) ? lcd_stat.obj_boundary_a : lcd_stat.obj_boundary_b;
			u32 base = (x < 128) ? 0x6400000 : 0x6600000;

			//Tiled OBJ address calculation
			if(obj[x].mode != 3) { obj[x].addr = base + (obj[x].tile_number * boundary); }
			
			//Bitmap OBJ address calculation
			else
			{
				//Engine A - 1D
				if((x < 128) && (lcd_stat.display_control_a & 0x40)) { obj[x].addr = base + (obj[x].tile_number * a_bound); }

				//Engine B - 1D - Force 128 pixel boundary
				else if(lcd_stat.display_control_b & 0x40) { obj[x].addr = base + (obj[x].tile_number * 128); }

				//Engine A - 2D
				else if(x < 128) { obj[x].addr = base + ((obj[x].tile_number & bitmap_mask_a) * 0x10) + ((obj[x].tile_number & ~bitmap_mask_a) * 0x80); }

				//Engine B - 2D
				else { obj[x].addr = base + ((obj[x].tile_number & bitmap_mask_a) * 0x10) + ((obj[x].tile_number & ~bitmap_mask_a) * 0x80); }
			}

			//Read and parse OAM affine attribute
			attribute = mem->read_u16_fast(oam_ptr - 2);
			
			//Only update if this attribute is non-zero
			if(attribute)
			{	
				if(attribute & 0x8000) 
				{ 
					u16 temp = ((attribute >> 8) - 1);
					temp = (~temp & 0xFF);
					lcd_stat.obj_affine[x] = -1.0 * temp;
				}

				else { lcd_stat.obj_affine[x] = (attribute >> 8); }

				if((attribute & 0xFF) != 0) { lcd_stat.obj_affine[x] += (attribute & 0xFF) / 256.0; }
			}

			else { lcd_stat.obj_affine[x] = 0.0; }
		}

		else { oam_ptr += 8; }
	}

	//Update OBJ for affine transformations
	update_obj_affine_transformation();

	//Update render list for the current scanline
	update_obj_render_list();
}

/****** Updates the size and position of OBJs from affine transformation ******/
void NTR_LCD::update_obj_affine_transformation()
{
	//Cycle through all OAM entries
	for(int x = 0; x < 256; x++)
	{
		//Determine if affine transformations occur on this OBJ
		if(obj[x].affine_enable)
		{
			//Process regular-sized OBJs
			if(!obj[x].type)
			{
				//Find half width and half height
				obj[x].cw = obj[x].width >> 1;
				obj[x].ch = obj[x].height >> 1;

				//Find OBJ center
				obj[x].cx = obj[x].x + obj[x].cw;
				obj[x].cy = obj[x].y + obj[x].ch;

				if(obj[x].cx > 0xFF) { obj[x].cx -= 0x1FF; }
				if(obj[x].cy > 0xFF) { obj[x].cy -= 0xFF; }
			}

			//Process double-sized OBJs
			else
			{
				//Find half width and half height
				obj[x].cw = obj[x].width >> 1;
				obj[x].ch = obj[x].height >> 1;

				//Find OBJ center
				obj[x].cx = (obj[x].x + obj[x].width);
				obj[x].cy = (obj[x].y + obj[x].height);

				if(obj[x].cx > 0xFF) { obj[x].cx -= 0x1FF; }
				if(obj[x].cy > 0xFF) { obj[x].cy -= 0xFF; }

				obj[x].left = obj[x].x;
				obj[x].top = obj[x].y;

				obj[x].right = (obj[x].left + (obj[x].width << 1) - 1) & 0x1FF;
				obj[x].bottom = (obj[x].top + (obj[x].height << 1) - 1) & 0xFF;

				//Precalculate OBJ wrapping
				if(obj[x].left > obj[x].right)
				{
					obj[x].x_wrap = true;
					obj[x].x_wrap_val = ((obj[x].width << 1) - obj[x].right - 1);
				}

				else { obj[x].x_wrap = false; }

				if(obj[x].top > obj[x].bottom)
				{
					obj[x].y_wrap = true;
					obj[x].y_wrap_val = ((obj[x].height << 1) - obj[x].bottom - 1);
				}

				else { obj[x].y_wrap = false; }
			}
		}
	}
}

/****** Updates a list of OBJs to render on the current scanline ******/
void NTR_LCD::update_obj_render_list()
{
	obj_render_length_a = 0;
	obj_render_length_b = 0;

	//Sort them based on BG priorities
	for(int bg = 0; bg < 4; bg++)
	{
		//Cycle through all of the OBJs
		for(int x = 0; x < 256; x++)
		{	
			//Check to see if sprite is rendered on the current scanline
			if(!obj[x].visible) { continue; }
			else if((!obj[x].y_wrap) && ((lcd_stat.current_scanline < obj[x].top) || (lcd_stat.current_scanline > obj[x].bottom))) { continue; }
			else if((obj[x].y_wrap) && ((lcd_stat.current_scanline > obj[x].bottom) && (lcd_stat.current_scanline < obj[x].top))) { continue; }

			else if(obj[x].bg_priority == bg)
			{
				if(x < 128) { obj_render_list_a[obj_render_length_a++] = x; }
				else { obj_render_list_b[obj_render_length_b++] = x; } 
			}
		}
	}
}

/****** Updates palette entries when values in memory change ******/
void NTR_LCD::update_palettes()
{
	//Update BG palettes - Engine A
	if(lcd_stat.bg_pal_update_a)
	{
		lcd_stat.bg_pal_update_a = false;

		//Cycle through all updates to BG palettes
		for(int x = 0; x < 256; x++)
		{
			//If this palette has been updated, convert to ARGB
			if(lcd_stat.bg_pal_update_list_a[x])
			{
				lcd_stat.bg_pal_update_list_a[x] = false;

				u16 color_bytes = mem->read_u16_fast(0x5000000 + (x << 1));
				lcd_stat.raw_bg_pal_a[x] = color_bytes;

				u8 red = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 green = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 blue = ((color_bytes & 0x1F) << 3);

				lcd_stat.bg_pal_a[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
			}
		}
	}

	//Update Extended BG palettes - Engine A
	if(lcd_stat.bg_ext_pal_update_a)
	{
		lcd_stat.bg_ext_pal_update_a = false;

		//Cycle through all updates to Extended BG palettes
		for(int x = 0; x < 0x4000; x++)
		{
			//If this palette has been updated, convert to ARGB
			if(lcd_stat.bg_ext_pal_update_list_a[x])
			{
				lcd_stat.bg_ext_pal_update_list_a[x] = false;

				u16 color_bytes = mem->read_u16_fast(0x6880000 + (x << 1));
				lcd_stat.raw_bg_ext_pal_a[x] = color_bytes;

				u8 red = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 green = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 blue = ((color_bytes & 0x1F) << 3);

				lcd_stat.bg_ext_pal_a[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
			}
		}
	}

	//Update Extended BG palettes - Engine B
	if(lcd_stat.bg_ext_pal_update_b)
	{
		lcd_stat.bg_ext_pal_update_b = false;

		//Cycle through all updates to Extended BG palettes
		for(int x = 0; x < 0x4000; x++)
		{
			//If this palette has been updated, convert to ARGB
			if(lcd_stat.bg_ext_pal_update_list_b[x])
			{
				lcd_stat.bg_ext_pal_update_list_b[x] = false;

				u16 color_bytes = mem->read_u16_fast(0x6898000 + (x << 1));
				lcd_stat.raw_bg_ext_pal_b[x] = color_bytes;

				u8 red = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 green = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 blue = ((color_bytes & 0x1F) << 3);

				lcd_stat.bg_ext_pal_b[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
			}
		}
	}

	//Update BG palettes - Engine B
	if(lcd_stat.bg_pal_update_b)
	{
		lcd_stat.bg_pal_update_b = false;

		//Cycle through all updates to BG palettes
		for(int x = 0; x < 256; x++)
		{
			//If this palette has been updated, convert to ARGB
			if(lcd_stat.bg_pal_update_list_b[x])
			{
				lcd_stat.bg_pal_update_list_b[x] = false;

				u16 color_bytes = mem->read_u16_fast(0x5000400 + (x << 1));
				lcd_stat.raw_bg_pal_b[x] = color_bytes;

				u8 red = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 green = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 blue = ((color_bytes & 0x1F) << 3);

				lcd_stat.bg_pal_b[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
			}
		}
	}

	//Update OBJ palettes - Engine A
	if(lcd_stat.obj_pal_update_a)
	{
		lcd_stat.obj_pal_update_a = false;

		//Cycle through all updates to BG palettes
		for(int x = 0; x < 256; x++)
		{
			//If this palette has been updated, convert to ARGB
			if(lcd_stat.obj_pal_update_list_a[x])
			{
				lcd_stat.obj_pal_update_list_a[x] = false;

				u16 color_bytes = mem->read_u16_fast(0x5000200 + (x << 1));
				lcd_stat.raw_obj_pal_a[x] = color_bytes;

				u8 red = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 green = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 blue = ((color_bytes & 0x1F) << 3);

				lcd_stat.obj_pal_a[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
			}
		}
	}

	//Update Extended OBJ palettes - Engine A
	if(lcd_stat.obj_ext_pal_update_a)
	{
		lcd_stat.obj_ext_pal_update_a = false;

		//Cycle through all updates to Extended OBJ palettes
		for(u16 x = 0; x < 4096; x++)
		{
			//If this palette has been updated, convert to ARGB
			if(lcd_stat.obj_ext_pal_update_list_a[x])
			{
				lcd_stat.obj_ext_pal_update_list_a[x] = false;

				u16 color_bytes = (lcd_stat.vram_bank_enable[5]) ? mem->read_u16_fast(0x6890000 + (x << 1)) : mem->read_u16_fast(0x6894000 + (x << 1));
				lcd_stat.raw_obj_ext_pal_a[x] = color_bytes;

				u8 red = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 green = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 blue = ((color_bytes & 0x1F) << 3);

				lcd_stat.obj_ext_pal_a[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
			}
		}
	}

	//Update OBJ palettes - Engine B
	if(lcd_stat.obj_pal_update_b)
	{
		lcd_stat.obj_pal_update_b = false;

		//Cycle through all updates to BG palettes
		for(int x = 0; x < 256; x++)
		{
			//If this palette has been updated, convert to ARGB
			if(lcd_stat.obj_pal_update_list_b[x])
			{
				lcd_stat.obj_pal_update_list_b[x] = false;

				u16 color_bytes = mem->read_u16_fast(0x5000600 + (x << 1));
				lcd_stat.raw_obj_pal_b[x] = color_bytes;

				u8 red = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 green = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 blue = ((color_bytes & 0x1F) << 3);

				lcd_stat.obj_pal_b[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
			}
		}
	}

	//Update Extended OBJ palettes - Engine B
	if(lcd_stat.obj_ext_pal_update_b)
	{
		lcd_stat.obj_ext_pal_update_b = false;

		//Cycle through all updates to Extended OBJ palettes
		for(u16 x = 0; x < 4096; x++)
		{
			//If this palette has been updated, convert to ARGB
			if(lcd_stat.obj_ext_pal_update_list_b[x])
			{
				lcd_stat.obj_ext_pal_update_list_b[x] = false;

				u16 color_bytes = mem->read_u16_fast(0x68A0000 + (x << 1));
				lcd_stat.raw_obj_ext_pal_b[x] = color_bytes;

				u8 red = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 green = ((color_bytes & 0x1F) << 3);
				color_bytes >>= 5;

				u8 blue = ((color_bytes & 0x1F) << 3);

				lcd_stat.obj_ext_pal_b[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
			}
		}
	}
}

/****** Render the line for a BG ******/
void NTR_LCD::render_bg_scanline(u32 bg_control)
{
	u8 bg_mode = (bg_control & 0x1000) ? lcd_stat.bg_mode_b : lcd_stat.bg_mode_a;
	u8 bg_render_list[4];
	u8 bg_id = 0;

	//Render Engine A
	if((bg_control & 0x1000) == 0)
	{
		//Reset render buffer
		full_scanline_render_a = false;
		render_buffer_a.assign(0x100, 0);

		//Reset SFX buffer
		sfx_buffer.assign(0x100, 0x80);

		//Clear scanline with backdrop
		for(u16 x = 0; x < 256; x++) { scanline_buffer_a[x] = lcd_stat.bg_pal_a[0]; }

		//Render OBJs if possible
		if(lcd_stat.display_control_a & 0x1000) { render_obj_scanline(bg_control); }

		//Determine BG priority
		for(int x = 0, list_length = 0; x < 4; x++)
		{
			if(lcd_stat.bg_priority_a[0] == x) { bg_render_list[list_length++] = 0; }
			if(lcd_stat.bg_priority_a[1] == x) { bg_render_list[list_length++] = 1; }
			if(lcd_stat.bg_priority_a[2] == x) { bg_render_list[list_length++] = 2; }
			if(lcd_stat.bg_priority_a[3] == x) { bg_render_list[list_length++] = 3; }
		}

		//Render BGs based on priority (3 is the 'lowest', 0 is the 'highest')
		for(int x = 0; x < 4; x++)
		{
			bg_id = bg_render_list[x];
			bg_control = NDS_BG0CNT_A + (bg_id << 1);

			//Render 3D first
			if((bg_id == 0) && (lcd_stat.display_control_a & 0x8))
			{
				render_bg_3D();
				x++;

				bg_id = bg_render_list[x];
				bg_control = NDS_BG0CNT_A + (bg_id << 1);
			}

			switch(lcd_stat.bg_mode_a)
			{
				//BG Mode 0
				case 0x0:
					render_bg_mode_text(bg_control);
					break;

				//BG Mode 1
				case 0x1:
					
					//BG0-2 Text
					if(bg_id != 3) { render_bg_mode_text(bg_control); }
					
					//BG3 Affine
					else { render_bg_mode_affine(bg_control); }

					break;

				//BG Mode 2
				case 0x2:
					//BG0-1 Text
					if(bg_id < 2) { render_bg_mode_text(bg_control); }

					//BG2-3 Affine
					else { render_bg_mode_affine(bg_control); }

					break;

				//BG Mode 3
				case 0x3:
					//BG0-2 Text
					if(bg_id != 3) { render_bg_mode_text(bg_control); }

					//BG3 Extended
					else
					{
						u8 ext_mode = (lcd_stat.bg_control_a[bg_id] & 0x80) | (lcd_stat.bg_control_a[bg_id] & 0x4);
						switch(ext_mode)
						{
							//16-bit Affine
							case 0x0:
							case 0x4:
								render_bg_mode_affine_ext(bg_control);
								break;

							//256 color Affine
							case 0x80:
								render_bg_mode_bitmap(bg_control);
								break;

							//Direct color Affine
							case 0x84:
								render_bg_mode_direct(bg_control);
								break;
						}
					}

					break;

				//BG Mode 4
				case 0x4:
					//BG0-1 Text
					if(bg_id < 2) { render_bg_mode_text(bg_control); }

					//BG2 Affine
					else if(bg_id == 2) { render_bg_mode_affine(bg_control); }

					//BG3 Extended
					else
					{
						u8 ext_mode = (lcd_stat.bg_control_a[bg_id] & 0x80) | (lcd_stat.bg_control_a[bg_id] & 0x4);
						switch(ext_mode)
						{
							//16-bit Affine
							case 0x0:
							case 0x4:
								render_bg_mode_affine_ext(bg_control);
								break;

							//256 color Affine
							case 0x80:
								render_bg_mode_bitmap(bg_control);
								break;

							//Direct color Affine
							case 0x84:
								render_bg_mode_direct(bg_control);
								break;
						}
					}

					break;

				//BG Mode 5
				case 0x5:
					//BG0-1 Text
					if(bg_id < 2) { render_bg_mode_text(bg_control); }

					//BG2-3 Extended
					else
					{
						u16 ext_mode = (lcd_stat.bg_control_a[bg_id] & 0x80) | (lcd_stat.bg_control_a[bg_id] & 0x4);
						switch(ext_mode)
						{
							//16-bit Affine
							case 0x0:
							case 0x4:
								render_bg_mode_affine_ext(bg_control);
								break;

							//256 color Affine
							case 0x80:
								render_bg_mode_bitmap(bg_control);
								break;

							//Direct color Affine
							case 0x84:
								render_bg_mode_direct(bg_control);
								break;
						}
					}

					break;

				default:
					std::cout<<"LCD::Engine A - invalid or unsupported BG Mode : " << std::dec << (u16)lcd_stat.bg_mode_a << "\n";
			}
		}

		//Apply SFX
		apply_sfx(NDS_DISPCNT_A);
	}

	//Render Engine B
	else
	{
		//Reset render buffer
		full_scanline_render_b = false;
		render_buffer_b.assign(0x100, 0);

		//Reset SFX buffer
		sfx_buffer.assign(0x100, 0x80);

		//Clear scanline with backdrop
		for(u16 x = 0; x < 256; x++) { scanline_buffer_b[x] = lcd_stat.bg_pal_b[0]; }

		//Render OBJs if possible
		if(lcd_stat.display_control_b & 0x1000) { render_obj_scanline(bg_control); }

		//Determine BG priority
		for(int x = 0, list_length = 0; x < 4; x++)
		{
			if(lcd_stat.bg_priority_b[0] == x) { bg_render_list[list_length++] = 0; }
			if(lcd_stat.bg_priority_b[1] == x) { bg_render_list[list_length++] = 1; }
			if(lcd_stat.bg_priority_b[2] == x) { bg_render_list[list_length++] = 2; }
			if(lcd_stat.bg_priority_b[3] == x) { bg_render_list[list_length++] = 3; }
		}

		//Render BGs based on priority (3 is the 'lowest', 0 is the 'highest')
		for(int x = 0; x < 4; x++)
		{
			bg_id = bg_render_list[x];
			bg_control = NDS_BG0CNT_B + (bg_id << 1);

			switch(lcd_stat.bg_mode_b)
			{
				//BG Mode 0
				case 0x0:
					render_bg_mode_text(bg_control);
					break;

				//BG Mode 1
				case 0x1:
					//BG0-2 Text
					if(bg_id != 3) { render_bg_mode_text(bg_control); }
					
					//BG3 Affine
					else { render_bg_mode_affine(bg_control); }

					break;

				//BG Mode 2
				case 0x2:
					//BG0-1 Text
					if(bg_id < 2) { render_bg_mode_text(bg_control); }

					//BG2-3 Affine
					else { render_bg_mode_affine(bg_control); }

					break;

				//BG Mode 3
				case 0x3:
					//BG0-2 Text
					if(bg_id != 3) { render_bg_mode_text(bg_control); }

					//BG3 Extended
					else
					{
						u8 ext_mode = (lcd_stat.bg_control_b[bg_id] & 0x80) | (lcd_stat.bg_control_b[bg_id] & 0x4);
						switch(ext_mode)
						{
							//16-bit Affine
							case 0x0:
							case 0x4:
								render_bg_mode_affine_ext(bg_control);
								break;

							//256 color Affine
							case 0x80:
								render_bg_mode_bitmap(bg_control);
								break;

							//Direct color Affine
							case 0x84:
								render_bg_mode_direct(bg_control);
								break;
						}
					}

					break;

				//BG Mode 4
				case 0x4:
					//BG0-1 Text
					if(bg_id < 2) { render_bg_mode_text(bg_control); }

					//BG2 Affine
					else if(bg_id == 2) { render_bg_mode_affine(bg_control); }

					//BG3 Extended
					else
					{
						u8 ext_mode = (lcd_stat.bg_control_b[bg_id] & 0x80) | (lcd_stat.bg_control_b[bg_id] & 0x4);
						switch(ext_mode)
						{
							//16-bit Affine
							case 0x0:
							case 0x4:
								render_bg_mode_affine_ext(bg_control);
								break;

							//256 color Affine
							case 0x80:
								render_bg_mode_bitmap(bg_control);
								break;

							//Direct color Affine
							case 0x84:
								render_bg_mode_direct(bg_control);
								break;
						}
					}

					break;

				//BG Mode 5
				case 0x5:
					//BG0-1 Text
					if(bg_id < 2) { render_bg_mode_text(bg_control); }

					//BG2-3 Extended
					else
					{
						u8 ext_mode = (lcd_stat.bg_control_b[bg_id] & 0x80) | (lcd_stat.bg_control_b[bg_id] & 0x4);
						switch(ext_mode)
						{
							//16-bit Affine
							case 0x0:
							case 0x4:
								render_bg_mode_affine_ext(bg_control);
								break;

							//256 color Affine
							case 0x80:
								render_bg_mode_bitmap(bg_control);
								break;

							//Direct color Affine
							case 0x84:
								render_bg_mode_direct(bg_control);
								break;
						}
					}

					break;

				default:
					std::cout<<"LCD::Engine B - invalid or unsupported BG Mode : " << std::dec << (u16)lcd_stat.bg_mode_b << "\n";
			}
		}

		//Apply SFX
		apply_sfx(NDS_DISPCNT_B);
	}
}

/****** Renders a scanline for OBJs ******/
void NTR_LCD::render_obj_scanline(u32 bg_control)
{
	//Detemine if Engine A or B
	u8 engine_id = (bg_control & 0x1000) ? 1 : 0;

	//Abort if no OBJs are rendered on this line
	if(!engine_id && !obj_render_length_a) { return; }
	else if(engine_id && !obj_render_length_b) { return; }

	u8 obj_id = 0;
	u8 pal_id = 0;
	u8 obj_render_length = engine_id ? obj_render_length_b : obj_render_length_a;
	u16 scanline_pixel_counter = 0;
	u16 render_width = 0;
	u8 raw_color = 0;
	u16 raw_pixel = 0;
	bool ext_pal = false;
	bool render_obj;
	bool direct_bitmap = false;
	s16 h_flip, v_flip = 0;
	u16 obj_x, obj_y = 0;
	u32 disp_cnt = engine_id ? lcd_stat.display_control_b : lcd_stat.display_control_a;

	//Cycle through all current OBJ and render them based on their priority
	for(int x = 0; x < obj_render_length; x++)
	{
		obj_id = engine_id ? obj_render_list_b[x] : obj_render_list_a[x];
		pal_id = obj[obj_id].palette_number;

		//Determine whether or not extended palettes are necessary
		if((!engine_id) && (lcd_stat.ext_pal_a & 0x2) && (obj[obj_id].bit_depth == 8)) { ext_pal = true; }
		else if((engine_id) && (lcd_stat.ext_pal_b & 0x2)&& (obj[obj_id].bit_depth == 8)) { ext_pal = true; }
		else { ext_pal = false; }
		
		//Check to see if OBJ is even onscreen
		if((obj[obj_id].left < 256) || (obj[obj_id].right < 256))
		{
			//Start rendering from X coordinate
			scanline_pixel_counter = obj[obj_id].left;
			render_width = 0;

			u8 meta_width = obj[obj_id].width / 8;
			u8 bit_depth = obj[obj_id].bit_depth * 8;
			u8 pixel_shift = (bit_depth == 32) ? 1 : 0;
			u16 draw_width = (obj[obj_id].affine_enable) ? (obj[obj_id].width * 2) : obj[obj_id].width;

			u8 bitmap_mask_a = (lcd_stat.display_control_a & 0x20) ? 0x1F : 0xF;
			u8 bitmap_mask_b = (lcd_stat.display_control_b & 0x20) ? 0x1F : 0xF; 
			
			direct_bitmap = (obj[obj_id].mode == 3) ? true : false;

			while(render_width < draw_width)
			{
				render_obj = true;

				if(scanline_pixel_counter < 256)
				{
					//Determine X and Y meta-tiles - Normal OBJ rendering
					if(!obj[obj_id].affine_enable)
					{
						obj_x = render_width;
						obj_y = obj[obj_id].y_wrap ? (lcd_stat.current_scanline + obj[obj_id].y_wrap_val) : (lcd_stat.current_scanline - obj[obj_id].y);

						//Horizontal flip the internal X coordinate
						if(obj[obj_id].h_flip)
						{
							h_flip = obj_x;
							h_flip -= (obj[obj_id].width - 1);

							if(h_flip < 0) { h_flip *= -1; }

							obj_x = h_flip;
						}

						//Vertical flip the internal Y coordinate
						if(obj[obj_id].v_flip)
						{
							v_flip = obj_y;
							v_flip -= (obj[obj_id].height - 1);

							if(v_flip < 0) { v_flip *= -1; }

							obj_y = v_flip;
						}
					}

					//Determine X and Y meta-tile - Affine OBJ rendering
					else
					{
						u8 index = (obj[obj_id].affine_group << 2);
						s16 current_x, current_y;

						//Determine current X position relative to the OBJ center X, account for screen wrapping
						if((obj[obj_id].x_wrap) && (scanline_pixel_counter < obj[obj_id].right))
						{
							current_x = scanline_pixel_counter - (obj[obj_id].cx - obj[obj_id].x_wrap);
						}

						else { current_x = scanline_pixel_counter - obj[obj_id].cx; }

						//Determine current Y position relative to the OBJ center Y, account for screen wrapping
						if((obj[obj_id].y_wrap) && (lcd_stat.current_scanline < obj[obj_id].bottom))
						{
							current_y = lcd_stat.current_scanline - (obj[obj_id].cy - obj[obj_id].y_wrap);
						}

						else { current_y = lcd_stat.current_scanline - obj[obj_id].cy; }

						s16 new_x = obj[obj_id].cw + (lcd_stat.obj_affine[index] * current_x) + (lcd_stat.obj_affine[index+1] * current_y);
						s16 new_y = obj[obj_id].ch + (lcd_stat.obj_affine[index+2] * current_x) + (lcd_stat.obj_affine[index+3] * current_y);

						//If out of bounds for the transformed sprite, abort rendering
						if((new_x < 0) || (new_y < 0) || (new_x >= obj[obj_id].width) || (new_y >= obj[obj_id].height)) { render_obj = false; }
		
						obj_x = new_x;
						obj_y = new_y;
					}

					u8 meta_x = obj_x / 8;
					u8 meta_y = obj_y / 8;

					//Determine address of this pixel
					u32 obj_addr = obj[obj_id].addr;
					u32 base = (obj_id < 128) ? 0x6400000 : 0x6600000;

					//Draw tiled OBJs
					if(!direct_bitmap)
					{
						//1D addressing
						if(disp_cnt & 0x10)
						{
							obj_addr += (((meta_y * meta_width) + meta_x) * bit_depth);
							obj_addr += (((obj_y % 8) * 8) + (obj_x % 8)) >> pixel_shift;
						}

						//2D addressing
						else
						{
							obj_addr = base + ((obj[obj_id].tile_number + meta_x + (meta_y << 5)) << 5);
							obj_addr += (((obj_y % 8) * 8) + (obj_x % 8)) >> pixel_shift; 
						}

						raw_color = mem->read_u8(obj_addr);

						//Process 4-bit depth if necessary
						if((bit_depth == 32) && (!ext_pal)) { raw_color = (obj_x & 0x1) ? (raw_color >> 4) : (raw_color & 0xF); }

						//Draw for Engine A
						if(!engine_id && raw_color && !render_buffer_a[scanline_pixel_counter] && render_obj)
						{
							scanline_buffer_a[scanline_pixel_counter] = (ext_pal) ? lcd_stat.obj_ext_pal_a[(pal_id * 256) + raw_color] : lcd_stat.obj_pal_a[(pal_id * 16) + raw_color];
							render_buffer_a[scanline_pixel_counter] = (obj[obj_id].bg_priority + 1);
							sfx_buffer[scanline_pixel_counter] = (render_buffer_a[scanline_pixel_counter] | 0x80);
						}

						//Draw for Engine B
						else if(engine_id && raw_color && !render_buffer_b[scanline_pixel_counter] && render_obj)
						{
							scanline_buffer_b[scanline_pixel_counter] = (ext_pal) ? lcd_stat.obj_ext_pal_b[(pal_id * 256) + raw_color] : lcd_stat.obj_pal_b[(pal_id * 16) + raw_color];
							render_buffer_b[scanline_pixel_counter] = (obj[obj_id].bg_priority + 1);
							sfx_buffer[scanline_pixel_counter] = (render_buffer_b[scanline_pixel_counter] | 0x80);
						}
					}

					//Draw direct bitmap OBJs
					else
					{
						//1D addressing
						if(disp_cnt & 0x10)
						{
							obj_addr += (((meta_y * meta_width) + meta_x) * 128);
							obj_addr += (((obj_y % 8) * 8) + (obj_x % 8)) >> 1;
						}

						//2D addressing
						else
						{
							u8 target_tile = (obj[obj_id].tile_number + meta_x + (meta_y << (disp_cnt & 0x20) ? 5 : 4));
							u8 mask = (engine_id) ? bitmap_mask_b : bitmap_mask_a;
							
							obj_addr = base + ((target_tile & mask) * 0x10) + ((target_tile & ~mask) * 0x80);
							obj_addr += (((obj_y % 8) * 8) + (obj_x % 8)) >> 1;
						}

						raw_pixel = mem->read_u16(obj_addr);

						//Draw for Engine A
						if(!engine_id && (raw_pixel & 0x8000) && !render_buffer_a[scanline_pixel_counter] && render_obj)
						{
							scanline_buffer_a[scanline_pixel_counter] = get_rgb15(raw_pixel);
							render_buffer_a[scanline_pixel_counter] = (obj[obj_id].bg_priority + 1);
							sfx_buffer[scanline_pixel_counter] = (render_buffer_a[scanline_pixel_counter] | 0x80);
						}

						//Draw for Engine B
						else if(engine_id && (raw_pixel & 0x8000) && !render_buffer_b[scanline_pixel_counter] && render_obj)
						{
							scanline_buffer_b[scanline_pixel_counter] = get_rgb15(raw_pixel);
							render_buffer_b[scanline_pixel_counter] = (obj[obj_id].bg_priority + 1);
							sfx_buffer[scanline_pixel_counter] = (render_buffer_b[scanline_pixel_counter] | 0x80);
						}
					}
						
				}

				scanline_pixel_counter++;
				scanline_pixel_counter &= 0x1FF;

				render_width++;
			}
		}
	}
}			

/****** Render BG Mode Text scanline ******/
void NTR_LCD::render_bg_mode_text(u32 bg_control)
{
	//Render Engine A
	if((bg_control & 0x1000) == 0)
	{
		//Grab BG ID
		u8 bg_id = (bg_control - 0x4000008) >> 1;
		u8 bg_priority = lcd_stat.bg_priority_a[bg_id] + 1;

		//Abort rendering if this BG is disabled
		if(!lcd_stat.bg_enable_a[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_a) { return; }

		bool full_render = true;

		//Grab tile offsets
		u8 tile_offset_x = lcd_stat.bg_offset_x_a[bg_id] & 0x7;

		u16 tile_id;
		u16 ext_pal_id;
		u8 pal_id;
		u8 flip;

		u16 scanline_pixel_counter = 0;
		u8 current_screen_line = (lcd_stat.current_scanline + lcd_stat.bg_offset_y_a[bg_id]);

		u8 current_screen_pixel = lcd_stat.bg_offset_x_a[bg_id];
		u16 current_tile_line = (lcd_stat.current_scanline + lcd_stat.bg_offset_y_a[bg_id]) % 8;

		//Grab BG bit-depth and offset for the current tile line
		u8 bit_depth = lcd_stat.bg_depth_a[bg_id] ? 64 : 32;
		u8 line_offset;

		//Get tile and map addresses
		u32 tile_addr = 0x6000000 + lcd_stat.bg_base_tile_addr_a[bg_id];
		u32 map_addr_base = 0x6000000 + lcd_stat.bg_base_map_addr_a[bg_id];
		u32 map_addr = 0;
 
		//Cycle through all tiles on this scanline
		for(u32 x = 0; x < 256;)
		{
			//Determine meta x-coordinate of rendered BG pixel
			u16 meta_x = ((scanline_pixel_counter + lcd_stat.bg_offset_x_a[bg_id]) & lcd_stat.text_width_a[bg_id]);

			//Determine meta Y-coordinate of rendered BG pixel
			u16 meta_y = ((lcd_stat.current_scanline + lcd_stat.bg_offset_y_a[bg_id]) & lcd_stat.text_height_a[bg_id]);

			//Determine the address offset for the screen
			switch(lcd_stat.bg_size_a[bg_id])
			{
				//Size 0 - 256x256
				case 0x0:
					map_addr = map_addr_base;
					break;

				//Size 1 - 512x256
				case 0x1: 
					map_addr = map_addr_base + screen_offset_lut[meta_x];
					break;

				//Size 2 - 256x512
				case 0x2:
					map_addr = map_addr_base + screen_offset_lut[meta_y];
					break;

				//Size 3 - 512x512
				case 0x3:
					map_addr = screen_offset_lut[meta_x];
					if(meta_y > 255) { map_addr |= 0x1000; }
					map_addr += map_addr_base;
					break;
			}
				
			//Determine which map entry to start looking up tiles
			u16 map_entry = ((current_screen_line >> 3) << 5);
			map_entry += (current_screen_pixel >> 3);

			//Pull map data from current map entry
			u16 map_data = mem->read_u16(map_addr + (map_entry << 1));

			//Get tile, palette number, and flipping parameters
			tile_id = (map_data & 0x3FF);
			pal_id = (map_data >> 12) & 0xF;
			ext_pal_id = (bg_id << 12) + (pal_id << 8);
			flip = (map_data >> 10) & 0x3;

			//Calculate VRAM address to start pulling up tile data
			line_offset = (flip & 0x2) ? ((bit_depth >> 3) * inv_lut[current_tile_line]) : ((bit_depth >> 3) * current_tile_line);
			u32 tile_data_addr = tile_addr + (tile_id * bit_depth) + line_offset;
			if(flip & 0x1) { tile_data_addr += ((bit_depth >> 3) - 1); }

			//Read 8 pixels from VRAM and put them in the scanline buffer
			for(u32 y = tile_offset_x; y < 8; y++, x++)
			{
				//Process 8-bit depth
				if(bit_depth == 64)
				{
					//Grab dot-data, account for horizontal flipping 
					u8 raw_color = (flip & 0x1) ? mem->read_u8(tile_data_addr--) : mem->read_u8(tile_data_addr++);

					//Only draw if no previous pixel was rendered
					if(!render_buffer_a[scanline_pixel_counter] || (bg_priority < render_buffer_a[scanline_pixel_counter]))
					{
						//Only draw colors if not transparent
						if(raw_color)
						{
							scanline_buffer_a[scanline_pixel_counter] = (lcd_stat.ext_pal_a) ? lcd_stat.bg_ext_pal_a[ext_pal_id + raw_color]  : lcd_stat.bg_pal_a[raw_color];
							render_buffer_a[scanline_pixel_counter] = bg_priority;
						}

						else { full_render = false; }
					}

					//SFX and line buffer
					sfx_buffer[scanline_pixel_counter] = render_buffer_a[scanline_pixel_counter];
					line_buffer[bg_id][scanline_pixel_counter] = (lcd_stat.ext_pal_a) ? lcd_stat.bg_ext_pal_a[ext_pal_id + raw_color]  : lcd_stat.bg_pal_a[raw_color];

					//Draw 256 pixels max
					scanline_pixel_counter++;
					current_screen_pixel++;
					if(scanline_pixel_counter & 0x100) { return; }
				}

				//Process 4-bit depth
				else
				{
					//Grab dot-data, account for horizontal flipping 
					u8 raw_color = (flip & 0x1) ? mem->read_u8(tile_data_addr--) : mem->read_u8(tile_data_addr++);

					u8 pal_1 = (flip & 0x1) ? ((pal_id * 16) + (raw_color >> 4)) : ((pal_id * 16) + (raw_color & 0xF));
					u8 pal_2 = (flip & 0x1) ? ((pal_id * 16) + (raw_color & 0xF)) : ((pal_id * 16) + (raw_color >> 4));

					//Only draw if no previous pixel was rendered
					if(!render_buffer_a[scanline_pixel_counter] || (bg_priority < render_buffer_a[scanline_pixel_counter]))
					{
						//Only draw colors if not transparent
						if(raw_color & 0xF)
						{
							scanline_buffer_a[scanline_pixel_counter] = lcd_stat.bg_pal_a[pal_1];
							render_buffer_a[scanline_pixel_counter] = bg_priority;
						}

						else { full_render = false; }
					}

					//SFX and line buffer
					sfx_buffer[scanline_pixel_counter] = (render_buffer_a[scanline_pixel_counter]);
					line_buffer[bg_id][scanline_pixel_counter] = lcd_stat.bg_pal_a[pal_1];

					//Draw 256 pixels max
					scanline_pixel_counter++;
					if(scanline_pixel_counter & 0x100) { return; }

					//Only draw if no previous pixel was rendered
					if(!render_buffer_a[scanline_pixel_counter] || (bg_priority < render_buffer_a[scanline_pixel_counter]))
					{
						//Only draw colors if not transparent
						if(raw_color >> 4)
						{
							scanline_buffer_a[scanline_pixel_counter] = lcd_stat.bg_pal_a[pal_2];
							render_buffer_a[scanline_pixel_counter] = bg_priority;
						}

						else { full_render = false; }
					}

					//SFX and line buffer
					sfx_buffer[scanline_pixel_counter] = (render_buffer_a[scanline_pixel_counter]);
					line_buffer[bg_id][scanline_pixel_counter] = lcd_stat.bg_pal_a[pal_2];

					//Draw 256 pixels max
					scanline_pixel_counter++;
					if(scanline_pixel_counter & 0x100) { return; }

					current_screen_pixel += 2;
					y++;
				}
			}

			tile_offset_x = 0;
		}

		full_scanline_render_a = full_render;
	}

	//Render Engine B
	else
	{
		//Grab BG ID
		u8 bg_id = (bg_control - 0x4001008) >> 1;
		u8 bg_priority = lcd_stat.bg_priority_b[bg_id] + 1;

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_b[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_b) { return; }

		bool full_render = true;

		//Grab tile offsets
		u8 tile_offset_x = lcd_stat.bg_offset_x_b[bg_id] & 0x7;

		u16 tile_id;
		u16 ext_pal_id;
		u8 pal_id;
		u8 flip;

		u16 scanline_pixel_counter = 0;
		u8 current_screen_line = (lcd_stat.current_scanline + lcd_stat.bg_offset_y_b[bg_id]);

		u8 current_screen_pixel = lcd_stat.bg_offset_x_b[bg_id];
		u16 current_tile_line = (lcd_stat.current_scanline + lcd_stat.bg_offset_y_b[bg_id]) % 8;

		//Grab BG bit-depth and offset for the current tile line
		u8 bit_depth = lcd_stat.bg_depth_b[bg_id] ? 64 : 32;
		u8 line_offset;

		//Get tile and map addresses
		u32 tile_addr = 0x6200000 + lcd_stat.bg_base_tile_addr_b[bg_id];
		u32 map_addr_base = 0x6200000 + lcd_stat.bg_base_map_addr_b[bg_id];
		u32 map_addr = 0;

		//Cycle through all tiles on this scanline
		for(u32 x = 0; x < 256;)
		{
			//Determine meta x-coordinate of rendered BG pixel
			u16 meta_x = ((scanline_pixel_counter + lcd_stat.bg_offset_x_b[bg_id]) & lcd_stat.text_width_b[bg_id]);

			//Determine meta Y-coordinate of rendered BG pixel
			u16 meta_y = ((lcd_stat.current_scanline + lcd_stat.bg_offset_y_b[bg_id]) & lcd_stat.text_height_b[bg_id]);

			//Determine the address offset for the screen
			switch(lcd_stat.bg_size_b[bg_id])
			{
				//Size 0 - 256x256
				case 0x0:
					map_addr = map_addr_base;
					break;

				//Size 1 - 512x256
				case 0x1: 
					map_addr = map_addr_base + screen_offset_lut[meta_x];
					break;

				//Size 2 - 256x512
				case 0x2:
					map_addr = map_addr_base + screen_offset_lut[meta_y];
					break;

				//Size 3 - 512x512
				case 0x3:
					map_addr = screen_offset_lut[meta_x];
					if(meta_y > 255) { map_addr |= 0x1000; }
					map_addr += map_addr_base;
					break;
			}

			//Determine which map entry to start looking up tiles
			u16 map_entry = ((current_screen_line >> 3) << 5);
			map_entry += (current_screen_pixel >> 3);

			//Pull map data from current map entry
			u16 map_data = mem->read_u16(map_addr + (map_entry << 1));

			//Get tile, palette number, and flipping parameters
			tile_id = (map_data & 0x3FF);
			pal_id = (map_data >> 12) & 0xF;
			ext_pal_id = (bg_id << 12) + (pal_id << 8);
			flip = (map_data >> 10) & 0x3;

			//Calculate VRAM address to start pulling up tile data
			line_offset = (flip & 0x2) ? ((bit_depth >> 3) * inv_lut[current_tile_line]) : ((bit_depth >> 3) * current_tile_line);
			u32 tile_data_addr = tile_addr + (tile_id * bit_depth) + line_offset;
			if(flip & 0x1) { tile_data_addr += ((bit_depth >> 3) - 1); }

			//Read 8 pixels from VRAM and put them in the scanline buffer
			for(u32 y = tile_offset_x; y < 8; y++, x++)
			{
				//Process 8-bit depth
				if(bit_depth == 64)
				{
					//Grab dot-data, account for horizontal flipping 
					u8 raw_color = (flip & 0x1) ? mem->read_u8(tile_data_addr--) : mem->read_u8(tile_data_addr++);

					//Only draw if no previous pixel was rendered
					if(!render_buffer_b[scanline_pixel_counter] || (bg_priority < render_buffer_b[scanline_pixel_counter]))
					{
						//Only draw colors if not transparent
						if(raw_color)
						{
							scanline_buffer_b[scanline_pixel_counter] = (lcd_stat.ext_pal_b) ? lcd_stat.bg_ext_pal_b[ext_pal_id + raw_color]  : lcd_stat.bg_pal_b[raw_color];
							render_buffer_b[scanline_pixel_counter] = bg_priority;
						}

						else { full_render = false; }
					}

					//SFX and line buffer
					sfx_buffer[scanline_pixel_counter] = (render_buffer_b[scanline_pixel_counter]);
					line_buffer[bg_id][scanline_pixel_counter] = (lcd_stat.ext_pal_b) ? lcd_stat.bg_ext_pal_b[ext_pal_id + raw_color]  : lcd_stat.bg_pal_b[raw_color];

					//Draw 256 pixels max
					scanline_pixel_counter++;
					current_screen_pixel++;
					if(scanline_pixel_counter & 0x100) { return; }
				}

				//Process 4-bit depth
				else
				{
					//Grab dot-data, account for horizontal flipping 
					u8 raw_color = (flip & 0x1) ? mem->read_u8(tile_data_addr--) : mem->read_u8(tile_data_addr++);

					u8 pal_1 = (flip & 0x1) ? ((pal_id * 16) + (raw_color >> 4)) : ((pal_id * 16) + (raw_color & 0xF));
					u8 pal_2 = (flip & 0x1) ? ((pal_id * 16) + (raw_color & 0xF)) : ((pal_id * 16) + (raw_color >> 4));

					//Only draw if no previous pixel was rendered
					if(!render_buffer_b[scanline_pixel_counter] || (bg_priority < render_buffer_b[scanline_pixel_counter]))
					{
						//Only draw colors if not transparent
						if(raw_color & 0xF)
						{
							scanline_buffer_b[scanline_pixel_counter] = lcd_stat.bg_pal_b[pal_1];
							render_buffer_b[scanline_pixel_counter] = bg_priority;
						}

						else { full_render = false; }
					}

					//SFX and line buffer
					sfx_buffer[scanline_pixel_counter] = (render_buffer_b[scanline_pixel_counter]);
					line_buffer[bg_id][scanline_pixel_counter] = lcd_stat.bg_pal_b[pal_1];

					//Draw 256 pixels max
					scanline_pixel_counter++;
					if(scanline_pixel_counter & 0x100) { return; }

					//Only draw if no previous pixel was rendered
					if(!render_buffer_b[scanline_pixel_counter] || (bg_priority < render_buffer_b[scanline_pixel_counter]))
					{
						//Only draw colors if not transparent
						if(raw_color >> 4)
						{
							scanline_buffer_b[scanline_pixel_counter] = lcd_stat.bg_pal_b[pal_2];
							render_buffer_b[scanline_pixel_counter] = bg_priority;
						}

						else { full_render = false; }
					}

					//SFX and line buffer
					sfx_buffer[scanline_pixel_counter] = (render_buffer_b[scanline_pixel_counter]);
					line_buffer[bg_id][scanline_pixel_counter] = lcd_stat.bg_pal_b[pal_2];

					//Draw 256 pixels max
					scanline_pixel_counter++;
					if(scanline_pixel_counter & 0x100) { return; }

					current_screen_pixel += 2;
					y++;
				}
			}

			tile_offset_x = 0;
		}

		full_scanline_render_b = full_render;	
	}
}

/****** Render BG Mode Affine scanline ******/
void NTR_LCD::render_bg_mode_affine(u32 bg_control)
{
	//Render Engine A
	if((bg_control & 0x1000) == 0)
	{
		//Grab BG ID and affine ID
		u8 bg_id = (bg_control - 0x4000008) >> 1;
		u8 affine_id = (bg_id & 0x1);
		u8 bg_priority = lcd_stat.bg_priority_a[bg_id] + 1;

		//Reload X-Y references at start of frame
		if(lcd_stat.current_scanline == 0) { reload_affine_references(bg_control); }

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_a[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_a) { return; }

		bool full_render = true;

		//Get BG size in tiles, pixels
		//0 - 128x128, 1 - 256x256, 2 - 512x512, 3 - 1024x1024
		u16 bg_tile_size = (16 << (lcd_stat.bg_control_a[bg_id] >> 14));
		u16 bg_pixel_size = bg_tile_size << 3;

		u16 scanline_pixel_counter = 0;
		u16 src_x, src_y = 0;
		double new_x, new_y = 0.0;

		//Set current texture position at X and Y references
		lcd_stat.bg_affine_a[affine_id].x_pos = lcd_stat.bg_affine_a[affine_id].x_ref - lcd_stat.bg_affine_a[affine_id].dx;
		lcd_stat.bg_affine_a[affine_id].y_pos = lcd_stat.bg_affine_a[affine_id].y_ref - lcd_stat.bg_affine_a[affine_id].dy;

		//Get tile and map addresses
		u32 tile_base = 0x6000000 + lcd_stat.bg_base_tile_addr_a[bg_id];
		u32 map_base = 0x6000000 + lcd_stat.bg_base_map_addr_a[bg_id];
 
		//Cycle through all tiles on this scanline
		for(u32 x = 0; x < 256; x++, scanline_pixel_counter++)
		{
			bool render_pixel = true;

			//Update texture position with DX and DY
			lcd_stat.bg_affine_a[affine_id].x_pos += lcd_stat.bg_affine_a[affine_id].dx;
			lcd_stat.bg_affine_a[affine_id].y_pos += lcd_stat.bg_affine_a[affine_id].dy;

			new_x = lcd_stat.bg_affine_a[affine_id].x_pos;
			new_y = lcd_stat.bg_affine_a[affine_id].y_pos;

			//Clip BG if coordinates overflow and overflow flag is not set
			if(!lcd_stat.bg_affine_a[affine_id].overflow)
			{
				if((new_x >= bg_pixel_size) || (new_x < 0)) { render_pixel = false; }
				if((new_y >= bg_pixel_size) || (new_y < 0)) { render_pixel = false; }
			}

			//Wrap BG if coordinates overflow and overflow flag is set
			else 
			{
				while(new_x >= bg_pixel_size) { new_x -= bg_pixel_size; }
				while(new_y >= bg_pixel_size) { new_y -= bg_pixel_size; }
				while(new_x < 0) { new_x += bg_pixel_size; }
				while(new_y < 0) { new_y += bg_pixel_size; } 
			}

			//Only draw if no previous pixel was rendered
			if(!render_buffer_a[scanline_pixel_counter] || (bg_priority < render_buffer_a[scanline_pixel_counter]))
			{
				if(render_pixel)
				{
					//Determine source pixel X-Y coordinates
					src_x = new_x; 
					src_y = new_y;

					//Get current map entry for rendered pixel
					u16 tile_number = ((src_y / 8) * bg_tile_size) + (src_x / 8);

					//Look at the Tile Map #(tile_number), see what Tile # it points to
					u8 map_entry = mem->memory_map[map_base + tile_number];

					//Get address of Tile #(map_entry)
					u32 tile_addr = tile_base + (map_entry * 64);

					u8 current_tile_pixel = ((src_y % 8) * 8) + (src_x % 8);

					//Grab the byte corresponding to (current_tile_pixel), render it as ARGB - 8-bit version
					tile_addr += current_tile_pixel;
					u8 raw_color = mem->memory_map[tile_addr];

					//Only draw BG color if not transparent
					if(raw_color != 0)
					{
						scanline_buffer_a[scanline_pixel_counter] = lcd_stat.bg_pal_a[raw_color];
						render_buffer_a[scanline_pixel_counter] = bg_priority;
					}

					else { full_render = false; }
				}

				else { full_render = false; }
			}
		}

		//Update XREF and YREF for next line
		lcd_stat.bg_affine_a[affine_id].x_ref += lcd_stat.bg_affine_a[affine_id].dmx;
		lcd_stat.bg_affine_a[affine_id].y_ref += lcd_stat.bg_affine_a[affine_id].dmy;

		full_scanline_render_a = full_render;
	}

	//Render Engine B
	else
	{
		//Grab BG ID and affine ID
		u8 bg_id = (bg_control - 0x4001008) >> 1;
		u8 affine_id = (bg_id & 0x1);
		u8 bg_priority = lcd_stat.bg_priority_b[bg_id] + 1;

		//Reload X-Y references at start of frame
		if(lcd_stat.current_scanline == 0) { reload_affine_references(bg_control); }

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_b[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_b) { return; }

		bool full_render = true;

		//Get BG size in tiles, pixels
		//0 - 128x128, 1 - 256x256, 2 - 512x512, 3 - 1024x1024
		u16 bg_tile_size = (16 << (lcd_stat.bg_control_b[bg_id] >> 14));
		u16 bg_pixel_size = bg_tile_size << 3;

		u16 scanline_pixel_counter = 0;
		u16 src_x, src_y = 0;
		double new_x, new_y = 0.0;

		//Set current texture position at X and Y references
		lcd_stat.bg_affine_b[affine_id].x_pos = lcd_stat.bg_affine_b[affine_id].x_ref - lcd_stat.bg_affine_b[affine_id].dx;
		lcd_stat.bg_affine_b[affine_id].y_pos = lcd_stat.bg_affine_b[affine_id].y_ref - lcd_stat.bg_affine_b[affine_id].dy;

		//Get tile and map addresses
		u32 tile_base = 0x6200000 + lcd_stat.bg_base_tile_addr_b[bg_id];
		u32 map_base = 0x6200000 + lcd_stat.bg_base_map_addr_b[bg_id];
 
		//Cycle through all tiles on this scanline
		for(u32 x = 0; x < 256; x++, scanline_pixel_counter++)
		{
			bool render_pixel = true;

			//Update texture position with DX and DY
			lcd_stat.bg_affine_b[affine_id].x_pos += lcd_stat.bg_affine_b[affine_id].dx;
			lcd_stat.bg_affine_b[affine_id].y_pos += lcd_stat.bg_affine_b[affine_id].dy;

			new_x = lcd_stat.bg_affine_b[affine_id].x_pos;
			new_y = lcd_stat.bg_affine_b[affine_id].y_pos;

			//Clip BG if coordinates overflow and overflow flag is not set
			if(!lcd_stat.bg_affine_b[affine_id].overflow)
			{
				if((new_x >= bg_pixel_size) || (new_x < 0)) { render_pixel = false; }
				if((new_y >= bg_pixel_size) || (new_y < 0)) { render_pixel = false; }
			}

			//Wrap BG if coordinates overflow and overflow flag is set
			else 
			{
				while(new_x >= bg_pixel_size) { new_x -= bg_pixel_size; }
				while(new_y >= bg_pixel_size) { new_y -= bg_pixel_size; }
				while(new_x < 0) { new_x += bg_pixel_size; }
				while(new_y < 0) { new_y += bg_pixel_size; } 
			}

			//Only draw if no previous pixel was rendered
			if(!render_buffer_b[scanline_pixel_counter] || (bg_priority < render_buffer_b[scanline_pixel_counter]))
			{
				if(render_pixel)
				{
					//Determine source pixel X-Y coordinates
					src_x = new_x; 
					src_y = new_y;

					//Get current map entry for rendered pixel
					u16 tile_number = ((src_y / 8) * bg_tile_size) + (src_x / 8);

					//Look at the Tile Map #(tile_number), see what Tile # it points to
					u8 map_entry = mem->memory_map[map_base + tile_number];

					//Get address of Tile #(map_entry)
					u32 tile_addr = tile_base + (map_entry * 64);

					u8 current_tile_pixel = ((src_y % 8) * 8) + (src_x % 8);

					//Grab the byte corresponding to (current_tile_pixel), render it as ARGB - 8-bit version
					tile_addr += current_tile_pixel;
					u8 raw_color = mem->memory_map[tile_addr];

					//Only draw BG color if not transparent
					if(raw_color != 0)
					{
						scanline_buffer_b[scanline_pixel_counter] = lcd_stat.bg_pal_b[raw_color];
						render_buffer_b[scanline_pixel_counter] = bg_priority;
					}

					else { full_render = false; }
				}

				else { full_render = false; }
			}
		}

		//Update XREF and YREF for next line
		lcd_stat.bg_affine_b[affine_id].x_ref += lcd_stat.bg_affine_b[affine_id].dmx;
		lcd_stat.bg_affine_b[affine_id].y_ref += lcd_stat.bg_affine_b[affine_id].dmy;

		full_scanline_render_b = full_render;
	}
}

/****** Render BG Mode Affine-Extended scanline ******/
void NTR_LCD::render_bg_mode_affine_ext(u32 bg_control)
{
	//Render Engine A
	if((bg_control & 0x1000) == 0)
	{
		//Grab BG ID and affine ID
		u8 bg_id = (bg_control - 0x4000008) >> 1;
		u8 affine_id = (bg_id & 0x1);
		u8 bg_priority = lcd_stat.bg_priority_a[bg_id] + 1;

		//Reload X-Y references at start of frame
		if(lcd_stat.current_scanline == 0) { reload_affine_references(bg_control); }

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_a[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_a) { return; }

		bool full_render = true;

		//Get BG size in tiles, pixels
		//0 - 128x128, 1 - 256x256, 2 - 512x512, 3 - 1024x1024
		u16 bg_tile_size = (16 << (lcd_stat.bg_control_a[bg_id] >> 14));
		u16 bg_pixel_size = bg_tile_size << 3;

		u8 scanline_pixel_counter = 0;
		u16 src_x, src_y = 0;
		double new_x, new_y = 0.0;
		u8 flip = 0;

		//Set current texture position at X and Y references
		lcd_stat.bg_affine_a[affine_id].x_pos = lcd_stat.bg_affine_a[affine_id].x_ref - lcd_stat.bg_affine_a[affine_id].dx;
		lcd_stat.bg_affine_a[affine_id].y_pos = lcd_stat.bg_affine_a[affine_id].y_ref - lcd_stat.bg_affine_a[affine_id].dy;

		//Get tile and map addresses
		u32 tile_base = 0x6000000 + lcd_stat.bg_base_tile_addr_a[bg_id];
		u32 map_base = 0x6000000 + lcd_stat.bg_base_map_addr_a[bg_id];

		//Cycle through all tiles on this scanline
		for(u32 x = 0; x < 256; x++, scanline_pixel_counter++)
		{
			bool render_pixel = true;

			//Update texture position with DX and DY
			lcd_stat.bg_affine_a[affine_id].x_pos += lcd_stat.bg_affine_a[affine_id].dx;
			lcd_stat.bg_affine_a[affine_id].y_pos += lcd_stat.bg_affine_a[affine_id].dy;

			new_x = lcd_stat.bg_affine_a[affine_id].x_pos;
			new_y = lcd_stat.bg_affine_a[affine_id].y_pos;

			//Clip BG if coordinates overflow and overflow flag is not set
			if(!lcd_stat.bg_affine_a[affine_id].overflow)
			{
				if((new_x >= bg_pixel_size) || (new_x < 0)) { render_pixel = false; }
				if((new_y >= bg_pixel_size) || (new_y < 0)) { render_pixel = false; }
			}

			//Wrap BG if coordinates overflow and overflow flag is set
			else 
			{
				while(new_x >= bg_pixel_size) { new_x -= bg_pixel_size; }
				while(new_y >= bg_pixel_size) { new_y -= bg_pixel_size; }
				while(new_x < 0) { new_x += bg_pixel_size; }
				while(new_y < 0) { new_y += bg_pixel_size; } 
			}

			//Only draw if no previous pixel was rendered
			if(!render_buffer_a[scanline_pixel_counter] || (bg_priority < render_buffer_a[scanline_pixel_counter]))
			{
				if(render_pixel)
				{
					//Determine source pixel X-Y coordinates
					src_x = new_x; 
					src_y = new_y;

					//Get current map entry for rendered pixel
					u16 tile_number = ((src_y / 8) * bg_tile_size) + (src_x / 8);

					//Look at the Tile Map #(tile_number), see what Tile # it points to
					u16 map_entry = mem->read_u16(map_base + (tile_number << 1));

					//Grab flipping attributes
					flip = (map_entry >> 10) & 0x3;
					src_x = (flip & 0x1) ? inv_lut[src_x % 8] : (src_x % 8);
					src_y = (flip & 0x2) ? inv_lut[src_y % 8] : (src_y % 8);

					//Get address of Tile #(map_entry)
					u32 tile_addr = tile_base + ((map_entry & 0x3FF) * 64);

					u8 current_tile_pixel = (src_y * 8) + src_x;

					//Grab the byte corresponding to (current_tile_pixel), render it as ARGB - 8-bit version
					tile_addr += current_tile_pixel;
					u8 raw_color = mem->memory_map[tile_addr];

					//Only draw BG color if not transparent
					if(raw_color != 0)
					{
						scanline_buffer_a[scanline_pixel_counter] = lcd_stat.bg_pal_a[raw_color];
						render_buffer_a[scanline_pixel_counter] = bg_priority;
					}

					else { full_render = false; }
				}

				else { full_render = false; }	
			}
		}

		//Update XREF and YREF for next line
		lcd_stat.bg_affine_a[affine_id].x_ref += lcd_stat.bg_affine_a[affine_id].dmx;
		lcd_stat.bg_affine_a[affine_id].y_ref += lcd_stat.bg_affine_a[affine_id].dmy;

		full_scanline_render_a = full_render;
	}

	//Render Engine B
	else
	{
		//Grab BG ID and affine ID
		u8 bg_id = (bg_control - 0x4001008) >> 1;
		u8 affine_id = (bg_id & 0x1);
		u8 bg_priority = lcd_stat.bg_priority_b[bg_id] + 1;

		//Reload X-Y references at start of frame
		if(lcd_stat.current_scanline == 0) { reload_affine_references(bg_control); }

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_b[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_b) { return; }

		bool full_render = true;

		//Get BG size in tiles, pixels
		//0 - 128x128, 1 - 256x256, 2 - 512x512, 3 - 1024x1024
		u16 bg_tile_size = (16 << (lcd_stat.bg_control_b[bg_id] >> 14));
		u16 bg_pixel_size = bg_tile_size << 3;

		u8 scanline_pixel_counter = 0;
		u16 src_x, src_y = 0;
		double new_x, new_y = 0.0;
		u8 flip = 0;

		//Set current texture position at X and Y references
		lcd_stat.bg_affine_b[affine_id].x_pos = lcd_stat.bg_affine_b[affine_id].x_ref - lcd_stat.bg_affine_b[affine_id].dx;
		lcd_stat.bg_affine_b[affine_id].y_pos = lcd_stat.bg_affine_b[affine_id].y_ref - lcd_stat.bg_affine_b[affine_id].dy;

		//Get tile and map addresses
		u32 tile_base = 0x6200000 + lcd_stat.bg_base_tile_addr_b[bg_id];
		u32 map_base = 0x6200000 + lcd_stat.bg_base_map_addr_b[bg_id];

		//Cycle through all tiles on this scanline
		for(u32 x = 0; x < 256; x++, scanline_pixel_counter++)
		{
			bool render_pixel = true;

			//Update texture position with DX and DY
			lcd_stat.bg_affine_b[affine_id].x_pos += lcd_stat.bg_affine_b[affine_id].dx;
			lcd_stat.bg_affine_b[affine_id].y_pos += lcd_stat.bg_affine_b[affine_id].dy;

			new_x = lcd_stat.bg_affine_b[affine_id].x_pos;
			new_y = lcd_stat.bg_affine_b[affine_id].y_pos;

			//Clip BG if coordinates overflow and overflow flag is not set
			if(!lcd_stat.bg_affine_b[affine_id].overflow)
			{
				if((new_x >= bg_pixel_size) || (new_x < 0)) { render_pixel = false; }
				if((new_y >= bg_pixel_size) || (new_y < 0)) { render_pixel = false; }
			}

			//Wrap BG if coordinates overflow and overflow flag is set
			else 
			{
				while(new_x >= bg_pixel_size) { new_x -= bg_pixel_size; }
				while(new_y >= bg_pixel_size) { new_y -= bg_pixel_size; }
				while(new_x < 0) { new_x += bg_pixel_size; }
				while(new_y < 0) { new_y += bg_pixel_size; } 
			}

			//Only draw if no previous pixel was rendered
			if(!render_buffer_b[scanline_pixel_counter] || (bg_priority < render_buffer_b[scanline_pixel_counter]))
			{
				if(render_pixel)
				{
					//Determine source pixel X-Y coordinates
					src_x = new_x; 
					src_y = new_y;

					//Get current map entry for rendered pixel
					u16 tile_number = ((src_y / 8) * bg_tile_size) + (src_x / 8);

					//Look at the Tile Map #(tile_number), see what Tile # it points to
					u16 map_entry = mem->read_u16(map_base + (tile_number << 1));

					//Grab flipping attributes
					flip = (map_entry >> 10) & 0x3;
					src_x = (flip & 0x1) ? inv_lut[src_x % 8] : (src_x % 8);
					src_y = (flip & 0x2) ? inv_lut[src_y % 8] : (src_y % 8);

					//Get address of Tile #(map_entry)
					u32 tile_addr = tile_base + ((map_entry & 0x3FF) * 64);

					u8 current_tile_pixel = (src_y * 8) + src_x;

					//Grab the byte corresponding to (current_tile_pixel), render it as ARGB - 8-bit version
					tile_addr += current_tile_pixel;
					u8 raw_color = mem->memory_map[tile_addr];

					//Only draw BG color if not transparent
					if(raw_color != 0)
					{
						scanline_buffer_b[scanline_pixel_counter] = lcd_stat.bg_pal_b[raw_color];
						render_buffer_b[scanline_pixel_counter] = bg_priority;
					}

					else { full_render = false; }
				}

				else { full_render = false; }	
			}
		}

		//Update XREF and YREF for next line
		lcd_stat.bg_affine_b[affine_id].x_ref += lcd_stat.bg_affine_b[affine_id].dmx;
		lcd_stat.bg_affine_b[affine_id].y_ref += lcd_stat.bg_affine_b[affine_id].dmy;

		full_scanline_render_b = full_render;
	}
}

/****** Render BG Mode 256-color scanline ******/
void NTR_LCD::render_bg_mode_bitmap(u32 bg_control)
{
	//Render Engine A
	if((bg_control & 0x1000) == 0)
	{
		//Grab BG ID
		u8 bg_id = (bg_control - 0x4000008) >> 1;
		u8 affine_id = (bg_id & 0x1);
		u8 bg_priority = lcd_stat.bg_priority_a[bg_id] + 1;

		//Reload X-Y references at start of frame
		if(lcd_stat.current_scanline == 0) { reload_affine_references(bg_control); }

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_a[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_a) { return; }

		bool full_render = true;

		u8 raw_color = 0;
		u8 scanline_pixel_counter = 0;

		u16 src_x, src_y = 0;
		double new_x, new_y = 0.0;
		u16 bg_pixel_width, bg_pixel_height = 0;

		u32 bitmap_addr = lcd_stat.bg_bitmap_base_addr_a[bg_id & 0x1];

		//Determine bitmap dimensions
		switch(lcd_stat.bg_size_a[bg_id])
		{
			case 0x0:
				bg_pixel_width = 128;
				bg_pixel_height = 128;
				break;

			case 0x1:
				bg_pixel_width = 256;
				bg_pixel_height = 256;
				break;

			case 0x2:
				bg_pixel_width = 512;
				bg_pixel_height = 256;
				break;

			case 0x3:
				bg_pixel_width = 512;
				bg_pixel_height = 512;
				break;
		}

		//Set current texture position at X and Y references
		lcd_stat.bg_affine_a[affine_id].x_pos = lcd_stat.bg_affine_a[affine_id].x_ref;
		lcd_stat.bg_affine_a[affine_id].y_pos = lcd_stat.bg_affine_a[affine_id].y_ref;
		
		for(int x = 0; x < 256; x++)
		{
			bool render_pixel = true;

			new_x = lcd_stat.bg_affine_a[affine_id].x_pos;
			new_y = lcd_stat.bg_affine_a[affine_id].y_pos;

			//Clip BG if coordinates overflow and overflow flag is not set
			if(!lcd_stat.bg_affine_a[affine_id].overflow)
			{
				if((new_x >= bg_pixel_width) || (new_x < 0)) { render_pixel = false; }
				if((new_y >= bg_pixel_height) || (new_y < 0)) { render_pixel = false; }
			}

			//Wrap BG if coordinates overflow and overflow flag is set
			else 
			{
				while(new_x >= bg_pixel_width) { new_x -= bg_pixel_width; }
				while(new_y >= bg_pixel_height) { new_y -= bg_pixel_height; }
				while(new_x < 0) { new_x += bg_pixel_width; }
				while(new_y < 0) { new_y += bg_pixel_height; } 
			}

			//Only draw if no previous pixel was rendered
			if(!render_buffer_a[scanline_pixel_counter] || (bg_priority < render_buffer_a[scanline_pixel_counter]))
			{
				if(render_pixel)
				{
					//Determine source pixel X-Y coordinates
					src_x = new_x;
					src_y = new_y;

					raw_color = mem->memory_map[bitmap_addr + (src_y * bg_pixel_width) + src_x];
			
					if(raw_color)
					{
						scanline_buffer_a[scanline_pixel_counter] = lcd_stat.bg_pal_a[raw_color];
						render_buffer_a[scanline_pixel_counter] = bg_priority;
					}

					else { full_render = false; }
				}

				else { full_render = false; }
			}

			scanline_pixel_counter++;

			//Update texture position with DX and DY
			lcd_stat.bg_affine_a[affine_id].x_pos += lcd_stat.bg_affine_a[affine_id].dx;
			lcd_stat.bg_affine_a[affine_id].y_pos += lcd_stat.bg_affine_a[affine_id].dy;
		}

		//Update XREF and YREF for next line
		lcd_stat.bg_affine_a[affine_id].x_ref += lcd_stat.bg_affine_a[affine_id].dmx;
		lcd_stat.bg_affine_a[affine_id].y_ref += lcd_stat.bg_affine_a[affine_id].dmy;

		full_scanline_render_a = full_render;
	}

	//Render Engine B
	else
	{
		//Grab BG ID
		u8 bg_id = (bg_control - 0x4001008) >> 1;
		u8 affine_id = (bg_id & 0x1);
		u8 bg_priority = lcd_stat.bg_priority_b[bg_id] + 1;

		//Reload X-Y references at start of frame
		if(lcd_stat.current_scanline == 0) { reload_affine_references(bg_control); }

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_b[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_b) { return; }

		bool full_render = true;

		u8 raw_color = 0;
		u8 scanline_pixel_counter = 0;

		u16 src_x, src_y = 0;
		double new_x, new_y = 0.0;
		u16 bg_pixel_width, bg_pixel_height = 0;

		u32 bitmap_addr = lcd_stat.bg_bitmap_base_addr_b[bg_id & 0x1];

		//Determine bitmap dimensions
		switch(lcd_stat.bg_size_b[bg_id])
		{
			case 0x0:
				bg_pixel_width = 128;
				bg_pixel_height = 128;
				break;

			case 0x1:
				bg_pixel_width = 256;
				bg_pixel_height = 256;
				break;

			case 0x2:
				bg_pixel_width = 512;
				bg_pixel_height = 256;
				break;

			case 0x3:
				bg_pixel_width = 512;
				bg_pixel_height = 512;
				break;
		}

		//Set current texture position at X and Y references
		lcd_stat.bg_affine_b[affine_id].x_pos = lcd_stat.bg_affine_b[affine_id].x_ref;
		lcd_stat.bg_affine_b[affine_id].y_pos = lcd_stat.bg_affine_b[affine_id].y_ref;
		
		for(int x = 0; x < 256; x++)
		{
			bool render_pixel = true;

			new_x = lcd_stat.bg_affine_b[affine_id].x_pos;
			new_y = lcd_stat.bg_affine_b[affine_id].y_pos;

			//Clip BG if coordinates overflow and overflow flag is not set
			if(!lcd_stat.bg_affine_b[affine_id].overflow)
			{
				if((new_x >= bg_pixel_width) || (new_x < 0)) { render_pixel = false; }
				if((new_y >= bg_pixel_height) || (new_y < 0)) { render_pixel = false; }
			}

			//Wrap BG if coordinates overflow and overflow flag is set
			else 
			{
				while(new_x >= bg_pixel_width) { new_x -= bg_pixel_width; }
				while(new_y >= bg_pixel_height) { new_y -= bg_pixel_height; }
				while(new_x < 0) { new_x += bg_pixel_width; }
				while(new_y < 0) { new_y += bg_pixel_height; } 
			}

			//Only draw if no previous pixel was rendered
			if(!render_buffer_b[scanline_pixel_counter] || (bg_priority < render_buffer_b[scanline_pixel_counter]))
			{
				if(render_pixel)
				{
					//Determine source pixel X-Y coordinates
					src_x = new_x;
					src_y = new_y;

					raw_color = mem->memory_map[bitmap_addr + (src_y * bg_pixel_width) + src_x];
			
					if(raw_color)
					{
						scanline_buffer_b[scanline_pixel_counter] = lcd_stat.bg_pal_b[raw_color];
						render_buffer_b[scanline_pixel_counter] = bg_priority;
						sfx_buffer[scanline_pixel_counter] = (render_buffer_b[scanline_pixel_counter] | 0x80);
					}

					else { full_render = false; }
				}

				else { full_render = false; }
			}
				
			scanline_pixel_counter++;

			//Update texture position with DX and DY
			lcd_stat.bg_affine_b[affine_id].x_pos += lcd_stat.bg_affine_b[affine_id].dx;
			lcd_stat.bg_affine_b[affine_id].y_pos += lcd_stat.bg_affine_b[affine_id].dy;
		}

		//Update XREF and YREF for next line
		lcd_stat.bg_affine_b[affine_id].x_ref += lcd_stat.bg_affine_b[affine_id].dmx;
		lcd_stat.bg_affine_b[affine_id].y_ref += lcd_stat.bg_affine_b[affine_id].dmy;

		full_scanline_render_b = full_render;
	}
}

/****** Render BG Mode direct color scanline ******/
void NTR_LCD::render_bg_mode_direct(u32 bg_control)
{
	//Render Engine A
	if((bg_control & 0x1000) == 0)
	{
		//Grab BG ID
		u8 bg_id = (bg_control - 0x4000008) >> 1;
		u8 affine_id = (bg_id & 0x1);
		u8 bg_priority = lcd_stat.bg_priority_a[bg_id] + 1;

		//Reload X-Y references at start of frame
		if(lcd_stat.current_scanline == 0) { reload_affine_references(bg_control); }

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_a[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_a) { return; }

		bool full_render = true;

		u16 raw_color = 0;
		u8 scanline_pixel_counter = 0;

		u16 src_x, src_y = 0;
		double new_x, new_y = 0.0;
		u16 bg_pixel_width, bg_pixel_height = 0;

		u32 bitmap_addr = lcd_stat.bg_bitmap_base_addr_a[bg_id & 0x1];

		//Determine bitmap dimensions
		switch(lcd_stat.bg_size_a[bg_id])
		{
			case 0x0:
				bg_pixel_width = 128;
				bg_pixel_height = 128;
				break;

			case 0x1:
				bg_pixel_width = 256;
				bg_pixel_height = 256;
				break;

			case 0x2:
				bg_pixel_width = 512;
				bg_pixel_height = 256;
				break;

			case 0x3:
				bg_pixel_width = 512;
				bg_pixel_height = 512;
				break;
		}

		//Set current texture position at X and Y references
		lcd_stat.bg_affine_a[affine_id].x_pos = lcd_stat.bg_affine_a[affine_id].x_ref;
		lcd_stat.bg_affine_a[affine_id].y_pos = lcd_stat.bg_affine_a[affine_id].y_ref;
		
		for(int x = 0; x < 256; x++)
		{
			bool render_pixel = true;

			new_x = lcd_stat.bg_affine_a[affine_id].x_pos;
			new_y = lcd_stat.bg_affine_a[affine_id].y_pos;

			//Clip BG if coordinates overflow and overflow flag is not set
			if(!lcd_stat.bg_affine_a[affine_id].overflow)
			{
				if((new_x >= bg_pixel_width) || (new_x < 0)) { render_pixel = false; }
				if((new_y >= bg_pixel_height) || (new_y < 0)) { render_pixel = false; }
			}

			//Wrap BG if coordinates overflow and overflow flag is set
			else 
			{
				while(new_x >= bg_pixel_width) { new_x -= bg_pixel_width; }
				while(new_y >= bg_pixel_height) { new_y -= bg_pixel_height; }
				while(new_x < 0) { new_x += bg_pixel_width; }
				while(new_y < 0) { new_y += bg_pixel_height; } 
			}

			//Only draw if no previous pixel was rendered
			if(!render_buffer_a[scanline_pixel_counter] || (bg_priority < render_buffer_a[scanline_pixel_counter]))
			{
				if(render_pixel)
				{
					//Determine source pixel X-Y coordinates
					src_x = new_x;
					src_y = new_y;

					raw_color = mem->read_u16(bitmap_addr + (((src_y * bg_pixel_width) + src_x) * 2));
			
					//Convert 16-bit ARGB to 32-bit ARGB - Bit 15 is alpha transparency
					if(raw_color & 0x8000)
					{
						u8 red = ((raw_color & 0x1F) << 3);
						raw_color >>= 5;

						u8 green = ((raw_color & 0x1F) << 3);
						raw_color >>= 5;

						u8 blue = ((raw_color & 0x1F) << 3);

						scanline_buffer_a[scanline_pixel_counter] = 0xFF000000 | (red << 16) | (green << 8) | (blue);
						render_buffer_a[scanline_pixel_counter] = bg_priority;
						sfx_buffer[scanline_pixel_counter] = (render_buffer_a[scanline_pixel_counter] | 0x80);
					}

					else { full_render = false; }
				}

				else { full_render = false; }
			}

			scanline_pixel_counter++;

			//Update texture position with DX and DY
			lcd_stat.bg_affine_a[affine_id].x_pos += lcd_stat.bg_affine_a[affine_id].dx;
			lcd_stat.bg_affine_a[affine_id].y_pos += lcd_stat.bg_affine_a[affine_id].dy;
		}

		//Update XREF and YREF for next line
		lcd_stat.bg_affine_a[affine_id].x_ref += lcd_stat.bg_affine_a[affine_id].dmx;
		lcd_stat.bg_affine_a[affine_id].y_ref += lcd_stat.bg_affine_a[affine_id].dmy;

		full_scanline_render_a = full_render;
	}

	//Render Engine B
	else
	{
		//Grab BG ID
		u8 bg_id = (bg_control - 0x4001008) >> 1;
		u8 affine_id = (bg_id & 0x1);
		u8 bg_priority = lcd_stat.bg_priority_b[bg_id] + 1;

		//Reload X-Y references at start of frame
		if(lcd_stat.current_scanline == 0) { reload_affine_references(bg_control); }

		//Abort rendering if this bg is disabled
		if(!lcd_stat.bg_enable_b[bg_id]) { return; }

		//Abort rendering if BGs with high priority have already completely rendered a scanline
		if(full_scanline_render_b) { return; }

		bool full_render = true;

		u16 raw_color = 0;
		u8 scanline_pixel_counter = 0;

		u16 src_x, src_y = 0;
		double new_x, new_y = 0.0;
		u16 bg_pixel_width, bg_pixel_height = 0;

		u32 bitmap_addr = lcd_stat.bg_bitmap_base_addr_b[bg_id & 0x1];

		//Determine bitmap dimensions
		switch(lcd_stat.bg_size_b[bg_id])
		{
			case 0x0:
				bg_pixel_width = 128;
				bg_pixel_height = 128;
				break;

			case 0x1:
				bg_pixel_width = 256;
				bg_pixel_height = 256;
				break;

			case 0x2:
				bg_pixel_width = 512;
				bg_pixel_height = 256;
				break;

			case 0x3:
				bg_pixel_width = 512;
				bg_pixel_height = 512;
				break;
		}

		//Set current texture position at X and Y references
		lcd_stat.bg_affine_b[affine_id].x_pos = lcd_stat.bg_affine_b[affine_id].x_ref;
		lcd_stat.bg_affine_b[affine_id].y_pos = lcd_stat.bg_affine_b[affine_id].y_ref;
		
		for(int x = 0; x < 256; x++)
		{
			bool render_pixel = true;

			new_x = lcd_stat.bg_affine_b[affine_id].x_pos;
			new_y = lcd_stat.bg_affine_b[affine_id].y_pos;

			//Clip BG if coordinates overflow and overflow flag is not set
			if(!lcd_stat.bg_affine_b[affine_id].overflow)
			{
				if((new_x >= bg_pixel_width) || (new_x < 0)) { render_pixel = false; }
				if((new_y >= bg_pixel_height) || (new_y < 0)) { render_pixel = false; }
			}

			//Wrap BG if coordinates overflow and overflow flag is set
			else 
			{
				while(new_x >= bg_pixel_width) { new_x -= bg_pixel_width; }
				while(new_y >= bg_pixel_height) { new_y -= bg_pixel_height; }
				while(new_x < 0) { new_x += bg_pixel_width; }
				while(new_y < 0) { new_y += bg_pixel_height; } 
			}

			//Only draw if no previous pixel was rendered
			if(!render_buffer_b[scanline_pixel_counter] || (bg_priority < render_buffer_b[scanline_pixel_counter]))
			{
				if(render_pixel)
				{
					//Determine source pixel X-Y coordinates
					src_x = new_x;
					src_y = new_y;

					raw_color = mem->read_u16(bitmap_addr + (((src_y * bg_pixel_width) + src_x) * 2));
			
					//Convert 16-bit ARGB to 32-bit ARGB - Bit 15 is alpha transparency
					if(raw_color & 0x8000)
					{
						u8 red = ((raw_color & 0x1F) << 3);
						raw_color >>= 5;

						u8 green = ((raw_color & 0x1F) << 3);
						raw_color >>= 5;

						u8 blue = ((raw_color & 0x1F) << 3);

						scanline_buffer_b[scanline_pixel_counter] = 0xFF000000 | (red << 16) | (green << 8) | (blue);
						render_buffer_b[scanline_pixel_counter] = bg_priority;
						sfx_buffer[scanline_pixel_counter] = (render_buffer_b[scanline_pixel_counter] | 0x80);
					}

					else { full_render = false; }
				}

				else { full_render = false; }
			}

			scanline_pixel_counter++;

			//Update texture position with DX and DY
			lcd_stat.bg_affine_b[affine_id].x_pos += lcd_stat.bg_affine_b[affine_id].dx;
			lcd_stat.bg_affine_b[affine_id].y_pos += lcd_stat.bg_affine_b[affine_id].dy;
		}

		//Update XREF and YREF for next line
		lcd_stat.bg_affine_b[affine_id].x_ref += lcd_stat.bg_affine_b[affine_id].dmx;
		lcd_stat.bg_affine_b[affine_id].y_ref += lcd_stat.bg_affine_b[affine_id].dmy;

		full_scanline_render_b = full_render;
	}
}

/****** Render pixels for a given scanline (per-pixel) ******/
void NTR_LCD::render_scanline()
{
	//Engine A - Render based on display modes
	switch(lcd_stat.display_mode_a)
	{
		//Display Mode 0 - Blank screen
		case 0x0:

		//Forced Blank
		case 0x80:
		case 0x81:
		case 0x82:
			for(u16 x = 0; x < 256; x++) { scanline_buffer_a[x] = 0xFFFFFFFF; }
			break;

		//Display Mode 1 - Tiled BG and OBJ
		case 0x1:
			render_bg_scanline(NDS_DISPCNT_A);
			break;

		//Display Mode 2 - VRAM
		case 0x2:
			{
				u8 vram_block = ((lcd_stat.display_control_a >> 18) & 0x3);
				u32 vram_addr = lcd_stat.vram_bank_addr[vram_block] + (lcd_stat.current_scanline * 256 * 2);

				for(u16 x = 0; x < 256; x++)
				{
					u16 color_bytes = mem->read_u16_fast(vram_addr + (x << 1));

					u8 red = ((color_bytes & 0x1F) << 3);
					color_bytes >>= 5;

					u8 green = ((color_bytes & 0x1F) << 3);
					color_bytes >>= 5;

					u8 blue = ((color_bytes & 0x1F) << 3);

					scanline_buffer_a[x] = 0xFF000000 | (red << 16) | (green << 8) | (blue);
				}
			}

			break;

		//Display Mode 3 - Main Memory
		default:
			std::cout<<"LCD::Warning - Engine A - Unsupported Display Mode 3 \n";
			break;
	}

	//Engine B - Render based on display modes
	switch(lcd_stat.display_mode_b)
	{
		//Display Mode 0 - Blank screen
		case 0x0:

		//Forced Blank
		case 0x80:
		case 0x81:
			for(u16 x = 0; x < 256; x++) { scanline_buffer_b[x] = 0xFFFFFFFF; }
			break;

		//Display Mode 1 - Tiled BG and OBJ
		case 0x1:
			render_bg_scanline(NDS_DISPCNT_B);
			break;

		//Modes 2 and 3 unsupported by Engine B
		default:
			std::cout<<"LCD::Warning - Engine B - Unsupported Display Mode " << std::dec << (int)lcd_stat.display_mode_b << "\n";
			break;
	}		
}

/****** Apply SFX to scanline pixels ******/
void NTR_LCD::apply_sfx(u32 bg_control)
{
	//Calculate SFX buffer
	for(u16 x = 0; x < 256; x++)
	{
		if((bg_control == NDS_DISPCNT_A) && (!sfx_buffer[x])) { sfx_buffer[x] = render_buffer_a[x]; }
		else if((bg_control == NDS_DISPCNT_B) && (!sfx_buffer[x])) { sfx_buffer[x] = render_buffer_b[x]; }
	}

	//TODO - Use Window coordinates to determine if SFX is applicable here
	//TODO - Determine is SFX can be applied here based on various other conditions

	nds_sfx_types temp_type = (bg_control == NDS_DISPCNT_A) ? lcd_stat.current_sfx_type_a : lcd_stat.current_sfx_type_b;

	//Apply the specified SFX
	switch(temp_type)
	{
		case NDS_BRIGHTNESS_UP:
			brightness_up(bg_control); 
			break;

		case NDS_BRIGHTNESS_DOWN:
			brightness_down(bg_control); 
			break;
	}
}

/****** SFX - Adjust scanline brightness up ******/
void NTR_LCD::brightness_up(u32 bg_control)
{
	u8 target = 0;
	u8 red = 0;
	u8 green = 0;
	u8 blue = 0;
	u8 layer = 0;
	u16 result = 0;
	u32 color = 0;

	bool sfx_target = false;
	double coef = (bg_control == NDS_DISPCNT_A) ? lcd_stat.brightness_coef_a : lcd_stat.brightness_coef_b;

	for(u16 x = 0; x < 256; x++)
	{
		//Determine if target is valid
		switch(sfx_buffer[x])
		{
			case 0x0: target = 5; break;
			case 0x1: target = 0; break;
			case 0x2: target = 1; break;
			case 0x3: target = 2; break;
			case 0x4: target = 3; break;

			case 0x81:
			case 0x82:
			case 0x83:
			case 0x84:
				target = 4; break;
		}

		sfx_target = (bg_control == NDS_DISPCNT_A) ? lcd_stat.sfx_target_a[target][0] : lcd_stat.sfx_target_b[target][0];
		layer = sfx_buffer[x] - 1;

		//Proceed with SFX
		if(sfx_target)
		{
			//Pull color from backdrop
			if(layer == 0xFF) { color = (bg_control == NDS_DISPCNT_A) ? lcd_stat.bg_pal_a[0] : lcd_stat.bg_pal_b[0]; }

			//Pull color from BG layers
			else { color = line_buffer[layer][x]; }

			//Increase RGB intensities
			red = ((color >> 19) & 0x1F);
			result = red + ((0x1F - red) * coef);
			red = (result > 0x1F) ? 0x1F : result;

			green = ((color >> 11) & 0x1F);
			result = green + ((0x1F - green) * coef);
			green = (result > 0x1F) ? 0x1F : result;

			blue = ((color >> 3) & 0x1F);
			result = blue + ((0x1F - blue) * coef);
			blue = (result > 0x1F) ? 0x1F : result;

			//Copy 32-bit color to scanline buffer
			if(bg_control == NDS_DISPCNT_A) { scanline_buffer_a[x] = 0xFF000000 | (red << 19) | (green << 11) | (blue << 3); }
			else { scanline_buffer_b[x] = 0xFF000000 | (red << 19) | (green << 11) | (blue << 3); }
		}
	}
}

/****** SFX - Adjust scanline brightness down ******/
void NTR_LCD::brightness_down(u32 bg_control)
{
	u8 target = 0;
	u8 red = 0;
	u8 green = 0;
	u8 blue = 0;
	u8 layer = 0;
	u16 result = 0;
	u32 color = 0;

	bool sfx_target = false;
	double coef = (bg_control == NDS_DISPCNT_A) ? lcd_stat.brightness_coef_a : lcd_stat.brightness_coef_b;

	for(u16 x = 0; x < 256; x++)
	{
		//Determine if target is valid
		switch(sfx_buffer[x])
		{
			case 0x0: target = 5; break;
			case 0x1: target = 0; break;
			case 0x2: target = 1; break;
			case 0x3: target = 2; break;
			case 0x4: target = 3; break;

			case 0x81:
			case 0x82:
			case 0x83:
			case 0x84:
				target = 4; break;
		}

		sfx_target = (bg_control == NDS_DISPCNT_A) ? lcd_stat.sfx_target_a[target][0] : lcd_stat.sfx_target_b[target][0];
		layer = sfx_buffer[x] - 1;

		//Proceed with SFX
		if(sfx_target)
		{
			//Pull color from backdrop
			if(layer == 0xFF) { color = (bg_control == NDS_DISPCNT_A) ? lcd_stat.bg_pal_a[0] : lcd_stat.bg_pal_b[0]; }

			//Pull color from BG layers
			else { color = line_buffer[layer][x]; }

			//Decrease RGB intensities 
			red = ((color >> 19) & 0x1F);
			red -= (red * coef);

			green = ((color >> 11) & 0x1F);
			green -= (green * coef);

			blue = ((color >> 3) & 0x1F);
			blue -= (blue * coef);

			//Copy 32-bit color to scanline buffer
			if(bg_control == NDS_DISPCNT_A) { scanline_buffer_a[x] = 0xFF000000 | (red << 19) | (green << 11) | (blue << 3); }
			else { scanline_buffer_b[x] = 0xFF000000 | (red << 19) | (green << 11) | (blue << 3); }
		}
	}
}

/****** Adjusts master brightness before final scanline output ******/
void NTR_LCD::adjust_master_brightness()
{
	double factor = (lcd_stat.master_bright & 0xF) / 16.0;
	u32 color = 0;
	
	u8 r = 0;
	u8 g = 0;
	u8 b = 0;

	//Master Brightness Up
	if((lcd_stat.master_bright >> 14) == 0x1)
	{
		//Engine A pixels
		for(u32 x = 0; x < 256; x++)
		{
			color = scanline_buffer_a[x];

			r = (color >> 19) & 0x1F;
			r = r + ((63 - r) * factor);

			g = (color >> 11) & 0x1F;
			g = g + ((63 - g) * factor);

			b = (color >> 3) & 0x1F;
			b = b + ((63 - b) * factor);

			scanline_buffer_a[x] = 0xFF000000 | (r << 19) | (g << 11) | (b << 3);
		}

		//Engine B pixels
		for(u32 x = 0; x < 256; x++)
		{
			color = scanline_buffer_b[x];

			r = (color >> 19) & 0x1F;
			r = r + ((63 - r) * factor);

			g = (color >> 11) & 0x1F;
			g = g + ((63 - g) * factor);

			b = (color >> 3) & 0x1F;
			b = b + ((63 - b) * factor);

			scanline_buffer_b[x] = 0xFF000000 | (r << 19) | (g << 11) | (b << 3);
		}
	}

	//Master Bright Down
	if((lcd_stat.master_bright >> 14) == 0x2)
	{
		//Engine A pixels
		for(u32 x = 0; x < 256; x++)
		{
			color = scanline_buffer_a[x];

			r = (color >> 19) & 0x1F;
			r = r - (r * factor);

			g = (color >> 11) & 0x1F;
			g = g - (g * factor);

			b = (color >> 3) & 0x1F;
			b = b - (b * factor);

			scanline_buffer_a[x] = 0xFF000000 | (r << 19) | (g << 11) | (b << 3);
		}

		//Engine B pixels
		for(u32 x = 0; x < 256; x++)
		{
			color = scanline_buffer_b[x];

			r = (color >> 19) & 0x1F;
			r = r - (r * factor);

			g = (color >> 11) & 0x1F;
			g = g - (g * factor);

			b = (color >> 3) & 0x1F;
			b = b - (b * factor);

			scanline_buffer_b[x] = 0xFF000000 | (r << 19) | (g << 11) | (b << 3);
		}
	}
}	
		
/****** Immediately draw current buffer to the screen ******/
void NTR_LCD::update()
{
	//Use SDL
	if(config::sdl_render)
	{
		//Lock source surface
		if(SDL_MUSTLOCK(final_screen)){ SDL_LockSurface(final_screen); }
		u32* out_pixel_data = (u32*)final_screen->pixels;

		for(int a = 0; a < 0x18000; a++) { out_pixel_data[a] = screen_buffer[a]; }

		//Unlock source surface
		if(SDL_MUSTLOCK(final_screen)){ SDL_UnlockSurface(final_screen); }
		
		//Display final screen buffer - OpenGL
		if(config::use_opengl) { opengl_blit(); }

		//Display final screen buffer - SDL
		else
		{
			if(SDL_UpdateWindowSurface(window) != 0) { std::cout<<"LCD::Error - Could not blit\n"; }
		}
	}

	//Use external rendering method (GUI)
	else
	{
		if(!config::use_opengl) { config::render_external_sw(screen_buffer); }

		else
		{
			//Lock source surface
			if(SDL_MUSTLOCK(final_screen)){ SDL_LockSurface(final_screen); }
			u32* out_pixel_data = (u32*)final_screen->pixels;

			for(int a = 0; a < 0x18000; a++) { out_pixel_data[a] = screen_buffer[a]; }

			//Unlock source surface
			if(SDL_MUSTLOCK(final_screen)){ SDL_UnlockSurface(final_screen); }

			config::render_external_hw(final_screen);
		}
	}
}


/****** Run LCD for one cycle ******/
void NTR_LCD::step()
{
	lcd_stat.lcd_clock++;

	//Process GX commands and states
	if(lcd_3D_stat.process_command) { process_gx_command(); }
	
	if(lcd_3D_stat.render_polygon) { render_geometry(); }

	//Mode 0 - Scanline rendering
	if(((lcd_stat.lcd_clock % 2130) <= 1536) && (lcd_stat.lcd_clock < 408960)) 
	{
		//Change mode
		if(lcd_stat.lcd_mode != 0) 
		{
			lcd_stat.lcd_mode = 0;

			//Reset HBlank flag in DISPSTAT
			lcd_stat.display_stat_nds9 &= ~0x2;
			lcd_stat.display_stat_nds7 &= ~0x2;
			
			lcd_stat.current_scanline++;

			//Update VCOUNT
			mem->write_u16_fast(NDS_VCOUNT, lcd_stat.current_scanline);

			scanline_compare();
		}
	}

	//Mode 1 - H-Blank
	else if(((lcd_stat.lcd_clock % 2130) > 1536) && (lcd_stat.lcd_clock < 408960))
	{
		//Change mode
		if(lcd_stat.lcd_mode != 1) 
		{
			lcd_stat.lcd_mode = 1;

			//Set HBlank flag in DISPSTAT
			lcd_stat.display_stat_nds9 |= 0x2;
			lcd_stat.display_stat_nds7 |= 0x2;

			//Trigger HBlank IRQ
			if(lcd_stat.hblank_irq_enable_a) { mem->nds9_if |= 0x2; }
			if(lcd_stat.hblank_irq_enable_b) { mem->nds7_if |= 0x2; }

			//Update 2D engine OAM
			if(lcd_stat.oam_update) { update_oam(); }

			//Update OBJ render list
			update_obj_render_list();

			//Update 2D engine palettes
			if(lcd_stat.bg_pal_update_a || lcd_stat.bg_pal_update_b || lcd_stat.bg_ext_pal_update_a || lcd_stat.bg_ext_pal_update_b
			|| lcd_stat.obj_pal_update_a || lcd_stat.obj_pal_update_b || lcd_stat.obj_ext_pal_update_a || lcd_stat.obj_ext_pal_update_b) { update_palettes(); }

			//Update BG control registers - Engine A
			if(lcd_stat.update_bg_control_a)
			{
				mem->write_u8(NDS_BG0CNT_A, mem->memory_map[NDS_BG0CNT_A]);
				mem->write_u8(NDS_BG1CNT_A, mem->memory_map[NDS_BG1CNT_A]);
				mem->write_u8(NDS_BG2CNT_A, mem->memory_map[NDS_BG2CNT_A]);
				mem->write_u8(NDS_BG3CNT_A, mem->memory_map[NDS_BG3CNT_A]);
				lcd_stat.update_bg_control_a = false;
			}

			//Update BG control registers - Engine B
			if(lcd_stat.update_bg_control_b)
			{
				mem->write_u8(NDS_BG0CNT_B, mem->memory_map[NDS_BG0CNT_B]);
				mem->write_u8(NDS_BG1CNT_B, mem->memory_map[NDS_BG1CNT_B]);
				mem->write_u8(NDS_BG2CNT_B, mem->memory_map[NDS_BG2CNT_B]);
				mem->write_u8(NDS_BG3CNT_B, mem->memory_map[NDS_BG3CNT_B]);
				lcd_stat.update_bg_control_b = false;
			}

			//Render scanline data
			render_scanline();

			//Apply Master Brightness if necessary
			if(lcd_stat.master_bright != lcd_stat.old_master_bright) { adjust_master_brightness(); }

			u32 render_position = (lcd_stat.current_scanline * config::sys_width);

			//Swap top and bottom if POWERCNT1 Bit 15 is not set, otherwise A is top, B is bottom
			u16 disp_a_offset = (mem->power_cnt1 & 0x8000) ? 0 : 0xC000;
			u16 disp_b_offset = (mem->power_cnt1 & 0x8000) ? 0xC000 : 0;

			//Swap top and bottom if LCD configuration calls for it
			if(config::lcd_config & 0x1)
			{
				disp_a_offset = (disp_a_offset) ? 0 : 0xC000;
				disp_b_offset = (disp_b_offset) ? 0 : 0xC000;
			}

			//Horizontal vs. Vertical mode
			if(config::lcd_config & 0x2)
			{
				disp_a_offset = (disp_a_offset) ? 0x100 : 0;
				disp_b_offset = (disp_b_offset) ? 0x100 : 0;
			} 
			
			//Push scanline pixel data to screen buffer
			for(u16 x = 0; x < 256; x++)
			{
				screen_buffer[render_position + x + disp_a_offset] = scanline_buffer_a[x];
				screen_buffer[render_position + x + disp_b_offset] = scanline_buffer_b[x];
			}

			//Start HBlank DMA
			mem->start_hblank_dma();
		}
	}

	//Mode 2 - VBlank
	else
	{
		//Change mode
		if(lcd_stat.lcd_mode != 2) 
		{
			lcd_stat.lcd_mode = 2;

			//Set VBlank flag in DISPSTAT
			lcd_stat.display_stat_nds9 |= 0x1;
			lcd_stat.display_stat_nds7 |= 0x1;

			//Reset HBlank flag in DISPSTAT
			lcd_stat.display_stat_nds9 &= ~0x2;
			lcd_stat.display_stat_nds7 &= ~0x2;

			//Trigger VBlank IRQ
			if(lcd_stat.vblank_irq_enable_a) { mem->nds9_if |= 0x1; }
			if(lcd_stat.vblank_irq_enable_b) { mem->nds7_if |= 0x1; }

			//Increment scanline count
			lcd_stat.current_scanline++;
			scanline_compare();

			//Update VCOUNT
			mem->write_u16_fast(NDS_VCOUNT, lcd_stat.current_scanline);

			//Display any OSD messages
			if(config::osd_count)
			{
				config::osd_count--;
				draw_osd_msg(config::osd_message, screen_buffer, 0, 0);
			}

			//Use SDL
			if(config::sdl_render)
			{
				//If using SDL and no OpenGL, manually stretch for fullscreen via SDL
				if((config::flags & SDL_WINDOW_FULLSCREEN_DESKTOP) && (!config::use_opengl))
				{
					//Lock source surface
					if(SDL_MUSTLOCK(original_screen)){ SDL_LockSurface(original_screen); }
					u32* out_pixel_data = (u32*)original_screen->pixels;

					for(int a = 0; a < 0x18000; a++) { out_pixel_data[a] = screen_buffer[a]; }

					//Unlock source surface
					if(SDL_MUSTLOCK(original_screen)){ SDL_UnlockSurface(original_screen); }
		
					//Blit the original surface to the final stretched one
					SDL_Rect dest_rect;
					dest_rect.w = config::sys_width * max_fullscreen_ratio;
					dest_rect.h = config::sys_height * max_fullscreen_ratio;
					dest_rect.x = ((config::win_width - dest_rect.w) >> 1);
					dest_rect.y = ((config::win_height - dest_rect.h) >> 1);
					SDL_BlitScaled(original_screen, NULL, final_screen, &dest_rect);

					if(SDL_UpdateWindowSurface(window) != 0) { std::cout<<"LCD::Error - Could not blit\n"; }
				}
					
				//Otherwise, render normally (SDL 1:1, OpenGL handles its own stretching)
				else
				{
					//Lock source surface
					if(SDL_MUSTLOCK(final_screen)){ SDL_LockSurface(final_screen); }
					u32* out_pixel_data = (u32*)final_screen->pixels;

					for(int a = 0; a < 0x18000; a++) { out_pixel_data[a] = screen_buffer[a]; }

					//Unlock source surface
					if(SDL_MUSTLOCK(final_screen)){ SDL_UnlockSurface(final_screen); }
		
					//Display final screen buffer - OpenGL
					if(config::use_opengl) { opengl_blit(); }
				
					//Display final screen buffer - SDL
					else 
					{
						if(SDL_UpdateWindowSurface(window) != 0) { std::cout<<"LCD::Error - Could not blit\n"; }
					}
				}
			}

			//Use external rendering method (GUI)
			else
			{
				if(!config::use_opengl) { config::render_external_sw(screen_buffer); }

				else
				{
					//Lock source surface
					if(SDL_MUSTLOCK(final_screen)){ SDL_LockSurface(final_screen); }
					u32* out_pixel_data = (u32*)final_screen->pixels;

					for(int a = 0; a < 0x18000; a++) { out_pixel_data[a] = screen_buffer[a]; }

					//Unlock source surface
					if(SDL_MUSTLOCK(final_screen)){ SDL_UnlockSurface(final_screen); }

					config::render_external_hw(final_screen);
				}
			}

			//Limit framerate
			if(!config::turbo)
			{
				frame_current_time = SDL_GetTicks();
				if((frame_current_time - frame_start_time) < 16) { SDL_Delay(16 - (frame_current_time - frame_start_time));}
				frame_start_time = SDL_GetTicks();
			}

			//Update FPS counter + title
			fps_count++;
			if(((SDL_GetTicks() - fps_time) >= 1000) && (config::sdl_render))
			{ 
				fps_time = SDL_GetTicks(); 
				config::title.str("");
				config::title << "GBE+ " << fps_count << "FPS";
				SDL_SetWindowTitle(window, config::title.str().c_str());
				fps_count = 0; 
			}

			//Check for screen resize - Horizontal vs Vertical
			if(config::request_resize)
			{
				if(config::resize_mode) { config::lcd_config |= 0x2; }
				else { config::lcd_config &= ~0x2; }

				config::sys_width = (config::resize_mode) ? 512 : 256;
				config::sys_height = (config::resize_mode) ? 192 : 384;
				screen_buffer.clear();
				screen_buffer.resize(0x18000, 0xFFFFFFFF);
					
				if((window != NULL) && (config::sdl_render)) { SDL_DestroyWindow(window); }
				init();
					
				if(config::sdl_render) { config::request_resize = false; }
			}

			//3D - Swap Buffers command
			if((lcd_3D_stat.gx_state & 0x80) && (lcd_stat.display_stat_nds9 & 0x1))
			{
				lcd_3D_stat.buffer_id += 1;
				lcd_3D_stat.buffer_id &= 0x1;
				lcd_3D_stat.vertex_list_index = 0;
				lcd_3D_stat.gx_state &= ~0x80;
				lcd_3D_stat.render_polygon = false;
				lcd_3D_stat.poly_count = 0;
				lcd_3D_stat.vert_count = 0;

				//Clear polygons (and vertices as well)
				gx_triangles.clear();
				gx_quads.clear();

				//Clear 3D buffer and fill with rear plane
				gx_screen_buffer[lcd_3D_stat.buffer_id].clear();
				gx_screen_buffer[lcd_3D_stat.buffer_id].resize(0xC000, lcd_3D_stat.rear_plane_color);

				//Clear render buffer
				if(!lcd_3D_stat.rear_plane_alpha) { gx_render_buffer.assign(0xC000, 0); }
				else { gx_render_buffer.assign(0xC000, 1); }

				//Clear z-buffer
				gx_z_buffer.assign(0xC000, -4096);
			}

			//Start VBlank DMA
			mem->start_vblank_dma();
		}

		//Increment scanline after HBlank starts
		else if((lcd_stat.lcd_clock % 2130) == 1536)
		{
			lcd_stat.current_scanline++;
			scanline_compare();

			//Update VCOUNT
			mem->write_u16_fast(NDS_VCOUNT, lcd_stat.current_scanline);

			//Reset VBlank flag in DISPSTAT on line 261
			if(lcd_stat.current_scanline == 261)
			{
				lcd_stat.display_stat_nds9 &= ~0x1;
				lcd_stat.display_stat_nds7 &= ~0x1;
			}

			//Set HBlank flag in DISPSTAT
			lcd_stat.display_stat_nds9 |= 0x2;
			lcd_stat.display_stat_nds7 |= 0x2;

			//Trigger HBlank IRQ
			if(lcd_stat.hblank_irq_enable_a) { mem->nds9_if |= 0x2; }
			if(lcd_stat.hblank_irq_enable_b) { mem->nds7_if |= 0x2; }
		}

		//Turn off HBlank flag at the start of a scanline
		else if((lcd_stat.lcd_clock % 2130) == 0)
		{
			//Reset HBlank flag in DISPSTAT
			lcd_stat.display_stat_nds9 &= ~0x2;
			lcd_stat.display_stat_nds7 &= ~0x2;

			//Reset LCD clock
			if(lcd_stat.current_scanline == 263)
			{
				lcd_stat.lcd_clock -= 560190;
				lcd_stat.current_scanline = 0xFFFF;

				//Start Display Sync DMA
				mem->start_dma(3);

				//Reset master brightness flag
				lcd_stat.master_bright = lcd_stat.old_master_bright;
			}
		}
	}
}

/****** Compare VCOUNT to LYC ******/
void NTR_LCD::scanline_compare()
{
	//Raise VCOUNT interrupt - Engine A
	if(lcd_stat.current_scanline == lcd_stat.lyc_nds9)
	{
		//Check to see if the VCOUNT IRQ is enabled in DISPSTAT
		if(lcd_stat.vcount_irq_enable_a) { mem->nds9_if |= 0x4; }

		//Toggle VCOUNT flag ON
		lcd_stat.display_stat_nds9 |= 0x4;
	}

	else
	{
		//Toggle VCOUNT flag OFF
		lcd_stat.display_stat_nds9 &= ~0x4;
	}

	//Raise VCOUNT interrupt - Engine B
	if(lcd_stat.current_scanline == lcd_stat.lyc_nds7)
	{
		//Check to see if the VCOUNT IRQ is enabled in DISPSTAT
		if(lcd_stat.vcount_irq_enable_b) { mem->nds7_if |= 0x4; }

		//Toggle VCOUNT flag ON
		lcd_stat.display_stat_nds7 |= 0x4;
	}

	else
	{
		//Toggle VCOUNT flag OFF
		lcd_stat.display_stat_nds7 &= ~0x4;
	}
}

/****** Grabs the most current X-Y references for affine backgrounds ******/
void NTR_LCD::reload_affine_references(u32 bg_control)
{
	u32 x_raw, y_raw;
	bool engine_a;
	u8 aff_id;

	//Read X-Y references from memory, determine which engine it belongs to
	switch(bg_control)
	{
		case NDS_BG2CNT_A:
			x_raw = mem->read_u32_fast(NDS_BG2X_A);
			y_raw = mem->read_u32_fast(NDS_BG2Y_A);
			engine_a = true;
			aff_id = 0;
			break;

		case NDS_BG3CNT_A:
			x_raw = mem->read_u32_fast(NDS_BG3X_A);
			y_raw = mem->read_u32_fast(NDS_BG3Y_A);
			engine_a = true;
			aff_id = 1;
			break;

		case NDS_BG2CNT_B:
			x_raw = mem->read_u32_fast(NDS_BG2X_B);
			y_raw = mem->read_u32_fast(NDS_BG2Y_B);
			engine_a = false;
			aff_id = 0;
			break;

		case NDS_BG3CNT_B:
			x_raw = mem->read_u32_fast(NDS_BG3X_B);
			y_raw = mem->read_u32_fast(NDS_BG3Y_B);
			aff_id = 1;
			break;

		default: return;
	}
			
	//Get X reference point
	if(x_raw & 0x8000000) 
	{ 
		u32 x = ((x_raw >> 8) - 1);
		x = (~x & 0x7FFFF);
		
		if(engine_a) { lcd_stat.bg_affine_a[aff_id].x_ref = -1.0 * x; }
		else { lcd_stat.bg_affine_b[aff_id].x_ref = -1.0 * x; }
	}
	
	else
	{
		if(engine_a) { lcd_stat.bg_affine_a[aff_id].x_ref = (x_raw >> 8) & 0x7FFFF; }
		else { lcd_stat.bg_affine_b[aff_id].x_ref = (x_raw >> 8) & 0x7FFFF; }
	}
	
	if((x_raw & 0xFF) != 0)
	{
		if(engine_a) { lcd_stat.bg_affine_a[aff_id].x_ref += (x_raw & 0xFF) / 256.0; }
		else { lcd_stat.bg_affine_b[aff_id].x_ref += (x_raw & 0xFF) / 256.0; }
	}

	//Set current X position as the new reference point
	if(engine_a) { lcd_stat.bg_affine_a[aff_id].x_pos = lcd_stat.bg_affine_a[aff_id].x_ref; }
	else { lcd_stat.bg_affine_b[aff_id].x_pos = lcd_stat.bg_affine_b[aff_id].x_ref; }

	//Get Y reference point
	if(y_raw & 0x8000000) 
	{ 
		u32 y = ((y_raw >> 8) - 1);
		y = (~y & 0x7FFFF);
		
		if(engine_a) { lcd_stat.bg_affine_a[aff_id].y_ref = -1.0 * y; }
		else { lcd_stat.bg_affine_b[aff_id].y_ref = -1.0 * y; }
	}
	
	else
	{
		if(engine_a) { lcd_stat.bg_affine_a[aff_id].y_ref = (y_raw >> 8) & 0x7FFFF; }
		else { lcd_stat.bg_affine_b[aff_id].y_ref = (y_raw >> 8) & 0x7FFFF; }
	}
	
	if((y_raw & 0xFF) != 0)
	{
		if(engine_a) { lcd_stat.bg_affine_a[aff_id].y_ref += (y_raw & 0xFF) / 256.0; }
		else { lcd_stat.bg_affine_b[aff_id].y_ref += (y_raw & 0xFF) / 256.0; }
	}

	//Set current Y position as the new reference point
	if(engine_a) { lcd_stat.bg_affine_a[aff_id].y_pos = lcd_stat.bg_affine_a[aff_id].y_ref; }
	else { lcd_stat.bg_affine_b[aff_id].y_pos = lcd_stat.bg_affine_b[aff_id].y_ref; }
}

/****** Generates the game icon (non-animated) from NDS cart header ******/
bool NTR_LCD::get_cart_icon(SDL_Surface* nds_icon)
{
	u32 icon_base = mem->read_cart_u32(0x68);
	
	if(!icon_base) { return false; }

	//Create SDL_Surface for icon
	nds_icon = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32, 0, 0, 0, 0);

	//Lock source surface
	if(SDL_MUSTLOCK(nds_icon)){ SDL_LockSurface(nds_icon); }
	u32* icon_data = (u32*)nds_icon->pixels;

	//Generate palettes
	u32 pals[16];

	for(u32 x = 0; x < 16; x++)
	{
		u16 raw_pal = mem->read_cart_u16(icon_base + 0x220 + (x << 1));
		
		u8 red = ((raw_pal & 0x1F) << 3);
		raw_pal >>= 5;

		u8 green = ((raw_pal & 0x1F) << 3);
		raw_pal >>= 5;

		u8 blue = ((raw_pal & 0x1F) << 3);

		pals[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
	}

	u16 data_pos = 0;

	//Cycle through all tiles to generate icon data
	for(u32 x = 0; x < 16; x++)
	{
		u16 pixel_pos = ((x / 4) * 256) + ((x % 4) * 8);

		for(u32 y = 0; y < 32; y++)
		{
			u8 icon_char = mem->cart_data[icon_base + 0x20 + data_pos];
			u8 char_r = (icon_char >> 4);
			u8 char_l = (icon_char & 0xF);
			data_pos++;

			icon_data[pixel_pos++] = pals[char_l];
			icon_data[pixel_pos++] = pals[char_r];

			if((pixel_pos % 8) == 0) { pixel_pos += 24; }
		}
	}

	//Unlock source surface
	if(SDL_MUSTLOCK(nds_icon)){ SDL_UnlockSurface(nds_icon); }

	return true;
}

/****** Saves the game icon (non-animated) from NDS cart header ******/
bool NTR_LCD::save_cart_icon(std::string nds_icon_file)
{
	u32 icon_base = mem->read_cart_u32(0x68);
	
	if(!icon_base) { return false; }

	//Create SDL_Surface for icon
	SDL_Surface* nds_icon = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32, 0, 0, 0, 0);

	//Lock source surface
	if(SDL_MUSTLOCK(nds_icon)){ SDL_LockSurface(nds_icon); }
	u32* icon_data = (u32*)nds_icon->pixels;

	//Generate palettes
	u32 pals[16];

	for(u32 x = 0; x < 16; x++)
	{
		u16 raw_pal = mem->read_cart_u16(icon_base + 0x220 + (x << 1));
		
		u8 red = ((raw_pal & 0x1F) << 3);
		raw_pal >>= 5;

		u8 green = ((raw_pal & 0x1F) << 3);
		raw_pal >>= 5;

		u8 blue = ((raw_pal & 0x1F) << 3);

		pals[x] =  0xFF000000 | (red << 16) | (green << 8) | (blue);
	}

	u16 data_pos = 0;

	//Cycle through all tiles to generate icon data
	for(u32 x = 0; x < 16; x++)
	{
		u16 pixel_pos = ((x / 4) * 256) + ((x % 4) * 8);

		for(u32 y = 0; y < 32; y++)
		{
			u8 icon_char = mem->cart_data[icon_base + 0x20 + data_pos];
			u8 char_r = (icon_char >> 4);
			u8 char_l = (icon_char & 0xF);
			data_pos++;

			icon_data[pixel_pos++] = pals[char_l];
			icon_data[pixel_pos++] = pals[char_r];

			if((pixel_pos % 8) == 0) { pixel_pos += 24; }
		}
	}

	//Unlock source surface
	if(SDL_MUSTLOCK(nds_icon)){ SDL_UnlockSurface(nds_icon); }

	SDL_SaveBMP(nds_icon, nds_icon_file.c_str());

	return true;
}
