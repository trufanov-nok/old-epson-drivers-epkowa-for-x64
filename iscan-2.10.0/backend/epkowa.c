/* epkowa.c - SANE backend for EPSON flatbed scanners
   	      (Image Scan! version)

   Based on the SANE Epson backend (originally from sane-1.0.3)
   - updated to sane-backends-1.0.6
   - renamed from epson to epkowa to avoid confusion
   - updated to sane-backends-1.0.12
   - updated to sane-backends-1.0.15
	      
   Based on Kazuhiro Sasayama previous
   Work on epson.[ch] file from the SANE package.

   Original code taken from sane-0.71
   Copyright (C) 1997 Hypercore Software Design, Ltd.

   modifications
   Copyright (C) 1998-1999 Christian Bucher <bucher@vernetzt.at>
   Copyright (C) 1998-1999 Kling & Hautzinger GmbH
   Copyright (C) 1999 Norihiko Sawa <sawa@yb3.so-net.ne.jp>
   Copyright (C) 2000 Mike Porter <mike@udel.edu> (mjp)
   Copyright (C) 1999-2004 Karl Heinz Kremer <khk@khk.net>
   Copyright (C) 2001-2007 SEIKO EPSON Corporation

*/

#define	SANE_EPSON_VERSION	\
	"EPKOWA SANE Backend " PACKAGE_VERSION " - 2007-03-07"
#define SANE_EPSON_BUILD	208

/*
   This file is part of the EPKOWA SANE backend.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.

   As a special exception, the authors of SANE give permission for
   additional uses of the libraries contained in this release of SANE.

   The exception is that, if you link a SANE library with other files
   to produce an executable, this does not by itself cause the
   resulting executable to be covered by the GNU General Public
   License.  Your use of that executable is in no way restricted on
   account of linking the SANE library code into it.

   This exception does not, however, invalidate any other reasons why
   the executable file might be covered by the GNU General Public
   License.

   If you submit changes to SANE to the maintainers to be included in
   a subsequent release, you agree by submitting the changes that
   those changes may be distributed with this exception intact.

   If you write modifications of your own for SANE, it is your choice
   whether to permit this exception to apply to your modifications.
   If you do not wish that, delete this exception notice.  */

/*
   2004-10-16   Added USB ID for Expression 10000XL
   2004-05-08   Disable feed() for Perfection1640
   2004-02-08   Reformat all source code with "indent -bli0"
   2004-02-01   Added D7 function level as copy of D1 for CX-6400
        	Added IDs for CX-6400 and Perfection 4870
   2003-10-27   Replaced DBG(0, ... with DBG(1, ...
   2003-09-12   Increment only once in loop to find USB scanners
		Fix rounding problem when determining number of lines to scan
   2003-08-21   Removed '//' comments - again ...
		Added EPSON copyright message
   2003-08-15	Added support for GT-30000, with support for the ADF in simplex mode
		Borrowed some code from the 'epkowa' backend
		Use sanei_scsi_cmd2() to send commands. This makes this backend 
		useable for SBP-2 under FreeBSD
   2003-05-11   Initialize OPT_LIMIT_RESOLUTION before first call to filter_resolution_list()
		Fix memory problem in get_identity_information(). Both problems were
		reported to the Debian bug database.
   2003-03-26	Fixed two warnings reported by der Mouse
   2003-02-16	Code cleanup, use more descriptive variable names.
   2003-02-15   Move sanei_usb_init() to sane_init(). Thanks to Ron Cemer
   				for providing the patch.
   2003-02-15	Fix problem with "usb <vendor> <product> syntax in config file
   2002-12-28	Added advanced option to display only short resolution list for 
   		displays that can not show the complete list.
   2002-11-23	Fixed problem with dropout color.
   2002-11-03	Full libusb support.
   2002-10-05	Fixed problem with incorrect response to sane_get_parameters()
   		in certain situations.
   2002-09-01	USB scanners are now using libsane-usb funtions
   2002-08-17	Fixed typo in variable name. 
		Fixed IEEE-1394 problem with Perfection-2450.
		Fixed problem with older B3 level SCSI scanners that do
		not support the extended status request.
   2002-04-22	Declare close_scanner() and open_scanner() before they
		are used.
   2002-04-13	Check if scanner needs to be opened for the reset call.
		(Thanks to Thomas Wenrich for pointing this out)
		Added product IDs for Perfection 1650 and 2450
   2002-01-18	Recognize GT-xxxx type scanners also when using the SCSI 
   		or IEEE-1394 interface
   2002-01-06   Disable TEST_IOCTL again, which was enabled by accident. Also
		protect the ioctl portion with an #ifdef __linux__
   2002-01-05   Version 0.2.17
		Check for and set s->hw->fd to -1 when device is closed.		
		Removed black gamma table - only use RGB even for grayscale
   2002-01-01   Do not call access() for OS/2 systems
   2001-11-13   Version 0.2.16
		Do not call access() for parallel port scanners.
   2001-11-11   Version 0.2.15
		Fixed "wait-for-button" functionality, accidentially merged back wrong
		version after code freeze.
		Corrected "need-strange-reorder" recognition.
		Added IOCTL support to header file.
   2001-11-10   Version 0.2.14
		Added "wait-for-button" functionality
   2001-10-30   I18N patches (Stefan Roellin)
   2001-10-28   Fixed bug with 1650 recognition
   2001-06-09   Version 0.2.09
		Changed debug level for sense handler from 0 to 2
   2001-05-25	Version 0.2.07
		Allow more than 8 bit color depth even for preview mode 
		since Xsane can handle this. Some code cleanup. 
   2001-05-24   Removed ancient code that was used to determine the resolution
		back when the backend still had a slider for the resolution
		selection.
   2001-05-22   Version 0.2.06
		Added sense_handler to support the GT-8000 scanner. Thanks to Matthias Trute
		for figuring out the details.
		Also added experimental code to use USB scanner probing. Need kernel patch
		for this.
   2001-05-19   Version 0.2.05
		fixed the year in the recent change log entries - I now that it's
		2001...
		Finally fixed the TPU problem with B4 level scanners
   2001-05-13	Version 0.2.04
		Removed check for '\n' before end of line
		Free memory malloced in sane_get_devices() in sane_exit() again
   2001-04-22	Version 0.2.03
		Check first if the scanner does support the set film type
		and set focus position before the GUI elements are displayed.
		This caused problems with older (B4 level) scanners when a TPU
		was connected.
   2001-03-31   Version 0.2.02
   2001-03-17   Next attempt to get the reported number of lines correct
   		for the "color shuffling" part.
		Added more comments.
   2000-12-25	Version 0.2.01
    		Fixed problem with bilevel scanning with Perfection610: The
		line count has to be an even number with this scanner.
		Several initialization fixes regarding bit depth selection.
		This version goes back into the CVS repository, the 1.0.4 
		release is out and therefore the code freeze is over.
		Some general cleanup, added more comments.
   2000-12-09   Version 0.2.00
   		Cleaned up printing of gamma table data. 16 elements
		are now printed in one line without the [epson] in 
		between the values. Values are only printed for
		Debug levels >= 10.
   2000-12-04	We've introduced the concept of inverting images
 		when scanning from a TPU.  This is fine, but
 		the user supplied gamma tables no longer work.
 		This is because the data a frontend is going
 		to compute a gamma table for is not what the
 		scanner actually sent.	So, we have to back into
 		the proper gamma table.  I think this works.  See
 		set_gamma_table.  (mjp)
   2000-12-03   added the 12/14/16 bit support again.
   2000-12-03   Version 0.1.38
   		removed changes regarding 12/14 bit support because
   		of SANE feature freeze for 1.0.4. The D1 fix for 
		reading the values from the scanner instead of using
		hardcoded values and the fix for the off-by-one error
		in the reorder routine are still in the code base.
		Also force reload after change of scan mode.
		The full backend can be downloaded from my web site at
		http://www.freecolormanagement.com/sane
   2000-12-03   Fixed off-by-one error in color reordering function.
   2000-12-02   Read information about optical resolution and line
   		distance from scanner instead of hardcoded values.
		Add support for color depth > 8 bits per channel.
   2000-11-23   Display "Set Focus" control only for scanners that
                can actually handle the command.
   2000-11-19   Added support for the "set focus position" command,
                this is necessary for the Expression1600.
   2000-07-28   Changed #include <...> to #include "..." for the
   		sane/... include files.
   2000-07-26   Fixed problem with Perfection610: The variable
   		s->color_shuffle_line was never correctly initialized
   2000-06-28   When closing the scanner device the data that's	
		still in the scanner, waiting to be transferred
		is flushed. This fixes the problem with scanimage -T
   2000-06-13   Invert image when scanning negative with TPU,
                Show film type only when TPU is selected
   2000-06-13   Initialize optical_res to 0 (Dave Hill)
   2000-06-07   Fix in sane_close() - found by Henning Meier-Geinitz 
   2000-06-01	Threshhold should only be active when scan depth
		is 1 and halftoning is off.  (mjp)
   2000-05-28	Turned on scanner based color correction.
                Dependancies between many options are now
                being enforced.  For instance, auto area seg
                (AAS) should only be on when scan depth == 1.
                Added some routines to active and deactivate
                options.  Routines report if option changed.
                Help prevent extraneous option reloads.  Split
                sane_control_option in getvalue and setvalue.
                Further split up setvalue into several different
                routines. (mjp)                         
   2000-05-21   In sane_close use close_scanner instead of just the 
		SCSI close function.
   2000-05-20   ... finally fixed the problem with the 610
                Added resolution_list to Epson_Device structure in
		epson.h - this fixes a bug that caused problems when 
		more than one EPSON scanner was connected.
   2000-05-13   Fixed the color problem with the Perfection 610. The few
		lines with "garbage" at the beginning of the scan are not
		yet removed. 
   2000-05-06   Added support for multiple EPSON scanners. At this time
		this may not be bug free, but it's a start and it seems
		to work well with just one scanner.
   2000-04-06   Did some cleanup on the gamma correction part. The user
		defined table is now initialized to gamma=1, the gamma
		handling is also no longer depending on platform specific
		tables (handled instead by pointers to the actual tables)
   2000-03-27	Disable request for push button status
   2000-03-22   Removed free() calls to static strings to remove
		compile warnings. These were introduced to apparently
		fix an OS/2 bug. It now turned out that they are not
		necessary. The real fix was in the repository for a
		long time (2000-01-25).
   2000-03-19	Fixed problem with A4 level devices - they use the 
		line mode instead of the block mode. The routine to 
		handle this was screwed up pretty bad. Now I have 
		a solid version that handles all variations of line
		mode (automatically deals with the order the color
		lines are sent).
   2000-03-06   Fixed occasional crash after warm up when the "in warmup
		state" went away in between doing ESC G and getting the
		extended status message.  
   2000-03-02	Code cleanup, disabled ZOOM until I have time to 
		deal with all the side effects.
   2000-03-01   More D1 fixes. In the future I have to come up with
		a more elegant solution to destinguish between different
		function levels. The level > n does not work anymore with
		D1. 
		Added support for "set threshold" and "set zoom".
   2000-02-23   First stab at level D1 support, also added a test
		for valid "set halftone" command to enable OPT_HALFTONE
   2000-02-21   Check for "warming up" in after sane_start. This is
		IMHO a horrible hack, but that's the only way without
		a major redesign that will work. (KHK)
   2000-02-20   Added some cleanup on error conditions in attach()
                Use new sanei_config_read() instead of fgets() for
		compatibility with OS/2 (Yuri Dario)
   2000-02-19   Changed some "int" to "size_t" types
		Removed "Preview Resolution"
		Implemented resolution list as WORD_LIST instead of
		a RANGE (KHK)
   2000-02-11   Default scan source is always "Flatbed", regardless
		of installed options. Corrected some typos. (KHK)
   2000-02-03   Gamma curves now coupled with gamma correction menu.
		Only when "User defined" is selected are the curves
		selected. (Dave Hill)
		Renamed "Contrast" to "Gamma Correction" (KHK)
   2000-02-02   "Brown Paper Bag Release" Put the USB fix finally
		into the CVS repository.
   2000-02-01   Fixed problem with USB scanner not being recognized
		because of hte changes to attach a few days ago. (KHK)
   2000-01-29   fixed core dump with xscanimage by moving the gamma
		curves to the standard interface (no longer advanced)
   		Removed pragma pack() from source code to make it 
		easier to compile on non-gcc compilers (KHK)
   2000-01-26   fixed problem with resolution selection when using the
		resolution list in xsane (KHK)
   2000-01-25	moved the section where the device name is assigned 
		in attach. This avoids the core dump of frontend
		applications when no scanner is found (Dave Hill)
   2000-01-24	reorganization of SCSI related "helper" functions
		started support for user defined color correction -
		this is not yet available via the UI (Christian Bucher)
   2000-01-24	Removed C++ style comments '//' (KHK)
*/


#ifdef  _AIX
#include  <lalloca.h>		/* MUST come first for AIX! */
#endif

/* --------------------- SANE INTERNATIONALISATION ------------------ */

#ifndef SANE_I18N
#define SANE_I18N(text) (text)
#endif

#ifdef HAVE_CONFIG_H
#include  <config.h>
#endif

#include  <lalloca.h>

#ifdef  MEMORY_DEBUGGING_ENABLED
#include <stdlib.h>
#define malloc(x)    calloc( 1, (x) )
#define realloc(p,n) wrealloc((p),(n))
#define delete(p)    do { if (p) free(p); p = 0; } while (p)
void *
wrealloc (void *src, size_t n)
{
  void *dst = malloc (n);
  if (dst && src)
  {
    char *pd = dst;
    char *ps = src;
    while (*ps && --n)
      *pd++ = *ps++;
  }
  delete (src);
  return dst;
}
#else
#define delete(p)    free(p)
#endif

#include  <limits.h>
#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <ctype.h>
#include  <fcntl.h>
#include  <unistd.h>
#include  <errno.h>
#include  <math.h>
#include  <time.h>

#include  <sane/sane.h>
#include  <sane/saneopts.h>

/* Make sure that the debugging interface is _defined_ before the main
   header file gets included.  The main header file can then just make
   sure this interface is _declared_ for the other source files.  Note
   that sanei_debug.h requires the BACKEND_NAME to be defined.  */
#ifndef BACKEND_NAME
#define BACKEND_NAME epkowa
#endif
#include  "sane/sanei_debug.h"

#include  "epkowa.h"		/* main header file */
#include  "sane/sanei_config.h"
#include  "epkowa_scsi.h"
#include  "epkowa_usb.h"
#include  "sane/sanei_pio.h"
#include  "epkowa_ip.h"		/* interpreter-based scanner support */

#define  EPSON_CONFIG_FILE	"epkowa.conf"
				/* although we could in principle use
				   epson.conf, current version skew
				   between this and the epson backend
				   makes that not such a smart move */
#ifndef  PATH_MAX
#define  PATH_MAX	(1024)
#endif

#ifndef MM_PER_INCH
#define MM_PER_INCH	25.4
#endif

#define  walloc(x)	(x *)malloc(sizeof(x))
#define  walloca(x)	(x *)alloca(sizeof(x))

#ifndef  XtNumber
#define  XtNumber(x)  ( sizeof(x)/ sizeof(x[0]) )
#define  XtOffset(p_type,field)  ((size_t)&(((p_type)NULL)->field))
#define  XtOffsetOf(s_type,field)  XtOffset(s_type*,field)
#endif

#define NUM_OF_HEX_ELEMENTS (16)	/* number of hex numbers per line for data dump */
#define DEVICE_NAME_LEN	(16)	/* length of device name in extended status */

struct ScannerName
{
  char *overseas;
  char *japan;
};
typedef struct ScannerName scanner_name_t;

struct ScannerData
{
  char *fw_name;
  int profile_ID;
  int command_ID;
  const scanner_name_t name;
};
typedef struct ScannerData scanner_data_t;

struct EpsonScanCommand
{
  int command_ID;
  unsigned char set_focus_position;
  unsigned char feed;
  unsigned char eject;
};
typedef struct EpsonScanCommand scan_command_t;

/* NOTE: you can find these codes with "man ascii". */
#define	 STX	0x02
#define	 ACK	0x06
#define	 NAK	0x15
#define	 CAN	0x18
#define	 ESC	0x1B
#define	 PF	0x19

#define	 S_ACK	"\006"
#define	 S_CAN	"\030"

#define  STATUS_FER		0x80	/* fatal error */
#define  STATUS_NOT_READY	0x40	/* not ready */
#define  STATUS_AREA_END	0x20	/* area end */
#define  STATUS_OPTION		0x10	/* option installed */

#define  EXT_STATUS_FER		0x80	/* fatal error */
#define  EXT_STATUS_FBF		0x40	/* flat bed scanner */
#define  EXT_STATUS_WU		0x02	/* warming up */
#define  EXT_STATUS_PB		0x01	/* scanner has a push button */

#define  EXT_STATUS_IST		0x80	/* option detected */
#define  EXT_STATUS_EN		0x40	/* option enabled */
#define  EXT_STATUS_ERR		0x20	/* other error */
#define  EXT_STATUS_PE		0x08	/* no paper */
#define  EXT_STATUS_PJ		0x04	/* paper jam */
#define  EXT_STATUS_OPN		0x02	/* cover open */

#define	 EPSON_LEVEL_A1		 0
#define	 EPSON_LEVEL_A2		 1
#define	 EPSON_LEVEL_B1		 2
#define	 EPSON_LEVEL_B2		 3
#define	 EPSON_LEVEL_B3		 4
#define	 EPSON_LEVEL_B4		 5
#define	 EPSON_LEVEL_B5		 6
#define	 EPSON_LEVEL_B6		 7
#define	 EPSON_LEVEL_B7		 8
#define	 EPSON_LEVEL_B8		 9
#define	 EPSON_LEVEL_F5		10
#define  EPSON_LEVEL_D1		11
#define  EPSON_LEVEL_D7		12
#define  EPSON_LEVEL_D8		13

/* there is also a function level "A5", which I'm igoring here until somebody can 
   convince me that this is still needed. The A5 level was for the GT-300, which
   was (is) a monochrome only scanner. So if somebody really wants to use this
   scanner with SANE get in touch with me and we can work something out - khk */

#define	 EPSON_LEVEL_DEFAULT	EPSON_LEVEL_B3

