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
  (supported/not supported), current state of HBA mode (enabled/disabled) and
  whether the controller unmasks physical drives (HBA_UNMASK_DRIVES).
  It is recommended to run this before trying to enable or disable HBA mode.

* **hpsahba -E DEVICE_PATH**

  Enable HBA mode: set NVRAM flag bits 3 (HBA mode) and 2 (unmask physical
  drives), as the controller's own "set hba_mode on" command does.

* **hpsahba -d DEVICE_PATH**

  Disable HBA mode: clear both bits.

## Kernel driver support

**hpsahba** itself is able to work on any modern Linux system.

**hpsahba -E** sets NVRAM flag bit 2 together with bit 3. With it the
controller firmware stops masking physical drives in its LUN reports, and the
**stock, unpatched hpsa driver** exposes them as ordinary SCSI disks. No kernel
patches, DKMS package or module parameter are needed for that. Tested on a P410
(board ID 0x3243103c, firmware 6.64) with the unmodified hpsa 3.4.20-200 of a
Proxmox 7.0 kernel: all four SATA drives appear as "PHYS DRV" direct-access
devices across reboots and carry Btrfs RAID1 filesystems.

Bit 2 comes from the firmware: P410 firmware 6.64 has the diagnostic command
"set hba_mode [off | on]", where "on" (printed as "ON (unconfigured drives also
unmasked)") sets bits 2 and 3 and "off" clears both. Earlier hpsahba builds
set bit 3 only, which is why the drives stayed masked and the kernel patches
below were needed. A controller switched with an older build has bit 3 only;
running **hpsahba -E** again sets bit 2 as well. That path is untested, and a
reboot may be needed before the drives appear.

Other controllers and firmware versions are untested with bit 2. If a
controller keeps masking its drives, the previous approach still works: the
kernel patches in <https://github.com/im-0/hpsahba/tree/master/kernel>, loaded
with the hpsa module parameter hpsa_use_nvram_hba_flag set to "1" (or
"hpsa.hpsa_use_nvram_hba_flag=1" on the kernel command line).

Patchset changelog:

* V1 -> V2:
  * Device visibility change properly detected if device is both updated
    and masked/unmasked in the same time.

This will never be upstreamed and officially supported (for P410), see
the email from Don Brace: <https://lkml.org/lkml/2018/12/17/618>. So use
at your own risk.

You can use DKMS package in
<https://github.com/im-0/hpsahba/tree/master/contrib/dkms> to patch hpsa driver
in a compiled kernel.

## Supported hardware

Tested on following hardware so far:

* HP Smart Array P410i (PCI ID: 103c:3245, board ID: 0x3245103c,
firmware: 6.64)
* HP Smart Array P812 (PCI ID: 103c:3249, board ID: 0x3249103c,
firmware: 6.64)
* Hewlett-Packard Company Smart Array G6 controllers / P410 (PCI ID:
103c:323a, board ID: 0x3243103c, firmware: 6.64; also without kernel patches,
see above)
* Hewlett-Packard Company Smart Array G6 controllers / P212 (PCI ID:
103c:323a, board ID: 0x3241103c, firmware: 6.64)

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
