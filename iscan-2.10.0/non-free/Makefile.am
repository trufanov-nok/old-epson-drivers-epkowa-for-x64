##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2004--2006  Olaf Meeuwissen
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

NON_FREE_INCLUDES = \
	filter.hh
NON_FREE_FRONTEND_SOLIBS = \
	libesmod-i386.so \
	libesmod-i386.c2.so
NON_FREE_LICENSES = \
	EAPL.en.txt \
	EAPL.ja.txt

EXTRA_DIST = \
	$(NON_FREE_LICENSES) \
	$(NON_FREE_FRONTEND_SOLIBS) \
	$(NON_FREE_INCLUDES)


V_MAJOR = 1
V_MINOR = 1
V_REV = 0

NON_FREE_SOLIBS = libesmod.so

if ENABLE_FRONTEND

all: $(NON_FREE_SOLIBS)

$(NON_FREE_SOLIBS):
	-rm -f $@
	case $(ISCAN_HOST_CPU) in \
	  i?86) arch=i386;; \
	  *)	echo "unsupported architecture" 1>&2; \
		exit 1;; \
	esac; \
	$(LN_S) $(srcdir)/libesmod-$${arch}$(PACKAGE_CXX_ABI).so $@

install-exec-local: $(NON_FREE_SOLIBS)
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	@list="$(NON_FREE_SOLIBS)"; for nfl in $$list; do \
	  echo installing $${nfl} in $(libdir)/$${nfl}...; \
	  $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) $(srcdir)/$${nfl} \
	      $(DESTDIR)$(libdir)/$${nfl}.$(V_MAJOR).$(V_MINOR).$(V_REV); \
	  (cd $(DESTDIR)$(libdir) \
	   && ln -fs $${nfl}.$(V_MAJOR).$(V_MINOR).$(V_REV) $${nfl}.$(V_MAJOR)); \
	  (cd $(DESTDIR)$(libdir) \
	   && ln -fs $${nfl}.$(V_MAJOR).$(V_MINOR).$(V_REV) $${nfl}); \
	done

uninstall-local:
	@list="$(NON_FREE_SOLIBS)"; for nfl in $$list; do \
	  rm -f $(DESTDIR)$(libdir)/$${nfl}; \
	  rm -f $(DESTDIR)$(libdir)/$${nfl}.$(V_MAJOR); \
	  rm -f $(DESTDIR)$(libdir)/$${nfl}.$(V_MAJOR).$(V_MINOR).$(V_REV); \
	done

.PHONY: $(NON_FREE_SOLIBS)

endif