static EpsonCmdRec epson_cmd[] = {
/*
 *       request identity
 *       |   request identity2
 *       |   |   request status
 *       |   |   |   request condition
 *       |   |   |   |   set color mode
 *       |   |   |   |   |   start scanning
 *       |   |   |   |   |   |   set data format
 *       |   |   |   |   |   |   |   set resolution
 *       |   |   |   |   |   |   |   |   set zoom
 *       |   |   |   |   |   |   |   |   |   set scan area
 *       |   |   |   |   |   |   |   |   |   |   set brightness
 *       |   |   |   |   |   |   |   |   |   |   |              set gamma
 *       |   |   |   |   |   |   |   |   |   |   |              |   set halftoning
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   set color correction
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   initialize scanner
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   set speed
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   set lcount
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   mirror image
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   set gamma table
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   set outline emphasis
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   set dither
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   set color correction coefficients
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   request extension status
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   control an extension
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    forward feed / eject
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   feed
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     request push button status
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   control auto area segmentation
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   set film type
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   set exposure time
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   set bay
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   set threshold
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   |   set focus position
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   |   |   request focus position 
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   |   |   |
 *       |   |   |   |   |   |   |   |   |   |   |              |   |   |   |   |   |   |   |   |   |   |   |   |    |   |     |   |   |   |   |   |   |   |
 */
  {"A1",'I', 0 ,'F','S', 0 ,'G', 0 ,'R', 0 ,'A', 0 ,{ 0, 0, 0}, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"A2",'I', 0 ,'F','S', 0 ,'G','D','R','H','A','L',{-3, 3, 0},'Z','B', 0 ,'@', 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B1",'I', 0 ,'F','S','C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0}, 0 ,'B', 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B2",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B', 0 ,'@', 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B3",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@', 0 , 0 , 0 , 0 , 0 , 0 ,'m','f','e',  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B4",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@','g','d', 0 ,'z','Q','b','m','f','e',  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B5",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@','g','d','K','z','Q','b','m','f','e',  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B6",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z','B','M','@','g','d','K','z','Q','b','m','f','e',  0 , 0   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
  {"B7",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-4, 3, 0},'Z','B','M','@','g','d','K','z','Q','b','m','f','e','\f', 0   ,'!','s','N', 0 , 0 ,'t', 0 , 0 },
  {"B8",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-4, 3, 0},'Z','B','M','@','g','d','K','z','Q','b','m','f','e','\f', 0x19,'!','s','N', 0 , 0 ,'t','p','q'},
  {"F5",'I', 0 ,'F','S','C','G','D','R','H','A','L',{-3, 3, 0},'Z', 0 ,'M','@','g','d','K','z','Q', 0 ,'m','f','e','\f', 0   , 0 , 0 ,'N','T','P', 0 , 0 , 0 },
  {"D1",'I','i','F', 0 ,'C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0},'Z', 0 , 0 ,'@','g','d', 0 ,'z', 0 , 0 ,'m','f', 0 ,  0 , 0   ,'!', 0 , 0 , 0 , 0 ,'t', 0 , 0 },
  {"D2",'I','i','F', 0 ,'C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0},'Z', 0 , 0 ,'@','g','d', 0 ,'z', 0 , 0 ,'m','f','e',  0 , 0   ,'!', 0 ,'N', 0 , 0 ,'t', 0 , 0 },
  {"D7",'I','i','F', 0 ,'C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0},'Z', 0 , 0 ,'@','g','d', 0 ,'z', 0 , 0 , 0 ,'f','e','\f', 0   ,'!', 0 ,'N', 0 , 0 ,'t', 0 , 0 },
  {"D8",'I','i','F', 0 ,'C','G','D','R', 0 ,'A', 0 ,{ 0, 0, 0},'Z', 0 , 0 ,'@','g','d', 0 ,'z', 0 , 0 ,'0','f','e','\f', 0   ,'!', 0 ,'N', 0 , 0 ,'t', 0 , 0 },
};

/*--------------------------------------------------------------*/
static EpsonScanHardRec epson_scan_hard[] = {
  /* reflect, color nega, mono nega, posi */
  /* id,       (0,0)   (0,1)   (0,2)   (1,0)   (1,1)   (1,2)   (2,0)   (2,1)   (2,2) */
  {0x00,
   {{1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* default */
  {0x05,
   {{1.1419, -0.0596, -0.0825, -0.1234, 1.2812, -0.1413, 0.0703, -0.5720,
     1.5016},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1419, -0.0596, -0.0825, -0.1234, 1.2812, -0.1413, 0.0703, -0.5720, 1.5016}}},	/* ES-6000H */
  {0x06,
   {{1.1442, -0.0705, -0.0737, -0.0702, 1.1013, -0.0311, -0.0080, -0.3588,
     1.3668},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* GT-6600 */
  {0x07,
   {{1.1967, -0.1379, -0.0588, -0.0538, 1.0385, 0.0153, 0.0348, -0.4070,
     1.3721},
    {1.0010, -0.0010, 0.0000, -0.1120, 1.1710, -0.0590, 0.0000, -0.0910,
     1.0920},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1967, -0.1379, -0.0588, -0.0538, 1.0385, 0.0153, 0.0348, -0.4070, 1.3721}}},	/* GT-7600 */
  {0x0D,
   {{1.1980, -0.1365, -0.0616, -0.1530, 1.1729, -0.0198, -0.0025, -0.2776,
     1.2801},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1980, -0.1365, -0.0616, -0.1530, 1.1729, -0.0198, -0.0025, -0.2776, 1.2801}}},	/* ES-2000 */
  {0x0F,
   {{1.0961, -0.0181, -0.0779, -0.1279, 1.1957, -0.0678, 0.0315, -0.3891,
     1.3576},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0961, -0.0181, -0.0779, -0.1279, 1.1957, -0.0678, 0.0315, -0.3891, 1.3576}}},	/* ES-8500 */
  {0x15,
   {{1.0999, -0.0425, -0.0574, -0.0806, 1.0835, -0.0028, 0.0057, -0.2924,
     1.2866},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* GT-6700 */
  {0x16,
   {{1.2020, -0.1518, -0.0502, -0.0847, 1.1385, -0.0538, 0.0059, -0.3255,
     1.3196},
    {1.0030, -0.0030, 0.0000, -0.0980, 1.1500, -0.0520, -0.0030, -0.0840,
     1.0880},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2456, -0.1617, -0.0839, -0.1160, 1.1862, -0.0702, -0.0036, -0.3438, 1.3473}}},	/* GT-8700 */
  {0x18,
   {{1.1339, -0.0526, -0.0813, -0.1177, 1.1661, -0.0485, -0.0030, -0.3298,
     1.3328},
    {1.0010, -0.0010, 0.0000, -0.1120, 1.1710, -0.0590, 0.0000, -0.0910,
     1.0920},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2066, -0.0360, -0.1706, -0.1313, 1.2523, -0.1210, -0.0299, -0.3377, 1.3676}}},	/* GT-7700 */
  {0x1A,
   {{1.0986, 0.0235, -0.1221, -0.1294, 1.0896, 0.0399, 0.0928, -0.6043,
     1.5115},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* ES-9000H */
  {0x1B,
   {{1.1855, -0.1372, -0.0483, -0.2060, 1.2468, -0.0407, 0.0358, -0.3059,
     1.2701},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1976, -0.1182, -0.0794, -0.1578, 1.2720, -0.1142, 0.0122, -0.3467, 1.3345}}},	/* ES-2200 */
  {0x1D,
   {{1.0675, -0.0586, -0.0088, -0.0332, 0.9716, 0.0616, 0.0175, -0.4054,
     1.3879},
    {1.0090, -0.0090, 0.0000, -0.0390, 1.0750, -0.0360, -0.0070, -0.1060,
     1.1130},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1394, -0.0829, -0.0564, -0.0003, 1.0008, -0.0004, -0.0059, -0.3674, 1.3733}}},	/* GT-7200 */
  {0x1F,
   {{1.0800, -0.0607, -0.0193, -0.0787, 1.0846, -0.0059, 0.0135, -0.3334,
     1.3199},
    {1.0040, -0.0040, 0.0000, -0.0780, 1.1360, -0.0570, -0.0020, -0.0810,
     1.0830},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1334, -0.0929, -0.0405, -0.0418, 1.0689, -0.0271, -0.0521, -0.3262, 1.3783}}},	/* GT-8200 */
  {0x21,
   {{1.0919, -0.0739, -0.0180, -0.0941, 1.1150, -0.0209, 0.0220, -0.3744,
     1.3524},
    {1.0090, -0.0100, 0.0010, -0.0720, 1.1310, -0.0600, 0.0000, -0.1000,
     1.1000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1374, -0.1396, 0.0021, -0.0489, 1.0655, -0.0166, 0.0081, -0.3492, 1.3411}}},	/* GT-9700 */
  {0x23,
   {{1.0339, -0.0166, -0.0173, -0.0117, 0.9797, 0.0319, 0.0010, -0.3609,
     1.3599},
    {1.0090, -0.0090, 0.0000, -0.0390, 1.0750, -0.0360, -0.0070, -0.1060,
     1.1130},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1666, -0.0898, -0.0768, -0.0076, 1.0157, -0.0081, 0.0012, -0.3048, 1.3036}}},	/* GT-7300 */
  {0x25,
   {{1.0800, -0.0607, -0.0193, -0.0787, 1.0846, -0.0059, 0.0135, -0.3334,
     1.3199},
    {1.0040, -0.0040, 0.0000, -0.0780, 1.1360, -0.0570, -0.0020, -0.0810,
     1.0830},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1334, -0.0929, -0.0405, -0.0418, 1.0689, -0.0271, -0.0521, -0.3262, 1.3783}}},	/* GT-8300 */
  {0x27,
   {{1.0919, -0.0739, -0.0180, -0.0941, 1.1150, -0.0209, 0.0220, -0.3744,
     1.3524},
    {1.0083, -0.0094, 0.0011, -0.0760, 1.1379, -0.0619, -0.0002, -0.0945,
     1.0947},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1952, -0.1519, -0.0433, -0.0932, 1.1613, -0.0681, -0.0418, -0.3140, 1.3558}}},	/* GT-9300 */
  {0x29,
   {{1.0369, -0.0210, -0.0160, -0.0820, 1.1160, -0.0341, 0.0150, -0.5035,
     1.4885},
    {1.0122, -0.0151, 0.0029, -0.0861, 1.1402, -0.0542, -0.0061, -0.1607,
     1.1669},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1764, -0.1749, -0.0014, -0.0590, 1.0983, -0.0393, 0.0208, -0.5194, 1.4986}}},	/* GT-9800F */
  {0x2B,
   {{1.0305, -0.0116, -0.0189, -0.0936, 1.1245, -0.0309, -0.0072, -0.1413,
     1.1485},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* ES-7000H */
  {0x32,
   {{1.0932, -0.0529, -0.0403, -0.1077, 1.1416, -0.0338, 0.0079, -0.5525,
     1.5446},
    {1.0259, -0.0356, 0.0097, -0.1085, 1.2225, -0.1140, -0.0046, -0.1848,
     1.1894},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2720, -0.2665, -0.0054, -0.0672, 1.1301, -0.0629, -0.0048, -0.3917, 1.3965}}},	/* GT-9400 */
  {0x2D,
   {{1.0436, -0.0078, -0.0359, -0.0169, 1.0114, 0.0056, 0.0308, -0.4425,
     1.4117},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* CC-600PX */
  {0x3A,
   {{1.1150, -0.0677, -0.0473, -0.1179, 1.1681, -0.0502, 0.0052, -0.4858,
     1.4806},
    {1.0133, -0.0151, 0.0017, -0.1216, 1.2207, -0.0991, -0.0003, -0.1512,
     1.1515},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2105, -0.1644, -0.0461, -0.1124, 1.1945, -0.0820, -0.0450, -0.3367, 1.3817}}},	/* PM-A850 */
  {0x36,
   {{1.0848, -0.0153, -0.0695, -0.0902, 1.0611, 0.0291, 0.0344, -0.5002,
     1.4658},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* CX5300/CX5400 */
  {0x34,
   {{1.1032, -0.0590, -0.0442, -0.1915, 1.3371, -0.1456, 0.0387, -0.5804,
     1.5417},
    {1.0232, -0.0258, 0.0026, -0.1296, 1.2882, -0.1587, -0.0011, -0.1928,
     1.1940},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2662, -0.2664, 0.0002, -0.1050, 1.3168, -0.2118, -0.0058, -0.4370, 1.4428}}},	/* GT-X700 */
  {0x38,
   {{1.1150, -0.0677, -0.0473, -0.1179, 1.1681, -0.0502, 0.0052, -0.4858,
     1.4806},
    {1.0133, -0.0151, 0.0017, -0.1216, 1.2207, -0.0991, -0.0003, -0.1512,
     1.1515},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2105, -0.1644, -0.0461, -0.1124, 1.1945, -0.0820, -0.0450, -0.3367, 1.3817}}},	/* RX500/RX510 */
  {0x37,
   {{0.9640, 0.1455, -0.1095, 0.0108, 1.1933, -0.2041, 0.0071, -0.3487,
     1.3416},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* CX6300/CX6400 */
  {0x3F,
   {{1.1223, -0.0985, -0.0238, -0.0847, 1.1502, -0.0655, 0.0118, -0.5022,
     1.4904},
    {1.0077, -0.0129, 0.0052, -0.0904, 1.1785, -0.0881, 0.0000, -0.1528,
     1.1528},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1927, -0.1646, -0.0280, -0.0655, 1.1033, -0.0378, 0.0034, -0.4173, 1.4139}}},	/* ES-10000G */
  {0x41,
   {{1.0732, -0.0581, -0.0150, -0.0897, 1.1553, -0.0657,-0.0179, -0.6500,
     1.6679},
    {1.0163, -0.0203, 0.0040, -0.1125, 1.1797, -0.0672, -0.0091, -0.2343,
     1.2434},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2437, -0.2022, -0.0415, -0.0352, 1.0735, -0.0383,-0.0188, -0.5020, 1.5209}}},   /* GT-F500/F550 */
  {0x43,
   {{1.0782, -0.0697, -0.0085, -0.1605, 1.2862, -0.1257, 0.0148, -0.5854,
     1.5706},
    {1.0136, -0.0151, 0.0016, -0.1836, 1.3422, -0.1586, -0.0014, -0.1851,
     1.1865},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1491, -0.1456, -0.0035, -0.0990, 1.2657, -0.1666, 0.0015, -0.3868, 1.3853}}},    /* GT-F600 */
  {0x46,
   {{0.9828, 0.0924, -0.0752, 0.0255, 1.1510, -0.1765, 0.0049, -0.3250,
     1.3201},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* CX3500/CX3600/CX4500/CX4600 */
  {0x48,
   {{0.9716, 0.0927, -0.0643, 0.0010, 1.1068, -0.1078, 0.0101, -0.3046,
     1.2945},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* PM-A700/RX420/RX430 */
  {0x49,
   {{0.9640, 0.1455, -0.1095, 0.0108, 1.1933, -0.2041, 0.0071, -0.3487,
     1.3416},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* CX6500/CX6600 */
  {0x4B,
   {{1.1150, -0.0677, -0.0473, -0.1179, 1.1681, -0.0502, 0.0052, -0.4858,
     1.4806},
    {1.0133, -0.0151, 0.0017, -0.1216, 1.2207, -0.0991, -0.0003, -0.1512,
     1.1515},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2105, -0.1644, -0.0461, -0.1124, 1.1945, -0.0820, -0.0450, -0.3367, 1.3817}}},	/* PM-A870 */
  {0x4D,
   {{1.1011, -0.0824, -0.0186, -0.0970, 1.1991, -0.1021, -0.0161, -0.6247,
     1.6408},
    {1.0259, -0.0356, 0.0097, -0.1085, 1.2225, -0.1140, -0.0046, -0.1848,
     1.1894},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2150, -0.2074, -0.0076, -0.0521, 1.1430, -0.0909, -0.0204, -0.4156, 1.4360}}},  /* PM-A900 */
  {0x4F,
   {{1.1052, -0.0850, -0.0202, -0.1050, 1.2294, -0.1245, -0.0486, -0.4160,
     1.4646},
    {1.0255, -0.0272, 0.0017, -0.0919, 1.2098, -0.1180, -0.0021, -0.1296,
     1.1317},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2950, -0.2619, -0.0332, -0.0562, 1.1587, -0.1025, -0.0397, -0.3100, 1.3497}}},  /* GT-X800 */
  {0x51,
   {{1.0614, -0.0361, -0.0253, -0.1081, 1.1320, -0.0240, -0.0536, -0.2045,
     1.2580},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* LP-A500 */
  {0x52,
   {{1.0978, -0.0806, -0.0173, -0.0802, 1.1515, -0.0713, -0.0476, -0.4656,
     1.5132},
    {1.0192, -0.0192,  0.0000, -0.0974, 1.1846, -0.0872, -0.0031, -0.1797,
     1.1828},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2490, -0.2030, -0.0460, -0.0469, 1.1046, -0.0577, -0.0361, -0.3857,
     1.4217}}},	/* GT-F520/F570 */
  {0x54,
   {{1.0905, -0.0654, -0.0251, -0.1030, 1.1801, -0.0771, -0.0685, -0.4238,
     1.4923},
    {1.0206, -0.0207,  0.0000, -0.0890, 1.1770, -0.0880, -0.0014, -0.1450,
     1.1464},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.3041, -0.2907, -0.0134, -0.0383, 1.0908, -0.0525, -0.0327, -0.2947,
     1.3275}}},	/* GT-X750 */
  {0x56,
   {{1.0784, -0.0560, -0.0224, -0.1793, 1.2234, -0.0441, -0.0041, -0.2636,
     1.2677},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* LP-M5500 */
  {0x57,
   {{0.9828, 0.0924, -0.0752, 0.0255, 1.1510, -0.1765, 0.0049, -0.3250,
     1.3201},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* Stylus CX3700/CX3800/DX3800 */
  {0x58,
   {{0.9716, 0.0927, -0.0643, 0.0010, 1.1068, -0.1078, 0.0101, -0.3046,
     1.2945},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* PX-A650/Stylus CX4700/CX4800/DX4800 */
  {0x59,
   {{0.9716, 0.0927, -0.0643, 0.0010, 1.1068, -0.1078, 0.0101, -0.3046,
     1.2945},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* Stylus CX4100/CX4200/DX4200 */
  {0x5B,
   {{0.9764,  0.1095, -0.0859,  0.0149, 1.1154, -0.1303,  0.0051, -0.2851,
     1.2800},
    {1.0024, -0.0149,  0.0124, -0.2569, 1.3432, -0.0864, -0.0043, -0.1306,
     1.1349},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1003, -0.0493, -0.0510, -0.1607, 1.2748, -0.1142, -0.0059, -0.3161,
     1.3220}}},	/* Stylus CX7700/CX7800 */
  {0x5D,
   {{0.9764,  0.1095, -0.0859,  0.0149, 1.1154, -0.1303,  0.0051, -0.2851,
     1.2800},
    {1.0024, -0.0149,  0.0124, -0.2569, 1.3432, -0.0864, -0.0043, -0.1306,
     1.1349},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.1003, -0.0493, -0.0510, -0.1607, 1.2748, -0.1142, -0.0059, -0.3161,
     1.3220}}},	/* Stylus Photo RX520/RX530 */
  {0x5F,
   {{1.0697, -0.0561, -0.0137, -0.0824, 1.1291, -0.0467, -0.0390, -0.5218,
     1.5608},
    {1.0208, -0.0209,  0.0000, -0.0923, 1.2017, -0.1093, -0.0020, -0.1290,
     1.1310},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2606, -0.2125, -0.0482, -0.0567, 1.1441, -0.0874, -0.0431, -0.3490,
     1.3921}}},	/* Stylus Photo RX640/RX650 */
  {0x61,
   {{1.0921, -0.0722, -0.0199, -0.0831, 1.1550, -0.0718, -0.0452, -0.3721,
     1.4173},
    {1.0168, -0.0168,  0.0000, -0.0953, 1.1928, -0.0975, -0.0012, -0.1235,
     1.1247},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2603, -0.2763,  0.0155, -0.0398, 1.1033, -0.0635, -0.0249, -0.2675,
     1.2924}}},	/* PM-A950 */
  {0x63,
   {{1.0976, -0.0789, -0.0187, -0.0958, 1.1821, -0.0863, -0.0565, -0.4179,
     1.4744},
    {1.0250, -0.0267,  0.0016, -0.0930, 1.2108, -0.1178, -0.0022, -0.1296,
     1.1317},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.3111, -0.2979, -0.0132, -0.0441, 1.1148, -0.0707, -0.0348, -0.2971,
     1.3319}}},	/* GT-X900 */
  {0x65,
   {{1.0359, -0.0146, -0.0213,  -0.0752, 1.0963, -0.0211, -0.0456, -0.3238,
     1.3693},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* ES-H300 */
  {0x66,
   {{1.0878, -0.0667, -0.0211, -0.0892, 1.1513, -0.0622, -0.0654, -0.5175,
     1.5829},
    {1.0208, -0.0209,  0.0000, -0.0923, 1.2017, -0.1093, -0.0020, -0.1290,
     1.1310},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2688, -0.2522, -0.0166, -0.0559, 1.1291, -0.0733, -0.0377, -0.3519,
     1.3896}}},	/* GT-S600/F650, Perfection V10/V100 */
  {0x68,
   {{1.0950, -0.0646, -0.0305, -0.0792, 1.1398, -0.0606, -0.0123, -0.5175,
     1.5298},
    {1.0258, -0.0306,  0.0048, -0.0995, 1.2173, -0.1178, -0.0054, -0.1242,
     1.1296},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2697, -0.2501, -0.0195, -0.0351, 1.1236, -0.0885, -0.0131, -0.3268,
     1.3400}}},	/* GT-F700, Perfection V350 */
  {0x6A,
   {{0.9828, 0.0924, -0.0752, 0.0255, 1.1510, -0.1765, 0.0049, -0.3250, 
     1.3201},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* Stylus CX2800/CX2900/ME200 */
  {0x6B,
   {{0.9828, 0.0924, -0.0752, 0.0255, 1.1510, -0.1765, 0.0049, -0.3250, 
     1.3201},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* Stylus PX-A620, CX3900/DX4000  */
  {0x6C,
   {{0.9716, 0.0927, -0.0643,  0.0010, 1.1068, -0.1078,  0.0101, -0.3046, 
     1.2945},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* Stylus CX5900/CX6000/DX6000 */
  {0x70,
   {{0.9533, 0.0885, -0.0418, 0.0033, 1.0627, -0.0660, -0.0137, -0.1904,
     1.2041},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* Stylus Photo RX560/RX580/RX590 */
  {0x71,
   {{1.0697, -0.0561, -0.0137, -0.0824, 1.1291, -0.0467, -0.0390, -0.5218,
     1.5608},
    {1.0208, -0.0209, 0.0000,  -0.0923, 1.2017, -0.1093,  -0.0020, -0.1290,
     1.1310},    
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2606, -0.2125, -0.0482, -0.0567, 1.1441, -0.0874, -0.0431, -0.3490, 
     1.3921}}},	/* PM-A920 */
  {0x73,
   {{1.0828, -0.0739, -0.0089, -0.0895, 1.1597, -0.0702, -0.0531, -0.4291,
     1.4822},
    {1.0258, -0.0306, 0.0048, -0.0995, 1.2173, -0.1178, -0.0054, -0.1242, 
     1.1296},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2579, -0.2384, -0.0195, -0.0569, 1.1454, -0.0884, -0.0411, -0.3072, 
     1.3483}}}, /* PM-A970 */
  {0x75,
   {{1.0828, -0.0739, -0.0089, -0.0895, 1.1597, -0.0702, -0.0531, -0.4291,
     1.4822},
    {1.0258, -0.0306, 0.0048, -0.0995, 1.2173, -0.1178, -0.0054, -0.1242, 
     1.1296},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.2579, -0.2384, -0.0195, -0.0569, 1.1454, -0.0884, -0.0411, -0.3072, 
     1.3483}}}, /* PM-T990 */
  {0x77,
   {{0.9716, 0.0927, -0.0643,  0.0010, 1.1068, -0.1078,  0.0101, -0.3046,
     1.2945},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* Stylus CX4900/CX5000/DX5000 */
  {0x78,
   {{1.0784, -0.0560, -0.0224, -0.1793, 1.2234, -0.0441, -0.0041, -0.2636,
     1.2677},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* LP-M5600 */
  {0x79,
   {{1.0614, -0.0361, -0.0253, -0.1081, 1.1320, -0.0240, -0.0536, -0.2045,
     1.2580},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    {1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},	/* AcuLaser CX21 */
  {0x7A,	/* GT-F670, Perfection V200 */
   {{ 1.1754,-0.1173,-0.0580,-0.0687, 1.1307,-0.0620,-0.0255,-0.4699, 1.4954},
    { 1.0150,-0.0173, 0.0022,-0.0853, 1.2238,-0.1384,-0.0073,-0.1490, 1.1562},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.4283,-0.4335, 0.0052,-0.0170, 1.1308,-0.1138,-0.0147,-0.2230, 1.2377}}},
  {0x7E,	/* Stylus CX4300/CX4400/CX5500/CX5600/DX4400 */
   {{ 0.9828, 0.0924,-0.0752, 0.0255, 1.1510,-0.1765, 0.0049,-0.3250, 1.3201},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},
  {0x7F,	/* Stylus CX7300/CX7400/DX7400 */
   {{ 1.0936,-0.0142,-0.0795,-0.0001, 1.0951,-0.0949, 0.0308,-0.2967, 1.2659},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},
  {0x80,	/* PX-A740, Stylus CX8300/CX8400/DX8400 */
   {{ 1.0936,-0.0142,-0.0795,-0.0001, 1.0951,-0.0949, 0.0308,-0.2967, 1.2659},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},
  {0x81,	/* Stylus CX9300F/CX9400Fax/DX9400F */
   {{ 1.1090,-0.0304,-0.0786, 0.0194, 1.1078,-0.1272,-0.0077,-0.1293, 1.1370},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},
  {0x82,	/* PM-T960 */
   {{ 1.1622,-0.1102,-0.0519,-0.0717, 1.1060,-0.0343,-0.0248,-0.4138, 1.4385},
    { 0.9913, 0.0082, 0.0005,-0.1259, 1.0452, 0.0807,-0.0072,-0.0767, 1.0839},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.3900,-0.3008,-0.0892,-0.0254, 1.0890,-0.0636,-0.0300,-0.2501, 1.2801}}},
  {0x84,	/* PM-A940, Stylus Photo RX680/RX685/RX690 */
   {{ 1.0934,-0.0042,-0.0892, 0.0052, 1.1019,-0.1071, 0.0259,-0.2651, 1.2392},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},
  {0x85,	/* PM-A840, Stylus Photo RX585/RX595/RX610 */
   {{ 1.0534, 0.0399,-0.0934, 0.0098, 1.0589,-0.0687, 0.0016,-0.1131, 1.1115},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000},
    { 1.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 1.0000}}},
};

const struct ScannerData scanner_data[] = {
  {"GT-10000", 0x05, 0x05, {NULL, NULL}},
  {"ES-6000", 0x05, 0x05, {NULL, NULL}},
  {"Perfection610", 0x06, 0x01, {"Perfection 610", NULL}},
  {"GT-6600", 0x06, 0x01, {NULL, NULL}},
  {"Perfection1200", 0x07, 0x01, {"Perfection 1200", NULL}},
  {"GT-7600", 0x07, 0x01, {NULL, NULL}},
  {"Expression1600", 0x0D, 0x02, {"Expression 1600", NULL}},
  {"ES-2000", 0x0D, 0x02, {NULL, NULL}},
  {"Expression1640XL", 0x0F, 0x04, {"Expression 1640XL", NULL}},
  {"ES-8500", 0x0F, 0x04, {NULL, NULL}},
  {"Perfection640", 0x15, 0x01, {"Perfection 640", NULL}},
  {"GT-6700", 0x15, 0x01, {NULL, NULL}},
  {"Perfection1640", 0x16, 0x01, {"Perfection 1640", NULL}},
  {"GT-8700", 0x16, 0x01, {NULL, NULL}},
  {"Perfection1240", 0x18, 0x01, {"Perfection 1240", NULL}},
  {"GT-7700", 0x18, 0x01, {NULL, NULL}},
  {"GT-30000", 0x1A, 0x05, {NULL, NULL}},
  {"ES-9000H", 0x1A, 0x05, {NULL, NULL}},
  {"Expression1680", 0x1B, 0x02, {"Expression 1680", NULL}},
  {"ES-2200", 0x1B, 0x02, {NULL, NULL}},
  {"GT-7200", 0x1D, 0x01, {"Perfection 1250", "GT-7200"}},
  {"GT-8200", 0x1F, 0x01, {"Perfection 1650", "GT-8200"}},
  {"GT-9700", 0x21, 0x01, {"Perfection 2450", "GT-9700"}},
  {"GT-7300", 0x23, 0x01, {"Perfection 1260", "GT-7300"}},
  {"GT-8300", 0x25, 0x01, {"Perfection 1660", "GT-8300"}},
  {"GT-9300", 0x27, 0x01, {"Perfection 2400", "GT-9300"}},
  {"GT-9800", 0x29, 0x01, {"Perfection 3200", "GT-9800"}},
  {"ES-7000H", 0x2B, 0x05, {"GT-15000", "ES-7000H"}},
  {"LP-A500", 0x51, 0x01, {NULL, NULL}},
  {"AL-CX11", 0x51, 0x01, {"AcuLaser CX11", NULL}},
  {"GT-9400", 0x32, 0x06, {"Perfection 3170", "GT-9400"}},
  {"CC-600PX", 0x2D, 0x01, {"Stylus CX5100/CX5200", "CC-600PX"}},
  {"PM-A850", 0x3A, 0x01, {"Stylus Photo RX600", "PM-A850"}},
  {"CX5400", 0x36, 0x01, {"Stylus CX5300/CX5400", NULL}},
  {"GT-X700", 0x34, 0x01, {"Perfection 4870", "GT-X700"}},
  {"RX500", 0x38, 0x01, {"Stylus Photo RX500/RX510", NULL}},
  {"PX-A650", 0x37, 0x01, {"Stylus CX6300/CX6400", NULL}},
  {"ES-10000G", 0x3F, 0x05, {NULL, NULL}},
  {"Expression10000", 0x3F, 0x05, {"Expression 10000XL", NULL}},
  {"CX4600", 0x46, 0x01, {"Stylus CX4500/CX4600", NULL}},
  {"CX6600", 0x49, 0x01, {"Stylus CX6500/CX6600", NULL}},
  {"CX3600", 0x46, 0x01, {"Stylus CX3500/CX3600", "PX-A550"}},
  {"RX420", 0x48, 0x01, {"Stylus Photo RX420/RX425/RX430", NULL}},
  {"PM-A700", 0x48, 0x01, {NULL, NULL}},
  {"PM-A870", 0x4B, 0x01, {"Stylus Photo RX620/RX630", "PM-A870"}},
  {"GT-F500", 0x41, 0x06, {"Perfection 2480/2580", "GT-F500/F550"}},
  {"GT-F600", 0x43, 0x06, {"Perfection 4180", "GT-F600"}},
  {"PM-A900", 0x4D, 0x01, {"Stylus Photo RX700", "PM-A900"}},
  {"GT-X800", 0x4F, 0x01, {"Perfection 4990", "GT-X800"}},
  {"GT-X750", 0x54, 0x01, {"Perfection 4490", "GT-X750"}},
  {"LP-M5500", 0x56, 0x05, {NULL, NULL}},
  {"LP-M5600", 0x78, 0x05, {NULL, "LP-M5600"}},
  {"GT-F520", 0x52, 0x01, {"Perfection 3490/3590", "GT-F520/F570"}},
  {"CX3800", 0x57, 0x01, {"Stylus CX3700/CX3800/DX3800", NULL}},
  {"CX7800", 0x5B, 0x01, {"Stylus CX7700/CX7800", NULL}},
  {"PM-A750", 0x5D, 0x01, {"Stylus Photo RX520/RX530", "PM-A750"}},
  {"CX4800", 0x58, 0x01, {"Stylus CX4700/CX4800/DX4800", "PX-A650"}},
  {"CX4200", 0x59, 0x01, {"Stylus CX4100/CX4200/DX4200", NULL}},
  {"PM-A950", 0x61, 0x01, {NULL, "PM-A950"}},
  {"PM-A890", 0x5F, 0x01, {"Stylus Photo RX640/RX650", "PM-A890"}},
  {"GT-X900", 0x63, 0x01, {"Perfection V700/V750", "GT-X900"}},
  {"CX4000", 0x6B, 0x01, {"Stylus CX3900/DX4000", "PX-A620"}},
  {"CX3000v", 0x6A, 0x01, {"Stylus CX2800/CX2900/ME200", NULL}},
  {"ES-H300", 0x65, 0x01, {"GT-2500", "ES-H300"}},
  {"CX6000", 0x6C, 0x01, {"Stylus CX5900/CX6000/DX6000", "PX-A720"}},
  {"PM-A820", 0x70, 0x01, {"Stylus Photo RX560/RX580/RX590", "PM-A820"}},
  {"PM-A920", 0x71, 0x01, {NULL, "PM-A920"}},
  {"PM-A970", 0x73, 0x01, {NULL, "PM-A970"}},
  {"PM-T990", 0x75, 0x01, {NULL, "PM-T990"}},
  {"CX5000", 0x77, 0x01, {"Stylus CX4900/CX5000/DX5000", NULL}},
  {"GT-S600", 0x66, 0x01, {"Perfection V10/V100", "GT-S600/GT-F650"}},
  {"GT-F700", 0x68, 0x01, {"Perfection V350", "GT-F700"}},
  {"AL-CX21", 0x79, 0x01, {"AcuLaser CX21", NULL}},
  {"GT-F670", 0x7A, 0x01, {"Perfection V200", "GT-F670"}},
  {"CX4400", 0x7E, 0x01, {"Stylus CX4300/CX4400/CX5500/CX5600/DX4400", NULL}},
  {"CX7400", 0x7F, 0x01, {"Stylus CX7300/CX7400/DX7400", NULL}},
  {"CX8400", 0x80, 0x01, {"Stylus CX8300/CX8400/DX8400", "PX-A740"}},
  {"CX9400Fax", 0x81, 0x01, {"Stylus CX9300F/CX9400Fax/DX9400F", NULL}},
  {"PM-T960", 0x82, 0x01, {NULL, "PM-T960"}},
  {"PM-A940", 0x84, 0x01, {"Stylus Photo RX680/RX685/RX690", "PM-A940"}},
  {"PM-A840", 0x85, 0x01, {"Stylus Photo RX585/RX595/RX610", "PM-A840"}},
/* array terminator */
  {NULL, 0x00, 0x00, {NULL, NULL}},
};

#define ILLEGAL_CMD   0xFF

static struct EpsonScanCommand scan_command[] = {
  {0x01, 0, 0, ILLEGAL_CMD},
  {0x02, ILLEGAL_CMD, 0, ILLEGAL_CMD},
  {0x03, 0, ILLEGAL_CMD, ILLEGAL_CMD},
  {0x04, ILLEGAL_CMD, ILLEGAL_CMD, ILLEGAL_CMD},
  {0x05, 0, 0x19, ILLEGAL_CMD},
  {0x06, 0, 0, '\f'},
};



/*
 * Definition of the mode_param struct, that is used to 
 * specify the valid parameters for the different scan modes.
 *
 * The depth variable gets updated when the bit depth is modified.
 */

struct mode_param
{
  int color;
  int mode_flags;
  int dropout_mask;
  int depth;
};

static struct mode_param mode_params[] = {
  {0, 0x00, 0x30, 1},
  {0, 0x00, 0x30, 8},
  {1, 0x02, 0x00, 8}
};

static const SANE_String_Const mode_list[] = {
  SANE_I18N ("Binary"),
  SANE_I18N ("Gray"),
  SANE_I18N ("Color"),
  NULL
};

static const SANE_String_Const adf_mode_list[] = {
  SANE_I18N ("Simplex"),
  SANE_I18N ("Duplex"),
  NULL
};


/*
 * Define the different scan sources:
 */

#define  FBF_STR	SANE_I18N("Flatbed")
#define  TPU_STR	SANE_I18N("Transparency Unit")
#define  ADF_STR	SANE_I18N("Automatic Document Feeder")

/*
 * source list need one dummy entry (save device settings is crashing).
 * NOTE: no const - this list gets created while exploring the capabilities
 * of the scanner.
 */

static SANE_String_Const source_list[] = { 
  FBF_STR, 
  NULL, 
  NULL, 
  NULL
};

/* some defines to make handling the TPU easier: */
#define FILM_TYPE_POSITIVE	(0)
#define FILM_TYPE_NEGATIVE	(1)

static const SANE_String_Const film_list[] = { 
  SANE_I18N ("Positive Film"), 
  SANE_I18N ("Negative Film"), 
  NULL
};

static const SANE_String_Const focus_list[] = {
  SANE_I18N ("Focus on glass"),
  SANE_I18N ("Focus 2.5mm above glass"),
  NULL
};

/*
 * TODO: add some missing const.
 */

#define HALFTONE_NONE 0x01
#define HALFTONE_TET 0x03

static int halftone_params[] = { 
  HALFTONE_NONE, 
  0x00, 
  0x10, 
  0x20, 
  0x80, 
  0x90, 
  0xa0, 
  0xb0, 
  HALFTONE_TET,
  0xc0, 
  0xd0
};

static const SANE_String_Const halftone_list[] = { 
  SANE_I18N ("None"), 
  SANE_I18N ("Halftone A (Hard Tone)"),
  SANE_I18N ("Halftone B (Soft Tone)"), 
  SANE_I18N ("Halftone C (Net Screen)"),
  NULL
};

static const SANE_String_Const halftone_list_4[] = { 
  SANE_I18N ("None"), 
  SANE_I18N ("Halftone A (Hard Tone)"),
  SANE_I18N ("Halftone B (Soft Tone)"), 
  SANE_I18N ("Halftone C (Net Screen)"),
  SANE_I18N ("Dither A (4x4 Bayer)"),
  SANE_I18N ("Dither B (4x4 Spiral)"),
  SANE_I18N ("Dither C (4x4 Net Screen)"),
  SANE_I18N ("Dither D (8x4 Net Screen)"), 
  NULL
};

static const SANE_String_Const halftone_list_7[] = { 
  SANE_I18N ("None"), 
  SANE_I18N ("Halftone A (Hard Tone)"),
  SANE_I18N ("Halftone B (Soft Tone)"), 
  SANE_I18N ("Halftone C (Net Screen)"),
  SANE_I18N ("Dither A (4x4 Bayer)"),
  SANE_I18N ("Dither B (4x4 Spiral)"),
  SANE_I18N ("Dither C (4x4 Net Screen)"),
  SANE_I18N ("Dither D (8x4 Net Screen)"),
  SANE_I18N ("Text Enhanced Technology"), 
  SANE_I18N ("Download pattern A"),
  SANE_I18N ("Download pattern B"), 
  NULL
};

static int dropout_params[] = { 
  0x00,	/* none */
  0x10,	/* red */
  0x20,	/* green */
  0x30	/* blue */
};

static const SANE_String_Const dropout_list[] = { 
  SANE_I18N ("None"), 
  SANE_I18N ("Red"), 
  SANE_I18N ("Green"),
  SANE_I18N ("Blue"), 
  NULL
};

/*
 * Color correction:
 * One array for the actual parameters that get sent to the scanner (color_params[]),
 * one array for the strings that get displayed in the user interface (color_list[])
 * and one array to mark the user defined color correction (dolor_userdefined[]). 
 */

static int color_params_ab[] = {
  0x00,
  0x01,
  0x10,
  0x20,
  0x40,
  0x80
};

static SANE_Bool color_userdefined_ab[] = {
  SANE_FALSE,
  SANE_TRUE,
  SANE_FALSE,
  SANE_FALSE,
  SANE_FALSE,
  SANE_FALSE
};

static const SANE_String_Const color_list_ab[] = {
  SANE_I18N ("No Correction"),
  SANE_I18N ("User defined"),
  SANE_I18N ("Impact-dot printers"),
  SANE_I18N ("Thermal printers"),
  SANE_I18N ("Ink-jet printers"),
  SANE_I18N ("CRT monitors"),
  NULL
};

static int color_params_d[] = {
  0x00
};

static SANE_Bool color_userdefined_d[] = {
  SANE_TRUE
};

static const SANE_String_Const color_list_d[] = {
  "User defined",
  NULL
};

static SANE_Bool *color_userdefined;
static int *color_params;

/*
 * Gamma correction:
 * The A and B level scanners work differently than the D level scanners, therefore
 * I define two different sets of arrays, plus one set of variables that get set to
 * the actally used params and list arrays at runtime.
 */

static int gamma_params_ab[] = {
  0x01,
  0x03,
  0x04,
  0x00,
  0x10,
  0x20
};

static const SANE_String_Const gamma_list_ab[] = {
  SANE_I18N ("Default"),
  SANE_I18N ("User defined (Gamma=1.0)"),
  SANE_I18N ("User defined (Gamma=1.8)"),
  SANE_I18N ("High density printing"),
  SANE_I18N ("Low density printing"),
  SANE_I18N ("High contrast printing"),
  NULL
};

static SANE_Bool gamma_userdefined_ab[] = {
  SANE_FALSE,
  SANE_TRUE,
  SANE_TRUE,
  SANE_FALSE,
  SANE_FALSE,
  SANE_FALSE,
};

static int gamma_params_d[] = { 
  0x03, 
  0x04
};

static const SANE_String_Const gamma_list_d[] = { 
  SANE_I18N ("User defined (Gamma=1.0)"),
  SANE_I18N ("User defined (Gamma=1.8)"), 
  NULL
};

static SANE_Bool gamma_userdefined_d[] = {
  SANE_TRUE,
  SANE_TRUE
};

static SANE_Bool *gamma_userdefined;
static int *gamma_params;


/* Bay list:
 * this is used for the FilmScan
 */

static const SANE_String_Const bay_list[] = { 
  " 1 ", 
  " 2 ", 
  " 3 ", 
  " 4 ", 
  " 5 ", 
  " 6 ", 
  NULL
};

/*
 *  minimum, maximum, quantization.
 */

static const SANE_Range u8_range = { 0, 255, 0 };
static const SANE_Range s8_range = { -127, 127, 0 };
static const SANE_Range zoom_range = { 50, 200, 0 };

/*
 * The "switch_params" are used for several boolean choices
 */
static int switch_params[] = {
  0,
  1
};

#define  mirror_params  switch_params
#define  speed_params	switch_params
#define  film_params	switch_params

static const SANE_Range outline_emphasis_range = { -2, 2, 0 };

/* static const SANE_Range gamma_range = { -2, 2, 0 }; */

struct qf_param
{
  SANE_Word tl_x;
  SANE_Word tl_y;
  SANE_Word br_x;
  SANE_Word br_y;
};

/* gcc don't like to overwrite const field */
static /*const */ struct qf_param qf_params[] = { 
  {0, 0, SANE_FIX (120.0), SANE_FIX (120.0)},
  {0, 0, SANE_FIX (148.5), SANE_FIX (210.0)},
  {0, 0, SANE_FIX (210.0), SANE_FIX (148.5)},
  {0, 0, SANE_FIX (215.9), SANE_FIX (279.4)},	/* 8.5" x 11" */
  {0, 0, SANE_FIX (210.0), SANE_FIX (297.0)},
  {0, 0, 0, 0}
};

static const SANE_String_Const qf_list[] = { 
  SANE_I18N ("CD"), 
  SANE_I18N ("A5 portrait"), 
  SANE_I18N ("A5 landscape"),
  SANE_I18N ("Letter"), 
  SANE_I18N ("A4"), 
  SANE_I18N ("Max"), 
  NULL
};


static SANE_Word *bitDepthList = NULL;



/*
 * List of pointers to devices - will be dynamically allocated depending
 * on the number of devices found. 
 */
static const SANE_Device **devlist = 0;


/*
 * Some utility functions
 */

/*! Returns the larger of the arguments \a a and \a b.

  \note
  Rather than use a macro that needs to evaluate \a a and \a b twice,
  and is therefore unsafe, we try to inline this function where this
  is possible.
 */
static inline
int
max (int a, int b)
{
  return (a < b ? b : a);
}

/*! Returns the smaller of the arguments \a a and \a b.

  \note
  Rather than use a macro that needs to evaluate \a a and \a b twice,
  and is therefore unsafe, we try to inline this function where this
  is possible.
 */
static inline
int
min (int a, int b)
{
  return (a < b ? a : b);
}


static size_t
max_string_size (const SANE_String_Const strings[])
{
  size_t size, max_size = 0;
  int i;

  for (i = 0; strings[i]; i++)
  {
    size = strlen (strings[i]) + 1;
    if (size > max_size)
      max_size = size;
  }
  return max_size;
}

typedef struct
{
  u_char code;
  u_char status;
  u_char count1;
  u_char count2;
  u_char buf[1];

} EpsonHdrRec, *EpsonHdr;

typedef struct
{
  u_char code;
  u_char status;
  u_char count1;
  u_char count2;

  u_char type;
  u_char level;

  u_char buf[1];

} EpsonIdentRec, *EpsonIdent;

typedef struct
{
  u_char code;
  u_char status;

  u_char buf[4];

} EpsonDataRec, *EpsonData;

/*
 *
 *
 */

static EpsonHdr command (Epson_Scanner * s, u_char * cmd, size_t cmd_size,
			 SANE_Status * status);
static SANE_Status get_identity_information (SANE_Handle handle);
static SANE_Status get_identity2_information (SANE_Handle handle);
static int send (Epson_Scanner * s, void *buf, size_t buf_size,
		 SANE_Status * status);
static ssize_t receive (Epson_Scanner * s, void *buf, ssize_t buf_size,
			SANE_Status * status);
static SANE_Status color_shuffle (SANE_Handle handle, int *new_length);
#if 0
static SANE_Status request_focus_position (SANE_Handle handle,
					   u_char * position);
#endif
static SANE_Status request_push_button_status (SANE_Handle handle,
					       SANE_Bool * theButtonStatus);
static void activateOption (Epson_Scanner * s, SANE_Int option,
			    SANE_Bool * change);
static void deactivateOption (Epson_Scanner * s, SANE_Int option,
			      SANE_Bool * change);
static void setOptionState (Epson_Scanner * s, SANE_Bool state,
			    SANE_Int option, SANE_Bool * change);
static void close_scanner (Epson_Scanner * s);
static SANE_Status open_scanner (Epson_Scanner * s);
SANE_Status sane_auto_eject (Epson_Scanner * s);
static SANE_Status attach_one_usb (SANE_String_Const devname);
static void filter_resolution_list (Epson_Scanner * s);
static void get_size (char c1, char c2, double *w, double *h);
static void scan_finish (Epson_Scanner * s);

static void get_colorcoeff_from_profile (double *profile,
					 unsigned char *color_coeff);
static void round_cct (double org_cct[], int rnd_cct[]);
static int get_roundup_index (double frac[], int n);
static int get_rounddown_index (double frac[], int n);

static void color_correct (SANE_Byte * data, SANE_Int size, double *profile);

static void handle_mode (Epson_Scanner * s, SANE_Int optindex,
			 SANE_Bool * reload);
static void change_profile_matrix (Epson_Scanner * s);
static void handle_depth_halftone (Epson_Scanner * s, SANE_Int optindex,
				   SANE_Bool * reload);

static SANE_Status check_warmup (Epson_Scanner * s);
static SANE_Status check_ext_status (Epson_Scanner * s, int *max_x,
				     int *max_y);
static const char *second_guess_model_name (Epson_Scanner * s,
					    const char *fw_name);

static void check_control_extension (Epson_Scanner * s, SANE_Status * status);

/*
 *
 *
 */


static int
send (Epson_Scanner * s, void *buf, size_t buf_size, SANE_Status * status)
{
  DBG (30, "send buf, size = %lu\n", (u_long) buf_size);

#if 1
  {
    unsigned int k;
    const u_char *s = buf;

    for (k = 0; k < buf_size; k++)
    {
      DBG (125, "buf[%d] %02x %c\n", k, s[k], isprint (s[k]) ? s[k] : '.');
    }
  }
#endif

  if (s->hw->interpreter)		/* takes care of own communication */
  {
    return s->hw->interpreter->send (s->hw, buf, buf_size, status);
  }

  if (s->hw->connection == SANE_EPSON_SCSI)
  {
    return sanei_epson_scsi_write (s->hw->fd, buf, buf_size, status);
  }
  else if (s->hw->connection == SANE_EPSON_PIO)
  {
    size_t n;

    if (buf_size == (n = sanei_pio_write (s->hw->fd, buf, buf_size)))
      *status = SANE_STATUS_GOOD;
    else
      *status = SANE_STATUS_INVAL;

    return n;
  }
  else if (s->hw->connection == SANE_EPSON_USB)
  {
    {
      size_t n;
      n = buf_size;
      *status = sanei_usb_write_bulk (s->hw->fd, buf, &n);
      return n;
    }
  }

  return SANE_STATUS_INVAL;
  /* never reached */
}

/*
 *
 *
 */

static ssize_t
receive (Epson_Scanner * s, void *buf, ssize_t buf_size, SANE_Status * status)
{
  ssize_t n = 0;

  if (s->hw->interpreter)		/* takes care of own communication */
  {
    return s->hw->interpreter->recv (s->hw, buf, buf_size, status);
  }

  if (s->hw->connection == SANE_EPSON_SCSI)
  {
    n = sanei_epson_scsi_read (s->hw->fd, buf, buf_size, status);
  }
  else if (s->hw->connection == SANE_EPSON_PIO)
  {
    if (buf_size == (n = sanei_pio_read (s->hw->fd, buf, (size_t) buf_size)))
      *status = SANE_STATUS_GOOD;
    else
      *status = SANE_STATUS_INVAL;
  }
  else if (s->hw->connection == SANE_EPSON_USB)
  {
    {
      /* !!! only report an error if we don't read anything */
      n = buf_size;		/* buf_size gets overwritten */
      *status =
	sanei_usb_read_bulk (s->hw->fd, (SANE_Byte *) buf, (size_t *) & n);
      if (n > 0)
	*status = SANE_STATUS_GOOD;
    }
  }

  DBG (7, "receive buf, expected = %lu, got = %ld\n", (u_long) buf_size, (long) n);

#if 1
  if (n > 0)
  {
    int k;
    const u_char *s = buf;

    for (k = 0; k < n; k++)
    {
      DBG (127, "buf[%d] %02x %c\n", k, s[k], isprint (s[k]) ? s[k] : '.');
    }
  }
#else
  {
    int i;
    ssize_t k;
    ssize_t hex_start = 0;
    const u_char *s = buf;
    char hex_str[NUM_OF_HEX_ELEMENTS * 3 + 1];
    char tmp_str[NUM_OF_HEX_ELEMENTS * 3 + 1];
    char ascii_str[NUM_OF_HEX_ELEMENTS * 2 + 1];

    hex_str[0] = '\0';
    ascii_str[0] = '\0';

    for (k = 0; k < buf_size; k++)
    {
      /* write out the data in lines of 16 bytes */
      /* add the next hex value to the hex string */
      sprintf (tmp_str, "%s %02x", hex_str, s[k]);
      strcpy (hex_str, tmp_str);

      /* add the character to the ascii string */
      sprintf (tmp_str, "%s %c", ascii_str, isprint (s[k]) ? s[k] : '.');
      strcpy (ascii_str, tmp_str);

      if ((k % (NUM_OF_HEX_ELEMENTS)) == 0)
      {
	if (k != 0)		/* don't do this the first time */
	{
	  for (i = strlen (hex_str); i < (NUM_OF_HEX_ELEMENTS * 3); i++)
	  {
	    hex_str[i] = ' ';
	  }
	  hex_str[NUM_OF_HEX_ELEMENTS + 1] = '\0';

	  DBG (125, "recv buf[%05d]: %s   %s\n", hex_start, hex_str,
	       ascii_str);
	  hex_start = k;
	  hex_str[0] = '\0';
	  ascii_str[0] = '\0';
	}
      }
    }

    for (i = strlen (hex_str); i < NUM_OF_HEX_ELEMENTS * 3; i++)
    {
      hex_str[i] = ' ';
    }
    hex_str[NUM_OF_HEX_ELEMENTS + 1] = '\0';

    DBG (125, "recv buf[%05d]: %s   %s\n", hex_start, hex_str, ascii_str);
  }
#endif

  return n;
}

/*
 *
 *
 */

static SANE_Status
expect_ack (Epson_Scanner * s)
{
  u_char result[1];
  size_t len;
  SANE_Status status;

  len = sizeof (result);

  receive (s, result, len, &status);

  if (SANE_STATUS_GOOD != status)
    return status;

  if (ACK != result[0])
    return SANE_STATUS_INVAL;

  return SANE_STATUS_GOOD;
}

/*
 *
 *
 */

static SANE_Status
set_cmd (Epson_Scanner * s, u_char cmd, int val)
{
  SANE_Status status;
  u_char params[2];

  if (!cmd)
    return SANE_STATUS_UNSUPPORTED;

  params[0] = ESC;
  params[1] = cmd;

  send (s, params, 2, &status);
  if (SANE_STATUS_GOOD != (status = expect_ack (s)))
    return status;

  params[0] = val;
  send (s, params, 1, &status);
  status = expect_ack (s);

  return status;
}

/* A little helper function to correct the extended status reply
   gotten from scanners with known buggy firmware.
 */
static void
fix_up_extended_status_reply (const char *model, u_char * buf)
{

  if (0 == strncmp (model, "ES-9000H", strlen ("ES-9000H"))
      || 0 == strncmp (model, "GT-30000", strlen ("GT-30000")))
  {
    DBG (1, "Fixing up buggy ADF max scan dimensions.\n");
    buf[2] = 0xB0;
    buf[3] = 0x6D;
    buf[4] = 0x60;
    buf[5] = 0x9F;
  }
}

static void
print_params (const SANE_Parameters params)
{
  DBG (5, "params.format = %d\n", params.format);
  DBG (5, "params.last_frame = %d\n", params.last_frame);
  DBG (5, "params.bytes_per_line = %d\n", params.bytes_per_line);
  DBG (5, "params.pixels_per_line = %d\n", params.pixels_per_line);
  DBG (5, "params.lines = %d\n", params.lines);
  DBG (5, "params.depth = %d\n", params.depth);
}


/*
 *
 *
 */

#define  set_focus_position(s,v)        set_cmd( s,(s)->hw->cmd->set_focus_position,v)
#define  set_color_mode(s,v)		set_cmd( s,(s)->hw->cmd->set_color_mode,v)
#define  set_data_format(s,v)		set_cmd( s,(s)->hw->cmd->set_data_format, v)
#define  set_halftoning(s,v)		set_cmd( s,(s)->hw->cmd->set_halftoning, v)
#define  set_gamma(s,v)			set_cmd( s,(s)->hw->cmd->set_gamma, v)
#define  set_color_correction(s,v)	set_cmd( s,(s)->hw->cmd->set_color_correction, v)
#define  set_lcount(s,v)		set_cmd( s,(s)->hw->cmd->set_lcount, v)
#define  set_bright(s,v)		set_cmd( s,(s)->hw->cmd->set_bright, v)
#define  mirror_image(s,v)		set_cmd( s,(s)->hw->cmd->mirror_image, v)
#define  set_speed(s,v)			set_cmd( s,(s)->hw->cmd->set_speed, v)
#define  set_outline_emphasis(s,v)	set_cmd( s,(s)->hw->cmd->set_outline_emphasis, v)
#define  control_auto_area_segmentation(s,v)	set_cmd( s,(s)->hw->cmd->control_auto_area_segmentation, v)
#define  set_film_type(s,v)		set_cmd( s,(s)->hw->cmd->set_film_type, v)
#define  set_exposure_time(s,v)		set_cmd( s,(s)->hw->cmd->set_exposure_time, v)
#define  set_bay(s,v)			set_cmd( s,(s)->hw->cmd->set_bay, v)
#define  set_threshold(s,v)		set_cmd( s,(s)->hw->cmd->set_threshold, v)
#define  control_extension(s,v)		set_cmd( s,(s)->hw->cmd->control_an_extension, v)

/*#define  (s,v)		set_cmd( s,(s)->hw->cmd->, v) */

static SANE_Status
set_zoom (Epson_Scanner * s, int x_zoom, int y_zoom)
{
  SANE_Status status;
  u_char cmd[2];
  u_char params[2];

  if (!s->hw->cmd->set_zoom)
    return SANE_STATUS_GOOD;

  cmd[0] = ESC;
  cmd[1] = s->hw->cmd->set_zoom;

  send (s, cmd, 2, &status);
  status = expect_ack (s);

  if (status != SANE_STATUS_GOOD)
    return status;

  params[0] = x_zoom;
  params[1] = y_zoom;

  send (s, params, 2, &status);
  status = expect_ack (s);

  return status;
}


static SANE_Status
set_resolution (Epson_Scanner * s, int xres, int yres)
{
  SANE_Status status;
  u_char params[4];

  if (!s->hw->cmd->set_resolution)
    return SANE_STATUS_GOOD;

  params[0] = ESC;
  params[1] = s->hw->cmd->set_resolution;

  send (s, params, 2, &status);
  status = expect_ack (s);

  if (status != SANE_STATUS_GOOD)
    return status;

  params[0] = xres;
  params[1] = xres >> 8;
  params[2] = yres;
  params[3] = yres >> 8;

  send (s, params, 4, &status);
  status = expect_ack (s);

  return status;
}

/*
 * set_scan_area() 
 *
 * Sends the "set scan area" command to the scanner with the currently selected 
 * scan area. This scan area is already corrected for "color shuffling" if 
 * necessary.
 */
static SANE_Status
set_scan_area (Epson_Scanner * s, int x, int y, int width, int height)
{
  SANE_Status status;
  u_char params[8];

  DBG (1, "set_scan_area: %p %d %d %d %d\n", (void *) s, x, y, width, height);

  if (!s->hw->cmd->set_scan_area)
  {
    DBG (1, "set_scan_area not supported\n");
    return SANE_STATUS_GOOD;
  }

  /* verify the scan area */
  if (x < 0 || y < 0 || width <= 0 || height <= 0)
    return SANE_STATUS_INVAL;

  params[0] = ESC;
  params[1] = s->hw->cmd->set_scan_area;

  send (s, params, 2, &status);
  status = expect_ack (s);
  if (status != SANE_STATUS_GOOD)
    return status;

  params[0] = x;
  params[1] = x >> 8;
  params[2] = y;
  params[3] = y >> 8;
  params[4] = width;
  params[5] = width >> 8;
  params[6] = height;
  params[7] = height >> 8;

  send (s, params, 8, &status);
  status = expect_ack (s);

  return status;
}

/*
 * set_color_correction_coefficients()
 *
 * Sends the "set color correction coefficients" command with the currently selected
 * parameters to the scanner.
 */

static SANE_Status
set_color_correction_coefficients (Epson_Scanner * s)
{
  SANE_Status status;
  u_char cmd = s->hw->cmd->set_color_correction_coefficients;
  u_char params[2];
  const int length = 9;
  unsigned char cccoeff[9];
  double cct[9];

  DBG (1, "set_color_correction_coefficients: starting.\n");
  if (!cmd)
    return SANE_STATUS_UNSUPPORTED;

  params[0] = ESC;
  params[1] = cmd;

  send (s, params, 2, &status);
  if (SANE_STATUS_GOOD != (status = expect_ack (s)))
    return status;

  cct[0] = SANE_UNFIX (s->val[OPT_CCT_1].w);
  cct[1] = SANE_UNFIX (s->val[OPT_CCT_2].w);
  cct[2] = SANE_UNFIX (s->val[OPT_CCT_3].w);
  cct[3] = SANE_UNFIX (s->val[OPT_CCT_4].w);
  cct[4] = SANE_UNFIX (s->val[OPT_CCT_5].w);
  cct[5] = SANE_UNFIX (s->val[OPT_CCT_6].w);
  cct[6] = SANE_UNFIX (s->val[OPT_CCT_7].w);
  cct[7] = SANE_UNFIX (s->val[OPT_CCT_8].w);
  cct[8] = SANE_UNFIX (s->val[OPT_CCT_9].w);

  get_colorcoeff_from_profile (cct, cccoeff);

  DBG (1, "set_color_correction_coefficients: %d,%d,%d %d,%d,%d %d,%d,%d.\n",
       cccoeff[0], cccoeff[1], cccoeff[2], cccoeff[3],
       cccoeff[4], cccoeff[5], cccoeff[6], cccoeff[7], cccoeff[8]);

  send (s, cccoeff, length, &status);
  status = expect_ack (s);
  DBG (1, "set_color_correction_coefficients: ending=%d.\n", status);

  return status;
}

/*
 *
 *
 */

static SANE_Status
set_gamma_table (Epson_Scanner * s)
{

  SANE_Status status;
  u_char cmd = s->hw->cmd->set_gamma_table;
  u_char params[2];
  const int length = 257;
  u_char gamma[257];
  int n;
  int table;
/*	static const char gamma_cmds[] = { 'M', 'R', 'G', 'B' }; */
  static char gamma_cmds[] = { 'R', 'G', 'B' };


  DBG (1, "set_gamma_table: starting.\n");
  if (!cmd)
    return SANE_STATUS_UNSUPPORTED;

  params[0] = ESC;
  params[1] = cmd;

/*
 *	Print the gamma tables before sending them to the scanner.
 */

  if (DBG_LEVEL > 0)
  {
    int c, i, j;

    DBG (1, "set_gamma_table()\n");
    for (c = 0; c < 3; c++)
    {
      for (i = 0; i < 256; i += 16)
      {
	char gammaValues[16 * 3 + 1], newValue[4];

	gammaValues[0] = '\0';

	for (j = 0; j < 16; j++)
	{
	  sprintf (newValue, " %02x", s->gamma_table[c][i + j]);
	  strcat (gammaValues, newValue);
	}

	DBG (10, "Gamma Table[%d][%d] %s\n", c, i, gammaValues);
      }
    }
  }


/*
 * TODO: &status in send makes no sense like that.
 */

/*
 *  When handling inverted images, we must also invert the user
 *  supplied gamma function.  This is *not* just 255-gamma -
 *  this gives a negative image.
 */

  if (strcmp ("GT-6600", s->hw->sane.model) == 0 ||
      strcmp ("Perfection 610", s->hw->sane.model) == 0)
  {
    gamma_cmds[0] = 'R';
    gamma_cmds[1] = 'B';
    gamma_cmds[2] = 'G';
  }

  for (table = 0; table < 3; table++)
  {
    gamma[0] = gamma_cmds[table];
    if (s->invert_image)
    {
      for (n = 0; n < 256; ++n)
      {
	gamma[n + 1] = 255 - s->gamma_table[table][255 - n];
      }
    }
    else
    {
      for (n = 0; n < 256; ++n)
      {
	gamma[n + 1] = s->gamma_table[table][n];
      }
    }

    send (s, params, 2, &status);
    if (SANE_STATUS_GOOD != (status = expect_ack (s)))
      return status;

    send (s, gamma, length, &status);
    if (SANE_STATUS_GOOD != (status = expect_ack (s)))
      return status;

  }

  DBG (1, "set_gamma_table: complete = %d.\n", status);

  return status;
}

static SANE_Status
check_warmup (Epson_Scanner * s)
{
  SANE_Status status;
  int x, y;

  status = check_ext_status (s, &x, &y);

  if (status == SANE_STATUS_DEVICE_BUSY)
  {
    int timeout;

    for (timeout = 0; timeout < 60; timeout++)
    {
      status = check_ext_status (s, &x, &y);

      if (status == SANE_STATUS_DEVICE_BUSY)
	sleep (1);
      else
      {
	return status;
      }
    }
  }
  else
    return status;

  return status;
}

void
get_size (char c1, char c2, double *w, double *h)
{
  int ind;
  unsigned char flag;

  double wsizetbl[] = {
    11.60,			/* A3V */
    11.00,			/* WLT */
    10.12,			/* B4V */
    8.50,			/* LGV */
    8.27,			/* A4V */
    11.69,			/* A4H */
    8.50,			/* LTV */
    11.00,			/* LTH */
    7.17,			/* B5V */
    10.12,			/* B5H */
    5.83,			/* A5V */
    8.27,			/* A5H */
    7.25,			/* EXV */
    10.50,			/* EXH */
    11.69,			/* unknown */
    11.69,			/* unknown */
    11.69,			/* unknown */
  };
  double hsizetbl[] = {
    16.54,			/* A3V */
    17.00,			/* WLT */
    14.33,			/* B4V */
    14.00,			/* LGV */
    11.69,			/* A4V */
    8.27,			/* A4H */
    11.00,			/* LTV */
    8.50,			/* LTH */
    10.12,			/* B5V */
    7.17,			/* B5H */
    8.27,			/* A5V */
    5.83,			/* A5H */
    10.50,			/* EXV */
    7.25,			/* EXH */
    17.00,			/* unknown */
    17.00,			/* unknown */
    17.00,			/* unknown */
  };

  flag = c1;
  for (ind = 0; ind < 8; ind++)
  {
    if (flag & 0x80)
      goto DetectSize;
    flag = flag << 1;
  }
  flag = c2;
  for (; ind < 16; ind++)
  {
    if (flag & 0x80)
      goto DetectSize;
    flag = flag << 1;
  }

DetectSize:

  *w = wsizetbl[ind];
  *h = hsizetbl[ind];

  DBG (10, "detected width: %f\n", *w);
  DBG (10, "detected height: %f\n", *h);
}


/*
 * check_ext_status()
 *
 * Requests the extended status flag from the scanner. The "warming up" condition
 * is reported as a warning (only visible if debug level is set to 10 or greater) -
 * every other condition is reported as an error.
 *
 * This function only gets called when we are dealing with a scanner that supports the 
 * "warming up" code, so it's not a problem for B3 level scanners, that don't handle
 * request extended status commands.
 */

static SANE_Status
check_ext_status (Epson_Scanner * s, int *max_x, int *max_y)
{
  SANE_Status status;
  u_char cmd = s->hw->cmd->request_extended_status;
  u_char params[2];
  u_char *buf;
  EpsonHdr head;

  *max_x = 0;
  *max_y = 0;

  if (cmd == 0)
    return SANE_STATUS_UNSUPPORTED;

  params[0] = ESC;
  params[1] = cmd;

  head = (EpsonHdr) command (s, params, 2, &status);
  if (NULL == head)
  {
    DBG (1, "Extended status flag request failed\n");
    return status;
  }

  buf = &head->buf[0];

  if (buf[0] & EXT_STATUS_WU)
  {
    DBG (10, "option: warming up\n");
    status = SANE_STATUS_DEVICE_BUSY;
  }

  if (buf[0] & EXT_STATUS_FER)
  {
    DBG (1, "option: fatal error\n");
    status = SANE_STATUS_INVAL;
  }

  if (s->hw->ADF && s->hw->use_extension && s->hw->cmd->feed)
  {
    fix_up_extended_status_reply (s->hw->sane.model, buf);

    *max_x = buf[3] << 8 | buf[2];
    *max_y = buf[5] << 8 | buf[4];

    if (0 == strcmp ("ES-9000H", s->hw->sane.model)
	|| 0 == strcmp ("GT-30000", s->hw->sane.model)
	|| 0 == strcmp ("ES-7000H", s->hw->sane.model)
	|| 0 == strcmp ("LP-M5500", s->hw->sane.model)
	|| 0 == strcmp ("GT-15000", s->hw->sane.model)
	|| 0 == strcmp ("ES-10000G", s->hw->sane.model)
	|| 0 == strcmp ("Expression 10000XL", s->hw->sane.model))
    {
      /* set size of current sheet, but don't clobber zoom
         settings (which should always be smaller than the
         detected sheet size), if any */
      double doctype_br_x, doctype_br_y;
      double         br_x,         br_y;

      const SANE_Int doctype = buf[17] << 8 | buf[16];

      get_size (s->hw->doctype & 0xFF, (s->hw->doctype & 0xFF00) >> 8,
		&doctype_br_x, &doctype_br_y);
      doctype_br_x = SANE_FIX (doctype_br_x * MM_PER_INCH);
      doctype_br_y = SANE_FIX (doctype_br_y * MM_PER_INCH);

      get_size (doctype & 0xFF, (doctype & 0xFF00) >> 8, &br_x, &br_y);
      br_x = SANE_FIX (br_x * MM_PER_INCH);
      br_y = SANE_FIX (br_y * MM_PER_INCH);

      if (doctype != s->hw->doctype
	  && s->val[OPT_BR_X].w == doctype_br_x
	  && s->val[OPT_BR_Y].w == doctype_br_y)
      {				/* no zoom in effect */
	  s->val[OPT_BR_X].w = br_x;
	  s->val[OPT_BR_Y].w = br_y;
      }

      if (br_x < s->val[OPT_BR_X].w)
        s->val[OPT_BR_X].w = br_x;
      if (br_y < s->val[OPT_BR_Y].w)
        s->val[OPT_BR_Y].w = br_y;

      s->hw->doctype = doctype;
    }
  }


  if (buf[1] & EXT_STATUS_ERR)
  {
    DBG (1, "ADF: other error\n");
    status = SANE_STATUS_INVAL;
  }

  if (buf[1] & EXT_STATUS_PE)
  {
    DBG (1, "ADF: no paper\n");
    status = SANE_STATUS_NO_DOCS;
  }

  if (buf[1] & EXT_STATUS_PJ)
  {
    DBG (1, "ADF: paper jam\n");
    status = SANE_STATUS_JAMMED;
  }

  if (buf[1] & EXT_STATUS_OPN)
  {
    DBG (1, "ADF: cover open\n");
    status = SANE_STATUS_COVER_OPEN;
  }

  if (buf[6] & EXT_STATUS_ERR)
  {
    DBG (1, "TPU: other error\n");
    status = SANE_STATUS_INVAL;
  }

  /* ??? is this for MFDs? */
  if (buf[11] & EXT_STATUS_OPN)
  {
    DBG (1, "UNIT: Scanner Unit open\n");
    status = SANE_STATUS_COVER_OPEN;
  }

  /* return the max. scan area for the ADF */
  if (buf[6] & EXT_STATUS_IST)
  {
    *max_x = buf[8] << 8 | buf[7];
    *max_y = buf[10] << 8 | buf[9];
  }

  /* return the max. scan area for the flatbed */
  if (s->hw->devtype == 3 && s->hw->use_extension == 0)
  {
    double w, h;
    get_size (buf[18], buf[19], &w, &h);
    *max_x = (int) (w * s->hw->dpi_range.max);
    *max_y = (int) (h * s->hw->dpi_range.max);
  }

  delete (head);

  return status;
}

/*
 * reset()
 *
 * Send the "initialize scanner" command to the device and reset it.
 *
 */

static SANE_Status
reset (Epson_Scanner * s)
{
  SANE_Status status;
  u_char param[2];
  SANE_Bool needToClose = SANE_FALSE;

  DBG (5, "reset()\n");

  if (!s->hw->cmd->initialize_scanner)
    return SANE_STATUS_GOOD;

  param[0] = ESC;
  param[1] = s->hw->cmd->initialize_scanner;

  if (s->hw->fd == -1)
  {
    needToClose = SANE_TRUE;
    DBG (5, "reset calling open_scanner\n");
    if ((status = open_scanner (s)) != SANE_STATUS_GOOD)
      return status;
  }

  send (s, param, 2, &status);
  status = expect_ack (s);

  if (needToClose)
  {
    close_scanner (s);
  }

  return status;
}


/*! Closes a file descriptor associated with a scanner device.
 *
 *  Closes the communication channel between the scanner handle and
 *  the physical device.  The file descriptor is guaranteed to be set
 *  to an invalid file descriptor upon completion.  This function is a
 *  no-op in case the channel is not open.
 */

static void
close_scanner (Epson_Scanner * s)
{
  DBG (5, "close_scanner(fd = %d)\n", s->hw->fd);

  if (s->hw->fd < 0)
    return;

  if (s->hw->ADF && s->hw->use_extension && s->hw->cmd->feed
      && s->val[OPT_AUTO_EJECT].w && s->adf_has_been_fed)
  {
    /* emulate sane_auto_eject() because it calls
       close_scanner() which would lead to a possibility of
       infinite recursion if we were to call sane_auto_eject
       here directly */
    SANE_Status status;
    u_char params[1];
    u_char cmd = s->hw->cmd->eject;
    /* re-enable the option unit, just in case */
    control_extension (s, s->hw->use_extension);

    if (0 == strcmp ("ES-10000G", s->hw->sane.model)
	|| 0 == strcmp ("Expression 10000XL", s->hw->sane.model))
      check_control_extension (s, &status);

    if (cmd)
    {
      params[0] = cmd;
      send (s, params, 1, &status);
      status = expect_ack (s);
    }
    s->adf_has_been_fed = SANE_FALSE;
  }

  if (s->hw->interpreter)
  {
    s->hw->interpreter->close (s->hw);
  }

  if (s->hw->connection == SANE_EPSON_SCSI)
  {
    sanei_scsi_close (s->hw->fd);
  }
  else if (s->hw->connection == SANE_EPSON_PIO)
  {
    sanei_pio_close (s->hw->fd);
  }
  else if (s->hw->connection == SANE_EPSON_USB)
  {
    sanei_usb_close (s->hw->fd);
  }

  s->hw->fd = -1;
  return;
}

/*
 * open_scanner()
 *
 * Open the scanner device. Depending on the connection method, 
 * different open functions are called. 
 */

static SANE_Status
open_scanner (Epson_Scanner * s)
{
  SANE_Status status = 0;

  DBG (5, "open_scanner()\n");

  if (s->hw->fd != -1)
  {
    DBG (5, "scanner is already open: fd = %d\n", s->hw->fd);
    return SANE_STATUS_GOOD;	/* no need to open the scanner */
  }

  /* don't do this for OS2: */
#ifndef HAVE_OS2_H
#if 0
  /* test the device name */
  if ((s->hw->connection != SANE_EPSON_PIO)
      && (access (s->hw->sane.name, R_OK | W_OK) != 0))
  {
    DBG (1, "sane_start: access(%s, R_OK | W_OK) failed\n", s->hw->sane.name);
    return SANE_STATUS_ACCESS_DENIED;
  }
#endif
#endif


  if (s->hw->interpreter)
  {
    s->hw->interpreter->open (s->hw);
  }

  if (s->hw->connection == SANE_EPSON_SCSI)
  {
    status = sanei_scsi_open (s->hw->sane.name, &s->hw->fd,
			      sanei_epson_scsi_sense_handler, NULL);
    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: %s open failed: %s\n", s->hw->sane.name,
	   sane_strstatus (status));
      return status;
    }
  }
  else if (s->hw->connection == SANE_EPSON_PIO)
  {
    status = sanei_pio_open (s->hw->sane.name, &s->hw->fd);
    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: %s open failed: %s\n", s->hw->sane.name,
	   sane_strstatus (status));
      return status;
    }
  }
  else if (s->hw->connection == SANE_EPSON_USB)
  {
    status = sanei_usb_open (s->hw->sane.name, &s->hw->fd);

    return status;
  }

  return SANE_STATUS_GOOD;
}

/*
 * feed ( )
 */

static SANE_Status
feed (Epson_Scanner * s)
{
  SANE_Status status;
  u_char params[2];
  u_char cmd = s->hw->cmd->feed;

  DBG (5, "feed()\n");

  if (!cmd)
  {
    DBG (5, "feed() is not supported\n");
    return SANE_STATUS_UNSUPPORTED;
  }

  params[0] = cmd;

  send (s, params, 1, &status);

  if (SANE_STATUS_GOOD != (status = expect_ack (s)))
  {
    close_scanner (s);
    return status;
  }

  return status;
}


/*
 * eject()
 * 
 * Eject the current page from the ADF. The scanner is opened prior to
 * sending the command and closed afterwards.
 *
 */

static SANE_Status
eject (Epson_Scanner * s)
{
  SANE_Status status;
  u_char params[2];
  u_char cmd = s->hw->cmd->eject;
  SANE_Bool needToClose = SANE_FALSE;

  DBG (5, "eject()\n");

  if (!cmd)
    return SANE_STATUS_UNSUPPORTED;

  if (s->hw->fd == -1)
  {
    needToClose = SANE_TRUE;
    if (SANE_STATUS_GOOD != (status = open_scanner (s)))
      return status;
  }

  params[0] = cmd;

  send (s, params, 1, &status);

  if (SANE_STATUS_GOOD != (status = expect_ack (s)))
  {
    close_scanner (s);
    return status;
  }

  if (needToClose)
    close_scanner (s);

  return status;
}

/*
 *
 *
 */

static int num_devices = 0;	/* number of EPSON scanners attached to backend */
static Epson_Device *first_dev = NULL;	/* first EPSON scanner in list */
static Epson_Scanner *first_handle = NULL;


static EpsonHdr
command (Epson_Scanner * s, u_char * cmd, size_t cmd_size,
	 SANE_Status * status)
{
  EpsonHdr head;
  u_char *buf;
  int count;

  if (NULL == (head = walloc (EpsonHdrRec)))
  {
    DBG (1, "out of memory (line %d)\n", __LINE__);
    *status = SANE_STATUS_NO_MEM;
    return (EpsonHdr) 0;
  }

  send (s, cmd, cmd_size, status);

  if (SANE_STATUS_GOOD != *status)
  {
    /* this is necessary for the GT-8000. I don't know why, but
       it seems to fix the problem. It should not have any
       ill effects on other scanners.  */
    *status = SANE_STATUS_GOOD;
    send (s, cmd, cmd_size, status);
    if (SANE_STATUS_GOOD != *status)
      return (EpsonHdr) 0;
  }

  buf = (u_char *) head;

  if (s->hw->connection == SANE_EPSON_SCSI)
  {
    receive (s, buf, 4, status);
    buf += 4;
  }
  else if (s->hw->connection == SANE_EPSON_USB)
  {
    int bytes_read;
    bytes_read = receive (s, buf, 4, status);
    buf += bytes_read;
  }
  else
  {
    receive (s, buf, 1, status);
    buf += 1;
  }

  if (SANE_STATUS_GOOD != *status)
  {
    delete (head);
    return (EpsonHdr) 0;
  }

  DBG (4, "code   %02x\n", (int) head->code);

  switch (head->code)
  {

  case NAK:
    /* fall through */
    /* !!! is this really sufficient to report an error ? */
  case ACK:
    break;			/* no need to read any more data after ACK or NAK */

  case STX:
    if (s->hw->connection == SANE_EPSON_SCSI)
    {
      /* nope */
    }
    else if (s->hw->connection == SANE_EPSON_USB)
    {
      /* we've already read the complete data */
    }
    else
    {
      receive (s, buf, 3, status);
      /*              buf += 3; */
    }

    if (SANE_STATUS_GOOD != *status)
    {
      delete (head);
      return (EpsonHdr) 0;
    }

    DBG (4, "status %02x\n", (int) head->status);

    count = head->count2 * 255 + head->count1;
    DBG (4, "count  %d\n", count);

    if (NULL == (head = realloc (head, sizeof (EpsonHdrRec) + count)))
    {
      DBG (1, "out of memory (line %d)\n", __LINE__);
      *status = SANE_STATUS_NO_MEM;
      return (EpsonHdr) 0;
    }

    buf = head->buf;
    receive (s, buf, count, status);

    if (SANE_STATUS_GOOD != *status)
    {
      delete (head);
      return (EpsonHdr) 0;
    }

    break;

  default:
    if (0 == head->code)
      DBG (1, "Incompatible printer port (probably bi/directional)\n");
    else if (cmd[cmd_size - 1] == head->code)
      DBG (1, "Incompatible printer port (probably not bi/directional)\n");

    DBG (2, "Illegal response of scanner for command: %02x\n", head->code);
    break;
  }

  return head;
}


/*
 * static SANE_Status attach()
 *
 * Attach one device with name *dev_name to the backend.
 */

static SANE_Status
attach (const char *dev_name, Epson_Device * *devp, int type)
{
  SANE_Status status;
  Epson_Scanner *s = walloca (Epson_Scanner);
  char *str;
  struct Epson_Device *dev;
  SANE_String_Const *source_list_add = source_list;
  int port;

  DBG (1, "%s\n", SANE_EPSON_VERSION);

  DBG (5, "attach(%s, %d)\n", dev_name, type);

  for (dev = first_dev; dev; dev = dev->next)
  {
    if (strcmp (dev->sane.name, dev_name) == 0)
    {
      if (devp)
      {
	*devp = dev;
      }
      return SANE_STATUS_GOOD;
    }
  }

  dev = malloc (sizeof (*dev));
  if (!dev)
  {
    DBG (1, "out of memory (line %d)\n", __LINE__);
    return SANE_STATUS_NO_MEM;
  }
  memset (dev, 0, sizeof (*dev));

  /* check for PIO devices */
  /* can we convert the device name to an integer? This is only possible 
     with PIO devices */
  port = atoi (dev_name);
  if (port != 0)
  {
    type = SANE_EPSON_PIO;
  }

  if (strncmp
      (dev_name, SANE_EPSON_CONFIG_PIO, strlen (SANE_EPSON_CONFIG_PIO)) == 0)
  {
    /* we have a match for the PIO string and adjust the device name */
    dev_name += strlen (SANE_EPSON_CONFIG_PIO);
    dev_name = sanei_config_skip_whitespace (dev_name);
    type = SANE_EPSON_PIO;
  }


  /*
   *  set dummy values.
   */

  dev->interpreter = NULL;	/* the interpreter implementation
				   relies on this! */

  s->hw = dev;
  s->hw->sane.name = NULL;
  s->hw->sane.type = "flatbed scanner";
  s->hw->sane.vendor = "Epson";
  s->hw->sane.model = NULL;
  s->hw->optical_res = 0;	/* just to have it initialized */
  s->hw->color_shuffle = SANE_FALSE;
  s->hw->extension = SANE_FALSE;
  s->hw->use_extension = SANE_FALSE;

  s->hw->need_color_reorder = SANE_FALSE;
  s->hw->need_double_vertical = SANE_FALSE;

  s->hw->cmd = &epson_cmd[EPSON_LEVEL_DEFAULT];	/* default function level */
  s->hw->connection = type;

  s->hw->productID = 0;

  DBG (3, "attach: opening %s\n", dev_name);

  s->hw->last_res = 0;
  s->hw->last_res_preview = 0;	/* set resolution to safe values */

  /*
   *  decide if interface is USB, SCSI or parallel.
   */

  /*
   *  if interface is SCSI do an inquiry.
   */

  if (s->hw->connection == SANE_EPSON_SCSI)
  {
    char buf[INQUIRY_BUF_SIZE + 1];
    size_t buf_size = INQUIRY_BUF_SIZE;

    status =
      sanei_scsi_open (dev_name, &s->hw->fd, sanei_epson_scsi_sense_handler,
		       NULL);
    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "attach: open failed: %s\n", sane_strstatus (status));
      delete (dev);
      return status;
    }
    DBG (3, "attach: sending INQUIRY\n");

    status = sanei_epson_scsi_inquiry (s->hw->fd, 0, buf, &buf_size);
    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "attach: inquiry failed: %s\n", sane_strstatus (status));
      close_scanner (s);
      delete (dev);
      return status;
    }

    buf[INQUIRY_BUF_SIZE] = 0;
    DBG (1, ">%s<\n", buf + 8);

    /* 
     * For USB and PIO scanners this will be done later, once
     * we have communication established with the device.
     */

    if (buf[0] != TYPE_PROCESSOR
	|| strncmp (buf + 8, "EPSON", 5) != 0
	|| (strncmp (buf + 16, "SCANNER ", 8) != 0
	    && strncmp (buf + 14, "SCANNER ", 8) != 0
	    && strncmp (buf + 14, "Perfection", 10) != 0
	    && strncmp (buf + 16, "Perfection", 10) != 0
	    && strncmp (buf + 16, "Expression", 10) != 0
	    && strncmp (buf + 16, "GT", 2) != 0
	    && strncmp (buf + 16, "ES-7000H", 8) != 0
	    && strncmp (buf + 16, "GT-15000", 8) != 0))
    {
      DBG (1, "attach: device doesn't look like an EPSON  scanner\n");
      close_scanner (s);
      delete (dev);
      return SANE_STATUS_INVAL;
    }
  }
  /* use the SANEI functions to handle a PIO device */
  else if (s->hw->connection == SANE_EPSON_PIO)
  {
    if (SANE_STATUS_GOOD != (status = sanei_pio_open (dev_name, &s->hw->fd)))
    {
      DBG (1, "Cannot open %s as a parallel-port device: %s\n",
	   dev_name, sane_strstatus (status));
      delete (dev);
      return status;
    }
  }
  /* use the SANEI functions to handle a USB device */
  else if (s->hw->connection == SANE_EPSON_USB)
  {
    SANE_Word vendor;
    SANE_Word product;
    SANE_Bool isLibUSB;

    isLibUSB = (strncmp (dev_name, "libusb:", strlen ("libusb:")) == 0);

    if ((!isLibUSB) && (strlen (dev_name) == 0))
    {
      int i;
      int numIds;

      numIds = sanei_epson_getNumberOfUSBProductIds ();

      for (i = 0; i < numIds; i++)
      {
	product = sanei_epson_usb_product_ids[i];
	vendor = 0x4b8;

	status = sanei_usb_find_devices (vendor, product, attach_one_usb);
      }
      find_interpreted_devices (attach_one_usb);
      return SANE_STATUS_INVAL;	/* return - the attach_one_usb() 
				   will take care of this */
    }

    status = sanei_usb_open (dev_name, &s->hw->fd);

    if (SANE_STATUS_GOOD != status)
    {
      delete (dev);
      return status;
    }

    /* if the sanei_usb_get_vendor_product call is not supported,
       then we just ignore this and rely on the user to config
       the correct device.
     */

    if (sanei_usb_get_vendor_product (s->hw->fd, &vendor, &product) ==
	SANE_STATUS_GOOD)
    {
      int i;			/* loop variable */
      int numIds;
      SANE_Bool is_valid;

      /* check the vendor ID to see if we are dealing with an EPSON device */
      if (vendor != SANE_EPSON_VENDOR_ID)
      {
	/* this is not a supported vendor ID */
	DBG (1,
	     "The device at %s is not manufactured by EPSON (vendor id=0x%x)\n",
	     dev_name, vendor);
	sanei_usb_close (s->hw->fd);
	s->hw->fd = -1;
	delete (dev);
	return SANE_STATUS_INVAL;
      }

      numIds = sanei_epson_getNumberOfUSBProductIds ();
      is_valid = SANE_FALSE;
      i = 0;

      /* check all known product IDs to verify that we know 
         about the device */
      while (i != numIds && !is_valid)
      {
	if (product == sanei_epson_usb_product_ids[i])
	  is_valid = SANE_TRUE;

	i++;
      }

      {				/* try interpreter-based support */
	status = create_interpreter (s->hw, product);
	if (SANE_STATUS_GOOD != status)
	{
	  sanei_usb_close (s->hw->fd);
	  s->hw->fd = -1;
	  delete (dev);
	  return status;
	}
	if (!is_valid)
	{
	  is_valid = (NULL != s->hw->interpreter);
	}
      }

      if (is_valid == SANE_FALSE)
      {
	DBG (1,
	     "The device at %s is not a supported EPSON scanner (product id=0x%x)\n",
	     dev_name, product);
	sanei_usb_close (s->hw->fd);
	s->hw->fd = -1;
	delete (dev);
	return SANE_STATUS_INVAL;
      }
      DBG (1, "Found valid EPSON scanner: 0x%x/0x%x (vendorID/productID)\n",
	   vendor, product);

      s->hw->productID = product;
    }
    else
    {
      DBG (1,
	   "Cannot use IOCTL interface to verify that device is a scanner - will continue\n");
    }
  }

  /*
   * Initialize the scanner (ESC @).
   */
  reset (s);


  /*
   *  Identification Request (ESC I).
   */
  if (s->hw->cmd->request_identity != 0)
  {
    status = get_identity_information (s);
    if (status != SANE_STATUS_GOOD)
    {
      close_scanner (s);
      delete (dev);
      return status;
    }
  }				/* request identity */


  /*
   * Check for "Request Identity 2" command. If this command is available
   * get the information from the scanner and store it in dev
   */

  if (s->hw->cmd->request_identity2 != 0)
  {
    get_identity2_information (s);
    if (status != SANE_STATUS_GOOD)
    {
      close_scanner (s);
      delete (dev);
      return status;
    }
  }				/* request identity 2 */


  /*
   * Check for the max. supported color depth and assign
   * the values to the bitDepthList.
   */

  bitDepthList = malloc (sizeof (SANE_Word) * 4);
  if (bitDepthList == NULL)
  {
    DBG (1, "out of memory (line %d)\n", __LINE__);
    return SANE_STATUS_NO_MEM;
  }

  bitDepthList[0] = 1;		/* we start with one element in the list */
  bitDepthList[1] = 8;		/* 8bit is the default */

  if (set_data_format (s, 16) == SANE_STATUS_GOOD)
  {
    s->hw->maxDepth = 16;

    bitDepthList[0]++;
    bitDepthList[bitDepthList[0]] = 16;

  }
  else if (set_data_format (s, 14) == SANE_STATUS_GOOD)
  {
    s->hw->maxDepth = 14;

    bitDepthList[0]++;
    bitDepthList[bitDepthList[0]] = 14;
  }
  else if (set_data_format (s, 12) == SANE_STATUS_GOOD)
  {
    s->hw->maxDepth = 12;

    bitDepthList[0]++;
    bitDepthList[bitDepthList[0]] = 12;
  }
  else
  {
    s->hw->maxDepth = 8;

    /* the default depth is already in the list */
  }

  DBG (1, "Max. supported color depth = %d\n", s->hw->maxDepth);


  /*
   * Check for "request focus position" command. If this command is 
   * supported, then the scanner does also support the "set focus
   * position" command.
   */

#if 0
  if (request_focus_position (s, &s->currentFocusPosition) ==
      SANE_STATUS_GOOD)
  {
    DBG (1, "Enabling 'Set Focus' support\n");
    s->hw->focusSupport = SANE_TRUE;
    s->opt[OPT_FOCUS].cap &= ~SANE_CAP_INACTIVE;

    /* reflect the current focus position in the GUI */
    if (s->currentFocusPosition < 0x4C)
    {
      /* focus on glass */
      s->val[OPT_FOCUS].w = 0;
    }
    else
    {
      /* focus 2.5mm above glass */
      s->val[OPT_FOCUS].w = 1;
    }

  }
  else
  {
    DBG (1, "Disabling 'Set Focus' support\n");
    s->hw->focusSupport = SANE_FALSE;
    s->opt[OPT_FOCUS].cap |= SANE_CAP_INACTIVE;
    s->val[OPT_FOCUS].w = 0;	/* on glass - just in case */
  }
#endif


/*
 *  Set defaults for no extension.
 */

  dev->x_range = &dev->fbf_x_range;
  dev->y_range = &dev->fbf_y_range;

/*
 *  Add the flatbed option to the source list
 */

  *source_list_add++ = FBF_STR;

/*
 *  Extended status flag request (ESC f).
 *    this also requests the scanner device name from the the scanner
 */
  /*
   * because we are also using the device name from this command, 
   * we have to run this block even if the scanner does not report
   * an extension. The extensions are only reported if the ADF or
   * the TPU are actually detected. 
   */
  if (s->hw->cmd->request_extended_status != 0)
  {
    u_char *buf;
    u_char params[2];
    EpsonHdr head;

    params[0] = ESC;
    params[1] = s->hw->cmd->request_extended_status;

    if (NULL == (head = (EpsonHdr) command (s, params, 2, &status)))
    {
      DBG (1, "Extended status flag request failed\n");
      dev->sane.model = strdup ("Unknown model");
    }
    else
    {
      buf = &head->buf[0];

      /* disable if push button functionality is
         absent, regardless of what the command
         level may say */
      if (!(buf[0] & EXT_STATUS_PB))
      {
	s->hw->cmd->request_push_button_status = 0;
      }


/*
 *  ADF
 */

      if (dev->extension && (buf[1] & EXT_STATUS_IST))
      {
	DBG (1, "ADF detected\n");

	fix_up_extended_status_reply ((const char *) buf + 26, buf);

	dev->duplexSupport = (buf[0] & 0x10) != 0;
	if (dev->duplexSupport)
	{
	  DBG (1, "Found DUPLEX ADF\n");
	}

	if (buf[1] & EXT_STATUS_EN)
	{
	  DBG (1, "ADF is enabled\n");
	  dev->x_range = &dev->adf_x_range;
	  dev->y_range = &dev->adf_y_range;
	}

	dev->adf_x_range.min = 0;
	dev->adf_x_range.max =
	  SANE_FIX ((buf[3] << 8 | buf[2]) * MM_PER_INCH /
		    dev->dpi_range.max);
	dev->adf_x_range.quant = 0;

	dev->adf_max_x = buf[3] << 8 | buf[2];

	dev->adf_y_range.min = 0;
	if (s->hw->cmd->request_identity2)
	  dev->adf_y_range.max =
	    SANE_FIX (((buf[5] << 8 | buf[4]) -
		       2 * (s->hw->max_line_distance)) * MM_PER_INCH /
		      dev->dpi_range.max);
	else
	  dev->adf_y_range.max =
	    SANE_FIX ((buf[5] << 8 | buf[4]) * MM_PER_INCH /
		      dev->dpi_range.max);
	dev->adf_y_range.quant = 0;

	dev->adf_max_y = buf[5] << 8 | buf[4];

	DBG (5, "adf tlx %f tly %f brx %f bry %f [mm]\n",
	     SANE_UNFIX (dev->adf_x_range.min),
	     SANE_UNFIX (dev->adf_y_range.min),
	     SANE_UNFIX (dev->adf_x_range.max),
	     SANE_UNFIX (dev->adf_y_range.max));

	*source_list_add++ = ADF_STR;

	dev->ADF = SANE_TRUE;
      }


/*
 *  TPU
 */

      if (dev->extension && (buf[6] & EXT_STATUS_IST))
      {
	DBG (1, "TPU detected\n");

	if (buf[6] & EXT_STATUS_EN)
	{
	  DBG (1, "TPU is enabled\n");
	  dev->x_range = &dev->tpu_x_range;
	  dev->y_range = &dev->tpu_y_range;
	}

	dev->tpu_x_range.min = 0;
	dev->tpu_x_range.max =
	  SANE_FIX ((buf[8] << 8 | buf[7]) * MM_PER_INCH /
		    dev->dpi_range.max);
	dev->tpu_x_range.quant = 0;

	dev->tpu_y_range.min = 0;
	if (s->hw->cmd->request_identity2)
	  dev->tpu_y_range.max =
	    SANE_FIX (((buf[10] << 8 | buf[9]) -
		       2 * (s->hw->max_line_distance)) * MM_PER_INCH /
		      dev->dpi_range.max);
	else
	  dev->tpu_y_range.max =
	    SANE_FIX ((buf[10] << 8 | buf[9]) * MM_PER_INCH /
		      dev->dpi_range.max);
	dev->tpu_y_range.quant = 0;

	DBG (5, "tpu tlx %f tly %f brx %f bry %f [mm]\n",
	     SANE_UNFIX (dev->tpu_x_range.min),
	     SANE_UNFIX (dev->tpu_y_range.min),
	     SANE_UNFIX (dev->tpu_x_range.max),
	     SANE_UNFIX (dev->tpu_y_range.max));

	*source_list_add++ = TPU_STR;

	dev->TPU = SANE_TRUE;
      }

/*
 *	Get the device name and copy it to dev->sane.model.
 *	The device name starts at buf[0x1A] and is up to 16 bytes long
 *	We are overwriting whatever was set previously!
 */
      {
	char device_name[DEVICE_NAME_LEN + 1];
	char *end_ptr;

	/* make sure that the end of string is marked */
	device_name[DEVICE_NAME_LEN] = '\0';

	/* copy the string to an area where we can work with it */
	memcpy (device_name, buf + 0x1A, DEVICE_NAME_LEN);
	end_ptr = strchr (device_name, ' ');
	if (end_ptr != NULL)
	{
	  *end_ptr = '\0';
	}

	/* finally set the device name */
	dev->sane.model = second_guess_model_name (s, device_name);
      }
    }

    DBG (1, "model : %s\n", dev->sane.model);

    if (strcmp ("GT-8200", dev->sane.model) == 0 ||
	strcmp ("Perfection 1650", dev->sane.model) == 0)
    {

      /*
       * Correct for a firmware bug in some Perfection 1650 scanners:
       * Firmware version 1.08 reports only half the vertical scan area,
       * we have to double the number. 
       * To find out if we have to do this, we just compare
       * is the vertical range is smaller than the horizontal range.
       */

      if ((dev->fbf_x_range.max - dev->fbf_x_range.min) >
	  (dev->fbf_y_range.max - dev->fbf_y_range.min))
      {
	dev->fbf_y_range.max += (dev->fbf_y_range.max - dev->fbf_y_range.min);
	dev->fbf_y_max *= 2;
	dev->need_double_vertical = SANE_TRUE;
	dev->need_color_reorder = SANE_TRUE;

	if (dev->TPU)
	{
	  dev->tpu_y_range.max +=
	    (dev->tpu_y_range.max - dev->tpu_y_range.min);
	}
      }
    }
  }
  else				/* command is not known */
  {
    dev->sane.model = strdup ("EPSON Scanner");
  }

  *source_list_add = NULL;	/* add end marker to source list */

  DBG (1, "scanner model: %s\n", dev->sane.model);

  /* establish defaults */
  s->hw->need_reset_on_source_change = SANE_FALSE;

  if (strcmp ("ES-9000H", dev->sane.model) == 0 ||
      strcmp ("GT-30000", dev->sane.model) == 0)
  {
    s->hw->cmd->set_focus_position = 0;
    s->hw->cmd->feed = 0x19;
  }
  else if (strcmp ("GT-8200", dev->sane.model) == 0 ||
	   strcmp ("Perfection1650", dev->sane.model) == 0 ||
	   strcmp ("Perfection1640", dev->sane.model) == 0 ||
	   strcmp ("GT-8700", dev->sane.model) == 0)
  {
    s->hw->cmd->feed = 0;
    s->hw->cmd->set_focus_position = 0;
    s->hw->need_reset_on_source_change = SANE_TRUE;
  }


/*
 *  Set values for quick format "max" entry.
 */

  qf_params[XtNumber (qf_params) - 1].tl_x = dev->x_range->min;
  qf_params[XtNumber (qf_params) - 1].tl_y = dev->y_range->min;
  qf_params[XtNumber (qf_params) - 1].br_x = dev->x_range->max;
  qf_params[XtNumber (qf_params) - 1].br_y = dev->y_range->max;


/*
 *	Now we can finally set the device name:
 */
  str = malloc (strlen (dev_name) + 1);
  dev->sane.name = strcpy (str, dev_name);

  /* The close_scanner function does _way_ more than we need
     to.  In this function, attach, all we do is open a file
     descriptor and query the device.  Kludging it here only
     for all the USB scanners to avoid setting an alarm in
     close_scanner that shouldn't be set.  */
  if (s->hw->connection == SANE_EPSON_USB)
  {
    sanei_usb_close (s->hw->fd);
    s->hw->fd = -1;
  }
  else
    close_scanner( s);

  /* 
   * we are done with this one, prepare for the next scanner: 
   */

  ++num_devices;
  dev->next = first_dev;
  first_dev = dev;

  if (devp)
  {
    *devp = dev;
  }

  return SANE_STATUS_GOOD;
}



/*
 * attach_one()
 *
 * Part of the SANE API: Attaches the scanner with the device name in *dev.
 */

static SANE_Status
attach_one (const char *dev)
{
  DBG (5, "attach_one(%s)\n", dev);

  return attach (dev, 0, SANE_EPSON_SCSI);
}

SANE_Status
attach_one_usb (SANE_String_Const devname)
{
  int len = strlen (devname);
  char *attach_string;

  DBG (5, "attach_one_usb(%s)\n", devname);

  attach_string = alloca (len + 5);
  if (attach_string == NULL)
    return SANE_STATUS_NO_MEM;

  return attach (devname, 0, SANE_EPSON_USB);
}

/*
 * sane_init()
 *
 *
 */
SANE_Status
sane_init (SANE_Int * version_code, SANE_Auth_Callback authorize)
{
  size_t len;
  FILE *fp;

  authorize = authorize;	/* get rid of compiler warning */

  /* sanei_authorization(devicename, STRINGIFY(BACKEND_NAME), auth_callback); */

  DBG_INIT ();
#if defined PACKAGE && defined VERSION
  DBG (2, "sane_init: " PACKAGE " " VERSION "\n");
#endif

  if (version_code != NULL)
    *version_code = SANE_VERSION_CODE (V_MAJOR, V_MINOR, SANE_EPSON_BUILD);

  sanei_usb_init ();

  /* This is at the moment an interpreter only issue --> move to the
     interpreter code and use a global(?) variable to track the need
     for this.  */
  if (0 != lt_dlinit ())
  {
    DBG (1, "%s\n", lt_dlerror ());
  }

  /* default to /dev/scanner instead of insisting on config file */
  if ((fp = sanei_config_open (EPSON_CONFIG_FILE)))
  {
    char line[PATH_MAX];

    while (sanei_config_read (line, sizeof (line), fp))
    {
      int vendor, product;

      DBG (4, "sane_init, >%s<\n", line);
      if (line[0] == '#')	/* ignore line comments */
	continue;
      len = strlen (line);
      if (!len)
	continue;		/* ignore empty lines */

      if (sscanf (line, "usb %i %i", &vendor, &product) == 2)
      {
	int numIds;

	/* add the vendor and product IDs to the list of 
	   known devices before we call the attach function */
	numIds = sanei_epson_getNumberOfUSBProductIds ();
	if (vendor != 0x4b8)
	  continue;		/* this is not an EPSON device */

	sanei_epson_usb_product_ids[numIds - 1] = product;
	sanei_usb_attach_matching_devices (line, attach_one_usb);
      }
      else if (strncmp (line, "usb", 3) == 0)
      {
	const char *dev_name;
	/* remove the "usb" sub string */
	dev_name = sanei_config_skip_whitespace (line + 3);
	attach_one_usb (dev_name);
      }
      else
      {
	sanei_config_attach_matching_devices (line, attach_one);
      }
    }
    fclose (fp);
  }

  /* read the option section and assign the connection type to the
     scanner structure - which we don't have at this time. So I have
     to come up with something :-) */

  return SANE_STATUS_GOOD;
}

/*
 * void sane_exit(void)
 *
 * Clean up the list of attached scanners. 
 */

void
sane_exit (void)
{
  Epson_Device *dev, *next;

  DBG (1, "sane_exit\n");

  for (dev = first_dev; dev; dev = next)
  {
    next = dev->next;
    if (dev->interpreter)
    {
      dev->interpreter->dtor (dev);
    }
    delete ((void *) dev->sane.name);
    delete ((void *) dev->sane.model);
    delete (dev);
  }

  lt_dlexit ();

  delete (devlist);
}

/*
 *
 *
 */

SANE_Status
sane_get_devices (const SANE_Device * **device_list, SANE_Bool local_only)
{
  Epson_Device *dev;
  int i;

  DBG (5, "sane_get_devices()\n");

  local_only = local_only;	/* just to get rid of the compiler warning */

  if (devlist)
  {
    delete (devlist);
  }

  devlist = malloc ((num_devices + 1) * sizeof (devlist[0]));
  if (!devlist)
  {
    DBG (1, "out of memory (line %d)\n", __LINE__);
    return SANE_STATUS_NO_MEM;
  }

  i = 0;

  for (dev = first_dev; i < num_devices; dev = dev->next)
  {
    devlist[i++] = &dev->sane;
  }

  devlist[i++] = 0;

  *device_list = devlist;

  return SANE_STATUS_GOOD;
}

/*
 *
 *
 */

static SANE_Status
init_options (Epson_Scanner * s)
{
  int i;
  SANE_Bool dummy;
  SANE_Bool reload;

  DBG (5, "init_options()\n");

  for (i = 0; i < NUM_OPTIONS; ++i)
  {
    s->opt[i].size = sizeof (SANE_Word);
    s->opt[i].cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;
  }

  s->opt[OPT_NUM_OPTS].title = SANE_TITLE_NUM_OPTIONS;
  s->opt[OPT_NUM_OPTS].desc = SANE_DESC_NUM_OPTIONS;
  s->opt[OPT_NUM_OPTS].cap = SANE_CAP_SOFT_DETECT;
  s->val[OPT_NUM_OPTS].w = NUM_OPTIONS;

  /* "Scan Mode" group: */

  s->opt[OPT_MODE_GROUP].title = SANE_I18N ("Scan Mode");
  s->opt[OPT_MODE_GROUP].desc = "";
  s->opt[OPT_MODE_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_MODE_GROUP].cap = 0;

  /* scan mode */
  s->opt[OPT_MODE].name = SANE_NAME_SCAN_MODE;
  s->opt[OPT_MODE].title = SANE_TITLE_SCAN_MODE;
  s->opt[OPT_MODE].desc = SANE_DESC_SCAN_MODE;
  s->opt[OPT_MODE].type = SANE_TYPE_STRING;
  s->opt[OPT_MODE].size = max_string_size (mode_list);
  s->opt[OPT_MODE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_MODE].constraint.string_list = mode_list;
  s->val[OPT_MODE].w = 0;	/* Binary */

  /* bit depth */
  s->opt[OPT_BIT_DEPTH].name = SANE_NAME_BIT_DEPTH;
  s->opt[OPT_BIT_DEPTH].title = SANE_TITLE_BIT_DEPTH;
  s->opt[OPT_BIT_DEPTH].desc = SANE_DESC_BIT_DEPTH;
  s->opt[OPT_BIT_DEPTH].type = SANE_TYPE_INT;
  s->opt[OPT_BIT_DEPTH].unit = SANE_UNIT_NONE;
  s->opt[OPT_BIT_DEPTH].constraint_type = SANE_CONSTRAINT_WORD_LIST;
  s->opt[OPT_BIT_DEPTH].constraint.word_list = bitDepthList;
  s->opt[OPT_BIT_DEPTH].cap |= SANE_CAP_INACTIVE;
  s->val[OPT_BIT_DEPTH].w = bitDepthList[1];	/* the first "real" element is the default */

  if (bitDepthList[0] == 1)	/* only one element in the list -> hide the option */
    s->opt[OPT_BIT_DEPTH].cap |= SANE_CAP_INACTIVE;

  /* halftone */
  s->opt[OPT_HALFTONE].name = SANE_NAME_HALFTONE;
  s->opt[OPT_HALFTONE].title = SANE_TITLE_HALFTONE;
  s->opt[OPT_HALFTONE].desc = SANE_I18N ("Selects the halftone.");

  s->opt[OPT_HALFTONE].type = SANE_TYPE_STRING;
  s->opt[OPT_HALFTONE].size = max_string_size (halftone_list_7);
  s->opt[OPT_HALFTONE].constraint_type = SANE_CONSTRAINT_STRING_LIST;

  if (s->hw->level >= 7)
    s->opt[OPT_HALFTONE].constraint.string_list = halftone_list_7;
  else if (s->hw->level >= 4)
    s->opt[OPT_HALFTONE].constraint.string_list = halftone_list_4;
  else
    s->opt[OPT_HALFTONE].constraint.string_list = halftone_list;

  s->val[OPT_HALFTONE].w = 1;	/* Halftone A */

  if (!s->hw->cmd->set_halftoning)
  {
    s->opt[OPT_HALFTONE].cap |= SANE_CAP_INACTIVE;
  }

  /* dropout */
  s->opt[OPT_DROPOUT].name = "dropout";
  s->opt[OPT_DROPOUT].title = SANE_I18N ("Dropout");
  s->opt[OPT_DROPOUT].desc = SANE_I18N ("Selects the dropout.");

  s->opt[OPT_DROPOUT].type = SANE_TYPE_STRING;
  s->opt[OPT_DROPOUT].size = max_string_size (dropout_list);
  s->opt[OPT_DROPOUT].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_DROPOUT].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_DROPOUT].constraint.string_list = dropout_list;
  s->val[OPT_DROPOUT].w = 0;	/* None */

  /* brightness */
  s->opt[OPT_BRIGHTNESS].name = SANE_NAME_BRIGHTNESS;
  s->opt[OPT_BRIGHTNESS].title = SANE_TITLE_BRIGHTNESS;
  s->opt[OPT_BRIGHTNESS].desc = SANE_I18N ("Selects the brightness.");

  s->opt[OPT_BRIGHTNESS].type = SANE_TYPE_INT;
  s->opt[OPT_BRIGHTNESS].unit = SANE_UNIT_NONE;
  s->opt[OPT_BRIGHTNESS].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_BRIGHTNESS].constraint.range = &s->hw->cmd->bright_range;
  s->val[OPT_BRIGHTNESS].w = 0;	/* Normal */

  if (!s->hw->cmd->set_bright)
  {
    s->opt[OPT_BRIGHTNESS].cap |= SANE_CAP_INACTIVE;
  }

  /* sharpness */
  s->opt[OPT_SHARPNESS].name = "sharpness";
  s->opt[OPT_SHARPNESS].title = SANE_I18N ("Sharpness");
  s->opt[OPT_SHARPNESS].desc = "";

  s->opt[OPT_SHARPNESS].type = SANE_TYPE_INT;
  s->opt[OPT_SHARPNESS].unit = SANE_UNIT_NONE;
  s->opt[OPT_SHARPNESS].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_SHARPNESS].constraint.range = &outline_emphasis_range;
  s->val[OPT_SHARPNESS].w = 0;	/* Normal */

  if (!s->hw->cmd->set_outline_emphasis)
  {
    s->opt[OPT_SHARPNESS].cap |= SANE_CAP_INACTIVE;
  }


  /* gamma */
  s->opt[OPT_GAMMA_CORRECTION].name = SANE_NAME_GAMMA_CORRECTION;
  s->opt[OPT_GAMMA_CORRECTION].title = SANE_TITLE_GAMMA_CORRECTION;
  s->opt[OPT_GAMMA_CORRECTION].desc = SANE_DESC_GAMMA_CORRECTION;

  s->opt[OPT_GAMMA_CORRECTION].type = SANE_TYPE_STRING;
  s->opt[OPT_GAMMA_CORRECTION].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  /* 
   * special handling for D1 function level - at this time I'm not
   * testing for D1, I'm just assuming that all D level scanners will
   * behave the same way. This has to be confirmed with the next D-level
   * scanner 
   */
  if (s->hw->cmd->level[0] == 'D')
  {
    s->opt[OPT_GAMMA_CORRECTION].size = max_string_size (gamma_list_d);
    s->opt[OPT_GAMMA_CORRECTION].constraint.string_list = gamma_list_d;
    s->val[OPT_GAMMA_CORRECTION].w = 1;	/* Default */
    gamma_userdefined = gamma_userdefined_d;
    gamma_params = gamma_params_d;
  }
  else
  {
    s->opt[OPT_GAMMA_CORRECTION].size = max_string_size (gamma_list_ab);
    s->opt[OPT_GAMMA_CORRECTION].constraint.string_list = gamma_list_ab;
    s->val[OPT_GAMMA_CORRECTION].w = 0;	/* Default */
    gamma_userdefined = gamma_userdefined_ab;
    gamma_params = gamma_params_ab;
  }

  if (!s->hw->cmd->set_gamma)
  {
    s->opt[OPT_GAMMA_CORRECTION].cap |= SANE_CAP_INACTIVE;
  }


  /* gamma vector */
/* 
		s->opt[ OPT_GAMMA_VECTOR].name  = SANE_NAME_GAMMA_VECTOR;
		s->opt[ OPT_GAMMA_VECTOR].title = SANE_TITLE_GAMMA_VECTOR;
		s->opt[ OPT_GAMMA_VECTOR].desc  = SANE_DESC_GAMMA_VECTOR;

		s->opt[ OPT_GAMMA_VECTOR].type = SANE_TYPE_INT;
		s->opt[ OPT_GAMMA_VECTOR].unit = SANE_UNIT_NONE;
		s->opt[ OPT_GAMMA_VECTOR].size = 256 * sizeof (SANE_Word);
		s->opt[ OPT_GAMMA_VECTOR].constraint_type = SANE_CONSTRAINT_RANGE;
		s->opt[ OPT_GAMMA_VECTOR].constraint.range = &u8_range;
		s->val[ OPT_GAMMA_VECTOR].wa = &s->gamma_table [ 0] [ 0];
*/


  /* red gamma vector */
  s->opt[OPT_GAMMA_VECTOR_R].name = SANE_NAME_GAMMA_VECTOR_R;
  s->opt[OPT_GAMMA_VECTOR_R].title = SANE_TITLE_GAMMA_VECTOR_R;
  s->opt[OPT_GAMMA_VECTOR_R].desc = SANE_DESC_GAMMA_VECTOR_R;

  s->opt[OPT_GAMMA_VECTOR_R].type = SANE_TYPE_INT;
  s->opt[OPT_GAMMA_VECTOR_R].unit = SANE_UNIT_NONE;
  s->opt[OPT_GAMMA_VECTOR_R].size = 256 * sizeof (SANE_Word);
  s->opt[OPT_GAMMA_VECTOR_R].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_GAMMA_VECTOR_R].constraint.range = &u8_range;
  s->val[OPT_GAMMA_VECTOR_R].wa = &s->gamma_table[0][0];


  /* green gamma vector */
  s->opt[OPT_GAMMA_VECTOR_G].name = SANE_NAME_GAMMA_VECTOR_G;
  s->opt[OPT_GAMMA_VECTOR_G].title = SANE_TITLE_GAMMA_VECTOR_G;
  s->opt[OPT_GAMMA_VECTOR_G].desc = SANE_DESC_GAMMA_VECTOR_G;

  s->opt[OPT_GAMMA_VECTOR_G].type = SANE_TYPE_INT;
  s->opt[OPT_GAMMA_VECTOR_G].unit = SANE_UNIT_NONE;
  s->opt[OPT_GAMMA_VECTOR_G].size = 256 * sizeof (SANE_Word);
  s->opt[OPT_GAMMA_VECTOR_G].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_GAMMA_VECTOR_G].constraint.range = &u8_range;
  s->val[OPT_GAMMA_VECTOR_G].wa = &s->gamma_table[1][0];


  /* red gamma vector */
  s->opt[OPT_GAMMA_VECTOR_B].name = SANE_NAME_GAMMA_VECTOR_B;
  s->opt[OPT_GAMMA_VECTOR_B].title = SANE_TITLE_GAMMA_VECTOR_B;
  s->opt[OPT_GAMMA_VECTOR_B].desc = SANE_DESC_GAMMA_VECTOR_B;

  s->opt[OPT_GAMMA_VECTOR_B].type = SANE_TYPE_INT;
  s->opt[OPT_GAMMA_VECTOR_B].unit = SANE_UNIT_NONE;
  s->opt[OPT_GAMMA_VECTOR_B].size = 256 * sizeof (SANE_Word);
  s->opt[OPT_GAMMA_VECTOR_B].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_GAMMA_VECTOR_B].constraint.range = &u8_range;
  s->val[OPT_GAMMA_VECTOR_B].wa = &s->gamma_table[2][0];

  if (s->hw->cmd->set_gamma_table &&
      gamma_userdefined[s->val[OPT_GAMMA_CORRECTION].w] == SANE_TRUE)
  {
/*			s->opt[ OPT_GAMMA_VECTOR].cap &= ~SANE_CAP_INACTIVE; */
    s->opt[OPT_GAMMA_VECTOR_R].cap &= ~SANE_CAP_INACTIVE;
    s->opt[OPT_GAMMA_VECTOR_G].cap &= ~SANE_CAP_INACTIVE;
    s->opt[OPT_GAMMA_VECTOR_B].cap &= ~SANE_CAP_INACTIVE;
  }
  else
  {
/*			s->opt[ OPT_GAMMA_VECTOR].cap |= SANE_CAP_INACTIVE; */
    s->opt[OPT_GAMMA_VECTOR_R].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_GAMMA_VECTOR_G].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_GAMMA_VECTOR_B].cap |= SANE_CAP_INACTIVE;
  }

  /* initialize the Gamma tables */
  memset (&s->gamma_table[0], 0, 256 * sizeof (SANE_Word));
  memset (&s->gamma_table[1], 0, 256 * sizeof (SANE_Word));
  memset (&s->gamma_table[2], 0, 256 * sizeof (SANE_Word));
/*		memset(&s->gamma_table[3], 0, 256 * sizeof(SANE_Word)); */
  for (i = 0; i < 256; i++)
  {
    s->gamma_table[0][i] = i;
    s->gamma_table[1][i] = i;
    s->gamma_table[2][i] = i;
/*			s->gamma_table[3][i] = i; */
  }


  /* color correction */
  s->opt[OPT_COLOR_CORRECTION].name = "color-correction";
  s->opt[OPT_COLOR_CORRECTION].title = SANE_I18N ("Color correction");
  s->opt[OPT_COLOR_CORRECTION].desc =
    SANE_I18N
    ("Sets the color correction table for the selected output device.");

  s->opt[OPT_COLOR_CORRECTION].type = SANE_TYPE_STRING;
  s->opt[OPT_COLOR_CORRECTION].size = 32;
  s->opt[OPT_COLOR_CORRECTION].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_COLOR_CORRECTION].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  if (s->hw->cmd->level[0] == 'D')
  {
    s->opt[OPT_COLOR_CORRECTION].size = max_string_size (color_list_d);
    s->opt[OPT_COLOR_CORRECTION].constraint.string_list = color_list_d;
    s->val[OPT_COLOR_CORRECTION].w = 0;	/* Default */
    color_userdefined = color_userdefined_d;
    color_params = color_params_d;
  }
  else
  {
    s->opt[OPT_COLOR_CORRECTION].size = max_string_size (color_list_ab);
    s->opt[OPT_COLOR_CORRECTION].constraint.string_list = color_list_ab;
    s->val[OPT_COLOR_CORRECTION].w = 5;	/* scanner default: CRT monitors */
    color_userdefined = color_userdefined_ab;
    color_params = color_params_ab;
  }

  if (!s->hw->cmd->set_color_correction)
  {
    s->opt[OPT_COLOR_CORRECTION].cap |= SANE_CAP_INACTIVE;
  }

  /* resolution */
  s->opt[OPT_RESOLUTION].name = SANE_NAME_SCAN_RESOLUTION;
  s->opt[OPT_RESOLUTION].title = SANE_TITLE_SCAN_RESOLUTION;
  s->opt[OPT_RESOLUTION].desc = SANE_DESC_SCAN_RESOLUTION;

  s->opt[OPT_RESOLUTION].type = SANE_TYPE_INT;
  s->opt[OPT_RESOLUTION].unit = SANE_UNIT_DPI;

  if (s->hw->cmd->level[0] != 'D')
  {
    s->opt[OPT_RESOLUTION].constraint_type = SANE_CONSTRAINT_RANGE;
    s->opt[OPT_RESOLUTION].constraint.range = &s->hw->dpi_range;
  }
  else
  {
    s->opt[OPT_RESOLUTION].constraint_type = SANE_CONSTRAINT_WORD_LIST;
    s->opt[OPT_RESOLUTION].constraint.word_list = s->hw->resolution_list;
  }
  s->val[OPT_RESOLUTION].w = 300;

  /* threshold */
  s->opt[OPT_THRESHOLD].name = SANE_NAME_THRESHOLD;
  s->opt[OPT_THRESHOLD].title = SANE_TITLE_THRESHOLD;
  s->opt[OPT_THRESHOLD].desc = SANE_DESC_THRESHOLD;

  s->opt[OPT_THRESHOLD].type = SANE_TYPE_INT;
  s->opt[OPT_THRESHOLD].unit = SANE_UNIT_NONE;
  s->opt[OPT_THRESHOLD].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_THRESHOLD].constraint.range = &u8_range;
  s->val[OPT_THRESHOLD].w = 0x80;

  if (!s->hw->cmd->set_threshold)
  {
    s->opt[OPT_THRESHOLD].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_CCT_GROUP].title = SANE_I18N ("Color correction coefficients");
  s->opt[OPT_CCT_GROUP].desc = SANE_I18N ("Matrix multiplication of RGB");
  s->opt[OPT_CCT_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_CCT_GROUP].cap = SANE_CAP_ADVANCED;


  /* color correction coefficients */
  s->opt[OPT_CCT_1].name = "cct-1";
  s->opt[OPT_CCT_2].name = "cct-2";
  s->opt[OPT_CCT_3].name = "cct-3";
  s->opt[OPT_CCT_4].name = "cct-4";
  s->opt[OPT_CCT_5].name = "cct-5";
  s->opt[OPT_CCT_6].name = "cct-6";
  s->opt[OPT_CCT_7].name = "cct-7";
  s->opt[OPT_CCT_8].name = "cct-8";
  s->opt[OPT_CCT_9].name = "cct-9";

  s->opt[OPT_CCT_1].title = SANE_I18N ("Red");
  s->opt[OPT_CCT_2].title = SANE_I18N ("Shift green to red");
  s->opt[OPT_CCT_3].title = SANE_I18N ("Shift blue to red");
  s->opt[OPT_CCT_4].title = SANE_I18N ("Shift red to green");
  s->opt[OPT_CCT_5].title = SANE_I18N ("Green");
  s->opt[OPT_CCT_6].title = SANE_I18N ("Shift blue to green");
  s->opt[OPT_CCT_7].title = SANE_I18N ("Shift red to blue");
  s->opt[OPT_CCT_8].title = SANE_I18N ("Shift green to blue");
  s->opt[OPT_CCT_9].title = SANE_I18N ("Blue");

  s->opt[OPT_CCT_1].desc = SANE_I18N ("Controls red level");
  s->opt[OPT_CCT_2].desc = SANE_I18N ("Adds to red based on green level");
  s->opt[OPT_CCT_3].desc = SANE_I18N ("Adds to red based on blue level");
  s->opt[OPT_CCT_4].desc = SANE_I18N ("Adds to green based on red level");
  s->opt[OPT_CCT_5].desc = SANE_I18N ("Controls green level");
  s->opt[OPT_CCT_6].desc = SANE_I18N ("Adds to green based on blue level");
  s->opt[OPT_CCT_7].desc = SANE_I18N ("Adds to blue based on red level");
  s->opt[OPT_CCT_8].desc = SANE_I18N ("Adds to blue based on green level");
  s->opt[OPT_CCT_9].desc = SANE_I18N ("Control blue level");

  s->opt[OPT_CCT_1].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_2].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_3].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_4].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_5].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_6].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_7].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_8].type = SANE_TYPE_FIXED;
  s->opt[OPT_CCT_9].type = SANE_TYPE_FIXED;

  s->opt[OPT_CCT_1].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_2].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_3].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_4].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_5].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_6].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_7].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_8].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_CCT_9].cap |= SANE_CAP_ADVANCED;

  s->opt[OPT_CCT_1].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_2].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_3].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_4].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_5].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_6].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_7].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_8].cap |= SANE_CAP_INACTIVE;
  s->opt[OPT_CCT_9].cap |= SANE_CAP_INACTIVE;

  s->opt[OPT_CCT_1].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_2].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_3].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_4].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_5].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_6].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_7].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_8].unit = SANE_UNIT_NONE;
  s->opt[OPT_CCT_9].unit = SANE_UNIT_NONE;

  s->opt[OPT_CCT_1].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_2].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_3].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_4].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_5].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_6].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_7].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_8].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_CCT_9].constraint_type = SANE_CONSTRAINT_RANGE;

  s->hw->matrix_range.min = SANE_FIX (-2.0);
  s->hw->matrix_range.max = SANE_FIX (2.0);
  s->hw->matrix_range.quant = 0;

  s->opt[OPT_CCT_1].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_2].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_3].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_4].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_5].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_6].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_7].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_8].constraint.range = &s->hw->matrix_range;
  s->opt[OPT_CCT_9].constraint.range = &s->hw->matrix_range;

  s->val[OPT_CCT_1].w = SANE_FIX (1.0);
  s->val[OPT_CCT_2].w = 0;
  s->val[OPT_CCT_3].w = 0;
  s->val[OPT_CCT_4].w = 0;
  s->val[OPT_CCT_5].w = SANE_FIX (1.0);
  s->val[OPT_CCT_6].w = 0;
  s->val[OPT_CCT_7].w = 0;
  s->val[OPT_CCT_8].w = 0;
  s->val[OPT_CCT_9].w = SANE_FIX (1.0);

  if (!s->hw->cmd->set_color_correction_coefficients)
  {
    s->opt[OPT_CCT_1].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_CCT_2].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_CCT_3].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_CCT_4].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_CCT_5].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_CCT_6].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_CCT_7].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_CCT_8].cap |= SANE_CAP_INACTIVE;
    s->opt[OPT_CCT_9].cap |= SANE_CAP_INACTIVE;
  }


  /* "Advanced" group: */
  s->opt[OPT_ADVANCED_GROUP].title = SANE_I18N ("Advanced");
  s->opt[OPT_ADVANCED_GROUP].desc = "";
  s->opt[OPT_ADVANCED_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_ADVANCED_GROUP].cap = SANE_CAP_ADVANCED;


  /* mirror */
  s->opt[OPT_MIRROR].name = "mirror";
  s->opt[OPT_MIRROR].title = SANE_I18N ("Mirror image");
  s->opt[OPT_MIRROR].desc = SANE_I18N ("Mirror the image.");

  s->opt[OPT_MIRROR].type = SANE_TYPE_BOOL;
  s->val[OPT_MIRROR].w = SANE_FALSE;

  if (!s->hw->cmd->mirror_image)
  {
    s->opt[OPT_MIRROR].cap |= SANE_CAP_INACTIVE;
  }


  /* speed */
  s->opt[OPT_SPEED].name = SANE_NAME_SCAN_SPEED;
  s->opt[OPT_SPEED].title = SANE_TITLE_SCAN_SPEED;
  s->opt[OPT_SPEED].desc = SANE_DESC_SCAN_SPEED;

  s->opt[OPT_SPEED].type = SANE_TYPE_BOOL;
  s->val[OPT_SPEED].w = SANE_FALSE;

  if (!s->hw->cmd->set_speed)
  {
    s->opt[OPT_SPEED].cap |= SANE_CAP_INACTIVE;
  }

  /* preview speed */
  s->opt[OPT_PREVIEW_SPEED].name = "preview-speed";
  s->opt[OPT_PREVIEW_SPEED].title = SANE_I18N ("Speed");
  s->opt[OPT_PREVIEW_SPEED].desc = "";

  s->opt[OPT_PREVIEW_SPEED].type = SANE_TYPE_BOOL;
  s->val[OPT_PREVIEW_SPEED].w = SANE_FALSE;

  if (!s->hw->cmd->set_speed)
  {
    s->opt[OPT_PREVIEW_SPEED].cap |= SANE_CAP_INACTIVE;
  }

  /* auto area segmentation */
  s->opt[OPT_AAS].name = "auto-area-segmentation";
  s->opt[OPT_AAS].title = SANE_I18N ("Auto area segmentation");
  s->opt[OPT_AAS].desc = "";

  s->opt[OPT_AAS].type = SANE_TYPE_BOOL;
  s->val[OPT_AAS].w = SANE_TRUE;

  if (!s->hw->cmd->control_auto_area_segmentation)
  {
    s->opt[OPT_AAS].cap |= SANE_CAP_INACTIVE;
  }

  /* limit resolution list */
  s->opt[OPT_LIMIT_RESOLUTION].name = "short-resolution";
  s->opt[OPT_LIMIT_RESOLUTION].title = SANE_I18N ("Short resolution list");
  s->opt[OPT_LIMIT_RESOLUTION].desc =
    SANE_I18N ("Display short resolution list");
  s->opt[OPT_LIMIT_RESOLUTION].type = SANE_TYPE_BOOL;
  s->val[OPT_LIMIT_RESOLUTION].w = SANE_FALSE;


  /* zoom */
  s->opt[OPT_ZOOM].name = "zoom";
  s->opt[OPT_ZOOM].title = SANE_I18N ("Zoom");
  s->opt[OPT_ZOOM].desc =
    SANE_I18N ("Defines the zoom factor the scanner will use");

  s->opt[OPT_ZOOM].type = SANE_TYPE_INT;
  s->opt[OPT_ZOOM].unit = SANE_UNIT_NONE;
  s->opt[OPT_ZOOM].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_ZOOM].constraint.range = &zoom_range;
  s->val[OPT_ZOOM].w = 100;

  if (!s->hw->cmd->set_zoom)
  {
    s->opt[OPT_ZOOM].cap |= SANE_CAP_INACTIVE;
  }


  /* "Preview settings" group: */
  s->opt[OPT_PREVIEW_GROUP].title = SANE_TITLE_PREVIEW;
  s->opt[OPT_PREVIEW_GROUP].desc = "";
  s->opt[OPT_PREVIEW_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_PREVIEW_GROUP].cap = SANE_CAP_ADVANCED;

  /* preview */
  s->opt[OPT_PREVIEW].name = SANE_NAME_PREVIEW;
  s->opt[OPT_PREVIEW].title = SANE_TITLE_PREVIEW;
  s->opt[OPT_PREVIEW].desc = SANE_DESC_PREVIEW;

  s->opt[OPT_PREVIEW].type = SANE_TYPE_BOOL;
  s->val[OPT_PREVIEW].w = SANE_FALSE;

  /* "Geometry" group: */
  s->opt[OPT_GEOMETRY_GROUP].title = SANE_I18N ("Geometry");
  s->opt[OPT_GEOMETRY_GROUP].desc = "";
  s->opt[OPT_GEOMETRY_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_GEOMETRY_GROUP].cap = SANE_CAP_ADVANCED;

  /* top-left x */
  s->opt[OPT_TL_X].name = SANE_NAME_SCAN_TL_X;
  s->opt[OPT_TL_X].title = SANE_TITLE_SCAN_TL_X;
  s->opt[OPT_TL_X].desc = SANE_DESC_SCAN_TL_X;

  s->opt[OPT_TL_X].type = SANE_TYPE_FIXED;
  s->opt[OPT_TL_X].unit = SANE_UNIT_MM;
  s->opt[OPT_TL_X].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_TL_X].constraint.range = s->hw->x_range;
  s->val[OPT_TL_X].w = 0;

  /* top-left y */
  s->opt[OPT_TL_Y].name = SANE_NAME_SCAN_TL_Y;
  s->opt[OPT_TL_Y].title = SANE_TITLE_SCAN_TL_Y;
  s->opt[OPT_TL_Y].desc = SANE_DESC_SCAN_TL_Y;

  s->opt[OPT_TL_Y].type = SANE_TYPE_FIXED;
  s->opt[OPT_TL_Y].unit = SANE_UNIT_MM;
  s->opt[OPT_TL_Y].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_TL_Y].constraint.range = s->hw->y_range;
  s->val[OPT_TL_Y].w = 0;

  /* bottom-right x */
  s->opt[OPT_BR_X].name = SANE_NAME_SCAN_BR_X;
  s->opt[OPT_BR_X].title = SANE_TITLE_SCAN_BR_X;
  s->opt[OPT_BR_X].desc = SANE_DESC_SCAN_BR_X;

  s->opt[OPT_BR_X].type = SANE_TYPE_FIXED;
  s->opt[OPT_BR_X].unit = SANE_UNIT_MM;
  s->opt[OPT_BR_X].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_BR_X].constraint.range = s->hw->x_range;
  s->val[OPT_BR_X].w = s->hw->x_range->max;

  /* bottom-right y */
  s->opt[OPT_BR_Y].name = SANE_NAME_SCAN_BR_Y;
  s->opt[OPT_BR_Y].title = SANE_TITLE_SCAN_BR_Y;
  s->opt[OPT_BR_Y].desc = SANE_DESC_SCAN_BR_Y;

  s->opt[OPT_BR_Y].type = SANE_TYPE_FIXED;
  s->opt[OPT_BR_Y].unit = SANE_UNIT_MM;
  s->opt[OPT_BR_Y].constraint_type = SANE_CONSTRAINT_RANGE;
  s->opt[OPT_BR_Y].constraint.range = s->hw->y_range;
  s->val[OPT_BR_Y].w = s->hw->y_range->max;

  /* Quick format */
  s->opt[OPT_QUICK_FORMAT].name = "quick-format";
  s->opt[OPT_QUICK_FORMAT].title = SANE_I18N ("Quick format");
  s->opt[OPT_QUICK_FORMAT].desc = "";

  s->opt[OPT_QUICK_FORMAT].type = SANE_TYPE_STRING;
  s->opt[OPT_QUICK_FORMAT].size = max_string_size (qf_list);
  s->opt[OPT_QUICK_FORMAT].cap |= SANE_CAP_ADVANCED;
  s->opt[OPT_QUICK_FORMAT].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_QUICK_FORMAT].constraint.string_list = qf_list;
  s->val[OPT_QUICK_FORMAT].w = XtNumber (qf_params) - 1;	/* max */

  /* "Optional equipment" group: */
  s->opt[OPT_EQU_GROUP].title = SANE_I18N ("Optional equipment");
  s->opt[OPT_EQU_GROUP].desc = "";
  s->opt[OPT_EQU_GROUP].type = SANE_TYPE_GROUP;
  s->opt[OPT_EQU_GROUP].cap = SANE_CAP_ADVANCED;


  /* source */
  s->opt[OPT_SOURCE].name = SANE_NAME_SCAN_SOURCE;
  s->opt[OPT_SOURCE].title = SANE_TITLE_SCAN_SOURCE;
  s->opt[OPT_SOURCE].desc = SANE_DESC_SCAN_SOURCE;

  s->opt[OPT_SOURCE].type = SANE_TYPE_STRING;
  s->opt[OPT_SOURCE].size = max_string_size (source_list);

  s->opt[OPT_SOURCE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_SOURCE].constraint.string_list = source_list;

  if (!s->hw->extension)
  {
    s->opt[OPT_SOURCE].cap |= SANE_CAP_INACTIVE;
  }
  s->val[OPT_SOURCE].w = 0;	/* always use Flatbed as default */


  /* film type */
  s->opt[OPT_FILM_TYPE].name = "film-type";
  s->opt[OPT_FILM_TYPE].title = SANE_I18N ("Film type");
  s->opt[OPT_FILM_TYPE].desc = "";

  s->opt[OPT_FILM_TYPE].type = SANE_TYPE_STRING;
  s->opt[OPT_FILM_TYPE].size = max_string_size (film_list);

  s->opt[OPT_FILM_TYPE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_FILM_TYPE].constraint.string_list = film_list;

  s->val[OPT_FILM_TYPE].w = 0;

  deactivateOption (s, OPT_FILM_TYPE, &dummy);	/* default is inactive */

  /* focus position */
  s->opt[OPT_FOCUS].name = SANE_EPSON_FOCUS_NAME;
  s->opt[OPT_FOCUS].title = SANE_EPSON_FOCUS_TITLE;
  s->opt[OPT_FOCUS].desc = SANE_EPSON_FOCUS_DESC;
  s->opt[OPT_FOCUS].type = SANE_TYPE_STRING;
  s->opt[OPT_FOCUS].size = max_string_size (focus_list);
  s->opt[OPT_FOCUS].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_FOCUS].constraint.string_list = focus_list;
  s->val[OPT_FOCUS].w = 0;

  s->opt[OPT_FOCUS].cap |= SANE_CAP_ADVANCED;
  if (s->hw->focusSupport == SANE_TRUE)
  {
    s->opt[OPT_FOCUS].cap &= ~SANE_CAP_INACTIVE;
  }
  else
  {
    s->opt[OPT_FOCUS].cap |= SANE_CAP_INACTIVE;
  }

