/*
 * main.h
 *
 * Copyright © 2017 by John Sauter <John_Sauter@systemeyescomputerstore.com>
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

#include <gtk/gtk.h>

/* Subroutines defined in main.c.  These are used to retrieve option
 * values specified on the command line.  */

gchar *main_get_monitor_file_name ();
gchar *main_get_audio_output_string ();
gchar *main_get_device_name_string ();
gchar *main_get_client_name_string ();
gchar *main_get_server_name_string ();
gchar *main_get_trace_file_name ();
gint main_get_trace_sequencer_level ();
gchar *main_get_configuration_file_name ();

/* End of file main.h */
