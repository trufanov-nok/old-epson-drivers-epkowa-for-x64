##  Makefile.am -- an automake template for a Makefile.in file
##  Copyright (C) 2004, 2005  Olaf Meeuwissen
##  Copyright (C) 2007  EPSON AVASYS Corporation
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

##  Note: SUBDIRS are recursed in the order listed.

SUBDIRS = \
	include \
	libltdl \
	lib \
	sanei \
	backend \
	non-free \
	frontend \
	utils \
	po \
	intl \
	doc

ACLOCAL_AMFLAGS = -I m4

M4_MACROS = \
	m4/codeset.m4 \
	m4/gettext.m4 \
	m4/glibc21.m4 \
	m4/iconv.m4 \
	m4/intdiv0.m4 \
	m4/intmax.m4 \
	m4/inttypes-pri.m4 \
	m4/inttypes.m4 \
	m4/inttypes_h.m4 \
	m4/isc-posix.m4 \
	m4/lcmessage.m4 \
	m4/lib-ld.m4 \
	m4/lib-link.m4 \
	m4/lib-prefix.m4 \
	m4/longdouble.m4 \
	m4/longlong.m4 \
	m4/nls.m4 \
	m4/po.m4 \
	m4/printf-posix.m4 \
	m4/progtest.m4 \
	m4/signed.m4 \
	m4/size_max.m4 \
	m4/stdint_h.m4 \
	m4/uintmax_t.m4 \
	m4/ulonglong.m4 \
	m4/wchar_t.m4 \
	m4/wint_t.m4 \
	m4/xsize.m4

EXTRA_DIST = \
	$(M4_MACROS) \
	m4/libtool.m4 \
	NEWS.ja \
	README.ja \
	\
	debian/changelog \
	debian/control \
	debian/control.in \
	debian/copyright \
	debian/rules \
	\
	debian/dll.conf \
	debian/$(PACKAGE_TARNAME).docs \
	debian/$(PACKAGE_TARNAME).menu \
	debian/$(PACKAGE_TARNAME).desktop \
	\
	$(PACKAGE_TARNAME).spec


update-changelog:
	> $(srcdir)/ChangeLog


##  	Source tarball related targets

##  Things to do before we pack up the sources in a tarball.

dist-hook: \
	deb-check-control \
	deb-check-changelog \
	rpm-check-spec

##  	Debian packaging support related targets

##  Convenience variables to make writing the various Debian related
##  targets a bit easier.

DEB_PACKAGE_NAME = $(PACKAGE_TARNAME)_$(PACKAGE_VERSION)-$(PACKAGE_RELEASE)

$(distdir).tar.gz: dist

deb-src-dist: $(DEB_PACKAGE_NAME).dsc

$(DEB_PACKAGE_NAME).dsc $(DEB_PACKAGE_NAME).tar.gz: $(distdir).tar.gz
	GZIP=$(GZIP_ENV) gunzip -c $< | $(am__untar)
	dpkg-source -b $(distdir) && rm -rf $(distdir)
	-rm -f $<

##  When making `./configure` responsible for doing this, the resulting
##  ./debian/control becomes a candidate for removal by the 'distclean'
##  target.  The regular Debian build machinery scrubs the source tree
##  before doing a build which would remove ./debian/control.  Doh!

$(srcdir)/debian/control: $(srcdir)/debian/control.in \
	$(srcdir)/configure
	sed -e 's|[@]PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' \
	    -e 's|[@]PACKAGE_TARNAME[@]|$(PACKAGE_TARNAME)|g' \
	    -e 's|[@]PACKAGE_BUGREPORT[@]|$(PACKAGE_BUGREPORT)|g' \
	    $< > $@

##  A little QA check to make sure we don't accidentally release with
##  a control file that contains substitutable parameters.

deb-check-control: $(srcdir)/debian/control
	status=; \
	for f in $^; do \
	    if `LC_ALL=C $(EGREP) '@\w+@' $$f >/dev/null 2>&1`; then \
		echo "$$f contains unsubstituted variables; not releasing"; \
		status=1; \
	    fi; \
	done; \
	test x = x$$status || exit 1

##  Some more QA to make sure our debian/changelog is in sync with the
##  release.

deb-check-changelog: $(srcdir)/debian/changelog
	case `sed 1q $<` in \
	    "$(PACKAGE_TARNAME) ($(PACKAGE_VERSION)-$(PACKAGE_RELEASE))"*) \
		: \
		;; \
	    *) \
		echo "$< out of sync; not releasing" 1>&2; \
		exit 1 \
		;; \
	esac

.PHONY: deb-src-dist deb-check-control deb-check-changelog


##  ^L  RPM packaging related targets

##  A little QA check to make sure we don't accidentally release with
##  a spec file that contains substitutable parameters.

rpm-check-spec: $(srcdir)/$(PACKAGE_TARNAME).spec
	status=; \
	for f in $^; do \
	  if `LC_ALL=C $(EGREP) '@\w+@' $$f >/dev/null 2>&1`; then \
	    echo "$$f has unsubstituted variables; not releasing" 1>&2; \
	    status=1; \
	  fi; \
	done; \
	test x = x$$status || exit 1

.PHONY: rpm-check-spec