#if 0
  if ((!s->hw->TPU) && (!s->hw->cmd->set_bay))
  {				/* Hack: Using set_bay to indicate. */
    SANE_Bool dummy;
    deactivateOption (s, OPT_FILM_TYPE, &dummy);

  }
#endif


  /* forward feed / eject */
  s->opt[OPT_EJECT].name = "eject";
  s->opt[OPT_EJECT].title = SANE_I18N ("Eject");
  s->opt[OPT_EJECT].desc = SANE_I18N ("Eject the sheet in the ADF");

  s->opt[OPT_EJECT].type = SANE_TYPE_BUTTON;

  if ((!s->hw->ADF) && (!s->hw->cmd->set_bay))
  {				/* Hack: Using set_bay to indicate. */
    s->opt[OPT_EJECT].cap |= SANE_CAP_INACTIVE;
  }


  /* auto forward feed / eject */
  s->opt[OPT_AUTO_EJECT].name = "auto-eject";
  s->opt[OPT_AUTO_EJECT].title = SANE_I18N ("Auto eject");
  s->opt[OPT_AUTO_EJECT].desc = SANE_I18N ("Eject document after scanning");

  s->opt[OPT_AUTO_EJECT].type = SANE_TYPE_BOOL;
  s->val[OPT_AUTO_EJECT].w = SANE_TRUE;

  if (!s->hw->ADF)
  {
    s->opt[OPT_AUTO_EJECT].cap |= SANE_CAP_INACTIVE;
  }


  s->opt[OPT_ADF_MODE].name = "adf_mode";
  s->opt[OPT_ADF_MODE].title = SANE_I18N ("ADF Mode");
  s->opt[OPT_ADF_MODE].desc =
    SANE_I18N ("Selects the ADF mode (simplex/duplex)");
  s->opt[OPT_ADF_MODE].type = SANE_TYPE_STRING;
  s->opt[OPT_ADF_MODE].size = max_string_size (adf_mode_list);
  s->opt[OPT_ADF_MODE].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_ADF_MODE].constraint.string_list = adf_mode_list;
  s->val[OPT_ADF_MODE].w = 0;	/* simplex */

  if ((!s->hw->ADF) || (s->hw->duplexSupport == SANE_FALSE))
  {
    s->opt[OPT_ADF_MODE].cap |= SANE_CAP_INACTIVE;
  }

  /* select bay */
  s->opt[OPT_BAY].name = "bay";
  s->opt[OPT_BAY].title = SANE_I18N ("Bay");
  s->opt[OPT_BAY].desc = SANE_I18N ("Select bay to scan");

  s->opt[OPT_BAY].type = SANE_TYPE_STRING;
  s->opt[OPT_BAY].size = max_string_size (bay_list);
  s->opt[OPT_BAY].constraint_type = SANE_CONSTRAINT_STRING_LIST;
  s->opt[OPT_BAY].constraint.string_list = bay_list;
  s->val[OPT_BAY].w = 0;	/* Bay 1 */

  if (!s->hw->cmd->set_bay)
  {
    s->opt[OPT_BAY].cap |= SANE_CAP_INACTIVE;
  }


  s->opt[OPT_WAIT_FOR_BUTTON].name = SANE_EPSON_WAIT_FOR_BUTTON_NAME;
  s->opt[OPT_WAIT_FOR_BUTTON].title = SANE_EPSON_WAIT_FOR_BUTTON_TITLE;
  s->opt[OPT_WAIT_FOR_BUTTON].desc = SANE_EPSON_WAIT_FOR_BUTTON_DESC;

  s->opt[OPT_WAIT_FOR_BUTTON].type = SANE_TYPE_BOOL;
  s->opt[OPT_WAIT_FOR_BUTTON].unit = SANE_UNIT_NONE;
  s->opt[OPT_WAIT_FOR_BUTTON].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_WAIT_FOR_BUTTON].constraint.range = NULL;
  s->opt[OPT_WAIT_FOR_BUTTON].cap |= SANE_CAP_ADVANCED;

  if (!s->hw->cmd->request_push_button_status)
  {
    s->opt[OPT_WAIT_FOR_BUTTON].cap |= SANE_CAP_INACTIVE;
  }

  s->opt[OPT_MONITOR_BUTTON].name = SANE_EPSON_MONITOR_BUTTON_NAME;
  s->opt[OPT_MONITOR_BUTTON].title = SANE_EPSON_MONITOR_BUTTON_TITLE;
  s->opt[OPT_MONITOR_BUTTON].desc = SANE_EPSON_MONITOR_BUTTON_DESC;

  s->opt[OPT_MONITOR_BUTTON].type = SANE_TYPE_BOOL;
  s->opt[OPT_MONITOR_BUTTON].unit = SANE_UNIT_NONE;
  s->opt[OPT_MONITOR_BUTTON].constraint_type = SANE_CONSTRAINT_NONE;
  s->opt[OPT_MONITOR_BUTTON].constraint.range = NULL;
  s->opt[OPT_MONITOR_BUTTON].cap |= SANE_CAP_ADVANCED;

  if (!s->hw->cmd->request_push_button_status)
  {
    s->opt[OPT_MONITOR_BUTTON].cap |= SANE_CAP_INACTIVE;
  }

  handle_mode (s, s->val[OPT_MODE].w, &reload);
  change_profile_matrix (s);

  return SANE_STATUS_GOOD;
}

