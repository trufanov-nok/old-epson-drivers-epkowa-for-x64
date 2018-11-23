##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2004, 2005  Olaf Meeuwissen
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

exec_sanelibdir = $(libdir)/sane
exec_sanelib_LTLIBRARIES = \
	libsane-epkowa.la

libsane_epkowa_la_CPPFLAGS = \
	$(LTDLINCL) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/non-free \
	-DBACKEND_NAME=epkowa \
	-DV_MAJOR=$(SANE_MAJOR) -DV_MINOR=$(SANE_MINOR) \
	-DPKGLIBDIR=\"$(pkglibdir)\"
libsane_epkowa_la_LDFLAGS = \
	-export-symbols-regex ^sane_ \
	-version-info $(SANE_MAJOR):$(SANE_REVISION):$(SANE_MINOR)
libsane_epkowa_la_LIBADD = \
	libsane-epkowa-s.la \
	$(top_builddir)/sanei/libsanei.la \
	$(LIBLTDL)
libsane_epkowa_la_SOURCES = \
	epkowa.c \
	epkowa.h \
	epkowa_ip.c \
	epkowa_ip.h \
	epkowa_ip_api.h \
	epkowa_scsi.c \
	epkowa_scsi.h \
	epkowa_usb.c \
	epkowa_usb.h \
	sane_strstatus.c

noinst_LTLIBRARIES = \
	libsane-epkowa-s.la

libsane_epkowa_s_la_CPPFLAGS = $(libsane_epkowa_la_CPPFLAGS) -DSTUBS
libsane_epkowa_s_la_LDFLAGS  = -static
libsane_epkowa_s_la_SOURCES  = epkowa-s.c

BUILT_SOURCES = \
	epkowa-s.c

EXTRA_DIST = \
	epkowa.conf \
	stubs.c

%-s.c: $(srcdir)/stubs.c
	$(LN_S) $< $@


##  There should be NO libsane.so symlink in $(exec_sanelibdir), but
##  libtool insists on making one for us anyway.
##
install-exec-hook:
	-rm -f $(DESTDIR)$(exec_sanelibdir)/libsane.so.$(SANE_MAJOR)


##  Minimal sanity checks on the backends we distribute.
##  WARNING: These checks may fail on non-Linux systems for a variety
##           of reasons.  If you know why, we would like to know (and
##           patches are naturally welcome).
check-local:
	for d in .libs _libs; do \
	  test -d $$d || continue; \
	  for l in $$d/libsane-*.so; do \
	    soname=`objdump -p $$l | awk '/SONAME/ {print $$2}'`; \
	    if test "$$soname" != "libsane.so.$(SANE_MAJOR)"; then \
	      echo "$$l: libsane.so.$(SANE_MAJOR) != $$soname"; \
	      exit 1; \
	    fi; \
	  done; \
	done
