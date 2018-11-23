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

#include <config.h>

#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

/*------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------*/
#include "pisa_main_window.h"
#include "pisa_error.h"
#include "pisa_tool.h"
#include "xpm_data.h"
#include "pisa_view_manager.h"
#include "pisa_change_unit.h"
#include "pisa_default_val.h"
#include "pisa_scan_selector.h"

/*------------------------------------------------------------*/
static gint delete_event ( GtkWidget * widget, gpointer data );
static void click_preview_btn ( void );
static void click_zoom_btn ( void );
static void click_expose_btn ( void );
static void click_scan_btn ( void );
static void click_close ( void );

static void change_destination ( GtkWidget * widget, gpointer data );
static void change_docsrc ( GtkWidget * widget, gpointer data );
static void change_imgtype ( GtkWidget * widget, gpointer data );
static void change_resolution ( GtkWidget * widget, gpointer data );
static void change_unit ( GtkWidget * widget, gpointer data );
static void value_changed ( GtkAdjustment * adjust, gpointer data );
static void toggled_start_button (GtkWidget *widget, gpointer data);
static void toggled_usm ( GtkWidget * widget, gpointer data );
static void check_focus ( GtkWidget * widget, gpointer data );

/*------------------------------------------------------------*/
// destination
#define ID_MENU_FILE	0x0001
#define ID_MENU_PRINT	0x0002
menu_items g_destination_menu_item [ ] =
{
  { ID_MENU_FILE,  0, "File",    ( GtkSignalFunc * ) change_destination },
  { ID_MENU_PRINT, 0, "Printer", ( GtkSignalFunc * ) change_destination },
  { 0,             0, "",                                             0 }
};

/*------------------------------------------------------------*/
// document source
#define ID_MENU_FLATBED	0x0001
#define ID_MENU_TPU_NEG	0x0002
#define ID_MENU_TPU_POS 0x0003
#define ID_MENU_ADF     0x0004
menu_items g_docsrc_menu_item [ ] =
{
  { ID_MENU_FLATBED, 0, "Flatbed", ( GtkSignalFunc * ) change_docsrc },
  { ID_MENU_TPU_NEG, 0, "TPU for Neg.Film", ( GtkSignalFunc * ) change_docsrc },
  { ID_MENU_TPU_POS, 0, "TPU for Pos.Film", ( GtkSignalFunc * ) change_docsrc },
  { ID_MENU_ADF,     0, "Auto Document Feeder", ( GtkSignalFunc * ) change_docsrc },
  { 0,               0, "",                                        0 }
};

/*------------------------------------------------------------*/
// image type
#define ID_MENU_24C_PHOTO	0x0001
#define ID_MENU_24C_DOC		0x0002
#define ID_MENU_8G_PHOTO	0x0003
#define ID_MENU_8G_DOC		0x0004
#define ID_MENU_LINEART		0x0005
menu_items g_imgtype_menu_item [ ] =
{
  { ID_MENU_24C_PHOTO, 0, "Color Photo",            ( GtkSignalFunc * ) change_imgtype },
  { ID_MENU_24C_DOC,   0, "Color Document",         ( GtkSignalFunc * ) change_imgtype },
  { ID_MENU_8G_PHOTO,  0, "Black & White Photo",    ( GtkSignalFunc * ) change_imgtype },
  { ID_MENU_8G_DOC,    0, "Black & White Document", ( GtkSignalFunc * ) change_imgtype },
  { ID_MENU_LINEART,   0, "Line Art",               ( GtkSignalFunc * ) change_imgtype },
  { 0,                 0, "",                                                     0 }
};

imagetype g_imagetype_list [ ] =
{
  { ID_MENU_24C_PHOTO, PISA_PT_RGB,  PISA_BD_8, PISA_SS_NORMAL, PISA_DESCREEN_OFF, PISA_AE_PHOTO,  PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE },
  { ID_MENU_24C_DOC,   PISA_PT_RGB,  PISA_BD_8, PISA_SS_NORMAL, PISA_DESCREEN_ON,  PISA_AE_DOC,    PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE },
  { ID_MENU_8G_PHOTO,  PISA_PT_GRAY, PISA_BD_8, PISA_SS_NORMAL, PISA_DESCREEN_OFF, PISA_AE_PHOTO,  PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE },
  { ID_MENU_8G_DOC,    PISA_PT_GRAY, PISA_BD_8, PISA_SS_NORMAL, PISA_DESCREEN_ON,  PISA_AE_DOC,    PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE },
  { ID_MENU_LINEART,   PISA_PT_BW,   PISA_BD_1, PISA_SS_DRAFT,  PISA_DESCREEN_OFF, PISA_AE_GRAYED, PISA_DO_NONE, PISA_MO_NONE, PISA_HT_NONE }
};

/*------------------------------------------------------------*/
// resolution
menu_items g_resolution_menu_item [ ] =
{
  { 50,   0, "50dpi",   ( GtkSignalFunc * ) change_resolution },
  { 72,   0, "72dpi",   ( GtkSignalFunc * ) change_resolution },
  { 96,   0, "96dpi",   ( GtkSignalFunc * ) change_resolution },
  { 150,  0, "150dpi",  ( GtkSignalFunc * ) change_resolution },
  { 200,  0, "200dpi",  ( GtkSignalFunc * ) change_resolution },
  { 240,  0, "240dpi",  ( GtkSignalFunc * ) change_resolution },
  { 266,  0, "266dpi",  ( GtkSignalFunc * ) change_resolution },
  { 300,  0, "300dpi",  ( GtkSignalFunc * ) change_resolution },
  { 350,  0, "350dpi",  ( GtkSignalFunc * ) change_resolution },
  { 360,  0, "360dpi",  ( GtkSignalFunc * ) change_resolution },
  { 400,  0, "400dpi",  ( GtkSignalFunc * ) change_resolution },
  { 600,  0, "600dpi",  ( GtkSignalFunc * ) change_resolution },
  { 720,  0, "720dpi",  ( GtkSignalFunc * ) change_resolution },
  { 800,  0, "800dpi",  ( GtkSignalFunc * ) change_resolution },
  { 1200, 0, "1200dpi", ( GtkSignalFunc * ) change_resolution },
  { 1600, 0, "1600dpi", ( GtkSignalFunc * ) change_resolution },
  { 2400, 0, "2400dpi", ( GtkSignalFunc * ) change_resolution },
  { 3200, 0, "3200dpi", ( GtkSignalFunc * ) change_resolution },
  { 0,    0, "",                                           0  }
};

