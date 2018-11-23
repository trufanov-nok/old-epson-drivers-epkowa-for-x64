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

/*------------------------------------------------------------*/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <new>
using std::bad_alloc;

#include "filter.hh"

/*------------------------------------------------------------*/
#include "pisa_scan_manager.h"
#include "pisa_error.h"
#include "pisa_scan_tool.h"
#include "pisa_change_unit.h"

/*------------------------------------------------------------*/
void scan_manager::open_device ( char * name )
{
  sane_scan::init ( );

  m_resize_cls	= 0;
  m_moire_cls	= 0;
  m_sharp_cls	= 0;
  m_resize_img_info = 0;
  m_moire_img_info  = 0;
  m_sharp_img_info  = 0;

  m_scanner_info.name = 0;
  sane_scan::open_device ( & m_scanner_info, name );
}

void
scan_manager::init_scan (scan_parameter *param, int *width, int *height,
			 bool reset_params)
{
  if (reset_params)
    {
      for (int i = 0; i < 2; ++i)
	{
	  adjust_scan_param (&param->resolution[i], &param->zoom[i]);
	}

      init_img_process_info (param);
      sane_scan::set_scan_parameter (param);
    }

  if (area_is_too_large (param))
    {
      if (getenv ("ISCAN_DEBUG"))
	{
	  fprintf (stderr, "Backend will try to scan with these settings\n");
	  fprintf (stderr, "Resolution (main,sub): %ld,%ld\n",
		   param->resolution[0], param->resolution[1]);
	  fprintf (stderr, "Zoom       (main,sub): %ld,%ld\n",
		   param->zoom[0], param->zoom[1]);
	  fprintf (stderr, "Offset     (main,sub): %ld,%ld\n",
		   inch2pixel (param->offset[0], param->resolution[0],
			       param->zoom[0]),
		   inch2pixel (param->offset[1], param->resolution[1],
			       param->zoom[1]));
	  fprintf (stderr, "Area       (main,sub): %ld,%ld\n",
		   inch2pixel (true,
			       param->area[0], param->resolution[0],
			       param->zoom[0]),
		   inch2pixel (false,
			       param->area[1], param->resolution[1],
			       param->zoom[1]));
	}

      throw pisa_error (PISA_ERR_AREALARGE);
    }

  sane_scan::start_scan ( width, height );

  modify_img_info ( width, height );

  create_img_cls ( );
}

/*------------------------------------------------------------*/
void scan_manager::acquire_image ( unsigned char * img,
				   int row_bytes,
				   int height,
				   int cancel )
{
  if ( m_resize || m_moire || m_sharp )
    {
      int in_rowbytes = 0, in_line;
      unsigned char * in_img = 0;

      if ( m_sharp )
	in_rowbytes = m_sharp_info.in_rowbytes;
      if ( m_moire )
	in_rowbytes = m_moire_info.in_rowbytes;
      if ( m_resize )
	in_rowbytes = m_resize_info.in_rowbytes;

      if ( cancel || img == 0 )
	{
	  sane_scan::acquire_image ( 0, 0, 1, 1 );
	}
      else
	{
	  in_line = get_send_in_line ( 1 );

	  if ( 0 < in_line )
	    {
	      int buf_size = in_line * in_rowbytes;
	      
	      in_img = new unsigned char [ buf_size + in_rowbytes ];
	      
	      if ( in_img == 0 )
		{
		  sane_scan::acquire_image ( 0, 0, 1, 1 );
		  
		  throw pisa_error ( PISA_ERR_OUTOFMEMORY );
		}
	      
	      sane_scan::acquire_image ( in_img,
					 in_rowbytes,
					 in_line,
					 cancel );
	    }
	}

      ::memset ( img, 128, row_bytes );
      if ( cancel == 0 && PISA_ERR_SUCCESS != image_process ( in_img, img ) )
	{
	  delete [ ] in_img;
	  sane_scan::acquire_image ( 0, 0, 1, 1 );
	  throw pisa_error ( PISA_ERR_OUTOFMEMORY );
	}

      if ( in_img )
	delete [ ] in_img;
    }
  else
    sane_scan::acquire_image ( img, row_bytes, height, cancel );
}

