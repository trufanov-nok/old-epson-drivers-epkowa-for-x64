//  imgstream.cc -- 
//  Copyright (C) 2004, 2005  SEIKO EPSON Corporation
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

#include <argz.h>

#include "imgstream.hh"

namespace iscan
{

#if __GLIBC_PREREQ(2, 10)
  typedef const dirent** dirtype;
#else
  typedef const void* dirtype;
#endif

imgstream::imgstream ()
  : imgstream_base_t (NULL), _fbuf (new filebuf (NULL)), _mine (true)
{
  rdbuf (_fbuf);
  init ();
}

imgstream::imgstream (FILE *fp)
  : imgstream_base_t (NULL), _fbuf (new filebuf (fp)), _mine (true)
{
  rdbuf (_fbuf);
  init ();
}

imgstream::imgstream (filebuf *fb)
  : imgstream_base_t (NULL), _fbuf (fb), _mine (false)
{
  rdbuf (_fbuf);
  init ();
}

imgstream::imgstream (const char * pathname, ios_base::openmode mode)
  : imgstream_base_t (NULL), _fbuf (new filebuf (NULL)), _mine (true)
{
  _fbuf->open (pathname, mode);
  rdbuf (_fbuf);
  init ();
}

void
imgstream::init ()
{
  exceptions (badbit | failbit);

  _h_sz = _v_sz = 0;
  _hres = _vres = 0;
  _bits = 0;
  _cspc = NONE;
}

imgstream::~imgstream ()
{
  if (_mine) {
    if (is_open ()) {
      close ();
    }
    delete _fbuf;
  }
}

bool
imgstream::is_open () const
{
  return _fbuf->is_open ();
}

void
imgstream::open (const char *pathname, ios_base::openmode mode)
{
  if (!_fbuf->open (pathname, mode)) {
    setstate (ios_base::failbit);
  }
}

void
imgstream::close ()
{
  if (!_fbuf->close ()) {
    setstate (ios_base::failbit);
  }
}

imgstream::filebuf *
imgstream::rdbuf () const
{
  return _fbuf;
}

imgstream::filebuf *
imgstream::rdbuf (filebuf *fb)
{
  return static_cast<filebuf *> (imgstream_base_t::rdbuf (fb));
}

imgstream::operator FILE * ()
{
  return *_fbuf;
}

imgstream&
imgstream::size (size_type h_sz, size_type v_sz)
{
  _h_sz = h_sz;
  _v_sz = v_sz;
  return *this;
}

imgstream&
imgstream::resolution (size_type hres, size_type vres)
{
  _hres = hres;
  _vres = vres;
  return *this;
}

imgstream&
imgstream::depth (size_type bits)
{
  _bits = bits;
  return *this;
}

imgstream&
imgstream::colour (colour_space space)
{
  _cspc = space;
  return *this;
}

imgstream::dl_handle
imgstream::dlopen (const char *libname) throw (std::runtime_error)
{
  if (0 != lt_dlinit ()) {
    throw std::runtime_error (lt_dlerror ());
  }

  dl_handle lib = lt_dlopenext (libname);
  if (!lib) {
    const char *message = lt_dlerror ();
    lib = find_dlopen (libname);
    if (!lib) {
      lt_dlexit ();
      throw std::runtime_error (message);
    }
  }
  return lib;
}

imgstream::dl_ptr
imgstream::dlsym (dl_handle lib, const char *funcname)
{
  return lt_dlsym (lib, funcname);
}

int
imgstream::dlclose (dl_handle lib)
{
  return lt_dlclose (lib);
}

static int reversionsort (dirtype, dirtype);
int selector (const dirent *);
				// forward declarations

//!
/*! A number of distributions seems to have switched to a policy where
    the lib*.so files are provided by their -devel packages.  Moreover,
    the typical workstation install does not include such packages and
    lt_dlopenext() will understandably have trouble finding your lib*.

    This function is a fallback for such cases.  It will look for your
    library in the exact same places as lt_dlopenext(), but with this
    difference that it will try to open any file that matches lib*.so,
    starting with the one with the highest version number.

    Actually, it is just as smart lt_dlopenext() and uses the correct
    shared library extension for your platform.  However, it does not
    try libtool's .la extension.

    The general policy for memory allocation and access problems is to
    ignore them and muddle on or return the current result rightaway.

    This function returns NULL if no suitable library could be found
    and a handle to library otherwise.
 */
imgstream::dl_handle
imgstream::find_dlopen (const char *libname)
{
  using std::bad_alloc;

  dl_handle result = NULL;

  try {				// prepping the selector()
    char *name = new char[ strlen (libname) + strlen (LTDL_SHLIB_EXT) + 1 ];
    name = strcpy (name, libname);
    name = strcat (name, LTDL_SHLIB_EXT);

    _libname = name;		// deleting _libname below, never mind
				// that name goes out of scope here
  }
  catch (bad_alloc& oops) {
    return result;
  }

  char   *pathz  = NULL;
  size_t  length = 0;
  bool    is_pathz_ok = true;
  {				// set up a library search path like
				// that used by lt_dlopen()
    int delimiter = ':';

    const char *path = NULL;

    if ((path = lt_dlgetsearchpath ())
	&& 0 != argz_add_sep (&pathz, &length, path, delimiter)) {
      is_pathz_ok = false;
    }
    if ((path = getenv ("LTDL_LIBRARY_PATH"))
	&& 0 != argz_add_sep (&pathz, &length, path, delimiter)) {
      is_pathz_ok = false;
    }
    if ((path = getenv (LTDL_SHLIBPATH_VAR))
	&& 0 != argz_add_sep (&pathz, &length, path, delimiter)) {
      is_pathz_ok = false;
    }
    if ((path = LTDL_SYSSEARCHPATH)
	&& 0 != argz_add_sep (&pathz, &length, path, delimiter)) {
      is_pathz_ok = false;
    }
  }

  if (is_pathz_ok) {				// go fetch!
    const char *dir_name = NULL;
    while (!result
	   && (dir_name = argz_next (pathz, length, dir_name))) {
      struct dirent **match;
      int count = scandir (dir_name, &match, selector, reversionsort);

      for (int i = 0; !result && i < count; ++i) {

	const char *file_name = match[i]->d_name;
	try {
	  char *abs_file_name
	    = new char[ strlen (dir_name) + strlen ("/") + strlen (file_name)
			+ 1 ];
	  strcpy (abs_file_name, dir_name);
	  strcat (abs_file_name, "/");
	  strcat (abs_file_name, file_name);

	  result = lt_dlopen (abs_file_name);
	  delete abs_file_name;
	}
	catch (bad_alloc& oops) {
	  // just ignore and (try to) continue with the next match
	}
      }
      free (match);		// malloc'd by scandir()
    }
  }

  delete _libname;		// we new'd a name for our selector()
  free (pathz);			// malloc'd by argz_add_sep()

  return result;
}

//! Library name we are looking for.
/*! The scandir() API does not allow for passing arbitrary data to the
    selector().  We use this variable to work around that limitation.

    Note that this makes users of selector() thread unsafe.
 */
const char *imgstream::_libname = NULL;

//! Selects relevant library filenames.
int
selector (const dirent *dir)
{
  return (0 == strncmp (dir->d_name, imgstream::_libname,
			strlen (imgstream::_libname)));
}

//! The C library's versionsort() function in reverse.
static
int
reversionsort (dirtype a, dirtype b)
{
  return versionsort (b, a);
}

}	// namespace iscan