/*------------------------------------------------------------*/
// unit
menu_items g_unit_menu_item [ ] =
{
  { PISA_UNIT_INCHES, 0, "inches", ( GtkSignalFunc * ) change_unit },
  { PISA_UNIT_PIXELS, 0, "pixels", ( GtkSignalFunc * ) change_unit },
  { PISA_UNIT_CM ,    0, "cm",     ( GtkSignalFunc * ) change_unit },
  { 0,                0, "",                                     0 }
};

/*------------------------------------------------------------*/
// scale
#define ID_SCALE_ZOOM	0x0001

scale_items g_scale_zoom = { ID_SCALE_ZOOM, 0, 0,
			     ( GtkSignalFunc * ) value_changed,
			     100, 50, 200, 1, 10 };

/*------------------------------------------------------------*/
// focus

long	g_focus [ ] = { 0, 25 };

/*------------------------------------------------------------*/
static gint delete_event ( GtkWidget * widget, gpointer data )
{
  widget = widget;
  data = data;

  ::g_view_manager->close_window ( ID_WINDOW_MAIN, 0 );

  return FALSE;
}

/*------------------------------------------------------------*/
static void click_preview_btn ( void )
{
  preview_window	* prev_cls;

  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );

  prev_cls->start_preview ( PISA_PREV_WHOLE );
}

/*------------------------------------------------------------*/
static void click_zoom_btn ( void )
{
  preview_window	* prev_cls;

  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );

  prev_cls->start_preview ( PISA_PREV_ZOOM );
}

/*------------------------------------------------------------*/
static void click_expose_btn ( void )
{
  preview_window	* prev_cls;

  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );

  prev_cls->auto_exposure ( );
  prev_cls->update_img ( );
}

/*------------------------------------------------------------*/
static void click_scan_btn ( void )
{
  ::g_view_manager->start_scan ( );
}

/*------------------------------------------------------------*/
static void click_close ( void )
{
  ::g_view_manager->close_window ( ID_WINDOW_MAIN, 1 );
}

/*------------------------------------------------------------*/
static void click_config ( void )
{
  ::g_view_manager->create_window ( ID_WINDOW_CONFIG );
}

/*------------------------------------------------------------*/
static void change_destination ( GtkWidget * widget, gpointer data )
{
  char	dest;
  long	id;

  widget = widget;
  id = * ( long * ) data;

  switch ( id )
    {
    case ID_MENU_FILE:
      dest	= PISA_DE_FILE;
      break;

    case ID_MENU_PRINT:
      dest	= PISA_DE_PRINTER;
      break;

    default:
      return;
    }

  ::g_view_manager->m_set.destination = dest;
}

/*------------------------------------------------------------*/
static void change_docsrc ( GtkWidget * widget, gpointer data )
{
  char	option;
  char	film;
  long	id;

  widget = widget;
  id = * ( long * ) data;

  switch ( id )
    {
    case ID_MENU_FLATBED:
      option	= PISA_OP_FLATBED;
      film	= PISA_FT_REFLECT;
      break;
    case ID_MENU_ADF:
      option	= PISA_OP_ADF;
      film	= PISA_FT_REFLECT;
      break;
    case ID_MENU_TPU_NEG:
      option	= PISA_OP_TPU;
      film	= PISA_FT_NEGA;
      break;
      
    case ID_MENU_TPU_POS:
      option	= PISA_OP_TPU;
      film	= PISA_FT_POSI;
      break;
      
    default:
      return;
    }
  
  if ( ::g_view_manager->m_set.option == option &&
       ::g_view_manager->m_set.film == film )
    return;

  ::g_view_manager->m_set.option	= option;
  ::g_view_manager->m_set.film		= film;

  main_window *main_cls
    = static_cast <main_window *> (g_view_manager
				   ->get_window_cls (ID_WINDOW_MAIN));
  main_cls->enable_start_button (   PISA_OP_ADF != option
				 && PISA_OP_ADFDPLX != option);

  ::memcpy ( & ::g_view_manager->m_set.imgtype,
	     & ::g_imagetype_list [ 0 ],
	     sizeof ( imagetype ) );

  ::g_view_manager->change_document_source ( );
  ::g_view_manager->sensitive ( );
}

/*------------------------------------------------------------*/
static void change_imgtype ( GtkWidget * widget, gpointer data )
{
  unsigned	i;
  long		id;

  widget = widget;
  id = * ( long * ) data;

  for ( i = 0; i < sizeof ( ::g_imagetype_list ) /
	  sizeof ( ::g_imagetype_list [ 0 ] ); i++ )
    {
      if ( id == g_imagetype_list [ i ].id )
	{
	  preview_window	* prev_cls;

	  ::memcpy ( & ::g_view_manager->m_set.imgtype,
		     & g_imagetype_list [ i ],
		     sizeof ( imagetype ) );
	  
	  ::g_view_manager->sensitive ( );
	  
	  prev_cls = ( preview_window * ) ::g_view_manager->get_window_cls ( ID_WINDOW_PREV );
	  
	  prev_cls->auto_exposure ( );

	  ::g_view_manager->update_lut ( );

	  prev_cls->update_img ( );

	  return;
	}
    }
}

