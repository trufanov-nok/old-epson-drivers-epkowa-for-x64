/* file-selector.cc -- customized file selection dialog
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "file-selector.h"
#include "gettext.h"
#define  _(msg_id)	gettext (msg_id)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "pisa_aleart_dialog.h"
#include "pisa_enums.h"
#include "pisa_error.h"

#include "../lib/imgstream.hh"
#include "../lib/pnmstream.hh"
#include "../lib/pngstream.hh"
#include "../lib/jpegstream.hh"

#ifndef DEBUG
#define g_print(...)
#endif


#ifdef DONT_HAVE_GTK_2
#undef HAVE_GTK_2
#endif

#ifndef HAVE_GTK_2
#define G_CALLBACK  GTK_SIGNAL_FUNC
#define g_signal_stop_emission_by_name gtk_signal_emit_stop_by_name
#define g_signal_connect_swapped gtk_signal_connect_object
#define g_signal_connect gtk_signal_connect
#define GTK1_OBJ(obj) GTK_OBJECT (obj)
GtkWidget *			// "stolen" from GTK+2.0
gtk_widget_get_parent (GtkWidget *widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  return widget->parent;
}
#else
#define GTK1_OBJ(obj) obj
#endif

struct menu_info		// helper struct for callbacks
{
  file_selector *fs;
  int            index;
};


// callback implementations

static void
_destroy (file_selector *fs)
{
  g_print ("%s\n", __func__);

  fs->destroy (true);
}

#if 0
static void
_click_ng( GtkWidget *, file_selector * )
{
  DBG_FS fprintf( stderr, "_click_ng\n" );

  gtk_main_quit();
}
#endif

static void
_click_ok (file_selector *fs)
{
  g_print ("%s\n", __func__);

  if (fs->save_pathname ())
    {
      fs->destroy ();
      gtk_main_quit ();
    }
}

#if 0
#ifndef HAVE_GTK_2
static void
_select_filename( GtkCList *clist, gint row, gint column, GdkEventButton *,
		  file_selector *fs )
{
  DBG_FS fprintf( stderr, "_select_filename\n" );

  char *text;
  gtk_clist_get_text( clist, row, column, &text );

  fs->set_filename( text );
}
#endif

static void
_change_filename( GtkEditable *editable, file_selector *fs )
{
  DBG_FS fprintf( stderr, "_change_filename\n" );

  gchar *name = gtk_editable_get_chars( editable, 0, -1 );

  gint cursor = gtk_editable_get_position( editable );
  gtk_signal_handler_block_by_func( GTK_OBJECT( editable ),
				    GTK_SIGNAL_FUNC( _change_filename ),
				    fs );
  fs->set_filename( name, true );
  gtk_signal_handler_unblock_by_func( GTK_OBJECT( editable ),
				      GTK_SIGNAL_FUNC( _change_filename ),
				      fs );
  gtk_editable_set_position( editable, cursor );
  
  g_free( name );
}
#endif

static void
_change_format (GtkWidget *, struct menu_info *id)
{
  id->fs->change_format( id->index );
}

static void
_delete_text (GtkEditable *ed, gint start, gint end, file_selector *fs)
{
  g_print ("%s\n", __func__);

#ifndef HAVE_GTK_2
  gtk_signal_handler_block_by_func (GTK_OBJECT (ed), 
				    GTK_SIGNAL_FUNC (_delete_text), fs);
  fs->delete_text (start, end);
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (ed),
				      GTK_SIGNAL_FUNC (_delete_text), fs);
#else
  g_signal_handlers_block_by_func (ed, (gpointer) _delete_text, fs);
  fs->delete_text (start, end);
  g_signal_handlers_unblock_by_func (ed, (gpointer) _delete_text, fs);
#endif

  g_signal_stop_emission_by_name (GTK1_OBJ(ed), "delete-text"); 
}

static void
_insert_text (GtkEditable *ed, gchar *text, gint len, gint *pos,
	      file_selector *fs)
{
  g_print ("%s\n", __func__);

#ifndef HAVE_GTK_2
  gtk_signal_handler_block_by_func (GTK_OBJECT (ed),
				    GTK_SIGNAL_FUNC ( _insert_text), fs);
  fs->insert_text (text, len, pos);
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (ed),
				      GTK_SIGNAL_FUNC (_insert_text), fs);
#else
  g_signal_handlers_block_by_func (ed, (gpointer) _insert_text, fs);
  fs->insert_text (text, len, pos);
  g_signal_handlers_unblock_by_func (ed, (gpointer) _insert_text, fs);
#endif

  g_signal_stop_emission_by_name (GTK1_OBJ(ed), "insert-text"); 
}

static void
_change_seqnum (GtkAdjustment *, file_selector *fs)
{
  fs->change_seqnum ();
}

static void
_change_digits (GtkAdjustment *, file_selector *fs)
{
  fs->change_digits ();
}


// 	class implementation

			 	// define static variables
const file_selector::format
file_selector::_format[]  = {
  { "PNM", 0, "pnm", "pnm" },
  { "PNG", 0, "png", "png" },
  { "JPEG", 0, "jpg", "jpg|jpeg" },
  { "TIFF", 0, "tiff", "tiff" },
  { 0 }				// array terminator
};

void
file_selector::init ()
{
  g_print ("%s\n", __func__);

  _widget   = NULL;
  _filename = NULL;
  _pathname = NULL;

  _using_adf = false;
  _file_type = 0;
  _deleted_all = false;

  _number = -1;
  _seqnum = NULL;
  _digits = NULL;

  int size = 1;			// terminating { 0 } element
  for (int i = 0; 0 != _format[i].name; ++i)
    ++size;
  _fmt = new const format * [size];
				// okay, so we may waste a few

  int cnt = 0;
  int len = 0;
  for (int i = 0; i < size - 1; ++i)
    {
      _fmt[cnt] = &_format[i];	// FIXME: check support
      len += strlen (_fmt[cnt]->rgx);
      ++len;			// for the '|'
      ++cnt;
    }
  _fmt[cnt] = 0;		// array terminator

  _ext_regex = new char[len];	// overwrite last '|' with '\0'
  char *pos = _ext_regex;
  for (int i = 0; _fmt[i]; ++i)
    {
      strcpy (pos, _fmt[i]->rgx);
      pos += strlen (_fmt[i]->rgx);
      *pos++ = '|';		// overwrite '\0', then advance
    }
  *--pos = '\0';		// go back one, then terminate

  g_print ("%s: ext_regex =`%s'\n", __func__, _ext_regex);
}

void
file_selector::create_window( GtkWidget *parent, int option,
			      bool enable_start_button, bool monochrome)
{
  create_window (GTK_WINDOW (gtk_widget_get_parent (parent)), parent,
		 (   PISA_OP_ADF     == option
		  || PISA_OP_ADFDPLX == option
		  || enable_start_button));
}

void
file_selector::create_window (GtkWindow *window, GtkWidget *parent,
			      bool do_consecutive_scanning)
{
  g_print ("%s: enter\n", __func__);

  _parent    = parent;
  _using_adf = do_consecutive_scanning;

  _widget = GTK_FILE_SELECTION (gtk_file_selection_new (PACKAGE));

  if (!_widget)
    throw pisa_error( PISA_STATUS_NO_MEM );

  gtk_window_set_transient_for (GTK_WINDOW (_widget), window);
  gtk_file_selection_hide_fileop_buttons (_widget);
  add_dialog_extensions ();
				// needs to be called before we canonize

  if (!_filename)
    _filename = canonize ("default");  

  free (_pathname);
  _pathname = 0;
				// FIXME? 0 == _filename
  gtk_file_selection_set_filename (_widget, _filename);

  g_signal_connect_swapped (GTK1_OBJ (_widget->ok_button), "clicked",
			    G_CALLBACK (_click_ok), GTK1_OBJ (this));
  g_signal_connect_swapped (GTK1_OBJ (_widget->cancel_button), "clicked",
			    G_CALLBACK (_destroy), GTK1_OBJ (this));
  g_signal_connect_swapped (GTK1_OBJ (_widget), "delete_event",
			    G_CALLBACK (_destroy), GTK1_OBJ (this));
  g_signal_connect_swapped (GTK1_OBJ (_widget), "destroy",
			    G_CALLBACK (gtk_main_quit), GTK1_OBJ (_widget));

  // We need to interfere with users selecting and editing filenames
  // to guarantee a correct extension and the appropriate templating
  // bit when _using_adf.
  g_signal_connect (GTK1_OBJ (_widget->selection_entry), "delete-text",
		    G_CALLBACK (_delete_text), this);
  g_signal_connect (GTK1_OBJ (_widget->selection_entry), "insert-text",
		    G_CALLBACK (_insert_text), this);

  gtk_widget_show (GTK_WIDGET (_widget));
  gtk_grab_add (GTK_WIDGET (_widget));
  gtk_main ();
  if (_widget)
    {
      gtk_grab_remove (GTK_WIDGET (_widget));
    }
  g_print ("%s: exit\n", __func__);
}

void
file_selector::destroy ()
{
  g_print ("%s\n", __func__);

  if (_using_adf)
    {
      hide ();
    }
  else
    {
      destroy (true);
    }
}

void
file_selector::destroy (bool really)
{
  g_print ("%s (%i)\n", __func__, really);

  free (_filename);
  _filename = NULL;

  if (_widget)
    {
      gtk_widget_destroy (GTK_WIDGET (_widget));
    }
  _widget = NULL;

  if (_parent)
    {
      gtk_widget_set_sensitive (_parent, true);
    }
  _parent = NULL;

  _number = -1;
  _seqnum =  NULL;		// gtk_widget_destroy cleans these up
  _digits =  NULL;

  _using_adf = false;
  _file_type = 0;
  _deleted_all = false;

  delete [] _fmt;
  _fmt = NULL;
  delete [] _ext_regex;
  _ext_regex = NULL;
}

char *
file_selector::get_filename () const
{
  return (_pathname ? strdup (_pathname) : NULL);
}

bool
file_selector::set_filename( const char *text, bool edit )
{
  g_print ("file_selector::set_filename( %s, %d )\n", text, edit);

  if (!_filename)
    return false;		// logical error but return for now

  if (!text
      || !*text			// empty string
      || text == _filename || 0 == strcmp( text, _filename ))
    return false;		// nothing to change

  g_print ("filename bfore = `%s'\n", _filename);
  g_print ("filename inGUI = `%s'\n",
	   gtk_file_selection_get_filename( _widget ));

  char *old_name  = _filename;	// hang on to original
#ifdef HAVE_GTK_2
  {				// editing file extension not allowed
    int n = strrchr( _filename, '.') - _filename;
    edit = (0 == strncmp (text, _filename, n));
  }
#endif
  _filename = (edit ? validate( text ) : canonize( text ));

  if (_filename)		// we got a new name
    free( old_name );
  else
    _filename = old_name;	// revert to original

				// update the GUI
  gtk_file_selection_set_filename( _widget, _filename );

  g_print ("filename after = `%s'\n", _filename );
  g_print ("filename inGUI = `%s'\n",
	   gtk_file_selection_get_filename( _widget ) );

  g_print ("set_filename returns %d\n", _filename != old_name );
  return _filename != old_name;
}

int
file_selector::get_sequence_number () const
{
  return _number;
}

void
file_selector::set_sequence_number (int number)
{
  if (!_using_adf || !_seqnum)
    return;

  _number = number;

  while (_number > _seqnum->upper)
    {
      int max = int (_seqnum->upper) * 10 + 9;
      _seqnum->upper = max;
    }
  gtk_adjustment_set_value (_seqnum, _number);
  gtk_adjustment_changed (_seqnum);
}

bool
file_selector::save_pathname()
{
  g_print ("%s\n", __func__);

  if (!_widget)
    {
      g_print ("%s: widget's gone!\n", __func__);
      free (_pathname);
      _pathname = NULL;
      return true;
    }

  const char *current = gtk_file_selection_get_filename (_widget);

  if (128 < strlen (current)	// blame the spec for this one
      || !permission (current))
    {
      show_message (pisa_error (PISA_ERR_FILENAME));
      return false;
    }
  if (0 == access (current, F_OK))
    {
      bool ok = show_message (pisa_error (PISA_ERR_OVERWRITE), true);
      if (!ok)
	return false;
    }

  char *new_name = (char *) malloc ((strlen (current) + 1)
				    * sizeof (char));
  if (!new_name)
    throw pisa_error (PISA_STATUS_NO_MEM);

  strcpy (new_name, current);
  free (_pathname);
  _pathname = new_name;
  free (_filename);
  _filename = 0;

  g_print ("%s: pathname = `%s'\n", __func__, _pathname);

  return true;
}

void
file_selector::hide () const
{
  g_print ("%s\n", __func__);

  if (_using_adf)
    {
      g_print ("%s: calling gtk_widget_hide\n", __func__);
      if (_widget)
	{
	  gtk_widget_hide (GTK_WIDGET (_widget));
	}
      if (_parent)
	{
	  gtk_widget_set_sensitive (_parent, true);
	}
    }
}

void
file_selector::set_entry (const char *text)
{
  g_print ("%s (%s)\n", __func__, text);

  if (!_filename)
    return;			// logical error but return for now

  if (!text
      || !*text			// empty string
      || text == _filename
      || 0 == strcmp (text, _filename))
    return;			// nothing to change

  char *new_name = (canonize (text));
  if (new_name)
    {
      free (_filename);
      _filename = new_name;
#ifndef HAVE_GTK_2
      gtk_signal_handler_block_by_func (GTK_OBJECT (_widget->selection_entry),
					GTK_SIGNAL_FUNC (_delete_text), this);
      gtk_signal_handler_block_by_func (GTK_OBJECT (_widget->selection_entry),
					GTK_SIGNAL_FUNC (_insert_text), this);
      gtk_entry_set_text (GTK_ENTRY (_widget->selection_entry), _filename);
      gtk_signal_handler_unblock_by_func (GTK_OBJECT (_widget->selection_entry),
					  GTK_SIGNAL_FUNC (_insert_text), this);
      gtk_signal_handler_unblock_by_func (GTK_OBJECT (_widget->selection_entry),
					  GTK_SIGNAL_FUNC (_delete_text), this);
#else
      g_signal_handlers_block_by_func (GTK_EDITABLE (_widget->selection_entry),
				       (gpointer) _delete_text, this);
      g_signal_handlers_block_by_func (GTK_EDITABLE (_widget->selection_entry),
				       (gpointer) _insert_text, this);
      gtk_entry_set_text (GTK_ENTRY (_widget->selection_entry), _filename);
      g_signal_handlers_unblock_by_func (GTK_EDITABLE (_widget->selection_entry),
					 (gpointer) _insert_text, this);
      g_signal_handlers_unblock_by_func (GTK_EDITABLE (_widget->selection_entry),
					 (gpointer) _delete_text, this);
#endif
    }
}

void
file_selector::delete_text (gint start, gint end)
{
  g_print ("%s (%d,%d)\n", __func__, start, end);
  g_print ("%s orig _filename '%s'\n", __func__, _filename);

  _deleted_all = ((end - start) == (gint) strlen (_filename));

  if (0 > end) end = strlen (_filename) + 1;
  int dot = strrchr (_filename, (_using_adf ? '-' : '.')) - _filename;
  if (end > dot) end = dot;

  if (start < end)
    {
      GtkWidget *ed = _widget->selection_entry;
      gtk_editable_delete_text (GTK_EDITABLE (ed), start, end);
      _filename = strdup (gtk_entry_get_text (GTK_ENTRY (ed)));

      g_print ("%s new  _filename '%s'\n", __func__, _filename);
    }
}

void
file_selector::insert_text (gchar *text, gint len, gint *pos)
{
  g_print ("%s (%s,%d,%d)\n", __func__, text, len, (pos ? *pos : -1));
  g_print ("%s orig _filename '%s'\n", __func__, _filename);

  if (_deleted_all && pos && (0 == *pos))
    {
      _deleted_all = false;
      text = canonize (text);
      len  = strlen (text);

      GtkEditable *ed = GTK_EDITABLE (_widget->selection_entry);
#ifndef HAVE_GTK_2
      gtk_signal_handler_block_by_func (GTK_OBJECT (ed),
					GTK_SIGNAL_FUNC (_delete_text), this);
      gtk_editable_delete_text (ed, 0, -1);
      gtk_signal_handler_unblock_by_func (GTK_OBJECT (ed),
					GTK_SIGNAL_FUNC (_delete_text), this);
#else
      g_signal_handlers_block_by_func (ed, (gpointer) _delete_text, this);
      gtk_editable_delete_text (ed, 0, -1);
      g_signal_handlers_unblock_by_func (ed, (gpointer) _delete_text, this);
#endif
    }

  int dot = strrchr (_filename, (_using_adf ? '-' : '.')) - _filename;

  if (pos && (*pos <= dot))
    {
      GtkWidget *ed = _widget->selection_entry;
      gtk_editable_insert_text (GTK_EDITABLE (ed), text, len, pos);
#ifndef HAVE_GTK_2
      {				// not getting delete-event's for some
				// reason
	char *name = canonize (gtk_entry_get_text (GTK_ENTRY (ed)));
	gtk_entry_set_text (GTK_ENTRY (ed), name);
	free (name);
      }
#endif
      free (_filename);
      _filename = strdup (gtk_entry_get_text (GTK_ENTRY (ed)));

      g_print ("%s new  _filename '%s'\n", __func__, _filename);
    }
}

void
file_selector::change_format (int index)
{
  g_print ("%s (%d)\n", __func__, index);

  _file_type = index;
				// reflect changes in filename
  char *canon = canonize (_filename);
  set_entry (canon);
  free (canon);
}

void
file_selector::change_seqnum ()
{
  g_print ("%s\n", __func__);

  _number = int (_seqnum->value);
}

void
file_selector::change_digits ()
{
  g_print ("%s\n", __func__);

  int max = 9;			// there's at least one digit
  for (int i = 2; i <= int (_digits->value); ++i)
    {
      max *= 10;
      max +=  9;
    }

  if (_seqnum)
    {
      _seqnum->upper = max;
      if (max < _seqnum->value)	// also updates the GUI
	gtk_adjustment_set_value (_seqnum, max);

      gtk_adjustment_changed (_seqnum);
    }

  if (_filename)		// reflect changes in filename
    {
      char *canon = canonize (_filename);
      set_entry (canon);
      free (canon);
    }
}

// caller needs to free returned char *
char *
file_selector::validate (const char *text) const
{
  if (!text || 0 == *text)
    return 0;

  g_print ("%s (%s)\n", __func__, text);

  char *valid = 0;

  // match extension with filetype
  // match number of #'s with number of digits iff using ADF

  const char *regex_fmt = "^(.+)(-#{%d}){%d}\\.(%s)$";
  const char *ext_regex = _fmt[_file_type]->rgx;
  char *regex = new char[strlen( regex_fmt ) - 3	// 3 formatters
			 + strlen( ext_regex ) + 1];	// final '\0'
  sprintf(regex, regex_fmt,
	  (_digits    ? int (_digits->value) : 0),
	  (_using_adf ?      1               : 0),
	  ext_regex);

  regex_t *comp_regex = new regex_t;
  int comp = regcomp (comp_regex, regex, REG_EXTENDED);

  if (0 == comp)
    {
      int result = regexec (comp_regex, text, 0, 0, 0);
      if (0 == result)
	{
	  valid = (char *) malloc ((strlen (text) + 1)
				   * sizeof (char));
	  if (valid)
	    strcpy (valid, text);
	  else
	    {			// FIXME!
	      // no memory
	    }
	}
      else
	if (REG_NOMATCH != result)
	  regerror (comp, comp_regex);
    }
  else
    regerror (comp, comp_regex);

  regfree (comp_regex);
  delete comp_regex;
  delete[] regex;

  return valid;
}

// caller needs to free returned char *, because we validate()
char *
file_selector::canonize (const char *text) const
{
  if (!text || 0 == *text)
    return 0;

  g_print ("%s (%s)\n", __func__, text);

  char *interim = 0;

  {	    // "replace" existing extension with one matching filetype
    const char * ext_regex_fmt = "^(.+)\\.(%s)$";
    char *regex = new char[strlen (ext_regex_fmt) - 1
			   + strlen (_ext_regex) + 1];
    sprintf (regex, ext_regex_fmt, _ext_regex);

    regex_t *comp_regex = new regex_t;
    int comp = regcomp (comp_regex, regex, REG_EXTENDED);

    if (0 == comp)
      {
	char *file_ext = _fmt[_file_type]->ext;

	size_t      nsub  = comp_regex->re_nsub + 1;
	regmatch_t *match = new regmatch_t[nsub];

	int result = regexec (comp_regex, text, nsub, match, 0);
	if (0 == result)
	  {			// replace existing extension
	    // FIXME: keep extension if in _fmt[_file_type]->rgx
	    int pre = 1;

	    regoff_t l_pre = match[pre].rm_eo - match[pre].rm_so;

	    size_t len = l_pre + strlen (file_ext) + 2;
				// period and terminating '\0'
	    interim = (char *) malloc (len * sizeof (char));
	    if (interim)
	      {
		char *c = interim;
		{		// copy prefix
		  const char *p = text + match[pre].rm_so;
		  while (0 < l_pre--)
		    *c++ = *p++;
		}
		{		// append extension
		  *c++ = '.';
		  while (*file_ext)
		    *c++ = *file_ext++;
		  *c++ = '\0';
		}
	      }
	    else
	      {			// FIXME!
		// no memory
	      }
	  } // 0 == result
	else
	  if (REG_NOMATCH == result)
	    {			// append a new extension
	      size_t len = strlen (text) + strlen (file_ext) + 2;
				// period and terminating '\0'
	      interim = (char *) malloc (len * sizeof (char));
	      if (interim)
		sprintf (interim, "%s.%s", text, file_ext);
	      else
		{		// FIXME!
		  // no memory
		}
	    }
	  else
	    regerror (result, comp_regex);
	delete[] match;
      }	// 0 == comp
    else
      regerror (comp, comp_regex);

    regfree (comp_regex);
    delete comp_regex;
    delete[] regex;
  }

  // FIXME: free interim if one of the new's throws

  if (interim && _using_adf)
    {	   // adjust the number of #'s to the current number of digits

      const char * adf_regex_fmt = "^(((.+)-#+)|(.+))\\.(%s)$";
      char *regex = new char[strlen (adf_regex_fmt) - 1
			     + strlen (_ext_regex) + 1];
      sprintf (regex, adf_regex_fmt, _ext_regex);

      regex_t *comp_regex = new regex_t;
      int comp = regcomp (comp_regex, regex, REG_EXTENDED);

      if (0 == comp)
	{
	  size_t      nsub  = comp_regex->re_nsub + 1;
	  regmatch_t *match = new regmatch_t[nsub];

	  int result = regexec (comp_regex, interim, nsub, match, 0);
	  if (0 == result)
	    {			// see adf_regex_fmt
	      int pre = (-1 == match[4].rm_so
			 ? 3	// already contains sequence template
			 : 4);
	      int ext = 5;

	      regoff_t l_pre = match[pre].rm_eo - match[pre].rm_so;
	      regoff_t l_ext = match[ext].rm_eo - match[ext].rm_so;

	      size_t len = l_pre + l_ext + int( _digits->value ) + 3;
				// hyphen, period and terminating '\0'
	      char *name = (char *) malloc (len * sizeof (char));
	      if (name)
		{
		  char *c = name;
		  {		// copy prefix
		    const char *p = interim + match[pre].rm_so;
		    while (0 < l_pre--)
		      *c++ = *p++;
		  }
		  {		// insert -### part
		    *c++ = '-';
		    for (int i = 0; i < int (_digits->value); ++i)
		      *c++ = '#';
		  }
		  {		// append extension
		    *c++ = '.';
		    const char *p = interim + match[ext].rm_so;
		    while (0 < l_ext--)
		      *c++ = *p++;
		  }
		  *c = '\0';
		} // name
	      else
		{		// FIXME!
		  // no memory
		}
	      free (interim);
	      interim = name;
	    } // 0 == result
	  else
	    if (REG_NOMATCH != result)
	      regerror (result, comp_regex);
	  delete[] match;
	} // 0 == comp
      else
	regerror (comp, comp_regex);

      regfree (comp_regex);
      delete comp_regex;
      delete[] regex;
    } // _using_adf

  char *result = validate (interim);
  free (interim);		// clean up what we allocated

  return result;
}

void
file_selector::add_dialog_extensions ()
{
  g_print ("%s: enter\n", __func__);

  GtkWidget *frame = gtk_frame_new (_("Save Options"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);

  GtkTable *opts
    = (GtkTable *) gtk_table_new ((_using_adf ? 3 : 1), 2, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (opts));

  GtkWidget *w = 0;
  {				// file type selector
    w = gtk_label_new (_("Determine File Type:"));
    gtk_label_set_justify (GTK_LABEL (w), GTK_JUSTIFY_LEFT);
    gtk_table_attach_defaults (opts, w, 0, 1, 0, 1);
    gtk_misc_set_alignment (GTK_MISC (w), 0.1, 0.5);
    gtk_widget_show (w);

    w = gtk_option_menu_new ();
    {
      GtkWidget *m = gtk_menu_new ();

      for (int i = 0; _fmt[i]; ++i)
	{
	  GtkWidget *mi = gtk_menu_item_new_with_label (_fmt[i]->name);
	  gtk_menu_append (GTK_MENU (m), mi);
	  gtk_widget_show (mi);

	  struct menu_info *id = new struct menu_info;
	  id->fs = this;
	  id->index = i;

	  g_signal_connect (GTK1_OBJ (mi), "activate",
			    G_CALLBACK (_change_format), (void *) id);

	  // FIXME! ugly kludge to "check" for support
	  if (0 == strcmp ("PNM", _fmt[i]->name))
	    {
	      gtk_widget_set_sensitive (mi, iscan::pnmstream::is_usable ());
	    }
	  if (0 == strcmp ("PNG", _fmt[i]->name))
	    {
	      gtk_widget_set_sensitive (mi, iscan::pngstream::is_usable ());
	    }
	  if (0 == strcmp ("JPEG", _fmt[i]->name))
	    {
	      gtk_widget_set_sensitive (mi, iscan::jpegstream::is_usable ());
	    }
	  // FIXME: this is only here to show what the GUI would look like
	  //	TIFF support is planned but was not ready in time for this
	  //	release :-(
	  if (0 == strcmp ("TIFF", _fmt[i]->name))
	    {
	      gtk_widget_set_sensitive (mi, false);
	    }
	}
      gtk_option_menu_set_menu( GTK_OPTION_MENU( w ), m );
    }
    gtk_table_attach_defaults (opts, w, 1, 2, 0, 1);
    gtk_widget_show (w);
  }

  if (_using_adf)
    {
      g_print ("%s: using adf\n", __func__);

 				// sequence number selector
      w = gtk_label_new (_("Start filing at:"));
      gtk_table_attach_defaults (opts, w, 0, 1, 1, 2);
      gtk_misc_set_alignment (GTK_MISC (w), 0.1, 0.5);
      gtk_widget_show (w);

      if (!_seqnum)
	_seqnum = (GtkAdjustment *) gtk_adjustment_new (1, 0, 9, 1, 10, 0);
      if (_seqnum)
	_number = int (_seqnum->value);
      else			// play it safe and make sure
	_number = -1;

      w = gtk_spin_button_new (_seqnum, 0, 0);
      gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (w), true);
      gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (w), true);
      gtk_table_attach_defaults (opts, w, 1, 2, 1, 2);
      gtk_widget_show (w);

				// number of digits selector
      w = gtk_label_new (_("Number of digits:"));
      gtk_table_attach_defaults (opts, w, 0, 1, 2, 3);
      gtk_misc_set_alignment (GTK_MISC (w), 0.1, 0.5);
      gtk_widget_show (w);

      if (!_digits)
	_digits = (GtkAdjustment *) gtk_adjustment_new (3, 1, 6, 1,  1, 0);
      if (_digits)
	change_digits();

      w = gtk_spin_button_new (_digits, 0, 0);
      gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (w), true);
      gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (w), true);
      gtk_table_attach_defaults (opts, w, 1, 2, 2, 3);
      gtk_widget_show (w);

      g_signal_connect (GTK1_OBJ (_seqnum), "value_changed",
			G_CALLBACK (_change_seqnum), this);
      g_signal_connect (GTK1_OBJ (_digits), "value_changed",
			G_CALLBACK (_change_digits), this);
   }

  gtk_box_pack_end (GTK_BOX (_widget->main_vbox), frame, false, false, 0);

  gtk_widget_show (frame);
  gtk_widget_show (GTK_WIDGET (opts));  

  g_print ("%s: exit\n", __func__);
}

void
file_selector::add_dialog_extensions (bool)
{
  return add_dialog_extensions ();
}

bool
file_selector::permission( const char *file ) const
{
  if (0 == access( file, F_OK ))	// file exists
    return (0 == access( file, W_OK ));	// whether we can write to it

  // check write access to the directory (note that we need execute
  // privileges as well)

  char *slash = strrchr( file, '/');
  *slash = '\0';		// temporarily truncate to dirname
  const char *dir = (file == slash
		     ? "/"	// whoops!, file in root directory
		     : file);

  bool w_ok = false;		// assume the worst
  if (0 == access( dir, F_OK ))
    w_ok = (0 == access( dir, W_OK | X_OK ));

  *slash = '/';			// restore filename

  return w_ok;
}

bool
file_selector::show_message( const pisa_error& oops, bool yes_no ) const
{
  aleart_dialog dlg;

  if (yes_no)			// binary question
    return (1 == dlg.message_box( GTK_WIDGET( _widget ),
				  oops.get_error_string(),
				  _( "  Yes  " ), _( "  No  " ) ));

  dlg.message_box( GTK_WIDGET( _widget ), oops.get_error_string() );
  return true;
}

void
file_selector::regerror (int code, regex_t *regex) const
{
  size_t length = ::regerror (code, regex, 0, 0);
  char *message = new char[length];

  ::regerror (code, regex, message, length);
  fprintf (stderr, "%s\n", message);

  delete[] message;
}
