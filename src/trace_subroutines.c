/*
 * trace_subroutines.c
 *
 * Copyright © 2020 by John Sauter <John_Sauter@systemeyescomputerstore.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gst/gst.h>
#include "time_subroutines.h"
#include "trace_subroutines.h"
#include "sound_effects_player.h"
#include "main.h"

/* the persistent data used by the trace subroutines */
struct trace_info
{
  gchar *file_name;
  gint sequencer_level;
  gint fid;
  gint file_open;
};

/* Subroutines for tracing.  */

/* Initialize the trace subroutines.  */
void *
trace_init (GApplication * app)
{
  struct trace_info *trace_data;

  trace_data = g_malloc (sizeof (struct trace_info));
  trace_data->sequencer_level = main_get_trace_sequencer_level (app);
  trace_data->file_name = main_get_trace_file_name (app);
  trace_data->file_open = 0;
  
  if ((trace_data->file_name != NULL) && (trace_data->sequencer_level > 0))
    {
      /* Open the trace file for append.  */
      trace_data->fid =
        g_open (trace_data->file_name,
                O_WRONLY | O_APPEND | O_CREAT | O_DSYNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
      if (trace_data->fid == -1)
        {
          g_printf ("Trace file open failure on %s: %s.\n",
                    trace_data->file_name, g_strerror (errno));
          trace_data->sequencer_level = 0;
        }
      else
	trace_data->file_open = 1;
    }
  
  return (trace_data);
}

/* Finalize the trace subroutines.  */
void
trace_finalize (GApplication * app)
{
  struct trace_info *trace_data;

  trace_data = sep_get_trace_data (app);
  if (trace_data->file_open == 1)
    g_close (trace_data->fid, NULL);
  trace_data->file_open = 0;
  g_free (trace_data);
  return;
}

/* See if the sequencer should be traceing.  */
gint
trace_sequencer_level (GApplication * app)
{
  struct trace_info *trace_data;

  trace_data = sep_get_trace_data (app);
  if (trace_data->file_open == 0)
    return (0);
  
  return (trace_data->sequencer_level);
}

/* Subroutine to return the current time as a string.  */
static gchar *
trace_current_time (GApplication * app)
{
  struct tm current_time_tm, local_time_tm;
  gint nanoseconds;
  gchar string_buffer[64];
  gchar *result;

  /* Get the current time into a tm structure.  */
  time_current_tm_nano (&current_time_tm, &nanoseconds);

  /* Convert to local time.  */
  time_UTC_to_local (&current_time_tm, &local_time_tm, INT_MIN);

  /* Express the local time as a string.  */
  time_tm_nano_to_string (&local_time_tm, nanoseconds,
			  &string_buffer[0], sizeof (string_buffer));
  result = g_strdup (&string_buffer[0]);
  return (result);
}

/* Write a line into the trace file from the sequencer.  */
void
trace_sequencer_write (gchar * line, GApplication * app)
{
  struct trace_info *trace_data;
  size_t bytes_to_write;
  gchar *current_time;
  gchar *output_string;
  ssize_t bytes_written;

  trace_data = sep_get_trace_data (app);

  /* Prepend the current time onto the string.  */
  current_time = trace_current_time (app);
  output_string = g_strdup_printf ("%s %s\n", current_time, line);
  g_free (current_time);
  current_time = NULL;
  bytes_to_write = strlen (output_string);
  bytes_written = write (trace_data->fid, output_string, bytes_to_write);
  if (bytes_written < 0)
    {
      g_printf ("Trace file write failure on %s: %s.\n",
                trace_data->file_name, g_strerror (errno));
      trace_data->sequencer_level = 0;
    }
  g_free (output_string);
  output_string = NULL;

  return;
}
