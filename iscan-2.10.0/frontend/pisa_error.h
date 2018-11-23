/* 
   SANE EPSON backend
   Copyright (C) 2001 SEIKO EPSON Corporation

   Date         Author      Reason
   06/01/2001   N.Sasaki    New

   This file is part of the `iscan' program.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   As a special exception, the copyright holders give permission
   to link the code of this program with the esmod library and
   distribute linked combinations including the two.  You must obey
   the GNU General Public License in all respects for all of the
   code used other then esmod.
*/

#ifndef ___PISA_ERROR_H
#define ___PISA_ERROR_H

#include <sane/sane.h>

typedef enum
{
  PISA_ERR_SUCCESS,
  PISA_ERR_PARAMETER,
  PISA_ERR_OUTOFMEMORY,
  PISA_ERR_CONNECT,
  PISA_ERR_UNSUPPORT,
  PISA_ERR_AREALARGE,
  PISA_ERR_FILENAME,
  PISA_ERR_FILEOPEN,
  PISA_ERR_OVERWRITE,
  PISA_ERR_MRRESTOOHIGH,
  // Make sure we can use the SANE status symbols.  To avoid name
  // clashes, replace SANE with PISA and put them in another range.
  PISA_STATUS_GOOD = 0xff00,	//  0 Operation completed succesfully.
  PISA_STATUS_UNSUPPORTED,	//  1 Operation is not supported.
  PISA_STATUS_CANCELLED,	//  2 Operation was cancelled.
  PISA_STATUS_DEVICE_BUSY,	//  3 Device is busy---retry later.
  PISA_STATUS_INVAL,		//  4 Data or argument is invalid.
  PISA_STATUS_EOF,		//  5 No more data available (end-of-file).
  PISA_STATUS_JAMMED,		//  6 Document feeder jammed.
  PISA_STATUS_NO_DOCS,		//  7 Document feeder out of documents.
  PISA_STATUS_COVER_OPEN,	//  8 Scanner cover is open.
  PISA_STATUS_IO_ERROR,		//  9 Error during device I/O.
  PISA_STATUS_NO_MEM,		// 10 Out of memory.
  PISA_STATUS_ACCESS_DENIED,	// 11 Access to resource has been denied.

  PISA_ERR_INVALID_ERROR_ID
} pisa_error_id;

class pisa_error
{
 public:

  // constructor
  pisa_error ( pisa_error_id id );
  pisa_error ( SANE_Status id );

  pisa_error_id get_error_id ( void ) const { return remap(); };
  const char * get_error_string ( void ) const;

 private:

  // attribute
  pisa_error_id	m_id;

  pisa_error_id remap() const;
};

#endif // ___PISA_ERROR_H

