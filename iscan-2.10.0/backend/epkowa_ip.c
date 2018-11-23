/* epkowa_ip.c -- for scanners that don't speak EPSON ESC/I natively
   Copyright (C) 2005  Olaf Meeuwissen

   This file is part of the EPKOWA SANE backend.
   It defines a wrapper around the backend's "Interpreter" interface.

   The EPKOWA SANE backend is free software.
   You can redistribute it and/or modify it under the terms of the GNU
   General Public License as published by the Free Software Foundation;
   either version 2 of the License or at your option any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY;  without even the implied warranty of FITNESS
   FOR A PARTICULAR PURPOSE or MERCHANTABILITY.
   See the GNU General Public License for more details.

   You should have received a verbatim copy of the GNU General Public
   License along with this program; if not, write to:

       Free Software Foundation, Inc.
       59 Temple Place, Suite 330
       Boston, MA  02111-1307  USA

   Linking the EPKOWA SANE backend statically or dynamically with
   other modules is making a combined work based on the EPKOWA SANE
   backend.  Thus, the terms and conditions of the GNU General Public
   License cover the whole combination.

   As a special exception, the copyright holders of the EPKOWA SANE
   backend give you permission to link the EPKOWA SANE backend with
   independent modules that communicate with the EPKOWA SANE backend
   solely through the "Interpreter" interface, regardless of the
   license terms of these independent modules, and to copy and
   distribute the resulting combined work under terms of your choice,
   provided that every copy of the combined work is accompanied by a
   complete copy of the source code of the EPKOWA SANE backend (the
   version of the EPKOWA SANE backend used to produce the combined
   work), being distributed under the terms of the GNU General Public
   License plus this exception.  An independent module is a module
   which is not derived from or based on the EPKOWA SANE backend.

   Note that people who make modified versions of the EPKOWA SANE
   backend are not obligated to grant this special exception for their
   modified versions; it is their choice whether to do so.  The GNU
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ltdl.h>

#include "epkowa_ip.h"

#include "sane/sanei_usb.h"

#define TICK_INTERVAL	60	/* seconds */
#define TICK_TIME_OUT	15	/* intervals */

// FIXME: this ought to be part of the interpreter_type struct so that
//        every instance can have its own.
device_type *g_epson = NULL;


struct interpreter_data_type
{
  const unsigned int product_id;
  const char *module;
};

static const
struct interpreter_data_type interpreter_data[] = {
  {0x010f, "libesint1F"},	/* GT-7200 / Perfection 1250 */
  {0x011d, "libesint23"},	/* GT-7300 / Perfection 1260 */
  {0x0116, "libesint32"},	/* GT-9400 / Perfection 3170 */
  {0x0118, "libesint43"},	/* GT-F600 / Perfection 4180 */
  {0x0121, "libesint41"},	/* GT-F500 / Perfection 2480 */
				/* GT-F550 / Perfection 2580 */
  {0x0122, "libesint52"},	/* GT-F520 / Perfection 3490 */
				/* GT-F570 / Perfection 3590 */
  {0x012d, "libesint66"},	/* GT-S600 / Perfection V10  */
				/* GT-F650 / Perfection V100 */
  {0x012f, "libesint68"},	/* GT-F700 / Perfection V350 */
  {0x0119, "libesint54"},	/* GT-X750 / Perfection 4490 */
  {0x012e, "libesint7A"},	/* GT-F670 / Perfection V200 */
  {0x083f, "libesint7E"},	/* Stylus CX4300/CX4400/CX5500/CX5600/DX4400 */
  {0x0000, NULL},		/* terminator */
};


/* default member function implementors */

static SANE_Status _dtor (device_type *);
static int _open  (device_type *);
static int _close (device_type *);
static ssize_t _recv (device_type *, void *, size_t, SANE_Status *);
static ssize_t _send (device_type *, void *, size_t, SANE_Status *);

static SANE_Status _free (device_type *);
static SANE_Status _ftor0 (device_type *, SANE_Parameters *,
			   SANE_Byte *, SANE_Byte *);
