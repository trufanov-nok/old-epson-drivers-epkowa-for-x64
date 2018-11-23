//  jpegstream.hh -- 
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

#ifndef iscan_jpegstream_hh_included
#define iscan_jpegstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#include "imgstream.hh"

#ifdef HAVE_JPEGLIB_H
#include <jpeglib.h>
#endif

namespace iscan
{

class jpegstream : public imgstream
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
  char_type *_scanline;

  bool _write_init_done;

#ifdef HAVE_JPEGLIB_H
  struct jpeg_compress_struct _info;
  struct jpeg_error_mgr       _err;
#endif

  struct jpeg_lib_handle
  {
    bool        is_usable;
    std::string message;
    lt_dlhandle lib;

#ifdef HAVE_JPEGLIB_H
#ifndef jpeg_create_compress
    fundecl (void, create_compress, jpeg_compress_struct *);
#else
    fundecl (void, CreateCompress, jpeg_compress_struct *, int, size_t);
#endif
    fundecl (void, finish_compress, jpeg_compress_struct *);
    fundecl (void, destroy_compress, jpeg_compress_struct *);
    fundecl (void, destroy, jpeg_common_struct *);

    fundecl (void, stdio_dest, jpeg_compress_struct *, FILE *);
    fundecl (struct jpeg_error_mgr *, std_error, jpeg_error_mgr *);

    fundecl (void, write_scanlines, jpeg_compress_struct *, JSAMPLE **, int);
    fundecl (void, set_defaults, jpeg_compress_struct *);
    fundecl (void, start_compress, jpeg_compress_struct *, bool);
#endif
  };

  static jpeg_lib_handle *lib;

public:
  static bool is_usable ();

  explicit jpegstream (filebuf *fb);
  explicit jpegstream (const char *pathname,
		       ios_base::openmode mode = (ios_base::in |
						  ios_base::out));
  virtual ~jpegstream ();

  virtual void open (const char *pathname, ios_base::openmode mode);
  virtual imgstream& write (const char_type *line, std::streamsize n);

  virtual imgstream& size (size_type h_sz, size_type v_sz);
  virtual imgstream& colour (colour_space space);

private:
#ifdef HAVE_JPEGLIB_H
  static void error_exit (j_common_ptr info);
#endif

  void init ();

  imgstream& write_init ();

};

}	// namespace iscan

#endif	// iscan_jpegstream_hh_included
