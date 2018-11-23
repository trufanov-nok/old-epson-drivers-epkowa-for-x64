//  filter.hh -- 
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

//  FIXME: replace with an exception to link with libs implementing
//  the filter API instead

//  As a special exception, the copyright holders give permission
//  to link the code of this program with the esmod library and
//  distribute linked combinations including the two.  You must obey
//  the GNU General Public License in all respects for all of the
//  code used other then esmod.

#ifndef iscan_filter_hh_included
#define iscan_filter_hh_included

#ifndef __cplusplus
#error "This is a C++ header file; use a C++ compiler to compile it."
#endif

#include <cstddef>
#include <iostream>		// for subclass debugging output

// FIXME: remove dependency on these headers ASAP so as to not burden
//        the maintainer of the closed source library with syncing of
//        the relevant data structures.  Yes, this probably means that
//	  we get very long argument lists ... too bad.
#include "pisa_structs.h"
#include "pisa_esmod_structs.h"
#include "pisa_settings.h"

namespace iscan
{

class filter
{
public:
  typedef unsigned char byte_type;
  typedef size_t        size_type;

  virtual ~filter () {};

  //! Returns a block of n bytes of post-filter data.
  virtual filter& getblock (      byte_type *block, size_type n) = 0;
  //! Feeds the filter a block of n bytes of data.
  virtual filter& putblock (const byte_type *block, size_type n) = 0;
  //! Combines putblock() and getblock() in a single call.
  virtual filter& exec     (const byte_type *i_block, size_type i_n,
			          byte_type *o_block, size_type o_n)
  {
           putblock (i_block, i_n);
    return getblock (o_block, o_n);
  }

  //! Returns number of additional input lines needed for out_lines of output.
  virtual size_type get_line_quote (size_type out_lines)
  { return out_lines; };

protected:
  void *_hidden_data;
  filter () {};

private:			// undefined to prevent copying
  filter (const filter&);
  filter& operator= (const filter&);
};

class focus : public filter
{
public:
           focus (const pisa_image_info& parms);
           focus (struct sharp_img_info parms);
  virtual ~focus ();

  void set_parms (size_type resolution, bool film_type, bool is_dumb,
		  unsigned long *strength,
		  unsigned long *radius,
		  unsigned long *clipping);

  virtual filter& getblock (      byte_type *block, size_type n);
  virtual filter& putblock (const byte_type *block, size_type n);
  virtual filter& exec     (const byte_type *i_block, size_type i_n,
			          byte_type *o_block, size_type o_n);

  virtual size_type get_line_quote (size_type out_lines);
};

class moire : public filter
{
public:
           moire (struct moire_img_info parms, bool is_dumb);
  virtual ~moire ();

  virtual filter& getblock (      byte_type *block, size_type n);
  virtual filter& putblock (const byte_type *block, size_type n);
  virtual filter& exec     (const byte_type *i_block, size_type i_n,
			          byte_type *o_block, size_type o_n);

  static  size_type get_res_quote  (size_type out_res, bool is_dumb);
  virtual size_type get_line_quote (size_type out_lines);
};

class scale : public filter
{
public:
           scale (struct resize_img_info parms);
  virtual ~scale ();

  virtual filter& getblock (      byte_type *block, size_type n);
  virtual filter& putblock (const byte_type *block, size_type n);
  virtual filter& exec     (const byte_type *i_block, size_type i_n,
			          byte_type *o_block, size_type o_n);

  virtual size_type get_line_quote (size_type out_lines);
};

// WARNING: These quite likely modify global state in libesmod.
void auto_expose (char, char, const pisa_image_info *, const _rectL *,
		  marquee *, bool, bool is_dumb);
void build_LUT (settings *, marquee *, bool is_dumb);

}	// namespace iscan

#endif	// iscan_filter_hh_included
