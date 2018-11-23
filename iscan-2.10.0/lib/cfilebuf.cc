//  cfilebuf.cc -- a file buffer wrapped around C's FILE * type
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
//
//  As a special exception, you may use this file as part of any free
//  software library without restriction.
//  Specifically, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and
//  link it with other files to produce an executable, this file does
//  not by itself cause the resulting executable to be covered by the
//  GNU General Public License.
//  This exception does not however invalidate any other reasons why
//  the executable file might be covered by the GNU General Public
//  License.
//  If you modify this file, you may remove this exception from your
//  version of the file, but are not required to do so.

#include "cfilebuf.hh"

namespace iscan
{

template <class char_t, class traits>
basic_cfilebuf<char_t, traits>::basic_cfilebuf (FILE *fp)
  : _file (fp), _mine (false)
{
  // nothing left to do
}

template <class char_t, class traits>
basic_cfilebuf<char_t, traits>::~basic_cfilebuf ()
{
  if (!is_open ()) {
    return;
  }
  fflush (_file);
  close ();
}

template <class char_t, class traits>
bool
basic_cfilebuf<char_t, traits>::is_open () const
{
  return 0 != _file; 
}

template <class char_t, class traits>
basic_cfilebuf<char_t, traits> *
basic_cfilebuf<char_t, traits>::open (const char *pathname,
				      ios_base::openmode mode)
{
  if (is_open ()) {
    return 0;
  }

  std::string c_mode = c_openmode (mode);
  if (c_mode.empty ()) {	// mode combination not allowed
    return 0;
  }

  _file = fopen (pathname, c_mode.c_str ());
  if (!_file) {
    return 0;
  }

  if (mode & ios_base::ate) {
    seekpos (seekoff (0, std::basic_ios<char_t, traits>::end));
  }

  _mine = true;
  return this;
}

template <class char_t, class traits>
basic_cfilebuf<char_t, traits> *
basic_cfilebuf<char_t, traits>::close ()
{
  if (!_mine || !fclose (_file)) {
    return 0;
  }
  return this;
}

template <class char_t, class traits>
basic_cfilebuf<char_t, traits>::operator FILE * ()
{
  return _file;
}

template <>
basic_cfilebuf<char>::int_type
basic_cfilebuf<char>::underflow ()
{
  int_type c = getc (_file);
  if (c != traits_type::eof ()) {
    ungetc (c, _file);
  }
  return c;
}

template <>
basic_cfilebuf<wchar_t>::int_type
basic_cfilebuf<wchar_t>::underflow ()
{
  int_type c = getwc (_file);
  if (c != traits_type::eof ()) {
    ungetc (c, _file);
  }
  return c;
}

template <>
basic_cfilebuf<char>::int_type
basic_cfilebuf<char>::uflow ()
{
  return getc (_file);
}

template <>
basic_cfilebuf<wchar_t>::int_type
basic_cfilebuf<wchar_t>::uflow ()
{
  return getwc (_file);
}

template <class char_t, class traits>
std::streamsize
basic_cfilebuf<char_t, traits>::xsgetn (char_type *s,
					std::streamsize n)
{
  return fread (s, sizeof (char_type), n, _file);
}

template <>
basic_cfilebuf<char>::int_type
basic_cfilebuf<char>::pbackfail (int_type c)
{
  return (c != traits_type::eof ()
	  ? ungetc (c, _file) : traits_type::eof ());
}

template <>
basic_cfilebuf<wchar_t>::int_type
basic_cfilebuf<wchar_t>::pbackfail (int_type c)
{
  return (c != traits_type::eof ()
	  ? ungetwc (c, _file) : traits_type::eof ());
}

template <>
basic_cfilebuf<char>::int_type
basic_cfilebuf<char>::overflow  (int_type c)
{
  return (c != traits_type::eof ()
	  ? fputc (c, _file) : traits_type::eof ());
}

template <>
basic_cfilebuf<wchar_t>::int_type
basic_cfilebuf<wchar_t>::overflow  (int_type c)
{
  return (c != traits_type::eof ()
	  ? fputwc (c, _file) : traits_type::eof ());
}

template <class char_t, class traits>
std::streamsize
basic_cfilebuf<char_t, traits>::xsputn (const char_type *s,
					std::streamsize n)
{
  return fwrite (s, sizeof (char_type), n, _file);
}

template <class char_t, class traits>
typename basic_cfilebuf<char_t, traits>::pos_type
basic_cfilebuf<char_t, traits>::seekoff (off_type off,
					 ios_base::seekdir way,
					 ios_base::openmode)
{
  fseek (_file, off, way);
  return ftell (_file);
}

template <class char_t, class traits>
typename basic_cfilebuf<char_t, traits>::pos_type
basic_cfilebuf<char_t, traits>::seekpos (pos_type pos,
					 ios_base::openmode mode)
{
  fseek (_file, pos, std::basic_ios<char_t, traits>::beg);
  return ftell (_file);
}

template <class char_t, class traits>
int
basic_cfilebuf<char_t, traits>::sync ()
{
  return fflush (_file);
}

template <class char_t, class traits>
std::string
basic_cfilebuf<char_t, traits>::c_openmode (ios_base::openmode mode)
{
  bool b = mode & std::ios_base::binary;
  bool i = mode & std::ios_base::in;
  bool o = mode & std::ios_base::out;
  bool t = mode & std::ios_base::trunc;
  bool a = mode & std::ios_base::app;

  std::string result = "";

  if ( i && !o && !t && !a) result = "r";
  if ( i &&  o && !t && !a) result = "r+";
  if (!i &&  o &&  t && !a) result = "w";
  if ( i &&  o &&  t && !a) result = "w+";
  if (!i &&  o && !t &&  a) result = "a";
  if ( i &&  o && !t &&  a) result = "a+";

  if (b && !result.empty ())
    result += "b";

  return result;
}

// force a single location of code for compilation by explicit
// template instantiation, avoiding all kinds of nasty linking
// problems
template class basic_cfilebuf< char  >;
template class basic_cfilebuf<wchar_t>;

}	// namespace iscan
