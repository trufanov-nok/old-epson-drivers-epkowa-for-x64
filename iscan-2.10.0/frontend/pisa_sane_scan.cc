/* pisa_sane_scan.cc
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

/*------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*------------------------------------------------------------*/
#include "pisa_sane_scan.h"
#include "pisa_error.h"
#include "pisa_change_unit.h"

/*------------------------------------------------------------*/
#define MM_PER_INCH	25.4

#define SCSI_STR	"SCSI"
#define PIO_STR		"PIO"
#define USB_STR		"USB"

/*------------------------------------------------------------*/
void sane_scan::init ( void )
{
  m_hdevice	= 0;

  ::atexit ( sane_exit );
  ::sane_init ( 0, 0 );
}

/*------------------------------------------------------------*/
void sane_scan::open_device ( scanner_info * info, char * name )
{
  SANE_Status status;
  char *device_name = 0;

  if (!name)
    {
      device_name = new char[64];
      query_device( device_name );
    }
  else
    device_name = name;

  SANE_Handle device;
  status = ::sane_open ( device_name, &device );
  
  if ( SANE_STATUS_GOOD != status)
    {
      if (device_name != name)
	delete[] device_name;
      throw pisa_error ( status );
    }

  // we successfully opened a new device, now close whatever we are
  // hanging onto and rebind to the newly opened device

  close_device();
  m_hdevice = device;

  get_scanner_info ( info, device_name );

  if (device_name != name)
    delete[] device_name;
}

/*------------------------------------------------------------*/
void sane_scan::close_device ( void )
{
  if ( m_hdevice )
    ::sane_close ( m_hdevice );

  m_hdevice = 0;
}

/*------------------------------------------------------------*/
void sane_scan::set_option ( pisa_option_type option )
{
  char	documentsource [ 32 ];

  if ( ! m_hdevice )
    throw pisa_error ( PISA_ERR_CONNECT );

  switch ( option )
    {
    case PISA_OP_FLATBED:
      ::strcpy ( documentsource, "Flatbed" );
      break;
      
    case PISA_OP_ADF:
    case PISA_OP_ADFDPLX:
      ::strcpy ( documentsource, "Automatic Document Feeder" );
      break;

    case PISA_OP_TPU:
      ::strcpy ( documentsource, "Transparency Unit" );
      break;

    default:
      throw pisa_error ( PISA_ERR_PARAMETER );
    }

  set_value ( SANE_NAME_SCAN_SOURCE, ( void * ) documentsource );
}

/*------------------------------------------------------------*/
void sane_scan::set_film_type ( pisa_film_type film )
{
  char	film_type [ 32 ];

  switch ( film )
    {
    case PISA_FT_POSI:
      ::strcpy ( film_type, "Positive Film" );
      break;

    case PISA_FT_NEGA:
      ::strcpy ( film_type, "Negative Film" );
      break;

    case PISA_FT_REFLECT:
      return;
    }

  set_value ( "film-type", ( void * ) film_type );
}

/*------------------------------------------------------------*/
void sane_scan::get_current_max_size ( double * width, double * height )
{
  SANE_Word value;

  if ( ! m_hdevice )
    throw pisa_error (  PISA_ERR_CONNECT );

  // bottom-right x
  get_value ( SANE_NAME_SCAN_BR_X, ( void * ) & value );

  * width = SANE_UNFIX ( value ) / MM_PER_INCH;

  // bottom-right y
  get_value ( SANE_NAME_SCAN_BR_Y, ( void * ) & value );

  * height = SANE_UNFIX ( value ) / MM_PER_INCH;
}

/*------------------------------------------------------------*/
void sane_scan::get_color_profile ( double * coef )
{
  SANE_Word	value;
  char		option_name [ 16 ];
  int		i;

  if ( ! m_hdevice )
    throw pisa_error ( PISA_ERR_CONNECT );
  
  for ( i = 0; i < 9; i++ )
    {
      ::sprintf ( option_name, "cct-%d", i + 1 );

      get_value ( option_name, ( void * ) & value );

      coef [ i ] = SANE_UNFIX ( value );
    }
}

