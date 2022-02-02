# Patched hpsa DKMS package

Downloads and automatically patches hpsa driver from stable linux kernel tree.

# Usage

Run `install.sh`

# manual installation

Run `./patch.sh` to download and patch hpsa driver. This script also takes an
optional argument VERSION that sets what kernel version to patch.

Make sure that you have your current kernel headers and dkms package installed.

    sudo apt install dkms linux-headers-$(uname -r)

Add the dkms module to the tree

    sudo dkms add ./

And then you can install it.

    sudo dkms install --force hpsa-dkms/1.0

When running in a chroot you have to manually set the kernel version

    dkms install --force -k 4.19.0-9-amd64 hpsa-dkms/1.0

After that is done, unload the old hpsa driver and insert the new one

    sudo modprobe -r hpsa
    sudo modprobe hpsa hpsa_use_nvram_hba_flag=1


