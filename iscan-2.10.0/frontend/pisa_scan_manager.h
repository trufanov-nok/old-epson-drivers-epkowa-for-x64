/* 
   SANE EPSON backend
   Copyright (C) 2001, 2005 SEIKO EPSON Corporation

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

#ifndef ___PISA_SCAN_MANAGER_H
#define ___PISA_SCAN_MANAGER_H

#include "filter.hh"

#include "pisa_sane_scan.h"
#include "pisa_esmod_structs.h"

class scan_manager : public sane_scan
{
 public:

  // operator
  void	open_device ( char * name = 0 );

  void init_scan (scan_parameter *param, int *width, int *height,
		  bool reset_params = true);
  void  adjust_scan_param (long *resolution, long *scale);  

  void	acquire_image ( unsigned char * img,
			int row_bytes,
			int height,
			int cancel );
  void	finish_acquire ( void );
  
  const scanner_info * get_scanner_info ( void ) { return & m_scanner_info; }

 private:

  typedef struct _IMAGE_INFO {
    unsigned char* pImg_Buf;
    long	   Img_Width;
    long	   Img_Height;
    unsigned long  Img_RowBytes;
    short	   BitsPerPixel;
  } IMAGE_INFO, *LPIMAGE_INFO;

  
  // operation
  int	init_img_process_info ( scan_parameter * param );
  int	init_zoom ( resize_img_info * info, scan_parameter * param );
  int	init_moire ( moire_img_info * info, scan_parameter * param );
  int	init_sharp ( sharp_img_info * info, scan_parameter * param );

  int	modify_img_info ( int * width, int * height );

  int	create_img_cls ( void );
  int	release_memory ( void );

  void	set_img_info ( LPIMAGE_INFO in_img_info,
		       LPIMAGE_INFO out_img_info,
		       const img_size & size );

  // FIXME: the following block of member functions should not be the
  //        responsibility of the scan_manager.  They can be removed
  //        once the filter class hierarchy has support for chaining
  //        of filters and this class no longer needs to baby sit the
  //        data transfers.
  int	get_send_in_line ( int out_line );
  int	image_process ( unsigned char * in_img, unsigned char * out_img );

  SANE_Int get_valid_resolution (int min_res);

  // attribute
  scanner_info	m_scanner_info;
  
  // for image module
  long			m_resize;
  long			m_moire;
  long			m_sharp;

  resize_img_info	m_resize_info;
  moire_img_info	m_moire_info;
  sharp_img_info	m_sharp_info;

  iscan::scale		* m_resize_cls;
  iscan::moire		* m_moire_cls;
  iscan::focus		* m_sharp_cls;

  enum { _IN, _OUT };
  IMAGE_INFO            * m_resize_img_info;
  IMAGE_INFO            * m_moire_img_info;
  IMAGE_INFO            * m_sharp_img_info;

};

#endif // ___PISA_SCAN_MANAGER_H

