/*
 * sound_subroutines.h
 *
 * Copyright © 2015 by John Sauter <John_Sauter@systemeyescomputerstore.com>
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
#include <gst/gst.h>
#include "sound_structure.h"

/* Subroutines defined in sound_subroutines.c */

/* Initialize the sounds. */
GstPipeline * sound_init (GApplication * app);

/* Append a sound to the list of sounds.  */
void sound_append_sound (struct sound_info *sound_data, GApplication * app);

/* Start playing a sound in a specified cluster.  */
void sound_cluster_start (int cluster_number, GApplication * app);

/* Stop playing a sound in a specified cluster.  */
void sound_cluster_stop (int cluster_number, GApplication * app);

/* Start playing a sound.  */
void sound_start_playing (struct sound_info *sound_data, GApplication * app);

/* Stop playing a sound.  */
void sound_stop_playing (struct sound_info *sound_data, GApplication * app);

/* Note that a sound has completed.  */
void sound_completed (const gchar * sound_name, GApplication * app);

/* Note that a sound has terminated.  */
void sound_terminated (const gchar * sound_name, GApplication * app);

/* End of file sound_subroutines.h */