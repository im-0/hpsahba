# NAME

**hpsahba** - tool to enable/disable HBA mode on some HP Smart Array
controllers.

# SYNOPSIS

* hpsahba -h
* hpsahba -v
* hpsahba -i /dev/sgN
* hpsahba -E /dev/sgN
* hpsahba -d /dev/sgN

# DESCRIPTION

**CAUTION: This tool will destroy your data and may damage your hardware!**

**hpsahba** is able to enable or disable HBA mode on some HP Smart Array
controllers on which regular tools, like 'ssacli', reports HBA mode as not
supported.

When enabling or disabling HBA mode, RAID controller changes its internal
configuration immediately, without requiring reboot. Any existing data
may be lost after changing the HBA mode, so be extremely careful.

Some servers may not be able to boot from disks attached to controller in
HBA mode. Move the /boot partition and the bootloader out of disks attached to
controller onto USB flash driver or SD card to circumvent this limitation.

When HBA mode is enabled, controller usually prints some diagnostic message
during boot, like *"Hardware RAID support is disabled via controller NVRAM
configuration settings"* (example from P410i). Attempts to configure RAID
arrays in this mode are expected to fail.

## Options

* **hpsahba -h**

  Show help message and exit.

* **hpsahba -v**

  Show version and exit.

* **hpsahba -i DEVICE_PATH**

  Show some information about device. This Includes HBA mode support bit
  (supported/not supported) and current state of HBA mode (enabled/disabled).
  It is recommended to run this before trying to enable or disable HBA mode.

* **hpsahba -E DEVICE_PATH**

  Enable HBA mode.

* **hpsahba -d DEVICE_PATH**

  Disable HBA mode.

## Kernel driver support

**hpsahba** itself is able to work on any modern Linux system.

However, to get system actually see and use disks in HBA mode, few kernel
patches may be required:
<https://github.com/im-0/hpsahba/tree/master/kernel>.

This functionality is disabled by default. To enable, load module hpsa with
parameter hpsa_use_nvram_hba_flag set to "1". Or set it in the kernel command
line: "hpsa.hpsa_use_nvram_hba_flag=1".

Patchset changelog:
* V1 -> V2:
  * Device visibility change properly detected if device is both updated
    and masked/unmasked in the same time.

This will never be upstreamed and officially supported (for P410), see
the email from Don Brace: <https://lkml.org/lkml/2018/12/17/618>. So use
at your own risk.

## Supported hardware

Tested on following hardware so far:

* HP Smart Array P410i (PCI ID: 103c:3245, board ID: 0x3245103c,
firmware: 6.64)
* Hewlett-Packard Company Smart Array G6 controllers / P410 (PCI ID:
103c:323a, board ID: 0x3243103c, firmware: 6.64)

(open an issue or a pull request if you successfully used this tool on
other controllers)

## Additional links

* <https://support.hpe.com/hpsc/swd/public/detail?swItemId=MTX_0b76aec489764aea9802a6d27b>

  Archive containing saupdate.efi - EFI binary for ia64 architecture which
  implements the same functionality for P410i. Seems not usable anywhere except
  of old HP Integrity servers.

# BUGS

See issues on GitHub: <https://github.com/im-0/hpsahba/issues>.

# AUTHOR

**hpsahba** was written by Ivan Mironov \<mironov.ivan@gmail.com>
