#  iscan.spec.in -- an rpm spec file template
#  Copyright (C) 2004--2006  Olaf Meeuwissen
#
#  This file is part of the "Image Scan!" build infra-structure.
#
#  The "Image Scan!" build infra-structure is free software.
#  You can redistribute it and/or modify it under the terms of the GNU
#  General Public License as published by the Free Software Foundation;
#  either version 2 of the License or at your option any later version.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
#  FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
#  See the GNU General Public License for more details.
#
#  You should have received a verbatim copy of the GNU General Public
#  License along with this program; if not, write to:
#
#	 Free Software Foundation, Inc.
#	 59 Temple Place, Suite 330
#	 Boston, MA  02111-1307	 USA

#  Readability macro to help determine the C++ compiler's ABI version
#  at RPM build time.
#
%define cxx_abi	%{__cxx} -E -dM - < /dev/null | %{__awk} '/GXX_ABI/{print $3}'

#  Some handy macro definitions.
#
%define pkg	iscan
%define ver	2.10.0
%define rel	1
%define abi	%(test 1002 = `%cxx_abi` && echo .c2)
%define msg	%{_tmppath}/%{pkg}-%{ver}-%{rel}.msg

%define ia32			i386 i486 i586 i686
%define gimp_api_versions	1.2 2.0

# 	general package information

Name:		%{pkg}
Version:	%{ver}
Release:	%{rel}%{abi}
Source:		%{pkg}_%{ver}-%{rel}.tar.gz
License:	GPL (with exception clauses) and EAPL

Vendor:		EPSON AVASYS Corporation
URL:		http://www.avasys.jp/linux/index.html
Packager:	EPSON AVASYS Corporation <pipsnews@avasys.jp>

PreReq:		sane-backends

BuildRoot:	%{_tmppath}/%{pkg}-%{ver}-%{rel}-root
BuildRequires:	libusb >= 0.1.6, sane-backends-devel

%ifarch %{ia32}
BuildRequires:	gtk2-devel, gimp-devel, libpng-devel, libjpeg-devel
%endif

Group:		Applications/Multimedia
%ifnarch %{ia32}
Summary:	SANE backend for SEIKO EPSON scanners and all-in-ones
%description
The scanner driver provided by this package can be used by any SANE
standard compliant scanner utility.

Note that some scanners are only supported on the ia32 architecture.
%else
Summary:	simple, easy to use scanner utility for EPSON scanners
%description
Image Scan! is a graphical scanner utility for people that do not need
all the bells and whistles provided by several of the other utilities
out there (xsane, QuiteInsane, Kooka).

At the moment it only supports SEIKO EPSON scanners and all-in-ones.
However, the scanner driver it provides can be used by any other SANE
standard compliant scanner utility.

Note that several scanners require a non-free plugin before they can
be used with this software.
%endif


# 	rpmbuild sections

%prep
%setup -q


%build
%ifarch %{ia32}
%configure \
	--enable-frontend \
	--enable-jpeg \
	--enable-png
%else
%configure
%endif
make


%install
rm -rf ${RPM_BUILD_ROOT}
make install DESTDIR=${RPM_BUILD_ROOT}
test -d ${RPM_BUILD_ROOT}%{_sysconfdir}/hotplug/usb \
    || mkdir -p ${RPM_BUILD_ROOT}%{_sysconfdir}/hotplug/usb
%{__install} -m 0644 utils/hotplug/iscan.usermap \
    ${RPM_BUILD_ROOT}%{_sysconfdir}/hotplug/usb
%{__install} -m 0755 utils/hotplug/iscan-device \
    ${RPM_BUILD_ROOT}%{_sysconfdir}/hotplug/usb
#
#  Recent rpmbuild versions follow a FASCIST packaging policy and bomb
#  out on any installed files that are not packaged and non-installed
#  files that are.  Clean up 'make install's act here.
#
sane_confdir=%{_sysconfdir}/sane.d
./mkinstalldirs -m 0755 ${RPM_BUILD_ROOT}${sane_confdir}
install -m 0644 backend/epkowa.conf ${RPM_BUILD_ROOT}${sane_confdir}
rm ${RPM_BUILD_ROOT}%{_libdir}/sane/libsane-epkowa.so
rm ${RPM_BUILD_ROOT}%{_libdir}/sane/libsane-epkowa.a
#
#  Create a list of message catalogs that should be included in the RPM
#  binary package.  Use %lang(ll) notation so that sysadmins can choose
#  the catalogs they want installed.
#
> %{msg}
cd ${RPM_BUILD_ROOT}/%{_datadir}/locale
for lang in *
do
    test -f ${lang}/LC_MESSAGES/%{pkg}.mo || continue
    ll=`echo ${lang} | sed 's,@.*$,,'`
    echo "%lang(${ll}) %{_datadir}/locale/${lang}/LC_MESSAGES/%{pkg}.mo" \
	>> %{msg}
done