static SANE_Status _ftor1 (device_type *, SANE_Parameters *,
			   int, int, int);

/* private functions */

static void *_load (const char *, device_type *);


void
find_interpreted_devices (SANE_Status (*attach) (const char *dev))
{
  const struct interpreter_data_type *p = interpreter_data;
  const unsigned int vendor = 0x04b8;

  while (p && p->product_id)
  {
    sanei_usb_find_devices (vendor, p->product_id, attach);
    ++p;
  }
}

/*! Creates an interpreter object for a \a device if both necessary
    and available.  The object is accessible through the \a device's
    \c interpreter field once successfully created.
 */
SANE_Status
create_interpreter (device_type *device, unsigned int usb_product_id)
{
  const struct interpreter_data_type *p = interpreter_data;

  if (!device)			/* sanity check */
  {
    return SANE_STATUS_INVAL;
  }

  if (device->interpreter)	/* we've been here before */
  {
    if (device != device->interpreter->_device)
    {				/* internal inconsistency! */
      return SANE_STATUS_INVAL;
    }
    return SANE_STATUS_GOOD;
  }

  /* Check whether there is an interpreter that supports this
     device or not.  */
  while (p && p->product_id && usb_product_id != p->product_id)
  {
    ++p;
  }
  if (!p || !p->product_id)	/* no interpreter found */
  {
    return SANE_STATUS_GOOD;
  }

  /* If we are still here, we need to set up an interpreter and load
     the corresponding module.  */

  device->interpreter = ((struct interpreter_type *)
			 malloc (sizeof (struct interpreter_type)));

  if (!device->interpreter)
  {
    return SANE_STATUS_NO_MEM;
  }

  device->interpreter->_device = device;
  device->interpreter->_module = _load (p->module, device);

  if (!device->interpreter->_module)
  {
    free (device->interpreter);
    device->interpreter = NULL;
    return SANE_STATUS_INVAL;
  }

  device->interpreter->_tick_count = -1;
  device->interpreter->_table  = NULL;
  device->interpreter->_buffer = NULL;

  device->interpreter->dtor  = _dtor;
  device->interpreter->open  = _open;
  device->interpreter->close = _close;
  device->interpreter->recv  = _recv;
  device->interpreter->send  = _send;

  device->interpreter->free  = _free;
  device->interpreter->ftor0 = _ftor0;
  device->interpreter->ftor1 = _ftor1;

  return SANE_STATUS_GOOD;
}


/*! Destroys a \a device's interpreter object.
 */
SANE_Status
_dtor (device_type *device)
{
  if (!device || !device->interpreter)
  {
    return SANE_STATUS_INVAL;
  }

  device->interpreter->close (device);
  device->interpreter->free (device);

#ifdef USE_ALARM
  alarm (0);
#endif

  lt_dlclose (device->interpreter->_module);
  device->interpreter->_module = 0;

  free(device->interpreter);
  device->interpreter = NULL;

  return SANE_STATUS_GOOD;
}

/*! Frees several temporary buffers.
 */
SANE_Status
_free (device_type *device)
{
  if (!device || !device->interpreter)
  {
    return SANE_STATUS_INVAL;
  }

  if (device->interpreter->_table)
    free (device->interpreter->_table);
  device->interpreter->_table = 0;

  if (device->interpreter->_buffer)
    free (device->interpreter->_buffer);
  device->interpreter->_buffer = 0;

  return SANE_STATUS_GOOD;
}

/*! Performs whatever actions are needed when opening an interpreter
    "controlled" device.  One of these actions is disarming an alarm
    used to control whether the device can go into power saving mode
    or not.  */
int
_open  (device_type *device)
{
  if (!device || !device->interpreter)
  {
    return -1;
  }

#ifdef USE_ALARM
  alarm (0);
  //  DBG (5,"alarm (%d)\n", 0);
#endif
  device->interpreter->_tick_count = -1;

  g_epson = device;

  return device->fd;
}