/*------------------------------------------------------------*/
static void change_resolution ( GtkWidget * widget, gpointer data )
{
  widget = widget;

  ::g_view_manager->m_set.resolution = * ( long * ) data;

  ::g_view_manager->sensitive ( );
}

/*------------------------------------------------------------*/
static void change_unit ( GtkWidget * widget, gpointer data )
{
  widget = widget;

  ::g_view_manager->m_set.unit = * ( long * ) data;

  ::g_view_manager->sensitive ( );
}

/*------------------------------------------------------------*/
static void value_changed ( GtkAdjustment * adjust, gpointer data )
{
  long	id;
  long	val;
  int	change;
  marquee	* marq;

  id = * ( long * ) data;
  change = 1;
  marq = ::g_view_manager->m_set.get_current_marquee ( );

  switch ( id )
    {
    case ID_SCALE_ZOOM:
      val = ( long ) ( adjust->value );
      if ( marq->scale == val )
	change = 0;
      else
	marq->scale = val;
      break;
    }

  if ( change )
    {
      ::g_view_manager->sensitive ( );
    }
}

/*------------------------------------------------------------*/
static void toggled_start_button (GtkWidget *widget, gpointer data)
{
  data = data;

  if (::gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
    ::g_view_manager->m_set.enable_start_button = true;
  } else {
    ::g_view_manager->m_set.enable_start_button = false;
  }
}

/*------------------------------------------------------------*/
static void toggled_usm ( GtkWidget * widget, gpointer data )
{
  data = data;

  if ( ::gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( widget ) ) )
    ::g_view_manager->m_set.usm = 1;
  else
    ::g_view_manager->m_set.usm = 0;

  preview_window *prev_cls =
    (preview_window *) ::g_view_manager->get_window_cls (ID_WINDOW_PREV);

  prev_cls->update_img ( );
}

/*------------------------------------------------------------*/
static void check_focus ( GtkWidget * widget, gpointer data )
{
  long		id;
  marquee	* marq;

  widget = widget;

  marq = ::g_view_manager->m_set.get_current_marquee ( );
  id = * ( long * ) data;

  if ( id == g_focus [ 1 ] )
    marq->focus = 25;
  else
    marq->focus = 0;
}

/*------------------------------------------------------------*/
int main_window::init ( void )
{
  unsigned int	i;
  long	max_resolution;
  scan_manager	* scan_mgr;
  const scanner_info	* scan_info;

  scan_mgr = ::g_view_manager->get_scan_manager ( );
  scan_info = scan_mgr->get_scanner_info ( );

  for ( i = 0; i < WG_MAIN_NUM; i++ )
    m_widget [ i ] = 0;

  ::memcpy ( & ::g_view_manager->m_set.imgtype,
	     & ::g_imagetype_list [ 0 ],
	     sizeof ( imagetype ) );
  
  // destination
  ::strcpy ( ::g_destination_menu_item [ 0 ].name, _( "File" ) );
  ::strcpy ( ::g_destination_menu_item [ 1 ].name, _( "Printer" ) );

  // document source
  ::strcpy ( ::g_docsrc_menu_item [ 0 ].name, _( "Flatbed" ) );
  ::strcpy ( ::g_docsrc_menu_item [ 1 ].name, _( "TPU for Neg.Film" ) );
  ::strcpy ( ::g_docsrc_menu_item [ 2 ].name, _( "TPU for Pos.Film" ) );
  ::strcpy ( ::g_docsrc_menu_item [ 3 ].name, _( "Auto Document Feeder" ) );

  // image type
  ::strcpy ( ::g_imgtype_menu_item [ 0 ].name, _( "Color Photo" ) );
  ::strcpy ( ::g_imgtype_menu_item [ 1 ].name, _( "Color Document" ) );
  ::strcpy ( ::g_imgtype_menu_item [ 2 ].name, _( "Black & White Photo" ) );
  ::strcpy ( ::g_imgtype_menu_item [ 3 ].name, _( "Black & White Document" ) );
  ::strcpy ( ::g_imgtype_menu_item [ 4 ].name, _( "Line Art" ) );

  // resolution
  max_resolution = scan_info->max_resolution;
  for ( i = 0; i < sizeof ( g_resolution_menu_item ) /
		sizeof ( g_resolution_menu_item [ 0 ] ); i++ )
    {
      if ( max_resolution < g_resolution_menu_item [ i ].id )
	{
	  g_resolution_menu_item [ i ].id = 0;
	  g_resolution_menu_item [ i ].func = 0;
	}
    }

  // unit
  ::strcpy ( ::g_unit_menu_item [ 0 ].name, _( "inches" ) );
  ::strcpy ( ::g_unit_menu_item [ 1 ].name, _( "pixels" ) );
  ::strcpy ( ::g_unit_menu_item [ 2 ].name, _( "cm" ) );

  // ::fprintf ( stderr, "main_window::init ( ):end\n" );

  return PISA_ERR_SUCCESS;
}


