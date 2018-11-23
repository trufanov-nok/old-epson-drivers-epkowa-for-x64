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

#ifndef ___PISA_MAIN_WINDOW_H
#define ___PISA_MAIN_WINDOW_H

#include <gtk/gtk.h>
#include "pisa_enums.h"

class main_window
{
 public:

  // operation
  int	init ( void );
  GtkWidget *	create_window ( GtkWidget * parent = 0 );
  int	close_window ( int destroy );
  void	sensitive ( int is_prev_img );
  void  enable_start_button (bool yes);

  GtkWidget *	get_widget ( void ) { return m_widget [ WG_MAIN_TOP ]; }

private:

  // operation
  GtkWidget *	create_left_area ( void );
  GtkWidget *	create_preview_button ( void );
  GtkWidget *	create_preview_window ( void );
  GtkWidget *	create_scan_button ( void );
  GtkWidget *	create_scan_label ( void );
  GtkWidget *	create_right_area ( void );
  GtkWidget *	create_main_tab ( void );
  GtkWidget *	create_document_tab ( void );
  GtkWidget *	create_target ( void );
  GtkWidget *	create_focus ( void );
  GtkWidget *	create_penguin ( void );
  GtkWidget *	create_adjust_tab ( void );
  GtkWidget *	create_close_button ( void );

  void	sensitive_image_type ( void );
  void	sensitive_target ( void );
  void	sensitive_resolution ( void );
  void	sensitive_scale ( void );
  void	sensitive_focus ( void );
  void	sensitive_usm ( void );

  long	get_max_scale ( void );
  long	get_max_enable_res ( void );
  void	calc_scale_limit ( long * min_scale, long * max_scale );

  // attribute
  GtkWidget	* m_widget [ WG_MAIN_NUM ];
};

#endif // ___PISA_MAIN_WINDOW_H
