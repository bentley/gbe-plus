// GB Enhanced+ Copyright Daniel Baxter 2017
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : apu_data.h
// Date : September 10, 2017
// Description : Core data
//
// Defines the APU data structures that the MMU will update whenever values are written in memory
// Only the APU should read values from this namespace. Only the MMU should write values to this namespace.

#ifndef NDS_APU_DATA
#define NDS_APU_DATA

#include "common.h"

struct ntr_apu_data
{
	//Digital channels, new to the GBA
	struct digital_channels
	{
		double output_frequency;
		u32 data_src;
		u32 data_pos;
		u32 loop_start;
		u32 length;
		u32 samples;
		u32 cnt;

		bool playing;
		bool enable;
		bool right_enable;
		bool left_enable;
	} channel[16];

	bool sound_on;
	bool stereo;

	u8 main_volume;
	double sample_rate;

	u8 channel_master_volume;
	double channel_left_volume;
	double channel_right_volume;
};

#endif // NDS_APU_DATA
 