int
_close (device_type *device)
{
  if (!device || !device->interpreter)
  {
    return -1;
  }

  device->interpreter->_tick_count = 0;
#ifdef USE_ALARM
  alarm (TICK_INTERVAL);
  //  DBG (5, "alarm (%d)\n", TICK_INTERVAL);
#endif

  g_epson = NULL;

  return device->fd;
}

ssize_t
_send (device_type *device, void *buffer, size_t size,
       SANE_Status *status)
{
  if (!status)
  {
    return -1;
  }

  if (!device || !device->interpreter)
  {
    *status = SANE_STATUS_INVAL;
    return -1;
  }

  if (device->interpreter->_write (buffer, size))
  {
    *status = SANE_STATUS_GOOD;
  }
  else
  {
    *status = SANE_STATUS_INVAL;
  }

  return size;
}

ssize_t
_recv (device_type *device, void *buffer, size_t size,
       SANE_Status *status)
{
  if (!status)
  {
    return -1;
  }

  if (!device || !device->interpreter)
  {
    *status = SANE_STATUS_INVAL;
    return -1;
  }

  if (device->interpreter->_read (buffer, size))
  {
    *status = SANE_STATUS_GOOD;
  }
  else
  {
    *status = SANE_STATUS_INVAL;
  }

  return size;
}

/*! Callback for use by the interpreter.  */
ssize_t
usb_read (void *buffer, size_t size)
{
  size_t n = size;

  if (!g_epson || g_epson->fd < 0)
  {
    return 0;
  }

  if (SANE_STATUS_GOOD == sanei_usb_read_bulk (g_epson->fd, buffer, &n))
  {
    if (size != n)
      //      DBG (1, "Did not read number of bytes requested");
      ;
    return n;
  }
  else
  {
    return 0;
  }
}

/*! Callback for use by the interpreter.  */
ssize_t
usb_write (void *buffer, size_t size)
{
  size_t n = size;

  if (!g_epson || g_epson->fd < 0)
  {
    return 0;
  }

  if (SANE_STATUS_GOOD == sanei_usb_write_bulk (g_epson->fd, buffer, &n))
  {
    if (size != n)
      //      DBG (1, "Did not read number of bytes requested");
      ;
    return n;
  }
  else
    return 0;
}

#ifdef USE_ALARM
void
timer_event (int sig)
{
  sig = sig;

  //  DBG (3, "timer_event\n");

  if (!g_epson || !g_epson->interpreter)
    return;

  if (0 < g_epson->fd || g_epson->interpreter->_tick_count == -1)
  {
    return;
  }

  g_epson->interpreter->_tick_count++;

  if (TICK_TIME_OUT - 1 <= g_epson->interpreter->_tick_count)
  {
    g_epson->interpreter->_power ();
    g_epson->interpreter->_tick_count = -1;
    return;
  }

  alarm (TICK_INTERVAL);
  //  DBG (5, "alarm (%d)\n", TICK_INTERVAL);
}
#endif

/*! Loads the interpreter module and initializes the interpreter.  */
/*!
 */
