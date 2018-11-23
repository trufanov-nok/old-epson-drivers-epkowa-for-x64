//  pnmstream.cc -- 
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

#include "pnmstream.hh"

namespace iscan
{

pnmstream::pnmstream (filebuf *fb)
  : imgstream (fb), _header_done (false)
{}

pnmstream::pnmstream (const char *pathname, ios_base::openmode mode)
  : imgstream (pathname, mode), _header_done (false)
{}

pnmstream::~pnmstream ()
{}

bool
pnmstream::is_usable ()
{
  return true;
}

imgstream&
pnmstream::write (const char_type *line, std::streamsize n)
{
  if (!_header_done) {
    write_header ();
  }
  imgstream_base_t::write (line, n);
  return *this;
}

imgstream&
pnmstream::depth (size_type bits)
{
  imgstream::depth (bits);

  switch (_bits) {
  case 1:
    _magic = "P4";
    _max_val = 2 - 1;
    break;
  case 8:
    _max_val = 256 - 1;
    break;
  case 16:
    _max_val = 65536 - 1;
    break;
  default:
    // FIXME: implement
    ;
  }
  return *this;
}

imgstream&
pnmstream::colour (colour_space space)
{
  imgstream::colour (space);

  switch (_cspc) {
  case mono:
    _magic = "P4";
    break;
  case grey:
    _magic = "P5";
    break;
  case RGB:
    _magic = "P6";
    break;
  default:
    // FIXME: implement
    ;
  }
  return *this;
}

imgstream&
pnmstream::write_header ()
{
  check_sanity ();
  *this << _magic << std::endl
	<< _h_sz << " " << _v_sz << std::endl;
  if ("P4" != _magic) {
    *this << _max_val << std::endl;
  }
  _header_done = true;
  return *this;
}

void
pnmstream::check_sanity () const
{
  if (!("P4" == _magic || "P5" == _magic || "P6" == _magic)) {
    throw;
  }
  if (!(_max_val < _max_val_cap)) {
    throw;
  }
  if (("P5" == _magic || "P6" == _magic)
      && !(255 == _max_val || 65525 == _max_val)) {
    throw;
  }
}

}	// namespace iscan
