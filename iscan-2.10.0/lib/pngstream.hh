//  pngstream.hh -- 
//  Copyright (C) 2004  SEIKO EPSON Corporation
//
//  This file is part of 'iscan' program.
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

#ifndef iscan_pngstream_hh_included
#define iscan_pngstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#include <new>			// for std::bad_alloc
#include "imgstream.hh"

#ifdef HAVE_PNG_H
#include <png.h>
#endif

namespace iscan
{

class pngstream : public imgstream
{
public:
  // types (inherited from imgstream)
  typedef imgstream::char_type char_type;
  typedef imgstream::int_type int_type;
  typedef imgstream::pos_type pos_type;
  typedef imgstream::off_type off_type;
  typedef imgstream::traits_type traits_type;

  typedef imgstream::filebuf filebuf;

  typedef imgstream::size_type size_type;

private:
  bool _header_done;

#ifdef HAVE_PNG_H
  png_structp _png;
  png_infop   _info;
#endif

  int _colour_type;

  struct png_lib_handle
  {
    bool        is_usable;
    std::string message;
    lt_dlhandle lib;

#ifdef HAVE_PNG_H
    fundecl (png_uint_32, access_version_number,
	     void);
    fundecl (png_structp, create_write_struct,
	     png_const_charp, png_voidp, png_error_ptr, png_error_ptr);
    fundecl (png_infop  , create_info_struct ,
	     png_structp);
    fundecl (void, destroy_write_struct,
	     png_structpp, png_infopp);
    fundecl (void, init_io,
	     png_structp, FILE *);
    fundecl (void, set_IHDR,
	     png_structp, png_infop, png_uint_32, png_uint_32, int,
	     int, int, int, int);
    fundecl (void, set_pHYs,
	     png_structp, png_infop, png_uint_32, png_uint_32, int);
    fundecl (void, set_invert_mono, png_structp);
    fundecl (void, write_info,
	     png_structp, png_infop);
    fundecl (void, write_row,
	     png_structp, png_bytep);
    fundecl (void, write_flush,
	     png_structp);
    fundecl (void, write_end,
	     png_structp, png_infop);
#if PNG_LIBPNG_VER > 10499
    fundecl (png_uint_32, get_current_row_number,
         png_structp);
#endif
#endif // HAVE_PNG_H
  };

  static png_lib_handle *lib;
#ifdef HAVE_PNG_H
  static png_const_charp version_string;
#endif

public:
  static bool is_usable ();

  explicit pngstream (filebuf *fb);
  explicit pngstream (const char *pathname,
		      ios_base::openmode mode = (ios_base::in |
						 ios_base::out));
  virtual ~pngstream ();

  virtual imgstream& write (const char_type *line, std::streamsize n);

  virtual imgstream& resolution (size_type hres, size_type vres);
  virtual imgstream& colour (colour_space space);

private:
  void init ();

  imgstream& write_header ();

#ifdef HAVE_PNG_H
  friend void set_error_handler (png_structp, png_infop);
#endif
};


// 	implementation

// This function needs to be expanded verbatim at the location it's
// invoked for the setjmp call to work as intended.  Just declaring
// it inline is NOT guaranteed to do that --> use a #define?
#ifdef HAVE_PNG_H
inline
void
set_error_handler (png_structp png, png_infop info)
{
  if (setjmp (png_jmpbuf (png))) {
    pngstream::lib->destroy_write_struct (&png, &info);
    throw;			// FIXME! decide exception type
  }
}
#endif // HAVE_PNG_H

}	// namespace iscan

#endif	// iscan_pngstream_hh_included
