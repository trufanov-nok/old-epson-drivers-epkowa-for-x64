//  jpegstream.cc -- 
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

#include "jpegstream.hh"

#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

namespace iscan
{

jpegstream::jpeg_lib_handle *jpegstream::lib = NULL;

jpegstream::jpegstream (filebuf *fb)
  : imgstream (fb), _scanline (NULL), _write_init_done (false)
{
  init ();
}

jpegstream::jpegstream (const char *pathname, ios_base::openmode mode)
  : imgstream (pathname, mode | ios_base::binary),
    _scanline (NULL), _write_init_done (false)
{
  init ();
}

jpegstream::~jpegstream ()
{
  delete [] _scanline;

#ifdef HAVE_JPEGLIB_H
  lib->finish_compress (&_info);
  lib->destroy_compress (&_info);
#endif
}

#ifdef HAVE_JPEGLIB_H
void
jpegstream::error_exit (jpeg_common_struct *info)
{
  jpegstream::lib->destroy (info);
}
#endif

void
jpegstream::init ()
{
  if (!is_usable ()) {
    throw std::runtime_error (lib->message);
  }

#ifdef HAVE_JPEGLIB_H
  // set up JPEG library default error handlers first, then override
  // error handling for fatal errors (because the default would just
  // end up calling exit())
  _info.err = lib->std_error (&_err);
  _err.error_exit = error_exit;
#ifndef jpeg_create_compress
  lib->create_compress (&_info);
#else
  lib->CreateCompress (&_info, JPEG_LIB_VERSION,
		       (size_t) sizeof (struct jpeg_compress_struct));
#endif
  lib->stdio_dest (&_info, *this);
#endif // HAVE_JPEGLIB_H
}

#ifdef HAVE_JPEGLIB_H
#define funcsym(name) \
	lib->name \
	  = ((jpegstream::jpeg_lib_handle::name##_f) \
	     imgstream::dlsym (lib->lib, "jpeg_"#name));
#endif

bool
jpegstream::is_usable ()
{
  if (lib) {
    return lib->is_usable;
  }

  lib = new (std::nothrow) jpeg_lib_handle ();
  if (!lib) {
    return false;
  }

  lib->is_usable = false;
#ifdef HAVE_JPEGLIB_H
  try {
    lib->lib = imgstream::dlopen ("libjpeg");
  } catch (std::runtime_error& e) {
    lib->message = e.what ();
    return lib->is_usable;
  }

#ifndef jpeg_create_compress
  funcsym (create_compress);
#else
  funcsym (CreateCompress);
#endif
  funcsym (finish_compress);
  funcsym (destroy_compress);
  funcsym (destroy);
  funcsym (stdio_dest);
  funcsym (std_error);
  funcsym (write_scanlines);
  funcsym (set_defaults);
  funcsym (start_compress);

  lib->is_usable = (
#ifndef jpeg_create_compress
		    lib->create_compress
#else
		    lib->CreateCompress
#endif
		    && lib->finish_compress
		    && lib->destroy_compress
		    && lib->destroy
		    && lib->stdio_dest
		    && lib->std_error
		    && lib->write_scanlines
		    && lib->set_defaults
		    && lib->start_compress);

  if (!lib->is_usable) {
    imgstream::dlclose (lib->lib);
  }
#endif // HAVE_JPEGLIB_H

  return lib->is_usable;
}

#ifdef HAVE_JPEGLIB_H
#undef funcsym
#endif

void
jpegstream::open (const char *pathname, ios_base::openmode mode)
{
  imgstream::open (pathname, mode | ios_base::binary);
#ifdef HAVE_JPEGLIB_H
  lib->stdio_dest (&_info, *this);
#endif
}

imgstream&
jpegstream::write (const char_type *line, std::streamsize n)
{
  if (!_write_init_done) {
    write_init ();
  }

#ifdef HAVE_JPEGLIB_H
  if (!_scanline) {
    lib->write_scanlines (&_info, (JSAMPLE **) &line, 1);
  } else {
    // FIXME: this assumes that _bits == 1, whereas the condition for
    // _scanline to be true, see write_init (), merely requires that
    // _bits != 8.  Boundary alignment should be provided by the base
    // class so all subclasses that need it can use it.  BTW, I think
    // that there are more incorrect assumptions in all the imgstream
    // classes wrt to boundary alignment but for now everything works
    // as far as iscan is concerned.
    for (unsigned int i = 0; i < _h_sz; ++i) {
      div_t index = div (i, 8 * sizeof (JSAMPLE));
      int offset = 8 * sizeof (JSAMPLE) - 1 - index.rem;
      _scanline[i] = ((line[index.quot] & (1 << offset))
		      ? 0 : ~0);
    }
    lib->write_scanlines (&_info, (JSAMPLE **) &_scanline, 1);
  }
#endif // HAVE_JPEGLIB_H

  return *this;
}

imgstream&
jpegstream::size (size_type h_sz, size_type v_sz)
{
  imgstream::size (h_sz, v_sz);

#ifdef HAVE_JPEGLIB_H
  _info.image_width  = _h_sz;
  _info.image_height = _v_sz;
#endif

  return *this;
}

imgstream&
jpegstream::colour (colour_space space)
{
  imgstream::colour (space);

#ifdef HAVE_JPEGLIB_H
  switch (space) {
  case mono:
#if 0
    throw std::logic_error
      (_("The JPEG format is meant for full-colour or greyscale images of"
	 "'realistic' scenes.  An image needs about 16 grey levels before"
	 "saving it in JPEG format is useful.  The JPEG standard does not"
	 "provide for monochrome images, so one can only approximate them"
	 "as grey scale images with just two grey levels.\n"
	 "In short: saving monochrome images in JPEG format is stupid."));
    break;
#else
    // fall through
#endif
  case grey:
    _info.in_color_space = JCS_GRAYSCALE;
    _info.input_components = 1;
    break;
  case RGB:
    _info.in_color_space = JCS_RGB;
    _info.input_components = 3;
    break;
  default:
    // FIXME: implement this
    ;
  }
#endif // HAVE_JPEGLIB_H

  return *this;
}

imgstream&
jpegstream::write_init ()
{
#ifdef HAVE_JPEGLIB_H
  lib->set_defaults (&_info);
  lib->start_compress (&_info, true);

  if (mono == _cspc && 8 != _bits) {
    _scanline = new char_type[_h_sz];
  } else {
    _scanline = NULL;
  }
#endif // HAVE_JPEGLIB_H

  _write_init_done = true;
  return *this;
}

}	// namespace iscan
