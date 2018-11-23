/* pisa_sane_scan.h					-*- C++ -*-
   Copyright (C) 2001, 2004 SEIKO EPSON Corporation

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

#ifndef ___PISA_SANE_SCAN_H
#define ___PISA_SANE_SCAN_H

#ifdef __cplusplus
extern "C" {
#endif
#include <sane/sane.h>
#include <sane/saneopts.h>
#ifdef __cplusplus
}
#endif
#include "pisa_structs.h"

class sane_scan
{
 public:

  // operation
  void	init ( void );
  void	open_device ( scanner_info * info, char * name = 0 );
  void	close_device ( void );

  void	set_option ( pisa_option_type option );
  void	set_film_type ( pisa_film_type film );

  // information
  void	get_current_max_size ( double * width, double * height );
  void	get_color_profile ( double * coef );

  // scan
  void	set_scan_parameter ( const scan_parameter * param );
  void	start_scan ( int * width, int * height );
  void	acquire_image ( unsigned char * img,
			int row_bytes,
			int height,
			int cancel );

  bool is_button_pressed (void);
  bool area_is_too_large (const scan_parameter *param);

 private:

  // operation
  void	query_device ( char * device_name );

  long	get_max_resolution ( long cutoff = -1 );
  void	get_scanner_info ( scanner_info * info, char *name = 0 );

protected:
  int	is_activate ( char * option_name );
  int	get_option_id ( char * option_name );
  void	set_value ( char * option_name, void * value );
  void	get_value ( char * option_name, void * value );

private:
  void	set_focus ( long position );
  void	set_speed ( long speed );
  void	set_color_mode ( char pixeltype, char bitdepth );
  void	set_gamma_table ( const unsigned char * gamma_table );
  void	set_threshold ( long threshold );
  void	set_color_profile ( const double * coef );
  void	set_scan_resolution ( int resolution_x, int resolution_y );
  void	set_scan_zoom ( int zoom_x, int zoom_y );
  void	set_scan_area ( double offset_x, double offset_y,
			double width, double height );

  void	update_settings ( const scan_parameter * param );

  // attribute
protected:
  SANE_Handle		m_hdevice;
private:
  SANE_Parameters	m_sane_para;

  long			m_rows;
};
  

#endif // ___PISA_SANE_SCAN_H