/*
 *
 *
 */

SANE_Status
sane_open (SANE_String_Const devicename, SANE_Handle * handle)
{
  Epson_Device *dev;
  Epson_Scanner *s;

  DBG (5, "sane_open(%s)\n", devicename);

  /* search for device */
  if (devicename[0])
  {
    for (dev = first_dev; dev; dev = dev->next)
    {
      if (strcmp (dev->sane.name, devicename) == 0)
      {
	break;
      }
    }

    if (!dev)
    {
#if 0
      status = attach (devicename, &dev, SANE_EPSON_);
      if (status != SANE_STATUS_GOOD)
      {
	return status;
      }
#endif
      DBG (1, "Error opening the device");
      return SANE_STATUS_INVAL;
    }
  }
  else
  {
    dev = first_dev;
  }

  if (!dev)
  {
    return SANE_STATUS_INVAL;
  }

  s = calloc (sizeof (Epson_Scanner), 1);
  if (!s)
  {
    DBG (1, "out of memory (line %d)\n", __LINE__);
    return SANE_STATUS_NO_MEM;
  }

  s->hw = dev;
  s->hw->fd = -1;

  if (dev->interpreter)		/* FIXME: do we need this?  */
  {
    dev->interpreter->open (dev);
  }

  init_options (s);

  /* insert newly opened handle into list of open handles */
  s->next = first_handle;
  first_handle = s;

  *handle = (SANE_Handle) s;

  open_scanner (s);

  return SANE_STATUS_GOOD;
}

