//  cfilebuf.hh -- a file buffer wrapped around C's FILE * type
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

#ifndef iscan_cfilebuf_hh_included
#define iscan_cfilebuf_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#include <cstdio>
#include <cwchar>
#include <ios>
#include <string>

namespace iscan
{
  using std::ios_base;

template <class char_t,
	  class traits = std::char_traits<char_t> >
class basic_cfilebuf : public std::basic_streambuf<char_t, traits>
{
public:
  typedef char_t char_type;
  typedef typename traits::int_type int_type;
  typedef typename traits::pos_type pos_type;
  typedef typename traits::off_type off_type;
  typedef traits traits_type;

private:
  FILE *_file;
  bool  _mine;		// whether the instance owns _file

public:
  explicit basic_cfilebuf (FILE *fp = NULL);
  virtual ~basic_cfilebuf ();

  bool is_open () const;

  basic_cfilebuf<char_t, traits> *
  open  (const char *pathname,
	 ios_base::openmode mode = (ios_base::in | ios_base::out));

  basic_cfilebuf<char_t, traits> *
  close ();

  operator FILE * ();

protected:
  // input
  virtual int_type underflow ();
  virtual int_type uflow     ();
  virtual std::streamsize xsgetn (char_type *s, std::streamsize n);

  // output
  virtual int_type pbackfail (int_type c = traits::eof);
  virtual int_type overflow  (int_type c = traits::eof);
  virtual std::streamsize xsputn (const char_type *s, std::streamsize n);

  // buffer positioning
  virtual pos_type seekoff (off_type off, ios_base::seekdir way,
			    ios_base::openmode mode = (ios_base::in |
						       ios_base::out));
  virtual pos_type seekpos (pos_type pos,
			    ios_base::openmode mode = (ios_base::in |
						       ios_base::out));
  virtual int sync ();

  std::string c_openmode (ios_base::openmode mode);
};

typedef basic_cfilebuf< char  >  cfilebuf;
typedef basic_cfilebuf<wchar_t> wcfilebuf;

}	// namespace iscan

#endif	// iscan_cfilebuf_hh_included