/*------------------------------------------------------------*/
GtkWidget * main_window::create_window ( GtkWidget * parent )
{
  GtkWidget	* top;
  GtkWidget	* hbox;
  GtkWidget	* left_area, * right_area;

  parent = parent;

  // main window
  top = m_widget [ WG_MAIN_TOP ] = ::gtk_window_new ( GTK_WINDOW_TOPLEVEL );
  ::gtk_container_border_width ( GTK_CONTAINER ( top ), 10 );
  ::gtk_window_set_title ( GTK_WINDOW ( top ), PACKAGE_STRING );
  ::gtk_widget_set_uposition ( top, POS_MAIN_X, POS_MAIN_Y );
  
  ::gtk_signal_connect ( GTK_OBJECT ( top ), "delete_event",
			 GTK_SIGNAL_FUNC ( ::delete_event ), 0 );
  
  // hbox
  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_container_add ( GTK_CONTAINER ( top ), hbox );
  ::gtk_widget_show ( hbox );
  
  // left area
  left_area = create_left_area ( );
  // In order to avoid a segmentation error in the preview_window
  // class (resulting from sloppy image size logic), we add a size
  // kludge here so that the conditions that trigger it become very
  // unlikely.
  // FIXME: remove kludge once preview_window gets fixed.
  ::gtk_widget_set_usize (GTK_WIDGET (left_area), 360, 480);
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), left_area, TRUE, TRUE, 0 );
  ::gtk_widget_show ( left_area );
  
  // right area
  right_area = create_right_area ( );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), right_area, FALSE, FALSE, 0 );
  ::gtk_widget_show ( right_area );
  
  ::gtk_widget_show ( top );

  ::gtk_widget_grab_focus ( m_widget [ WG_MAIN_PREV_BTN ] );

  ::g_view_manager->sensitive ( );

  return top;
}