/*
 *
 *
 */

void
sane_close (SANE_Handle handle)
{
  Epson_Scanner *s, *prev;

  /*
   * Test if there is still data pending from 
   * the scanner. If so, then do a cancel
   */

  s = (Epson_Scanner *) handle;

  /* remove handle from list of open handles */
  prev = 0;
  for (s = first_handle; s; s = s->next)
  {
    if (s == handle)
      break;
    prev = s;
  }

  if (!s)
  {
    DBG (1, "close: invalid handle (0x%p)\n", handle);
    return;
  }

  if (prev)
    prev->next = s->next;
  else
    first_handle = s->next;

  if (s->hw->interpreter)	/* FIXME: do we need this?  */
  {
    s->hw->interpreter->close (s->hw);
  }

  close_scanner (s);

  s->hw = 0;
  delete (s);
}

/*
 *
 *
 */

const SANE_Option_Descriptor *
sane_get_option_descriptor (SANE_Handle handle, SANE_Int option)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;

  if (option < 0 || option >= NUM_OPTIONS)
    return NULL;

  return (s->opt + option);
}

/*
 *
 *
 */

static const SANE_String_Const *
search_string_list (const SANE_String_Const * list, SANE_String value)
{
  while (*list != NULL && strcmp (value, *list) != 0)
  {
    ++list;
  }

  return ((*list == NULL) ? NULL : list);
}

/*
 *
 *
 */

