/*
 * sound_subroutines.h
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

#include <gtk/gtk.h>
#include <gst/gst.h>
#include "sound_structure.h"

/* Subroutines defined in sound_subroutines.c */

/* Initialize the sound system.  */
void *sound_init (GApplication *app);

/* Shut down the sound system.  */
void sound_finish (GApplication *app);

/* Start the sound system. */
GstPipeline *sound_start (GApplication *app);

/* Set the name of a cluster.  */
void sound_cluster_set_name (gchar *sound_name, guint cluster_number,
                             GApplication *app);

/* Append a sound to the list of sounds.  */
void sound_append_sound (struct sound_info *sound_data, GApplication *app);

/* Associate a sound with a cluster.  */
struct sound_info *sound_bind_to_cluster (gchar *sound_name,
                                          guint cluster_number,
                                          GApplication *app);

/* Disassociate a sound from its cluster.  */
void sound_unbind_from_cluster (struct sound_info *sound_data,
                                GApplication *app);

/* Given a widget, find the corresponding sound effect.  */
struct sound_info
*sound_get_sound_effect_from_widget (GtkWidget *cluster_widget,
				     GApplication *app);

/* Start playing a sound.  */
void sound_start_playing (struct sound_info *sound_data, GApplication *app);

/* Stop playing a sound.  */
void sound_stop_playing (struct sound_info *sound_data, GApplication *app);

/* Get the elapsed time of a playing sound.  */
guint64 sound_get_elapsed_time (struct sound_info *sound_data,
                                GApplication *app);

/* Get the remaining time of a playing sound.  */
guint64 sound_get_remaining_time (struct sound_info *sound_data,
                                  GApplication *app);

/* Note that a sound has completed.  */
void sound_completed (const gchar *sound_name, GApplication *app);

/* Note that a sound has entered the release stage of its amplitude envelope.  
 */
void sound_release_started (const gchar *sound_name, GApplication *app);

/* The Pause button has been pushed.  */
void sound_button_pause (GApplication *app);

/* The Continue button has been pushed.  */
void sound_button_continue (GApplication *app);

/* Count the bit depth and number of sound channels in a WAV file.
 * Return 1 on success, 0 on failure.  */
gint
sound_parse_wav_file_header (const gchar *wav_file_name,
			     struct sound_info *sound_effect,
			     GApplication *app);

/* Add a channel description to a sound.  */
void sound_append_channel (struct channel_info *channel_data,
			   struct sound_info *sound_data,
			   GApplication *app);

/* Add a speaker description to a channel.  */
void sound_append_speaker (struct speaker_info *speaker_data,
			   struct channel_info *channel_data,
			   struct sound_info *sound_data,
			   GApplication *app);

/* Find the channel mask.  It has a bit set for each active speaker.  */
guint64 sound_get_channel_mask (GApplication *app);

/* Determine the volume of sound from the specified input channel
 * to the specified output channel.  */
gfloat
sound_mix_matrix_volume (gint in_chan, gint out_chan,
			 struct sound_info *sound_effect, GApplication *app);

/* Return the appreviated name of the speaker connected to the specified
 * output channel.  */
gchar *
sound_output_channel_name (gint output_channel, GApplication *app);

/* End of file sound_subroutines.h */
