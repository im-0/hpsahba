/*
 * Tool to enable/disable HBA mode on some HP Smart Array controllers.
 * Copyright (C) 2018  Ivan Mironov <mironov.ivan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef HPSAHBA_HPSA_H
#define HPSAHBA_HPSA_H

#include <stdint.h>

/* Just to annotate fields. It looks like all ints are little-endian. */
#define u8 uint8_t
#define u16le uint16_t
#define u32le uint32_t

/*
 * Most of information about various structures and constants was borrowed
 * from $KERNEL_SRC/drivers/scsi/hpsa* and cciss_vol_status tool.
 * Some parts was guessed by author.
 */

#pragma pack(1)

#define BMIC_READ 0x26
#define BMIC_WRITE 0x27

#define BMIC_IDENTIFY_CONTROLLER 0x11
#define BMIC_SET_CONTROLLER_PARAMETERS 0x63
#define BMIC_SENSE_CONTROLLER_PARAMETERS 0x64

#define FIRMWARE_REV_LEN 4
#define VENDOR_ID_LEN 8
#define PRODUCT_ID_LEN 16
#define SOFTWARE_NAME_LEN 64
#define HARDWARE_NAME_LEN 32
#define MAX_STR_BUF_LEN SOFTWARE_NAME_LEN

struct bmic_identify_controller {
	u8 num_logical_drives;
	u32le signature;
	char running_firm_rev[FIRMWARE_REV_LEN];
	char rom_firm_rev[FIRMWARE_REV_LEN];
	u8 hardware_rev;
	u8 reserved_1[4];
	u32le drive_present_bit_map;
	u32le external_drive_bit_map;
	u32le board_id;
	u8 reserved_2;
	u32le non_disk_map;
	u8 reserved_3[5];
	u8 marketing_revision;
	u8 controller_flags;
	u8 host_flags;
	u8 expand_disable_code;
	u8 scsi_chip_count;
	u8 reserved_4[4];
	u32le ctlr_clock;
	u8 drives_per_scsi_bus;
	u16le big_drive_present_map[8];
	u16le big_ext_drive_map[8];
	u16le big_non_disk_map[8];

	/* used for FW debugging */
	u16le task_flags;
	/* Bitmap used for ICL between controllers */
	u8 icl_bus_map;
	/* See REDUNDANT MODE VALUES */
	u8 redund_ctlr_modes_support;
	/* See REDUNDANT MODE VALUES */
	u8 curr_redund_ctlr_mode;
	/* See REDUNDANT STATUS FLAG */
	u8 redund_ctlr_status;
	/* See REDUNDANT FAILURE VALUES */
	u8 redund_op_failure_code;
	u8 unsupported_nile_bus;
	u8 host_i2c_autorev;
	u8 cpld_revision;
	u8 fibre_chip_count;
	u8 daughterboard_type;
	u8 reserved_5[2];

	u8 access_module_status;
	u8 features_supported[12];
	/* Recovery ROM inactive f/w revision */
	char rec_rom_inactive_rev[FIRMWARE_REV_LEN];
	/* Recovery ROM flags */
	u8 rec_rom_flags;
	u8 pci_to_pci_bridge_status;
	/* Reserved for future use */
	u8 reserved_6[4];
	/* Percent of memory allocated to write cache */
	u8 percent_write_cache;
	/* Total cache board size */
	u16le daughter_board_cache_size;
	/* Number of cache batteries */
	u8 cache_battery_count;
	/* Total size (MB) of atttached memory */
	u16le total_memory_size;
	/* Additional controller flags byte */
	u8 more_controller_flags;
	/* 2nd byte of 3 byte autorev field */
	u8 x_board_host_i2c_autorev;
	/* BBWC PIC revision */
	u8 battery_pic_rev;
	/* DDFF update engine version */
	u8 ddff_version[4];
	/* Maximum logical units supported */
	u16le max_logical_units;
	/* Big num configured logical units */
	u16le ext_logical_unit_count;
	/* Maximum physical devices supported */
	u16le max_physical_devices;
	/* Max physical drive per logical unit */
	u16le max_phy_drv_per_logical_unit;
	/* Number of attached enclosures */
	u8 enclosure_count;
	/* Number of expanders detected */
	u8 expander_count;
	/* Offset to extended drive present map*/
	u16le offset_to_edp_bitmap;
	/* Offset to extended external drive present map */
	u16le offset_to_eedp_bitmap;
	/* Offset to extended non-disk map */
	u16le offset_to_end_bitmap;
	/* Internal port status bytes */
	u8 internal_port_status[8];
	/* External port status bytes */
	u8 external_port_status[8];
	/* Yet More Controller flags  */
	u32le yet_more_controller_flags;
	/* Last lockup code */
	u8 last_lockup;
	/* PCI slot according to option ROM*/
	u8 pci_slot;
	/* Build number */
	u16le build_num;
	/* Maximum safe full stripe size */
	u32le max_safe_full_stripe_size;
	/* Total structure length */
	u32le total_length;
	/* Vendor ID */
	char vendor_id[VENDOR_ID_LEN];
	/* Product ID */
	char product_id[PRODUCT_ID_LEN];
	u8 reserved_7[288];
};

#define YET_MORE_CTLR_FLAG_HBA_MODE_SUPP (1 << 25)

struct bmic_controller_parameters {
	u8 led_flags;
	u8 enable_command_list_verification;
	u8 backed_out_write_drives;
	u16le stripes_for_parity;
	u8 parity_distribution_mode_flags;
	u16le max_driver_requests;
	u16le elevator_trend_count;
	u8 disable_elevator;
	u8 force_scan_complete;
	u8 scsi_transfer_mode;
	u8 force_narrow;
	u8 rebuild_priority;
	u8 expand_priority;
	u8 host_sdb_asic_fix;
	u8 pdpi_burst_from_host_disabled;
	char software_name[SOFTWARE_NAME_LEN];
	char hardware_name[HARDWARE_NAME_LEN];
	u8 bridge_revision;
	u8 snapshot_priority;
	u32le os_specific;
	u8 post_prompt_timeout;
	u8 automatic_drive_slamming;
	u8 reserved_1;
	u8 nvram_flags;
	u8 cache_nvram_flags;
	u8 drive_config_flags;
	u16le reserved_2;
	u8 temp_warning_level;
	u8 temp_shutdown_level;
	u8 temp_condition_reset;
	u8 max_coalesce_commands;
	u32le max_coalesce_delay;
	u8 orca_password[4];
	u8 access_id[16];
	u8 reserved[356];
};

#define NVRAM_FLAG_HBA_MODE_ENABLED (1 << 3)

#pragma pack()
#endif /* HPSAHBA_HPSA_H */