/*
    Activate, deactivate an option.  Subroutines so we can add
    debugging info if we want.  The change flag is set to TRUE
    if we changed an option.  If we did not change an option,
    then the value of the changed flag is not modified.
*/

static void
activateOption (Epson_Scanner * s, SANE_Int option, SANE_Bool * change)
{
  if (!SANE_OPTION_IS_ACTIVE (s->opt[option].cap))
  {
    s->opt[option].cap &= ~SANE_CAP_INACTIVE;
    *change = SANE_TRUE;
  }
}

static void
deactivateOption (Epson_Scanner * s, SANE_Int option, SANE_Bool * change)
{
  if (SANE_OPTION_IS_ACTIVE (s->opt[option].cap))
  {
    s->opt[option].cap |= SANE_CAP_INACTIVE;
    *change = SANE_TRUE;
  }
}

static void
setOptionState (Epson_Scanner * s, SANE_Bool state,
		SANE_Int option, SANE_Bool * change)
{
  if (state)
  {
    activateOption (s, option, change);
  }
  else
  {
    deactivateOption (s, option, change);
  }
}

/**
    End of activateOption, deactivateOption, setOptionState.
**/

static SANE_Status
getvalue (SANE_Handle handle, SANE_Int option, void *value)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Option_Descriptor *sopt = &(s->opt[option]);
  Option_Value *sval = &(s->val[option]);

  switch (option)
  {
/* 	case OPT_GAMMA_VECTOR: */
  case OPT_GAMMA_VECTOR_R:
  case OPT_GAMMA_VECTOR_G:
  case OPT_GAMMA_VECTOR_B:
    memcpy (value, sval->wa, sopt->size);
    break;

  case OPT_NUM_OPTS:
  case OPT_RESOLUTION:
  case OPT_TL_X:
  case OPT_TL_Y:
  case OPT_BR_X:
  case OPT_BR_Y:
  case OPT_MIRROR:
  case OPT_SPEED:
  case OPT_PREVIEW_SPEED:
  case OPT_AAS:
  case OPT_PREVIEW:
  case OPT_BRIGHTNESS:
  case OPT_SHARPNESS:
  case OPT_AUTO_EJECT:
  case OPT_CCT_1:
  case OPT_CCT_2:
  case OPT_CCT_3:
  case OPT_CCT_4:
  case OPT_CCT_5:
  case OPT_CCT_6:
  case OPT_CCT_7:
  case OPT_CCT_8:
  case OPT_CCT_9:
  case OPT_THRESHOLD:
  case OPT_ZOOM:
  case OPT_BIT_DEPTH:
  case OPT_WAIT_FOR_BUTTON:
  case OPT_LIMIT_RESOLUTION:
    *((SANE_Word *) value) = sval->w;
    break;
  case OPT_MODE:
  case OPT_ADF_MODE:
  case OPT_HALFTONE:
  case OPT_DROPOUT:
  case OPT_QUICK_FORMAT:
  case OPT_SOURCE:
  case OPT_FILM_TYPE:
  case OPT_GAMMA_CORRECTION:
  case OPT_COLOR_CORRECTION:
  case OPT_BAY:
  case OPT_FOCUS:
    strcpy ((char *) value, sopt->constraint.string_list[sval->w]);
    break;
#if 0
  case OPT_MODEL:
    strcpy (value, sval->s);
    break;
#endif

  case OPT_MONITOR_BUTTON:
    if (SANE_OPTION_IS_ACTIVE (option))
    {
      SANE_Bool pressed;
      SANE_Status status = SANE_STATUS_GOOD;
      SANE_Bool is_open = (s->hw->fd >= 0);
      if (!is_open)
      {
	status = open_scanner (s);
      }
      if (SANE_STATUS_GOOD == status)
      {
	status = request_push_button_status (s, &pressed);
	if (SANE_STATUS_GOOD == status)
	{
	  *((SANE_Bool *) value) = pressed;
	}
      }
      if (!is_open)
      {
	close_scanner (s);
      }
      return status;
    }
    else
    {
      return SANE_STATUS_UNSUPPORTED;
    }
    break;
  default:
    return SANE_STATUS_INVAL;

  }

  return SANE_STATUS_GOOD;
}


/**
    End of getvalue.
**/

static void
handle_mode (Epson_Scanner * s, SANE_Int optindex, SANE_Bool * reload)
{
  SANE_Bool dropout, aas, halftone, threshold, cct;

  *reload = SANE_FALSE;

  switch (optindex)
  {
  case 0:			/* b & w */
    dropout = SANE_TRUE;
    aas = SANE_TRUE;
    halftone = SANE_TRUE;
    threshold = SANE_TRUE;
    cct = SANE_FALSE;
    break;
  case 1:			/* gray */
    dropout = SANE_TRUE;
    aas = SANE_FALSE;
    halftone = SANE_FALSE;
    threshold = SANE_FALSE;
    cct = SANE_FALSE;
    break;
  case 2:			/* color */
    dropout = SANE_FALSE;
    aas = SANE_FALSE;
    halftone = SANE_FALSE;
    threshold = SANE_FALSE;
    cct = SANE_TRUE;
    break;
  default:
    return;
  }

  if (s->hw->cmd->level[0] == 'D')
  {
    dropout = SANE_FALSE;
    aas = SANE_FALSE;
    halftone = SANE_FALSE;
  }

  setOptionState (s, dropout, OPT_DROPOUT, reload);
  s->val[OPT_DROPOUT].w = 0;
  setOptionState (s, halftone, OPT_HALFTONE, reload);
  s->val[OPT_HALFTONE].w = 0;
  setOptionState (s, aas, OPT_AAS, reload);
  s->val[OPT_AAS].w = SANE_FALSE;

  setOptionState (s, threshold, OPT_THRESHOLD, reload);

  setOptionState (s, cct, OPT_CCT_1, reload);
  setOptionState (s, cct, OPT_CCT_2, reload);
  setOptionState (s, cct, OPT_CCT_3, reload);
  setOptionState (s, cct, OPT_CCT_4, reload);
  setOptionState (s, cct, OPT_CCT_5, reload);
  setOptionState (s, cct, OPT_CCT_6, reload);
  setOptionState (s, cct, OPT_CCT_7, reload);
  setOptionState (s, cct, OPT_CCT_8, reload);
  setOptionState (s, cct, OPT_CCT_9, reload);

  /* if binary, then disable the bit depth selection */
  if (optindex == 0)
  {
    s->opt[OPT_BIT_DEPTH].cap |= SANE_CAP_INACTIVE;
  }
  else
  {
    if (bitDepthList[0] == 1)
      s->opt[OPT_BIT_DEPTH].cap |= SANE_CAP_INACTIVE;
    else
    {
      s->opt[OPT_BIT_DEPTH].cap &= ~SANE_CAP_INACTIVE;
      s->val[OPT_BIT_DEPTH].w = mode_params[optindex].depth;
    }
  }

  if (optindex == 0)		/* b & w */
    handle_depth_halftone (s, 0, reload);

  *reload = SANE_TRUE;
}

/*--------------------------------------------------------------*/
static void
change_profile_matrix (Epson_Scanner * s)
{
  int index = 0;

  if (!s->hw->scan_hard)
  {				/* fall back on the default */
    s->hw->scan_hard = &epson_scan_hard[0];
  }

  if (s->hw->TPU && s->val[OPT_SOURCE].w == 1)	/* TPU */
  {
    if (s->val[OPT_FILM_TYPE].w == 0)	/* posi */
      index = 3;
    else
      index = 1;
  }
  else				/* Flatbed or ADF */
  {
    index = 0;
  }

  s->val[OPT_CCT_1].w = SANE_FIX (s->hw->scan_hard->color_profile[index][0]);
  s->val[OPT_CCT_2].w = SANE_FIX (s->hw->scan_hard->color_profile[index][1]);
  s->val[OPT_CCT_3].w = SANE_FIX (s->hw->scan_hard->color_profile[index][2]);
  s->val[OPT_CCT_4].w = SANE_FIX (s->hw->scan_hard->color_profile[index][3]);
  s->val[OPT_CCT_5].w = SANE_FIX (s->hw->scan_hard->color_profile[index][4]);
  s->val[OPT_CCT_6].w = SANE_FIX (s->hw->scan_hard->color_profile[index][5]);
  s->val[OPT_CCT_7].w = SANE_FIX (s->hw->scan_hard->color_profile[index][6]);
  s->val[OPT_CCT_8].w = SANE_FIX (s->hw->scan_hard->color_profile[index][7]);
  s->val[OPT_CCT_9].w = SANE_FIX (s->hw->scan_hard->color_profile[index][8]);
}


/*--------------------------------------------------------------*/
static unsigned char
int2cpt (int val)
{
  if (val >= 0)
  {
    if (val > 127)
      val = 127;
    return (unsigned char) val;
  }
  else
  {
    val = -val;
    if (val > 127)
      val = 127;
    return (unsigned char) (0x80 | val);
  }
}


/*--------------------------------------------------------------*/
static void
get_colorcoeff_from_profile (double *profile, unsigned char *color_coeff)
{
  int cc_idx[] = { 4, 1, 7, 3, 0, 6, 5, 2, 8 };
  int color_table[9];
  int i;

  round_cct (profile, color_table);

  for (i = 0; i < 9; i++)
    color_coeff[i] = int2cpt (color_table[cc_idx[i]]);
}


/*--------------------------------------------------------------*/
static void
round_cct (double org_cct[], int rnd_cct[])
{
  int i, j, index;
  double mult_cct[9], frac[9];
  int sum[3];
  int loop;

  for (i = 0; i < 9; i++)
    mult_cct[i] = org_cct[i] * 32;

  for (i = 0; i < 9; i++)
    rnd_cct[i] = (int) floor (org_cct[i] * 32 + 0.5);

  loop = 0;

  do
  {
    for (i = 0; i < 3; i++)
    {
      if ((rnd_cct[i * 3] == 11) &&
	  (rnd_cct[i * 3] == rnd_cct[i * 3 + 1]) &&
	  (rnd_cct[i * 3] == rnd_cct[i * 3 + 2]))
      {
	rnd_cct[i * 3 + i]--;
	mult_cct[i * 3 + i] = rnd_cct[i * 3 + i];
      }
    }

    for (i = 0; i < 3; i++)
    {
      sum[i] = 0;
      for (j = 0; j < 3; j++)
	sum[i] += rnd_cct[i * 3 + j];
    }

    for (i = 0; i < 9; i++)
      frac[i] = mult_cct[i] - rnd_cct[i];

    for (i = 0; i < 3; i++)
    {
      if (sum[i] < 32)
      {
	index = get_roundup_index (&frac[i * 3], 3);
	if (index != -1)
	{
	  rnd_cct[i * 3 + index]++;
	  mult_cct[i * 3 + index] = rnd_cct[i * 3 + index];
	  sum[i]++;
	}
      }
      else if (sum[i] > 32)
      {
	index = get_rounddown_index (&frac[i * 3], 3);
	if (index != -1)
	{
	  rnd_cct[i * 3 + index]--;
	  mult_cct[i * 3 + index] = rnd_cct[i * 3 + index];
	  sum[i]--;
	}
      }
    }
  }
  while ((++loop < 2)
	 && ((sum[0] != 32) || (sum[1] != 32) || (sum[2] != 32)));
}


/*--------------------------------------------------------------*/
static int
get_roundup_index (double frac[], int n)
{
  int i, index = -1;
  double max_val = 0.0;

  for (i = 0; i < n; i++)
  {
    if (frac[i] < 0)
      continue;
    if (max_val < frac[i])
    {
      index = i;
      max_val = frac[i];
    }
  }

  return index;
}


/*--------------------------------------------------------------*/
static int
get_rounddown_index (double frac[], int n)
{
  int i, index = -1;
  double min_val = 1.0;

  for (i = 0; i < n; i++)
  {
    if (frac[i] > 0)
      continue;
    if (min_val > frac[i])
    {
      index = i;
      min_val = frac[i];
    }
  }

  return index;
}

/*--------------------------------------------------------------*/
static void
color_correct (SANE_Byte * data, SANE_Int size, double *profile)
{
  SANE_Int i, width;
  SANE_Byte *r_buf, *g_buf, *b_buf;
  double red, grn, blu;

  width = size / 3;

  for (i = 0; i < size / 3; i++)
  {
    r_buf = data;
    g_buf = data + 1;
    b_buf = data + 2;

    red =
      profile[0] * (*r_buf) + profile[1] * (*g_buf) + profile[2] * (*b_buf);
    grn =
      profile[3] * (*r_buf) + profile[4] * (*g_buf) + profile[5] * (*b_buf);
    blu =
      profile[6] * (*r_buf) + profile[7] * (*g_buf) + profile[8] * (*b_buf);

    if (255. < red)
      red = 255.;
    if (0 > red)
      red = 0.;
    if (255. < grn)
      grn = 255.;
    if (0 > grn)
      grn = 0.;
    if (255. < blu)
      blu = 255.;
    if (0 > blu)
      blu = 0.;

    *data++ = (unsigned char) (red);
    *data++ = (unsigned char) (grn);
    *data++ = (unsigned char) (blu);
  }
}

static void
handle_depth_halftone (Epson_Scanner * s, SANE_Int optindex,
		       SANE_Bool * reload)
/*
    This routine handles common options between OPT_MODE and
    OPT_HALFTONE.  These options are TET (a HALFTONE mode), AAS
    - auto area segmentation, and threshold.  Apparently AAS
    is some method to differentiate between text and photos.
    Or something like that.

    AAS is available when the scan color depth is 1 and the
    halftone method is not TET.

    Threshold is available when halftone is NONE, and depth is 1.
*/
{
  SANE_Bool threshold, aas, dropout;

  *reload = SANE_FALSE;

  switch (halftone_params[optindex])
  {
  case HALFTONE_NONE:
    threshold = SANE_TRUE;
    aas = SANE_TRUE;
    dropout = SANE_TRUE;
    break;
  case HALFTONE_TET:
    threshold = SANE_FALSE;
    aas = SANE_FALSE;
    dropout = SANE_FALSE;
    break;
  default:
    threshold = SANE_FALSE;
    aas = SANE_TRUE;
    dropout = SANE_TRUE;
  }

  setOptionState (s, threshold, OPT_THRESHOLD, reload);
  setOptionState (s, aas, OPT_AAS, reload);
  setOptionState (s, dropout, OPT_DROPOUT, reload);

  *reload = SANE_TRUE;
}

/**
    End of handle_depth_halftone.
**/

/*--------------------------------------------------------------*/
static void
handle_resolution (Epson_Scanner * s, SANE_Int option, void *value)
{
  int n, k = 0, f;
  int min_d = s->hw->res_list[s->hw->res_list_size - 1];
  SANE_Int v = *(SANE_Word *) value;
  SANE_Int best = v;

  int *last = &s->hw->last_res;

  if (s->hw->cmd->level[0] != 'D')
  {
    s->val[option].w = v;
  }
  else
  {
    /* find supported resolution closest to that requested */
    for (n = 0; n < s->hw->res_list_size; n++)
    {
      int d = abs (v - s->hw->res_list[n]);

      if (d < min_d)
      {
	min_d = d;
	k = n;
	best = s->hw->res_list[n];
      }
    }

    /* FIXME? what's this trying to do?  Use a resolution close to the
              last one used if best is far away?  Why???  */
    if ((v != best) && *last)
    {
      for (f = 0; f < s->hw->res_list_size; f++)
	if (*last == s->hw->res_list[f])
	  break;

      if (f != k && f != k - 1 && f != k + 1)
      {
	if (k > f)
	  best = s->hw->res_list[f + 1];
	else if (k < f)
	  best = s->hw->res_list[f - 1];
      }
    }

    *last = best;
    s->val[option].w = (SANE_Word) best;
  }
}


static void
handle_source (Epson_Scanner * s, SANE_Int optindex, char *value)
/*
    Handles setting the source (flatbed, transparency adapter (TPU),
    or auto document feeder (ADF)).

    For newer scanners it also sets the focus according to the 
    glass / TPU settings.
*/
{
  int force_max = SANE_FALSE;
  SANE_Bool dummy;

  /* reset the scanner when we are changing the source setting - 
     this is necessary for the Perfection 1650 */
  if (s->hw->need_reset_on_source_change)
    reset (s);

#if 0
  s->focusOnGlass = SANE_TRUE;	/* this is the default */
#endif

  if (s->val[OPT_SOURCE].w == optindex)
    return;

  s->val[OPT_SOURCE].w = optindex;

  if (s->val[OPT_TL_X].w == s->hw->x_range->min
      && s->val[OPT_TL_Y].w == s->hw->y_range->min
      && s->val[OPT_BR_X].w == s->hw->x_range->max
      && s->val[OPT_BR_Y].w == s->hw->y_range->max)
  {
    force_max = SANE_TRUE;
  }
  if (strcmp (ADF_STR, value) == 0)
  {
    s->hw->x_range = &s->hw->adf_x_range;
    s->hw->y_range = &s->hw->adf_y_range;
    s->hw->use_extension = SANE_TRUE;
    /* disable film type option */
    deactivateOption (s, OPT_FILM_TYPE, &dummy);
    s->val[OPT_FOCUS].w = 0;
    if (s->hw->duplexSupport)
    {
      activateOption (s, OPT_ADF_MODE, &dummy);
    }
    else
    {
      deactivateOption (s, OPT_ADF_MODE, &dummy);
      s->val[OPT_ADF_MODE].w = 0;
    }
  }
  else if (strcmp (TPU_STR, value) == 0)
  {
    s->hw->x_range = &s->hw->tpu_x_range;
    s->hw->y_range = &s->hw->tpu_y_range;
    s->hw->use_extension = SANE_TRUE;
    deactivateOption (s, OPT_ADF_MODE, &dummy);
    deactivateOption (s, OPT_EJECT, &dummy);
    deactivateOption (s, OPT_AUTO_EJECT, &dummy);
  }
  else				/* neither ADF nor TPU active */
  {
    s->hw->x_range = &s->hw->fbf_x_range;
    s->hw->y_range = &s->hw->fbf_y_range;
    s->hw->use_extension = SANE_FALSE;
    s->val[OPT_FOCUS].w = 0;
    deactivateOption (s, OPT_ADF_MODE, &dummy);
  }

  s->val[OPT_TL_X].w = 0;
  s->val[OPT_TL_Y].w = 0;
  s->val[OPT_BR_X].w = s->hw->x_range->max;
  s->val[OPT_BR_Y].w = s->hw->y_range->max;

  qf_params[XtNumber (qf_params) - 1].tl_x = s->hw->x_range->min;
  qf_params[XtNumber (qf_params) - 1].tl_y = s->hw->y_range->min;
  qf_params[XtNumber (qf_params) - 1].br_x = s->hw->x_range->max;
  qf_params[XtNumber (qf_params) - 1].br_y = s->hw->y_range->max;

  s->opt[OPT_BR_X].constraint.range = s->hw->x_range;
  s->opt[OPT_BR_Y].constraint.range = s->hw->y_range;

  /*
     if( s->val[ OPT_TL_X].w < s->hw->x_range->min || force_max)
     s->val[ OPT_TL_X].w = s->hw->x_range->min;

     if( s->val[ OPT_TL_Y].w < s->hw->y_range->min || force_max)
     s->val[ OPT_TL_Y].w = s->hw->y_range->min;

     if( s->val[ OPT_BR_X].w > s->hw->x_range->max || force_max)
     s->val[ OPT_BR_X].w = s->hw->x_range->max;

     if( s->val[ OPT_BR_Y].w > s->hw->y_range->max || force_max)
     s->val[ OPT_BR_Y].w = s->hw->y_range->max;
   */

  change_profile_matrix (s);

  setOptionState (s, s->hw->TPU && s->hw->use_extension,
		  OPT_FILM_TYPE, &dummy);
  setOptionState (s, s->hw->ADF && s->hw->use_extension,
		  OPT_AUTO_EJECT, &dummy);
  setOptionState (s, s->hw->ADF && s->hw->use_extension, OPT_EJECT, &dummy);

  if (s->hw->cmd->set_focus_position)
  {
    if (s->hw->TPU && s->hw->use_extension)
    {
      s->val[OPT_FOCUS].w = 1;
      setOptionState (s, SANE_TRUE, OPT_FOCUS, &dummy);
    }
    else if (s->hw->ADF && s->hw->use_extension)
    {
      s->val[OPT_FOCUS].w = 0;
      setOptionState (s, SANE_FALSE, OPT_FOCUS, &dummy);
    }
    else
    {
      s->val[OPT_FOCUS].w = 0;
      setOptionState (s, SANE_TRUE, OPT_FOCUS, &dummy);
    }
  }

#if 0
  BAY is part of the filmscan device.We are not sure
    if we are really going to support this device in this
    code.Is there an online manual for it ?
    setOptionState (s, s->hw->ADF && s->hw->use_extension, OPT_BAY, &reload);
#endif
}

/**
    End of handle_source.
**/

static void
handle_filmtype (Epson_Scanner * s, SANE_Int optindex, char *value)
{
  int force_max = SANE_FALSE;

  value = value;

  if (s->val[OPT_FILM_TYPE].w == optindex)
    return;

  s->val[OPT_FILM_TYPE].w = optindex;

  if (s->val[OPT_TL_X].w == s->hw->x_range->min
      && s->val[OPT_TL_Y].w == s->hw->y_range->min
      && s->val[OPT_BR_X].w == s->hw->x_range->max
      && s->val[OPT_BR_Y].w == s->hw->y_range->max)
  {
    force_max = SANE_TRUE;
  }

  s->hw->x_range = &s->hw->tpu_x_range;
  s->hw->y_range = &s->hw->tpu_y_range;

  s->val[OPT_TL_X].w = 0;
  s->val[OPT_TL_Y].w = 0;
  s->val[OPT_BR_X].w = s->hw->x_range->max;
  s->val[OPT_BR_Y].w = s->hw->y_range->max;

  qf_params[XtNumber (qf_params) - 1].tl_x = s->hw->x_range->min;
  qf_params[XtNumber (qf_params) - 1].tl_y = s->hw->y_range->min;
  qf_params[XtNumber (qf_params) - 1].br_x = s->hw->x_range->max;
  qf_params[XtNumber (qf_params) - 1].br_y = s->hw->y_range->max;

  s->opt[OPT_BR_X].constraint.range = s->hw->x_range;
  s->opt[OPT_BR_Y].constraint.range = s->hw->y_range;

  change_profile_matrix (s);
}


static SANE_Status
setvalue (SANE_Handle handle, SANE_Int option, void *value, SANE_Int * info)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Option_Descriptor *sopt = &(s->opt[option]);
  Option_Value *sval = &(s->val[option]);

  SANE_Status status;
  const SANE_String_Const *optval;
  int optindex;
  SANE_Bool reload = SANE_FALSE;

  DBG (5, "setvalue(option = %d, value = %p)\n", option, value);

  status = sanei_constrain_value (sopt, value, info);

  if (status != SANE_STATUS_GOOD)
    return status;

  s->option_has_changed = SANE_TRUE;

  optval = NULL;
  optindex = 0;

  if (sopt->constraint_type == SANE_CONSTRAINT_STRING_LIST)
  {
    optval = search_string_list (sopt->constraint.string_list,
				 (char *) value);

    if (optval == NULL)
      return SANE_STATUS_INVAL;
    optindex = optval - sopt->constraint.string_list;
  }

  switch (option)
  {
/* 	case OPT_GAMMA_VECTOR: */
  case OPT_GAMMA_VECTOR_R:
  case OPT_GAMMA_VECTOR_G:
  case OPT_GAMMA_VECTOR_B:
    memcpy (sval->wa, value, sopt->size);	/* Word arrays */
    break;

  case OPT_CCT_1:
  case OPT_CCT_2:
  case OPT_CCT_3:
  case OPT_CCT_4:
  case OPT_CCT_5:
  case OPT_CCT_6:
  case OPT_CCT_7:
  case OPT_CCT_8:
  case OPT_CCT_9:
    sval->w = *((SANE_Word *) value);	/* Simple values */
    break;

  case OPT_FILM_TYPE:
    handle_filmtype (s, optindex, (char *) value);
    reload = SANE_TRUE;
    break;

  case OPT_DROPOUT:
  case OPT_BAY:
  case OPT_FOCUS:
    sval->w = optindex;		/* Simple lists */
    break;

  case OPT_EJECT:
/*		return eject( s ); */
    eject (s);
    break;

  case OPT_RESOLUTION:
    handle_resolution (s, option, value);
    reload = SANE_TRUE;
    break;

  case OPT_TL_X:
  case OPT_TL_Y:
  case OPT_BR_X:
  case OPT_BR_Y:
    sval->w = *((SANE_Word *) value);
    DBG (1, "set = %f\n", SANE_UNFIX (sval->w));
    if (NULL != info)
      *info |= SANE_INFO_RELOAD_PARAMS;
    break;

  case OPT_SOURCE:
    handle_source (s, optindex, (char *) value);
    reload = SANE_TRUE;
    break;

  case OPT_MODE:
    {
      if (sval->w == optindex)
	break;

      sval->w = optindex;

      handle_mode (s, optindex, &reload);

      break;
    }

  case OPT_ADF_MODE:
    sval->w = optindex;
    break;

  case OPT_BIT_DEPTH:
    sval->w = *((SANE_Word *) value);
    mode_params[s->val[OPT_MODE].w].depth = sval->w;
    reload = SANE_TRUE;
    break;

  case OPT_HALFTONE:
    if (sval->w == optindex)
      break;
    sval->w = optindex;
    handle_depth_halftone (s, optindex, &reload);
    break;

  case OPT_COLOR_CORRECTION:
    {
      SANE_Bool f = color_userdefined[optindex];

      sval->w = optindex;
      setOptionState (s, f, OPT_CCT_1, &reload);
      setOptionState (s, f, OPT_CCT_2, &reload);
      setOptionState (s, f, OPT_CCT_3, &reload);
      setOptionState (s, f, OPT_CCT_4, &reload);
      setOptionState (s, f, OPT_CCT_5, &reload);
      setOptionState (s, f, OPT_CCT_6, &reload);
      setOptionState (s, f, OPT_CCT_7, &reload);
      setOptionState (s, f, OPT_CCT_8, &reload);
      setOptionState (s, f, OPT_CCT_9, &reload);

      break;
    }

  case OPT_GAMMA_CORRECTION:
    {
      SANE_Bool f = gamma_userdefined[optindex];

      sval->w = optindex;
/*		setOptionState(s, f, OPT_GAMMA_VECTOR, &reload ); */
      setOptionState (s, f, OPT_GAMMA_VECTOR_R, &reload);
      setOptionState (s, f, OPT_GAMMA_VECTOR_G, &reload);
      setOptionState (s, f, OPT_GAMMA_VECTOR_B, &reload);
      setOptionState (s, !f, OPT_BRIGHTNESS, &reload);	/* Note... */

      break;
    }

  case OPT_MIRROR:
  case OPT_SPEED:
  case OPT_PREVIEW_SPEED:
  case OPT_AAS:
  case OPT_PREVIEW:		/* needed? */
  case OPT_BRIGHTNESS:
  case OPT_SHARPNESS:
  case OPT_AUTO_EJECT:
  case OPT_THRESHOLD:
  case OPT_ZOOM:
  case OPT_WAIT_FOR_BUTTON:
    sval->w = *((SANE_Word *) value);
    break;

  case OPT_LIMIT_RESOLUTION:
    sval->w = *((SANE_Word *) value);
    filter_resolution_list (s);
    reload = SANE_TRUE;
    break;

  case OPT_QUICK_FORMAT:
    sval->w = optindex;

    s->val[OPT_TL_X].w = qf_params[sval->w].tl_x;
    s->val[OPT_TL_Y].w = qf_params[sval->w].tl_y;
    s->val[OPT_BR_X].w = qf_params[sval->w].br_x;
    s->val[OPT_BR_Y].w = qf_params[sval->w].br_y;

    if (s->val[OPT_TL_X].w < s->hw->x_range->min)
      s->val[OPT_TL_X].w = s->hw->x_range->min;

    if (s->val[OPT_TL_Y].w < s->hw->y_range->min)
      s->val[OPT_TL_Y].w = s->hw->y_range->min;

    if (s->val[OPT_BR_X].w > s->hw->x_range->max)
      s->val[OPT_BR_X].w = s->hw->x_range->max;

    if (s->val[OPT_BR_Y].w > s->hw->y_range->max)
      s->val[OPT_BR_Y].w = s->hw->y_range->max;

    reload = SANE_TRUE;
    break;

  case OPT_MONITOR_BUTTON:
  default:
    return SANE_STATUS_INVAL;
  }

  if (reload && info != NULL)
  {
    *info |= SANE_INFO_RELOAD_OPTIONS | SANE_INFO_RELOAD_PARAMS;
  }

  return SANE_STATUS_GOOD;
}

/**
    End of setvalue.
**/

SANE_Status
sane_control_option (SANE_Handle handle,
		     SANE_Int option,
		     SANE_Action action, void *value, SANE_Int * info)
{
  if (option < 0 || option >= NUM_OPTIONS)
    return SANE_STATUS_INVAL;

  if (info != NULL)
    *info = 0;

  switch (action)
  {
  case SANE_ACTION_GET_VALUE:
    return (getvalue (handle, option, value));

  case SANE_ACTION_SET_VALUE:
    return (setvalue (handle, option, value, info));
  default:
    return SANE_STATUS_INVAL;
  }

  return SANE_STATUS_GOOD;
}

/*
 * sane_get_parameters()
 *
 * This function is part of the SANE API and gets called when the front end
 * requests information aobut the scan configuration (e.g. color depth, mode,
 * bytes and pixels per line, number of lines. This information is returned
 * in the SANE_Parameters structure.
 *
 * Once a scan was started, this routine has to report the correct values, if
 * it is called before the scan is actually started, the values are based on
 * the current settings.
 *
 */
SANE_Status
sane_get_parameters (SANE_Handle handle, SANE_Parameters * params)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  int ndpi, zoom, max_x, max_y;
  int bytes_per_pixel;

  DBG (5, "sane_get_parameters()\n");

  /* If sane_start was already called, then just retrieve the parameters
   * from the scanner data structure.
   */

  if (!s->eof && s->ptr != NULL && params != NULL)
  {
    DBG (5, "Returning saved params structure\n");
    *params = s->params;

    DBG (3, "Preview = %d\n", s->val[OPT_PREVIEW].w);
    DBG (3, "Resolution = %d\n", s->val[OPT_RESOLUTION].w);

    DBG (1, "get para %p %p tlx %f tly %f brx %f bry %f [mm]\n", (void *) s,
	 (void *) s->val, SANE_UNFIX (s->val[OPT_TL_X].w),
	 SANE_UNFIX (s->val[OPT_TL_Y].w), SANE_UNFIX (s->val[OPT_BR_X].w),
	 SANE_UNFIX (s->val[OPT_BR_Y].w));

    return SANE_STATUS_GOOD;
  }

  /* otherwise initialize the params structure and gather the data */

  memset (&s->params, 0, sizeof (SANE_Parameters));

  ndpi = s->val[OPT_RESOLUTION].w;
  zoom = s->val[OPT_ZOOM].w;

  max_x = max_y = 0;
  if (s->hw->ADF && s->hw->use_extension && s->hw->cmd->feed)
  {
    max_x =
      (long unsigned long) s->hw->adf_max_x * ndpi * zoom /
      (s->hw->dpi_range.max * 100);
    max_y =
      (long unsigned long) s->hw->adf_max_y * ndpi * zoom /
      (s->hw->dpi_range.max * 100);
  }
  if (s->hw->devtype == 3 && s->hw->use_extension == 0)
  {
    max_x =
      (long unsigned long) s->hw->fbf_max_x * ndpi * zoom /
      (s->hw->dpi_range.max * 100);
    max_y =
      (long unsigned long) s->hw->fbf_max_y * ndpi * zoom /
      (s->hw->dpi_range.max * 100);
  }

  s->params.pixels_per_line =
    SANE_UNFIX (s->val[OPT_BR_X].w -
		s->val[OPT_TL_X].w) / MM_PER_INCH * ndpi * zoom / 100;
  s->params.lines =
    SANE_UNFIX (s->val[OPT_BR_Y].w -
		s->val[OPT_TL_Y].w) / MM_PER_INCH * ndpi * zoom / 100;

  DBG (2, "max x:%d y:%d\n", max_x, max_y);

  if (max_x != 0 && max_y != 0)
  {
    if (max_x < s->params.pixels_per_line)
      s->params.pixels_per_line = max_x;
    if (max_y < s->params.lines)
      s->params.lines = max_y;
  }

  if (s->params.pixels_per_line < 8)
    s->params.pixels_per_line = 8;
  if (s->params.lines < 1)
    s->params.lines = 1;

  DBG (3, "Preview = %d\n", s->val[OPT_PREVIEW].w);
  DBG (3, "Resolution = %d\n", s->val[OPT_RESOLUTION].w);

  DBG (1, "get para %p %p tlx %f tly %f brx %f bry %f [mm]\n", (void *) s,
       (void *) s->val, SANE_UNFIX (s->val[OPT_TL_X].w),
       SANE_UNFIX (s->val[OPT_TL_Y].w), SANE_UNFIX (s->val[OPT_BR_X].w),
       SANE_UNFIX (s->val[OPT_BR_Y].w));

  print_params (s->params);

  /* 
   * Calculate bytes_per_pixel and bytes_per_line for 
   * any color depths.
   * 
   * The default color depth is stored in mode_params.depth:
   */

  if (mode_params[s->val[OPT_MODE].w].depth == 1)
  {
    s->params.depth = 1;
  }
  else
  {
    s->params.depth = s->val[OPT_BIT_DEPTH].w;
  }

  if (s->params.depth > 8)
  {
    s->params.depth = 16;	/* 
				 * The frontends can only handle 8 or 16 bits 
				 * for gray or color - so if it's more than 8,
				 * it gets automatically set to 16. This works
				 * as long as EPSON does not come out with a
				 * scanner that can handle more than 16 bits
				 * per color channel.
				 */

  }

  bytes_per_pixel = s->params.depth / 8;	/* this works because it can only be set to 1, 8 or 16 */
  if (s->params.depth % 8)	/* just in case ... */
  {
    bytes_per_pixel++;
  }

  /* All models require alignment on a multiple of 8 pixels per line.
     However, some models require multiples of 32 pixels instead of 8
     when scanning in monochrome mode.
     We use the largest multiple that is not larger than the original
     value.
   */
  s->params.pixels_per_line &= ~7;
  if (1 == s->params.depth)
  {
    s->params.pixels_per_line &= ~31;
  }

  s->params.last_frame = SANE_TRUE;

  if (mode_params[s->val[OPT_MODE].w].color)
  {
    s->params.format = SANE_FRAME_RGB;
    s->params.bytes_per_line =
      3 * s->params.pixels_per_line * bytes_per_pixel;
  }
  else
  {
    s->params.format = SANE_FRAME_GRAY;
    s->params.bytes_per_line =
      s->params.pixels_per_line * s->params.depth / 8;
  }

  if (NULL != params)
    *params = s->params;

  print_params (s->params);

  return SANE_STATUS_GOOD;
}

