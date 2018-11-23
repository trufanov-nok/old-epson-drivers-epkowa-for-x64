#ifndef _EPSON_USB_H_
#define _EPSON_USB_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sane/sane.h>

#include "epkowa.h"
#include "sane/sanei_usb.h"

#define SANE_EPSON_VENDOR_ID	(0x4b8)

extern SANE_Word sanei_epson_usb_product_ids[];

extern int sanei_epson_getNumberOfUSBProductIds (void);

#endif
