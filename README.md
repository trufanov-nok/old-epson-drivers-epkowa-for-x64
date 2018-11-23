# old-epson-drivers-epkowa-for-x64
This is a deb package, source files and instructions about instalation of old Epson scanner drivers for 64-bit Nix systems.

Old Epson scanners require SANE's epkowa backend. And some of them additionaly require 32-bit proprietary firmware lib which is dynamically loaded at runtime. These are:
> GT-7200U, GT-7300U, GT-9400UF, GT-F500, GT-F520, GT-F550, GT-F570, GT-F600, Perfection 1250, Perfection 1250 Photo, Perfection 1260, Perfection 1260 Photo, Perfection 2480 Photo, Perfection 2580 Photo, Perfection 3170 Photo, Perfection 3490 Photo, Perfection 3590 Photo, Perfection 4180 Photo  

Thus they can't be used with 64-bit epkowa backend and 64-bit sane system without some tricks. Or you have to get a 32-bit sane and 32-bit frontend.

Here you can find the instruction and the deb package that allows to work with 32-bit Epson drivers on 64-bit Debian-based machine (tested in Kubuntu 18.10). The main idea is to install both 32-bit and 64-bit versions of libsane on your 64-bit system, copy 32-bit saned daemon and access drivers via it on localhost port. In other words 64-backend (Skanlite for ex.) launches 64-bit SANE, it looks for network scanners via 64-bit net backend, access 32-bit saned daemon listening localhost port and 32-bit saned loads 32-bit Epkowa backend that on its turn loads 32-bit firmware for your scanner.   

The deb package contains Epkowa backend, set of 32-bit firmwares, 32-bit frontend iscan for testing purposes, a copy of 32-bit saned daemon which is renamed to saned32 and service registration files for it for systemd (currently used in Kubuntu 18.x).
There is also a detailed instructions on how to install such drivers file by file that can be usful if you have a different 64-bit system.

If you have Epson scanner and 64-bit system, but your scanner doesn't require additional 32-bit firmware, then you may just look for 64-bit epkowa backend (googl for `sane-backends-iscan-1.0.25-7.mga7.x86_64.rpm` or something like that) or try to use SANE's plustek backend which is default for such devices.
