//  pnmstream.hh -- 
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

#ifndef iscan_pnmstream_hh_included
#define iscan_pnmstream_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#include "imgstream.hh"

namespace iscan
{

class pnmstream : public imgstream
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

  std::string _magic;

  static const int _max_val_cap = 65536;
               int _max_val;

public:
  static bool is_usable ();

  explicit pnmstream (filebuf *fb);
  explicit pnmstream (const char *pathname,
		      ios_base::openmode mode = (ios_base::in |
						 ios_base::out));
  virtual ~pnmstream ();

  virtual imgstream& write (const char_type *line, std::streamsize n);

  virtual imgstream& depth (size_type bits);
  virtual imgstream& colour (colour_space space);

private:
  imgstream& write_header ();

  void check_sanity () const;
};

}	// namespace iscan

#endif	// iscan_pnmstream_hh_included
