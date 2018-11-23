//  imgstream.hh -- 
//  Copyright (C) 2004  SEIKO EPSON Corporation
//
//  This file is part of the 'iscan' program.
//
//  The 'iscan' program is free-ish software.
//  You can redistribute it and/or modify it under the terms of the GNU
//  General Public License as published by the Free Software Foundation;
//  either version 2 of the License or at your option any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
//  FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
//  See the GNU General Public License for more details.
//
//  You should have received a verbatim copy of the GNU General Public
//  License along with this program; if not, write to:
//
//      Free Software Foundation, Inc.
//      59 Temple Place, Suite 330
//      Boston, MA  02111-1307  USA
//
//  As a special exception, the copyright holders give permission
//  to link the code of this program with the esmod library and
//  distribute linked combinations including the two.  You must obey
//  the GNU General Public License in all respects for all of the
//  code used other then esmod.

#ifndef iscan_imgstream_hh_included
#define iscan_imgstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <stdexcept>
#include "cfilebuf.hh"
#include "colour.hh"

#include <dirent.h>
#include <ltdl.h>

namespace iscan
{
  using std::ios_base;

class imgstream : protected std::basic_iostream<char>
{
protected:
  typedef std::basic_iostream<char> imgstream_base_t;

public:
  typedef imgstream_base_t::char_type char_type;
  typedef imgstream_base_t::int_type int_type;
  typedef imgstream_base_t::pos_type pos_type;
  typedef imgstream_base_t::off_type off_type;
  typedef imgstream_base_t::traits_type traits_type;

  typedef basic_cfilebuf<char> filebuf;

  typedef size_t size_type;

private:
  filebuf *_fbuf;
  bool     _mine;		// whether the instance owns _fbuf

protected:			// so subclasses have easy access
  size_type _h_sz;
  size_type _v_sz;
  size_type _hres;
  size_type _vres;

  size_type    _bits;
  colour_space _cspc;

public:
  virtual ~imgstream ();

  bool is_open () const;

  virtual void open  (const char *pathname, ios_base::openmode mode);
  virtual void close ();

  filebuf * rdbuf () const;
  filebuf * rdbuf (filebuf *fb);

  virtual imgstream& write (const char_type *line, std::streamsize n) = 0;

  operator FILE * ();

  virtual imgstream& size       (size_type h_sz, size_type v_sz);
  virtual imgstream& resolution (size_type hres, size_type vres);
  virtual imgstream& depth      (size_type bits);
  virtual imgstream& colour     (colour_space space);

protected:
           imgstream ();
  explicit imgstream (FILE *fp);
  explicit imgstream (filebuf *fb);
           imgstream (const char *pathname, ios_base::openmode mode);

  void init ();

private:			// undefined to prevent copying
  imgstream (const imgstream&);
  imgstream& operator= (const imgstream&);


protected:
  typedef lt_dlhandle dl_handle;
  typedef lt_ptr      dl_ptr;
  
  static dl_handle dlopen  (const char *libname)
    throw (std::runtime_error);
  static dl_ptr    dlsym   (dl_handle lib, const char *funcname);
  static int       dlclose (dl_handle lib);

private:
  static dl_handle    find_dlopen (const char *libname);
  static const char *_libname;

  friend int selector (const dirent *);

#ifdef __GNUC__
#define fundecl(returntype,funcname,arglist...) \
	typedef returntype (*funcname##_f) (arglist); \
	funcname##_f funcname;
#else
#error "Your compiler is not known to support macros with a variable"
#error "number of arguments.  In case it does, please report this to"
#error "the library maintainers and include a suitable preprocessor"
#error "check for them to add.  A patch will be most appreciated."
#endif

};

}	// namespace iscan

#endif	// iscan_imgstream_hh_included
