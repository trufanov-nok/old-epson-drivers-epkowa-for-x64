#include <sys/types.h>
#include "epkowa_usb.h"


SANE_Word sanei_epson_usb_product_ids[] = {
  0x101,			/* GT-7000 / Perfection 636 */
  0x103,			/* GT-6600 / Perfection 610 */
  0x104,			/* GT-7600 / Perfection 1200 */
  0x106,			/*         / Stylus Scan 2500 */
  0x107,			/* ES-2000 / Expression 1600 */
  0x109,			/* ES-8500 / Expression 1640 XL */
  0x10a,			/* GT-8700 / Perfection 1640 */
  0x10b,			/* GT-7700 / Perfection 1240 */
  0x10c,			/* GT-6700 / Perfection 640 */
  0x10e,			/* ES-2200 / Expression 1680 */
  0x110,			/* GT-8200 / Perfection 1650 */
  0x112,			/* GT-9700 / Perfection 2450 */
  0x11b,			/* GT-9300 / Perfection 2400 */
  0x11c,			/* GT-9800 / Perfection 3200 */
  0x11e,			/* GT-8300 / Perfection 1660 */
  0x126,			/* ES-7000 / GT-15000 */
  0x128,			/* GT-X700 / Perfection 4870 */
  0x129,			/* ES-10000G / Expression 10000XL */
  0x12a,			/* GT-X800 / Perfection 4990 */
  0x12b,			/* ES-H300 / GT-2500 */
  0x12c,			/* GT-X900 / Perfection V700/V750 */
  0x801,			/* CC-600  / Stylus CX5100/CX5200 */
  0x802,			/* CC-570  / Stylus CX3100/CX3200 */
  0x805,			/*         / Stylus CX6300/CX6400 */
  0x806,			/* PM-A850 / Stylus Photo RX600 */
  0x807,			/*         / Stylus Photo RX500/RX510 */
  0x808,			/*         / Stylus CX5300/CX5400 */
  0x80d,			/*         / Stylus CX4500/CX4600 */
  0x80e,			/* PX-A550 / Stylus CX3500/CX3600/CX3650 */
  0x80f,			/*         / Stylus Photo RX420/RX430 */
  0x810,			/* PM-A900 / Stylus Photo RX700 */
  0x811,			/* PM-A870 / Stylus Photo RX620/RX630 */
  0x813,			/*         / Stylus CX6500/CX6600 */
  0x814,			/* PM-A700 */
  0x815,			/* LP-A500 / AcuLaser CX11 */
  0x817,			/* LP-M5500 */
  0x818,			/*         / Stylus CX3700/CX3800/DX3800 */
  0x819,			/* PX-A650 / Stylus CX4700/CX4800/DX4800 */
  0x81a,			/* PM-A750 / Stylus Photo RX520/RX530 */
  0x81c,			/* PM-A890 / Stylus Photo RX640/RX640 */
  0x81d,			/* PM-A950 */
  0x81f,			/*         / Stylus CX7700/CX7800 */
  0x820,			/*         / Stylus CX4100/CX4200/DX4200 */
  0x827,			/* PM-A820 / Stylus Photo RX560/RX580/RX590 */
  0x828,			/* PM-A970 */
  0x829,			/* PM-T990 */
  0x82a,			/* PM-A920 */
  0x82b,			/*         / Stylus CX4900/CX5000/DX5000 */
  0x82e,			/* PX-A720 / Stylus CX5900/CX6000/DX6000 */
  0x82f,			/* PX-A620 / Stylus CX3900/DX4000 */
  0x830,			/*         / Stylus CX2800/CX2900/ME200 */
  0x833,			/* LP-M5600 */
  0x835,			/* AcuLaser CX21 */
  0x836,			/* PM-T960 */
  0x837,			/* PM-A940 / Stylus Photo RX680/RX685/RX690 */
  0x838,			/*         / Stylus CX7300/CX7400/DX7400 */
  0x839,			/* PX-A740 / Stylus CX8300/CX8400/DX8400 */
  0x83a,			/*          / Stylus CX9300F/CX9400Fax/DX9400F */
  0x83c,			/* PM-A840 / Stylus Photo RX585/RX595/RX610 */
  0				/* last entry - this is used for devices that are specified
				   in the config file as "usb <vendor> <product>" */
};



int
sanei_epson_getNumberOfUSBProductIds (void)
{
  return sizeof (sanei_epson_usb_product_ids) / sizeof (SANE_Word);
}
