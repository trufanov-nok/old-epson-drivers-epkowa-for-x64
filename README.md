# old-epson-drivers-epkowa-for-x64
This is a deb package, source files and instructions about instalation of old Epson scanner drivers for 64-bit Nix systems.

Old Epson scanners require SANE's epkowa backend. It may be downloaded from official Epson website, bit it's in rpm and it's 32-bit. There are sources of epkowa backend which may be compiled for 64-bit system with some patches. But the backend itself isn't enough. It dynamically loads 32-bit firmwares for scanners. And these are proprietary. As 64-bit application can't be linked with 32-bit library this means that 32-bit SANE version is required. Also only 32-bit frontend can load such driver.

Here you can find the instruction and the deb package that allows to work with 32-bit Epson drivers on 64-bit Debian-based machine (tested in Kubuntu 18.10). The main idea is to install both 32-bit and 64-bit versions of libsane on your 64-bit system, copy 32-bit saned daemon and access drivers with it on localhost. In other words 64-backend (Skanlite for ex.) launches 64-bit SANE, it looks for network scanners via 64-bit net backend, access 32-bit saned daemon running on localhost and 32-bit saned loads 32-bit Epkowa backend that in its turn loads 32-bit firmware for your scanner.
The deb package contains Epkowa backend, set of 32-bit firmwares, 32-bit frontend iscan for testing purposes, a copy of 32-bit saned daemon which is renamed to saned32 and service registration files for it for systemd (currently used in Kubuntu 18.x).
There is also a detailed instructions on how to install such drivers file by file that can be usful if you have a different 64-bit system.