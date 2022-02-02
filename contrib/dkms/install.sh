#!/bin/bash
echo -e "This script recompiles and reinstalls the hpsahba dkms module after a kernel update\n"
echo ""

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

#check if launched from right directory
if [[ $(pwd) != *"/contrib/dkms" ]]; then
	echo -e "you need to run this script from within the contrib/dkms directory!\n"
	echo "please cd and run this script again"
	exit
fi

./patch.sh

apt update

#check if proxmox kernel is installed
if [[ $(uname -r) == *"pve" ]]; then
	echo "installing proxmox kernel headers"
	apt install -y pve-headers-$(uname -r)
else
	echo "installing default linux-headers"
	apt install -y linux-headers-$(uname -r)
fi

echo "removing old module"
#"force" remove as dkms remove won't work
rm -r /var/lib/dkms/hpsa-dkms

dkms add ./
dkms install --force hpsa-dkms/1.0

echo "reloading hpsa with hpsa_use_nvram_hba_flag=1"
modprobe -r hpsa
modprobe hpsa hpsa_use_nvram_hba_flag=1

echo "done."
echo "You may want to reboot or rescan the SCSI bus"
