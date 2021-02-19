This directory contains the INDI driver for the mount (so we can control it whithin kstars/Ekos)

# Dependencies

Note: If you are missing buttons' icons in kstars: sudo apt install breeze-icon-theme

Dependancy: sudo apt-get install cdbs cmake git libcfitsio-dev libnova-dev libusb-1.0-0-dev libjpeg-dev libusb-dev libftdi-dev fxload libkrb5-dev libcurl4-gnutls-dev libraw-dev libgphoto2-dev libgsl0-dev dkms libboost-regex-dev libgps-dev libdc1394-22-dev

# Compile

cd eqmount2driver/
chmod u+x compile.sh
./compile.sh

# Use

## Connect to the Telescope driver

Open KStars
Tools/Devices/ Devices Manager
Clients Tab/ Hosts / New
Eqmount2 / localhost / 7624
Clients Tab/ Connections / Connect

## Connect Camera (+others)

Tools/Ekos
Profile > New profile
 - Name: Choose a name
 - Mode: Local
 - ... Add you other devices
