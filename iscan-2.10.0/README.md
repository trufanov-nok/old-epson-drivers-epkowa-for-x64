This folder contains source of iscan_2.10.0-1.tar.gz  
It contains iscan and SANE's epkowa backend sources.   
The sources were for x86, so I've applied a few patches to make them compilable in 64-bit system. 

To compile use:
> ./configure --enable-jpeg --enable-png --enable-tiff CPPFLAGS='-fPIC -fpermissive'  
> make

then check `./.backend/.libs` for resulted `libsane-epkowa.so*` and `./backend/epkowa.conf`
It'll be 64-bit. I was able to debug it at runtime so it's loaded by 64-bit SANE without a problem.
But I had no device that work with kust epkowa and without 32-bit firmware, and as 64-bit libsane-epkowa.so can't load 32-bit libs I couldn't test it further.

If your device doesn't need any 32-bit firmware lib (epson official site doesn't suggest you any additional "patch for iscan" rpm) you may try to work with it with just a 64-bit `libsane-epkowa.so*` instead of 32-bit drivers and `saned`.
