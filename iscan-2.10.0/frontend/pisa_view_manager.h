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

#ifndef ___PISA_VIEW_MANAGER_H
#define ___PISA_VIEW_MANAGER_H

#include "pisa_enums.h"

#include "pisa_settings.h"

#include "pisa_scan_manager.h"

#include "pisa_main_window.h"
#include "pisa_preview_window.h"
#include "pisa_progress_window.h"
#include "pisa_image_controls.h"
#include "pisa_gamma_correction.h"
#include "pisa_configuration.h"
#include "pisa_error.h"
#include "pisa_scan_selector.h"

#include "file-selector.h"

class view_manager
{
 public:
  
  // operation
  static int	create_view_manager ( int argc, char * argv [ ] );
  
  void	release_view_manager ( void );
  
  int	main ( void );

  void	init ( void );
  void	destroy ( void );

  GtkWidget *	create_window ( pisa_window_id id );
  int	close_window ( pisa_window_id id, int destroy_flag );
  void *	get_window_cls ( pisa_window_id id );

  scan_manager *	get_scan_manager ( void ) { return m_scanmanager_cls; }

  void	sensitive ( void );

  int	is_gimp ( void );

  void	start_scan ( void );

  int	update_lut (void);
  int	update_lut (long i);
  int	update_lut (marquee *m);

  void	change_document_source ( void );
  void	set_device ( char * name );
  char * get_device_name() const;

  // attribute
  settings	m_set;

 private:

  // operation
  void	open_device ( void );
  void	close_device ( void );

  void	load_preference ( void );
  void	save_preference ( void );

  int	init_img_info ( pisa_option_type option, pisa_film_type film );

  int	init_scan_param ( scan_parameter * param, long i );
  void	update_document_source ( scan_parameter * param );
  void	update_image_type ( scan_parameter * param );
  void	update_scan_area ( scan_parameter * param, marquee * marq );
  void	update_color_table ( scan_parameter * param, marquee * marq );
  void	update_focus ( scan_parameter * param, marquee * marq );
  void	update_imgprocess ( scan_parameter * param, marquee * marq );

  int	do_scan_file( scan_parameter *param, file_selector *fs = 0,
		      bool first_time_around = true );
  int	do_scan_printer ( scan_parameter * param, int fd = -1,
			  bool first_time_around = true );
  int	do_scan_gimp ( scan_parameter * param,
		       bool first_time_around = true );

  bool	scan_file( scan_parameter * param,
		   const char *filename, int *cancel,
		   bool first_time_around = true );
  bool  scan_gimp( scan_parameter *param, int *cancel,
		   bool first_time_around = true );
  int	dialog_reply( const pisa_error& err ) const;

  pisa_file_type get_file_type (const char * filename);
  int   get_temp_filename (char **filename);
  int	check_overwrite( const char *regexp );
  void	regerror( int code, regex_t *regex );

  // attribute
  scan_manager		* m_scanmanager_cls;

  main_window		* m_main_cls;
  preview_window	* m_prev_cls;
  image_controls	* m_imgctrl_cls;
  gamma_correction	* m_gamma_cls;
  config_window		* m_config_cls;
  file_selector		* m_filsel_cls;
  scan_selector		* m_scansel_cls;

  progress_window *_feedback;
};

extern view_manager	* g_view_manager;

#endif // ___PISA_VIEW_MANAGER_H