/*------------------------------------------------------------*/
void sane_scan::set_scan_parameter ( const scan_parameter * param )
{
  if ( ! m_hdevice )
    throw pisa_error ( PISA_ERR_CONNECT );

  update_settings ( param );
}

/*------------------------------------------------------------*/
void sane_scan::start_scan ( int * width, int * height )
{
  SANE_Status	status;

  if ( ! m_hdevice )
    throw pisa_error ( PISA_ERR_CONNECT );

  // start scan
  if ( SANE_STATUS_GOOD != ( status = ::sane_start ( m_hdevice ) ) )
    {
      ::sane_cancel ( m_hdevice );
      throw pisa_error ( status );	// error
    }

  // read header
  if ( SANE_STATUS_GOOD != ( status = ::sane_get_parameters ( m_hdevice,
								   & m_sane_para ) ) )
    {
      ::sane_cancel ( m_hdevice );
      throw pisa_error ( status );	// error
    }
  
  * width  = m_sane_para.pixels_per_line;
  * height = m_sane_para.lines;
  
  m_rows = 0;
}

/*------------------------------------------------------------*/
void sane_scan::acquire_image ( unsigned char * img,
				int row_bytes,
				int height,
				int cancel )
{
  if ( ! m_hdevice )
    throw pisa_error ( PISA_ERR_CONNECT );

  SANE_Status    status  = SANE_STATUS_GOOD;
  unsigned char *cur_pos = img;

  for (int i = 0; i < height; i++, cur_pos += row_bytes)
    {
      m_rows++;
      if ( cancel )
	{
	  ::sane_cancel ( m_hdevice );
	  return;	// cancel
	}

      // The SANE standard does not promise to return as much data as
      // we request, so we keep asking until we got all that we want.

      int cnt = row_bytes;
      int len = 0;
      while (SANE_STATUS_GOOD == status && cnt > 0)
	{
	  status = ::sane_read (m_hdevice, cur_pos + (row_bytes - cnt),
				cnt, & len);

	  cnt -= len;
	}

      if ( status == SANE_STATUS_EOF )
	break;

      if ( status != SANE_STATUS_GOOD && status != SANE_STATUS_CANCELLED )
	{
	  throw pisa_error ( status );	// error
	}

    }

  if ( m_rows == m_sane_para.lines )
    {
      ::sane_cancel ( m_hdevice );
    }

  if ( status == SANE_STATUS_EOF && m_sane_para.last_frame != 0 )
    ::sane_cancel ( m_hdevice );

  return;
}

/*! \brief  Returns the largest resolution not larger than a \a cutoff.

    Returns the largest supported hardware resolution that does not
    exceed the \a cutoff.  Passing a negative value for the \a cutoff,
    which is the default, returns the maximum resolution supported by
    the hardware.

    Throws an unsupported exception when no suitable resolution can be
    determined.
 */
long
sane_scan::get_max_resolution (long cutoff)
{
  int opt_id = get_option_id (SANE_NAME_SCAN_RESOLUTION);

  const SANE_Option_Descriptor
    *opt_desc = ::sane_get_option_descriptor (m_hdevice, opt_id);

  if (!opt_desc || SANE_TYPE_INT != opt_desc->type)
    throw pisa_error (PISA_STATUS_UNSUPPORTED);

  long result = -1;

  switch (opt_desc->constraint_type)
    {
    case SANE_CONSTRAINT_RANGE:
      {
	const SANE_Range *const range = opt_desc->constraint.range;

	if (0 > cutoff) cutoff = range->max;
	if (0 == range->quant)
	  {
	    result = cutoff;
	  }
	else
	  {			// relies on integer arithmetic
	    result = (((cutoff - range->min) / range->quant)
		      * range->quant) + range->min;
	  }
	break;
      }
    case SANE_CONSTRAINT_WORD_LIST:
      {				// assumes list is in ascending order
	const SANE_Word *list = opt_desc->constraint.word_list;

	size_t last = list[0];

	if (0 > cutoff) cutoff = list[last];
	while (0 < last && cutoff < list[last])
	  {
	    --last;
	  }
	if (0 < last) result = list[last];
	break;
      }
    }

  if (0 > result)
    throw pisa_error (PISA_STATUS_UNSUPPORTED);

  return result;
}

