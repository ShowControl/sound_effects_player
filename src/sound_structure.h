/*
 * sound_structure.h
 *
 * Copyright © 2020 by John Sauter <John_Sauter@systemeyescomputerstore.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

/* Only define the structure once per source file.  */
#ifndef SOUND_STRUCTURE_H
#define SOUND_STRUCTURE_H

/* Define the structure which holds the definition of a sound.
 * This structure is shared between sound_effects_player, 
 * parse_xml_subroutines, button_subroutines, sound_subroutines
 * and sequence_subroutines. */

#include <gtk/gtk.h>
#include <gst/gst.h>

struct sound_info
{
  gchar *name;                  /* name of the sound */
  gboolean disabled;            /* disabled because file is missing */
  gchar *wav_file_name;         /* name of the file holding the waveform */
  gchar *wav_file_name_full;    /* absolute path to the file */
  guint64 attack_duration_time; /* attack time, in nanoseconds */
  gdouble attack_level;         /* 1.0 means 100% of volume */
  guint64 decay_duration_time;  /* decay time, in nanoseconds */
  gdouble sustain_level;        /* 1.0 means 100% of volume */
  guint64 release_start_time;   /* release start time, in nanoseconds */
  guint64 release_duration_time;        /* release duration time, 
                                         * in nanoseconds */
  gboolean release_duration_infinite;   /* TRUE if duration is 
                                           infinite */
  gint64 loop_from_time;        /* loop from time, in nanoseconds */
  gint64 loop_to_time;          /* loop to time, in nanoseconds */
  gint loop_limit;              /* loop limit, a count */
  guint64 max_duration_time;    /* maximum time taken from WAV file,
                                 * in nanoseconds.  */
  guint64 start_time;           /* start time, in nanoseconds */
  gfloat designer_volume_level; /* 1.0 means 100% of waveform's volume */
  gfloat designer_pan;          /* -1.0 is left, 0.0 is center, 1.0 is right */
  gfloat default_volume_level;  /* Initial operator volume.  */
  gint MIDI_program_number;     /* MIDI program number, if specified */
  gboolean MIDI_program_number_specified;       /* TRUE if not empty */
  gint MIDI_note_number;        /* MIDI note number, if specified */
  gboolean MIDI_note_number_specified;  /* TRUE if not empty */
  gchar *function_key;          /* name of function key */
  gboolean function_key_specified;      /* TRUE if not empty */
  gboolean omit_panning;        /* Do not let the operator pan this sound.  */

  guint64 starting_time;        /* the time that the sound started playing.  */
  guint64 releasing_time;       /* the time that the sound entered the
                                 * release segment of its envelope.  */
  GtkWidget *cluster_widget;    /* The cluster this sound is in.  */
  GstBin *sound_control;        /* The Gstreamer bin for this sound effect */
  gint cluster_number;          /* The number of the cluster the sound is in */
  gboolean running;             /* The sound is playing.  */
  gboolean release_sent;        /* A Release command was given.  */
  gboolean release_has_started; /* The sound has started its release stage.  */
  gchar *format_name;           /* The format of the WAV file.  */
  gint channel_count;           /* The number of channels in this sound's wav
				 * file.  Momo = 1, stereo = 2, etc.  */
  guint64 channel_mask;         /* A bit set for each speaker.  */
  GList *channels;              /* Information about each channel.  */
};

struct channel_info
{
  gint number;                  /* 0, 1, ... */
  GList *speakers;              /* Information about each speaker.  */
};

struct speaker_info
{
  gchar *name;                 /* front_left, front_right, ... */
  gint speaker_code;           /* code for speaker name */
  gint output_channel;         /* number of output channel, based on
				* the overall channel mask.  */
  gfloat volume_level;         /* 1.0 means passthrough.  */
};
  
#endif /* ifndef SOUND_STRUCTURE_H */
/* End of file sound_structure.h */
