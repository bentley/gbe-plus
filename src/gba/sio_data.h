// GB Enhanced+ Copyright Daniel Baxter 2018
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : sio_data.h
// Date : August 13, 2018
// Description : Serial I/O data
//
// Defines the SIO data structures and enums that the MMU will update whenever values are written in memory
// Only the SIO should read values from this namespace. Only the MMU should write values to this namespace.

#ifndef GBA_SIO_DATA
#define GBA_SIO_DATA

#include "common.h"

//Serial Input-Output device-type enumeration
enum agb_sio_types
{ 
	NO_GBA_DEVICE,
	INVALID_GBA_DEVICE,
	GBA_LINK,
	GBA_PLAYER_RUMBLE,
	GBA_SOUL_DOLL_ADAPTER,
	GBA_BATTLE_CHIP_GATE,
	GBA_POWER_ANTENNA,
	GBA_MOBILE_ADAPTER,
	GBA_MULTI_PLUST_ON_SYSTEM,
};

//Serial Input-Output device mode enumeration
enum agb_sio_modes
{
	NORMAL_8BIT,
	NORMAL_32BIT,
	MULTIPLAY_16BIT,
	UART,
	GENERAL_PURPOSE,
	JOY_BUS,
};

//GB Player Rumble enumerations
enum gb_player_rumble_state
{
	GB_PLAYER_RUMBLE_INACTIVE,
	GB_PLAYER_RUMBLE_ACTIVE,
};

//Soul Doll Adapter enumerations
enum soul_doll_adapter_state
{
	GBA_SOUL_DOLL_ADAPTER_ECHO,
	GBA_SOUL_DOLL_ADAPTER_READ,
	GBA_SOUL_DOLL_ADAPTER_WRITE,
};

//Battle Chip Gate enumerations
enum battle_chip_gate_state
{
	GBA_BATTLE_CHIP_GATE_STANDBY,
	GBA_BATTLE_CHIP_GATE_CHIP_IN,
};

//Mobile Adapter enumerations
enum agb_mobile_state
{
	AGB_GBMA_AWAITING_PACKET,
	AGB_GBMA_RECEIVE_HEADER,
	AGB_GBMA_RECEIVE_DATA,
	AGB_GBMA_RECEIVE_CHECKSUM,
	AGB_GBMA_ACKNOWLEDGE_PACKET,
	AGB_GBMA_ECHO_PACKET,
};

//Multi Plust On System enumerations
enum agb_mpos_state
{
	AGB_MPOS_INIT,
	AGB_MPOS_SEND_DATA,
};

struct agb_sio_data
{
	bool connected;
	bool active_transfer;
	bool internal_clock;
	bool sync;
	bool connection_ready;
	bool emu_device_ready;
	u32 sync_counter;
	u32 sync_clock;
	u32 transfer_data;
	u32 shift_counter;
	u32 shift_clock;
	u32 shifts_left;
	u16 cnt;
	u16 r_cnt;
	u8 player_id;
	agb_sio_types sio_type;
	agb_sio_modes sio_mode;
};

#endif // GBA_SIO_DATA 