%clean
make clean			# rm -rf %{_builddir}/%{name}-%{version}
rm -rf ${RPM_BUILD_ROOT}
rm -f %{msg}


# 	rpm (un)installation scripts

#  Believe it or not, but there are distros out there that do not have
#  the sane service listed in their /etc/services.  Also note that the
#  official service name, as registered with the IANA is not sane, but
#  sane-port.  The saned alias is also commonly used.
#
%pre
srv=/etc/services
if [ -z "`grep 6566/tcp ${srv}`" ]
then
    cat >> ${srv} <<EOF
sane-port       6566/tcp   sane saned   # SANE Control Port
sane-port       6566/udp   sane saned   # SANE Control Port
EOF
fi


%post
#
#  Create udev rules from our installed hotplug usermap.
#
%{_libdir}/%{pkg}/make-udev-rules \
    %{_sysconfdir}/hotplug/usb/iscan.usermap \
    %{_sysconfdir}/udev/rules.d
#
#  Enable the epkowa backend unconditionally, assuming that people who
#  install this package want to use it.
#
dll=%{_sysconfdir}/sane.d/dll.conf
if [ -n "`grep '#[[:space:]]*epkowa' ${dll}`" ]
then				# uncomment existing entry
    sed -i 's,#[[:space:]]*\(epkowa\),\1,' ${dll}
elif [ -z "`grep epkowa ${dll}`" ]
then				# append brand new entry
    echo epkowa >> ${dll}
fi
#
#  Enable the GIMP plug-ins functionality system wide.
#
if test -x %{_bindir}/iscan
then
    for prefix in %{_libdir} /opt/gnome/lib
    do
        [ -d ${prefix} ] || continue
        for version in %{gimp_api_versions}
        do
	    dir=${prefix}/gimp/${version}/plug-ins
            [ -d ${dir} ] || mkdir -p ${dir}
	    ln -fs %{_bindir}/iscan ${dir}
        done
    done
    plugindir="`gimptool --gimpplugindir 2> /dev/null`"
    if [ x  = x"${plugindir}" ]; then
        plugindir="`gimptool-2.0 --gimpplugindir 2> /dev/null`"
    fi
    if [ x != x"${plugindir}" ]; then
        ln -fs %{_bindir}/iscan ${plugindir}/plug-ins
    fi
fi
#
#  Append our usermap data to usb.usermap for ancient hotplug versions
#  that don't look below $HOTPLUG_DIR/usb/.
#
agent=%{_sysconfdir}/hotplug/usb.agent
map=%{_sysconfdir}/hotplug/usb.usermap

if [ -x $agent -a -f $map ]; then

    notice='08-Aug-2002[[:space:]]*support for multiple usermaps'
    source='for MAP in $MAP_USERMAP $HOTPLUG_DIR/usb/\*\.usermap'

    if [ -z "$(grep "$notice" $agent)" -a -z "$(grep "$source" $agent)" ]
    then
	# We've got an ancient hotplug version that doesn't look for
	# maps in $HOTPLUG_DIR/usb/*.usermap.  Append our usermap to
	# usb.usermap to work around this.

	header='# Following info courtesy of Image Scan!'
	footer='# Preceding info courtesy of Image Scan!'

	# Clean out any previous additions we've made

	if [ -n "$(grep "$header" $map)" ]; then
	    cp $map $map.orig
	    sed "/^$header/,/^$footer/d" $map.orig > $map \
		&& rm $map.orig
	fi

	# Add our latest and greatest usermap

	echo "$header" >> $map
	cat %{_sysconfdir}/hotplug/usb/iscan.usermap >> $map
	echo "$footer" >> $map
    fi
fi


#  We have nothing to do.
#
%preun


#  Clean up left-overs from old iscan versions that silently clobbered
#  libsane-epson.  These versions only used library version numbers of
#  1.0.3 and 1.0.6.  Take care not to wipe symlinks to a sane-backends
#  EPSON backend.  For most of the currently used distributions, these
#  should all have a version number > 1.0.6.
#  FTR, sane-backends-1.0.6 was released on 2001-11-05.
#  Disable the backend we provide.
#  Also clean up our data directory, but only if it is empty.
#
%postun
if [ $1 = 0 ]
then
    for lib_ver in 1.0.3 1.0.6
    do
	libname=%{_libdir}/sane/libsane-epson.so
	if [ "`readlink ${libname} 2>/dev/null`" = libsane-epson.so.${lib_ver} ]
	then
	    rm -f ${libname}.1
	    rm -f ${libname}
	fi
    done
    #
    #  Clean out any additions to the top-level usermap
    #
    map=%{_sysconfdir}/hotplug/usb.usermap
    header='# Following info courtesy of Image Scan!'
    footer='# Preceding info courtesy of Image Scan!'
    if [ -f $map ]; then
	if [ -n "$(grep "$header" $map)" ]; then
	    cp $map $map.orig
	    sed "/^$header/,/^$footer/d" $map.orig > $map \
		&& rm $map.orig
	fi
    fi
    dll=%{_sysconfdir}/sane.d/dll.conf
    if [ -n "`grep ^epkowa ${dll}`" ]
    then
	sed -i 's/^epkowa/#epkowa/' ${dll}
    fi