/*------------------------------------------------------------*/
void
scan_manager::finish_acquire ( void )
{
  sane_cancel (m_hdevice);
  release_memory ();
}

/*------------------------------------------------------------*/
int scan_manager::init_img_process_info ( scan_parameter * param )
{
  // set flag
  if ( m_scanner_info.support_zoom == 0 )
    m_resize = 1;
  else
    m_resize = 0;

  if ( param->de_screening )	// color or B/W document
    {
      m_moire = 1;
      m_resize = 0;
    }
  else
    m_moire = 0;

  m_sharp = param->usm;

  if ( m_resize )
    init_zoom ( & m_resize_info, param );

  if ( m_moire )
    init_moire ( & m_moire_info, param );

  if ( m_sharp )
    init_sharp ( & m_sharp_info, param );

  return PISA_ERR_PARAMETER;
}

/*------------------------------------------------------------*/
int scan_manager::init_zoom ( resize_img_info * info, scan_parameter * param )
{
  long act_res[2];

  info->resolution = param->resolution[0];

  for (int i = 0; i < 2; ++i)
    {
      act_res[i] = param->resolution[i] * param->zoom[i] / 100;
    }

  info->out_width	= ::inch2width ( param->area [ 0 ],
					 act_res [ 0 ] );
  info->out_height	= ::inch2height ( param->area [ 1 ],
					 act_res [ 1 ] );
  info->out_rowbytes	= ::calc_rowbytes ( info->out_width,
					    static_cast <pisa_pixel_type>
					    ( param->pixeltype ) );

  for (int i = 0; i < 2; ++i)
    {
      param->resolution[i] = get_valid_resolution (act_res[i]);
      if (!param->resolution[i])
	{
	  param->resolution[i] = m_scanner_info.max_resolution;
	}
      param->zoom[i] = 100;
    }

  if (   act_res[0] == param->resolution[0]
      && act_res[1] == param->resolution[1])
    {
      m_resize = 0;
      return PISA_ERR_SUCCESS;
    }

  info->in_width	= ::inch2width ( param->area [ 0 ],
					 param->resolution [ 0 ] );
  info->in_height	= ::inch2height ( param->area [ 1 ],
					 param->resolution [ 1 ] );
  info->in_rowbytes	= ::calc_rowbytes ( info->in_width,
					    static_cast <pisa_pixel_type>
					    ( param->pixeltype ) );
  
  info->bits_per_pixel	= ::calc_bitperpix ( static_cast <pisa_pixel_type>
					    ( param->pixeltype ),
					     static_cast <pisa_bitdepth_type> 
					     ( param->bitdepth ) );

  if ( param->pixeltype == PISA_PT_BW )
    info->resize_flag	= PISA_RS_NN;
  else
    info->resize_flag	= PISA_RS_BC;
  
  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int scan_manager::init_moire( moire_img_info * info,
			      scan_parameter * param )
{
  info->resolution = param->resolution [ 0 ] * param->zoom [ 0 ] / 100;

  param->resolution [ 0 ] =
  param->resolution [ 1 ] =
    iscan::moire::get_res_quote (info->resolution, m_scanner_info.dumb);

  if (!m_scanner_info.support_zoom)
    {
      long resolution = get_valid_resolution (param->resolution[0]);

      if (resolution)
	{
	  param->resolution[0] = param->resolution[1] = resolution;
	}
    }

  info->in_resolution = param->resolution [ 0 ];

  param->zoom [ 0 ] = 100;
  param->zoom [ 1 ] = 100;

  info->in_width	= ::inch2width ( param->area [ 0 ],
					 param->resolution [ 0 ] );
  info->in_height	= ::inch2height ( param->area [ 1 ],
					 param->resolution [ 1 ] );
  info->in_rowbytes	= ::calc_rowbytes ( info->in_width,
					    static_cast <pisa_pixel_type>
					    ( param->pixeltype ) );

  info->out_width	= ::inch2width ( param->area [ 0 ],
					 info->resolution );
  info->out_height	= ::inch2height ( param->area [ 1 ],
					 info->resolution );
  info->out_rowbytes	= ::calc_rowbytes ( info->out_width,
					    static_cast <pisa_pixel_type>
					    (param->pixeltype ) );

  info->bits_per_pixel	= ::calc_bitperpix ( static_cast <pisa_pixel_type>
					     ( param->pixeltype ),
					     static_cast <pisa_bitdepth_type>
					     ( param->bitdepth ) );

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int scan_manager::init_sharp ( sharp_img_info * info,
			       scan_parameter * param )
{
  long resolution;

  if ( m_resize )
    {
      info->in_width	= m_resize_info.out_width;
      info->in_height	= m_resize_info.out_height;

      resolution	= m_resize_info.resolution;
    }
  else if ( m_moire )
    {
      info->in_width	= m_moire_info.out_width;
      info->in_height	= m_moire_info.out_height;

      resolution	= m_moire_info.resolution;
    }
  else
    {
      info->in_width	= ::inch2width ( param->area [ 0 ],
					 param->resolution [ 0 ],
					 param->zoom [ 0 ] );
      info->in_height	= ::inch2height ( param->area [ 1 ],
					 param->resolution [ 1 ],
					 param->zoom [ 1 ] );

      resolution	= param->resolution [ 0 ];
    }

  info->in_rowbytes	= ::calc_rowbytes ( info->in_width,
					    static_cast <pisa_pixel_type>
					    ( param->pixeltype ) );
  
  info->bits_per_pixel	= ::calc_bitperpix ( static_cast <pisa_pixel_type>
					    ( param->pixeltype ),
					     static_cast <pisa_bitdepth_type> 
					     ( param->bitdepth ) );

  info->out_width	= info->in_width;
  info->out_height	= info->in_height;
  info->out_rowbytes	= info->in_rowbytes;

  m_sharp_cls->set_parms (resolution,
			  param->film == PISA_FT_NEGA
			  || param->film == PISA_FT_POSI,
			  m_scanner_info.dumb,
			  & info->strength,
			  & info->radius,
			  & info->clipping);

  info->sharp_flag	= PISA_SH_UMASK;
  
  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int scan_manager::modify_img_info ( int * width, int * height )
{
  // resize
  if ( m_resize )
    {
      if ( m_resize_info.in_width == m_resize_info.out_width )
	{
	  m_resize_info.out_width = * width;
	  m_resize_info.out_rowbytes = ( m_resize_info.out_width *
					 m_resize_info.bits_per_pixel +
					 7 ) / 8;
	}

      m_resize_info.in_width	= * width;
      m_resize_info.in_rowbytes	= ( m_resize_info.in_width *
				    m_resize_info.bits_per_pixel +
				    7 ) / 8;

      if ( m_resize_info.in_height == m_resize_info.out_height )
	{
	  m_resize_info.out_height = * height;
	}

      m_resize_info.in_height	= * height;
    }
 
  if ( m_sharp )
    {
      m_sharp_info.in_width	= * width;
      m_sharp_info.in_height	= * height;
      m_sharp_info.in_rowbytes	= ( m_sharp_info.in_width *
				    m_sharp_info.bits_per_pixel +
				    7 ) / 8;

      if ( m_resize )
	{
	  m_sharp_info.in_width		= m_resize_info.out_width;
	  m_sharp_info.in_height	= m_resize_info.out_height;
	  m_sharp_info.in_rowbytes	= m_resize_info.out_rowbytes;
	}

      if ( m_moire )
	{
	  m_sharp_info.in_width		= m_moire_info.out_width;
	  m_sharp_info.in_height	= m_moire_info.out_height;
	  m_sharp_info.in_rowbytes	= m_moire_info.out_rowbytes;
	}

      m_sharp_info.out_width	= m_sharp_info.in_width;
      m_sharp_info.out_height	= m_sharp_info.in_height;
      m_sharp_info.out_rowbytes	= m_sharp_info.in_rowbytes;
    }

  // update width and height
  if ( m_resize )
    {
      * width	= m_resize_info.out_width;
      * height	= m_resize_info.out_height;
    }

  if ( m_moire )
    {
      * width	= m_moire_info.out_width;
      * height	= m_moire_info.out_height;
    }

  if ( m_sharp )
    {
      * width	= m_sharp_info.out_width;
      * height	= m_sharp_info.out_height;
    }


  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int scan_manager::create_img_cls ( void )
{
  release_memory ();

  if (m_sharp)
    {
      m_sharp_img_info = new IMAGE_INFO [ 2 ];

      set_img_info ( & m_sharp_img_info [ _IN ],
		     & m_sharp_img_info [ _OUT ],
		     m_sharp_info );

      m_sharp_cls = new iscan::focus (m_sharp_info);
    }

  if ( m_moire )
    {
      m_moire_img_info = new IMAGE_INFO [ 2 ];

      set_img_info ( & m_moire_img_info [ _IN ],
		     & m_moire_img_info [ _OUT ],
		     m_moire_info );

      m_moire_cls = new iscan::moire (m_moire_info, m_scanner_info.dumb);
    }
  
  if ( m_resize )
    {
      m_resize_img_info = new IMAGE_INFO [ 2 ];

      set_img_info ( & m_resize_img_info [ _IN ],
		     & m_resize_img_info [ _OUT ],
		     m_resize_info );

      m_resize_cls = new iscan::scale (m_resize_info);
    }

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
int scan_manager::release_memory ( void )
{
  if ( m_resize_cls )
    delete m_resize_cls;
  m_resize_cls = 0;

  if ( m_moire_cls )
    delete m_moire_cls;
  m_moire_cls = 0;

  if ( m_sharp_cls )
    delete m_sharp_cls;
  m_sharp_cls = 0;

  if ( m_resize_img_info )
    delete m_resize_img_info;
  m_resize_img_info = 0;

  if ( m_moire_img_info )
    delete m_moire_img_info;
  m_moire_img_info = 0;

  if ( m_sharp_img_info )
    delete m_sharp_img_info;
  m_sharp_img_info = 0;

  return PISA_ERR_SUCCESS;
}

/*------------------------------------------------------------*/
void scan_manager::set_img_info ( LPIMAGE_INFO in_img_info,
				  LPIMAGE_INFO out_img_info,
				  const img_size & size )
{
  in_img_info->pImg_Buf		= 0;
  in_img_info->Img_Width	= size.in_width;
  in_img_info->Img_Height	= size.in_height;
  in_img_info->Img_RowBytes	= size.in_rowbytes;
  in_img_info->BitsPerPixel	= size.bits_per_pixel;

  out_img_info->pImg_Buf	= 0;
  out_img_info->Img_Width	= size.out_width;
  out_img_info->Img_Height	= size.out_height;
  out_img_info->Img_RowBytes	= size.out_rowbytes;
  out_img_info->BitsPerPixel	= size.bits_per_pixel;
}

/*------------------------------------------------------------*/
int scan_manager::get_send_in_line ( int out_line )
{
  size_t quote = out_line;

  if (m_sharp)
    {
      m_sharp_img_info [ _OUT ].Img_Height = quote;
      quote = m_sharp_cls->get_line_quote (quote);
      m_sharp_img_info [ _IN ].Img_Height  = quote;
    }
  if (m_moire)
    {
      m_moire_img_info [ _OUT ].Img_Height = quote;
      quote = m_moire_cls->get_line_quote (quote);
      m_moire_img_info [ _IN ].Img_Height  = quote;
    }
  if (m_resize)
    {
      m_resize_img_info [ _OUT ].Img_Height = quote;
      quote = m_resize_cls->get_line_quote (quote);
      m_resize_img_info [ _IN ].Img_Height  = quote;
    }

  return quote;
}

/*------------------------------------------------------------*/
int scan_manager::image_process ( unsigned char * in_img,
				  unsigned char * out_img )
{
  unsigned char *inbuf, *outbuf;
  size_t         in_sz,  out_sz;

  unsigned char *tmpbuf = NULL;
  size_t         tmp_sz = 0;
  bool           zapbuf = false;

  try
    {
      if (m_resize)
	{
	  in_sz = m_resize_img_info [ _IN ].Img_Height * m_resize_img_info [ _IN].Img_RowBytes;
	  inbuf = in_img;

	  if (m_sharp)
	    {
	      out_sz = tmp_sz = (m_resize_img_info [ _OUT ].Img_Height
				 * m_resize_img_info [ _OUT ].Img_RowBytes);
	      outbuf = tmpbuf = new unsigned char [out_sz];
	      zapbuf = true;
	    }
	  else
	    {
	      out_sz = m_resize_img_info [ _OUT ].Img_Height * m_resize_img_info [ _OUT ].Img_RowBytes;
	      outbuf = out_img;
	    }
	  m_resize_cls->exec (inbuf, in_sz, outbuf, out_sz);
	}

      if (m_moire)
	{
	  in_sz = m_moire_img_info [ _IN ].Img_Height * m_moire_img_info [ _IN ].Img_RowBytes;
	  inbuf = in_img;

	  if (m_sharp)
	    {
	      out_sz = tmp_sz = (m_moire_img_info [ _OUT ].Img_Height
				 * m_moire_img_info [ _OUT ].Img_RowBytes);
	      outbuf = tmpbuf = new unsigned char [out_sz];
	      zapbuf = true;
	    }
	  else
	    {
	      out_sz = m_moire_img_info [ _OUT ].Img_Height * m_moire_img_info [ _OUT ].Img_RowBytes;
	      outbuf = out_img;
	    }
	  m_moire_cls->exec (inbuf, in_sz, outbuf, out_sz);
	}

      if (m_sharp)
	{
	  out_sz = m_sharp_img_info [ _OUT ].Img_Height * m_sharp_img_info [ _OUT ].Img_RowBytes;
	  outbuf = out_img;

	  if (m_resize || m_moire)
	    {
	      in_sz = tmp_sz;
	      inbuf = tmpbuf;
	    }
	  else
	    {
	      in_sz = m_sharp_img_info [ _IN ].Img_Height * m_sharp_img_info [ _IN ].Img_RowBytes;
	      inbuf = in_img;
	    }
	  m_sharp_cls->exec (inbuf, in_sz, outbuf, out_sz);
	}

      if (zapbuf)
	{
	  delete [] tmpbuf;
	}
    }
  catch (bad_alloc& oops)
    {
      if (zapbuf)
	{
	  delete [] tmpbuf;
	}
      return PISA_ERR_OUTOFMEMORY;
    }

  return PISA_ERR_SUCCESS;
}

void
scan_manager::adjust_scan_param (long *resolution, long *scale)
{
  int min_res   = 50;
  int max_res   = m_scanner_info.max_resolution;
  int adj_res   = *resolution;

  int min_scale =  50;
  int max_scale = 200;
  int adj_scale = 100;		// assume no scaling is needed

  if (adj_res < min_res)
    {
      adj_scale = adj_res * 100 / min_res;

      if (adj_scale < min_scale)
	{
	  adj_scale = min_scale;
	}
      adj_res = min_res;
    }

  if (max_res < adj_res)
    {
      adj_scale = adj_res * 100 / max_res;

      if (adj_scale > max_scale)
	{
	  adj_scale = max_scale;
	}
      adj_res = max_res;
    }

  *resolution = adj_res;
  *scale      = adj_scale;
}


SANE_Int
scan_manager::get_valid_resolution (int min_res)
{
  SANE_Int def_res_tbl[] = { 75, 150, 300, 600, 1200 };
  SANE_Int def_res_cnt   = 4;

  const SANE_Int *res_tbl;
        SANE_Int  res_cnt;

  {				// initialize resolution table
    int opt_id = get_option_id (SANE_NAME_SCAN_RESOLUTION);
    const SANE_Option_Descriptor *opt_desc
      = sane_get_option_descriptor (m_hdevice, opt_id);

    if (opt_desc
	&& SANE_TYPE_INT             == opt_desc->type
	&& SANE_CONSTRAINT_WORD_LIST == opt_desc->constraint_type)
      {
	res_cnt =   opt_desc->constraint.word_list[0];
	res_tbl = &(opt_desc->constraint.word_list[1]);
      }
    else
      {
	res_cnt = def_res_cnt;
	res_tbl = def_res_tbl;

	if (1200 == m_scanner_info.max_resolution)
	  ++res_cnt;
      }
  }

  {				// find appropriate resolution
    int i = 0;
    while (i < res_cnt && res_tbl[i] < min_res)
      {
	++i;
      }
    return (res_cnt == i ? 0 : res_tbl[i]);
  }
}
