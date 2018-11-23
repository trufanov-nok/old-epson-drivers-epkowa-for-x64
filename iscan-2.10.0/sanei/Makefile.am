##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2004  Olaf Meeuwissen
##
##  This file is part of the "Image Scan!" build infra-structure.
##
##  The "Image Scan!" build infra-structure is free software.
##  You can redistribute it and/or modify it under the terms of the GNU
##  General Public License as published by the Free Software Foundation;
##  either version 2 of the License or at your option any later version.
##
##  This program is distributed in the hope that it will be useful, but
##  WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
##  FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
##  See the GNU General Public License for more details.
##
##  You should have received a verbatim copy of the GNU General Public
##  License along with this program; if not, write to:
##
##      Free Software Foundation, Inc.
##      59 Temple Place, Suite 330
##      Boston, MA  02111-1307  USA

noinst_LTLIBRARIES = libsanei.la
libsanei_la_CPPFLAGS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include
libsanei_la_LDFLAGS = \
	-static
libsanei_la_SOURCES = \
	sanei_config.c \
	sanei_config2.c \
	sanei_constrain_value.c \
	sanei_init_debug.c \
	sanei_pio.c \
	sanei_scsi.c \
	sanei_usb.c

EXTRA_DIST = \
	linux_sg3_err.h \
	sanei_DomainOS.c \
	sanei_DomainOS.h
