// GB Enhanced+ Copyright Daniel Baxter 2015
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : general_settings.h
// Date : June 28, 2015
// Description : Main menu
//
// Dialog for various options
// Deals with Graphics, Audio, Input, Paths, etc

#include <iostream>

#include "general_settings.h"
#include "render.h"
#include "qt_common.h"

#include "common/config.h"
#include "common/cgfx_common.h"
#include "common/util.h"

/****** General settings constructor ******/
gen_settings::gen_settings(QWidget *parent) : QDialog(parent)
{
	init_chip_list[0] = config::chip_list[0];
	init_chip_list[1] = config::chip_list[1];
	init_chip_list[2] = config::chip_list[2];
	init_chip_list[3] = config::chip_list[3];

	//Set up tabs
	tabs = new QTabWidget(this);
	
	general = new QDialog;
	display = new QDialog;
	sound = new QDialog;
	controls = new QDialog;
	netplay = new QDialog;
	paths = new QDialog;

	tabs->addTab(general, tr("General"));
	tabs->addTab(display, tr("Display"));
	tabs->addTab(sound, tr("Sound"));
	tabs->addTab(controls, tr("Controls"));
	tabs->addTab(netplay, tr("Netplay"));
	tabs->addTab(paths, tr("Paths"));

	QWidget* button_set = new QWidget;
	tabs_button = new QDialogButtonBox(QDialogButtonBox::Close);
	controls_combo = new QComboBox;
	controls_combo->addItem("Standard Controls");
	controls_combo->addItem("Advanced Controls");
	controls_combo->addItem("Hotkey Controls");
	controls_combo->addItem("Battle Chip Gate Controls");

	QHBoxLayout* button_layout = new QHBoxLayout;
	button_layout->setAlignment(Qt::AlignTop | Qt::AlignRight);
	button_layout->addWidget(controls_combo);
	button_layout->addWidget(tabs_button);
	button_set->setLayout(button_layout);

	//General settings - Emulated system type
	QWidget* sys_type_set = new QWidget(general);
	QLabel* sys_type_label = new QLabel("Emulated System Type : ", sys_type_set);
	sys_type = new QComboBox(sys_type_set);
	sys_type->setToolTip("Forces GBE+ to emulate a certain system");
	sys_type->addItem("Auto");
	sys_type->addItem("Game Boy [DMG]");
	sys_type->addItem("Game Boy Color [GBC]");
	sys_type->addItem("Game Boy Advance [GBA]");
	sys_type->addItem("Nintendo DS [NDS]");
	sys_type->addItem("Super Game Boy [SGB]");
	sys_type->addItem("Super Game Boy 2 [SGB2]");

	QHBoxLayout* sys_type_layout = new QHBoxLayout;
	sys_type_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	sys_type_layout->addWidget(sys_type_label);
	sys_type_layout->addWidget(sys_type);
	sys_type_set->setLayout(sys_type_layout);

	//General settings - Use BIOS
	QWidget* bios_set = new QWidget(general);
	QLabel* bios_label = new QLabel("Use BIOS/Boot ROM", bios_set);
	bios = new QCheckBox(bios_set);
	bios->setToolTip("Instructs GBE+ to boot a system's BIOS or Boot ROM");

	QHBoxLayout* bios_layout = new QHBoxLayout;
	bios_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	bios_layout->addWidget(bios);
	bios_layout->addWidget(bios_label);
	bios_set->setLayout(bios_layout);

	//General settings - Use Firmware
	QWidget* firmware_set = new QWidget(general);
	QLabel* firmware_label = new QLabel("Use NDS Firmware", firmware_set);
	firmware = new QCheckBox(firmware_set);
	firmware->setToolTip("Instructs GBE+ to boot using NDS firmware if applicable");

	QHBoxLayout* firmware_layout = new QHBoxLayout;
	firmware_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	firmware_layout->addWidget(firmware);
	firmware_layout->addWidget(firmware_label);
	firmware_set->setLayout(firmware_layout);

	//General settings - Emulate specialty game carts
	QWidget* special_cart_set = new QWidget(general);
	QLabel* special_cart_label = new QLabel("Special ROM Type : ", special_cart_set);
	special_cart = new QComboBox(special_cart_set);
	special_cart->setToolTip("Emulates various special cart setups");
	special_cart->addItem("None");
	special_cart->addItem("DMG - MBC1M");
	special_cart->addItem("DMG - MBC1S");
	special_cart->addItem("DMG - MMM01");
	special_cart->addItem("DMG - MBC30");
	special_cart->addItem("AGB - RTC");
	special_cart->addItem("AGB - Solar Sensor");
	special_cart->addItem("AGB - Rumble");
	special_cart->addItem("AGB - Gyro Sensor");
	special_cart->addItem("AGB - Tilt Sensor");

	QHBoxLayout* special_cart_layout = new QHBoxLayout;
	special_cart_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	special_cart_layout->addWidget(special_cart_label);
	special_cart_layout->addWidget(special_cart);
	special_cart_set->setLayout(special_cart_layout);

	//General settings - Use cheats
	QWidget* cheats_set = new QWidget(general);
	QLabel* cheats_label = new QLabel("Use cheats", cheats_set);
	QPushButton* edit_cheats = new QPushButton("Edit Cheats");
	cheats = new QCheckBox(cheats_set);
	cheats->setToolTip("Enables Game Genie and GameShark cheat codes");

	QHBoxLayout* cheats_layout = new QHBoxLayout;
	cheats_layout->addWidget(cheats, 0, Qt::AlignLeft);
	cheats_layout->addWidget(cheats_label, 1, Qt::AlignLeft);
	cheats_layout->addWidget(edit_cheats, 0, Qt::AlignRight);
	cheats_set->setLayout(cheats_layout);

	//General settings - RTC offsets
	QWidget* rtc_set = new QWidget(general);
	QLabel* rtc_label = new QLabel(" ", rtc_set);
	QPushButton* edit_rtc = new QPushButton("Edit RTC Offsets");
	edit_rtc->setToolTip("Adjusts the emulated real-time clock by adding specific offset values.");

	QHBoxLayout* rtc_layout = new QHBoxLayout;
	rtc_layout->addWidget(edit_rtc, 0, Qt::AlignLeft);
	rtc_layout->addWidget(rtc_label, 1, Qt::AlignRight);
	rtc_set->setLayout(rtc_layout);

	//General settings - Emulated SIO device
	QWidget* sio_set = new QWidget(general);
	QLabel* sio_label = new QLabel("Serial IO Device", sio_set);
	sio_dev = new QComboBox(sio_set);
	sio_dev->setToolTip("Changes the emulated Serial Input-Output device connected to the emulated Game Boy");
	sio_dev->addItem("None");
	sio_dev->addItem("GB Link Cable");
	sio_dev->addItem("GB Printer");
	sio_dev->addItem("GB Mobile Adapter");
	sio_dev->addItem("Bardigun Barcode Scanner");
	sio_dev->addItem("Barcode Boy");
	sio_dev->addItem("DMG-07");
	sio_dev->addItem("GBA Link Cable");
	sio_dev->addItem("GB Player Rumble");
	sio_dev->addItem("Soul Doll Adapter");
	sio_dev->addItem("Battle Chip Gate");
	sio_dev->addItem("Progress Chip Gate");
	sio_dev->addItem("Beast Link Gate");
	sio_dev->addItem("Power Antenna");
	sio_dev->addItem("Singer IZEK 1500");
	sio_dev->addItem("Multi Plust On System");

	config_sio = new QPushButton("Configure");

	QHBoxLayout* sio_layout = new QHBoxLayout;
	sio_layout->addWidget(sio_label, 0, Qt::AlignLeft);
	sio_layout->addWidget(sio_dev, 1, Qt::AlignLeft);
	sio_layout->addWidget(config_sio, 0, Qt::AlignRight);
	sio_set->setLayout(sio_layout);

	//General settings - Emulated IR device
	QWidget* ir_set = new QWidget(general);
	QLabel* ir_label = new QLabel("Infrared Device", ir_set);
	ir_dev = new QComboBox(ir_set);
	ir_dev->setToolTip("Changes the emulated IR device that will communicate with the emulated Game Boy");
	ir_dev->addItem("GBC IR Port");
	ir_dev->addItem("Full Changer");
	ir_dev->addItem("Pokemon Pikachu 2");
	ir_dev->addItem("Pocket Sakura");
	ir_dev->addItem("TV Remote");
	ir_dev->addItem("Constant IR Light");

	config_ir = new QPushButton("Configure");

	QHBoxLayout* ir_layout = new QHBoxLayout;
	ir_layout->addWidget(ir_label, 0, Qt::AlignLeft);
	ir_layout->addWidget(ir_dev, 1, Qt::AlignLeft);
	ir_layout->addWidget(config_ir, 0, Qt::AlignRight);
	ir_set->setLayout(ir_layout);

	//General settings - Emulated CPU Speed
	QWidget* overclock_set = new QWidget(general);
	QLabel* overclock_label = new QLabel("Emulated CPU Speed", overclock_set);
	overclock = new QComboBox(overclock_set);
	overclock->setToolTip("Changes the emulated CPU speed. More demanding, but reduces lag that happens on real hardware. May break some games.");
	overclock->addItem("1x");
	overclock->addItem("2x");
	overclock->addItem("4x");
	overclock->addItem("8x");

	QHBoxLayout* overclock_layout = new QHBoxLayout;
	overclock_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	overclock_layout->addWidget(overclock_label);
	overclock_layout->addWidget(overclock);
	overclock_set->setLayout(overclock_layout);

	//General settings - Enable patches
	QWidget* patch_set = new QWidget(general);
	QLabel* patch_label = new QLabel("Enable ROM patches", patch_set);
	auto_patch = new QCheckBox(patch_set);
	auto_patch->setToolTip("Enables automatically patching a ROM when booting");

	QHBoxLayout* patch_layout = new QHBoxLayout;
	patch_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	patch_layout->addWidget(auto_patch);
	patch_layout->addWidget(patch_label);
	patch_set->setLayout(patch_layout);

	QVBoxLayout* gen_layout = new QVBoxLayout;
	gen_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	gen_layout->addWidget(sys_type_set);
	gen_layout->addWidget(sio_set);
	gen_layout->addWidget(ir_set);
	gen_layout->addWidget(special_cart_set);
	gen_layout->addWidget(overclock_set);
	gen_layout->addWidget(bios_set);
	gen_layout->addWidget(firmware_set);
	gen_layout->addWidget(cheats_set);
	gen_layout->addWidget(patch_set);
	gen_layout->addWidget(rtc_set);
	general->setLayout(gen_layout);

	//Display settings - Screen scale
	QWidget* screen_scale_set = new QWidget(display);
	QLabel* screen_scale_label = new QLabel("Screen Scale : ");
	screen_scale = new QComboBox(screen_scale_set);
	screen_scale->setToolTip("Scaling factor for the system's original screen size");
	screen_scale->addItem("1x");
	screen_scale->addItem("2x");
	screen_scale->addItem("3x");
	screen_scale->addItem("4x");
	screen_scale->addItem("5x");
	screen_scale->addItem("6x");
	screen_scale->addItem("7x");
	screen_scale->addItem("8x");
	screen_scale->addItem("9x");
	screen_scale->addItem("10x");

	QHBoxLayout* screen_scale_layout = new QHBoxLayout;
	screen_scale_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	screen_scale_layout->addWidget(screen_scale_label);
	screen_scale_layout->addWidget(screen_scale);
	screen_scale_set->setLayout(screen_scale_layout);

	//Display settings - CGFX scale
	QWidget* cgfx_scale_set = new QWidget(display);
	QLabel* cgfx_scale_label = new QLabel("Custom Graphics (CGFX) Scale : ");
	cgfx_scale = new QComboBox(cgfx_scale_set);
	cgfx_scale->setToolTip("Scaling factor for all custom graphics.\nOnly applies when CGFX are loaded.");
	cgfx_scale->addItem("1x");
	cgfx_scale->addItem("2x");
	cgfx_scale->addItem("3x");
	cgfx_scale->addItem("4x");
	cgfx_scale->addItem("5x");
	cgfx_scale->addItem("6x");
	cgfx_scale->addItem("7x");
	cgfx_scale->addItem("8x");
	cgfx_scale->addItem("9x");
	cgfx_scale->addItem("10x");

	QHBoxLayout* cgfx_scale_layout = new QHBoxLayout;
	cgfx_scale_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	cgfx_scale_layout->addWidget(cgfx_scale_label);
	cgfx_scale_layout->addWidget(cgfx_scale);
	cgfx_scale_set->setLayout(cgfx_scale_layout);

	//Display settings - DMG on GBC palette
	QWidget* dmg_gbc_pal_set = new QWidget(display);
	QLabel* dmg_gbc_pal_label = new QLabel("DMG Color Palette : ");
	dmg_gbc_pal = new QComboBox(dmg_gbc_pal_set);
	dmg_gbc_pal->setToolTip("Selects the original Game Boy palette");
	dmg_gbc_pal->addItem("OFF");
	dmg_gbc_pal->addItem("GBC BOOTROM - NO INPUT");
	dmg_gbc_pal->addItem("GBC BOOTROM - UP");
	dmg_gbc_pal->addItem("GBC BOOTROM - DOWN");
	dmg_gbc_pal->addItem("GBC BOOTROM - LEFT");
	dmg_gbc_pal->addItem("GBC BOOTROM - RIGHT");
	dmg_gbc_pal->addItem("GBC BOOTROM - UP+A");
	dmg_gbc_pal->addItem("GBC BOOTROM - DOWN+A");
	dmg_gbc_pal->addItem("GBC BOOTROM - LEFT+A");
	dmg_gbc_pal->addItem("GBC BOOTROM - RIGHT+A");
	dmg_gbc_pal->addItem("GBC BOOTROM - UP+B");
	dmg_gbc_pal->addItem("GBC BOOTROM - DOWN+B");
	dmg_gbc_pal->addItem("GBC BOOTROM - LEFT+B");
	dmg_gbc_pal->addItem("GBC BOOTROM - RIGHT+B");
	dmg_gbc_pal->addItem("DMG - Classic Green");
	dmg_gbc_pal->addItem("DMG - Game Boy Light");

	QHBoxLayout* dmg_gbc_pal_layout = new QHBoxLayout;
	dmg_gbc_pal_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	dmg_gbc_pal_layout->addWidget(dmg_gbc_pal_label);
	dmg_gbc_pal_layout->addWidget(dmg_gbc_pal);
	dmg_gbc_pal_set->setLayout(dmg_gbc_pal_layout);

	//Display settings - OpenGL Fragment Shader
	QWidget* ogl_frag_shader_set = new QWidget(display);
	QLabel* ogl_frag_shader_label = new QLabel("Post-Processing Shader : ");
	ogl_frag_shader = new QComboBox(ogl_frag_shader_set);
	ogl_frag_shader->setToolTip("Applies an OpenGL GLSL post-processing shader for graphical effects.");
	ogl_frag_shader->addItem("OFF");
	ogl_frag_shader->addItem("2xBR");
	ogl_frag_shader->addItem("4xBR");
	ogl_frag_shader->addItem("Bad Bloom");
	ogl_frag_shader->addItem("Badder Bloom");
	ogl_frag_shader->addItem("Chrono");
	ogl_frag_shader->addItem("DMG Mode");
	ogl_frag_shader->addItem("GBA Gamma");
	ogl_frag_shader->addItem("GBC Gamma");
	ogl_frag_shader->addItem("Grayscale");
	ogl_frag_shader->addItem("LCD Mode");
	ogl_frag_shader->addItem("Pastel");
	ogl_frag_shader->addItem("Scale2x");
	ogl_frag_shader->addItem("Scale3x");
	ogl_frag_shader->addItem("Sepia");
	ogl_frag_shader->addItem("Spotlight");
	ogl_frag_shader->addItem("TV Mode");
	ogl_frag_shader->addItem("Washout");

	QHBoxLayout* ogl_frag_shader_layout = new QHBoxLayout;
	ogl_frag_shader_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	ogl_frag_shader_layout->addWidget(ogl_frag_shader_label);
	ogl_frag_shader_layout->addWidget(ogl_frag_shader);
	ogl_frag_shader_set->setLayout(ogl_frag_shader_layout);

	//Display settings - OpenGL Vertex Shader
	QWidget* ogl_vert_shader_set = new QWidget(display);
	QLabel* ogl_vert_shader_label = new QLabel("Vertex Shader : ");
	ogl_vert_shader = new QComboBox(ogl_vert_shader_set);
	ogl_vert_shader->setToolTip("Applies an OpenGL GLSL vertex shader to change screen positions");
	ogl_vert_shader->addItem("Default");
	ogl_vert_shader->addItem("X Inverse");

	QHBoxLayout* ogl_vert_shader_layout = new QHBoxLayout;
	ogl_vert_shader_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	ogl_vert_shader_layout->addWidget(ogl_vert_shader_label);
	ogl_vert_shader_layout->addWidget(ogl_vert_shader);
	ogl_vert_shader_set->setLayout(ogl_vert_shader_layout);

	//Display settings - Use OpenGL
	QWidget* ogl_set = new QWidget(display);
	QLabel* ogl_label = new QLabel("Use OpenGL");
	ogl = new QCheckBox(ogl_set);
	ogl->setToolTip("Draw the screen using OpenGL.\n Allows for faster drawing operations and shader support.");

	QHBoxLayout* ogl_layout = new QHBoxLayout;
	ogl_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	ogl_layout->addWidget(ogl);
	ogl_layout->addWidget(ogl_label);
	ogl_set->setLayout(ogl_layout);

	//Display settings - Use CGFX
	QWidget* load_cgfx_set = new QWidget(display);
	QLabel* load_cgfx_label = new QLabel("Load Custom Graphics (CGFX)");
	load_cgfx = new QCheckBox(load_cgfx_set);
	load_cgfx->setToolTip("Enables custom graphics and loads them when booting a game");

	QHBoxLayout* load_cgfx_layout = new QHBoxLayout;
	load_cgfx_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	load_cgfx_layout->addWidget(load_cgfx);
	load_cgfx_layout->addWidget(load_cgfx_label);
	load_cgfx_set->setLayout(load_cgfx_layout);

	//Display settings - Aspect Ratio
	QWidget* aspect_set = new QWidget(display);
	QLabel* aspect_label = new QLabel("Maintain Aspect Ratio");
	aspect_ratio = new QCheckBox(aspect_set);
	aspect_ratio->setToolTip("Forces GBE+ to maintain the original system's aspect ratio");

	QHBoxLayout* aspect_layout = new QHBoxLayout;
	aspect_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	aspect_layout->addWidget(aspect_ratio);
	aspect_layout->addWidget(aspect_label);
	aspect_set->setLayout(aspect_layout);

	//Display settings - On-Screen Display
	QWidget* osd_set = new QWidget(display);
	QLabel* osd_label = new QLabel("Enable On-Screen Display Messages");
	osd_enable = new QCheckBox(osd_set);
	osd_enable->setToolTip("Displays messages from the emulator on-screen");

	QHBoxLayout* osd_layout = new QHBoxLayout;
	osd_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	osd_layout->addWidget(osd_enable);
	osd_layout->addWidget(osd_label);
	osd_set->setLayout(osd_layout);

	QVBoxLayout* disp_layout = new QVBoxLayout;
	disp_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	disp_layout->addWidget(screen_scale_set);
	disp_layout->addWidget(cgfx_scale_set);
	disp_layout->addWidget(dmg_gbc_pal_set);
	disp_layout->addWidget(ogl_frag_shader_set);
	disp_layout->addWidget(ogl_vert_shader_set);
	disp_layout->addWidget(ogl_set);
	disp_layout->addWidget(load_cgfx_set);
	disp_layout->addWidget(aspect_set);
	disp_layout->addWidget(osd_set);
	display->setLayout(disp_layout);

	//Sound settings - Output frequency
	QWidget* freq_set = new QWidget(sound);
	QLabel* freq_label = new QLabel("Output Frequency : ");
	freq = new QComboBox(freq_set);
	freq->setToolTip("Selects the final output frequency of all sound.");
	freq->addItem("48000Hz");
	freq->addItem("44100Hz");
	freq->addItem("22050Hz");
	freq->addItem("11025Hz");
	freq->setCurrentIndex(1);

	QHBoxLayout* freq_layout = new QHBoxLayout;
	freq_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	freq_layout->addWidget(freq_label);
	freq_layout->addWidget(freq);
	freq_set->setLayout(freq_layout);

	//Sound settings - Sound on/off
	QWidget* sound_on_set = new QWidget(sound);
	QLabel* sound_on_label = new QLabel("Enable Sound");
	sound_on = new QCheckBox(sound_on_set);
	sound_on->setToolTip("Enables all sounds output.");
	sound_on->setChecked(true);

	QHBoxLayout* sound_on_layout = new QHBoxLayout;
	sound_on_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	sound_on_layout->addWidget(sound_on);
	sound_on_layout->addWidget(sound_on_label);
	sound_on_set->setLayout(sound_on_layout);

	//Sound settings - Enable stereo sound
	QWidget* stereo_enable_set = new QWidget(sound);
	QLabel* stereo_enable_label = new QLabel("Enable Stereo Output");
	stereo_enable = new QCheckBox(stereo_enable_set);
	stereo_enable->setToolTip("Enables stereo sound output.");
	stereo_enable->setChecked(true);

	QHBoxLayout* stereo_enable_layout = new QHBoxLayout;
	stereo_enable_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	stereo_enable_layout->addWidget(stereo_enable);
	stereo_enable_layout->addWidget(stereo_enable_label);
	stereo_enable_set->setLayout(stereo_enable_layout);

	//Sound settings - Volume
	QWidget* volume_set = new QWidget(sound);
	QLabel* volume_label = new QLabel("Volume : ");
	volume = new QSlider(sound);
	volume->setToolTip("Master volume for GBE+");
	volume->setMaximum(128);
	volume->setMinimum(0);
	volume->setValue(128);
	volume->setOrientation(Qt::Horizontal);

	QHBoxLayout* volume_layout = new QHBoxLayout;
	volume_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	volume_layout->addWidget(volume_label);
	volume_layout->addWidget(volume);
	volume_set->setLayout(volume_layout);

	QVBoxLayout* audio_layout = new QVBoxLayout;
	audio_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	audio_layout->addWidget(freq_set);
	audio_layout->addWidget(sound_on_set);
	audio_layout->addWidget(stereo_enable_set);
	audio_layout->addWidget(volume_set);
	sound->setLayout(audio_layout);

	//Control settings - Device
	input_device_set = new QWidget(controls);
	QLabel* input_device_label = new QLabel("Input Device : ");
	input_device = new QComboBox(input_device_set);
	input_device->setToolTip("Selects the current input device to configure");
	input_device->addItem("Keyboard");
	input_device->installEventFilter(this);

	QHBoxLayout* input_device_layout = new QHBoxLayout;
	input_device_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	input_device_layout->addWidget(input_device_label);
	input_device_layout->addWidget(input_device);
	input_device_set->setLayout(input_device_layout);

	//Control settings- Dead-zone
	dead_zone_set = new QWidget(controls);
	QLabel* dead_zone_label = new QLabel("Dead Zone : ");
	dead_zone = new QSlider(controls);
	dead_zone->setToolTip("Sets the dead-zone for joystick axes.");
	dead_zone->setMaximum(32767);
	dead_zone->setMinimum(0);
	dead_zone->setValue(16000);
	dead_zone->setOrientation(Qt::Horizontal);

	QHBoxLayout* dead_zone_layout = new QHBoxLayout;
	dead_zone_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	dead_zone_layout->addWidget(dead_zone_label);
	dead_zone_layout->addWidget(dead_zone);
	dead_zone_layout->setContentsMargins(6, 0, 0, 0);
	dead_zone_set->setLayout(dead_zone_layout);

	//Control settings - A button
	input_a_set = new QWidget(controls);
	QLabel* input_a_label = new QLabel("Button A : ");
	input_a = new QLineEdit(controls);
	config_a = new QPushButton("Configure");
	input_a->setMaximumWidth(100);
	config_a->setMaximumWidth(100);

	QHBoxLayout* input_a_layout = new QHBoxLayout;
	input_a_layout->addWidget(input_a_label, 0, Qt::AlignLeft);
	input_a_layout->addWidget(input_a, 0, Qt::AlignLeft);
	input_a_layout->addWidget(config_a, 0, Qt::AlignLeft);
	input_a_layout->setContentsMargins(6, 0, 0, 0);
	input_a_set->setLayout(input_a_layout);

	//Control settings - B button
	input_b_set = new QWidget(controls);
	QLabel* input_b_label = new QLabel("Button B : ");
	input_b = new QLineEdit(controls);
	config_b = new QPushButton("Configure");
	input_b->setMaximumWidth(100);
	config_b->setMaximumWidth(100);

	QHBoxLayout* input_b_layout = new QHBoxLayout;
	input_b_layout->addWidget(input_b_label, 0, Qt::AlignLeft);
	input_b_layout->addWidget(input_b, 0, Qt::AlignLeft);
	input_b_layout->addWidget(config_b, 0, Qt::AlignLeft);
	input_b_layout->setContentsMargins(6, 0, 0, 0);
	input_b_set->setLayout(input_b_layout);

	//Control settings - X button
	input_x_set = new QWidget(controls);
	QLabel* input_x_label = new QLabel("Button X : ");
	input_x = new QLineEdit(controls);
	config_x = new QPushButton("Configure");
	input_x->setMaximumWidth(100);
	config_x->setMaximumWidth(100);

	QHBoxLayout* input_x_layout = new QHBoxLayout;
	input_x_layout->addWidget(input_x_label, 0, Qt::AlignLeft);
	input_x_layout->addWidget(input_x, 0, Qt::AlignLeft);
	input_x_layout->addWidget(config_x, 0, Qt::AlignLeft);
	input_x_layout->setContentsMargins(6, 0, 0, 0);
	input_x_set->setLayout(input_x_layout);

	//Control settings - Y button
	input_y_set = new QWidget(controls);
	QLabel* input_y_label = new QLabel("Button Y : ");
	input_y = new QLineEdit(controls);
	config_y = new QPushButton("Configure");
	input_y->setMaximumWidth(100);
	config_y->setMaximumWidth(100);

	QHBoxLayout* input_y_layout = new QHBoxLayout;
	input_y_layout->addWidget(input_y_label, 0, Qt::AlignLeft);
	input_y_layout->addWidget(input_y, 0, Qt::AlignLeft);
	input_y_layout->addWidget(config_y, 0, Qt::AlignLeft);
	input_y_layout->setContentsMargins(6, 0, 0, 0);
	input_y_set->setLayout(input_y_layout);

	//Control settings - START button
	input_start_set = new QWidget(controls);
	QLabel* input_start_label = new QLabel("START : ");
	input_start = new QLineEdit(controls);
	config_start = new QPushButton("Configure");
	input_start->setMaximumWidth(100);
	config_start->setMaximumWidth(100);

	QHBoxLayout* input_start_layout = new QHBoxLayout;
	input_start_layout->addWidget(input_start_label, 0, Qt::AlignLeft);
	input_start_layout->addWidget(input_start, 0, Qt::AlignLeft);
	input_start_layout->addWidget(config_start, 0, Qt::AlignLeft);
	input_start_layout->setContentsMargins(6, 0, 0, 0);
	input_start_set->setLayout(input_start_layout);

	//Control settings - SELECT button
	input_select_set = new QWidget(controls);
	QLabel* input_select_label = new QLabel("SELECT : ");
	input_select = new QLineEdit(controls);
	config_select = new QPushButton("Configure");
	input_select->setMaximumWidth(100);
	config_select->setMaximumWidth(100);

	QHBoxLayout* input_select_layout = new QHBoxLayout;
	input_select_layout->addWidget(input_select_label, 0, Qt::AlignLeft);
	input_select_layout->addWidget(input_select, 0, Qt::AlignLeft);
	input_select_layout->addWidget(config_select, 0, Qt::AlignLeft);
	input_select_layout->setContentsMargins(6, 0, 0, 0);
	input_select_set->setLayout(input_select_layout);

	//Control settings - Left
	input_left_set = new QWidget(controls);
	QLabel* input_left_label = new QLabel("LEFT : ");
	input_left = new QLineEdit(controls);
	config_left = new QPushButton("Configure");
	input_left->setMaximumWidth(100);
	config_left->setMaximumWidth(100);

	QHBoxLayout* input_left_layout = new QHBoxLayout;
	input_left_layout->addWidget(input_left_label, 0, Qt::AlignLeft);
	input_left_layout->addWidget(input_left, 0, Qt::AlignLeft);
	input_left_layout->addWidget(config_left, 0, Qt::AlignLeft);
	input_left_layout->setContentsMargins(6, 0, 0, 0);
	input_left_set->setLayout(input_left_layout);

	//Control settings - Right
	input_right_set = new QWidget(controls);
	QLabel* input_right_label = new QLabel("RIGHT : ");
	input_right = new QLineEdit(controls);
	config_right = new QPushButton("Configure");
	input_right->setMaximumWidth(100);
	config_right->setMaximumWidth(100);

	QHBoxLayout* input_right_layout = new QHBoxLayout;
	input_right_layout->addWidget(input_right_label, 0, Qt::AlignLeft);
	input_right_layout->addWidget(input_right, 0, Qt::AlignLeft);
	input_right_layout->addWidget(config_right, 0, Qt::AlignLeft);
	input_right_layout->setContentsMargins(6, 0, 0, 0);
	input_right_set->setLayout(input_right_layout);

	//Control settings - Up
	input_up_set = new QWidget(controls);
	QLabel* input_up_label = new QLabel("UP : ");
	input_up = new QLineEdit(controls);
	config_up = new QPushButton("Configure");
	input_up->setMaximumWidth(100);
	config_up->setMaximumWidth(100);

	QHBoxLayout* input_up_layout = new QHBoxLayout;
	input_up_layout->addWidget(input_up_label, 0, Qt::AlignLeft);
	input_up_layout->addWidget(input_up, 0, Qt::AlignLeft);
	input_up_layout->addWidget(config_up, 0, Qt::AlignLeft);
	input_up_layout->setContentsMargins(6, 0, 0, 0);
	input_up_set->setLayout(input_up_layout);

	//Control settings - Down
	input_down_set = new QWidget(controls);
	QLabel* input_down_label = new QLabel("DOWN : ");
	input_down = new QLineEdit(controls);
	config_down = new QPushButton("Configure");
	input_down->setMaximumWidth(100);
	config_down->setMaximumWidth(100);

	QHBoxLayout* input_down_layout = new QHBoxLayout;
	input_down_layout->addWidget(input_down_label, 0, Qt::AlignLeft);
	input_down_layout->addWidget(input_down, 0, Qt::AlignLeft);
	input_down_layout->addWidget(config_down, 0, Qt::AlignLeft);
	input_down_layout->setContentsMargins(6, 0, 0, 0);
	input_down_set->setLayout(input_down_layout);

	//Control settings - Right Trigger
	input_r_set = new QWidget(controls);
	QLabel* input_r_label = new QLabel("Trigger R : ");
	input_r = new QLineEdit(controls);
	config_r = new QPushButton("Configure");
	input_r->setMaximumWidth(100);
	config_r->setMaximumWidth(100);

	QHBoxLayout* input_r_layout = new QHBoxLayout;
	input_r_layout->addWidget(input_r_label, 0, Qt::AlignLeft);
	input_r_layout->addWidget(input_r, 0, Qt::AlignLeft);
	input_r_layout->addWidget(config_r, 0, Qt::AlignLeft);
	input_r_layout->setContentsMargins(6, 0, 0, 0);
	input_r_set->setLayout(input_r_layout);

	//Control settings - Left Trigger
	input_l_set = new QWidget(controls);
	QLabel* input_l_label = new QLabel("Trigger L : ");
	input_l = new QLineEdit(controls);
	config_l = new QPushButton("Configure");
	input_l->setMaximumWidth(100);
	config_l->setMaximumWidth(100);

	QHBoxLayout* input_l_layout = new QHBoxLayout;
	input_l_layout->addWidget(input_l_label, 0, Qt::AlignLeft);
	input_l_layout->addWidget(input_l, 0, Qt::AlignLeft);
	input_l_layout->addWidget(config_l, 0, Qt::AlignLeft);
	input_l_layout->setContentsMargins(6, 0, 0, 0);
	input_l_set->setLayout(input_l_layout);

	//Advanced control settings - Enable rumble
	rumble_set = new QWidget(controls);
	QLabel* rumble_label = new QLabel("Enable rumble", rumble_set);
	rumble_on = new QCheckBox(rumble_set);

	QHBoxLayout* rumble_layout = new QHBoxLayout;
	rumble_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	rumble_layout->addWidget(rumble_on);
	rumble_layout->addWidget(rumble_label);
	rumble_set->setLayout(rumble_layout);

	//Advanced control settings - Context left
	con_left_set = new QWidget(controls);
	QLabel* con_left_label = new QLabel("Context Left : ");
	input_con_left = new QLineEdit(controls);
	config_con_left = new QPushButton("Configure");
	input_con_left->setMaximumWidth(100);
	config_con_left->setMaximumWidth(100);

	QHBoxLayout* con_left_layout = new QHBoxLayout;
	con_left_layout->addWidget(con_left_label, 1, Qt::AlignLeft);
	con_left_layout->addWidget(input_con_left, 1, Qt::AlignLeft);
	con_left_layout->addWidget(config_con_left, 1, Qt::AlignLeft);
	con_left_layout->setContentsMargins(6, 0, 0, 0);
	con_left_set->setLayout(con_left_layout);

	//Advanced control settings - Context right
	con_right_set = new QWidget(controls);
	QLabel* con_right_label = new QLabel("Context Right : ");
	input_con_right = new QLineEdit(controls);
	config_con_right = new QPushButton("Configure");
	input_con_right->setMaximumWidth(100);
	config_con_right->setMaximumWidth(100);

	QHBoxLayout* con_right_layout = new QHBoxLayout;
	con_right_layout->addWidget(con_right_label, 1, Qt::AlignLeft);
	con_right_layout->addWidget(input_con_right, 1, Qt::AlignLeft);
	con_right_layout->addWidget(config_con_right, 1, Qt::AlignLeft);
	con_right_layout->setContentsMargins(6, 0, 0, 0);
	con_right_set->setLayout(con_right_layout);

	//Advanced control settings - Context up
	con_up_set = new QWidget(controls);
	QLabel* con_up_label = new QLabel("Context Up : ");
	input_con_up = new QLineEdit(controls);
	config_con_up = new QPushButton("Configure");
	input_con_up->setMaximumWidth(100);
	config_con_up->setMaximumWidth(100);

	QHBoxLayout* con_up_layout = new QHBoxLayout;
	con_up_layout->addWidget(con_up_label, 1, Qt::AlignLeft);
	con_up_layout->addWidget(input_con_up, 1, Qt::AlignLeft);
	con_up_layout->addWidget(config_con_up, 1, Qt::AlignLeft);
	con_up_layout->setContentsMargins(6, 0, 0, 0);
	con_up_set->setLayout(con_up_layout);

	//Advanced control settings - Context down
	con_down_set = new QWidget(controls);
	QLabel* con_down_label = new QLabel("Context Down : ");
	input_con_down = new QLineEdit(controls);
	config_con_down = new QPushButton("Configure");
	input_con_down->setMaximumWidth(100);
	config_con_down->setMaximumWidth(100);

	QHBoxLayout* con_down_layout = new QHBoxLayout;
	con_down_layout->addWidget(con_down_label, 1, Qt::AlignLeft);
	con_down_layout->addWidget(input_con_down, 1, Qt::AlignLeft);
	con_down_layout->addWidget(config_con_down, 1, Qt::AlignLeft);
	con_down_layout->setContentsMargins(6, 0, 0, 0);
	con_down_set->setLayout(con_down_layout);

	//Advanced control settings - Context 1
	con_1_set = new QWidget(controls);
	QLabel* con_1_label = new QLabel("Context 1 : ");
	input_con_1 = new QLineEdit(controls);
	config_con_1 = new QPushButton("Configure");
	input_con_1->setMaximumWidth(100);
	config_con_1->setMaximumWidth(100);

	QHBoxLayout* con_1_layout = new QHBoxLayout;
	con_1_layout->addWidget(con_1_label, 1, Qt::AlignLeft);
	con_1_layout->addWidget(input_con_1, 1, Qt::AlignLeft);
	con_1_layout->addWidget(config_con_1, 1, Qt::AlignLeft);
	con_1_layout->setContentsMargins(6, 0, 0, 0);
	con_1_set->setLayout(con_1_layout);

	//Advanced control settings - Context 2
	con_2_set = new QWidget(controls);
	QLabel* con_2_label = new QLabel("Context 2 : ");
	input_con_2 = new QLineEdit(controls);
	config_con_2 = new QPushButton("Configure");
	input_con_2->setMaximumWidth(100);
	config_con_2->setMaximumWidth(100);

	QHBoxLayout* con_2_layout = new QHBoxLayout;
	con_2_layout->addWidget(con_2_label, 1, Qt::AlignLeft);
	con_2_layout->addWidget(input_con_2, 1, Qt::AlignLeft);
	con_2_layout->addWidget(config_con_2, 1, Qt::AlignLeft);
	con_2_layout->setContentsMargins(6, 0, 0, 0);
	con_2_set->setLayout(con_2_layout);

	//Hotkey settings - Turbo
	hotkey_turbo_set = new QWidget(controls);
	QLabel* hotkey_turbo_label = new QLabel("Toggle Turbo : ");
	input_turbo = new QLineEdit(controls);
	config_turbo = new QPushButton("Configure");
	input_turbo->setMaximumWidth(100);
	config_turbo->setMaximumWidth(100);

	QHBoxLayout* hotkey_turbo_layout = new QHBoxLayout;
	hotkey_turbo_layout->addWidget(hotkey_turbo_label, 1, Qt::AlignLeft);
	hotkey_turbo_layout->addWidget(input_turbo, 1, Qt::AlignLeft);
	hotkey_turbo_layout->addWidget(config_turbo, 1, Qt::AlignLeft);
	hotkey_turbo_layout->setContentsMargins(6, 0, 0, 0);
	hotkey_turbo_set->setLayout(hotkey_turbo_layout);

	//Hotkey settings - Mute
	hotkey_mute_set = new QWidget(controls);
	QLabel* hotkey_mute_label = new QLabel("Toggle Mute : ");
	input_mute = new QLineEdit(controls);
	config_mute = new QPushButton("Configure");
	input_mute->setMaximumWidth(100);
	config_mute->setMaximumWidth(100);

	QHBoxLayout* hotkey_mute_layout = new QHBoxLayout;
	hotkey_mute_layout->addWidget(hotkey_mute_label, 1, Qt::AlignLeft);
	hotkey_mute_layout->addWidget(input_mute, 1, Qt::AlignLeft);
	hotkey_mute_layout->addWidget(config_mute, 1, Qt::AlignLeft);
	hotkey_mute_layout->setContentsMargins(6, 0, 0, 0);
	hotkey_mute_set->setLayout(hotkey_mute_layout);

	//Hotkey settings - Camera
	hotkey_camera_set = new QWidget(controls);
	QLabel* hotkey_camera_label = new QLabel("GB Camera File : ");
	input_camera = new QLineEdit(controls);
	config_camera = new QPushButton("Configure");
	input_camera->setMaximumWidth(100);
	config_camera->setMaximumWidth(100);

	QHBoxLayout* hotkey_camera_layout = new QHBoxLayout;
	hotkey_camera_layout->addWidget(hotkey_camera_label, 1, Qt::AlignLeft);
	hotkey_camera_layout->addWidget(input_camera, 1, Qt::AlignLeft);
	hotkey_camera_layout->addWidget(config_camera, 1, Qt::AlignLeft);
	hotkey_camera_layout->setContentsMargins(6, 0, 0, 0);
	hotkey_camera_set->setLayout(hotkey_camera_layout);

	//Hotkey settings - NDS Screen Swap
	hotkey_swap_screen_set = new QWidget(controls);
	QLabel* hotkey_swap_screen_label = new QLabel("DS Swap LCD : ");
	input_swap_screen = new QLineEdit(controls);
	config_swap_screen = new QPushButton("Configure");
	input_swap_screen->setMaximumWidth(100);
	config_swap_screen->setMaximumWidth(100);

	QHBoxLayout* hotkey_swap_screen_layout = new QHBoxLayout;
	hotkey_swap_screen_layout->addWidget(hotkey_swap_screen_label, 1, Qt::AlignLeft);
	hotkey_swap_screen_layout->addWidget(input_swap_screen, 1, Qt::AlignLeft);
	hotkey_swap_screen_layout->addWidget(config_swap_screen, 1, Qt::AlignLeft);
	hotkey_swap_screen_layout->setContentsMargins(6, 0, 0, 0);
	hotkey_swap_screen_set->setLayout(hotkey_swap_screen_layout);

	//Hotkey settings - NDS Screen Shift
	hotkey_shift_screen_set = new QWidget(controls);
	QLabel* hotkey_shift_screen_label = new QLabel("DS V/H Mode : ");
	input_shift_screen = new QLineEdit(controls);
	config_shift_screen = new QPushButton("Configure");
	input_shift_screen->setMaximumWidth(100);
	config_shift_screen->setMaximumWidth(100);

	QHBoxLayout* hotkey_shift_screen_layout = new QHBoxLayout;
	hotkey_shift_screen_layout->addWidget(hotkey_shift_screen_label, 1, Qt::AlignLeft);
	hotkey_shift_screen_layout->addWidget(input_shift_screen, 1, Qt::AlignLeft);
	hotkey_shift_screen_layout->addWidget(config_shift_screen, 1, Qt::AlignLeft);
	hotkey_shift_screen_layout->setContentsMargins(6, 0, 0, 0);
	hotkey_shift_screen_set->setLayout(hotkey_shift_screen_layout);

	//Battle Chip Gate settings - Type
	bcg_gate_set = new QWidget(controls);
	QLabel* bcg_gate_label = new QLabel("Battle Chip Gate Type : ");
	chip_gate_type = new QComboBox(bcg_gate_set);
	chip_gate_type->setToolTip("Selects the list of available chips for each Battle Chip Gate");
	chip_gate_type->addItem("Battle Chip Gate");
	chip_gate_type->addItem("Progress Chip Gate");
	chip_gate_type->addItem("Beast Link Gate");

	QHBoxLayout* bcg_gate_layout = new QHBoxLayout;
	bcg_gate_layout->addWidget(bcg_gate_label, 1, Qt::AlignLeft);
	bcg_gate_layout->addWidget(chip_gate_type, 1, Qt::AlignLeft);
	bcg_gate_layout->setContentsMargins(6, 0, 0, 0);
	bcg_gate_set->setLayout(bcg_gate_layout);

	//Battle Chip Gate settings - Chip 1
	bcg_chip_1_set = new QWidget(controls);
	QLabel* bcg_chip_1_label = new QLabel("Battle Chip 1 : ");
	battle_chip_1 = new QComboBox(bcg_chip_1_set);
	battle_chip_1->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	QHBoxLayout* bcg_chip_1_layout = new QHBoxLayout;
	bcg_chip_1_layout->addWidget(bcg_chip_1_label, 1, Qt::AlignLeft);
	bcg_chip_1_layout->addWidget(battle_chip_1, 1, Qt::AlignLeft);
	bcg_chip_1_layout->setContentsMargins(6, 0, 0, 0);
	bcg_chip_1_set->setLayout(bcg_chip_1_layout);

	//Battle Chip Gate settings - Chip 2
	bcg_chip_2_set = new QWidget(controls);
	QLabel* bcg_chip_2_label = new QLabel("Battle Chip 2 : ");
	battle_chip_2 = new QComboBox(bcg_chip_2_set);
	battle_chip_2->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	QHBoxLayout* bcg_chip_2_layout = new QHBoxLayout;
	bcg_chip_2_layout->addWidget(bcg_chip_2_label, 1, Qt::AlignLeft);
	bcg_chip_2_layout->addWidget(battle_chip_2, 1, Qt::AlignLeft);
	bcg_chip_2_layout->setContentsMargins(6, 0, 0, 0);
	bcg_chip_2_set->setLayout(bcg_chip_2_layout);

	//Battle Chip Gate settings - Chip 3
	bcg_chip_3_set = new QWidget(controls);
	QLabel* bcg_chip_3_label = new QLabel("Battle Chip 3 : ");
	battle_chip_3 = new QComboBox(bcg_chip_3_set);
	battle_chip_3->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	QHBoxLayout* bcg_chip_3_layout = new QHBoxLayout;
	bcg_chip_3_layout->addWidget(bcg_chip_3_label, 1, Qt::AlignLeft);
	bcg_chip_3_layout->addWidget(battle_chip_3, 1, Qt::AlignLeft);
	bcg_chip_3_layout->setContentsMargins(6, 0, 0, 0);
	bcg_chip_3_set->setLayout(bcg_chip_3_layout);

	//Battle Chip Gate settings - Chip 4
	bcg_chip_4_set = new QWidget(controls);
	QLabel* bcg_chip_4_label = new QLabel("Battle Chip 4 : ");
	battle_chip_4 = new QComboBox(bcg_chip_4_set);
	battle_chip_4->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	QHBoxLayout* bcg_chip_4_layout = new QHBoxLayout;
	bcg_chip_4_layout->addWidget(bcg_chip_4_label, 1, Qt::AlignLeft);
	bcg_chip_4_layout->addWidget(battle_chip_4, 1, Qt::AlignLeft);
	bcg_chip_4_layout->setContentsMargins(6, 0, 0, 0);
	bcg_chip_4_set->setLayout(bcg_chip_4_layout);

	controls_layout = new QVBoxLayout;
	controls_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	controls_layout->addWidget(input_device_set);
	controls_layout->addWidget(input_a_set);
	controls_layout->addWidget(input_b_set);
	controls_layout->addWidget(input_x_set);
	controls_layout->addWidget(input_y_set);
	controls_layout->addWidget(input_start_set);
	controls_layout->addWidget(input_select_set);
	controls_layout->addWidget(input_left_set);
	controls_layout->addWidget(input_right_set);
	controls_layout->addWidget(input_up_set);
	controls_layout->addWidget(input_down_set);
	controls_layout->addWidget(input_l_set);
	controls_layout->addWidget(input_r_set);
	controls_layout->addWidget(dead_zone_set);
	controls->setLayout(controls_layout);

	advanced_controls_layout = new QVBoxLayout;
	advanced_controls_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	advanced_controls_layout->addWidget(rumble_set);
	advanced_controls_layout->addWidget(con_up_set);
	advanced_controls_layout->addWidget(con_down_set);
	advanced_controls_layout->addWidget(con_left_set);
	advanced_controls_layout->addWidget(con_right_set);
	advanced_controls_layout->addWidget(con_1_set);
	advanced_controls_layout->addWidget(con_2_set);

	hotkey_controls_layout = new QVBoxLayout;
	hotkey_controls_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	hotkey_controls_layout->addWidget(hotkey_turbo_set);
	hotkey_controls_layout->addWidget(hotkey_mute_set);
	hotkey_controls_layout->addWidget(hotkey_camera_set);
	hotkey_controls_layout->addWidget(hotkey_swap_screen_set);
	hotkey_controls_layout->addWidget(hotkey_shift_screen_set);
	
	bcg_controls_layout = new QVBoxLayout;
	bcg_controls_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	bcg_controls_layout->addWidget(bcg_gate_set);
	bcg_controls_layout->addWidget(bcg_chip_1_set);
	bcg_controls_layout->addWidget(bcg_chip_2_set);
	bcg_controls_layout->addWidget(bcg_chip_3_set);
	bcg_controls_layout->addWidget(bcg_chip_4_set);
	
	rumble_set->setVisible(false);
	con_up_set->setVisible(false);
	con_down_set->setVisible(false);
	con_left_set->setVisible(false);
	con_right_set->setVisible(false);
	con_1_set->setVisible(false);
	con_2_set->setVisible(false);

	hotkey_turbo_set->setVisible(false);
	hotkey_mute_set->setVisible(false);
	hotkey_camera_set->setVisible(false);
	hotkey_swap_screen_set->setVisible(false);
	hotkey_shift_screen_set->setVisible(false);

	bcg_gate_set->setVisible(false);
	bcg_chip_1_set->setVisible(false);
	bcg_chip_2_set->setVisible(false);
	bcg_chip_3_set->setVisible(false);
	bcg_chip_4_set->setVisible(false);

	//Netplay - Enable Netplay
	QWidget* enable_netplay_set = new QWidget(netplay);
	QLabel* enable_netplay_label = new QLabel("Enable netplay");
	enable_netplay = new QCheckBox(netplay);
	enable_netplay->setToolTip("Enables network communications for playing online.");

	QHBoxLayout* enable_netplay_layout = new QHBoxLayout;
	enable_netplay_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	enable_netplay_layout->addWidget(enable_netplay);
	enable_netplay_layout->addWidget(enable_netplay_label);
	enable_netplay_set->setLayout(enable_netplay_layout);

	//Netplay - Enable hard syncing
	QWidget* hard_sync_set = new QWidget(netplay);
	QLabel* hard_sync_label = new QLabel("Use hard syncing");
	hard_sync = new QCheckBox(netplay);
	hard_sync->setToolTip("Forces GBE+ to pause during netplay to wait for other players.\nCauses slowdowns but necessary to avoid desyncing in some cases.");

	QHBoxLayout* hard_sync_layout = new QHBoxLayout;
	hard_sync_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	hard_sync_layout->addWidget(hard_sync);
	hard_sync_layout->addWidget(hard_sync_label);
	hard_sync_set->setLayout(hard_sync_layout);

	//Netplay - Enable Net Gate
	QWidget* net_gate_set = new QWidget(netplay);
	QLabel* net_gate_label = new QLabel("Use Net Gate");
	net_gate = new QCheckBox(netplay);
	net_gate->setToolTip("Allows GBE+ to receive chip IDs for Battle Chip Gate via TCP");

	QHBoxLayout* net_gate_layout = new QHBoxLayout;
	net_gate_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	net_gate_layout->addWidget(net_gate);
	net_gate_layout->addWidget(net_gate_label);
	net_gate_set->setLayout(net_gate_layout);

	//Netplay - Sync Threshold
	QWidget* sync_threshold_set = new QWidget(netplay);
	QLabel* sync_threshold_label = new QLabel("Sync threshold");
	sync_threshold = new QSpinBox(netplay);
	sync_threshold->setMinimum(0);
	sync_threshold->setToolTip("Amount of emulated CPU cycles GBE+ will pause when enabling hard sync.");

	QHBoxLayout* sync_threshold_layout = new QHBoxLayout;
	sync_threshold_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	sync_threshold_layout->addWidget(sync_threshold_label);
	sync_threshold_layout->addWidget(sync_threshold);
	sync_threshold_set->setLayout(sync_threshold_layout);

	//Netplay - Server port
	QWidget* server_port_set = new QWidget(netplay);
	QLabel* server_port_label = new QLabel("Server Port : ");
	server_port = new QSpinBox(netplay);
	server_port->setMinimum(0);
	server_port->setMaximum(0xFFFF);
	server_port->setToolTip("GBE+ server port number.\nMust match the client port number of another instance of GBE+ when networking.");

	QHBoxLayout* server_port_layout = new QHBoxLayout;
	server_port_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	server_port_layout->addWidget(server_port_label);
	server_port_layout->addWidget(server_port);
	server_port_set->setLayout(server_port_layout);

	//Netplay - Client port
	QWidget* client_port_set = new QWidget(netplay);
	QLabel* client_port_label = new QLabel("Client Port : ");
	client_port = new QSpinBox(netplay);
	client_port->setMinimum(0);
	client_port->setMaximum(0xFFFF);
	client_port->setToolTip("GBE+ client port number.\nMust match the server port number of another instance of GBE+ when networking.");

	QHBoxLayout* client_port_layout = new QHBoxLayout;
	client_port_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	client_port_layout->addWidget(client_port_label);
	client_port_layout->addWidget(client_port);
	client_port_set->setLayout(client_port_layout);

	//Netplay - IP address
	QWidget* ip_address_set = new QWidget(netplay);
	QLabel* ip_address_label = new QLabel("Client IP Address : ");
	ip_address = new QLineEdit(netplay);
	ip_address->setToolTip("IP address GBE+ will establish a connection with.");
	ip_update = new QPushButton("Update IP");

	QHBoxLayout* ip_address_layout = new QHBoxLayout;
	ip_address_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	ip_address_layout->addWidget(ip_address_label);
	ip_address_layout->addWidget(ip_address);
	ip_address_layout->addWidget(ip_update);
	ip_address_set->setLayout(ip_address_layout);

	QVBoxLayout* netplay_layout = new QVBoxLayout;
	netplay_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	netplay_layout->addWidget(enable_netplay_set);
	netplay_layout->addWidget(net_gate_set);
	netplay_layout->addWidget(hard_sync_set);
	netplay_layout->addWidget(sync_threshold_set);
	netplay_layout->addWidget(server_port_set);
	netplay_layout->addWidget(client_port_set);
	netplay_layout->addWidget(ip_address_set);
	netplay->setLayout(netplay_layout);

	//Path settings - DMG BIOS
	QWidget* dmg_bios_set = new QWidget(paths);
	dmg_bios_label = new QLabel("DMG Boot ROM :  ");
	QPushButton* dmg_bios_button = new QPushButton("Browse");
	dmg_bios = new QLineEdit(paths);
	
	QHBoxLayout* dmg_bios_layout = new QHBoxLayout;
	dmg_bios_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	dmg_bios_layout->addWidget(dmg_bios_label);
	dmg_bios_layout->addWidget(dmg_bios);
	dmg_bios_layout->addWidget(dmg_bios_button);
	dmg_bios_set->setLayout(dmg_bios_layout);
	dmg_bios_label->resize(50, dmg_bios_label->height());

	//Path settings - GBC BIOS
	QWidget* gbc_bios_set = new QWidget(paths);
	gbc_bios_label = new QLabel("GBC Boot ROM :  ");
	QPushButton* gbc_bios_button = new QPushButton("Browse");
	gbc_bios = new QLineEdit(paths);

	QHBoxLayout* gbc_bios_layout = new QHBoxLayout;
	gbc_bios_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	gbc_bios_layout->addWidget(gbc_bios_label);
	gbc_bios_layout->addWidget(gbc_bios);
	gbc_bios_layout->addWidget(gbc_bios_button);
	gbc_bios_set->setLayout(gbc_bios_layout);

	//Path settings - GBA BIOS
	QWidget* gba_bios_set = new QWidget(paths);
	gba_bios_label = new QLabel("GBA BIOS :  ");
	QPushButton* gba_bios_button = new QPushButton("Browse");
	gba_bios = new QLineEdit(paths);

	QHBoxLayout* gba_bios_layout = new QHBoxLayout;
	gba_bios_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	gba_bios_layout->addWidget(gba_bios_label);
	gba_bios_layout->addWidget(gba_bios);
	gba_bios_layout->addWidget(gba_bios_button);
	gba_bios_set->setLayout(gba_bios_layout);

	//Path settings - System Firmware
	QWidget* nds_firmware_set = new QWidget(paths);
	nds_firmware_label = new QLabel("NDS Firmware :  ");
	QPushButton* nds_firmware_button = new QPushButton("Browse");
	nds_firmware = new QLineEdit(paths);

	QHBoxLayout* nds_firmware_layout = new QHBoxLayout;
	nds_firmware_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	nds_firmware_layout->addWidget(nds_firmware_label);
	nds_firmware_layout->addWidget(nds_firmware);
	nds_firmware_layout->addWidget(nds_firmware_button);
	nds_firmware_set->setLayout(nds_firmware_layout);

	//Path settings - CGFX Manifest
	QWidget* manifest_set = new QWidget(paths);
	manifest_label = new QLabel("CGFX Manifest :  ");
	QPushButton* manifest_button = new QPushButton("Browse");
	manifest = new QLineEdit(paths);

	QHBoxLayout* manifest_layout = new QHBoxLayout;
	manifest_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	manifest_layout->addWidget(manifest_label);
	manifest_layout->addWidget(manifest);
	manifest_layout->addWidget(manifest_button);
	manifest_set->setLayout(manifest_layout);

	//Path settings - CGFX BG Tile Dump Folder
	QWidget* dump_bg_set = new QWidget(paths);
	dump_bg_label = new QLabel("BG Dump :  ");
	QPushButton* dump_bg_button = new QPushButton("Browse");
	dump_bg = new QLineEdit(paths);

	QHBoxLayout* dump_bg_layout = new QHBoxLayout;
	dump_bg_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	dump_bg_layout->addWidget(dump_bg_label);
	dump_bg_layout->addWidget(dump_bg);
	dump_bg_layout->addWidget(dump_bg_button);
	dump_bg_set->setLayout(dump_bg_layout);
	dump_bg_label->resize(50, dump_bg_label->height());

	//Path settings - CGFX OBJ Tile Dump Folder
	QWidget* dump_obj_set = new QWidget(paths);
	dump_obj_label = new QLabel("OBJ Dump :  ");
	QPushButton* dump_obj_button = new QPushButton("Browse");
	dump_obj = new QLineEdit(paths);

	QHBoxLayout* dump_obj_layout = new QHBoxLayout;
	dump_obj_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	dump_obj_layout->addWidget(dump_obj_label);
	dump_obj_layout->addWidget(dump_obj);
	dump_obj_layout->addWidget(dump_obj_button);
	dump_obj_set->setLayout(dump_obj_layout);
	dump_obj_label->resize(50, dump_obj_label->height());

	//Path settings - Screenshot
	QWidget* screenshot_set = new QWidget(paths);
	screenshot_label = new QLabel("Screenshots :  ");
	QPushButton* screenshot_button = new QPushButton("Browse");
	screenshot = new QLineEdit(paths);

	QHBoxLayout* screenshot_layout = new QHBoxLayout;
	screenshot_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	screenshot_layout->addWidget(screenshot_label);
	screenshot_layout->addWidget(screenshot);
	screenshot_layout->addWidget(screenshot_button);
	screenshot_set->setLayout(screenshot_layout);

	//Path settings - Game saves
	QWidget* game_saves_set = new QWidget(paths);
	game_saves_label = new QLabel("Game Saves :  ");
	QPushButton* game_saves_button = new QPushButton("Browse");
	game_saves = new QLineEdit(paths);

	QHBoxLayout* game_saves_layout = new QHBoxLayout;
	game_saves_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	game_saves_layout->addWidget(game_saves_label);
	game_saves_layout->addWidget(game_saves);
	game_saves_layout->addWidget(game_saves_button);
	game_saves_set->setLayout(game_saves_layout);

	//Path settings - Cheats file
	QWidget* cheats_path_set = new QWidget(paths);
	cheats_path_label = new QLabel("Cheats File :  ");
	QPushButton* cheats_path_button = new QPushButton("Browse");
	cheats_path = new QLineEdit(paths);

	QHBoxLayout* cheats_path_layout = new QHBoxLayout;
	cheats_path_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	cheats_path_layout->addWidget(cheats_path_label);
	cheats_path_layout->addWidget(cheats_path);
	cheats_path_layout->addWidget(cheats_path_button);
	cheats_path_set->setLayout(cheats_path_layout);

	QVBoxLayout* paths_layout = new QVBoxLayout;
	paths_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	paths_layout->addWidget(dmg_bios_set);
	paths_layout->addWidget(gbc_bios_set);
	paths_layout->addWidget(gba_bios_set);
	paths_layout->addWidget(nds_firmware_set);
	paths_layout->addWidget(manifest_set);
	paths_layout->addWidget(dump_bg_set);
	paths_layout->addWidget(dump_obj_set);
	paths_layout->addWidget(screenshot_set);
	paths_layout->addWidget(game_saves_set);
	paths_layout->addWidget(cheats_path_set);
	paths->setLayout(paths_layout);

	//Setup warning message box
	warning_box = new QMessageBox;
	QPushButton* warning_box_ok = warning_box->addButton("OK", QMessageBox::AcceptRole);
	warning_box->setIcon(QMessageBox::Warning);
	warning_box->hide();

	data_folder = new data_dialog;

	connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(close_input()));
	connect(tabs_button, SIGNAL(accepted()), this, SLOT(accept()));
	connect(tabs_button, SIGNAL(rejected()), this, SLOT(reject()));
	connect(tabs_button->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close_settings()));
	connect(bios, SIGNAL(stateChanged(int)), this, SLOT(set_bios()));
	connect(firmware, SIGNAL(stateChanged(int)), this, SLOT(set_firmware()));
	connect(sio_dev, SIGNAL(currentIndexChanged(int)), this, SLOT(sio_dev_change()));
	connect(ir_dev, SIGNAL(currentIndexChanged(int)), this, SLOT(ir_dev_change()));
	connect(overclock, SIGNAL(currentIndexChanged(int)), this, SLOT(overclock_change()));
	connect(auto_patch, SIGNAL(stateChanged(int)), this, SLOT(set_patches()));
	connect(edit_cheats, SIGNAL(clicked()), this, SLOT(show_cheats()));
	connect(edit_rtc, SIGNAL(clicked()), this, SLOT(show_rtc()));
	connect(config_sio, SIGNAL(clicked()), this, SLOT(show_sio_config()));
	connect(config_ir, SIGNAL(clicked()), this, SLOT(show_ir_config()));
	connect(ogl, SIGNAL(stateChanged(int)), this, SLOT(set_ogl()));
	connect(screen_scale, SIGNAL(currentIndexChanged(int)), this, SLOT(screen_scale_change()));
	connect(aspect_ratio, SIGNAL(stateChanged(int)), this, SLOT(aspect_ratio_change()));
	connect(osd_enable, SIGNAL(stateChanged(int)), this, SLOT(set_osd()));
	connect(dmg_gbc_pal, SIGNAL(currentIndexChanged(int)), this, SLOT(dmg_gbc_pal_change()));
	connect(ogl_frag_shader, SIGNAL(currentIndexChanged(int)), this, SLOT(ogl_frag_change()));
	connect(ogl_vert_shader, SIGNAL(currentIndexChanged(int)), this, SLOT(ogl_vert_change()));
	connect(load_cgfx, SIGNAL(stateChanged(int)), this, SLOT(set_cgfx()));
	connect(volume, SIGNAL(valueChanged(int)), this, SLOT(volume_change()));
	connect(freq, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_rate_change()));
	connect(sound_on, SIGNAL(stateChanged(int)), this, SLOT(mute()));
	connect(dead_zone, SIGNAL(valueChanged(int)), this, SLOT(dead_zone_change()));
	connect(input_device, SIGNAL(currentIndexChanged(int)), this, SLOT(input_device_change()));
	connect(controls_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(switch_control_layout()));
	connect(enable_netplay, SIGNAL(stateChanged(int)), this, SLOT(set_netplay()));
	connect(hard_sync, SIGNAL(stateChanged(int)), this, SLOT(set_hard_sync()));
	connect(net_gate, SIGNAL(stateChanged(int)), this, SLOT(set_net_gate()));
	connect(chip_gate_type, SIGNAL(currentIndexChanged(int)), this, SLOT(get_chip_list()));
	connect(battle_chip_1, SIGNAL(currentIndexChanged(int)), this, SLOT(set_battle_chip()));
	connect(battle_chip_2, SIGNAL(currentIndexChanged(int)), this, SLOT(set_battle_chip()));
	connect(battle_chip_3, SIGNAL(currentIndexChanged(int)), this, SLOT(set_battle_chip()));
	connect(battle_chip_4, SIGNAL(currentIndexChanged(int)), this, SLOT(set_battle_chip()));
	connect(sync_threshold, SIGNAL(valueChanged(int)), this, SLOT(update_sync_threshold()));
	connect(server_port, SIGNAL(valueChanged(int)), this, SLOT(update_server_port()));
	connect(client_port, SIGNAL(valueChanged(int)), this, SLOT(update_client_port()));
	connect(ip_update, SIGNAL(clicked()), this, SLOT(update_ip_addr()));
	connect(data_folder, SIGNAL(accepted()), this, SLOT(select_folder()));
	connect(data_folder, SIGNAL(rejected()), this, SLOT(reject_folder()));

	QSignalMapper* paths_mapper = new QSignalMapper(this);
	connect(dmg_bios_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(gbc_bios_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(gba_bios_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(nds_firmware_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(manifest_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(dump_bg_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(dump_obj_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(screenshot_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(game_saves_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));
	connect(cheats_path_button, SIGNAL(clicked()), paths_mapper, SLOT(map()));

	paths_mapper->setMapping(dmg_bios_button, 0);
	paths_mapper->setMapping(gbc_bios_button, 1);
	paths_mapper->setMapping(gba_bios_button, 2);
	paths_mapper->setMapping(nds_firmware_button, 3);
	paths_mapper->setMapping(manifest_button, 4);
	paths_mapper->setMapping(screenshot_button, 5);
	paths_mapper->setMapping(dump_bg_button, 6);
	paths_mapper->setMapping(dump_obj_button, 7);
	paths_mapper->setMapping(game_saves_button, 8);
	paths_mapper->setMapping(cheats_path_button, 9);
	connect(paths_mapper, SIGNAL(mapped(int)), this, SLOT(set_paths(int)));

	QSignalMapper* button_config = new QSignalMapper(this);
	connect(config_a, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_b, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_x, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_y, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_start, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_select, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_left, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_right, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_up, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_down, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_l, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_r, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_con_up, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_con_down, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_con_left, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_con_right, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_con_1, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_con_2, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_turbo, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_mute, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_camera, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_swap_screen, SIGNAL(clicked()), button_config, SLOT(map()));
	connect(config_shift_screen, SIGNAL(clicked()), button_config, SLOT(map()));
	
	button_config->setMapping(config_a, 0);
	button_config->setMapping(config_b, 1);
	button_config->setMapping(config_x, 2);
	button_config->setMapping(config_y, 3);
	button_config->setMapping(config_start, 4);
	button_config->setMapping(config_select, 5);
	button_config->setMapping(config_left, 6);
	button_config->setMapping(config_right, 7);
	button_config->setMapping(config_up, 8);
	button_config->setMapping(config_down, 9);
	button_config->setMapping(config_l, 10);
	button_config->setMapping(config_r, 11);
	button_config->setMapping(config_con_up, 12);
	button_config->setMapping(config_con_down, 13);
	button_config->setMapping(config_con_left, 14);
	button_config->setMapping(config_con_right, 15);
	button_config->setMapping(config_con_1, 16);
	button_config->setMapping(config_con_2, 17);
	button_config->setMapping(config_turbo, 18);
	button_config->setMapping(config_mute, 19);
	button_config->setMapping(config_camera, 20);
	button_config->setMapping(config_swap_screen, 21);
	button_config->setMapping(config_shift_screen, 22);
	connect(button_config, SIGNAL(mapped(int)), this, SLOT(configure_button(int))) ;

	//Final tab layout
	QVBoxLayout* main_layout = new QVBoxLayout;
	main_layout->addWidget(tabs);
	main_layout->addWidget(button_set);
	setLayout(main_layout);

	//Config button formatting
	config_a->setMinimumWidth(150);
	config_b->setMinimumWidth(150);
	config_x->setMinimumWidth(150);
	config_y->setMinimumWidth(150);
	config_start->setMinimumWidth(150);
	config_select->setMinimumWidth(150);
	config_left->setMinimumWidth(150);
	config_right->setMinimumWidth(150);
	config_up->setMinimumWidth(150);
	config_down->setMinimumWidth(150);
	config_l->setMinimumWidth(150);
	config_r->setMinimumWidth(150);
	config_con_up->setMinimumWidth(150);
	config_con_down->setMinimumWidth(150);
	config_con_left->setMinimumWidth(150);
	config_con_right->setMinimumWidth(150);
	config_con_1->setMinimumWidth(150);
	config_con_2->setMinimumWidth(150);
	config_turbo->setMinimumWidth(150);
	config_mute->setMinimumWidth(150);
	config_camera->setMinimumWidth(150);
	config_swap_screen->setMinimumWidth(150);
	config_shift_screen->setMinimumWidth(150);

	input_a->setReadOnly(true);
	input_b->setReadOnly(true);
	input_x->setReadOnly(true);
	input_y->setReadOnly(true);
	input_start->setReadOnly(true);
	input_select->setReadOnly(true);
	input_left->setReadOnly(true);
	input_right->setReadOnly(true);
	input_up->setReadOnly(true);
	input_down->setReadOnly(true);
	input_l->setReadOnly(true);
	input_r->setReadOnly(true);
	input_con_up->setReadOnly(true);
	input_con_down->setReadOnly(true);
	input_con_left->setReadOnly(true);
	input_con_right->setReadOnly(true);
	input_turbo->setReadOnly(true);
	input_mute->setReadOnly(true);
	input_camera->setReadOnly(true);
	input_swap_screen->setReadOnly(true);
	input_shift_screen->setReadOnly(true);

	//Install event filters
	config_a->installEventFilter(this);
	config_b->installEventFilter(this);
	config_x->installEventFilter(this);
	config_y->installEventFilter(this);
	config_start->installEventFilter(this);
	config_select->installEventFilter(this);
	config_left->installEventFilter(this);
	config_right->installEventFilter(this);
	config_up->installEventFilter(this);
	config_down->installEventFilter(this);
	config_l->installEventFilter(this);
	config_r->installEventFilter(this);
	config_con_up->installEventFilter(this);
	config_con_down->installEventFilter(this);
	config_con_left->installEventFilter(this);
	config_con_right->installEventFilter(this);
	config_con_1->installEventFilter(this);
	config_con_2->installEventFilter(this);
	config_turbo->installEventFilter(this);
	config_mute->installEventFilter(this);
	config_camera->installEventFilter(this);
	config_swap_screen->installEventFilter(this);
	config_shift_screen->installEventFilter(this);

	input_a->installEventFilter(this);
	input_b->installEventFilter(this);
	input_x->installEventFilter(this);
	input_y->installEventFilter(this);
	input_start->installEventFilter(this);
	input_select->installEventFilter(this);
	input_left->installEventFilter(this);
	input_right->installEventFilter(this);
	input_up->installEventFilter(this);
	input_down->installEventFilter(this);
	input_l->installEventFilter(this);
	input_r->installEventFilter(this);
	input_con_up->installEventFilter(this);
	input_con_down->installEventFilter(this);
	input_con_left->installEventFilter(this);
	input_con_right->installEventFilter(this);
	input_con_1->installEventFilter(this);
	input_con_2->installEventFilter(this);
	input_turbo->installEventFilter(this);
	input_mute->installEventFilter(this);
	input_camera->installEventFilter(this);
	input_swap_screen->installEventFilter(this);
	input_shift_screen->installEventFilter(this);

	//Set focus policies
	config_a->setFocusPolicy(Qt::NoFocus);
	config_b->setFocusPolicy(Qt::NoFocus);
	config_x->setFocusPolicy(Qt::NoFocus);
	config_y->setFocusPolicy(Qt::NoFocus);
	config_start->setFocusPolicy(Qt::NoFocus);
	config_select->setFocusPolicy(Qt::NoFocus);
	config_left->setFocusPolicy(Qt::NoFocus);
	config_right->setFocusPolicy(Qt::NoFocus);
	config_up->setFocusPolicy(Qt::NoFocus);
	config_down->setFocusPolicy(Qt::NoFocus);
	config_l->setFocusPolicy(Qt::NoFocus);
	config_r->setFocusPolicy(Qt::NoFocus);
	config_con_up->setFocusPolicy(Qt::NoFocus);
	config_con_down->setFocusPolicy(Qt::NoFocus);
	config_con_left->setFocusPolicy(Qt::NoFocus);
	config_con_right->setFocusPolicy(Qt::NoFocus);
	config_con_1->setFocusPolicy(Qt::NoFocus);
	config_con_2->setFocusPolicy(Qt::NoFocus);
	config_turbo->setFocusPolicy(Qt::NoFocus);
	config_mute->setFocusPolicy(Qt::NoFocus);
	config_camera->setFocusPolicy(Qt::NoFocus);
	config_swap_screen->setFocusPolicy(Qt::NoFocus);
	config_shift_screen->setFocusPolicy(Qt::NoFocus);

	input_a->setFocusPolicy(Qt::NoFocus);
	input_b->setFocusPolicy(Qt::NoFocus);
	input_x->setFocusPolicy(Qt::NoFocus);
	input_y->setFocusPolicy(Qt::NoFocus);
	input_start->setFocusPolicy(Qt::NoFocus);
	input_select->setFocusPolicy(Qt::NoFocus);
	input_left->setFocusPolicy(Qt::NoFocus);
	input_right->setFocusPolicy(Qt::NoFocus);
	input_up->setFocusPolicy(Qt::NoFocus);
	input_down->setFocusPolicy(Qt::NoFocus);
	input_l->setFocusPolicy(Qt::NoFocus);
	input_r->setFocusPolicy(Qt::NoFocus);
	input_con_up->setFocusPolicy(Qt::NoFocus);
	input_con_down->setFocusPolicy(Qt::NoFocus);
	input_con_left->setFocusPolicy(Qt::NoFocus);
	input_con_right->setFocusPolicy(Qt::NoFocus);
	input_con_1->setFocusPolicy(Qt::NoFocus);
	input_con_2->setFocusPolicy(Qt::NoFocus);
	input_turbo->setFocusPolicy(Qt::NoFocus);
	input_mute->setFocusPolicy(Qt::NoFocus);
	input_camera->setFocusPolicy(Qt::NoFocus);
	input_swap_screen->setFocusPolicy(Qt::NoFocus);
	input_shift_screen->setFocusPolicy(Qt::NoFocus);

	//Joystick handling
	jstick = SDL_JoystickOpen(0);
	joystick_count = SDL_NumJoysticks();
	
	for(int x = 0; x < joystick_count; x++)
	{
		SDL_Joystick* jstick = SDL_JoystickOpen(x);
		std::string joy_name = SDL_JoystickName(jstick);
		input_device->addItem(QString::fromStdString(joy_name));
	}

	sample_rate = config::sample_rate;
	resize_screen = false;
	grab_input = false;
	input_type = 0;
	last_control_id = 0;
	is_sgb_core = false;

	dmg_cheat_menu = new cheat_menu;
	real_time_clock_menu = new rtc_menu;
	pokemon_pikachu_menu = new pp2_menu;
	pocket_sakura_menu = new ps_menu;
	full_changer_menu = new zzh_menu;
	chalien_menu = new con_ir_menu;

	multi_plust_menu = new mpos_menu;

	get_chip_list();

	resize(450, 450);
	setWindowTitle(tr("GBE+ Settings"));
}

/****** Sets various widgets to values based on the current config paramters (from .ini file) ******/
void gen_settings::set_ini_options()
{
	//Emulated system type
	sys_type->setCurrentIndex(config::gb_type);

	//Emulated SIO device
	sio_dev->setCurrentIndex(config::sio_device);

	switch(config::sio_device)
	{
		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
		case 9:
		case 10:
		case 11:
		case 12:
		case 15:
			config_sio->setEnabled(true);
			break;

		default: config_sio->setEnabled(false);
	}

	//Emulated IR device
	ir_dev->setCurrentIndex(config::ir_device);

	if((config::ir_device == 1) || (config::ir_device == 2) || (config::ir_device == 3) || (config::ir_device == 5)) { config_ir->setEnabled(true); }
	else { config_ir->setEnabled(false); }

	//Emulated CPU speed
	overclock->setCurrentIndex(config::oc_flags);

	//BIOS or Boot ROM option
	if(config::use_bios) { bios->setChecked(true); }
	else { bios->setChecked(false); }

	//Use firmware
	if(config::use_firmware) { firmware->setChecked(true); }
	else { firmware->setChecked(false); }

	//Enable patches
	if(config::use_patches) { auto_patch->setChecked(true); }
	else { auto_patch->setChecked(false); }

	//Use cheats
	if(config::use_cheats) { cheats->setChecked(true); }
	else { cheats->setChecked(false); }

	//RTC offsets
	real_time_clock_menu->secs_offset->setValue(config::rtc_offset[0]);
	real_time_clock_menu->mins_offset->setValue(config::rtc_offset[1]);
	real_time_clock_menu->hours_offset->setValue(config::rtc_offset[2]);
	real_time_clock_menu->days_offset->setValue(config::rtc_offset[3]);

	//Full Changer
	if(config::ir_db_index < 1) { full_changer_menu->cosmic_character->setCurrentIndex(config::ir_db_index); }
	else { full_changer_menu->cosmic_character->setCurrentIndex(0); }

	//Pokemon Pikachu 2
	if(config::ir_db_index < 6) { pokemon_pikachu_menu->watts->setCurrentIndex(config::ir_db_index); }
	else { pokemon_pikachu_menu->watts->setCurrentIndex(0); }

	//Pocket Sakura
	if(config::ir_db_index < 4) { pocket_sakura_menu->points->setCurrentIndex(config::ir_db_index); }
	else { pocket_sakura_menu->points->setCurrentIndex(0); }

	//Constant IR Light
	if(config::ir_db_index < 2) { chalien_menu->ir_mode->setCurrentIndex(config::ir_db_index); }
	else { chalien_menu->ir_mode->setCurrentIndex(0); }

	//Screen scale options
	screen_scale->setCurrentIndex(config::scaling_factor - 1);

	//CGFX scale options
	cgfx_scale->setCurrentIndex(cgfx::scaling_factor - 1);

	//DMG-on-GBC palette options
	dmg_gbc_pal->setCurrentIndex(config::dmg_gbc_pal);

	//OpenGL Fragment Shader
	if(config::fragment_shader == (config::data_path + "shaders/fragment.fs")) { ogl_frag_shader->setCurrentIndex(0); }
	else if(config::fragment_shader == (config::data_path + "shaders/2xBR.fs")) { ogl_frag_shader->setCurrentIndex(1); }
	else if(config::fragment_shader == (config::data_path + "shaders/4xBR.fs")) { ogl_frag_shader->setCurrentIndex(2); }
	else if(config::fragment_shader == (config::data_path + "shaders/bad_bloom.fs")) { ogl_frag_shader->setCurrentIndex(3); }
	else if(config::fragment_shader == (config::data_path + "shaders/badder_bloom.fs")) { ogl_frag_shader->setCurrentIndex(4); }
	else if(config::fragment_shader == (config::data_path + "shaders/chrono.fs")) { ogl_frag_shader->setCurrentIndex(5); }
	else if(config::fragment_shader == (config::data_path + "shaders/dmg_mode.fs")) { ogl_frag_shader->setCurrentIndex(6); }
	else if(config::fragment_shader == (config::data_path + "shaders/gba_gamma.fs")) { ogl_frag_shader->setCurrentIndex(7); }
	else if(config::fragment_shader == (config::data_path + "shaders/gbc_gamma.fs")) { ogl_frag_shader->setCurrentIndex(8); }
	else if(config::fragment_shader == (config::data_path + "shaders/grayscale.fs")) { ogl_frag_shader->setCurrentIndex(9); }
	else if(config::fragment_shader == (config::data_path + "shaders/lcd_mode.fs")) { ogl_frag_shader->setCurrentIndex(10); }
	else if(config::fragment_shader == (config::data_path + "shaders/pastel.fs")) { ogl_frag_shader->setCurrentIndex(11); }
	else if(config::fragment_shader == (config::data_path + "shaders/scale2x.fs")) { ogl_frag_shader->setCurrentIndex(12); }
	else if(config::fragment_shader == (config::data_path + "shaders/scale3x.fs")) { ogl_frag_shader->setCurrentIndex(13); }
	else if(config::fragment_shader == (config::data_path + "shaders/sepia.fs")) { ogl_frag_shader->setCurrentIndex(14); }
	else if(config::fragment_shader == (config::data_path + "shaders/spotlight.fs")) { ogl_frag_shader->setCurrentIndex(15); }
	else if(config::fragment_shader == (config::data_path + "shaders/tv_mode.fs")) { ogl_frag_shader->setCurrentIndex(16); }
	else if(config::fragment_shader == (config::data_path + "shaders/washout.fs")) { ogl_frag_shader->setCurrentIndex(17); }

	//OpenGL Vertex Shader
	if(config::vertex_shader == (config::data_path + "shaders/vertex.fs")) { ogl_vert_shader->setCurrentIndex(0); }
	else if(config::vertex_shader == (config::data_path + "shaders/invert_x.vs")) { ogl_vert_shader->setCurrentIndex(1); }

	//OpenGL option
	if(config::use_opengl)
	{
		ogl->setChecked(true);
		ogl_frag_shader->setEnabled(true);
		ogl_vert_shader->setEnabled(true);
	}

	else
	{
		ogl->setChecked(false);
		ogl_frag_shader->setEnabled(false);
		ogl_vert_shader->setEnabled(false);
	}

	//CGFX option
	if(cgfx::load_cgfx)
	{
		load_cgfx->setChecked(true);
		cgfx_scale->setEnabled(true);
	}

	else
	{
		load_cgfx->setChecked(false);
		cgfx_scale->setEnabled(false);
	}

	//Maintain aspect ratio option
	if(config::maintain_aspect_ratio) { aspect_ratio->setChecked(true); }
	else { aspect_ratio->setChecked(false); }

	//OSD option
	if(config::use_osd) { osd_enable->setChecked(true); }
	else { osd_enable->setChecked(false); }

	//Sample rate option
	switch((int)config::sample_rate)
	{
		case 11025: freq->setCurrentIndex(3); break;
		case 22050: freq->setCurrentIndex(2); break;
		case 44100: freq->setCurrentIndex(1); break;
		case 48000: freq->setCurrentIndex(0); break;
	}

	//Volume option
	volume->setValue(config::volume);

	//Mute option
	if(config::mute == 1)
	{
		config::volume = 0;
		sound_on->setChecked(false);
		volume->setEnabled(false);
	}

	else
	{
		sound_on->setChecked(true);
		volume->setEnabled(true);
	}

	//Stereo sound option
	if(config::use_stereo) { stereo_enable->setChecked(true); }
	else { stereo_enable->setChecked(false); }

	//Dead-zone
	dead_zone->setValue(config::dead_zone);

	//Keyboard controls
	input_a->setText(QString::number(config::gbe_key_a));
	input_b->setText(QString::number(config::gbe_key_b));
	input_x->setText(QString::number(config::gbe_key_x));
	input_y->setText(QString::number(config::gbe_key_y));
	input_start->setText(QString::number(config::gbe_key_start));
	input_select->setText(QString::number(config::gbe_key_select));
	input_left->setText(QString::number(config::gbe_key_left));
	input_right->setText(QString::number(config::gbe_key_right));
	input_up->setText(QString::number(config::gbe_key_up));
	input_down->setText(QString::number(config::gbe_key_down));
	input_l->setText(QString::number(config::gbe_key_l_trigger));
	input_r->setText(QString::number(config::gbe_key_r_trigger));
	input_con_up->setText(QString::number(config::con_key_up));
	input_con_down->setText(QString::number(config::con_key_down));
	input_con_left->setText(QString::number(config::con_key_left));
	input_con_right->setText(QString::number(config::con_key_right));
	input_con_1->setText(QString::number(config::con_key_1));
	input_con_2->setText(QString::number(config::con_key_2));
	input_turbo->setText(QString::number(config::hotkey_turbo));
	input_mute->setText(QString::number(config::hotkey_mute));
	input_camera->setText(QString::number(config::hotkey_camera));
	input_swap_screen->setText(QString::number(config::hotkey_swap_screen));
	input_shift_screen->setText(QString::number(config::hotkey_shift_screen));

	//BIOS, Boot ROM and Manifest paths
	QString path_1(QString::fromStdString(config::dmg_bios_path));
	QString path_2(QString::fromStdString(config::gbc_bios_path));
	QString path_3(QString::fromStdString(config::agb_bios_path));
	QString path_4(QString::fromStdString(config::nds_firmware_path));
	QString path_5(QString::fromStdString(cgfx::manifest_file));
	QString path_6(QString::fromStdString(config::ss_path));
	QString path_7(QString::fromStdString(cgfx::dump_bg_path));
	QString path_8(QString::fromStdString(cgfx::dump_obj_path));
	QString path_9(QString::fromStdString(config::save_path));
	QString path_10(QString::fromStdString(config::cheats_path));

	//Rumble
	if(config::use_haptics) { rumble_on->setChecked(true); }
	else { rumble_on->setChecked(false); }

	//Netplay
	if(config::use_netplay) { enable_netplay->setChecked(true); }
	else { enable_netplay->setChecked(false); }

	if(config::netplay_hard_sync)
	{
		hard_sync->setChecked(true);
		sync_threshold->setEnabled(true);
	}

	else
	{
		hard_sync->setChecked(false);
		sync_threshold->setEnabled(false);
	}

	//Net Gate
	if(config::use_net_gate) { net_gate->setChecked(true); }
	else { net_gate->setChecked(false); }

	//Battle Gate Type
	if(config::sio_device == 11) { chip_gate_type->setCurrentIndex(1); }
	else if(config::sio_device == 12) { chip_gate_type->setCurrentIndex(2); }
	else { chip_gate_type->setCurrentIndex(0); }

	//Battle Chips 1-4
	for(u32 x = 0; x < 512; x++)
	{
		if(chip_list[x] == init_chip_list[0]) { battle_chip_1->setCurrentIndex(x); }
		if(chip_list[x] == init_chip_list[1]) { battle_chip_2->setCurrentIndex(x); }
		if(chip_list[x] == init_chip_list[2]) { battle_chip_3->setCurrentIndex(x); }
		if(chip_list[x] == init_chip_list[3]) { battle_chip_4->setCurrentIndex(x); }
	}

	sync_threshold->setValue(config::netplay_sync_threshold);
	server_port->setValue(config::netplay_server_port);
	client_port->setValue(config::netplay_client_port);
	ip_address->setText(QString::fromStdString(config::netplay_client_ip));

	dmg_bios->setText(path_1);
	gbc_bios->setText(path_2);
	gba_bios->setText(path_3);
	nds_firmware->setText(path_4);
	manifest->setText(path_5);
	screenshot->setText(path_6);
	dump_bg->setText(path_7);
	dump_obj->setText(path_8);
	game_saves->setText(path_9);
	cheats_path->setText(path_10);
}

/****** Toggles whether to use the Boot ROM or BIOS ******/
void gen_settings::set_bios()
{
	if(bios->isChecked()) { config::use_bios = true; }
	else { config::use_bios = false; }
}

/****** Toggles whether to use firmware ******/
void gen_settings::set_firmware()
{
	if(firmware->isChecked()) { config::use_firmware = true; }
	else { config::use_firmware = false; }
}

/****** Changes the emulated Serial IO device ******/
void gen_settings::sio_dev_change()
{
	config::sio_device = sio_dev->currentIndex();

	switch(config::sio_device)
	{
		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
		case 9:
		case 10:
		case 11:
		case 12:
		case 15:
			config_sio->setEnabled(true);
			break;

		default: config_sio->setEnabled(false);
	}
}

/****** Changes the emulated IR device ******/
void gen_settings::ir_dev_change()
{
	//Reset IR database index when switching devices
	if(config::ir_device != ir_dev->currentIndex())
	{
		config::ir_db_index = 0;
		full_changer_menu->cosmic_character->setCurrentIndex(0);
		pokemon_pikachu_menu->watts->setCurrentIndex(0);
		pocket_sakura_menu->points->setCurrentIndex(0);
		chalien_menu->ir_mode->setCurrentIndex(0);
	}

	config::ir_device = ir_dev->currentIndex();

	if((config::ir_device == 1) || (config::ir_device == 2) || (config::ir_device == 3) || config::ir_device == 5) { config_ir->setEnabled(true); }
	else { config_ir->setEnabled(false); }
}

/****** Changes the emulated CPU speed ******/
void gen_settings::overclock_change()
{
	config::oc_flags = overclock->currentIndex();
}

/****** Toggles whether to enable auto-patching ******/
void gen_settings::set_patches()
{
	if(auto_patch->isChecked()) { config::use_patches = true; }
	else { config::use_patches = false; }
}

/****** Displays the cheats window ******/
void gen_settings::show_cheats()
{
	dmg_cheat_menu->fetch_cheats();
	dmg_cheat_menu->show();
}

/****** Displays the cheats window ******/
void gen_settings::show_rtc()
{
	real_time_clock_menu->show();
}

/****** Displays relevant SIO configuration window ******/
void gen_settings::show_sio_config()
{
	switch(config::sio_device)
	{
		case 1: tabs->setCurrentIndex(4); break;
		case 4: qt_gui::draw_surface->set_card_file(); break;
		case 5: qt_gui::draw_surface->set_card_file(); break;
		case 6: tabs->setCurrentIndex(4); break;
		case 7: tabs->setCurrentIndex(4); break;
		case 9: qt_gui::draw_surface->set_data_file(); break;
		case 10: tabs->setCurrentIndex(3); controls_combo->setCurrentIndex(3); chip_gate_type->setCurrentIndex(0); break;
		case 11: tabs->setCurrentIndex(3); controls_combo->setCurrentIndex(3); chip_gate_type->setCurrentIndex(1); break;
		case 12: tabs->setCurrentIndex(3); controls_combo->setCurrentIndex(3); chip_gate_type->setCurrentIndex(2); break;
		case 15: multi_plust_menu->show(); break;
	}
}

/****** Displays relevant IR configuration window ******/
void gen_settings::show_ir_config()
{
	switch(config::ir_device)
	{
		case 0x1: full_changer_menu->show(); break;
		case 0x2: pokemon_pikachu_menu->show(); break;
		case 0x3: pocket_sakura_menu->show(); break;
		case 0x5: chalien_menu->show(); break;
	}	
}

/****** Toggles enabling or disabling the fragment and vertex shader widgets when setting OpenGL ******/
void gen_settings::set_ogl()
{
	if(ogl->isChecked())
	{
		ogl_frag_shader->setEnabled(true);
		ogl_vert_shader->setEnabled(true);
	}

	else
	{
		ogl_frag_shader->setEnabled(false);
		ogl_frag_shader->setEnabled(false);
	}
}

/****** Changes the display scale ******/
void gen_settings::screen_scale_change()
{
	config::scaling_factor = (screen_scale->currentIndex() + 1);
	resize_screen = true;
}

/****** Sets whether to maintain the aspect ratio or not ******/
void gen_settings::aspect_ratio_change()
{
	if(aspect_ratio->isChecked()) { config::maintain_aspect_ratio = true; }
	else { config::maintain_aspect_ratio = false; }
}

/****** Toggles enabling or disabling on-screen display messages ******/
void gen_settings::set_osd()
{
	if(osd_enable->isChecked()) { config::use_osd = true; }
	else { config::use_osd = false; }
}

/****** Changes the emulated DMG-on-GBC palette ******/
void gen_settings::dmg_gbc_pal_change()
{
	config::dmg_gbc_pal = (dmg_gbc_pal->currentIndex());
	set_dmg_colors(config::dmg_gbc_pal);
}

/****** Changes the current OpenGL fragment shader ******/
void gen_settings::ogl_frag_change()
{
	switch(ogl_frag_shader->currentIndex())
	{
		case 0: config::fragment_shader = config::data_path + "shaders/fragment.fs"; break;
		case 1: config::fragment_shader = config::data_path + "shaders/2xBR.fs"; break;
		case 2: config::fragment_shader = config::data_path + "shaders/4xBR.fs"; break;
		case 3: config::fragment_shader = config::data_path + "shaders/bad_bloom.fs"; break;
		case 4: config::fragment_shader = config::data_path + "shaders/badder_bloom.fs"; break;
		case 5: config::fragment_shader = config::data_path + "shaders/chrono.fs"; break;
		case 6: config::fragment_shader = config::data_path + "shaders/dmg_mode.fs"; break;
		case 7: config::fragment_shader = config::data_path + "shaders/gba_gamma.fs"; break;
		case 8: config::fragment_shader = config::data_path + "shaders/gbc_gamma.fs"; break;
		case 9: config::fragment_shader = config::data_path + "shaders/grayscale.fs"; break;
		case 10: config::fragment_shader = config::data_path + "shaders/lcd_mode.fs"; break;
		case 11: config::fragment_shader = config::data_path + "shaders/pastel.fs"; break;
		case 12: config::fragment_shader = config::data_path + "shaders/scale2x.fs"; break;
		case 13: config::fragment_shader = config::data_path + "shaders/scale3x.fs"; break;
		case 14: config::fragment_shader = config::data_path + "shaders/sepia.fs"; break;
		case 15: config::fragment_shader = config::data_path + "shaders/spotlight.fs"; break;
		case 16: config::fragment_shader = config::data_path + "shaders/tv_mode.fs"; break;
		case 17: config::fragment_shader = config::data_path + "shaders/washout.fs"; break;
	}

	if((main_menu::gbe_plus != NULL) && (config::use_opengl))
	{
		qt_gui::draw_surface->hw_screen->reload_shaders();
		qt_gui::draw_surface->hw_screen->update();
	}
}

/****** Changes the current OpenGL vertex shader ******/
void gen_settings::ogl_vert_change()
{
	switch(ogl_vert_shader->currentIndex())
	{
		case 0: config::vertex_shader = config::data_path + "shaders/vertex.vs"; break;
		case 1: config::vertex_shader = config::data_path + "shaders/invert_x.vs"; break;
	}

	if((main_menu::gbe_plus != NULL) && (config::use_opengl))
	{
		qt_gui::draw_surface->hw_screen->reload_shaders();
		qt_gui::draw_surface->hw_screen->update();
	}
}

/****** Toggles activation of custom graphics ******/
void gen_settings::set_cgfx()
{
	if(load_cgfx->isChecked())
	{
		//Test that the manifest file exists before enabling anything
		QFile test_file(QString::fromStdString(cgfx::manifest_file));

		//Display warning if the file is not there
		if(!test_file.exists())
		{
			load_cgfx->setChecked(false);
			load_cgfx->setCheckState(Qt::Unchecked);

			std::string mesg_text = "The manifest file: '" + cgfx::manifest_file + "' could not be loaded. Check paths and file permissions. CGFX will not load without a manifest file!"; 
			warning_box->setText(QString::fromStdString(mesg_text));
			warning_box->show();
			return;
		}
		
		cgfx::load_cgfx = true;
		cgfx_scale->setEnabled(true);
	}

	else
	{
		cgfx::load_cgfx = false;
		cgfx_scale->setEnabled(false);
	}

	//Instruct core to invalidate any CGFX when turning option on or off during gameplay
	if(main_menu::gbe_plus != NULL)
	{
		//Prevent SGB core from CGFX actions while running
		if(is_sgb_core)
		{
			load_cgfx->setChecked(false);
			load_cgfx->setCheckState(Qt::Unchecked);
			cgfx::load_cgfx = false;
			cgfx_scale->setEnabled(false);
		}

		switch(config::gb_type)
		{
			case 0:
			case 1:
			case 2:	
				main_menu::gbe_plus->get_core_data(2);
				break;
		}
	}
}

/****** Dynamically changes the core's volume ******/
void gen_settings::volume_change() 
{
	//Update volume while playing
	if((main_menu::gbe_plus != NULL) && (sound_on->isChecked()))
	{
		main_menu::gbe_plus->update_volume(volume->value());
	}

	//Update the volume while using only the GUI
	else { config::volume = volume->value(); }	
}

/****** Updates the core's volume - Used when loading save states ******/
void gen_settings::update_volume() { mute(); }

/****** Mutes the core's volume ******/
void gen_settings::mute()
{
	//Mute/unmute while playing
	if(main_menu::gbe_plus != NULL)
	{
		//Unmute, use slider volume
		if(sound_on->isChecked())
		{
			main_menu::gbe_plus->update_volume(volume->value());
			volume->setEnabled(true);
		}

		//Mute
		else
		{
			main_menu::gbe_plus->update_volume(0);
			volume->setEnabled(false);
		}
	}

	//Mute/unmute while using only the GUI
	else
	{
		//Unmute, use slider volume
		if(sound_on->isChecked())
		{
			config::volume = volume->value();
			volume->setEnabled(true);
		}

		//Mute
		else
		{
			config::volume = 0;
			volume->setEnabled(false);
		}
	}
}

/****** Changes the core's sample rate ******/
void gen_settings::sample_rate_change()
{
	switch(freq->currentIndex())
	{
		case 0: sample_rate = 48000.0; break;
		case 1: sample_rate = 44100.0; break;
		case 2: sample_rate = 22050.0; break;
		case 3: sample_rate = 11025.0; break;
	}
}

/****** Sets a path via file browser ******/
void gen_settings::set_paths(int index)
{
	QString path;

	//Open file browser for Boot ROMs, BIOS, Firmware, cheats, and manifests
	if((index < 5) || (index == 9))
	{
		path = QFileDialog::getOpenFileName(this, tr("Open"), "", tr("All files (*)"));
		if(path.isNull()) { return; }
	}

	//Open folder browser for screenshots, CGFX dumps, game saves
	else
	{
		//Open the data folder for CGFX dumps
		//On Linux or Unix, this is supposed to be a hidden folder, so we need a custom dialog
		//This uses relative paths, but for game saves we need full path, so ignore if index is 8
		if((index >= 6) && (index != 8))
		{
			data_folder->open_data_folder();			

			while(!data_folder->finish) { QApplication::processEvents(); }
	
			path = data_folder->directory().path();
			path = data_folder->path.relativeFilePath(path);
		}

		else { path = QFileDialog::getExistingDirectory(this, tr("Open"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks); }
		
		if(path.isNull()) { return; }	

		//Make sure path is complete, e.g. has the correct separator at the end
		//Qt doesn't append this automatically
		std::string temp_str = path.toStdString();
		std::string temp_chr = "";
		temp_chr = temp_str[temp_str.length() - 1];

		if((temp_chr != "/") && (temp_chr != "\\")) { path.append("/"); }
		path = QDir::toNativeSeparators(path);
	}

	switch(index)
	{
		case 0: 
			config::dmg_bios_path = path.toStdString();
			dmg_bios->setText(path);
			break;

		case 1:
			config::gbc_bios_path = path.toStdString();
			gbc_bios->setText(path);
			break;

		case 2:
			config::agb_bios_path = path.toStdString();
			gba_bios->setText(path);
			break;

		case 3:
			config::nds_firmware_path = path.toStdString();
			nds_firmware->setText(path);
			break;

		case 4:
			cgfx::manifest_file = path.toStdString();
			manifest->setText(path);
			break;

		case 5:
			config::ss_path = path.toStdString();
			screenshot->setText(path);
			break;

		case 6:
			cgfx::dump_bg_path = path.toStdString();
			dump_bg->setText(path);
			break;

		case 7:
			cgfx::dump_obj_path = path.toStdString();
			dump_obj->setText(path);
			break;

		case 8:
			config::save_path = path.toStdString();
			game_saves->setText(path);
			break;

		case 9:
			config::cheats_path = path.toStdString();
			cheats_path->setText(path);

			//Make sure to update cheats from new file
			if(!dmg_cheat_menu->empty_cheats) { dmg_cheat_menu->empty_cheats = parse_cheats_file(true); }
			else { parse_cheats_file(false); }

			break;
	}
}

/****** Rebuilds input device index ******/
void gen_settings::rebuild_input_index()
{
	SDL_JoystickUpdate();

	//Rebuild input device index
	if(SDL_NumJoysticks() != joystick_count)
	{
		joystick_count = SDL_NumJoysticks();
		input_device->clear();
		input_device->addItem("Keyboard");

		for(int x = 0; x < joystick_count; x++)
		{
			SDL_Joystick* jstick = SDL_JoystickOpen(x);
			std::string joy_name = SDL_JoystickName(jstick);
			input_device->addItem(QString::fromStdString(joy_name));
			
			if(x == 0) { config::joy_sdl_id = SDL_JoystickInstanceID(jstick); }
		}

		//Set input index to 0
		input_device->setCurrentIndex(0);

		//Set joy id to 0 until new joystick selected
		config::joy_id = 0;
	}
}

/****** Changes the input device to configure ******/
void gen_settings::input_device_change()
{
	input_type = input_device->currentIndex();

	//Switch to keyboard input configuration
	if(input_type == 0)
	{
		input_a->setText(QString::number(config::gbe_key_a));
		input_b->setText(QString::number(config::gbe_key_b));
		input_x->setText(QString::number(config::gbe_key_x));
		input_y->setText(QString::number(config::gbe_key_y));
		input_start->setText(QString::number(config::gbe_key_start));
		input_select->setText(QString::number(config::gbe_key_select));
		input_left->setText(QString::number(config::gbe_key_left));
		input_right->setText(QString::number(config::gbe_key_right));
		input_up->setText(QString::number(config::gbe_key_up));
		input_down->setText(QString::number(config::gbe_key_down));
		input_l->setText(QString::number(config::gbe_key_l_trigger));
		input_r->setText(QString::number(config::gbe_key_r_trigger));
		input_con_up->setText(QString::number(config::con_key_up));
		input_con_down->setText(QString::number(config::con_key_down));
		input_con_left->setText(QString::number(config::con_key_left));
		input_con_right->setText(QString::number(config::con_key_right));
		input_con_1->setText(QString::number(config::con_key_1));
		input_con_2->setText(QString::number(config::con_key_2));
	}

	else
	{
		input_a->setText(QString::number(config::gbe_joy_a));
		input_b->setText(QString::number(config::gbe_joy_b));
		input_x->setText(QString::number(config::gbe_joy_x));
		input_y->setText(QString::number(config::gbe_joy_y));
		input_start->setText(QString::number(config::gbe_joy_start));
		input_select->setText(QString::number(config::gbe_joy_select));
		input_left->setText(QString::number(config::gbe_joy_left));
		input_right->setText(QString::number(config::gbe_joy_right));
		input_up->setText(QString::number(config::gbe_joy_up));
		input_down->setText(QString::number(config::gbe_joy_down));
		input_l->setText(QString::number(config::gbe_joy_l_trigger));
		input_r->setText(QString::number(config::gbe_joy_r_trigger));
		input_con_up->setText(QString::number(config::con_joy_up));
		input_con_down->setText(QString::number(config::con_joy_down));
		input_con_left->setText(QString::number(config::con_joy_left));
		input_con_right->setText(QString::number(config::con_joy_right));
		input_con_1->setText(QString::number(config::con_joy_1));
		input_con_2->setText(QString::number(config::con_joy_2));

		//Use new joystick id
		config::joy_id = input_device->currentIndex() - 1;
		jstick = SDL_JoystickOpen(config::joy_id);
		config::joy_sdl_id = SDL_JoystickInstanceID(jstick);
	}

	close_input();
}

/****** Dynamically changes the core pad's dead-zone ******/
void gen_settings::dead_zone_change() { config::dead_zone = dead_zone->value(); }

/****** Sets the netplay enable option ******/
void gen_settings::set_netplay()
{
	if(enable_netplay->isChecked()) { config::use_netplay = true; }
	else { config::use_netplay = false; }
}

/****** Sets the netplay hard sync option ******/
void gen_settings::set_hard_sync()
{
	if(hard_sync->isChecked())
	{
		config::netplay_hard_sync = true;
		sync_threshold->setEnabled(true);
	}

	else
	{
		config::netplay_hard_sync = false;
		sync_threshold->setEnabled(false);
	}
}

/****** Sets the Net Gate option ******/
void gen_settings::set_net_gate()
{
	if(net_gate->isChecked()) { config::use_net_gate = true; }
	else { config::use_net_gate = false; }
}

/****** Updates the current Battle Chip list ******/
void gen_settings::get_chip_list()
{
	std::string list_file = config::data_path + "chip_gate/";
	std::string input_line = "";
	u32 line_count = 0;
	u32 chip_count = 0;

	switch(chip_gate_type->currentIndex())
	{
		case 0: list_file += "bcg_list.txt"; break;
		case 1: list_file += "pcg_list.txt"; break;
		case 2: list_file += "blg_list.txt"; break;
	}

	//Clear current combo-boxes for Battle Chips 1-4
	battle_chip_1->clear();
	battle_chip_2->clear();
	battle_chip_3->clear();
	battle_chip_4->clear();

	//Open text file and grab all lines
	std::ifstream file(list_file.c_str(), std::ios::in);

	if(!file.is_open()) { return; }

	while(getline(file, input_line))
	{
		line_count++;

		//Add chip 
		if(!input_line.empty())
		{
			chip_list[chip_count++] = line_count;
			battle_chip_1->addItem(QString::fromStdString(input_line));
			battle_chip_2->addItem(QString::fromStdString(input_line));
			battle_chip_3->addItem(QString::fromStdString(input_line));
			battle_chip_4->addItem(QString::fromStdString(input_line));
		}
	}
}

/****** Sets Battle Chip ID based on chip list ******/
void gen_settings::set_battle_chip()
{
	u16 index = battle_chip_1->currentIndex();
	config::chip_list[0] = chip_list[index];

	index = battle_chip_2->currentIndex();
	config::chip_list[1] = chip_list[index];

	index = battle_chip_3->currentIndex();
	config::chip_list[2] = chip_list[index];

	index = battle_chip_4->currentIndex();
	config::chip_list[3] = chip_list[index];
}

/****** Sets the netplay sync threshold ******/
void gen_settings::update_sync_threshold()
{
	config::netplay_sync_threshold = sync_threshold->value();
}

/****** Sets the netplay server port ******/
void gen_settings::update_server_port()
{
	config::netplay_server_port = server_port->value();
}

/****** Sets the netplay client port ******/
void gen_settings::update_client_port()
{
	config::netplay_client_port = client_port->value();
}

/****** Sets the client IP address ******/
void gen_settings::update_ip_addr()
{
	std::string temp = ip_address->text().toStdString();
	u32 check = 0;

	if(!util::ip_to_u32(temp, check))
	{
		ip_address->setText(QString::fromStdString(config::netplay_client_ip));
	}

	else
	{
		config::netplay_client_ip = temp;
	}
}

/****** Prepares GUI to receive input for controller configuration ******/
void gen_settings::configure_button(int button)
{
	if(grab_input) { return; }

	grab_input = true;

	switch(button)
	{
		case 0: 
			input_delay(config_a);
			input_a->setFocus();
			input_index = 0;
			break;

		case 1: 
			input_delay(config_b);
			input_b->setFocus();
			input_index = 1;
			break;

		case 2: 
			input_delay(config_x);
			input_x->setFocus();
			input_index = 2;
			break;

		case 3: 
			input_delay(config_y);
			input_y->setFocus();
			input_index = 3;
			break;

		case 4: 
			input_delay(config_start);
			input_start->setFocus();
			input_index = 4;
			break;

		case 5: 
			input_delay(config_select);
			input_select->setFocus();
			input_index = 5;
			break;

		case 6: 
			input_delay(config_left);
			input_left->setFocus();
			input_index = 6;
			break;

		case 7: 
			input_delay(config_right);
			input_right->setFocus();
			input_index = 7;
			break;

		case 8: 
			input_delay(config_up);
			input_up->setFocus();
			input_index = 8;
			break;

		case 9: 
			input_delay(config_down);
			input_down->setFocus();
			input_index = 9;
			break;

		case 10: 
			input_delay(config_l);
			input_l->setFocus();
			input_index = 10;
			break;

		case 11: 
			input_delay(config_r);
			input_r->setFocus();
			input_index = 11;
			break;

		case 12: 
			input_delay(config_con_up);
			input_con_up->setFocus();
			input_index = 12;
			break;

		case 13: 
			input_delay(config_con_down);
			input_con_down->setFocus();
			input_index = 13;
			break;

		case 14: 
			input_delay(config_con_left);
			input_con_left->setFocus();
			input_index = 14;
			break;

		case 15: 
			input_delay(config_con_right);
			input_con_right->setFocus();
			input_index = 15;
			break;

		case 16: 
			input_delay(config_con_1);
			input_con_1->setFocus();
			input_index = 16;
			break;

		case 17: 
			input_delay(config_con_2);
			input_con_2->setFocus();
			input_index = 17;
			break;

		case 18:
			input_delay(config_turbo);
			input_turbo->setFocus();
			input_index = 18;
			break;

		case 19:
			input_delay(config_mute);
			input_turbo->setFocus();
			input_index = 19;
			break;

		case 20:
			input_delay(config_camera);
			input_turbo->setFocus();
			input_index = 20;
			break;

		case 21:
			input_delay(config_swap_screen);
			input_turbo->setFocus();
			input_index = 21;
			break;

		case 22:
			input_delay(config_shift_screen);
			input_turbo->setFocus();
			input_index = 22;
			break;
	}

	if(input_type != 0) { process_joystick_event(); }
}			

/****** Delays input from the GUI ******/
void gen_settings::input_delay(QPushButton* input_button)
{
	//Delay input for joysticks
	if(input_type != 0)
	{
		input_button->setText("Enter Input - 3");
		QApplication::processEvents();
		input_button->setText("Enter Input - 3");
		input_button->update();
		QApplication::processEvents();
		SDL_Delay(1000);

		input_button->setText("Enter Input - 2");
		QApplication::processEvents();
		input_button->setText("Enter Input - 2");
		input_button->update();
		QApplication::processEvents();
		SDL_Delay(1000);

		input_button->setText("Enter Input - 1");
		QApplication::processEvents();
		input_button->setText("Enter Input - 1");
		input_button->update();
		QApplication::processEvents();
		SDL_Delay(1000);
	}

	//Grab input immediately for keyboards
	else { input_button->setText("Enter Input"); }	
}

/****** Handles joystick input ******/
void gen_settings::process_joystick_event()
{
	SDL_Event joy_event;
	int pad = 0;

	while(SDL_PollEvent(&joy_event))
	{
		//Generate pad id
		switch(joy_event.type)
		{
			case SDL_JOYBUTTONDOWN:
				pad = 100 + joy_event.jbutton.button; 
				grab_input = false;
				break;

			case SDL_JOYAXISMOTION:
				if(abs(joy_event.jaxis.value) >= config::dead_zone)
				{
					pad = 200 + (joy_event.jaxis.axis * 2);
					if(joy_event.jaxis.value > 0) { pad++; }
					grab_input = false;
				}

				break;

			case SDL_JOYHATMOTION:
				pad = 300 + (joy_event.jhat.hat * 4);
				grab_input = false;
						
				switch(joy_event.jhat.value)
				{
					case SDL_HAT_RIGHT: pad += 1; break;
					case SDL_HAT_UP: pad += 2; break;
					case SDL_HAT_DOWN: pad += 3; break;
				}

				break;
		}
	}

	switch(input_index)
	{
		case 0:
			if(pad != 0)
			{
				config::gbe_joy_a = pad;
				input_a->setText(QString::number(pad));
			}

			config_a->setText("Configure");
			input_a->clearFocus();
			break;

		case 1:
			if(pad != 0)
			{
				config::gbe_joy_b = pad;
				input_b->setText(QString::number(pad));
			}

			config_b->setText("Configure");
			input_b->clearFocus();
			break;

		case 2:
			if(pad != 0)
			{
				config::gbe_joy_x = pad;
				input_x->setText(QString::number(pad));
			}

			config_x->setText("Configure");
			input_x->clearFocus();
			break;

		case 3:
			if(pad != 0)
			{
				config::gbe_joy_y = pad;
				input_y->setText(QString::number(pad));
			}

			config_y->setText("Configure");
			input_y->clearFocus();
			break;

		case 4:
			if(pad != 0)
			{
				config::gbe_joy_start = pad;
				input_start->setText(QString::number(pad));
			}

			config_start->setText("Configure");
			input_start->clearFocus();
			break;

		case 5:
			if(pad != 0)
			{
				config::gbe_joy_select = pad;
				input_select->setText(QString::number(pad));
			}

			config_select->setText("Configure");
			input_select->clearFocus();
			break;

		case 6:
			if(pad != 0)
			{
				config::gbe_joy_left = pad;
				input_left->setText(QString::number(pad));
			}

			config_left->setText("Configure");
			input_left->clearFocus();
			break;

		case 7:
			if(pad != 0)
			{
				config::gbe_joy_right = pad;
				input_right->setText(QString::number(pad));
			}

			config_right->setText("Configure");
			input_right->clearFocus();
			break;

		case 8:
			if(pad != 0)
			{
				config::gbe_joy_up = pad;
				input_up->setText(QString::number(pad));
			}

			config_up->setText("Configure");
			input_up->clearFocus();
			break;

		case 9:
			if(pad != 0)
			{
				config::gbe_joy_down = pad;
				input_down->setText(QString::number(pad));
			}

			config_down->setText("Configure");
			input_down->clearFocus();
			break;

		case 10: 
			if(pad != 0)
			{
				config::gbe_joy_l_trigger = pad;
				input_l->setText(QString::number(pad));
			}

			config_l->setText("Configure");
			input_l->clearFocus();
			break;

		case 11:
			if(pad != 0)
			{
				config::gbe_joy_r_trigger = pad;
				input_r->setText(QString::number(pad));
			}

			config_r->setText("Configure");
			input_r->clearFocus();
			break;

		case 12:
			if(pad != 0)
			{
				config::con_joy_up = pad;
				input_con_up->setText(QString::number(pad));
			}

			config_con_up->setText("Configure");
			input_con_up->clearFocus();
			break;

		case 13:
			if(pad != 0)
			{
				config::con_joy_down = pad;
				input_con_down->setText(QString::number(pad));
			}

			config_con_down->setText("Configure");
			input_con_down->clearFocus();
			break;

		case 14:
			if(pad != 0)
			{
				config::con_joy_left = pad;
				input_con_left->setText(QString::number(pad));
			}

			config_con_left->setText("Configure");
			input_con_left->clearFocus();
			break;

		case 15:
			if(pad != 0)
			{
				config::con_joy_right = pad;
				input_con_right->setText(QString::number(pad));
			}

			config_con_right->setText("Configure");
			input_con_right->clearFocus();
			break;

		case 16:
			if(pad != 0)
			{
				config::con_joy_1 = pad;
				input_con_1->setText(QString::number(pad));
			}

			config_con_1->setText("Configure");
			input_con_1->clearFocus();
			break;

		case 17:
			if(pad != 0)
			{
				config::con_joy_2 = pad;
				input_con_2->setText(QString::number(pad));
			}

			config_con_2->setText("Configure");
			input_con_2->clearFocus();
			break;
	}

	input_index = -1;
	QApplication::processEvents();
	grab_input = false;
}

/****** Close all pending input configuration ******/
void gen_settings::close_input()
{
	config_a->setText("Configure");
	config_b->setText("Configure");
	config_x->setText("Configure");
	config_y->setText("Configure");
	config_start->setText("Configure");
	config_select->setText("Configure");
	config_left->setText("Configure");
	config_right->setText("Configure");
	config_up->setText("Configure");
	config_down->setText("Configure");
	config_l->setText("Configure");
	config_r->setText("Configure");
	config_con_up->setText("Configure");
	config_con_down->setText("Configure");
	config_con_left->setText("Configure");
	config_con_right->setText("Configure");
	config_con_1->setText("Configure");
	config_con_2->setText("Configure");
	config_turbo->setText("Configure");
	config_mute->setText("Configure");
	config_camera->setText("Configure");
	config_swap_screen->setText("Configure");
	config_shift_screen->setText("Configure");

	input_index = -1;
	grab_input = false;

	//Additionally, controls combo to visible or invisible when switching tabs
	if(tabs->currentIndex() == 3) { controls_combo->setVisible(true); }
	else { controls_combo->setVisible(false); }
}

/****** Changes Qt widget layout - Switches between advanced control configuration mode ******/
void gen_settings::switch_control_layout()
{
	//Switch to Advanced Control layout
	if(controls_combo->currentIndex() == 1)
	{
		//Set all advanced control widgets to visible
		for(int x = 0; x < advanced_controls_layout->count(); x++)
		{
			advanced_controls_layout->itemAt(x)->widget()->setVisible(true);
		}

		//Set all standard control widgets to invisible
		for(int x = 0; x < controls_layout->count(); x++)
		{
			controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		//Set all hotkey control widgets to invisible
		for(int x = 0; x < hotkey_controls_layout->count(); x++)
		{
			hotkey_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		//Set all Battle Chip Gate control widgets to invisible
		for(int x = 0; x < bcg_controls_layout->count(); x++)
		{
			bcg_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		delete controls->layout();
		advanced_controls_layout->insertWidget(0, input_device_set);
		controls->setLayout(advanced_controls_layout);

		input_device_set->setVisible(true);
		input_device_set->setEnabled(true);
	}

	//Switch to Standard Control layout
	else if(controls_combo->currentIndex() == 0)
	{
		//Set all advanced control widgets to invisible
		for(int x = 0; x < advanced_controls_layout->count(); x++)
		{
			advanced_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		//Set all standard control widgets to visible
		for(int x = 0; x < controls_layout->count(); x++)
		{
			controls_layout->itemAt(x)->widget()->setVisible(true);
		}

		//Set all hotkey control widgets to invisible
		for(int x = 0; x < hotkey_controls_layout->count(); x++)
		{
			hotkey_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		//Set all Battle Chip Gate control widgets to invisible
		for(int x = 0; x < bcg_controls_layout->count(); x++)
		{
			bcg_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		delete controls->layout();
		controls_layout->insertWidget(0, input_device_set);
		controls->setLayout(controls_layout);

		input_device_set->setVisible(true);
		input_device_set->setEnabled(true);
	}

	//Switch to Hotkey layout
	else if(controls_combo->currentIndex() == 2)
	{
		//Set all advanced control widgets to invisible
		for(int x = 0; x < advanced_controls_layout->count(); x++)
		{
			advanced_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		//Set all standard control widgets to invisible
		for(int x = 0; x < controls_layout->count(); x++)
		{
			controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		//Set all hotkey control widgets to visible
		for(int x = 0; x < hotkey_controls_layout->count(); x++)
		{
			hotkey_controls_layout->itemAt(x)->widget()->setVisible(true);
		}

		//Set all Battle Chip Gate control widgets to invisible
		for(int x = 0; x < bcg_controls_layout->count(); x++)
		{
			bcg_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		delete controls->layout();
		hotkey_controls_layout->insertWidget(0, input_device_set);
		controls->setLayout(hotkey_controls_layout);

		input_device_set->setVisible(true);
		input_device_set->setEnabled(false);
		input_device->setCurrentIndex(0);
	}

	//Switch to Battle Chip Gate layout
	else if(controls_combo->currentIndex() == 3)
	{
		//Set all advanced control widgets to invisible
		for(int x = 0; x < advanced_controls_layout->count(); x++)
		{
			advanced_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		//Set all standard control widgets to invisible
		for(int x = 0; x < controls_layout->count(); x++)
		{
			controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		//Set all Battle Chip Gate control widgets to visible
		for(int x = 0; x < bcg_controls_layout->count(); x++)
		{
			bcg_controls_layout->itemAt(x)->widget()->setVisible(true);
		}

		//Set all hotkey control widgets to invisible
		for(int x = 0; x < hotkey_controls_layout->count(); x++)
		{
			hotkey_controls_layout->itemAt(x)->widget()->setVisible(false);
		}

		delete controls->layout();
		bcg_controls_layout->insertWidget(0, input_device_set);
		controls->setLayout(bcg_controls_layout);

		input_device_set->setVisible(true);
		input_device_set->setEnabled(false);
		input_device->setCurrentIndex(0);
	}

	//Rebuild old layout (was deleted above)
	switch(last_control_id)
	{
		case 0:
			controls_layout = new QVBoxLayout;
			controls_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			controls_layout->addWidget(input_a_set);
			controls_layout->addWidget(input_b_set);
			controls_layout->addWidget(input_x_set);
			controls_layout->addWidget(input_y_set);
			controls_layout->addWidget(input_start_set);
			controls_layout->addWidget(input_select_set);
			controls_layout->addWidget(input_left_set);
			controls_layout->addWidget(input_right_set);
			controls_layout->addWidget(input_up_set);
			controls_layout->addWidget(input_down_set);
			controls_layout->addWidget(input_l_set);
			controls_layout->addWidget(input_r_set);
			controls_layout->addWidget(dead_zone_set);
			break;

		case 1:
			advanced_controls_layout = new QVBoxLayout;
			advanced_controls_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			advanced_controls_layout->addWidget(rumble_set);
			advanced_controls_layout->addWidget(con_up_set);
			advanced_controls_layout->addWidget(con_down_set);
			advanced_controls_layout->addWidget(con_left_set);
			advanced_controls_layout->addWidget(con_right_set);
			advanced_controls_layout->addWidget(con_1_set);
			advanced_controls_layout->addWidget(con_2_set);
			break;

		case 2:
			hotkey_controls_layout = new QVBoxLayout;
			hotkey_controls_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			hotkey_controls_layout->addWidget(hotkey_turbo_set);
			hotkey_controls_layout->addWidget(hotkey_mute_set);
			hotkey_controls_layout->addWidget(hotkey_camera_set);
			hotkey_controls_layout->addWidget(hotkey_swap_screen_set);
			hotkey_controls_layout->addWidget(hotkey_shift_screen_set);
			break;

		case 3:
			bcg_controls_layout = new QVBoxLayout;
			bcg_controls_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			bcg_controls_layout->addWidget(bcg_gate_set);
			bcg_controls_layout->addWidget(bcg_chip_1_set);
			bcg_controls_layout->addWidget(bcg_chip_2_set);
			bcg_controls_layout->addWidget(bcg_chip_3_set);
			bcg_controls_layout->addWidget(bcg_chip_4_set);
			break;
	}

	last_control_id = controls_combo->currentIndex();
}

/****** Updates the settings window ******/
void gen_settings::paintEvent(QPaintEvent* event)
{
	gbc_bios_label->setMinimumWidth(dmg_bios_label->width());
	gba_bios_label->setMinimumWidth(dmg_bios_label->width());
	nds_firmware_label->setMinimumWidth(dmg_bios_label->width());
	manifest_label->setMinimumWidth(dmg_bios_label->width());
	dump_bg_label->setMinimumWidth(dmg_bios_label->width());
	dump_obj_label->setMinimumWidth(dmg_bios_label->width());
	screenshot_label->setMinimumWidth(dmg_bios_label->width());
	game_saves_label->setMinimumWidth(dmg_bios_label->width());
	cheats_path_label->setMinimumWidth(dmg_bios_label->width());
}

/****** Closes the settings window ******/
void gen_settings::closeEvent(QCloseEvent* event)
{
	//Close any on-going input configuration
	close_input();

	//Restore old text for netplay IP address if it hasn't been updated
	ip_address->setText(QString::fromStdString(config::netplay_client_ip));
}

/****** Closes the settings window - Used for the Close tab button ******/
void gen_settings::close_settings()
{
	//Close any on-going input configuration
	close_input();

	//Restore old text for netplay IP address if it hasn't been updated
	ip_address->setText(QString::fromStdString(config::netplay_client_ip));
}

/****** Handle keypress input ******/
void gen_settings::keyPressEvent(QKeyEvent* event)
{
	if(grab_input)
	{
		last_key = qtkey_to_sdlkey(event->key());

		switch(input_index)
		{
			case 0:
				if(last_key != -1)
				{
					config::gbe_key_a = last_key;
					input_a->setText(QString::number(last_key));
				}

				config_a->setText("Configure");
				input_a->clearFocus();
				break;

			case 1:
				if(last_key != -1)
				{
					config::gbe_key_b = last_key;
					input_b->setText(QString::number(last_key));
				}

				config_b->setText("Configure");
				input_b->clearFocus();
				break;

			case 2:
				if(last_key != -1)
				{
					config::gbe_key_x = last_key;
					input_x->setText(QString::number(last_key));
				}

				config_x->setText("Configure");
				input_x->clearFocus();
				break;

			case 3:
				if(last_key != -1)
				{
					config::gbe_key_y = last_key;
					input_y->setText(QString::number(last_key));
				}

				config_y->setText("Configure");
				input_y->clearFocus();
				break;

			case 4:
				if(last_key != -1)
				{
					config::gbe_key_start = last_key;
					input_start->setText(QString::number(last_key));
				}

				config_start->setText("Configure");
				input_start->clearFocus();
				break;

			case 5:
				if(last_key != -1)
				{
					config::gbe_key_select = last_key;
					input_select->setText(QString::number(last_key));
				}

				config_select->setText("Configure");
				input_select->clearFocus();
				break;

			case 6:
				if(last_key != -1)
				{
					config::gbe_key_left = last_key;
					input_left->setText(QString::number(last_key));
				}

				config_left->setText("Configure");
				input_left->clearFocus();
				break;

			case 7:
				if(last_key != -1)
				{
					config::gbe_key_right = last_key;
					input_right->setText(QString::number(last_key));
				}

				config_right->setText("Configure");
				input_right->clearFocus();
				break;

			case 8:
				if(last_key != -1)
				{
					config::gbe_key_up = last_key;
					input_up->setText(QString::number(last_key));
				}

				config_up->setText("Configure");
				input_up->clearFocus();
				break;

			case 9:
				if(last_key != -1)
				{
					config::gbe_key_down = last_key;
					input_down->setText(QString::number(last_key));
				}

				config_down->setText("Configure");
				input_down->clearFocus();
				break;

			case 10: 
				if(last_key != -1)
				{
					config::gbe_key_l_trigger = last_key;
					input_l->setText(QString::number(last_key));
				}

				config_l->setText("Configure");
				input_l->clearFocus();
				break;

			case 11:
				if(last_key != -1)
				{
					config::gbe_key_r_trigger = last_key;
					input_r->setText(QString::number(last_key));
				}

				config_r->setText("Configure");
				input_r->clearFocus();
				break;

			case 12:
				if(last_key != -1)
				{
					config::con_key_up = last_key;
					input_con_up->setText(QString::number(last_key));
				}

				config_con_up->setText("Configure");
				input_con_up->clearFocus();
				break;

			case 13:
				if(last_key != -1)
				{
					config::con_key_down = last_key;
					input_con_down->setText(QString::number(last_key));
				}

				config_con_down->setText("Configure");
				input_con_down->clearFocus();
				break;

			case 14:
				if(last_key != -1)
				{
					config::con_key_left = last_key;
					input_con_left->setText(QString::number(last_key));
				}

				config_con_left->setText("Configure");
				input_con_left->clearFocus();
				break;

			case 15:
				if(last_key != -1)
				{
					config::con_key_right = last_key;
					input_con_right->setText(QString::number(last_key));
				}

				config_con_right->setText("Configure");
				input_con_right->clearFocus();
				break;

			case 16:
				if(last_key != -1)
				{
					config::con_key_1 = last_key;
					input_con_1->setText(QString::number(last_key));
				}

				config_con_1->setText("Configure");
				input_con_1->clearFocus();
				break;

			case 17:
				if(last_key != -1)
				{
					config::con_key_2 = last_key;
					input_con_2->setText(QString::number(last_key));
				}

				config_con_2->setText("Configure");
				input_con_2->clearFocus();
				break;

			case 18:
				if(last_key != -1)
				{
					config::hotkey_turbo = last_key;
					input_turbo->setText(QString::number(last_key));
				}

				config_turbo->setText("Configure");
				input_turbo->clearFocus();
				break;

			case 19:
				if(last_key != -1)
				{
					config::hotkey_mute = last_key;
					input_mute->setText(QString::number(last_key));
				}

				config_mute->setText("Configure");
				input_mute->clearFocus();
				break;

			case 20:
				if(last_key != -1)
				{
					config::hotkey_camera = last_key;
					input_camera->setText(QString::number(last_key));
				}

				config_camera->setText("Configure");
				input_camera->clearFocus();
				break;

			case 21:
				if(last_key != -1)
				{
					config::hotkey_swap_screen = last_key;
					input_swap_screen->setText(QString::number(last_key));
				}

				config_swap_screen->setText("Configure");
				input_swap_screen->clearFocus();
				break;

			case 22:
				if(last_key != -1)
				{
					config::hotkey_shift_screen = last_key;
					input_shift_screen->setText(QString::number(last_key));
				}

				config_shift_screen->setText("Configure");
				input_shift_screen->clearFocus();
				break;

		}

		grab_input = false;
		input_index = -1;
	}
}

/****** Event filter for settings window ******/
bool gen_settings::eventFilter(QObject* target, QEvent* event)
{
	//Filter all keypresses, pass them along to input configuration if necessary
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
		keyPressEvent(key_event);
	}

	//Check mouse click for input device
	else if((target == input_device) && (event->type() == QEvent::MouseButtonPress))
	{
		rebuild_input_index();
	}

	return QDialog::eventFilter(target, event);
}

/****** Selects folder ******/
void gen_settings::select_folder() { data_folder->finish = true; }

/****** Rejectss folder ******/
void gen_settings::reject_folder()
{
	data_folder->finish = true;
	data_folder->setDirectory(data_folder->last_path);
}