/*------------------------------------------------------------*/
int main_window::close_window ( int destroy )
{
  ::g_view_manager->close_window ( ID_WINDOW_PREV, 1 );

  if ( destroy && m_widget [ WG_MAIN_TOP ] )
    ::gtk_widget_destroy ( m_widget [ WG_MAIN_TOP ] );
  
  m_widget [ WG_MAIN_TOP ] = 0;

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void main_window::sensitive ( int is_prev_img )
{
  gboolean enable_zoom, enable_expose, enable_dest, enable_config;
  long	marq_num;

  enable_zoom = enable_expose = enable_dest = enable_config= TRUE;

  marq_num = ::g_view_manager->m_set.get_marquee_size ( );
  
  if ( marq_num < 2 || is_prev_img == 0 )
    enable_zoom = FALSE;

  if ( is_prev_img == 0 ||
       ::g_view_manager->m_set.imgtype.pixeltype == PISA_PT_BW )
    enable_expose = FALSE;

  if ( ::g_view_manager->is_gimp ( ) )
    {
      enable_dest = FALSE;
      enable_config = FALSE;
    }

  ::gtk_widget_set_sensitive ( m_widget [ WG_MAIN_ZOOM_BTN ], enable_zoom );
  ::gtk_widget_set_sensitive ( m_widget [ WG_MAIN_EXPO_BTN ], enable_expose );
  ::gtk_widget_set_sensitive ( m_widget [ WG_MAIN_DEST_MENU ], enable_dest );
  ::gtk_widget_set_sensitive ( m_widget [ WG_MAIN_CONFIG_BTN ], enable_config );
  
  // image type
  sensitive_image_type ( );

  // scale
  sensitive_scale ( );

  // resolution
  sensitive_resolution ( );

  // target
  sensitive_target ( );

  // focus
  sensitive_focus ( );

  // unsharp mask
  sensitive_usm ( );
}

void
main_window::enable_start_button (bool yes)
{
  ::gtk_widget_set_sensitive (m_widget [WG_MAIN_START_BTN], yes);
}


/*------------------------------------------------------------*/
GtkWidget * main_window::create_left_area ( void )
{
  GtkWidget	* vbox, * hbox;
  GtkWidget	* widget;
  
  vbox = ::gtk_vbox_new ( FALSE, 0 );

  hbox = ::gtk_hbox_new ( FALSE, 0 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

  // preview area
  widget = create_preview_window ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, TRUE, TRUE, 0 );
  ::gtk_widget_show ( widget );

  ::gtk_widget_realize ( m_widget [ WG_MAIN_TOP ] );

  // preview, zoom, auto exposure button
  widget = create_preview_button ( );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), widget, TRUE, TRUE, 0 );
  ::gtk_widget_show ( widget );

  ::gtk_widget_show ( hbox );

  // destination menu and scan button
  widget = create_scan_button ( );
  ::gtk_box_pack_end ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );

  return vbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_preview_button ( void )
{
  GtkWidget	* hbox;
  GtkWidget	* button;
  GtkWidget	* imgbox;

  hbox = ::gtk_hbox_new ( FALSE, 5 );
  
  // preview button
  button = ::gtk_button_new ( );
  imgbox = ::xpmlabelbox ( m_widget [ WG_MAIN_TOP ],
			   preview_xpm, _( "Preview" ) );
  ::gtk_container_add ( GTK_CONTAINER ( button ), imgbox );
  ::gtk_widget_show ( imgbox );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( ::click_preview_btn ), 0 );
  ::gtk_widget_show ( button );
  m_widget [ WG_MAIN_PREV_BTN ] = button;

  // zoom button
  button = ::gtk_button_new ( );
  imgbox = ::xpmlabelbox ( m_widget [ WG_MAIN_TOP ], zoom_xpm, _( "Zoom" ) );
  ::gtk_container_add ( GTK_CONTAINER ( button ), imgbox );
  ::gtk_widget_show ( imgbox );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( ::click_zoom_btn ), 0 );
  ::gtk_widget_show ( button );
  m_widget [ WG_MAIN_ZOOM_BTN ] = button;

  // auto exposure button
  button = ::gtk_button_new ( );
  imgbox = ::xpmlabelbox ( m_widget [ WG_MAIN_TOP ],
			   auto_xpm, _( "Auto Exposure" ) );
  ::gtk_container_add ( GTK_CONTAINER ( button ), imgbox );
  ::gtk_widget_show ( imgbox );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( ::click_expose_btn ), 0 );
  ::gtk_widget_show ( button );
  m_widget [ WG_MAIN_EXPO_BTN ] = button;

  return hbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_preview_window ( void )
{
  GtkWidget	* prev;

  prev = ::g_view_manager->create_window ( ID_WINDOW_PREV );

  return prev;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_scan_button ( void )
{
  GtkWidget	* table;
  GtkWidget	* label;
  GtkWidget	* menu;
  GtkWidget     * start_button;
  GtkWidget	* button;
  GtkWidget	* imgbtn;

  table = ::gtk_table_new ( 2, 3, TRUE );
  ::gtk_container_border_width ( GTK_CONTAINER ( table ), 5 );
  ::gtk_table_set_row_spacings ( GTK_TABLE ( table ), 3 );
  ::gtk_table_set_col_spacings ( GTK_TABLE ( table ), 3 );
  
  label = ::gtk_label_new ( _( "Destination:" ) );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  menu = ::pisa_create_option_menu ( g_destination_menu_item );
  ::gtk_table_attach ( GTK_TABLE ( table ), menu, 1, 2, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( menu );
  m_widget [ WG_MAIN_DEST_MENU ] = menu;

  button = ::gtk_button_new ( );
  imgbtn = ::xpmlabelbox ( m_widget [ WG_MAIN_TOP ], scan_xpm, _( "Scan" ) );
  ::gtk_container_add ( GTK_CONTAINER ( button ), imgbtn );
  ::gtk_widget_show ( imgbtn );

  ::gtk_table_attach ( GTK_TABLE ( table ), button, 2, 3, 0, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( ::click_scan_btn ), 0 );


  ::gtk_widget_show ( button );
  m_widget [ WG_MAIN_SCAN_BTN ] = button;

  start_button = ::gtk_check_button_new_with_label ( _( "enable Start button" ) );
  :: gtk_signal_connect ( GTK_OBJECT ( start_button ), "toggled",
			  GTK_SIGNAL_FUNC ( ::toggled_start_button ), 0 );
  gtk_table_attach ( GTK_TABLE ( table ), start_button, 0, 2, 1, 2,
		     GTK_FILL, GTK_FILL, 0, 0 );

  {
    const scanner_info *info
      = g_view_manager->get_scan_manager ()->get_scanner_info ();

    if (info->support_start_button) {
      ::gtk_widget_show ( start_button );
    }
  }
  m_widget [WG_MAIN_START_BTN ] = start_button;

  return table;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_right_area ( void )
{
  GtkWidget	* vbox;
  GtkWidget	* widget;

  vbox = ::gtk_vbox_new ( FALSE, 5 );

#if THE_ORIGINAL_SOURCES_WERE_NOT_SUCH_A_MESS
  scan_selector  *ss = new scan_selector();
  ::gtk_box_pack_start( GTK_BOX( vbox ), ss->widget(), FALSE, FALSE, 0 );
  ss->update();			// seed list of available scanners
  ss->select();			// pick one
  ss->show();
#else
  widget = create_scan_label ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );
#endif /* THE_ORIGINAL_SOURCES_WERE_NOT_SUCH_A_MESS */
  
  widget = create_main_tab ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );

  widget = create_close_button ( );
  ::gtk_box_pack_end ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );

  return vbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_scan_label ( void )
{
  GtkWidget *hbox;
  GtkWidget *widget;

  hbox = ::gtk_hbox_new( FALSE, 0 );

  widget = ::gtk_label_new( _("Scanner:") );
  ::gtk_box_pack_start( GTK_BOX( hbox ), widget, FALSE, FALSE, 5 );
  ::gtk_widget_show( widget );

  widget = ::gtk_label_new( ::g_view_manager->get_device_name() );
  ::gtk_box_pack_start( GTK_BOX( hbox ), widget, FALSE, FALSE, 5 );
  ::gtk_widget_show( widget );

  return hbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_main_tab ( void )
{
  GtkWidget	* notebook;
  GtkWidget	* label;
  GtkWidget	* widget;

  notebook = ::gtk_notebook_new ( );
  ::gtk_container_border_width ( GTK_CONTAINER ( notebook ), 5 );

  // document tab
  label = ::gtk_label_new ( _( "  Document  " ) );
  widget = create_document_tab ( );

  ::gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), widget, label );
  ::gtk_widget_show ( label );
  ::gtk_widget_show ( widget );

  // adjust tab
  label = ::gtk_label_new ( _( "  Adjust  " ) );
  widget = create_adjust_tab ( );

  ::gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), widget, label );
  ::gtk_widget_show ( label );
  ::gtk_widget_show ( widget );

  return notebook;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_document_tab ( void )
{
  GtkWidget	* vbox;
  GtkWidget	* table;
  GtkWidget	* label;
  GtkWidget	* menu;
  GtkWidget	* usm;
  GtkWidget	* size;
  GtkWidget	* focus;
  GtkWidget	* penguin;
  scan_manager	* scan_mgr;
  const scanner_info	* scan_info;

  scan_mgr = ::g_view_manager->get_scan_manager ( );
  scan_info = scan_mgr->get_scanner_info ( );

  vbox = ::gtk_vbox_new ( FALSE, 0 );

  table = ::gtk_table_new ( 3, 2, FALSE );
  ::gtk_container_border_width ( GTK_CONTAINER ( table ), 5 );
  ::gtk_table_set_row_spacings ( GTK_TABLE ( table ), 3 );
  ::gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), table, FALSE, FALSE, 5 );
  ::gtk_widget_show ( table );

  // document source
  label = ::gtk_label_new ( _( "Document Source:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  menu = ::pisa_create_option_menu ( g_docsrc_menu_item );
  ::gtk_table_attach ( GTK_TABLE ( table ), menu, 1, 2, 0, 1,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( menu );

  if ( scan_info->support_option != PISA_OP_ADF )
    {
      ::gtk_widget_set_sensitive (  g_docsrc_menu_item [ 3 ].widget, FALSE );
    }
  if ( scan_info->support_option != PISA_OP_TPU )
    {
      ::gtk_widget_set_sensitive (  g_docsrc_menu_item [ 1 ].widget, FALSE );
      ::gtk_widget_set_sensitive (  g_docsrc_menu_item [ 2 ].widget, FALSE );
    }

  // image type
  label = ::gtk_label_new ( _( "Image Type:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  menu = ::pisa_create_option_menu ( g_imgtype_menu_item );
  ::gtk_table_attach ( GTK_TABLE ( table ), menu, 1, 2, 1, 2,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( menu );
  m_widget [ WG_MAIN_IMG_MENU ] = menu;

  // resolution
  label = ::gtk_label_new ( _( "Resolution:" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
  ::gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( label );

  menu = ::pisa_create_option_menu ( g_resolution_menu_item );
  ::gtk_table_attach ( GTK_TABLE ( table ), menu, 1, 2, 2, 3,
		       GTK_FILL, GTK_FILL, 0, 0 );
  ::gtk_widget_show ( menu );
  m_widget [ WG_MAIN_RES_MENU ] = menu;

  // unsharp mask
  usm = ::gtk_check_button_new_with_label ( _( "Unsharp mask" ) );
  ::gtk_signal_connect ( GTK_OBJECT ( usm ), "toggled",
			 GTK_SIGNAL_FUNC ( ::toggled_usm ), 0 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), usm, FALSE, FALSE, 0 );
  ::gtk_widget_show ( usm );
  m_widget [ WG_MAIN_USM ] = usm;
  
  // target
  size = create_target ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), size, FALSE, FALSE, 0 );
  ::gtk_widget_show ( size );

  // focus
  if ( scan_info->support_focus )
    {
      focus = create_focus ( );
      ::gtk_box_pack_start ( GTK_BOX ( vbox ), focus, FALSE, FALSE, 0 );
      ::gtk_widget_show ( focus );
    }

  // penguin image
  penguin = create_penguin ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), penguin, TRUE, TRUE, 0 );
  ::gtk_widget_show ( penguin );

  return vbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_target ( void )
{
  GtkWidget	* frame;
  GtkWidget	* vbox, * subvbox;
  GtkWidget	* hbox;
  GtkWidget	* label;
  GtkWidget	* entry;
  GtkWidget	* menu;
  GtkWidget	* scale;
  GtkWidget	* separator;

  frame = ::gtk_frame_new ( _( "Target" ) );
  ::gtk_container_border_width ( GTK_CONTAINER ( frame ), 5 );

  vbox = ::gtk_vbox_new ( FALSE, 5 );
  ::gtk_container_add ( GTK_CONTAINER ( frame ), vbox );
  ::gtk_widget_show ( vbox );

  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 2 );
  ::gtk_widget_show ( hbox );

  label = ::gtk_label_new ( _( "W:" ) );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
  ::gtk_widget_show ( label );

  entry = ::gtk_entry_new ( );
  ::gtk_widget_set_usize ( entry, 60, -1 );
  ::gtk_entry_set_text ( GTK_ENTRY ( entry ), "8.5" );
  ::gtk_widget_set_sensitive ( entry, FALSE );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), entry, FALSE, FALSE, 2 );
  ::gtk_widget_show ( entry );
  m_widget [ WG_MAIN_WIDTH ] = entry;

  label = ::gtk_label_new ( _( "H:" ) );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
  ::gtk_widget_show ( label );

  entry = ::gtk_entry_new ( );
  ::gtk_widget_set_usize ( entry, 60, -1 );
  ::gtk_entry_set_text ( GTK_ENTRY ( entry ), "11.7" );
  ::gtk_widget_set_sensitive ( entry, FALSE );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), entry, FALSE, FALSE, 2 );
  ::gtk_widget_show ( entry );
  m_widget [ WG_MAIN_HEIGHT ] = entry;

  menu = ::pisa_create_option_menu ( ::g_unit_menu_item );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), menu, FALSE, FALSE, 5 );
  ::gtk_widget_show ( menu );

  // separator
  separator = ::gtk_hseparator_new ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 5 );
  ::gtk_widget_show ( separator );

  // scale
  subvbox = ::gtk_vbox_new ( FALSE, 0 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), subvbox, FALSE, FALSE, 0 );
  ::gtk_widget_show ( subvbox );

  hbox = ::gtk_hbox_new ( FALSE, 0 );
  ::gtk_box_pack_start ( GTK_BOX ( subvbox ), hbox, FALSE, FALSE, 0 );
  ::gtk_widget_show ( hbox );
  
  label = ::gtk_label_new ( _( "Scale" ) );
  ::gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
  ::gtk_widget_show ( label );

  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_box_pack_start ( GTK_BOX ( subvbox ), hbox, FALSE, FALSE, 5 );
  ::gtk_widget_show ( hbox );

  scale = ::pisa_create_scale ( & ::g_scale_zoom );
  ::gtk_widget_set_usize ( scale, 200, -1 );
  ::gtk_scale_set_digits ( GTK_SCALE ( scale ), 0 );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), scale, TRUE, TRUE, 5 );
  ::gtk_widget_show ( scale );

  return frame;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_focus ( void )
{
  GtkWidget	* frame;
  GtkWidget	* hbox;
  GtkWidget	* radio;
  GSList	* group;

  frame = ::gtk_frame_new ( _( "Focus" ) );
  ::gtk_container_border_width ( GTK_CONTAINER ( frame ), 5 );

  hbox = ::gtk_hbox_new ( FALSE, 5 );
  ::gtk_container_border_width ( GTK_CONTAINER ( hbox ), 5 );
  ::gtk_container_add ( GTK_CONTAINER ( frame ), hbox );
  ::gtk_widget_show ( hbox );
 
  radio = ::gtk_radio_button_new_with_label ( 0, "0.0" );
  ::gtk_signal_connect ( GTK_OBJECT ( radio ), "toggled",
			 GTK_SIGNAL_FUNC ( check_focus ), & ::g_focus [ 0 ] );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), radio, TRUE, TRUE, 0 );
  ::gtk_widget_show ( radio );
  m_widget [ WG_MAIN_FOCUS_0 ] = radio;

  group = ::gtk_radio_button_group ( GTK_RADIO_BUTTON ( radio ) );
  radio = ::gtk_radio_button_new_with_label ( group, "2.5" );
  ::gtk_signal_connect ( GTK_OBJECT ( radio ), "toggled",
			 GTK_SIGNAL_FUNC ( check_focus ), & ::g_focus [ 1 ] );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), radio, TRUE, TRUE, 0 );
  ::gtk_widget_show ( radio );
  m_widget [ WG_MAIN_FOCUS_25 ] = radio;

  return frame;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_penguin ( void )
{
  GtkWidget	* penguin;

  penguin = ::xpm2widget ( m_widget [ WG_MAIN_TOP ], penguin_xpm );

  return penguin;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_adjust_tab ( void )
{
  GtkWidget	* vbox;
  GtkWidget	* widget;

  vbox = gtk_vbox_new ( FALSE, 5 );
  
  // image controls
  widget = ::g_view_manager->create_window ( ID_WINDOW_IMGCTRL );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );

  // tone correction
  widget = ::g_view_manager->create_window ( ID_WINDOW_GAMMA );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );
  ::gtk_widget_show ( widget );

  return vbox;
}

