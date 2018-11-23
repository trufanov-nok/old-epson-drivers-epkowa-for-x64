/* file-selector.h -- customized file selection dialog	-*- C++ -*-
   Copyright (C) 2003, 2005  SEIKO EPSON Corporation

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

#ifndef ISCAN_FILE_SELECTOR_H
#define ISCAN_FILE_SELECTOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <regex.h>
#include <gtk/gtk.h>

#include "pisa_error.h"

#define G_CALLBACK_API	public		/* EEK!  */
#define DEPRECATED	__attribute__ ((deprecated))

class file_selector
{
public:
  void init ();
  void create_window (GtkWindow *window, GtkWidget *parent,
		      bool do_consecutive_scanning = false);
  void create_window (GtkWidget *parent = 0, int option = 0,
		      bool enable_start_button = false,
		      bool monochrome = false) DEPRECATED;
  void destroy ();

  char *get_filename () const;
  bool  set_filename (const char *name, bool edit = false) DEPRECATED;

  int  get_sequence_number () const;
  void set_sequence_number (int number);

  bool save_pathname ();	// really G_CALLBACK_API only

  void hide () const;

G_CALLBACK_API:

  void destroy (bool really);
  void delete_text (gint start, gint end);
  void insert_text (gchar *text, gint len, gint *pos);
  void change_format (int index);
  void change_seqnum ();
  void change_digits ();

private:

  char *validate (const char *text) const;
  char *canonize (const char *text) const;

  void add_dialog_extensions ();
  void add_dialog_extensions (bool monochrome) DEPRECATED;

  bool permission (const char *file) const;
  bool show_message (const pisa_error& oops, bool yes_no = false) const;

  void set_entry (const char *text);

  void regerror (int code, regex_t *regex) const;

  GtkWidget        *_parent;
  GtkFileSelection *_widget;
  char *_filename;		// relative filename
  char *_pathname;		// absolute filename

  bool _using_adf;
  int  _file_type;
  bool _deleted_all;

  int _number;			// for consecutive scans
  GtkAdjustment *_seqnum;
  GtkAdjustment *_digits;

  typedef struct		// graphic format info
  {
    char *name;			// used in option menu
    char *lib;			// library providing support
    char *ext;			// default filename extension
    char *rgx;			// filename extension regex
  } format;

  static const format   _format[];
         const format **_fmt;

  char *_ext_regex;
};

#endif /* ISCAN_FILE_SELECTOR_H */
