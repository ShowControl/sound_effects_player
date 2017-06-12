/*
 * main.c
 * Copyright © 2017 by John Sauter <John_Sauter@systemeyescomputerstore.com>
 *
 * sound_effects_player is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * sound_effects_player is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "main.h"
#include "sound_effects_player.h"

#include <glib/gi18n.h>

/* Persistent data.  */
static gchar *monitor_file_name = NULL;
static gchar *audio_output_string = NULL;
static gchar *device_name_string = NULL;
static gchar *trace_file_name = NULL;
static gint trace_sequencer_level = 0;

/* The entry point for the sound_effects_player application.  
 * This is a GTK application, so much of what is done here is standard 
 * boilerplate and command-line argument processing.  
 */
int
main (int argc, char *argv[])
{
  Sound_Effects_Player *app;
  int status;
  gchar **filenames = NULL;
  gchar *pid_file_name = NULL;
  gboolean pid_file_written = FALSE;
  FILE *pid_file = NULL;
  const GOptionEntry entries[] = {
    {"process-id-file", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME,
     &pid_file_name,
     "name of the file written with the process id, for signaling"},
    {"monitor-file", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME,
     &monitor_file_name, "name of the file which records produced sound"},
    {"audio-output", 'a', 0, G_OPTION_ARG_STRING, &audio_output_string,
     "type of audio output: ALSA, none, jack, pulse"},
    {"device-name", 'd', 0, G_OPTION_ARG_STRING, &device_name_string,
     "for ALSA output, name of the device"},
    {"trace-file", 't', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME,
     &trace_file_name, "name of the file for trace output"},
    {"trace-sequencer-level", 'v', 0, G_OPTION_ARG_INT,
     &trace_sequencer_level,
     "The amount of sequencer tracing: 0 = none, 1 = all"},
    /* add more command line options here */
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &filenames,
     "Special option that collects any remaining arguments for us"},
    {NULL,}
  };

  GOptionContext *ctx;
  GError *err = NULL;
  const gchar *nano_str;
  const gchar *check_version_str;
  guint major, minor, micro, nano;
  extern const guint glib_major_version;
  extern const guint glib_minor_version;
  extern const guint glib_micro_version;
  extern const guint glib_binary_age;
  extern const guint glib_interface_age;
  int fake_argc;
  char *fake_argv[2];

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  /* Initialize gtk and Gstreamer. */
  gtk_init (&argc, &argv);

  /* Parse the command line.  */
  ctx = g_option_context_new ("[project_file]");
  g_option_context_add_group (ctx, gtk_get_option_group (TRUE));
  g_option_context_add_group (ctx, gst_init_get_option_group ());
  g_option_context_add_main_entries (ctx, entries, NULL);
  g_option_context_set_summary (ctx, "Play sound effects for ShowControl.");

  if (!g_option_context_parse (ctx, &argc, &argv, &err))
    {
      g_print ("Error initializing: %s\n", GST_STR_NULL (err->message));
      return -1;
    }
  g_option_context_free (ctx);

  /* If a process ID file was specified, write our process ID to it.  */
  if (pid_file_name != NULL)
    {
      errno = 0;
      pid_file = fopen (pid_file_name, "w");
      if (pid_file != NULL)
        {
          fprintf (pid_file, "%d\n", getpid ());
          fclose (pid_file);
          pid_file_written = TRUE;
        }
      else
        {
          g_print ("Cannot create process ID file %s: %s", pid_file_name,
                   strerror (errno));
          return 1;
        }
    }

  /* Print the version of glib that we are linked against. */
  g_print ("This program is linked against glib %d.%d.%d ages %d and %d.\n",
           glib_major_version, glib_minor_version, glib_micro_version,
           glib_binary_age, glib_interface_age);

  /* Print the version of gtk that we are linked against. */
  major = gtk_get_major_version ();
  minor = gtk_get_minor_version ();
  micro = gtk_get_micro_version ();
  g_print ("This program is linked against gtk %d.%d.%d.\n", major, minor,
           micro);

  /* Check that the version of gtk is good. */
  check_version_str =
    gtk_check_version (GTK_MAJOR_VERSION, GTK_MINOR_VERSION,
                       GTK_MICRO_VERSION);
  if (check_version_str != NULL)
    {
      g_print (check_version_str);
      return -1;
    }

  /* Print the version of Gstreamer that we are linked against. */
  gst_version (&major, &minor, &micro, &nano);

  if (nano == 1)
    nano_str = "(CVS)";
  else if (nano == 2)
    nano_str = "(Prerelease)";
  else
    nano_str = "";
  g_print ("This program is linked against GStreamer %d.%d.%d%s.\n", major,
           minor, micro, nano_str);

  /* Initialize gstreamer */
  gst_init (&argc, &argv);

  /* Run the program.  The values from argc and argv have already been
   * parsed, so create a fake version of argc and argv with the filename
   * argument, if present.  */
  fake_argc = argc;
  fake_argv[0] = argv[0];
  if ((filenames != NULL) && (filenames[0] != NULL))
    {
      fake_argc = 2;
      fake_argv[1] = filenames[0];
    }
  app = sound_effects_player_new ();
  status = g_application_run (G_APPLICATION (app), fake_argc, fake_argv);

  /* We are done. */
  g_object_unref (app);

  /* If we wrote a file with the process ID, delete it.  */
  if (pid_file_written)
    remove (pid_file_name);
  free (pid_file_name);
  pid_file_name = NULL;

  free (monitor_file_name);
  monitor_file_name = NULL;
  free (audio_output_string);
  audio_output_string = NULL;
  free (device_name_string);
  device_name_string = NULL;
  free (trace_file_name);
  trace_file_name = NULL;

  return status;
}

/* Fetch the command line options.  */
gchar *
main_get_monitor_file_name ()
{
  return monitor_file_name;
}

gchar *
main_get_audio_output_string ()
{
  return audio_output_string;
}

gchar *
main_get_device_name_string ()
{
  return device_name_string;
}

gchar *
main_get_trace_file_name ()
{
  return trace_file_name;
}

gint
main_get_trace_sequencer_level ()
{
  return trace_sequencer_level;
}

/* End of file main.c */
