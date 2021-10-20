# Patched hpsa DKMS package

Downloads and automatically patches hpsa driver from stable linux kernel tree.

# Usage

Run `./patch.sh` to download and patch hpsa driver. This script also takes an
optional argument VERSION that sets what kernel version to patch.

Make sure that you have your current kernel headers and dkms package installed.

    sudo apt install dkms linux-headers-$(uname -r)
    
# Configuring the Makefile
By default, the hpsa DKMS Makefile has the module location set to /usr/lib/modules as of 10-20-21. 
This default location will not work for some flavors of Linux, e.g., Ubuntu 20.04 (c.f., Issue #15).

For example: For Ubuntu 20.04, the location should be switched to /lib/modules.

In such a case, you would *change*:
 KDIR := /usr/lib/modules/$(KRELEASE)/build
 *to* 
 KDIR := /lib/modules/$(KRELEASE)/build

Please confirm that the hpsa dkms Makefile (located as of 10-20-21 at /contrib/dkms) has the correct location of your kernel modules directory written on or around line 9 of said Makefile.

If you need help finding your kernel modules directory, please follow a tutorial on doing so before proceeding. Once you have confirmed that the *dkms* Makefile (not to be confused with the Makefile for hpsahba located in the project root directory) has the correct location of your kernel modules directory, proceed.
      
# Adding the dkms module

Add the dkms module to the tree

    sudo dkms add ./

And then you can install it.

    sudo dkms install --force hpsa-dkms/1.0

When running in a chroot you have to manually set the kernel version

    dkms install --force -k 4.19.0-9-amd64 hpsa-dkms/1.0

After that is done, unload the old hpsa driver and insert the new one

    sudo modprobe -r hpsa
    sudo modprobe hpsa hpsa_use_nvram_hba_flag=1