/*
 * sane_start()
 *
 * This function is part of the SANE API and gets called from the front end to 
 * start the scan process.
 *
 */

SANE_Status
sane_start (SANE_Handle handle)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Status status;
  SANE_Bool button_status;
  const struct mode_param *mparam;
  u_char params[4];
  int ndpi;
  int left, top;
  int lcount;
  int i, j;			/* loop counter */

  DBG (5, "sane_start()\n");

  status = open_scanner (s);
  if (SANE_STATUS_GOOD != status)
  {
    return status;
  }

  status = reset (s);
  if (SANE_STATUS_GOOD != status)
  {
    close_scanner (s);
    return status;
  }

  /* The 'canceling' member of s may have been set by a call to
     sane_cancel(), but there is NO way the scanner can be in a
     canceling state after open_scanner() AND reset() have just
     been called.
     FIXME? this should really be done by reset(), not?  */
  s->canceling = SANE_FALSE;

/*
 *  There is some undocumented special behavior with the TPU enable/disable.
 *      TPU power	ESC e		status
 *	on		0		NAK
 *	on		1		ACK
 *	off		0		ACK
 *	off		1		NAK
 *
 * It makes no sense to scan with TPU powered on and source flatbed, because 
 * light will come from both sides.
 */

  if (s->hw->extension)
  {
    int max_x, max_y;

    int extensionCtrl;
    extensionCtrl = (s->hw->use_extension ? 1 : 0);
    if (s->hw->use_extension && (s->val[OPT_ADF_MODE].w == 1))
      extensionCtrl = 2;

    status = control_extension (s, extensionCtrl);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "You may have to power %s your TPU\n",
	   s->hw->use_extension ? "on" : "off");

      DBG (1, "Also you may have to restart the Sane frontend.\n");
      close_scanner (s);
      return status;
    }

    if ((0 == strcmp ("ES-10000G", s->hw->sane.model)
	 || 0 == strcmp ("Expression 10000XL", s->hw->sane.model))
	&& s->hw->ADF && s->hw->use_extension)
      check_control_extension (s, &status);

    if (s->hw->cmd->request_extended_status != 0)
    {
      status = check_ext_status (s, &max_x, &max_y);

      if (SANE_STATUS_GOOD != status && SANE_STATUS_DEVICE_BUSY != status)
      {
	close_scanner (s);
	return status;
      }
    }

    if (s->hw->ADF && s->hw->use_extension && s->hw->cmd->feed)
    {
      status = feed (s);
      if (SANE_STATUS_GOOD != status)
      {
	close_scanner (s);
	return status;
      }

      status = check_ext_status (s, &max_x, &max_y);
      s->hw->adf_max_x = max_x;
      s->hw->adf_max_y = max_y;

      if (SANE_STATUS_NO_DOCS == status
	  && (0 == strcmp ("ES-7000H", s->hw->sane.model)
	      || 0 == strcmp ("GT-15000", s->hw->sane.model)
	      || 0 == strcmp ("ES-10000G", s->hw->sane.model)
	      || 0 == strcmp ("Expression 10000XL", s->hw->sane.model)))
      {				/* Some scanners, e.g. the ES-7000H,
				   immediately signal that their ADF
				   unit is out of documents after they
				   have just successfully loaded a
				   page.  This gums up sane_start()
				   eventually because ESC G signals a
				   fatal error.  This ugly work-around
				   at least gets the last page scanned
				   instead of timing out.  The flag is
				   used to let close_scanner() know it
				   has to eject the documents that are
				   still on the bed and is only set
				   here.  */
	u_char params[2];
	u_char result[6];
	params[0] = ESC;
	params[1] = s->hw->cmd->start_scanning;

	send (s, params, 2, &status);
	receive (s, result, s->block ? 6 : 4, &status);
	params[1] = 0x18;	/* CANcel */
	send (s, params, 2, &status);
	expect_ack (s);

	if (0 == strcmp ("ES-10000G", s->hw->sane.model)
	    || 0 == strcmp ("Expression 10000XL", s->hw->sane.model))
	{
	  sleep (5);
	}

	s->adf_has_been_fed = SANE_TRUE;
      }
    }


    /* 
     * set the focus position according to the extension used:
     * if the TPU is selected, then focus 2.5mm above the glass,
     * otherwise focus on the glass. Scanners that don't support
     * this feature, will just ignore these calls.
     */

    if (s->hw->focusSupport == SANE_TRUE)
    {
      if (s->val[OPT_FOCUS].w == 0)
      {
	DBG (1, "Setting focus to glass surface\n");
	status = set_focus_position (s, 0x40);
      }
      else
      {
	DBG (1, "Setting focus to 2.5mm above glass\n");
	status = set_focus_position (s, 0x59);
      }

      if (SANE_STATUS_GOOD != status)
      {
	close_scanner (s);
	return status;
      }
    }
  }

  /* use the flatbed size for the max. scansize for the GT-30000
     and similar scanners if the ADF is not enabled */
  if (s->hw->devtype == 3 && s->hw->use_extension == 0)
  {
    int max_x, max_y;

    status = check_ext_status (s, &max_x, &max_y);
    if (SANE_STATUS_GOOD != status && SANE_STATUS_DEVICE_BUSY != status)
    {
      close_scanner (s);
      return status;
    }

    s->hw->fbf_max_x = max_x;
    s->hw->fbf_max_y = max_y;
  }


  mparam = mode_params + s->val[OPT_MODE].w;
  DBG (1, "sane_start: Setting data format to %d bits\n", mparam->depth);
  status = set_data_format (s, mparam->depth);

  if (SANE_STATUS_GOOD != status)
  {
    DBG (1, "sane_start: set_data_format failed: %s\n",
	 sane_strstatus (status));
    close_scanner (s);
    return status;
  }

  /*
   * The byte sequence mode was introduced in B5, for B[34] we need line sequence mode 
   */

  if ((s->hw->cmd->level[0] == 'D' ||
       (s->hw->cmd->level[0] == 'B' && s->hw->level >= 5)) &&
      mparam->mode_flags == 0x02)
  {
    status = set_color_mode (s, 0x13);
  }
  else
  {
    status = set_color_mode (s, mparam->mode_flags | (mparam->dropout_mask
						      & dropout_params[s->
								       val
								       [OPT_DROPOUT].
								       w]));
  }

  if (SANE_STATUS_GOOD != status)
  {
    DBG (1, "sane_start: set_color_mode failed: %s\n",
	 sane_strstatus (status));
    close_scanner (s);
    return status;
  }

  if (s->hw->cmd->set_halftoning &&
      SANE_OPTION_IS_ACTIVE (s->opt[OPT_HALFTONE].cap))
  {
    status = set_halftoning (s, halftone_params[s->val[OPT_HALFTONE].w]);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_halftoning failed: %s\n",
	   sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }


  /*
     if (SANE_OPTION_IS_ACTIVE( s->opt[ OPT_BRIGHTNESS].cap) ) 
     {
     status = set_bright( s, s->val[OPT_BRIGHTNESS].w);

     if( SANE_STATUS_GOOD != status) 
     {
     DBG( 1, "sane_start: set_bright failed: %s\n", sane_strstatus( status));
     close_scanner( s);
     return status;
     }
     }
   */

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_MIRROR].cap))
  {
    status = mirror_image (s, mirror_params[s->val[OPT_MIRROR].w]);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: mirror_image failed: %s\n",
	   sane_strstatus (status));
      close_scanner (s);
      return status;
    }

  }

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_SPEED].cap))
  {

    if (s->val[OPT_PREVIEW].w)
      status = set_speed (s, speed_params[s->val[OPT_PREVIEW_SPEED].w]);
    else
      status = set_speed (s, speed_params[s->val[OPT_SPEED].w]);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_speed failed: %s\n", sane_strstatus (status));
      close_scanner (s);
      return status;
    }

  }