/*------------------------------------------------------------*/
void sane_scan::get_scanner_info ( scanner_info * info, char * name )
{
  int	i, opt_id;
  const SANE_Option_Descriptor	* opt_desc;
  const char * tpu = "Transparency Unit";
  const char * adf = "Automatic Document Feeder";

  if (name)
    {
      char *copy = (char *) malloc( (strlen( name ) + 1) * sizeof( char ) );
      if (!copy)
	throw pisa_error( PISA_STATUS_NO_MEM );
      strcpy( copy, name );

      if (info->name)
	free( info->name );
      info->name = copy;
    }

  // option
  info->support_option = PISA_OP_FLATBED;
  opt_id = get_option_id ( SANE_NAME_SCAN_SOURCE );
  opt_desc = ::sane_get_option_descriptor ( m_hdevice, opt_id );
  if ( opt_desc->type != SANE_TYPE_STRING ||
       opt_desc->constraint_type != SANE_CONSTRAINT_STRING_LIST )
    throw pisa_error ( PISA_ERR_CONNECT );
  for ( i = 0; opt_desc->constraint.string_list [ i ]; i++ )
    {
      if ( opt_desc->constraint.string_list [ i ] != NULL &&
	   0 == ::strcmp ( opt_desc->constraint.string_list [ i ], tpu ) )
	{
	  info->support_option = PISA_OP_TPU;
	  break;
	}
      if ( opt_desc->constraint.string_list [ i ] != NULL &&
	   0 == ::strcmp ( opt_desc->constraint.string_list [ i ], adf )
	   && 0 != strncmp( name, "net:", strlen( "net:" ) ))
	{			// FIXME: kludged to disable network
				// based ADF scanning
	  info->support_option = PISA_OP_ADF;
	  break;
	}
    }

  // zoom
  info->support_zoom = is_activate ( "zoom" );

  // focus
  info->support_focus = is_activate ( "focus-position" );

  // do we have a start button?
  info->support_start_button = is_activate ("monitor-button");

  // resolution
  info->max_resolution = get_max_resolution ( );

  // dumb scanner
  info->dumb = ( info->support_zoom ) ? 0 : 1;

  info->max_descreen_resolution = get_max_resolution ((info->dumb)
						      ? 600 : 800);
}

/*------------------------------------------------------------*/
void sane_scan::query_device ( char * device_name )
{
  const SANE_Device	** ppdevice_list;
  SANE_Status		status;
  int			i;

  status = ::sane_get_devices ( & ppdevice_list, SANE_TRUE );

  if ( status != SANE_STATUS_GOOD )
    throw pisa_error ( status );

  if ( * ppdevice_list == 0 ||
       ppdevice_list [ 0 ]->vendor == 0 )
    throw pisa_error ( PISA_ERR_CONNECT );

  for ( i = 0; ppdevice_list [ i ]; i++ )
    {
      if ( 0 == ::strcasecmp ( ppdevice_list [ i ]->vendor, "epson" ) )
	{
	  ::strcpy ( device_name, ppdevice_list [ i ]->name );
	  return;
	}
    }
  
  throw pisa_error ( PISA_ERR_CONNECT );
}

/*------------------------------------------------------------*/
int sane_scan::is_activate ( char * option_name )
{
  const SANE_Option_Descriptor	* opt_desc;
  SANE_Int			num_dev_options;
  SANE_Status			status;
  int				i;
  int				ret;
  
  ret = 0;

  status = ::sane_control_option ( m_hdevice, 0,
				   SANE_ACTION_GET_VALUE,
				   & num_dev_options,
				   0 );
  
  if ( status != SANE_STATUS_GOOD )
    throw pisa_error ( status );
  
  for ( i = 0; i < num_dev_options; i++ )
    {
      opt_desc = ::sane_get_option_descriptor ( m_hdevice, i );
      
      if ( opt_desc->name!=NULL && ::strcmp ( opt_desc->name, option_name ) == 0 )
	{
	  if ( SANE_OPTION_IS_ACTIVE ( opt_desc->cap ) )
	    {
	      ret = 1;
	      break;
	    }
	  else
	    break;
	}
    }

  return ret;
}