fi
rmdir %{_datadir}/%{pkg} 2> /dev/null || true
rmdir %{_libdir}/%{pkg}  2> /dev/null || true
#
#  Clean up the GIMP plug-ins.
#
for prefix in %{_libdir} /opt/gnome/lib
do
    for version in %{gimp_api_versions}
    do
	dir=${prefix}/gimp/${version}/plug-ins
	rm    ${dir}/iscan 2> /dev/null || true
	rmdir ${dir}       2> /dev/null || true
	rmdir `dirname ${dir}` 2> /dev/null || true
    done
    rmdir ${prefix}/gimp 2> /dev/null || true
done
plugindir="`gimptool --gimpplugindir 2> /dev/null`"
if [ x  = x"${plugindir}" ]; then
    plugindir="`gimptool-2.0 --gimpplugindir 2> /dev/null`"
fi
if [ x != x"${plugindir}" ]; then
    rm ${plugindir}/plug-ins/iscan 2> /dev/null || true
fi


# 	package contents

#  Note that we generate the %%{msg} file during the %%install phase
#  so that we can use %%lang(xx) notation without the need to resort
#  to a manually maintained list here.
#
%files -f %{msg}
%defattr(-,root,root)

%doc NEWS    README    AUTHORS
%doc COPYING COPYING.LIB
%doc non-free/EAPL.en.txt
#  This should really go into ${RPM_DOC_DIR}/examples/.
%doc doc/xinetd.sane

%doc NEWS.ja README.ja
%doc non-free/EAPL.ja.txt

%config(noreplace)	%{_sysconfdir}/sane.d/epkowa.conf

%ifarch %{ia32}
%{_bindir}/iscan
%{_libdir}/libesmod.so*
%endif
%{_libdir}/sane/libsane-epkowa.la
%{_libdir}/sane/libsane-epkowa.so.*
%{_libdir}/%{pkg}/make-udev-rules
%{_mandir}/man*/*
%{_sysconfdir}/hotplug/usb


# 	significant packaging changes

%changelog
* Tue May 23 2006  Olaf Meeuwissen <olaf.meeuwissen@avasys.jp>
  - create udev rules from hotplug usermap using custom utility

* Tue Jan 24 2006  Olaf Meeuwissen <olaf.meeuwissen@avasys.jp>
  - turned the dependency on sane-backend into a PreReq: because our
    %post mucks with its dll.conf
  - disable the backend when uninstalling
  - use sed's -i option to modify dll.conf in place

* Sat Jan 21 2006  Olaf Meeuwissen <iscan@member.fsf.org>
  - removed packaging of interpreter plugins

* Fri Sep 30 2005  Olaf Meeuwissen <olaf.meeuwissen@avasys.jp>
  - package the backend interpreter plugins in their own RPMs
  - added support for build on architectures other than IA32
  - beefed up the BuildRequires:

* Thu Jun 16 2005  Olaf Meeuwissen <olaf.meeuwissen@avasys.jp>
  - use gimptool to find the plugin directory (note that some distro's
    prefer to provide gimptool-2.0 only!)

* Thu May 19 2005  Olaf Meeuwissen <olaf.meeuwissen@avasys.jp>
  - fixed broken dependency specification
  - use release number defined in configure.ac

* Mon Apr 25 2005  Olaf Meeuwissen <olaf.meeuwissen@avasys.jp>
  - modified %doc entries to handle license file name changes
  - updated URL: to reflect corporate name change

* Sun Apr 10 2005  Olaf Meeuwissen <iscan@member.fsf.org>
  - integrated iscan-1.14.0 changes to add hotplug support

* Fri Dec 24 2004  Olaf Meeuwissen <iscan@member.fsf.org>
  - bumped libusb versioned dependency to use the correct version

* Fri Dec  3 2004  Olaf Meeuwissen <olaf@epkowa.co.jp>
  - fixed creation of GIMP plug-in symlinks

* Mon Nov  1 2004  Olaf Meeuwissen <olaf@epkowa.co.jp>
  - fixed libesmod.so* path (it's not in %{_libdir}/%{pkg} yet)
  - added Japanese documentation
  - doubled %s in the spec file parts to un-confuse rpmbuild
  - fixed typo in config file installation command
  - fixed generation of message catalog list

* Fri Oct 29 2004  Olaf Meeuwissen <iscan@member.fsf.org>
  - adapt to Fascist build policy
  - no longer install libsane-epkowa.so as this seems to have become
    the vogue for non-development packages
  - install libsane-epkowa.la so that the dll backend can still find
    the epkowa backend

* Tue Oct 19 2004  Olaf Meeuwissen <iscan@member.fsf.org>
  - initial spec file

# 	end of file
