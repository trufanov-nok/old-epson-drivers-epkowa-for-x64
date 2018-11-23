//  pngstream.cc -- 
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

#include "pngstream.hh"

namespace iscan
{

pngstream::png_lib_handle *pngstream::lib = NULL;
#ifdef HAVE_PNG_H
png_const_charp pngstream::version_string = PNG_LIBPNG_VER_STRING;
#endif

pngstream::pngstream (filebuf *fb)
  : imgstream (fb), _header_done (false)
{
  init ();
}

pngstream::pngstream (const char *pathname, ios_base::openmode mode)
  : imgstream (pathname, mode), _header_done (false)
{
  init ();
}

pngstream::~pngstream ()
{
#ifdef HAVE_PNG_H
  set_error_handler (_png, _info);

  lib->write_flush (_png);	// just in case
  if (_png->num_rows == _png->flush_rows) {
    lib->write_end (_png, _info);
  }
  lib->destroy_write_struct (&_png, &_info);
#endif
}

void
pngstream::init ()
{
  if (!is_usable ()) {
    throw std::runtime_error (lib->message);
  }

#ifdef HAVE_PNG_H
  _png = lib->create_write_struct (version_string,
				   NULL, NULL, NULL);
  if (!_png) {
    throw std::bad_alloc ();
  }

  _info = lib->create_info_struct (_png);
  if (!_info) {
    lib->destroy_write_struct (&_png, NULL);
    throw std::bad_alloc ();
  }
#endif // HAVE_PNG_H
}

#ifdef HAVE_PNG_H
#define funcsym(name) \
	lib->name \
	  = ((pngstream::png_lib_handle::name##_f) \
	     imgstream::dlsym (lib->lib, "png_"#name));
#endif

bool
pngstream::is_usable ()
{
  if (lib) {
    return lib->is_usable;
  }

  lib = new (std::nothrow) png_lib_handle ();
  if (!lib) {
    return false;
  }

  lib->is_usable = false;
#ifdef HAVE_PNG_H
  try {
    // Some PNG libraries are *not* linked with libz which leads to a
    // "file not found" error when dlopen()ing.  We just dlopen() the
    // libz library unconditionally to work around this.
    imgstream::dlopen ("libz");
    lib->lib = imgstream::dlopen ("libpng");
  } catch (std::runtime_error& e) {
    lib->message = e.what ();
    return lib->is_usable;
  }

  funcsym (access_version_number);
  funcsym (create_write_struct);
  funcsym (create_info_struct);
  funcsym (destroy_write_struct);
  funcsym (init_io);
  funcsym (set_IHDR);
  funcsym (set_pHYs);
  funcsym (set_invert_mono);
  funcsym (write_info);
  funcsym (write_row);
  funcsym (write_flush);
  funcsym (write_end);

  if (lib->access_version_number
      && lib->create_write_struct
      && lib->create_info_struct
      && lib->destroy_write_struct
      && lib->init_io
      && lib->set_IHDR
      && lib->set_pHYs
      && lib->set_invert_mono
      && lib->write_info
      && lib->write_row
      && lib->write_flush
      && lib->write_end) {
    lib->is_usable = (PNG_LIBPNG_VER <= lib->access_version_number ());
  }

  if (!lib->is_usable) {
    imgstream::dlclose (lib->lib);
  }
#endif // HAVE_PNG_H

  return lib->is_usable;
}

#ifdef HAVE_PNG_H
#undef funcsym
#endif

imgstream&
pngstream::write (const char_type *line, std::streamsize n)
{
  if (!_header_done) {
    write_header ();
  }
#ifdef HAVE_PNG_H
  set_error_handler (_png, _info);

  lib->write_row (_png, (png_byte *) line);
#endif

  return *this;
}

// use dots per meter (dpm) instead of dpi internally
imgstream&
pngstream::resolution (size_type hres, size_type vres)
{
  _hres = size_t (100 / 2.54 * hres + 0.5);
  _vres = size_t (100 / 2.54 * vres + 0.5);

  return *this;
}

imgstream&
pngstream::colour (colour_space space)
{
  imgstream::colour (space);

#ifdef HAVE_PNG_H
  switch (_cspc) {
  case mono:
    _colour_type = PNG_COLOR_TYPE_GRAY;
    _bits = 1;
    lib->set_invert_mono (_png);
    break;
  case grey:
    _colour_type = PNG_COLOR_TYPE_GRAY;
    break;
  case RGB:
    _colour_type = PNG_COLOR_TYPE_RGB;
    break;
  default:
    // FIXME: implement this
    ;
  }
#endif // HAVE_PNG_H

  return *this;
}

imgstream&
pngstream::write_header ()
{
#ifdef HAVE_PNG_H
  set_error_handler (_png, _info);

  lib->init_io (_png, *this);

  lib->set_IHDR (_png, _info, _h_sz, _v_sz, _bits, _colour_type,
		 PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT,
		 PNG_FILTER_TYPE_DEFAULT);

  lib->set_pHYs (_png, _info, _hres, _vres, PNG_RESOLUTION_METER);

  lib->write_info (_png, _info);
#endif // HAVE_PNG_H

  _header_done = true;
  return *this;
}

}	// namespace iscan
