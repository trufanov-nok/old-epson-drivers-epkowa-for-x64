/* pisa_view_manager.cc
   Copyright (C) 2001, 2004, 2005 SEIKO EPSON Corporation

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

#include <config.h>

#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

/*------------------------------------------------------------*/
#include <gtk/gtk.h>
#ifndef HAVE_GTK_2
#include <gdk_imlib.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <locale.h>

/*------------------------------------------------------------*/
#include "pisa_view_manager.h"
#include "pisa_error.h"
#include "pisa_main.h"
#include "pisa_scan_tool.h"
#include "pisa_default_val.h"
#include "pisa_gimp.h"
#include "pisa_aleart_dialog.h"
#include "pisa_change_unit.h"
#include "pisa_preference.h"
#include "pisa_scan_selector.h"

#include "file-selector.h"
// FIXME: lump these together in ../lib/image-stream.hh or something
#include "../lib/imgstream.hh"
#include "../lib/pnmstream.hh"
#include "../lib/pngstream.hh"
#include "../lib/jpegstream.hh"
#include "../lib/colour.hh"

/*------------------------------------------------------------*/
view_manager * g_view_manager = 0;

/*------------------------------------------------------------*/
int view_manager::create_view_manager ( int argc, char * argv [ ] )
{
  ::gtk_set_locale ( );
  ::setlocale (LC_NUMERIC, "C");
  ::gtk_init ( & argc, & argv );
  
#ifndef HAVE_GTK_2
  ::gdk_imlib_init ( );
  ::gtk_widget_push_visual ( ::gdk_imlib_get_visual ( ) );
  ::gtk_widget_push_colormap ( ::gdk_imlib_get_colormap ( ) );
#endif
  
  try
    {
      if ( ::g_view_manager )
	throw pisa_error (  PISA_ERR_PARAMETER );

      ::g_view_manager = new view_manager;
      if ( ::g_view_manager == 0 )
	throw pisa_error (  PISA_ERR_OUTOFMEMORY );

      // initialize parameters
      ::g_view_manager->init ( );

    }
  catch ( pisa_error & err )
    {
      aleart_dialog aleart_dlg;

      aleart_dlg.message_box ( 0, err.get_error_string ( ) );

      if ( ::g_view_manager )
	{
	  delete ::g_view_manager;
	  ::g_view_manager = 0;
	}

      return err.get_error_id ( );
    }

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void view_manager::release_view_manager ( void )
{
  if ( ::g_view_manager )
    {
      delete ::g_view_manager;
      ::g_view_manager = 0;
    }
}

/*------------------------------------------------------------*/
int view_manager::main ( void )
{
  gtk_main ( );

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void view_manager::init ( void )
{
  // initialize
  m_scanmanager_cls	= 0;
  m_main_cls		= 0;
  m_prev_cls		= 0;
  m_imgctrl_cls		= 0;
  m_gamma_cls		= 0;
  m_config_cls		= 0;
  m_filsel_cls		= 0;
  m_scansel_cls		= 0;

  // open scanner
  m_scanmanager_cls = new scan_manager;

  if ( ! m_scanmanager_cls )
    throw pisa_error ( PISA_ERR_OUTOFMEMORY );

  open_device ( );

  m_set.option		= PISA_OP_FLATBED;
  m_set.film		= PISA_FT_REFLECT;
  m_set.resolution	= 300;
  m_set.enable_start_button = false;
  m_set.usm		= 1;
  m_set.unit		= PISA_UNIT_INCHES;

  if ( pisa_gimp_plugin ( ) )
    m_set.destination	= PISA_DE_GIMP;
  else
    m_set.destination	= PISA_DE_FILE;

  m_main_cls	= new main_window;
  m_prev_cls	= new preview_window;
  m_imgctrl_cls	= new image_controls;
  m_gamma_cls	= new gamma_correction;
  m_config_cls	= new config_window;
  m_filsel_cls	= new file_selector;

  if ( ! m_main_cls ||
       ! m_prev_cls ||
       ! m_imgctrl_cls ||
       ! m_gamma_cls ||
       ! m_config_cls ||
       ! m_filsel_cls )
    {
      destroy ( );
      throw ( PISA_ERR_OUTOFMEMORY );
    }

  init_img_info ( static_cast <pisa_option_type> ( m_set.option ),
		  static_cast <pisa_film_type> ( m_set.film ) );

  m_main_cls->init ( );
  m_prev_cls->init ( );
  m_gamma_cls->init ( );
  m_config_cls->init ( );
  m_filsel_cls->init ( );

  load_preference ( );

}

/*------------------------------------------------------------*/
void view_manager::destroy ( void )
{
  save_preference ( );

  m_set.delete_all ( );

  if ( m_scanmanager_cls )
    {
      try
	{
	  close_device ( );
	}
      catch ( pisa_error & err )
	{
	  aleart_dialog aleart_dlg;
	  
	  aleart_dlg.message_box ( 0, err.get_error_string ( ) );
	}

      delete m_scanmanager_cls;
      m_scanmanager_cls = 0;
    }
  
  if ( m_main_cls )
    {
      delete m_main_cls;
      m_main_cls = 0;
    }

  if ( m_prev_cls )
    {
      delete m_prev_cls;
      m_prev_cls = 0;
    }

  if ( m_imgctrl_cls )
    {
      delete m_imgctrl_cls;
      m_imgctrl_cls = 0;
    }

  if ( m_gamma_cls )
    {
      delete m_gamma_cls;
      m_gamma_cls = 0;
    }
  
  if ( m_config_cls )
    {
      delete m_config_cls;
      m_config_cls = 0;
    }

  if ( m_filsel_cls )
    {
      delete m_filsel_cls;
      m_filsel_cls = 0;
    }

  if ( m_scansel_cls )
    {
      delete m_scansel_cls;
      m_scansel_cls = 0;
    }

  ::pisa_quit ( );
}

/*------------------------------------------------------------*/
GtkWidget * view_manager::create_window ( pisa_window_id id )
{
  GtkWidget * ret = 0;

  switch ( id )
    {
    case ID_WINDOW_MAIN:
      ret = m_main_cls->create_window ( 0 );
      break;

    case ID_WINDOW_PREV:
      ret = m_prev_cls->create_window ( 0 );
      break;

    case ID_WINDOW_IMGCTRL:
      ret = m_imgctrl_cls->create_window ( 0 );
      break;

    case ID_WINDOW_GAMMA:
      ret = m_gamma_cls->create_window ( m_main_cls->get_widget ( ) );
      break;

    case ID_WINDOW_CONFIG:
      ret = m_config_cls->create_window ( m_main_cls->get_widget ( ) );
      break;
    }

  return ret;
}

/*------------------------------------------------------------*/
int view_manager::close_window ( pisa_window_id id, int destroy_flag )
{
  switch ( id )
    {
    case ID_WINDOW_MAIN:
      m_main_cls->close_window ( destroy_flag );
      destroy ( );
      break;

    case ID_WINDOW_PREV:
      m_prev_cls->close_window ( destroy_flag );
      break;

    case ID_WINDOW_GAMMA:
      m_gamma_cls->close_window ( destroy_flag );
      break;

    case ID_WINDOW_CONFIG:
      m_config_cls->close_window ( destroy_flag );
      break;

    default:
      return PISA_ERR_PARAMETER;
    }

  return PISA_ERR_PARAMETER;
}

/*------------------------------------------------------------*/
void * view_manager::get_window_cls ( pisa_window_id id )
{
  void * ret = 0;

  switch ( id )
    {
    case ID_WINDOW_MAIN:
      ret = m_main_cls;
      break;

    case ID_WINDOW_PREV:
      ret = m_prev_cls;
      break;
      
    case ID_WINDOW_IMGCTRL:
      ret = m_imgctrl_cls;
      break;

    case ID_WINDOW_GAMMA:
      ret = m_gamma_cls;
      break;

    case ID_WINDOW_CONFIG:
      ret = m_config_cls;
      break;

    }

  return ret;
}

/*------------------------------------------------------------*/
void view_manager::sensitive ( void )
{
  int is_img = m_prev_cls->is_prev_img ( );
  
  m_main_cls->sensitive ( is_img );
  m_imgctrl_cls->sensitive ( is_img );
  m_gamma_cls->sensitive ( is_img );
}

/*------------------------------------------------------------*/
int view_manager::is_gimp ( void )
{
  return pisa_gimp_plugin ( );
}

/*------------------------------------------------------------*/
void view_manager::start_scan ( void )
{
  long			id;
  scan_parameter	param;

  id = m_set.get_marquee_size ( ) - 1;

  if ( PISA_ERR_SUCCESS != init_scan_param ( & param, id ) )
    return;

  // All types of scans need to provide visual feedback on their
  // progress and for consecutive scans (via ADF or with the start
  // button enabled) it is better to create it here.  That way, it
  // will not disappear and reappear between calls to scan_file() and
  // scan_gimp(), eliminating rather annoying flicker.
  _feedback = new progress_window (m_main_cls->get_widget ());
  switch ( m_set.destination )
    {
    case PISA_DE_FILE:
      do_scan_file ( & param );
      break;

    case PISA_DE_PRINTER:
      do_scan_printer ( & param );
      break;

    case PISA_DE_GIMP:
      do_scan_gimp ( & param );
      break;
    }
  delete _feedback;
}

int
view_manager::update_lut (void)
{
  return update_lut (m_set.get_current_marquee ());
}

int
view_manager::update_lut (long i)
{
  return update_lut (m_set.get_marquee (i));
}

int
view_manager::update_lut (marquee *m)
{
  const scanner_info *info = m_scanmanager_cls->get_scanner_info ();

  iscan::build_LUT (&m_set, m, info->dumb);

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void view_manager::change_document_source ( void )
{
  m_set.delete_all ( );

  try
    {
      init_img_info ( static_cast <pisa_option_type> ( m_set.option ),
		      static_cast <pisa_film_type> ( m_set.film ) );
    }
  catch ( pisa_error & err )
    {
      aleart_dialog aleart_dlg;
      
      aleart_dlg.message_box ( m_main_cls->get_widget ( ),
			       err.get_error_string ( ) );
    }
  
  m_gamma_cls->reset ( 1 );

  m_prev_cls->resize_window ( );
}

void view_manager::set_device( char * name )
{
  m_scanmanager_cls->close_device();
  m_scanmanager_cls->open_device( name );
}

char * view_manager::get_device_name() const
{
  return m_scansel_cls->get_device( true );
}

/*------------------------------------------------------------*/
void view_manager::open_device ( void )
{
  sane_init( 0, 0 );
  if (!m_scansel_cls)
    m_scansel_cls = new scan_selector( true );	// dialog box
  m_scanmanager_cls->open_device ( m_scansel_cls->get_device() );
}

/*------------------------------------------------------------*/
void view_manager::close_device ( void )
{
  m_scanmanager_cls->close_device ( );
}

/*------------------------------------------------------------*/
void view_manager::load_preference ( void )
{
  char pref_path [ 256 ];
  char pips_path [ 1024 ] = "lpr";

  cfg_struct cfg [ ] =
  {
    { "PIPS", CFG_STRING, pips_path }
  };

  ::strcpy ( pref_path, ::getenv ( "HOME" ) );
  ::strcat ( pref_path, "/" );
  ::strcat ( pref_path, PREFERENCE );

  ::get_cfg ( pref_path, cfg, sizeof ( cfg ) / sizeof ( cfg [ 0 ] ) );

  ::strcpy ( m_config_cls->m_cmd, pips_path );
}

/*------------------------------------------------------------*/
void view_manager::save_preference ( void )
{
  char pref_path [ 256 ];
  char pips_path [ 1024 ] = "";

  cfg_struct cfg [ ] =
  {
    { "PIPS", CFG_STRING, pips_path }
  };

  ::strcpy ( pref_path, ::getenv ( "HOME" ) );
  ::strcat ( pref_path, "/" );
  ::strcat ( pref_path, PREFERENCE );

  ::strcpy ( pips_path, m_config_cls->m_cmd );

  ::set_cfg ( pref_path, cfg, sizeof ( cfg ) / sizeof ( cfg [ 0 ] ) );
}

/*------------------------------------------------------------*/
int view_manager::init_img_info ( pisa_option_type option,
				  pisa_film_type film )
{
  int		i, j;
  marquee	* marq;
  double	max_width, max_height;
  double	coef [ 9 ];

  max_width	= 8.5;
  max_height	= 11.7;

  // get max area
  m_scanmanager_cls->set_option ( option );
  m_scanmanager_cls->set_film_type ( film );
  m_scanmanager_cls->get_current_max_size ( & max_width, & max_height );
  m_scanmanager_cls->get_color_profile ( coef );

  // initialize marquee
  marq = new marquee;

  marq->offset.x	= 0.0;
  marq->offset.y	= 0.0;
  marq->area.x		= max_width;
  marq->area.y		= max_height;

  marq->scale		= 100;

  marq->gamma		= ( long ) ( 100 * DEFGAMMA );
  marq->highlight	= DEFHIGHLIGHT;
  marq->shadow		= DEFSHADOW;
  marq->threshold	= DEFTHRESHOLD;

  for ( i = 0; i < 4; i++ )
    for ( j = 0; j < 256; j++ )
      marq->gamma_table [ i ] [ j ] = j;

  marq->graybalance	= DEFGRAYBALANCE;
  marq->saturation	= DEFSATURATION;

  if ( option == PISA_OP_TPU )
    marq->focus		= 25;
  else
    marq->focus		= 0;

  for ( i = 0; i < 3; i++ )
    {
      marq->film_gamma [ i ]	= 1.0;
      marq->film_yp [ i ]	= 0.0;
      marq->grayl [ i ]		= 0.0;
    }

  for ( i = 0; i < 256; i++ )
    {
      marq->lut.gamma_r [ i ]	= i;
      marq->lut.gamma_g [ i ]	= i;
      marq->lut.gamma_b [ i ]	= i;
    }

  m_set.init ( & marq );
 
  m_set.max_area [ 0 ]	= max_width;
  m_set.max_area [ 1 ]	= max_height;
  
  for ( i = 0; i < 9; i++ )
    m_set.coef [ i ] = coef [ i ];

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int view_manager::init_scan_param ( scan_parameter * param,
				    long i )
{
  marquee	* marq;

  ::memset ( param, 0, sizeof ( scan_parameter ) );

  try
    {
      update_document_source ( param );

      marq = m_set.get_marquee ( i );
      update_image_type ( param );
      update_scan_area ( param, marq );
      update_color_table ( param, marq );
      update_focus ( param, marq );
      update_imgprocess ( param, marq );
    }
  catch ( pisa_error & err )
    {
      aleart_dialog dlg;

      dlg.message_box ( m_main_cls->get_widget ( ), err.get_error_string ( ) );

      return err.get_error_id ( );
    }

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void view_manager::update_document_source ( scan_parameter * param )
{
  param->option	= m_set.option;
  param->film	= m_set.film;	// only option is tpu
}


/*------------------------------------------------------------*/
void view_manager::update_image_type ( scan_parameter * param )
{
  param->pixeltype	= m_set.imgtype.pixeltype;
  param->bitdepth	= m_set.imgtype.bitdepth;
  param->scanspeed	= m_set.imgtype.scanspeed;
  param->dropout	= m_set.imgtype.dropout;
  param->monoopt	= m_set.imgtype.monoopt;
  param->halftone	= m_set.imgtype.halftone;
}

/*------------------------------------------------------------*/
void view_manager::update_scan_area ( scan_parameter * param,
				      marquee * marq )
{
  param->offset [ 0 ]		= marq->offset.x;
  param->offset [ 1 ]		= marq->offset.y;
  param->area [ 0 ]		= marq->area.x;
  param->area [ 1 ]		= marq->area.y;
  param->resolution [ 0 ]	= ( m_set.resolution * marq->scale + 50 ) / 100;
  param->resolution [ 1 ]	= ( m_set.resolution * marq->scale + 50 ) / 100;
  param->zoom [ 0 ]		= 100;
  param->zoom [ 1 ]		= 100;

  if (m_scanmanager_cls->area_is_too_large (param))
    {
      throw pisa_error (PISA_ERR_AREALARGE);
    }

  if ( m_set.imgtype.de_screening == PISA_DESCREEN_ON )
    {
      const scanner_info * scan_info;

      scan_info = m_scanmanager_cls->get_scanner_info ( );

      if (   param->resolution[0] > scan_info->max_descreen_resolution
	  || param->resolution[1] > scan_info->max_descreen_resolution)
	{
	  throw pisa_error (PISA_ERR_MRRESTOOHIGH);
	}
    }
}


/*------------------------------------------------------------*/
void view_manager::update_color_table ( scan_parameter * param,
					marquee * marq )
{
  int		i;
  double	coef [ 9 ];

  // gamma table
  for ( i = 0; i < 256; i++ )
    {
      param->gamma.gamma_r [ i ] = marq->lut.gamma_r [ i ];
      param->gamma.gamma_g [ i ] = marq->lut.gamma_g [ i ];
      param->gamma.gamma_b [ i ] = marq->lut.gamma_b [ i ];
    }

  // speed
  param->speed	= 0;

  // threshold
  param->threshold = marq->threshold;

  // color correct
  if ( m_set.imgtype.pixeltype == PISA_PT_RGB )
    {
      ::generate_color_coef ( coef, m_set.coef, marq->saturation );
    }
  else
    {
      for ( i = 0; i < 9; i++ )
	coef [ i ] = 0.0;
      
      coef [ 0 ] = coef [ 4 ] = coef [ 8 ] = 1.0;
    }
  
  for ( i = 0; i < 9; i++ )
    param->coef [ i ] = coef [ i ];
}

/*------------------------------------------------------------*/
void view_manager::update_focus ( scan_parameter * param, marquee * marq )
{
  param->focus	= marq->focus;
}

/*------------------------------------------------------------*/
void view_manager::update_imgprocess ( scan_parameter * param, marquee * marq )
{
  marq = marq;

  // color correct
  param->usm = m_set.usm;

  if ( m_set.imgtype.pixeltype == PISA_PT_BW )
    param->usm = 0;

  // de-screening
  if ( m_set.imgtype.de_screening == PISA_DESCREEN_ON )
    param->de_screening = 1;
  else
    param->de_screening = 0;

}

/*------------------------------------------------------------*/
int view_manager::do_scan_file( scan_parameter *param,
				file_selector *fs_cls,
				bool first_time_around )
{
  int  cancel;

  if (!fs_cls)
    {
      fs_cls = (file_selector *) m_filsel_cls;

      fs_cls->init();
#ifndef HAVE_GTK_2
      fs_cls->create_window (m_main_cls->get_widget ( ), m_set.option,
			     m_set.enable_start_button,
			     PISA_PT_BW == m_set.imgtype.pixeltype);
#else
      GtkWidget *w = m_main_cls->get_widget ();
      fs_cls->create_window (GTK_WINDOW (gtk_widget_get_parent (w)), w,
			     (   PISA_OP_ADF     == m_set.option
			      || PISA_OP_ADFDPLX == m_set.option
			      || m_set.enable_start_button));
#endif /* HAVE_GTK_2 */
      fs_cls->hide();
    }

  char *filename = fs_cls->get_filename();

  if (   PISA_OP_ADF     != m_set.option
      && PISA_OP_ADFDPLX != m_set.option
      && !m_set.enable_start_button)
    {
      fs_cls->destroy();

      if (!filename)
	return PISA_ERR_SUCCESS;

      scan_file ( param, filename, & cancel );
    }
  else				// ADF scanning
    if (filename)
      {
	char *dash = strrchr( filename, '-' );
	char *dot  = strrchr( filename, '.' );
	size_t len = strlen( filename );
	int digits = dot - dash - 1;

	assert( digits < 10 ); // assumed for string lengths!
	char *format = new char[len + dash - dot +  5];
	char *regexp = new char[len + dash - dot + 29];
	char *f_name = new char[len + dash - dot + 11];

	++*dash = '\0';		// cut filename
	sprintf( format, "%s-%%%dd%s"          , filename, digits, dot );
	sprintf( regexp, "%s-([1-9][0-9]{%d,}|[0-9]{%d})\\%s$",
		 filename, digits, digits, dot );

	cancel = check_overwrite( regexp );
	delete[] regexp;

	bool eos = (0 != cancel);	// scan if NOT cancelled
	int  cnt = fs_cls->get_sequence_number();

	while (!eos)
	  {
	    sprintf( f_name, format, cnt );
	    char *p = strrchr( f_name, '-' );
	    while (' ' == *(++p))
	      *p = '0';	// zero out any spaces

	    // FIXME: update fs_cls' filename?
	    eos = scan_file ( param, f_name, & cancel, first_time_around );
	    first_time_around = false;
	    if (!eos)
	      {
		fs_cls->set_sequence_number( ++cnt );
	      }
	  }
	delete[] format;
	delete[] f_name;
	if (!cancel) {
	  do_scan_file( param, fs_cls, first_time_around );
	}
	fs_cls->destroy();
      }
    else
      fs_cls->destroy();

  free( filename );

  return PISA_ERR_SUCCESS;
}

//! Sends scan results to a printer.
/*!
 */
int
view_manager::do_scan_printer (scan_parameter *param,
			       int fd, bool first_time_around)
{
  int   cancel;
  char  cmd[256];		// FIXME: buffer overflow!

  if (   PISA_OP_ADF     != m_set.option
      && PISA_OP_ADFDPLX != m_set.option
      && !m_set.enable_start_button)
    {
      char *filename = NULL;
      int   status   = get_temp_filename (&filename);

      if (PISA_ERR_SUCCESS != status)
	{
	  free (filename);
	  return PISA_ERR_SUCCESS; // FIXME: should return status;
	}

      scan_file (param, filename, &cancel);

      if (!cancel)
	{
	  while (gtk_events_pending ())
	    gtk_main_iteration ();
      
	  sprintf (cmd, "%s %s", m_config_cls->m_cmd, filename);
	  system (cmd);
	}
      remove (filename);
      free (filename);
   }
  else				// ADF scanning
    {
      bool eos = false;
      while (!eos)
	{
	  char *filename = NULL;
	  int   status   = get_temp_filename (&filename);

	  if (PISA_ERR_SUCCESS != status)
	    {
	      free (filename);
	      return PISA_ERR_SUCCESS; // FIXME: should return status;
	    }

	  eos = scan_file( param, filename, &cancel, first_time_around );
	  first_time_around = false;

	  if (!cancel && !eos)
	    {
	      while (gtk_events_pending ())
		gtk_main_iteration ();

	      sprintf (cmd, "%s %s", m_config_cls->m_cmd, filename);
	      system (cmd);
	    }
	  remove (filename);
	  free (filename);
	}
      if (!cancel)
	do_scan_printer (param, fd, first_time_around);
    }

  return PISA_ERR_SUCCESS;
}

int
view_manager::do_scan_gimp( scan_parameter *param,
			    bool first_time_around )
{
  int cancel = 0;

  if (   PISA_OP_ADF     != m_set.option
      && PISA_OP_ADFDPLX != m_set.option
      && !m_set.enable_start_button)
    return scan_gimp( param, &cancel );

  bool eos    = false;

  while (!eos)
    {
      eos = scan_gimp( param, &cancel, first_time_around );
      first_time_around = false;
    }
  if (!cancel)
    do_scan_gimp( param, first_time_around );

  return PISA_ERR_SUCCESS;
}

int
view_manager::dialog_reply( const pisa_error& err ) const
{
  int reply = 0;
  
  aleart_dialog dlg;

  if ( ( PISA_STATUS_GOOD < err.get_error_id() ) &&
       ( PISA_OP_ADF     == m_set.option ||
         PISA_OP_ADFDPLX == m_set.option            ) )
    {
      int i = dlg.message_box( m_main_cls->get_widget(),
			       err.get_error_string(),
			       _("  Continue  "), _("  Cancel  ") );
      if (2 == i)
	reply = 1;
    }
  else if ( ( PISA_STATUS_GOOD == err.get_error_id() ) &&
	    m_set.enable_start_button )
    {

      int i = dlg.message_box( m_main_cls->get_widget(),
			       "Waiting for ...",
			       _("  Finish  ") );

      if ( 1 == i )
	reply = 1;
    }
  else
    {
      dlg.message_box( m_main_cls->get_widget(),
		       err.get_error_string() );

      if (PISA_ERR_FILEOPEN != err.get_error_id())
	reply = 1;
    }

  return reply;
}

/*------------------------------------------------------------*/
bool
view_manager::scan_gimp( scan_parameter *param, int *cancel,
			 bool first_time_around )
{

#ifndef HAVE_ANY_GIMP

  *cancel = 1;			// can't scan to GIMP
  return true;

#else
  bool error = false;		// be optimistic
  try
    {
      *cancel = 0;
      bool wait_for_button = (m_set.enable_start_button
			      && !(   PISA_OP_ADF     == m_set.option
				   || PISA_OP_ADFDPLX == m_set.option));

      bool reset_params = false;
      if (wait_for_button
	  && m_scanmanager_cls->is_button_pressed ()) {
	// We have an impatient user here who pressed the scanner's
	// button *before* we even got a chance to show our WAITING
	// message.
	// We ensure the user gets to see this message be (re)setting
	// the document source (which indirectly resets the scanner's
	// push button status) for lack of a more elegant way and raise
	// a flag so that any parameters that may have been erased as
	// a result are reset before we request scan data.
	m_scanmanager_cls
	  ->set_option (static_cast <pisa_option_type> (m_set.option));
	reset_params = true;
      }

      _feedback->set_text (wait_for_button
			   ? progress_window::WAITING
			   : progress_window::WARMING_UP);
      _feedback->set_progress (0, 1);
      _feedback->show ();

      if (wait_for_button) {
	while (!m_scanmanager_cls->is_button_pressed ()
	       && !_feedback->is_cancelled ()) {
	  sleep (1);
	  while (gtk_events_pending ()) {
	    gtk_main_iteration ();
	  }
	}
	if (_feedback->is_cancelled ()) {
	  *cancel = 1;
	  return true;
	}
      }

      _feedback->set_text (progress_window::WARMING_UP);
      
      int width, height;
      m_scanmanager_cls->init_scan (param, &width, &height,
				    first_time_around || reset_params);

      while ( ::gtk_events_pending ( ) )
	::gtk_main_iteration ( );

      char depth;
      int rowbytes;
      switch (m_set.imgtype.pixeltype)
	{
	case PISA_PT_RGB:
	  depth	   = 8;
	  rowbytes = width * 3;
	  break;
	case PISA_PT_GRAY:
	  depth	   = 8;
	  rowbytes = width;
	  break;
	case PISA_PT_BW:
	  depth	   = 1;
	  rowbytes = (width + 7) / 8;
	  break;
	default:
	  rowbytes = 0;
	}

      gimp_scan gimp_cls;
      if (PISA_ERR_SUCCESS !=
	  gimp_cls.create_gimp_image (width, height,
				      m_set.imgtype.pixeltype, depth))
	{
	  m_scanmanager_cls->acquire_image ( 0, 0, 1, 1 );
	  
	  throw pisa_error ( PISA_ERR_OUTOFMEMORY );
	}

      for (int i = 0; i < height; i++)
	{
	  m_scanmanager_cls->acquire_image ( gimp_cls.get_next_buf ( ),
					     rowbytes,
					     1,
					     *cancel );

	  if ( i == 0 )
	    _feedback->set_text (progress_window::SCANNING);

	  if (*cancel)
	    {
	      error = true;
	      break;
	    }

	  _feedback->set_progress (i, height);
	  *cancel = _feedback->is_cancelled ();

	  gimp_cls.set_image_rect ( );

	  while ( ::gtk_events_pending ( ) )
	    ::gtk_main_iteration ( );
	}
      _feedback->set_progress (height, height);

      gimp_cls.finish_scan ( *cancel );
    }
  catch ( pisa_error & err )
    {
      error = true;
      *cancel = dialog_reply( err );
    }

  m_scanmanager_cls->finish_acquire ( );
  
  return error;
#endif	// HAVE_ANY_GIMP
}

/*------------------------------------------------------------*/
bool view_manager::scan_file ( scan_parameter *param,
			       const char *filename, int *cancel,
			       bool first_time_around )
{
  bool error = false;		// be optimistic
  try
    {
      *cancel = 0;
      bool wait_for_button = (m_set.enable_start_button
			      && !(   PISA_OP_ADF     == m_set.option
				   || PISA_OP_ADFDPLX == m_set.option));

      bool reset_params = false;
      if (wait_for_button
	  && m_scanmanager_cls->is_button_pressed ()) {
	// We have an impatient user here who pressed the scanner's
	// button *before* we even got a chance to show our WAITING
	// message.
	// We ensure the user gets to see this message be (re)setting
	// the document source (which indirectly resets the scanner's
	// push button status) for lack of a more elegant way and raise
	// a flag so that any parameters that may have been erased as
	// a result are reset before we request scan data.
	m_scanmanager_cls
	  ->set_option (static_cast <pisa_option_type> (m_set.option));
	reset_params = true;
      }

      _feedback->set_text (wait_for_button
			   ? progress_window::WAITING
			   : progress_window::WARMING_UP);
      _feedback->set_progress (0, 1);
      _feedback->show ();

      while (::gtk_events_pending())
	::gtk_main_iteration();

      if (wait_for_button) {
	while (!m_scanmanager_cls->is_button_pressed ()
	       && !_feedback->is_cancelled ()) {
	  sleep (1);
	  while (gtk_events_pending ()) {
	    gtk_main_iteration ();
	  }
	}
	if (_feedback->is_cancelled ()) {
	  *cancel = 1;
	  return true;
	}
      }

      _feedback->set_text (progress_window::WARMING_UP);
      
      int width, height;
      m_scanmanager_cls->init_scan (param, &width, &height,
				    first_time_around || reset_params);

      while (::gtk_events_pending())
	::gtk_main_iteration();

      int rowbytes;
      iscan::colour_space cs;

      switch (m_set.imgtype.pixeltype)
	{
	case PISA_PT_RGB:
	  rowbytes = width * 3;
	  cs = iscan::RGB;
	  break;
	case PISA_PT_GRAY:
	  rowbytes = width;
	  cs = iscan::gray;
	  break;
	case PISA_PT_BW:
	  rowbytes = (width + 7) / 8;
	  cs = iscan::mono;
	  break;
	default:
	  rowbytes = 0;
	  throw pisa_error (PISA_ERR_PARAMETER);
	}

      iscan::imgstream *save_cls = NULL;
      iscan::imgstream::filebuf fb (fopen (filename, "wb"));

      switch (get_file_type( filename )) // FIXME: use factory method
	{
	case PISA_FI_PNM:
	  save_cls = new iscan::pnmstream (&fb);
	  break;
	case PISA_FI_PNG:
	  save_cls = new iscan::pngstream (&fb);
	  break;
	case PISA_FI_JPG:
	  save_cls = new iscan::jpegstream (&fb);
	  break;
	default:
	  throw pisa_error( PISA_ERR_FILEOPEN );
	}

      try
	{
      try {
	save_cls->size (width, height);
	save_cls->depth (PISA_PT_BW == m_set.imgtype.pixeltype
			 ? 1 : 8);
	save_cls->colour (cs);
	save_cls->resolution (m_set.resolution, m_set.resolution);
      } catch (std::ios_base::failure& oops) {
	// map to old API and rethrow
	throw (save_cls->rdbuf ()
	       ? pisa_error (PISA_ERR_OUTOFMEMORY)
	       : pisa_error (PISA_ERR_FILEOPEN));
      }
	}
      catch (pisa_error& oops)
	{
	  m_scanmanager_cls->acquire_image( 0, 0, 1, 1 );
	  ::remove( filename );
	  delete save_cls;
	  throw oops;
	}

      unsigned char *img = new unsigned char[rowbytes];
      for (int i = 0; i < height; ++i)
	{
	  m_scanmanager_cls->acquire_image( img, rowbytes, 1, *cancel );

	  if (0 == i)
	    _feedback->set_text (progress_window::SCANNING);

	  if (*cancel)
	    {
	      ::remove( filename );
	      error = true;
	      break;
	    }

	  _feedback->set_progress (i, height);
	  *cancel = _feedback->is_cancelled ();

	  try
	    {
	  try {
	    save_cls->write ((const char *)img, rowbytes);
	  } catch (std::ios_base::failure& oops) {
	    // map to old API and rethrow
	    throw (save_cls->rdbuf ()
		   ? pisa_error (PISA_ERR_OUTOFMEMORY)
		   : pisa_error (PISA_ERR_FILEOPEN));
	  }
	    }
	  catch (pisa_error& oops)
	    {
	      *cancel = 1;

	      if (i < height)
		m_scanmanager_cls->acquire_image( img, rowbytes, 1,
						  *cancel );
	      aleart_dialog aleart_dlg;
	      aleart_dlg.message_box( m_main_cls->get_widget(),
				      oops.get_error_string() );

	      ::remove( filename );
	      break;
	    }
	  while (::gtk_events_pending())
	    ::gtk_main_iteration();
	}
      delete[] img;
      delete save_cls;

      _feedback->set_progress (height, height);
    }
  catch (pisa_error& oops)
    {
      error = true;
      *cancel = dialog_reply( oops );
      ::remove (filename);
    }

  m_scanmanager_cls->finish_acquire();

  while (::gtk_events_pending())
    ::gtk_main_iteration();

  return error;
}

//! Indicates the expected output file format.
/*! File format is normally indicated by the file extension, but for
    scanning to printer we use temporary files that can not fit that
    rule.
 */
pisa_file_type
view_manager::get_file_type (const char *filename)
{
  char *dot   = strrchr (filename, '.');
  if (!dot)
    {
      char *slash = strrchr (filename, '/');
      if (   (strlen (slash) == strlen ("/" PACKAGE_TARNAME "XXXXXX"))
	  && (0 == strncmp (slash, "/" PACKAGE_TARNAME,
			    strlen ("/" PACKAGE_TARNAME))))
	return PISA_FI_PNG;	// temporary file for output to printer

      return PISA_FI_UNSUPPORTED;
    }

  if (0 == strcmp (dot, ".pnm"))
    return PISA_FI_PNM;
  if (0 == strcmp (dot, ".png"))
    return PISA_FI_PNG;
  if ((0 == strcmp (dot, ".jpg")) || (0 == strcmp (dot, ".jpeg")))
    return PISA_FI_JPG;

  return PISA_FI_UNSUPPORTED;
}

//! Creates a unique temporary filename and opens that file.
/*! The unique file is created and opened in the directory indicated
    by the TMPDIR environment variable, whatever your system's stdio.h
    file has defined for \c P_tmpdir or the \c /tmp directory.  The
    first directory that is writable will be used.

    Caller will need to free the memory pointed to by *filename.
 */
int
view_manager::get_temp_filename (char **filename)
{
  const char *tmpnam = "/" PACKAGE_TARNAME "XXXXXX";
  char *tmpdir;
  {
    char *dir_array[] = {
      getenv ("TMPDIR"),
#ifdef P_tmpdir
      P_tmpdir,
#endif
      "/tmp",
      NULL
    };
    struct stat buf;

    unsigned int i = 0;
    tmpdir = dir_array[i];
    while (i < sizeof (dir_array) / sizeof (char *)
	   && (!tmpdir
	       || (0 != access (tmpdir, W_OK | X_OK))
	       || (0 != stat (tmpdir, &buf))
	       || !S_ISDIR(buf.st_mode)))
      {
	tmpdir = dir_array[++i];
      }
    if (!tmpdir)
      return PISA_ERR_FILEOPEN;
  }

  *filename = (char *) malloc ((strlen (tmpdir) + strlen (tmpnam) + 1)
			       * sizeof (char));
  if (!*filename)
    return PISA_STATUS_NO_MEM;

  memset (*filename, 0, (strlen (tmpdir) + strlen (tmpnam) + 1));
  strcpy (*filename, tmpdir);
  strcpy (*filename + strlen (tmpdir), tmpnam);

  mode_t previous_mask = umask (0077);

  if (0 > mkstemp (*filename))
    {
      umask (previous_mask);
      free (*filename);
      *filename = NULL;
      return PISA_ERR_FILEOPEN;
    }

  umask (previous_mask);

  return PISA_ERR_SUCCESS;
}

// FIXME: file_selector can and should be responsible for this!
int
view_manager::check_overwrite( const char *regexp )
{
  int cancel = 0;		// default: don't cancel

  char *slash = strrchr( regexp, '/' );

  if (!slash)
    return cancel = 1;

  *slash = '\0';		// regexp now holds the directory name
  char dirname[ strlen( regexp )];
  strcpy( dirname, regexp );

  *slash = '^';			// re-anchor the regexp

  regex_t *comp_regex = new regex_t;
  int comp = regcomp( comp_regex, slash, REG_EXTENDED );

  if (0 == comp)
    {
      size_t     nsub = comp_regex->re_nsub + 1;
      regmatch_t match[nsub];

      file_selector *fs = (file_selector *) m_filsel_cls;

      DIR *dir = opendir( dirname );
      if (!dir)
	return 0;		// file creation failure handles this
      
      struct dirent *file = 0;
      bool overwrite = false;	// be conservative ;-)
      while (0 == cancel && !overwrite && (file = readdir( dir )))
	{
	  int result = regexec( comp_regex, file->d_name, nsub, match, 0 );
	  if (0 == result)
	    {
	      size_t digits = match[1].rm_eo - match[1].rm_so;
	      char num[digits + 1];
	      char *c = num;
	      {
		char *p = file->d_name + match[1].rm_so;
		while (0 < digits--)
		  *c++ = *p++;
	      }
	      int seq_num = atoi( num );

	      if (seq_num >= fs->get_sequence_number())
		{
		  aleart_dialog dlg;
		  int answer = dlg.message_box(m_main_cls->get_widget ( ),
					       _("Overwrite?"),
					       _("  Yes  "), _("  No  ") );

		  if (2 == answer)
		    cancel = 1;
		  if (1 == answer)
		    overwrite = true;
		}
	    }
	  else
	    if (REG_NOMATCH != result)
	      regerror( comp, comp_regex );
	}
      closedir( dir );
    }
  else
    regerror( comp, comp_regex );

  regfree( comp_regex );
  delete comp_regex;
  
  return cancel;
}

void
view_manager::regerror( int code, regex_t *regex )
{
  size_t length = ::regerror( code, regex, 0, 0 );
  char *message = new char[length];

  ::regerror( code, regex, message, length );
  fprintf( stderr, "%s\n", message );

  delete[] message;
}