/*------------------------------------------------------------*/
GtkWidget * main_window::create_close_button ( void )
{
  GtkWidget	* vbox;
  GtkWidget	* separator;
  GtkWidget	* hbox;
  GtkWidget	* button;

  vbox = ::gtk_vbox_new ( TRUE, 5 );

  separator = ::gtk_hseparator_new ( );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), separator, FALSE, FALSE, 0 );
  ::gtk_widget_show ( separator );

  hbox = ::gtk_hbox_new ( TRUE, 5 );
  ::gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

  // configuration button
  button = ::gtk_button_new_with_label ( _( "  Configuration  " ) );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( ::click_config ), 0 );
  ::gtk_widget_show ( button );
  m_widget [ WG_MAIN_CONFIG_BTN ] = button;

  // close button
  button = ::gtk_button_new_with_label ( _( "  Close  " ) );
  ::gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );
  ::gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC ( ::click_close ), 0 );
  ::gtk_widget_show ( button );

  ::gtk_widget_show ( hbox );

  return vbox;
}

/*------------------------------------------------------------*/
void main_window::sensitive_image_type ( void )
{
  gboolean	enable_bw;
  int		i;

  if ( ::g_view_manager->m_set.option == PISA_OP_TPU )
    enable_bw = FALSE;
  else
    enable_bw = TRUE;

  for ( i = 0; ::g_imgtype_menu_item [ i ].id != 0; i++ )
    {
      if ( ::g_imagetype_list [ i ].pixeltype == PISA_PT_BW )
      ::gtk_widget_set_sensitive ( ::g_imgtype_menu_item [ i ].widget, enable_bw );
    }

  for ( i = 0; ::g_imgtype_menu_item [ i ].id != 0; i++ )
    {
      if ( ::g_imgtype_menu_item [ i ].id == ::g_view_manager->m_set.imgtype.id )
	{
	  ::gtk_option_menu_set_history ( GTK_OPTION_MENU ( m_widget [ WG_MAIN_IMG_MENU ] ), i );
	  break;
	}
    }
}