/*------------------------------------------------------------*/
int sane_scan::get_option_id ( char * option_name )
{
  const SANE_Option_Descriptor	* opt_desc;
  SANE_Int			num_dev_options;
  SANE_Status			status;
  int				i;

  status = ::sane_control_option ( m_hdevice, 0,
				   SANE_ACTION_GET_VALUE,
				   & num_dev_options,
				   0 );
  
  if ( status != SANE_STATUS_GOOD )
    throw pisa_error ( status );

  for ( i = 0; i < num_dev_options; i++ )
    {
      opt_desc = ::sane_get_option_descriptor ( m_hdevice, i );
      
      if ( opt_desc->name != NULL && ::strcmp ( opt_desc->name, option_name ) == 0 )
	return i;	// find
    }
  
  throw pisa_error ( PISA_ERR_UNSUPPORT );
}

/*------------------------------------------------------------*/
void sane_scan::set_value ( char * option_name, void * value )
{
  SANE_Status	status;
  int		option_id;

  option_id = get_option_id ( option_name );

  status = ::sane_control_option ( m_hdevice,
				   option_id,
				   SANE_ACTION_SET_VALUE,
				   value,
				   0 );
  
  if ( status != SANE_STATUS_GOOD )
    throw pisa_error ( status );
}

/*------------------------------------------------------------*/
void sane_scan::get_value ( char * option_name, void * value )
{
  SANE_Status	status;
  int		option_id;

  option_id = get_option_id ( option_name );
  
  status = ::sane_control_option ( m_hdevice, option_id,
				   SANE_ACTION_GET_VALUE,
				   value,
				   0 );
  
  if ( status != SANE_STATUS_GOOD )
    throw pisa_error ( status );
}

/*------------------------------------------------------------*/
void sane_scan::set_focus ( long position )
{
  char	focus [ 32 ];

  if ( 0 == is_activate ( "focus-position" ) )
    return;

  if ( position == 25 )
    ::strcpy ( focus, "Focus 2.5mm above glass" );
  else
    ::strcpy ( focus, "Focus on glass" );

  set_value ( "focus-position", ( void * ) focus );
}

/*------------------------------------------------------------*/
void sane_scan::set_speed ( long speed )
{
  SANE_Bool	value;

  if ( 0 == is_activate ( "speed" ) )
    return;

  if ( speed == 1 )
    value = SANE_TRUE;
  else
    value = SANE_FALSE;

  set_value ( "speed", ( void * ) & value );
}

/*------------------------------------------------------------*/
bool sane_scan::is_button_pressed (void)
{
  SANE_Bool value = false;

  get_value ("monitor-button", (void *) &value);

  return value;
}

bool
sane_scan::area_is_too_large (const scan_parameter *param)
{
  long offset[2];
  long area[2];

  for (int i = 0; i < 2; ++i)
    {
      offset[i] = ::inch2pixel (param->offset[i], param->resolution[i],
				param->zoom[i]);
      area[i]   = ::inch2pixel (0 == i, // is_width
				param->area[i],	param->resolution[i],
				param->zoom[i]);
    }

  return (   65000 < offset[0] || 65000 < offset[1]
	  || 10200 < area[0]   || 15000 < area[1]
	  || 65000 < offset[0] + area[0]
	  || 65000 < offset[1] + area[1]);
}

/*------------------------------------------------------------*/
void sane_scan::set_color_mode ( char pixeltype, char bitdepth )
{
  char	color_mode [ 32 ];

  bitdepth = bitdepth;

  switch ( pixeltype )
    {
    case PISA_PT_RGB:
      ::strcpy ( color_mode, "Color" );
      break;

    case PISA_PT_GRAY:
      ::strcpy ( color_mode, "Gray" );
      break;

    case PISA_PT_BW:
      ::strcpy ( color_mode, "Binary" );
      break;
    }

  set_value ( SANE_NAME_SCAN_MODE, ( void * ) color_mode );
}

