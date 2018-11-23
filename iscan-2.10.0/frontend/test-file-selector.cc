/* test-filesel.cc -- customized file selection dialog
   Copyright (C) 2005  SEIKO EPSON Corporation

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
 */

#include "pisa_aleart_dialog.cc"
#include "pisa_error.cc"
#include "file-selector.cc"

/* Tester program specific stuff.  */

#include "pisa_enums.h"

#ifndef HAVE_GTK_2
#define G_OBJECT GTK_OBJECT
#endif

static int
get_option_value ()
{
  int result = PISA_OP_FLATBED;

  char *env_val = getenv ("PISA_OP");
  if (env_val)
    {
      if (0 == strcasecmp ("ADF", env_val))
	{
	  result = PISA_OP_ADF;
	}
      if (0 == strcasecmp ("ADFDPLX", env_val))
	{
	  result = PISA_OP_ADFDPLX;
	}
      if (0 == strcasecmp ("TPU", env_val))
	{
	  result = PISA_OP_TPU;
	}
    }
  return result;
}

static bool
is_button_set ()
{
  return (NULL != getenv ("PISA_BUTTON"));
}

static file_selector *file_sel = NULL;
static GtkWidget     *g_widget = NULL;

static int g_cancel = 0;
static int g_sheets = 1;

static bool
scan_file (gpointer param, const gchar *filename,
	   int *cancel = NULL, int cnt = -1)
{
  bool error = false;

  if (0 > cnt)
    {
      g_print ("Scanning to %s ... ", filename);
      sleep (1);
      g_print ("done.\n");
    }
  else
    {
      if (g_sheets)
	{
	  --g_sheets;
	  g_print ("Scanning to %s (%d) ... ", filename, cnt);
	  sleep (1);
	  g_print ("done.\n");
	}
      else
	{
	  error = true;
	  *cancel = (0 > --g_cancel);
	  if (!*cancel)
	    {
	      char *env_val = getenv ("SHEETS");
	      if (env_val)
		{
		  g_sheets = atoi (env_val);
		}
	      else
		{
		  g_sheets = 1;
		}
	    }
	}
    }
  return error;
}

static void
do_scan_file (gpointer param, file_selector *fs = NULL,
	      bool first_time_around = false)
{
  if (!fs)
    {
      fs = file_sel;
      fs->init ();

      int  option = get_option_value ();
      bool button = is_button_set ();

      fs->create_window (GTK_WINDOW (gtk_widget_get_parent (g_widget)),
			 g_widget, (   PISA_OP_ADF     == option
				    || PISA_OP_ADFDPLX == option
				    || button));
      fs->hide ();
    }

  char *filename = fs->get_filename ();

  if (   PISA_OP_ADF     != get_option_value ()
      && PISA_OP_ADFDPLX != get_option_value ()
      && !is_button_set ())
    {
      fs->destroy ();

      if (!filename)
	{
	  g_print ("No file selected\n");
	  return;
	}

      scan_file (param, filename);
    }
  else				// consecutive scanning
    if (filename)
      {
	g_print ("%s: consecutive scan\n", __func__);

	int  cancel = 0;
	// check_overwrite initialises the value of cancel, so ...
	bool eos = (0 != cancel);
	int  cnt = fs->get_sequence_number();

	while (!eos)
	  {
	    eos = scan_file (param, filename, &cancel, cnt);
	    first_time_around = false;
	    if (!eos)
	      {
		fs->set_sequence_number (++cnt);
	      }
	  }

	if (!cancel)
	  {
	    g_print ("%s: recursing\n", __func__);
	    do_scan_file (param, fs, first_time_around);
	    g_print ("%s: returning\n", __func__);
	  }
	g_print ("%s: consecutive scan, if branch\n", __func__);
	fs->destroy ();
      }
    else
      {
	g_print ("%s: consecutive scan, else branch\n", __func__);
	fs->destroy ();
      }
  free (filename);
}

/* Callback for the Scan button.  */
static void
callback (GtkWidget *widget, gpointer data )
{
  g_print ("%s was pressed\n", (char *) data);

  gtk_widget_set_sensitive (widget, false);
  g_widget = widget;

  char *env_val = NULL;

  env_val = getenv ("SHEETS");
  if (env_val)
    {
      g_sheets = atoi (env_val);
    }
  env_val = getenv ("CANCEL");
  if (env_val)
    {
      g_cancel = atoi (env_val);
    }

  do_scan_file (NULL);
}

int
main (int argc, char *argv[])
{
  if (!file_sel)
    {
      file_sel = new file_selector;
    }
  if (!file_sel)
    {
      abort ();
    }

  /* GtkWidget is the storage type for widgets */
  GtkWidget *window;
  GtkWidget *button;

  gtk_init (&argc, &argv);

  /* Create a new window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (window), "File Selection Tester");

  /* It's a good idea to do this for all windows. */
  g_signal_connect (G_OBJECT (window), "destroy",
		    G_CALLBACK (gtk_main_quit), NULL);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gtk_main_quit), NULL);

  /* Create a new button */
  button = gtk_button_new_with_label ("Scan");

  /* Connect the "clicked" signal of the button to our callback */
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (callback), (gpointer) "Scan button");

  gtk_widget_show (button);

  gtk_container_add (GTK_CONTAINER (window), button);

  gtk_widget_show (window);

  /* Rest in gtk_main and wait for the fun to begin! */
  gtk_main ();

  return 0;
}