/*
 *  use of speed_params is ok here since they are false and true.
 *  NOTE: I think I should throw that "params" stuff as long w is already the value.
 */
  /*
     if (SANE_OPTION_IS_ACTIVE( s->opt[ OPT_AAS].cap) ) 
     {
     status = control_auto_area_segmentation(s, 
     speed_params[ s->val[ OPT_AAS].w]);

     if( SANE_STATUS_GOOD != status) 
     {
     DBG( 1, "sane_start: control_auto_area_segmentation failed: %s\n", 
     sane_strstatus( status));
     close_scanner( s);
     return status;
     }
     }
   */

  s->invert_image = SANE_FALSE;	/* default: to not inverting the image */

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_FILM_TYPE].cap))
  {
#if 0
    s->invert_image = (s->val[OPT_FILM_TYPE].w == FILM_TYPE_NEGATIVE);
#endif
    status = set_film_type (s, film_params[s->val[OPT_FILM_TYPE].w]);
    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_film_type failed: %s\n",
	   sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_BAY].cap))
  {
    status = set_bay (s, s->val[OPT_BAY].w);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_bay: %s\n", sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

  if (SANE_OPTION_IS_ACTIVE (s->opt[OPT_SHARPNESS].cap))
  {

    status = set_outline_emphasis (s, s->val[OPT_SHARPNESS].w);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_outline_emphasis failed: %s\n",
	   sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

  if (s->hw->cmd->set_gamma &&
      SANE_OPTION_IS_ACTIVE (s->opt[OPT_GAMMA_CORRECTION].cap))
  {
    int val;
    if (s->hw->cmd->level[0] == 'D')
    {
      /*
       * The D1 level has only the two user defined gamma 
       * settings.
       */
      val = gamma_params[s->val[OPT_GAMMA_CORRECTION].w];
    }
    else
    {
      val = gamma_params[s->val[OPT_GAMMA_CORRECTION].w];

      /*
       * If "Default" is selected then determine the actual value
       * to send to the scanner: If bilevel mode, just send the 
       * value from the table (0x01), for grayscale or color mode 
       * add one and send 0x02.
       */
/*			if( s->val[ OPT_GAMMA_CORRECTION].w <= 1) { */
      if (s->val[OPT_GAMMA_CORRECTION].w == 0)
      {
	val += mparam->depth == 1 ? 0 : 1;
      }
    }

    DBG (1, "sane_start: set_gamma( s, 0x%x ).\n", val);
    status = set_gamma (s, val);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_gamma failed: %s\n", sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

  if (s->hw->cmd->set_gamma_table &&
      gamma_userdefined[s->val[OPT_GAMMA_CORRECTION].w])
  {				/* user defined. */
    status = set_gamma_table (s);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_gamma_table failed: %s\n",
	   sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

/*
 * TODO: think about if SANE_OPTION_IS_ACTIVE is a good criteria to send commands.
 */

  if (s->hw->cmd->set_color_correction &&
      s->hw->cmd->level[0] != 'D' &&
      SANE_OPTION_IS_ACTIVE (s->opt[OPT_COLOR_CORRECTION].cap))
  {
    int val = color_params[s->val[OPT_COLOR_CORRECTION].w];

    DBG (1, "sane_start: set_color_correction( s, 0x%x )\n", val);
    status = set_color_correction (s, val);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_color_correction failed: %s\n",
	   sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }
  if (s->hw->cmd->set_color_correction_coefficients &&
      s->hw->cmd->level[0] != 'D' && 1 == s->val[OPT_COLOR_CORRECTION].w)
  {				/* user defined. */
    status = set_color_correction_coefficients (s);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_color_correction_coefficients failed: %s\n",
	   sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

  {
    int i;
    double cct[9];

    cct[0] = SANE_UNFIX (s->val[OPT_CCT_1].w);
    cct[1] = SANE_UNFIX (s->val[OPT_CCT_2].w);
    cct[2] = SANE_UNFIX (s->val[OPT_CCT_3].w);
    cct[3] = SANE_UNFIX (s->val[OPT_CCT_4].w);
    cct[4] = SANE_UNFIX (s->val[OPT_CCT_5].w);
    cct[5] = SANE_UNFIX (s->val[OPT_CCT_6].w);
    cct[6] = SANE_UNFIX (s->val[OPT_CCT_7].w);
    cct[7] = SANE_UNFIX (s->val[OPT_CCT_8].w);
    cct[8] = SANE_UNFIX (s->val[OPT_CCT_9].w);

    for (i = 0; i < 9; i++)
      DBG (1, "cct [ %d ] = %f\n", i, cct[i]);
  }

  if (s->hw->cmd->set_threshold != 0
      && SANE_OPTION_IS_ACTIVE (s->opt[OPT_THRESHOLD].cap))
  {
    status = set_threshold (s, s->val[OPT_THRESHOLD].w);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_threshold(%d) failed: %s\n",
	   s->val[OPT_THRESHOLD].w, sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

  /* Both the GT-9400 and GT-F600 can't scan at the highest
     resolutions that the ESC I command has you believe are
     supported.  Rather than cause an error, we cap the re-
     solutions by force to their max.

     When used with iscan, you can still scan at the higher
     resolutions because the frontend will resize.

     I know this is ugly, but it is the only way that works
     without a redesign.  You will need separate resolution
     tables (that can't be had from the hardware!) for ADF,
     TPU and flatbed scanning.  Moreover, you would need to
     distinguish main and sub scan resolutions too.
   */

  if (0x0116 == s->hw->productID	/* GT-9400 */
      && s->hw->ADF && s->hw->use_extension
      && s->val[OPT_RESOLUTION].w >= 1600 ) 
  {
    ndpi = 1600;
  }
  else if (0x0118 == s->hw->productID	/* GT-F600 */
	   && s->hw->ADF && s->hw->use_extension
	   && s->val[OPT_RESOLUTION].w >= 1800)
  {
    ndpi = 1800;
  }
  else if (0x012b == s->hw->productID	/* GT-2500/ES-H300 */
	   && s->hw->ADF && s->hw->use_extension
	   && s->val[OPT_RESOLUTION].w >= 600)
  {
    ndpi = 600;
    s->val[OPT_RESOLUTION].w = ndpi;
  }
  else
  {
    ndpi = s->val[OPT_RESOLUTION].w;
  }

  status = set_resolution (s, ndpi, ndpi);

  if (SANE_STATUS_GOOD != status)
  {
    DBG (1, "sane_start: set_resolution(%d, %d) failed: %s\n",
	 ndpi, ndpi, sane_strstatus (status));
    close_scanner (s);
    return status;
  }

  /* set the zoom */
  if (s->hw->cmd->set_zoom != 0
      && SANE_OPTION_IS_ACTIVE (s->opt[OPT_ZOOM].cap))
  {
    status = set_zoom (s, s->val[OPT_ZOOM].w, s->val[OPT_ZOOM].w);
    if (status != SANE_STATUS_GOOD)
    {
      DBG (1, "sane_start: set_zoom(%d) failed: %s\n",
	   s->val[OPT_ZOOM].w, sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

  /* recompute and (mostly) finalise parameters to be spec compliant
     Note that for scanners that require colour shuffling, we need to
     modify params.lines a little and do that further below.  */
  status = sane_get_parameters (handle, NULL);

  if (status != SANE_STATUS_GOOD)
  {
    close_scanner (s);
    return status;
  }


/*
 * If WAIT_FOR_BUTTON is active, then do just that: Wait until the button is 
 * pressed. If the button was already pressed, then we will get the button
 * Pressed event right away.
 */

  if (s->val[OPT_WAIT_FOR_BUTTON].w == SANE_TRUE)
  {
    s->hw->wait_for_button = SANE_TRUE;

    while (s->hw->wait_for_button == SANE_TRUE)
    {
      if (s->canceling == SANE_TRUE)
      {
	s->hw->wait_for_button = SANE_FALSE;
      }
      /* get the button status from the scanner */
      else if (request_push_button_status (s, &button_status) ==
	       SANE_STATUS_GOOD)
      {
	if (button_status == SANE_TRUE)
	{
	  s->hw->wait_for_button = SANE_FALSE;
	}
	else
	{
	  sleep (1);
	}
      }
      else
      {
	/* we run into an eror condition, just continue */
	s->hw->wait_for_button = SANE_FALSE;
      }
    }
  }


/*
 *  in file:frontend/preview.c
 *
 *  The preview strategy is as follows:
 *
 *   1) A preview always acquires an image that covers the entire
 *      scan surface.  This is necessary so the user can see not
 *      only what is, but also what isn't selected.
 */

  left =
    SANE_UNFIX (s->val[OPT_TL_X].w) / MM_PER_INCH * ndpi *
    s->val[OPT_ZOOM].w / 100 + 0.5;
  top =
    SANE_UNFIX (s->val[OPT_TL_Y].w) / MM_PER_INCH * ndpi *
    s->val[OPT_ZOOM].w / 100 + 0.5;

  /*
   * Calculate correction for line_distance in D1 scanner:
   * Start line_distance lines earlier and add line_distance lines at the end
   *
   * Because the actual line_distance is not yet calculated we have to do this
   * first.
   */

  s->hw->color_shuffle = SANE_FALSE;
  s->current_output_line = 0;
  s->lines_written = 0;
  s->color_shuffle_line = 0;

  if ((s->hw->optical_res != 0) && (mparam->depth == 8)
      && (mparam->mode_flags != 0))
  {
    s->line_distance = s->hw->max_line_distance * ndpi / s->hw->optical_res;
    if (s->line_distance != 0)
    {
      s->hw->color_shuffle = SANE_TRUE;
    }
    else
      s->hw->color_shuffle = SANE_FALSE;
  }

  {				/* finalise the number of scanlines */
    int max_lines;
    int lines = s->params.lines;

    max_lines = (SANE_UNFIX(s->hw->y_range->max)
		 * s->val[OPT_RESOLUTION].w * s->val[OPT_ZOOM].w / 100
		 / MM_PER_INCH) + 0.5;

    /* add extra lines at the top and bottom to minimise the loss of
       scanlines due to colour shuffling */
    if (SANE_TRUE == s->hw->color_shuffle)
    {
      top   -= 1 * s->line_distance;
      lines += 2 * s->line_distance;
    }

    /* make sure values are within range
       In the worst case, this chomps 2 * s->line_distance lines from
       the area the user wanted to scan.  C'est la vie.  */
    top   = max (0, top);
    lines = min (lines, max_lines - top);

    status = set_scan_area (s, left, top,
			    s->params.pixels_per_line, lines);

    /* substract the additional lines needed for colour shuffling so
       the frontend can know how many lines to expect */
    if (SANE_TRUE == s->hw->color_shuffle)
    {
      lines -= 2 * s->line_distance;
    }
    s->params.lines = lines;

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_scan_area failed: %s\n",
	   sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

  s->block = SANE_FALSE;
  lcount = 1;

  /*
   * The set line count commands needs to be sent for certain scanners in 
   * color mode. The D1 level requires it, we are however only testing for
   * 'D' and not for the actual numeric level.
   */

  if (((s->hw->cmd->level[0] == 'B') &&
       ((s->hw->level >= 5) || ((s->hw->level >= 4) &&
				(!mode_params[s->val[OPT_MODE].w].color))))
      || (s->hw->cmd->level[0] == 'D'))
  {
    s->block = SANE_TRUE;
    lcount = sanei_scsi_max_request_size / s->params.bytes_per_line;

    if (lcount > 255)
    {
      lcount = 255;
    }

    if (s->hw->TPU && s->hw->use_extension && lcount > 32)
    {
      lcount = 32;
    }


    /*
     * The D1 series of scanners only allow an even line number
     * for bi-level scanning. If a bit depth of 1 is selected, then
     * make sure the next lower even number is selected.
     */
#if 0
    if (s->hw->cmd->level[0] == 'D')
#endif
    {
      if (3 < lcount && lcount % 2)
      {
	lcount -= 1;
      }
    }

    if (lcount == 0)
    {
      DBG (1, "out of memory (line %d)\n", __LINE__);
      close_scanner (s);
      return SANE_STATUS_NO_MEM;
    }

    status = set_lcount (s, lcount);

    if (SANE_STATUS_GOOD != status)
    {
      DBG (1, "sane_start: set_lcount(%d) failed: %s\n",
	   lcount, sane_strstatus (status));
      close_scanner (s);
      return status;
    }
  }

#if 0
  if (s->hw->cmd->request_extended_status != 0
      && SANE_TRUE == s->hw->extension)
  {
    u_char result[4];
    u_char *buf;
    size_t len;

    params[0] = ESC;
    params[1] = s->hw->cmd->request_extended_status;

    send (s, params, 2, &status);	/* send ESC f (request extended status) */

    if (SANE_STATUS_GOOD == status)
    {
      len = 4;			/* receive header */

      receive (s, result, len, &status);
      if (SANE_STATUS_GOOD != status)
      {
	close_scanner (s);
	return status;
      }

      fix_up_extended_status_reply (s->hw->sane.model, buf);

      len = result[3] << 8 | result[2];
      buf = alloca (len);

      receive (s, buf, len, &status);	/* reveive actual status data */

      if (buf[0] & EXT_STATUS_FER)
      {
	close_scanner (s);
	return SANE_STATUS_INVAL;
      }
    }
    else
    {
      DBG (1, "Extended status flag request failed\n");
    }
  }
#endif

/*
 *  for debug purpose
 *  check scanner conditions
 */
#if 1
  if (s->hw->cmd->request_condition != 0)
  {
    u_char result[4];
    u_char *buf;
    size_t len;

    params[0] = ESC;
    params[1] = s->hw->cmd->request_condition;

    send (s, params, 2, &status);	/* send request condition */

    if (SANE_STATUS_GOOD != status)
    {
      close_scanner (s);
      return status;
    }

    len = 4;
    receive (s, result, len, &status);

    if (SANE_STATUS_GOOD != status)
    {
      close_scanner (s);
      return status;
    }

    len = result[3] << 8 | result[2];
    buf = alloca (len);
    receive (s, buf, len, &status);

    if (SANE_STATUS_GOOD != status)
    {
      close_scanner (s);
      return status;
    }

#if 0
    DBG (10, "SANE_START: length=%d\n", len);
    for (i = 1; i <= len; i++)
    {
      DBG (10, "SANE_START: %d: %c\n", i, buf[i - 1]);
    }
#endif

    DBG (5, "SANE_START: Color: %d\n", (int) buf[1]);
    DBG (5, "SANE_START: Resolution (x, y): (%d, %d)\n",
	 (int) (buf[4] << 8 | buf[3]), (int) (buf[6] << 8 | buf[5]));
    DBG (5,
	 "SANE_START: Scan area(pixels) (x0, y0), (x1, y1): (%d, %d), (%d, %d)\n",
	 (int) (buf[9] << 8 | buf[8]), (int) (buf[11] << 8 | buf[10]),
	 (int) (buf[13] << 8 | buf[12]), (int) (buf[15] << 8 | buf[14]));
    DBG (5, "SANE_START: Data format: %d\n", (int) buf[17]);
    DBG (5, "SANE_START: Halftone: %d\n", (int) buf[19]);
    DBG (5, "SANE_START: Brightness: %d\n", (int) buf[21]);
    DBG (5, "SANE_START: Gamma: %d\n", (int) buf[23]);
    DBG (5, "SANE_START: Zoom (x, y): (%d, %d)\n", (int) buf[26],
	 (int) buf[25]);
    DBG (5, "SANE_START: Color correction: %d\n", (int) buf[28]);
    DBG (5, "SANE_START: Sharpness control: %d\n", (int) buf[30]);
    DBG (5, "SANE_START: Scanning mode: %d\n", (int) buf[32]);
    DBG (5, "SANE_START: Mirroring: %d\n", (int) buf[34]);
    DBG (5, "SANE_START: Auto area segmentation: %d\n", (int) buf[36]);
    DBG (5, "SANE_START: Threshold: %d\n", (int) buf[38]);
    DBG (5, "SANE_START: Line counter: %d\n", (int) buf[40]);
    DBG (5, "SANE_START: Option unit control: %d\n", (int) buf[42]);
    DBG (5, "SANE_START: Film type: %d\n", (int) buf[44]);
  }
#endif


  /* set the retry count to 0 */
  s->retry_count = 0;

  if (s->hw->color_shuffle == SANE_TRUE)
  {

    /* initialize the line buffers */
    for (i = 0; i < s->line_distance * 2 + 1; i++)
    {
      if (s->line_buffer[i] != NULL)
	delete (s->line_buffer[i]);

      s->line_buffer[i] = malloc (s->params.bytes_per_line);
      if (s->line_buffer[i] == NULL)
      {
	/* free the memory we've malloced so far */
	for (j = 0; j < i; j++)
	{
	  delete (s->line_buffer[j]);
	  s->line_buffer[j] = NULL;
	}
	DBG (1, "out of memory (line %d)\n", __LINE__);
	close_scanner (s);
	return SANE_STATUS_NO_MEM;
      }
    }
  }

  if (s->hw->interpreter)
  {
    status = s->hw->interpreter->ftor1 (s->hw, &s->params,
					mparam->depth, left, ndpi);
    if (SANE_STATUS_GOOD != status)
    {
      close_scanner (s);
      return status;
    }
  }

  if (SANE_STATUS_GOOD != (status = check_warmup (s))
      && !(SANE_STATUS_NO_DOCS == status
	   && s->hw->ADF && s->hw->use_extension))
  {
    close_scanner (s);
    return status;
  }

  params[0] = ESC;
  params[1] = s->hw->cmd->start_scanning;

  send (s, params, 2, &status);

  if (SANE_STATUS_GOOD != status)
  {
    DBG (1, "sane_start: start failed: %s\n", sane_strstatus (status));
    close_scanner (s);
    return status;
  }

  s->eof = SANE_FALSE;
  s->buf = realloc (s->buf, lcount * s->params.bytes_per_line);
  s->ptr = s->end = s->buf;
  s->canceling = SANE_FALSE;

  return SANE_STATUS_GOOD;
}				/* sane_start */

/*
 *
 * TODO: clean up the eject and direct cmd mess.
 */

SANE_Status
sane_auto_eject (Epson_Scanner * s)
{

  DBG (5, "sane_auto_eject()\n");

  if (s->hw->ADF && s->hw->use_extension && s->val[OPT_AUTO_EJECT].w)
  {				/* sequence! */
    SANE_Status status;

    u_char params[1];
    u_char cmd = s->hw->cmd->eject;

    if (!cmd)
      return SANE_STATUS_UNSUPPORTED;

    params[0] = cmd;

    send (s, params, 1, &status);

    if (SANE_STATUS_GOOD != (status = expect_ack (s)))
    {
      close_scanner (s);
      return status;
    }
  }

  return SANE_STATUS_GOOD;
}

/*
 *
 *
 */

static SANE_Status
read_data_block (Epson_Scanner * s, EpsonDataRec * result)
{
  SANE_Status status;
  u_char param[3];

  const int limit = 30;
        int ticks;

  receive (s, result, s->block ? 6 : 4, &status);

  if (SANE_STATUS_GOOD != status)
    return status;

  if (STX != result->code)
  {
    DBG (1, "code   %02x\n", (int) result->code);
    DBG (1, "error, expected STX\n");

    return SANE_STATUS_INVAL;
  }

  /* Although spec compliant scanners have their warming up bit set
     when they are getting ready for a scan, the world is less than
     perfect :-(

     We hack around non-compliant behaviour by looping until 'ESC G'
     succeeds or our patience runs out.  In the latter case we just
     claim that the device is busy.

     However, when the ADF function is used, and the last paper is
     scanned, we do not need looping any more.
   */
  ticks = 0;
  while (result->status & (STATUS_FER | STATUS_NOT_READY)
	 && !(s->hw->ADF && s->hw->use_extension
	      && (result->status & STATUS_AREA_END))
	 && ticks++ < limit)
  {
    DBG (1, "fatal error - Status = %02x\n", result->status);

    if (SANE_STATUS_GOOD != (status = check_warmup (s)))
      return status;

    param[0] = ESC;
    param[1] = s->hw->cmd->start_scanning;

    send (s, param, 2, &status);
    sleep (1);
    receive (s, result, s->block ? 6 : 4, &status);
  }

  return (ticks < limit
	  ? status
	  : SANE_STATUS_DEVICE_BUSY);
}

/*
 *
 *
 */


void
scan_finish (Epson_Scanner * s)
{
  int i, x, y;

  DBG (5, "scan_finish()\n");

  delete (s->buf);
  s->buf = 0;

  if (s->hw->ADF && s->hw->use_extension)
    {
      if (s->canceling
	  || SANE_STATUS_NO_DOCS == check_ext_status (s, &x, &y))
	sane_auto_eject (s);
    }
  close_scanner (s);

  for (i = 0; i < s->line_distance; i++)
  {
    if (s->line_buffer[i] != 0)
    {
      delete (s->line_buffer[i]);
      s->line_buffer[i] = 0;
    }
  }

  if (s->hw->interpreter)	/* FIXME: do we really need this?  */
  {
    s->hw->interpreter->free (s->hw);
  }
}

#define GET_COLOR(x)	((x.status>>2) & 0x03)

SANE_Status
sane_read (SANE_Handle handle, SANE_Byte * data, SANE_Int max_length,
	   SANE_Int * length)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Status status;
  int index = 0;
  SANE_Bool reorder = SANE_FALSE;
  SANE_Bool needStrangeReorder = SANE_FALSE;
  int bytes_to_process = 0;

  if (s->buf == 0 && s->canceling)
    return SANE_STATUS_CANCELLED;

START_READ:
  DBG (5, "sane_read: begin\n");

  if (s->ptr == s->end)
  {
    EpsonDataRec result;
    size_t buf_len, byte_counter, line_counter;

    line_counter = 1;

    if (s->eof)
    {
      *length = 0;
      close_scanner (s);
      return SANE_STATUS_EOF;
    }


    DBG (5, "sane_read: begin scan1\n");

    if (SANE_STATUS_GOOD != (status = read_data_block (s, &result)))
    {
      *length = 0;
      scan_finish (s);
      return status;
    }

    byte_counter = result.buf[1] << 8 | result.buf[0];

    if (s->block)
    {
      line_counter *= (result.buf[3] << 8 | result.buf[2]);
    }

    buf_len = byte_counter * line_counter;
    DBG (5, "sane_read: buf len (adjusted) = %lu\n", (u_long) buf_len);

    if (!s->block && SANE_FRAME_RGB == s->params.format)
    {
      /*
       * Read color data in line mode
       */


      /* 
       * read the first color line - the number of bytes to read
       * is already known (from last call to read_data_block()
       * We determine where to write the line from the color information
       * in the data block. At the end we want the order RGB, but the
       * way the data is delivered does not guarantee this - actually it's
       * most likely that the order is GRB if it's not RGB!
       */
      switch (GET_COLOR (result))
      {
      case 1:
	index = 1;
	break;
      case 2:
	index = 0;
	break;
      case 3:
	index = 2;
	break;
      }

      receive (s, s->buf + index * s->params.pixels_per_line, buf_len,
	       &status);

      if (SANE_STATUS_GOOD != status)
      {
	*length = 0;
	scan_finish (s);
	return status;
      }

      /* 
       * send the ACK signal to the scanner in order to make
       * it ready for the next data block.
       */
      send (s, S_ACK, 1, &status);

      /*
       * ... and request the next data block
       */
      if (SANE_STATUS_GOOD != (status = read_data_block (s, &result)))
      {
	*length = 0;
	scan_finish (s);
	return status;
      }

      buf_len = result.buf[1] << 8 | result.buf[0];
      /*
       * this should never happen, because we are already in
       * line mode, but it does not hurt to check ...
       */
      if (s->block)
	buf_len *= (result.buf[3] << 8 | result.buf[2]);

      DBG (5, "sane_read: buf len2 = %lu\n", (u_long) buf_len);

      switch (GET_COLOR (result))
      {
      case 1:
	index = 1;
	break;
      case 2:
	index = 0;
	break;
      case 3:
	index = 2;
	break;
      }

      receive (s, s->buf + index * s->params.pixels_per_line, buf_len,
	       &status);

      if (SANE_STATUS_GOOD != status)
      {
	*length = 0;
	scan_finish (s);
	return status;
      }

      send (s, S_ACK, 1, &status);

      /*
       * ... and the last data block
       */
      if (SANE_STATUS_GOOD != (status = read_data_block (s, &result)))
      {
	*length = 0;
	scan_finish (s);
	return status;
      }

      buf_len = result.buf[1] << 8 | result.buf[0];

      if (s->block)
	buf_len *= (result.buf[3] << 8 | result.buf[2]);

      DBG (5, "sane_read: buf len3 = %lu\n", (u_long) buf_len);

      switch (GET_COLOR (result))
      {
      case 1:
	index = 1;
	break;
      case 2:
	index = 0;
	break;
      case 3:
	index = 2;
	break;
      }

      receive (s, s->buf + index * s->params.pixels_per_line, buf_len,
	       &status);

      if (SANE_STATUS_GOOD != status)
      {
	*length = 0;
	scan_finish (s);
	return status;
      }
    }
    else
    {
      /*
       * Read data in block mode
       */

      /* do we have to reorder the data ? */
      if (GET_COLOR (result) == 0x01)
      {
	reorder = SANE_TRUE;
      }

      bytes_to_process = receive (s, s->buf, buf_len, &status);

      /* bytes_to_process = buf_len; */

      if (SANE_STATUS_GOOD != status)
      {
	*length = 0;
	scan_finish (s);
	return status;
      }
    }

    if (result.status & STATUS_AREA_END)
    {
      s->eof = SANE_TRUE;
    }
    else
    {
      if (s->canceling)
      {
	send (s, S_CAN, 1, &status);
	expect_ack (s);

	*length = 0;

	scan_finish (s);

	return SANE_STATUS_CANCELLED;
      }
      else
	send (s, S_ACK, 1, &status);
    }

    s->end = s->buf + buf_len;
    s->ptr = s->buf;

    /*
     * if we have to re-order the color components (GRB->RGB) we
     * are doing this here:
     */

    /*
     * Some scanners (e.g. the Perfection 1640 and GT-2200) seem
     * to have the R and G channels swapped.
     * The GT-8700 is the Asian version of the Perfection1640.
     * If the scanner name is one of these, and the scan mode is
     * RGB then swap the colors.
     */

    needStrangeReorder =
      (strstr (s->hw->sane.model, "GT-2200") ||
       ((strstr (s->hw->sane.model, "1640") &&
	 strstr (s->hw->sane.model, "Perfection")) ||
	strstr (s->hw->sane.model, "GT-8700"))) &&
      s->params.format == SANE_FRAME_RGB;

    /*
     * Certain Perfection 1650 also need this re-ordering of the two 
     * color channels. These scanners are identified by the problem 
     * with the half vertical scanning area. When we corrected this, 
     * we also set the variable s->hw->need_color_reorder
     */
    if (s->hw->need_color_reorder)
    {
      needStrangeReorder = SANE_TRUE;
    }

    if (needStrangeReorder)
      reorder = SANE_FALSE;	/* reordering once is enough */

    if (s->params.format != SANE_FRAME_RGB)
      reorder = SANE_FALSE;	/* don't reorder for BW or gray */

    if (reorder)
    {
      SANE_Byte *ptr;

      ptr = s->buf;
      while (ptr < s->end)
      {
	if (s->params.depth > 8)
	{
	  SANE_Byte tmp;

	  /* R->G G->R */
	  tmp = ptr[0];
	  ptr[0] = ptr[2];	/* first Byte G */
	  ptr[2] = tmp;		/* first Byte R */

	  tmp = ptr[1];
	  ptr[1] = ptr[3];	/* second Byte G */
	  ptr[3] = tmp;		/* second Byte R */

	  ptr += 6;		/* go to next pixel */
	}
	else
	{
	  /* R->G G->R */
	  SANE_Byte tmp;

	  tmp = ptr[0];
	  ptr[0] = ptr[1];	/* G */
	  ptr[1] = tmp;		/* R */
	  /* B stays the same */
	  ptr += 3;		/* go to next pixel */
	}
      }
    }

    /* 
     * Do the color_shuffle if everything else is correct - at this time
     * most of the stuff is hardcoded for the Perfection 610
     */
    if (s->hw->color_shuffle)
    {
      int new_length = 0;

      status = color_shuffle (s, &new_length);

      /* 
       * If no bytes are returned, check if the scanner is already done, if so, 
       * we'll probably just return, but if there is more data to process get
       * the next batch.
       */

      if (new_length == 0 && s->end != s->ptr)
      {
	goto START_READ;
      }

      s->end = s->buf + new_length;
      s->ptr = s->buf;

    }

    if (s->params.format == SANE_FRAME_RGB &&
	s->hw->cmd->set_color_correction_coefficients &&
	s->hw->cmd->level[0] == 'D')
    {
      double cct[9];

      cct[0] = SANE_UNFIX (s->val[OPT_CCT_1].w);
      cct[1] = SANE_UNFIX (s->val[OPT_CCT_2].w);
      cct[2] = SANE_UNFIX (s->val[OPT_CCT_3].w);
      cct[3] = SANE_UNFIX (s->val[OPT_CCT_4].w);
      cct[4] = SANE_UNFIX (s->val[OPT_CCT_5].w);
      cct[5] = SANE_UNFIX (s->val[OPT_CCT_6].w);
      cct[6] = SANE_UNFIX (s->val[OPT_CCT_7].w);
      cct[7] = SANE_UNFIX (s->val[OPT_CCT_8].w);
      cct[8] = SANE_UNFIX (s->val[OPT_CCT_9].w);

      color_correct (s->ptr, s->end - s->ptr, cct);
    }

    if (s->hw->interpreter)
    {
      s->hw->interpreter->ftor0 (s->hw, &s->params, s->ptr, s->end);
    }

    DBG (5, "sane_read: begin scan2\n");
  }



  /* 
   * copy the image data to the data memory area
   */

  if (!s->block && SANE_FRAME_RGB == s->params.format)
  {

    max_length /= 3;

    if (max_length > s->end - s->ptr)
      max_length = s->end - s->ptr;

    *length = 3 * max_length;

    if (s->canceling)
      s->ptr += max_length;
    else
    {
      if (s->invert_image == SANE_TRUE)
      {
	while (max_length-- != 0)
	{
	  /* invert the three values */
	  *data++ = (u_char) ~ (s->ptr[0]);
	  *data++ = (u_char) ~ (s->ptr[s->params.pixels_per_line]);
	  *data++ = (u_char) ~ (s->ptr[2 * s->params.pixels_per_line]);
	  ++s->ptr;
	}
      }
      else
      {
	while (max_length-- != 0)
	{
	  *data++ = s->ptr[0];
	  *data++ = s->ptr[s->params.pixels_per_line];
	  *data++ = s->ptr[2 * s->params.pixels_per_line];
	  ++s->ptr;
	}
      }
    }
  }
  else
  {
    if (max_length > s->end - s->ptr)
      max_length = s->end - s->ptr;

    *length = max_length;

    if (s->canceling)
      s->ptr += max_length;
    else
    {
      if (1 == s->params.depth)
      {
	if (s->invert_image == SANE_TRUE)
	{
	  while (max_length-- != 0)
	    *data++ = *s->ptr++;
	}
	else
	{
	  while (max_length-- != 0)
	    *data++ = ~*s->ptr++;
	}
      }
      else
      {

	if (s->invert_image == SANE_TRUE)
	{
	  int i;

	  for (i = 0; i < max_length; i++)
	  {
	    data[i] = (u_char) ~ (s->ptr[i]);
	  }
	}
	else
	{
	  memcpy (data, s->ptr, max_length);
	}
	s->ptr += max_length;
      }
    }
  }

  if (s->ptr == s->end && s->eof)
  {
    scan_finish (s);
  }
  DBG (5, "sane_read: end\n");

  return SANE_STATUS_GOOD;
}


/*! Puts raw scan data into the correct scan line.

    When scanning with a non-zero line distance, the RGB data is \e
    not on a single line in the raw scan data.  The RGB channels are
    separated by a number of lines that depends on the line distance
    as reported by the ESC i command and the resolution of the scan.

    This function reorganises raw scan data so that the RGB channels
    are no longer separated and all data is on the same scan line.

    \note
    It seems that the Perfection 610 and 640U both report a maximum
    scan area with the two line distances included (based on 11.7"
    at 600dpi = 7020, whereas the scan area is reportedly 7036 with
    both line distances equal to 8).
 */
static SANE_Status
color_shuffle (SANE_Handle handle, int *new_length)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Byte *buf = s->buf;
  int length = s->end - s->buf;

  if (s->hw->color_shuffle == SANE_TRUE)
  {
    SANE_Byte *data_ptr;	/* ptr to data to process */
    SANE_Byte *data_end;	/* ptr to end of processed data */
    SANE_Byte *out_data_ptr;	/* ptr to memory when writing data */
    int i;			/* loop counter */

    DBG (5, "sane_read: color_shuffle\n");

    /*
     * Initialize the variables we are going to use for the 
     * copying of the data. data_ptr is the pointer to
     * the currently worked on scan line. data_end is the
     * end of the data area as calculated from adding *length 
     * to the start of data.
     * out_data_ptr is used when writing out the processed data
     * and always points to the beginning of the next line to
     * write.
     */

    data_ptr = out_data_ptr = buf;
    data_end = data_ptr + length;

    /*
     * The image data is in *buf, we know that the buffer contains s->end - s->buf ( = length)
     * bytes of data. The width of one line is in s->params.bytes_per_line
     */

    /*
     * The buffer area is supposed to have a number of full scan
     * lines, let's test if this is the case. 
     */

    if (length % s->params.bytes_per_line != 0)
    {
      DBG (1, "ERROR in size of buffer: %d / %d\n",
	   length, s->params.bytes_per_line);
      return SANE_STATUS_INVAL;
    }

    while (data_ptr < data_end)
    {
      SANE_Byte *source_ptr, *dest_ptr;
      int loop;

      /* copy the green information into the current line */

      source_ptr = data_ptr + 1;
      dest_ptr = s->line_buffer[s->color_shuffle_line] + 1;

      for (i = 0; i < s->params.bytes_per_line / 3; i++)
      {
	*dest_ptr = *source_ptr;
	dest_ptr += 3;
	source_ptr += 3;
      }

      /* copy the red information n lines back */

      if (s->color_shuffle_line >= s->line_distance)
      {
	source_ptr = data_ptr + 2;
	dest_ptr =
	  s->line_buffer[s->color_shuffle_line - s->line_distance] + 2;

	for (loop = 0; loop < s->params.bytes_per_line / 3; loop++)

	{
	  *dest_ptr = *source_ptr;
	  dest_ptr += 3;
	  source_ptr += 3;
	}
      }

      /* copy the blue information n lines forward */

      source_ptr = data_ptr;
      dest_ptr = s->line_buffer[s->color_shuffle_line + s->line_distance];

      for (loop = 0; loop < s->params.bytes_per_line / 3; loop++)
      {
	*dest_ptr = *source_ptr;
	dest_ptr += 3;
	source_ptr += 3;
      }

      data_ptr += s->params.bytes_per_line;
      s->ptr   += s->params.bytes_per_line;

      if (s->color_shuffle_line == s->line_distance)
      {
	/*
	 * we just finished the line in line_buffer[0] - write it to the
	 * output buffer and continue.
	 */


	/*
	 * The ouput buffer ist still "buf", but because we are
	 * only overwriting from the beginning of the memory area
	 * we are not interfering with the "still to shuffle" data
	 * in the same area.
	 */

	/*
	 * Strip the first and last n lines and limit to 
	 */
	if ((s->current_output_line >= s->line_distance) &&
	    (s->current_output_line < s->params.lines + s->line_distance))
	{
	  memcpy (out_data_ptr, s->line_buffer[0], s->params.bytes_per_line);
	  out_data_ptr += s->params.bytes_per_line;

	  s->lines_written++;
	}

	s->current_output_line++;


	/*
	 * Now remove the 0-entry and move all other 
	 * lines up by one. There are 2*line_distance + 1 
	 * buffers, * therefore the loop has to run from 0 
	 * to * 2*line_distance, and because we want to
	 * copy every n+1st entry to n the loop runs
	 * from - to 2*line_distance-1!
	 */

	delete (s->line_buffer[0]);

	for (i = 0; i < s->line_distance * 2; i++)
	{
	  s->line_buffer[i] = s->line_buffer[i + 1];
	}

	/*
	 * and create one new buffer at the end
	 */

	s->line_buffer[s->line_distance * 2] =
	  malloc (s->params.bytes_per_line);
	if (s->line_buffer[s->line_distance * 2] == NULL)
	{
	  int i;
	  for (i = 0; i < s->line_distance * 2; i++)
	  {
	    delete (s->line_buffer[i]);
	    s->line_buffer[i] = NULL;
	  }
	  DBG (1, "out of memory (line %d)\n", __LINE__);
	  return SANE_STATUS_NO_MEM;
	}
      }
      else
      {
	s->color_shuffle_line++;	/* increase the buffer number */
      }
    }

    /*
     * At this time we've used up all the new data from the scanner, some of
     * it is still in the line_buffers, but we are ready to return some of it
     * to the front end software. To do so we have to adjust the size of the
     * data area and the *new_length variable.
     */

    *new_length = out_data_ptr - buf;
  }

  return SANE_STATUS_GOOD;

}




/*
 * static SANE_Status get_identity_information ( SANE_Handle handle)
 *
 * Request Identity information from scanner and fill in information
 * into dev and/or scanner structures.
 */
static SANE_Status
get_identity_information (SANE_Handle handle)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  Epson_Device *dev = s->hw;
  EpsonIdent ident;
  u_char param[3];
  SANE_Status status;
  u_char *buf;

  DBG (5, "get_identity_information()\n");

  if (!s->hw->cmd->request_identity)
    return SANE_STATUS_INVAL;


  param[0] = ESC;
  param[1] = s->hw->cmd->request_identity;
  param[2] = '\0';

  if (NULL == (ident = (EpsonIdent) command (s, param, 2, &status)))
  {
    DBG (1, "ident failed\n");
    return SANE_STATUS_INVAL;
  }

  DBG (1, "type  %3c 0x%02x\n", ident->type, ident->type);
  DBG (1, "level %3c 0x%02x\n", ident->level, ident->level);

  {
    char *force = getenv ("SANE_EPSON_CMD_LVL");

    if (force)
    {
      ident->type = force[0];
      ident->level = force[1];

      DBG (1, "type  %3c 0x%02x\n", ident->type, ident->type);
      DBG (1, "level %3c 0x%02x\n", ident->level, ident->level);

      DBG (1, "forced\n");
    }
  }

/*
 *  check if option equipment is installed.
 */

  if (ident->status & STATUS_OPTION)
  {
    DBG (1, "option equipment is installed\n");
    dev->extension = SANE_TRUE;
  }
  else
  {
    DBG (1, "no option equipment installed\n");
    dev->extension = SANE_FALSE;
  }

  dev->TPU = SANE_FALSE;
  dev->ADF = SANE_FALSE;

/*
 *  set command type and level.
 */

  {
    int n;

    for (n = 0; n < NELEMS (epson_cmd); n++)
    {
      char type_level[3];
      sprintf (type_level, "%c%c", ident->type, ident->level);
      if (!strncmp (type_level, epson_cmd[n].level, 2))
	break;
    }

    if (n < NELEMS (epson_cmd))
    {
      dev->cmd = &epson_cmd[n];
    }
    else
    {
      dev->cmd = &epson_cmd[EPSON_LEVEL_DEFAULT];
      DBG (1, "Unknown type %c or level %c, using %s\n",
	   ident->type, ident->level, dev->cmd->level);
    }

    s->hw->level = dev->cmd->level[1] - '0';
  }				/* set comand type and level */

/*
 *  Setting available resolutions and xy ranges for sane frontend.
 */

  s->hw->res_list_size = 0;
  s->hw->res_list =
    (SANE_Int *) calloc (s->hw->res_list_size, sizeof (SANE_Int));

  if (NULL == s->hw->res_list)
  {
    DBG (1, "out of memory (line %d)\n", __LINE__);
    return SANE_STATUS_NO_MEM;
  }

  {
    int n, k;
    int x = 0, y = 0;
    int count = ident->count2 * 255 + ident->count1;

    buf = ident->buf;
    if (0x0118 == s->hw->productID
	|| 0x0119 == s->hw->productID) {
      /* skip the first two resolutions for the GT-F600 and GT-X750
	 because otherwise the TPU and ADF don't work */
      buf += 6;
    }

    /* we need to correct for the difference in size between
       the EpsonIdentRec and the EpsonHdrRec */
    int correction = sizeof (EpsonIdentRec) - sizeof (EpsonHdrRec);

    for (n = (count - correction); n; n -= k, buf += k)
    {
      switch (*buf)
      {
      case 'R':
	{
	  int val = buf[2] << 8 | buf[1];

	  k = 3;

	  if (0x0119 == s->hw->productID && 400 == val)
	    continue;

	  s->hw->res_list_size++;
	  s->hw->res_list =
	    (SANE_Int *) realloc (s->hw->res_list,
				  s->hw->res_list_size * sizeof (SANE_Int));

	  if (NULL == s->hw->res_list)
	  {
	    DBG (1, "out of memory (line %d)\n", __LINE__);
	    return SANE_STATUS_NO_MEM;
	  }

	  s->hw->res_list[s->hw->res_list_size - 1] = (SANE_Int) val;

	  DBG (1, "resolution (dpi): %d\n", val);
	  continue;
	}
      case 'A':
	{
	  x = buf[2] << 8 | buf[1];
	  y = buf[4] << 8 | buf[3];

	  DBG (1, "maximum scan area: x %d y %d\n", x, y);
	  k = 5;
	  continue;
	}
      default:
	break;
      }				/* case */

      break;
    }				/* for */

    dev->dpi_range.min = s->hw->res_list[0];
    dev->dpi_range.max = s->hw->res_list[s->hw->res_list_size - 1];
    dev->dpi_range.quant = 0;

    dev->fbf_x_range.min = 0;
    dev->fbf_x_range.max = SANE_FIX (x * MM_PER_INCH / dev->dpi_range.max);
    dev->fbf_x_range.quant = 0;

    dev->fbf_max_x = x;
    dev->fbf_max_y = y;

    dev->fbf_y_range.min = 0;
    dev->fbf_y_range.max = SANE_FIX (y * MM_PER_INCH / dev->dpi_range.max);
    dev->fbf_y_range.quant = 0;

    dev->fbf_y_max = y;

    DBG (5, "fbf tlx %f tly %f brx %f bry %f [mm]\n",
	 SANE_UNFIX (dev->fbf_x_range.min), SANE_UNFIX (dev->fbf_y_range.min),
	 SANE_UNFIX (dev->fbf_x_range.max),
	 SANE_UNFIX (dev->fbf_y_range.max));

  }

  /*
   * Copy the resolution list to the resolution_list array so that the frontend can
   * display the correct values
   */

  s->hw->resolution_list =
    malloc ((s->hw->res_list_size + 1) * sizeof (SANE_Word));

  if (s->hw->resolution_list == NULL)
  {
    DBG (1, "out of memory (line %d)\n", __LINE__);
    delete (s->hw->res_list);
    return SANE_STATUS_NO_MEM;
  }
  *(s->hw->resolution_list) = s->hw->res_list_size;
  memcpy (&(s->hw->resolution_list[1]), s->hw->res_list,
	  s->hw->res_list_size * sizeof (SANE_Word));

  /* filter the resolution list */
  /* the option is not yet initialized, for now just set it to false */
  s->val[OPT_LIMIT_RESOLUTION].w = SANE_FALSE;
  filter_resolution_list (s);

  delete (ident);

  return SANE_STATUS_GOOD;

}				/* request identity */


/*
 * static SANE_Status get_identity2_information ( SANE_Handle handle)
 *
 * Request Identity2 information from scanner and fill in information
 * into dev and/or scanner structures.
 */
static SANE_Status
get_identity2_information (SANE_Handle handle)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Status status;
  EpsonHdr ident;
  u_char param[3];
  u_char *buf;

  DBG (5, "get_identity2_information()\n");

  if (s->hw->cmd->request_identity2 == 0)
    return SANE_STATUS_UNSUPPORTED;

  param[0] = ESC;
  param[1] = s->hw->cmd->request_identity2;
  param[2] = '\0';

  if (NULL == (ident = (EpsonHdr) command (s, param, 2, &status)))
  {
    DBG (1, "ident2 failed\n");
    return SANE_STATUS_INVAL;
  }

  buf = ident->buf;

#if 0
  send (s, param, 2, &status);

  if (SANE_STATUS_GOOD != status)
    return status;

  len = 4;			/* receive header */

  receive (s, result, len, &status);
  if (SANE_STATUS_GOOD != status)
    return status;

  len = result[3] << 8 | result[2];
  buf = alloca (len);

  receive (s, buf, len, &status);	/* reveive actual status data */

  if (buf[0] & 0x80)
  {
    close_scanner (s);
    return SANE_STATUS_INVAL;
  }
#endif

  /* the first two bytes of the buffer contain the optical resolution */
  s->hw->optical_res = buf[1] << 8 | buf[0];

  /*
   * the 4th and 5th byte contain the line distance. Both values have to
   * be identical, otherwise this software can not handle this scanner.
   */
  if (buf[4] != buf[5])
  {
    close_scanner (s);
    return SANE_STATUS_INVAL;
  }
  s->hw->max_line_distance = buf[4];
  s->hw->fbf_y_range.max = SANE_FIX ((s->hw->fbf_y_max -
				      2 * (s->hw->max_line_distance))
				     * MM_PER_INCH / s->hw->dpi_range.max);

  delete (ident);

  return SANE_STATUS_GOOD;
}

/*
 * void sane_cancel(SANE_Handle handle)
 * 
 * Set the cancel flag to true. The next time the backend requests data
 * from the scanner the CAN message will be sent.
 */

void
sane_cancel (SANE_Handle handle)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;

  /*
   * If the s->ptr pointer is not NULL, then a scan operation
   * was started and if s->eof is FALSE, it was not finished.
   */

  if (s->buf != NULL)
  {
    unsigned char *dummy;
    int len;

    /* malloc one line */
    dummy = malloc (s->params.bytes_per_line);
    if (dummy == NULL)
    {
      DBG (1, "Out of memory\n");
      return;
    }
    else
    {

      /* there is still data to read from the scanner */

      s->canceling = SANE_TRUE;

      while (!s->eof &&
	     SANE_STATUS_CANCELLED != sane_read (s, dummy,
						 s->params.bytes_per_line,
						 &len))
      {
	/* empty body, the while condition does the processing */
      }
      delete (dummy);
    }
  }
  close_scanner (s);
}

#if 0
static SANE_Status
request_focus_position (SANE_Handle handle, u_char * position)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Status status;
  int len;
  u_char param[3];
  u_char result[4];
  u_char *buf;

  DBG (5, "request_focus_position()\n");

  if (s->hw->cmd->request_focus_position == 0)
    return SANE_STATUS_UNSUPPORTED;

  param[0] = ESC;
  param[1] = s->hw->cmd->request_focus_position;
  param[2] = '\0';

  send (s, param, 2, &status);

  if (SANE_STATUS_GOOD != status)
    return status;

  len = 4;			/* receive header */

  receive (s, result, len, &status);
  if (SANE_STATUS_GOOD != status)
    return status;

  len = result[3] << 8 | result[2];
  buf = alloca (len);

  receive (s, buf, len, &status);	/* reveive actual status data */

  *position = buf[1];
  DBG (1, "Focus position = 0x%x\n", buf[1]);

  return SANE_STATUS_GOOD;
}
#endif

/*
 * Request the push button status 
 * returns SANE_TRUE if the button was pressed
 * and SANE_FALSE if the button was not pressed
 * it also returns SANE_TRUE in case of an error.
 * This is necessary so that a process that waits for
 * the button does not block indefinitely.
 */
static SANE_Status
request_push_button_status (SANE_Handle handle, SANE_Bool * theButtonStatus)
{
  Epson_Scanner *s = (Epson_Scanner *) handle;
  SANE_Status status;
  int len;
  u_char param[3];
  u_char result[4];
  u_char *buf;

  DBG (5, "request_push_button_status()\n");

  if (s->hw->cmd->request_push_button_status == 0)
  {
    DBG (1, "push button status unsupported\n");
    return SANE_STATUS_UNSUPPORTED;
  }

  param[0] = ESC;
  param[1] = s->hw->cmd->request_push_button_status;
  param[2] = '\0';

  send (s, param, 2, &status);

  if (SANE_STATUS_GOOD != status)
  {
    DBG (1, "error sending command\n");
    return status;
  }

  len = 4;			/* receive header */

  receive (s, result, len, &status);
  if (SANE_STATUS_GOOD != status)
    return status;

  len = result[3] << 8 | result[2];	/* this should be 1 for scanners with one button */
  buf = alloca (len);

  receive (s, buf, len, &status);	/* reveive actual status data */

  DBG (1, "Push button status = %d\n", buf[0] & 0x01);
  *theButtonStatus = ((buf[0] & 0x01) != 0);

  return (SANE_STATUS_GOOD);
}



static void
filter_resolution_list (Epson_Scanner * s)
{
  /* re-create the list */

  if (s->val[OPT_LIMIT_RESOLUTION].w == SANE_TRUE)
  {
    /* copy the short list */

    /* filter out all values that are not 300 or 400 dpi based */
    int i;

    int new_size = 0;
    SANE_Bool is_correct_resolution = SANE_FALSE;

    for (i = 1; i <= s->hw->res_list_size; i++)
    {
      SANE_Word res;
      res = s->hw->res_list[i];
      if ((res < 100) || (0 == (res % 300)) || (0 == (res % 400)))
      {
	/* add the value */
	new_size++;

	s->hw->resolution_list[new_size] = s->hw->res_list[i];

	/* check for a valid current resolution */
	if (res == s->val[OPT_RESOLUTION].w)
	{
	  is_correct_resolution = SANE_TRUE;
	}
      }
    }
    s->hw->resolution_list[0] = new_size;

    if (is_correct_resolution == SANE_FALSE)
    {
      for (i = 1; i <= new_size; i++)
      {
	if (s->val[OPT_RESOLUTION].w < s->hw->resolution_list[i])
	{
	  s->val[OPT_RESOLUTION].w = s->hw->resolution_list[i];
	  i = new_size + 1;
	}
      }
    }

  }
  else
  {
    /* copy the full list */
    s->hw->resolution_list[0] = s->hw->res_list_size;
    memcpy (&(s->hw->resolution_list[1]), s->hw->res_list,
	    s->hw->res_list_size * sizeof (SANE_Word));
  }
}

/**********************************************************************************/

/*
 * SANE_Status sane_set_io_mode()
 *
 * not supported - for asynchronous I/O
 */

SANE_Status
sane_set_io_mode (SANE_Handle handle, SANE_Bool non_blocking)
{
  /* get rid of compiler warning */
  handle = handle;
  non_blocking = non_blocking;

  return SANE_STATUS_UNSUPPORTED;
}

/*
 * SANE_Status sane_get_select_fd()
 *
 * not supported - for asynchronous I/O
 */

SANE_Status
sane_get_select_fd (SANE_Handle handle, SANE_Int * fd)
{
  /* get rid of compiler warnings */
  handle = handle;
  fd = fd;

  return SANE_STATUS_UNSUPPORTED;
}


/* Not all scanners report the model name that is printed on the case.
   Here we try to adjust for that by second guessing the name received
   from the hardware with the aid of some environmental data.
   WARNING: this is not and can not be foolproof.

   The caller gets to manage the memory occupied by the string that is
   returned.
 */

#include <assert.h>

static const char *generic_epson_scanner_name = "EPSON Scanner";

static const char *
second_guess_model_name (Epson_Scanner * s, const char *fw_name)
{
  const scanner_data_t *data = scanner_data;
  const char *result;

  struct tm *ptr = NULL;

  {				/* input validation */
    if (!s || !fw_name)
      return strdup (generic_epson_scanner_name);
  }

  while (data->fw_name != NULL	/* end of array */
	 && (0 != strcmp (data->fw_name, fw_name)))
  {
    ++data;
  }

  if (data->fw_name == NULL)	/* not a known scanner */
  {
    return strdup (fw_name);	/* use whatever we got */
  }

  /* Try to figure the name that is most likely to be on the scanner
     case.  We are using a fairly simple algorithm at the moment and
     welcome your patches ;-)  */

  result = (data->name.overseas ? data->name.overseas : data->fw_name);	/* fall back */

  {				/* get an idea of where we are */
    time_t lt = time (NULL);
    ptr = localtime (&lt);
    assert (ptr != NULL);
  }

  if (data->name.japan && 0 == strncmp ("JST", ptr->tm_zone, 3))
  {
    result = strdup (data->name.japan);
  }
  else
  {
    result = strdup (result);
  }

  {
    /* The remainder of this function does a few things that are not
       implied by its name and should therefore be moved some place
       else.  The only sane alternative is to rename this function.
     */

    /* find an appropriate colour correction profile */
    int i = sizeof (epson_scan_hard) / sizeof (EpsonScanHardRec);
    while (--i && data->profile_ID != epson_scan_hard[i].modelID)
      ;

    s->hw->scan_hard = &epson_scan_hard[i];


    if (data->command_ID)	/* customize command settings */
    {
      int id = data->command_ID - 1;	/* adjust for offset */
      assert (id >= 0);
      assert ((unsigned) id < (sizeof (scan_command)
			       / sizeof (scan_command_t)));

      if (ILLEGAL_CMD != scan_command[id].set_focus_position)
      {
	s->hw->cmd->set_focus_position = scan_command[id].set_focus_position;
      }

      if (ILLEGAL_CMD != scan_command[id].feed)
      {
	s->hw->cmd->feed = scan_command[id].feed;
      }

      if (ILLEGAL_CMD != scan_command[id].eject)
      {
	s->hw->cmd->eject = scan_command[id].eject;
      }
    }
  }

  return result;
}

/* check control_extension Status for ES-10000G or Expression 10000XL */
static void
check_control_extension (Epson_Scanner * s, SANE_Status * status)
{
  u_char *buf;
  u_char params[2];
  EpsonHdr head;
  int failures_allowed = 5;

  params[0] = ESC;
  params[1] = s->hw->cmd->request_extended_status;

  head = (EpsonHdr) command (s, params, 2, status);
  buf = &head->buf[0];

  while (!(buf[1] & EXT_STATUS_EN))
  {
    sleep (1);
    control_extension (s, s->hw->use_extension);

    head = (EpsonHdr) command (s, params, 2, status);
    if (status != SANE_STATUS_GOOD)
    {
      --failures_allowed;
      if (!failures_allowed)
	return;
    }
    buf = &head->buf[0];

  }
}


/*
vim:ts=2:sw=2:cindent:
*/