/*------------------------------------------------------------*/
void main_window::sensitive_target ( void )
{
  long		resolution, unit, scale, scale_px;
  double	w, h;
  char		w_buf [ 32 ], h_buf [ 32 ];
  marquee	* marq;
  scan_manager	* scan_mgr;

  scan_mgr = ::g_view_manager->get_scan_manager ( );

  resolution	= ::g_view_manager->m_set.resolution;
  unit		= ::g_view_manager->m_set.unit;

  marq = ::g_view_manager->m_set.get_current_marquee ( );

  w = marq->area.x;
  h = marq->area.y;
  scale = marq->scale;

  resolution = (resolution * scale + 50) / 100;

  scan_mgr->adjust_scan_param (&resolution, &scale_px);
  
  switch ( unit )
    {
    case PISA_UNIT_INCHES:
      w = ::inches_reflect_zoom ( w, scale ) + 0.005;
      h = ::inches_reflect_zoom ( h, scale ) + 0.005;
      ::sprintf ( w_buf, "%u.%02u",
		  ( int ) w,
		  ( int ) ( w * 100 - ( int ) w * 100 ) );
      ::sprintf ( h_buf, "%u.%02u",
		  ( int ) h,
		  ( int ) ( h * 100 - ( int ) h * 100 ) );
      break;
      
    case PISA_UNIT_PIXELS:
      w = ::inch2width ( w, resolution, scale_px,
			 PISA_PT_BW == ::g_view_manager->m_set.imgtype.pixeltype);
      h = ::inch2height ( h, resolution, scale_px );
      ::sprintf ( w_buf, "%u", ( int ) w );
      ::sprintf ( h_buf, "%u", ( int ) h );
      break;

    case PISA_UNIT_CM:
      w = ::inch2centi ( w, scale ) + 0.005;
      h = ::inch2centi ( h, scale ) + 0.005;
      ::sprintf ( w_buf, "%u.%02u",
		  ( int ) w,
		  ( int ) ( w * 100 - ( int ) w * 100 ) );
      ::sprintf ( h_buf, "%u.%02u",
		  ( int ) h,
		  ( int ) ( h * 100 - ( int ) h * 100 ) );
      break;
    }

  ::gtk_entry_set_text ( GTK_ENTRY ( m_widget [ WG_MAIN_WIDTH ] ), w_buf );
  ::gtk_entry_set_text ( GTK_ENTRY ( m_widget [ WG_MAIN_HEIGHT ] ), h_buf );
}