/*------------------------------------------------------------*/
void sane_scan::set_gamma_table ( const unsigned char * gamma_table )
{
  SANE_Word	table [ 256 ];
  int		i;
  char		user_defined [ 32 ];

  ::strcpy ( user_defined, "User defined (Gamma=1.8)" );
 
  // gamma correction
  set_value ( "gamma-correction", ( void * ) user_defined );

  // red
  for ( i = 0; i < 256; i++ )
    table [ i ] = gamma_table [ 256 * 0 + i ];

  set_value ( SANE_NAME_GAMMA_VECTOR_R, ( void * ) table );

  // green
  for ( i = 0; i < 256; i++ )
    table [ i ] = gamma_table [ 256 * 1 + i ];

  set_value ( SANE_NAME_GAMMA_VECTOR_G, ( void * ) table );

  // blue
  for ( i = 0; i < 256; i++ )
    table [ i ] = gamma_table [ 256 * 2 + i ];

  set_value ( SANE_NAME_GAMMA_VECTOR_B, ( void * ) table  );
}

/*------------------------------------------------------------*/
void sane_scan::set_threshold ( long threshold )
{
  set_value ( SANE_NAME_THRESHOLD, ( void * ) & threshold );
}  

/*------------------------------------------------------------*/
void sane_scan::set_color_profile ( const double * coef )
{
  SANE_Word	value;
  char		option_name [ 16 ];
  int		i;
  char		user_defined [ 32 ];

  ::strcpy ( user_defined, "User defined" );
  
  // color correction
  set_value ( "color-correction", ( void * ) user_defined );
 
  for ( i = 0; i < 9; i++ )
    {
      ::sprintf ( option_name, "cct-%d", i + 1 );

      value = SANE_FIX ( coef [ i ] );

      set_value ( option_name, ( void * ) & value );
    }
}

/*------------------------------------------------------------*/
void sane_scan::set_scan_resolution ( int resolution_x, int resolution_y )
{
  set_value ( SANE_NAME_SCAN_RESOLUTION, ( void * ) & resolution_x );

  resolution_y = resolution_y;
}

/*------------------------------------------------------------*/
void sane_scan::set_scan_zoom ( int zoom_x, int zoom_y )
{
  set_value ( "zoom", ( void * ) & zoom_x );

  zoom_y = zoom_y;
}

/*------------------------------------------------------------*/
void sane_scan::set_scan_area ( double offset_x, double offset_y,
				double width, double height )
{
  SANE_Word	value;
  double	tmp;

  // top-left x
  value = SANE_FIX ( offset_x * MM_PER_INCH );
  set_value ( SANE_NAME_SCAN_TL_X, ( void * ) & value );
  
  // top-left y
  value = SANE_FIX ( offset_y * MM_PER_INCH );
  set_value ( SANE_NAME_SCAN_TL_Y, ( void * ) & value );

  // buttom-right x
  tmp = offset_x + width;
  value = SANE_FIX ( tmp * MM_PER_INCH );
  set_value ( SANE_NAME_SCAN_BR_X, ( void * ) & value );

  // buttom-right y
  tmp = offset_y + height;
  value = SANE_FIX ( tmp * MM_PER_INCH );
  set_value ( SANE_NAME_SCAN_BR_Y, ( void * ) & value );
}

/*------------------------------------------------------------*/
void sane_scan::update_settings ( const scan_parameter * param )
{
  // option
  set_option ( static_cast <pisa_option_type> ( param->option ) );

  // film type
  set_film_type ( static_cast <pisa_film_type> ( param->film ) );

  // focus
  set_focus ( param->focus );

  // speed
  set_speed ( param->speed );

  // color mode
  set_color_mode ( param->pixeltype, param->bitdepth );

  // gamma table
  set_gamma_table ( param->gamma.gamma_r );

  // threshold
  if ( param->pixeltype == PISA_PT_BW )
    set_threshold ( param->threshold );

  // color correction
  if ( param->pixeltype == PISA_PT_RGB )
    set_color_profile ( param->coef );

  // resolution
  set_scan_resolution ( param->resolution [ 0 ], param->resolution [ 1 ] );

  // zoom
  set_scan_zoom ( param->zoom [ 0 ], param->zoom [ 1 ] );

  // area
  set_scan_area ( param->offset [ 0 ], param->offset [ 1 ],
		  param->area [ 0 ], param->area [ 1 ] );
}
