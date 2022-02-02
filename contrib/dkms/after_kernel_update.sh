#!/bin/bash
echo "This script recompiles and reinstalls dkms module after a kernel update"

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

apt update

if [[ $(uname -r) == *"pve" ]]; then
	echo "installing proxmox kernel headers"
	apt install -y pve-headers-$(uname -r)
else
	echo "installing default debian kernel headers"
	apt install -y linux-headers-$(uname -r)
fi

echo "removing old module"
rm -r /var/lib/dkms/hpsa-dkms
#cd /root/hpsahba/contrib/dkms

dkms add ./
dkms install --force hpsa-dkms/1.0

echo "reloading hpsa with hpsa_use_nvram_hba_flag=1"
modprobe -r hpsa
modprobe hpsa hpsa_use_nvram_hba_flag=1

echo "done."
echo "You now may want to reboot or rescan the SCSI bus"