/*------------------------------------------------------------*/
void main_window::sensitive_resolution ( void )
{
  long	max_resolution, i, resolution;

  max_resolution = get_max_enable_res ( );
 
  for ( i = 0; ::g_resolution_menu_item [ i ].id != 0; i++ )
    {
      if ( max_resolution < ::g_resolution_menu_item [ i ].id )
	::gtk_widget_set_sensitive ( ::g_resolution_menu_item [ i ].widget,
				     FALSE );
      else
	::gtk_widget_set_sensitive ( ::g_resolution_menu_item [ i ].widget,
				     TRUE );
    }

  resolution = ::g_view_manager->m_set.resolution;

  for ( i = 0; ::g_resolution_menu_item [ i ].id != 0; i++ )
    {
      if ( ::g_resolution_menu_item [ i ].id == resolution )
	{
	  ::gtk_option_menu_set_history ( GTK_OPTION_MENU ( m_widget [ WG_MAIN_RES_MENU ] ), i );
	  break;
	}
    }
}

/*------------------------------------------------------------*/
void main_window::sensitive_scale ( void )
{
  long	min_scale, max_scale;
  GtkAdjustment	* adjust;
  marquee	* marq;

  calc_scale_limit ( & min_scale, & max_scale );
  marq = ::g_view_manager->m_set.get_current_marquee ( );

  if ( marq->scale < min_scale )
    marq->scale = min_scale;
  if ( marq->scale > max_scale )
    marq->scale = max_scale;

  adjust = ::gtk_range_get_adjustment ( GTK_RANGE ( ::g_scale_zoom.widget ) );
  adjust->lower = min_scale;
  adjust->upper = max_scale;
  adjust->value = marq->scale;

  ::gtk_signal_emit_by_name ( GTK_OBJECT ( ::g_scale_zoom.adjust ),
			      "changed" );  
}

/*------------------------------------------------------------*/
void main_window::sensitive_focus ( void )
{
  scan_manager	* scan_mgr;
  const scanner_info	* scan_info;
  marquee	* marq;

  scan_mgr = ::g_view_manager->get_scan_manager ( );
  scan_info = scan_mgr->get_scanner_info ( );

  if ( scan_info->support_focus == 0 )
    return;

  marq = ::g_view_manager->m_set.get_current_marquee ( );

  if (m_widget[WG_MAIN_FOCUS_25] && marq->focus == 25)
    ::gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( m_widget [ WG_MAIN_FOCUS_25 ] ),
				     TRUE );
  if (m_widget[WG_MAIN_FOCUS_0 ] && marq->focus ==  0)
    ::gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( m_widget [ WG_MAIN_FOCUS_0 ] ),
				     TRUE );
    
}

/*------------------------------------------------------------*/
void main_window::sensitive_usm ( void )
{
  if ( ::g_view_manager->m_set.usm )
    ::gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( m_widget [ WG_MAIN_USM ] ),
				     TRUE );
  else
    ::gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( m_widget [ WG_MAIN_USM ] ),
				     FALSE );

  if ( ::g_view_manager->m_set.imgtype.pixeltype == PISA_PT_BW )
    ::gtk_widget_set_sensitive ( m_widget [ WG_MAIN_USM ],
				 FALSE );
  else
    ::gtk_widget_set_sensitive ( m_widget [ WG_MAIN_USM ],
				 TRUE );

}

/*------------------------------------------------------------*/
long main_window::get_max_scale ( void )
{
  long		i, marq_num;
  long		max_scale;
  settings	* set;
  marquee	* marq;

  max_scale = 1;
  set = & ::g_view_manager->m_set;

  marq_num = set->get_marquee_size ( );

  if ( marq_num == 1 )
    {
      marq = set->get_marquee ( 0 );

      max_scale = marq->scale;
    }
  else
    {
      for ( i = 1; i < marq_num; i++ )
	{
	  marq = set->get_marquee ( i );
	  
	  if ( max_scale < marq->scale )
	    max_scale = marq->scale;
	}
    }

  return max_scale;
}

/*------------------------------------------------------------*/
long main_window::get_max_enable_res ( void )
{
  long	ret_res, max_res;
  long	max_scale;
  scan_manager	* scan_mgr;
  const scanner_info	* scan_info;

  scan_mgr = ::g_view_manager->get_scan_manager ( );
  scan_info = scan_mgr->get_scanner_info ( );

  max_res = scan_info->max_resolution;
  max_scale = get_max_scale ( );

  ret_res = max_res * 2;

  if ( 100 < max_scale )
    ret_res = max_res * 200 / max_scale;

  if ( max_res < ret_res )
    ret_res = max_res;

  return ret_res;
}

/*------------------------------------------------------------*/
void main_window::calc_scale_limit ( long * min_scale, long * max_scale )
{
  long	resolution, min_res, max_res;
  scan_manager	* scan_mgr;
  const scanner_info	* scan_info;

  resolution = ::g_view_manager->m_set.resolution;

  scan_mgr = ::g_view_manager->get_scan_manager ( );
  scan_info = scan_mgr->get_scanner_info ( );

  min_res = 50;
  max_res = scan_info->max_resolution;

  if ( min_res * 50 % resolution )
    * min_scale = min_res * 50 / resolution + 1;
  else
    * min_scale = min_res * 50 / resolution;

  if ( * min_scale < 1 )
    * min_scale = 1;

  * max_scale = max_res * 200 / resolution;
}