void *
_load (const char *name, device_type *device)
{
  void *handle = NULL;

  /* FIXME: should set the search path to a list of directories based
     on the regular load path with the PACKAGE attached to each of the
     individual directories.  Should _not_ look in the regular load
     path's directories.  */
  {
    const char *path = lt_dlgetsearchpath ();
    if (!(path && strstr (path, PKGLIBDIR)))
    {
      lt_dladdsearchdir (PKGLIBDIR);
    }
    handle = lt_dlopenext (name);
  }
  if (!handle)
  {
    //    DBG (1, "%s\n", lt_dlerror());
    return NULL;
  }

  device->interpreter->_init  = lt_dlsym (handle, "int_init");
  device->interpreter->_fini  = lt_dlsym (handle, "int_fini");
  device->interpreter->_read  = lt_dlsym (handle, "int_read");
  device->interpreter->_write = lt_dlsym (handle, "int_write");
  device->interpreter->_power = lt_dlsym (handle, "int_power_saving_mode");
  device->interpreter->_s_0   = lt_dlsym (handle, "function_s_0");
  device->interpreter->_s_1   = lt_dlsym (handle, "function_s_1");

  if (!
      (   device->interpreter->_init
       && device->interpreter->_fini
       && device->interpreter->_read
       && device->interpreter->_write
       && device->interpreter->_s_0
       && device->interpreter->_s_1)
      )
  {
    //    DBG (1, "failed to find all required interpreter API\n");
    device->interpreter->_init  = NULL;
    device->interpreter->_fini  = NULL;
    device->interpreter->_read  = NULL;
    device->interpreter->_write = NULL;
    device->interpreter->_s_0   = NULL;
    device->interpreter->_s_1   = NULL;
    lt_dlclose (handle);
    return NULL;
  }

  g_epson = device->interpreter->_device;
				/* usb_read and usb_write callbacks
				   check for a valid g_epson! */
  if (!device->interpreter->_init (device->fd, usb_read, usb_write))
  {
    //    DBG (1, "failed to initialize interpreter\n");
    g_epson = NULL;
    lt_dlclose (handle);
    return NULL;
  }

  /* FIXME: if we are still here, we should set up the alarm stuff
     here because _load is really a sort of ctor.  Note that open
     should disarm the alarm and close should re-arm it.  Our dtor
     should take care of disarming the alarm permanently.  */
  /* FIXME: add error handling */
  {				/* set up alarm for power saving */
#ifdef USE_ALARM
    struct sigaction act;

    /* FIXME: set act.sa_sigaction instead because that can hold a
       void (*) (int, siginfo_t *, void *) so we can pass arguments
       such as device->interpreter->_device(!) to it, perhaps.  */
    act.sa_handler = timer_event;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;

    sigaction (SIGALRM, &act, 0);

    alarm (TICK_INTERVAL);
    //    DBG (5, "alarm (%d)\n", TICK_INTERVAL);
#endif
    device->interpreter->_tick_count = 0;
  }
  return handle;
}

/*
 This functor is called by sane_read only.
 */
SANE_Status
_ftor0 (device_type *device, SANE_Parameters *params,
	SANE_Byte *ptr, SANE_Byte *end)
{
  if (!device || !device->interpreter || !params)
  {
    return SANE_STATUS_INVAL;
  }

  if (params->depth != 1
      && device->interpreter->_table && device->interpreter->_buffer)
  {
    int i, row;

    row = (end - ptr) / params->bytes_per_line;

    for (i = 0; i < row; ++i)
    {
      memcpy (device->interpreter->_buffer, ptr + i * params->bytes_per_line,
	      params->bytes_per_line);
      device->interpreter->_s_1 (device->interpreter->_buffer,
				 ptr + i * params->bytes_per_line,
				 params->pixels_per_line,
				 params->format == SANE_FRAME_RGB,
				 device->interpreter->_table);
    }
  }
  return SANE_STATUS_GOOD;
}

/*
  This functor is called by sane_start only.
 */
SANE_Status
_ftor1 (device_type *device, SANE_Parameters *params,
	int depth, int left, int ndpi)
{
  if (!device || !device->interpreter || !params)
  {
    return SANE_STATUS_INVAL;
  }

  device->interpreter->free (device);

  if (depth != 1)
  {
    device->interpreter->_table =
      malloc (params->pixels_per_line * sizeof (double));

    if (!device->interpreter->_table)
    {
      return SANE_STATUS_NO_MEM;
    }

    if (device->interpreter->_s_0 (left, params->pixels_per_line,
				   ndpi, device->optical_res,
				   device->interpreter->_table))
    {
      device->interpreter->_buffer = malloc (params->bytes_per_line);

      if (!device->interpreter->_buffer)
      {
	free (device->interpreter->_table);
	device->interpreter->_table = NULL;
	return SANE_STATUS_NO_MEM;
      }
    }
    else
    {
      free (device->interpreter->_table);
      device->interpreter->_table = 0;
    }
  }
  return SANE_STATUS_GOOD;
}
