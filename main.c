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

/* Force assert() to work. */
#ifdef NDEBUG
#	undef NDEBUG
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>

#include <endian.h>
#include <sys/ioctl.h>
#include <linux/cciss_ioctl.h>

#include "hpsa.h"

static const char *const hpsahba_version = "0.0.0";

__attribute__((format(printf, 1, 2)))
__attribute__((noreturn))
static void really_die(const char *format, ...)
{	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);

	exit(1);
}

#define die(format, ...) \
	really_die("FATAL ERROR: " format "\n", ##__VA_ARGS__)
#define die_errno(format, ...) \
	die(format ": %d %s", ##__VA_ARGS__, errno, strerror(errno))
#define die_dev(path, format, ...) \
	die("%s: " format, path, ##__VA_ARGS__)
#define die_dev_errno(path, format, ...) \
	die_dev(path, format ": %d %s", ##__VA_ARGS__, errno, strerror(errno))

static void print_help(const char *exe_name)
{
	fprintf(stderr,
		"hpsahba version %s, Copyright (C) 2018  "
			"Ivan Mironov <mironov.ivan@gmail.com>\n"
		"\n"
		"Usage:\n"
		"\t%s -h\n"
		"\t%s -v\n"
		"\t%s -i /dev/sgN\n"
		"\t%s -E /dev/sgN\n"
		"\t%s -d /dev/sgN\n"
		"\n"
		"Options:\n"
		"\t-h\n"
		"\t\tPrint this help message and exit.\n"
		"\n"
		"\t-v\n"
		"\t\tPrint version number and exit.\n"
		"\n"
		"\t-i <device path>\n"
		"\t\tGet information about HP Smart Array controller.\n"
		"\n"
		"\t-E <device path>\n"
		"\t\tEnable HBA mode on controller.\n"
		"\n"
		"\t-d <device path>\n"
		"\t\tDisable HBA mode on controller.\n",
		hpsahba_version,
		exe_name, exe_name, exe_name, exe_name, exe_name);
}

static void print_version()
{
	printf("%s\n", hpsahba_version);
}

#define TERM_FMT_RESET "0"
#define TERM_FMT_BOLD "1"
#define TERM_FMT_BLINK "5"

#define TERM_FG_BLACK "30"
#define TERM_FG_YELLOW "33"

#define TERM_BG_RED "41"

#define TERM_ATTR_START "\033["
#define TERM_ATTR_END "m"
#define TERM_ATTR_RESET TERM_ATTR_START TERM_FMT_RESET TERM_ATTR_END

static void print_caution_sign(bool no_color)
{
	if (no_color) {
		fputs(" // CAUTION // ", stderr);
	} else {
		fputs(
			TERM_ATTR_START TERM_FMT_RESET ";" TERM_BG_RED ";"
				TERM_FMT_BOLD ";" TERM_FG_YELLOW TERM_ATTR_END
			" // "
			TERM_ATTR_START TERM_FMT_RESET ";" TERM_BG_RED ";"
				TERM_FMT_BLINK ";" TERM_FG_BLACK TERM_ATTR_END
			" CAUTION "
			TERM_ATTR_START TERM_FMT_RESET ";" TERM_BG_RED ";"
				TERM_FMT_BOLD ";" TERM_FG_YELLOW TERM_ATTR_END
			" // "
			TERM_ATTR_RESET,
			stderr);
	}
}

static void ask_user_confirmation()
{
	char buf[5];

	for (int i = 0; i < 5; i++)
		print_caution_sign((i % 2) || (isatty(2) != 1));
	fprintf(stderr,"\n"
		"\tHBA MODE CHANGE WILL DESTROY YOU DATA!\n"
		"\tHBA MODE CHANGE MAY DAMAGE YOUR HARDWARE!\n"
		"Type uppercase \"yes\" to accept the risks and continue: ");
	if (fgets(buf, sizeof(buf), stdin) == NULL)
		die_errno("fgets() failed to read user confirmation");
	if (strcmp(buf, "YES\n"))
		die("Cancelled by user");
}

static int open_dev(const char *path)
{
	int fd = open(path, O_RDWR);
	if (fd == -1) {
		die_dev_errno(path, "Unable to open device r/w");
	}
	return fd;
}

static void close_dev(const char *path, int fd)
{
	if (close(fd))
		die_dev_errno(path, "close() failed");
}

static void set_cdb_buf_size(uint8_t cdb[], size_t size)
{
	assert(size <= UINT16_MAX);
	cdb[7] = size >> 8;
	cdb[8] = size & 0XFF;
}

static void set_cmd_buf(IOCTL_Command_struct *cmd, void *buf,
	size_t size)
{
	set_cdb_buf_size(cmd->Request.CDB, size);
	cmd->buf_size = size;
	cmd->buf = buf;
}

static void fill_cmd(IOCTL_Command_struct *cmd, uint8_t cmd_num, void *buf,
	size_t size)
{
	int direction_write = 0;

	switch (cmd_num) {
	case BMIC_IDENTIFY_CONTROLLER:
	case BMIC_SENSE_CONTROLLER_PARAMETERS:
		/* direction_write = 0; */
		break;
	case BMIC_SET_CONTROLLER_PARAMETERS:
		direction_write = 1;
		break;
	default:
		/* Should never happen. */
		assert(0);
	}
	cmd->Request.CDB[6] = cmd_num;

	if (direction_write) {
		cmd->Request.CDB[0] = BMIC_WRITE;
		cmd->Request.Type.Direction = XFER_WRITE;
	} else {
		cmd->Request.CDB[0] = BMIC_READ;
		cmd->Request.Type.Direction = XFER_READ;
	}

	set_cmd_buf(cmd, buf, size);

	cmd->Request.CDBLen = 10;

	cmd->Request.Type.Type = TYPE_CMD;
	cmd->Request.Type.Attribute = ATTR_SIMPLE;
	cmd->Request.Timeout = 0;
}

static void print_cmd_error(const ErrorInfo_struct *info)
{
	size_t sense_len = info->SenseLen;

	fprintf(stderr,
		"HPSA SCSI error info:\n"
		"\tScsiStatus: 0x%02x\n"
		"\tSenseLen: %u\n"
		"\tCommandStatus: 0x%04x\n"
		"\tResidualCnt: 0x%08x\n"
		"\tSenseInfo:",
		info->ScsiStatus,
		info->SenseLen,
		info->CommandStatus,
		info->ResidualCnt);

	if (sense_len) {
		if (sense_len > sizeof(info->SenseInfo))
			sense_len = sizeof(info->SenseInfo);
		for (size_t i = 0; i < sense_len; i++)
			fprintf(stderr, " 0x%02x", info->SenseInfo[i]);
	} else {
		fputs(" <none>", stderr);
	}
	fputc('\n', stderr);
}

static void really_exec_cmd(const char *path, int fd, uint8_t cmd_num,
	const char *cmd_name, void *buf, size_t size)
{
	int rc;
	IOCTL_Command_struct cmd = {{{0}}, {0}, {0}, 0, 0};

	fill_cmd(&cmd, cmd_num, buf, size);
	rc = ioctl(fd, CCISS_PASSTHRU, &cmd);
	if (rc)
		die_dev_errno(path,
			"ioctl(CCISS_PASSTHRU) failed with command "
			"%s, rc == %d", cmd_name, rc);

	if (cmd.error_info.CommandStatus) {
		print_cmd_error(&cmd.error_info);
		die_dev(path, "Command %s failed", cmd_name);
	}
}

#define exec_cmd(path, fd, cmd, buf, size) \
	really_exec_cmd(path, fd, cmd, #cmd, buf, size)

static void identify_controller(const char *path, int fd,
	struct bmic_identify_controller *controller_id)
{
	exec_cmd(path, fd, BMIC_IDENTIFY_CONTROLLER, controller_id,
		sizeof(*controller_id));
}

static int is_hba_mode_supported(
	const struct bmic_identify_controller *controller_id)
{
	uint32_t f = le32toh(controller_id->yet_more_controller_flags);
	return (f & YET_MORE_CTLR_FLAG_HBA_MODE_SUPP) ? 1 : 0;
}

static void sense_controller_parameters(const char *path, int fd,
	struct bmic_controller_parameters *controller_params)
{
	exec_cmd(path, fd, BMIC_SENSE_CONTROLLER_PARAMETERS, controller_params,
		sizeof(*controller_params));
}

static void set_controller_parameters(const char *path, int fd,
	struct bmic_controller_parameters *controller_params)
{
	exec_cmd(path, fd, BMIC_SET_CONTROLLER_PARAMETERS, controller_params,
		sizeof(*controller_params));
}

static int is_hba_mode_enabled(
	const struct bmic_controller_parameters *controller_params)
{
	uint8_t f = controller_params->nvram_flags;
	return (f & NVRAM_FLAG_HBA_MODE_ENABLED) ? 1 : 0;
}

static void fill_hba_mode(
	struct bmic_controller_parameters *controller_params, int enabled)
{
	if (enabled)
		controller_params->nvram_flags |= NVRAM_FLAG_HBA_MODE_ENABLED;
	else
		controller_params->nvram_flags &= ~NVRAM_FLAG_HBA_MODE_ENABLED;
}

static const char *trim(char *str)
{
	/* Remove leading spaces. */
	while (isspace(*str))
		str++;

	/* Remove trailing spaces. */
	for (size_t i = strlen(str); i && isspace(str[i - 1]); i--)
		str[i - 1] = '\0';

	return str;
}

static void print_info_str_buf(const char *var_name, const char *str_buf,
	size_t max_str_len)
{
	/* Ensure that string is null-terminated. */
	char str[MAX_STR_BUF_LEN + 1] = {0};
	strncpy(str, str_buf, max_str_len);
	printf("%s='%s'\n", var_name, trim(str));
}

static void print_info_fw_rev(const char *var_name, const char *rev_buf)
{
	print_info_str_buf(var_name, rev_buf, FIRMWARE_REV_LEN);
}

static void print_info(const char *path, int fd)
{
	struct bmic_identify_controller controller_id = {0};
	struct bmic_controller_parameters controller_params = {0};
	int hba_mode_supported;
	int hba_mode_enabled = 0;

	identify_controller(path, fd, &controller_id);
	sense_controller_parameters(path, fd, &controller_params);

	print_info_str_buf("VENDOR_ID",
		controller_id.vendor_id, VENDOR_ID_LEN);
	print_info_str_buf("PRODUCT_ID",
		controller_id.product_id, PRODUCT_ID_LEN);
	printf("BOARD_ID='0x%08x'\n",
		le32toh(controller_id.board_id));
	print_info_str_buf("SOFTWARE_NAME",
		controller_params.software_name, SOFTWARE_NAME_LEN);
	print_info_str_buf("HARDWARE_NAME",
		controller_params.hardware_name, HARDWARE_NAME_LEN);
	print_info_fw_rev("RUNNING_FIRM_REV",
		controller_id.running_firm_rev);
	print_info_fw_rev("ROM_FIRM_REV",
		controller_id.rom_firm_rev);
	print_info_fw_rev("REC_ROM_INACTIVE_REV",
		controller_id.rec_rom_inactive_rev);
	printf("YET_MORE_CONTROLLER_FLAGS='0x%08x'\n",
		le32toh(controller_id.yet_more_controller_flags));
	printf("NVRAM_FLAGS='0x%02x'\n",
		controller_params.nvram_flags);

	hba_mode_supported = is_hba_mode_supported(&controller_id);
	if (hba_mode_supported)
		hba_mode_enabled = is_hba_mode_enabled(&controller_params);
	printf("HBA_MODE_SUPPORTED=%d\n",
		hba_mode_supported);
	printf("HBA_MODE_ENABLED=%d\n",
		hba_mode_enabled);
}

static void verify_hba_mode(const char *path, int fd, int should_be_enabled)
{
	struct bmic_controller_parameters controller_params = {0};

	sense_controller_parameters(path, fd, &controller_params);
	if (should_be_enabled) {
		if (!is_hba_mode_enabled(&controller_params))
			die_dev(path,
				"HBA mode enable failed, "
				"nvram_flags == 0x%02x",
				controller_params.nvram_flags);
	} else {
		if (is_hba_mode_enabled(&controller_params))
			die_dev(path,
				"HBA mode disable failed, "
				"nvram_flags == 0x%02x",
				controller_params.nvram_flags);
	}
}

static void rescan_scsi(const char *path, int fd)
{
	int rc = ioctl(fd, CCISS_REGNEWD);
	if (rc)
		die_dev_errno(path,
			"ioctl(CCISS_REGNEWD) failed, rc == %d", rc);
}

static void change_hba_mode(const char *path, int fd, int enabled)
{
	struct bmic_identify_controller controller_id = {0};
	struct bmic_controller_parameters controller_params = {0};

	ask_user_confirmation();

	identify_controller(path, fd, &controller_id);
	sense_controller_parameters(path, fd, &controller_params);

	if (!is_hba_mode_supported(&controller_id))
		die_dev(path, "HBA mode is not supported on this controller");

	fill_hba_mode(&controller_params, enabled);
	set_controller_parameters(path, fd, &controller_params);

	verify_hba_mode(path, fd, enabled);

	rescan_scsi(path, fd);
}

enum cli_action {
	ACTION_HELP,
	ACTION_VERSION,
	ACTION_INFO,
	ACTION_ENABLE,
	ACTION_DISABLE,

	ACTION_UNKNOWN,
};

static void set_action(enum cli_action *cur_action, enum cli_action new_action)
{
	if (*cur_action != ACTION_UNKNOWN)
		die("More than one option selected, try running with -h");
	*cur_action = new_action;
}

int main(int argc, char *argv[])
{
	int opt = 0;
	enum cli_action action = ACTION_UNKNOWN;
	const char *path = NULL;
	int fd = -1;

	opterr = 0;
	while (opt != -1) {
		opt = getopt(argc, argv, ":hvi:E:d:");

		switch (opt) {
		case -1:
			/* No more options. */
			break;
		case 'h':
			set_action(&action, ACTION_HELP);
			break;
		case 'v':
			set_action(&action, ACTION_VERSION);
			break;
		case 'i':
			set_action(&action, ACTION_INFO);
			path = optarg;
			break;
		case 'E':
			set_action(&action, ACTION_ENABLE);
			path = optarg;
			break;
		case 'd':
			set_action(&action, ACTION_DISABLE);
			path = optarg;
			break;
		case '?':
			die("Unknown command line option: '%c', try running "
				"with -h",
				optopt);
		case ':':
			die("Missing argument for option '%c', try running "
				"with -h",
				optopt);
		default:
			die("getopt() returned unexpected value: %d", opt);
		}
	}

	if (argc > optind)
		die("Invalid argument in command line, try running with -h");

	if (path != NULL)
		fd = open_dev(path);

	switch (action) {
	case ACTION_HELP:
		print_help(argv[0]);
		break;
	case ACTION_VERSION:
		print_version();
		break;
	case ACTION_INFO:
		print_info(path, fd);
		break;
	case ACTION_ENABLE:
		change_hba_mode(path, fd, 1);
		break;
	case ACTION_DISABLE:
		change_hba_mode(path, fd, 0);
		break;
	default:
		die("No option selected, try running with -h");
	}

	if (fd != -1)
		close_dev(path, fd);

	return 0;
}
