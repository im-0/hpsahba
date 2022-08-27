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
	#do not set a version here! 
	#If you do on a kernel update the headers will NOT be updated automatically!
	apt install -y pve-headers
else
	echo "installing default linux-headers"
	apt install -y "linux-headers-$(uname -r)"
fi

echo "check if hpsa dkms module is loaded"
if (dkms status | grep -q "hpsa-dkms")
	then
		echo "hpsa-dkms is loaded, getting module version"
		dkmsVersion=$(dkms status | grep "hpsa-dkms" | cut -d " " -f 2 | tr -d ",")
		echo "hpsa-dkms/$dkmsVersion is installed."
		echo "Removing module."
		dkms remove "hpsa-dkms/$dkmsVersion"
		#"force" remove as dkms remove sometimes won't work
		rm -r /var/lib/dkms/hpsa-dkms
else
		echo "module is not loaded, continuing"
fi

echo "installing new kernel module"
dkms add ./
dkms install --force hpsa-dkms/10.0

if (dkms status | grep -q "hpsa-dkms, 10.0")
	then
		echo -n "The installation should now be completed successfully.\n You probably need to reboot your machine to apply the changes."
		while true; do
			read -p "Do you want to restart now?  (y/n) " yn
			
			case $yn in 
				[yY] ) shutdown -r now;
					break;;
				[nN] ) echo exiting...;
					exit;;
				* ) echo invalid response;;
			esac

			done
	else
		echo -n "It seems that something went wrong. Please restart the installation and watch the output for error messages."
		echo "If the problem persists, feel free to open an Issue on https://github.com/mashuptwice/hpsahba"
		exit 1;
fi