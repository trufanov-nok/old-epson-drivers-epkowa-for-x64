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

/*--------------------------------------------------------------*/
#include "pisa_marquee.h"

/*--------------------------------------------------------------*/
marquee & marquee::operator= ( const marquee & x )
{
  if ( & x != this )
    {
      int i, j;

      active		= x.active;
      offset		= x.offset;
      area		= x.area;
      scale		= x.scale;
      gamma		= x.gamma;
      highlight		= x.highlight;
      shadow		= x.shadow;
      threshold		= x.threshold;
      
      for ( i = 0; i < 4; i++ )
	for ( j = 0; j < 256; j++ )
	  gamma_table [ i ] [ j ] = x.gamma_table [ i ] [ j ];

      graybalance	= x.graybalance;
      saturation	= x.saturation;

      focus		= x.focus;

      for ( i = 0; i < 3; i++ )
	{
	  film_gamma [ i ]	= x.film_gamma [ i ];
	  film_yp [ i ]		= x.film_yp [ i ];
	  grayl [ i ]		= x.grayl [ i ];
	}

      for ( i = 0; i < 256; i++ )
	{
	  lut.gamma_r [ i ] = x.lut.gamma_r [ i ];
	  lut.gamma_g [ i ] = x.lut.gamma_g [ i ];
	  lut.gamma_b [ i ] = x.lut.gamma_b [ i ];
	}
    }

  return ( * this );
}

/*--------------------------------------------------------------*/
