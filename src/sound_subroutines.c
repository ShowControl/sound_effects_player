/*
 * sound_subroutines.c
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

#include <stdlib.h>
#include <gst/gst.h>
#include "sound_subroutines.h"
#include "sound_structure.h"
#include "sound_effects_player.h"
#include "gstreamer_subroutines.h"
#include "button_subroutines.h"
#include "display_subroutines.h"
#include "sequence_subroutines.h"

#define TRACE_SOUND FALSE

/* The persistent data used by the sound subroutines.  */
struct sounds_info
{
  GList *sounds_list;              /* The list of sounds.  */
  guint64 channel_mask;            /* a bit set for each speaker */
  gpointer *speaker_abbreviations; /* A speaker name for each output channel.
                                    */
  gint speaker_count;              /* The number of speakers.  */
};
  
/* Subroutines for processing sounds.  */

/* Initialize the sound system.  */
void *
sound_init (GApplication *app)
{
  struct sounds_info *sounds_data;

  sounds_data = g_malloc (sizeof (struct sounds_info));
  sounds_data->sounds_list = NULL;
  sounds_data->channel_mask = 0;
  sounds_data->speaker_abbreviations = NULL;
  return (sounds_data);
}

/* Terminate the sound system.  */
void
sound_finish (GApplication *app)
{
  struct sounds_info *sounds_data;
  struct sound_info *sound_effect;
  struct channel_info *this_channel;
  struct speaker_info *this_speaker;
  GList *sound_effect_list, *channel_list, *speaker_list;
  GList *next_sound_effect, *next_channel, *next_speaker;
  
  /* Free all the heap storage allocated by the sound subroutines.  */
  sounds_data = sep_get_sounds_data (app);
  sound_effect_list = sounds_data->sounds_list;
  
  while (sound_effect_list != NULL)
    {
      sound_effect = sound_effect_list->data;
      next_sound_effect = sound_effect_list->next;
      g_free (sound_effect->name);
      g_free (sound_effect->wav_file_name);
      g_free (sound_effect->wav_file_name_full);
      g_free (sound_effect->function_key);
      channel_list = sound_effect->channels;
      while (channel_list != NULL)
	{
	  next_channel = channel_list->next;
	  this_channel = channel_list->data;
	  speaker_list = this_channel->speakers;
	  while (speaker_list != NULL)
	    {
	      next_speaker = speaker_list->next;
	      this_speaker = speaker_list->data;
	      g_free (this_speaker->name);
	      g_free (this_speaker);
	      this_channel->speakers =
		g_list_delete_link (this_channel->speakers, speaker_list);
	      speaker_list = next_speaker;
	    }
	  g_free (this_channel);
	  sound_effect->channels =
	    g_list_delete_link (sound_effect->channels, channel_list);
	  channel_list = next_channel;
	}
      g_free (sound_effect);
      sounds_data->sounds_list =
        g_list_delete_link (sounds_data->sounds_list, sound_effect_list);
      sound_effect_list = next_sound_effect;
    }

  g_free (sounds_data->speaker_abbreviations);
  sounds_data->speaker_abbreviations = NULL;
  g_free (sounds_data);
  sounds_data = NULL;
  return;
}

/* Go through all the speakers, including those specified implicitly,
 * to determine the channel masks and the mapping from sound channel
 * to output channel.  */
static void
sound_process_speakers (struct sounds_info *sounds_data,
			GApplication *app)
{
  GList *sounds_list, *channels_list, *speakers_list;
  struct sound_info *sound_effect;
  struct channel_info *this_channel;
  struct speaker_info *this_speaker;
  GHashTable *hash_table;
  gint i;
  gpointer *p;
  guint64 channel_mask;
  guint64 speaker_bit;
  gint unique_speakers;
  gint max_channels;
  gchar *speaker_name;
  gint speaker_count;
  gint max_speaker_code;
  gint out_chan;
  gint local_speaker_count;
  gint *map_table;
  gpointer *speaker_abbreviations;
  gchar *speaker_abbreviation;
  
  /* a hash table containing the valid speaker names */
  enum speaker_codes
  {speaker_front_left = 0, speaker_front_right = 1,
   speaker_front_center = 2, speaker_LFE1 = 3,
   speaker_rear_left = 4, speaker_rear_right = 5,
   speaker_front_left_of_center = 6, speaker_front_right_of_center = 7,
   speaker_rear_center = 8, speaker_LFE2 = 9,
   speaker_side_left = 10, speaker_side_right = 11,
   speaker_top_front_left = 12, speaker_top_front_right = 13,
   speaker_top_front_center = 14, speaker_top_center = 15,
   speaker_top_rear_left = 16, speaker_top_rear_right = 17,
   speaker_top_side_left = 18, speaker_top_side_right = 19,
   speaker_top_rear_center = 20, speaker_bottom_front_center = 21,
   speaker_bottom_front_left = 22, speaker_bottom_front_right = 23,
   speaker_wide_left = 24, speaker_wide_right = 25,
   speaker_surround_left = 26, speaker_surround_right = 27,
   speaker_all = -1, speaker_none = -2  };
  
  enum speaker_codes speaker_code;

  static enum speaker_codes speaker_values[] =
    { speaker_front_left, speaker_front_right,
      speaker_front_center, speaker_LFE1,
      speaker_rear_left, speaker_rear_right,
      speaker_front_left_of_center, speaker_front_right_of_center,
      speaker_rear_center, speaker_LFE2,
      speaker_side_left, speaker_side_right,
      speaker_top_front_left, speaker_top_front_right,
      speaker_top_front_center, speaker_top_center,
      speaker_top_rear_left, speaker_top_rear_right,
      speaker_top_side_left, speaker_top_side_right,
      speaker_top_rear_center, speaker_bottom_front_center,
      speaker_bottom_front_left, speaker_bottom_front_right,
      speaker_wide_left, speaker_wide_right,
      speaker_surround_left, speaker_surround_right,
      speaker_all, speaker_none
    };
  
  struct speaker_value_pairs
  {
    gpointer key;
    gpointer val;
  };

  static struct speaker_value_pairs speakers_with_values[] =
    {
     {"front_left", &speaker_values[0]},
     {"front_right", &speaker_values[1]},
     {"front_center", &speaker_values[2]},
     {"LFE1", &speaker_values[3]},
     {"rear_left", &speaker_values[4]},
     {"rear_right", &speaker_values[5]},
     {"front_left_of_center", &speaker_values[6]},
     {"front_right_of_center", &speaker_values[7]},
     {"rear_center", &speaker_values[8]},
     {"LFE2", &speaker_values[9]},
     {"side_left", &speaker_values[10]},
     {"side_right", &speaker_values[11]},
     {"top_front_left", &speaker_values[12]},
     {"top_front_right", &speaker_values[13]},
     {"top_front_center", &speaker_values[14]},
     {"top_center", &speaker_values[15]},
     {"top_rear_left", &speaker_values[16]},
     {"top_rear_right", &speaker_values[17]},
     {"top_side_left", &speaker_values[18]},
     {"top_side_right", &speaker_values[19]},
     {"top_rear_center", &speaker_values[20]},
     {"bottom_front_center", &speaker_values[21]},
     {"bottom_front_left", &speaker_values[22]},
     {"bottom_front_right", &speaker_values[23]},
     {"wide_left", &speaker_values[24]},
     {"wide_right", &speaker_values[25]},
     {"surround_left", &speaker_values[26]},
     {"surround_right", &speaker_values[27]},
     {"all", &speaker_values[28]},
     {"none", &speaker_values[29]}
    };

  speaker_count = sep_get_speaker_count (app);
  if (TRACE_SOUND)
    {
      g_print ("Process speakers; speaker count = %d.\n", speaker_count);
    }

  /* Allocate the hash table.  */
  hash_table = g_hash_table_new (g_str_hash, g_str_equal);

  /* Populate the hash table.  */
  for (i = 0;
       i < sizeof (speakers_with_values) / sizeof (speakers_with_values[0]);
       i++)
    {
      g_hash_table_insert (hash_table, speakers_with_values[i].key,
			   speakers_with_values[i].val);
    }

  /* For each sound channel that does not have speakers assigned,
   * assign them the default speakers.  The default speaker layout
   * for a sound is based on the number of channels.  */

  sounds_list = sounds_data->sounds_list;
  while (sounds_list != NULL)
    {
      sound_effect = sounds_list->data;
      if ((!sound_effect->disabled) && (sound_effect->channels == NULL))
	{
	  /* The sound has no speakers assigned.  */
	  switch (sound_effect->channel_count) {

	  case 1:
	    /* By default, a mono sound is sent to all channels.  */
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 0;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "all");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    break;

	  case 2:
	    /* By default, a stereo sound is sent to front_left and front_right.
	     */
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 0;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 1;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    break;

	  case 3:
	    /* By default, adding a third channel adds an LFE speaker.  */
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 0;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 1;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 2;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "LFE1");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    break;
	    
	  case 4:
	    /* By default, quadrophonic sound is two front and two rear
	     * speakers.  */
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 0;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 1;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 2;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 3;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    break;
	    
	  case 5:
	    /* By default, 5 channels adds front center.  */
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 0;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 1;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 2;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 3;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 4;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_center");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    break;

	  case 6:
	    /* By default, six channels is assumed to be 5.1, so add an LFE.  */
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 0;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 1;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 2;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 3;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 4;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_center");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 5;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "LFE1");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    break;

	  case 7:
	    /* By default, 7 channels adds a reer center to the above.  */
	    this_channel->number = 0;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 1;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 2;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 3;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 4;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_center");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 5;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "LFE1");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 6;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_center");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    break;

	  case 8:
	    /* By default, 8 channels is assumed to be 7.1, so add side left
	     * and side right speakers to the 5.1 configuration.  */
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 0;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 1;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 2;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 3;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "rear_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 4;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "front_center");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 5;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "LFE1");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;
	    
	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 6;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "side_left");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    this_channel = g_malloc (sizeof (struct channel_info));
	    this_channel->number = 7;
	    this_channel->speakers = NULL;

	    this_speaker = g_malloc (sizeof (struct speaker_info));
	    this_speaker->volume_level = 1.0;
	    this_speaker->name = g_strdup ((gchar *) "side_right");
	    this_channel->speakers = g_list_prepend (this_channel->speakers,
						     this_speaker);
	    this_speaker = NULL;
	    sound_effect->channels = g_list_prepend (sound_effect->channels,
						     this_channel);
	    this_channel = NULL;

	    break;
	    
	  default:
	    /* By default, more than eight channels just routes the channels
	     * to speakers on a 1:1 basis.  */
	    for (i = 0; i < sound_effect->channel_count; i++)
	      {
		this_channel = g_malloc (sizeof (struct channel_info));
		this_channel->number = i;
		this_channel->speakers = NULL;

		this_speaker = g_malloc (sizeof (struct speaker_info));
		this_speaker->volume_level = 1.0;
		this_speaker->name = g_strdup_printf ("%d", i);
		this_channel->speakers = g_list_prepend (this_channel->speakers,
							 this_speaker);
		this_speaker = NULL;
		sound_effect->channels = g_list_prepend (sound_effect->channels,
							 this_channel);
		this_channel = NULL;

	      }
	    break;
	  }
	}

      sounds_list = sounds_list->next;
    }
    
  /* Find all of the speakers.  
   * For each speaker, set its bit in the channel_mask.
   * Also, count the number of different speakers,
   * the maximum number of channels in a sound
   * and the maximum speaker code.
   */
  channel_mask = 0;
  unique_speakers = 0;
  max_channels = 0;
  max_speaker_code = 0;
  
  sounds_list = sounds_data->sounds_list;
  while (sounds_list != NULL)
    {
      sound_effect = sounds_list->data;
      if (!sound_effect->disabled)
	{
	  if (TRACE_SOUND)
	    {
	      g_print ("Sound %s.\n", sound_effect->name);
	    }
	  if (max_channels < sound_effect->channel_count)
	    max_channels = sound_effect->channel_count;

	  /* The channel mask for a particular sound effect is based on
	   * the number of output channels.  They will be re-routed
	   * to the correct speakers (possibly more speakers than
	   * output channels) just before being mixed with the
	   * other sound effects.  */
	  switch (sound_effect->channel_count)
	    {
	    case 1:
	      sound_effect->channel_mask = 1U << speaker_front_center;
	      break;

	    case 2:
	      sound_effect->channel_mask =
		(1U << speaker_front_left | 1U << speaker_front_right);
	      break;

	    case 3:
	      sound_effect->channel_mask =
		(1U << speaker_front_left | 1U << speaker_front_right |
		 1U << speaker_LFE1);
	      break;
    
	    case 4:
	      sound_effect->channel_mask =
		(1U << speaker_front_left | 1U << speaker_front_right |
		 1U << speaker_rear_left | 1U << speaker_rear_right);
	      break;
    
	    case 5:
	      sound_effect->channel_mask =
		(1U << speaker_front_left | 1U << speaker_front_right |
		 1U << speaker_rear_left | 1U << speaker_rear_right |
		 1U << speaker_front_center);
	      break;
    
	    case 6:
	      sound_effect->channel_mask =
		(1U << speaker_front_left | 1U << speaker_front_right |
		 1U << speaker_rear_left | 1U << speaker_rear_right |
		 1U << speaker_front_center | 1U << speaker_LFE1);
	      break;
    
	    case 7:
	      sound_effect->channel_mask =
		(1U << speaker_front_left | 1U << speaker_front_right |
		 1U << speaker_rear_left | 1U << speaker_rear_right |
		 1U << speaker_front_center | 1U << speaker_LFE1 |
		 1U << speaker_rear_center);
	      break;
	      
	    case 8:
	      sound_effect->channel_mask =
		(1U << speaker_front_left | 1U << speaker_front_right |
		 1U << speaker_rear_left | 1U << speaker_rear_right |
		 1U << speaker_front_center | 1U << speaker_LFE1 |
		 1U << speaker_side_left | 1U << speaker_side_right);
	      break;
    
	    default:
	      sound_effect->channel_mask = 0;
	      for (i = 0; i < sound_effect->channel_count; i++)
		{
		  sound_effect->channel_mask =
		    sound_effect->channel_mask | 1U << i;
		}
	      break;	      
	    }
	  channels_list = sound_effect->channels;
	  while (channels_list != NULL)
	    {
	      this_channel = channels_list->data;
	      if (TRACE_SOUND)
		{
		  g_print ("Channel %d.\n", this_channel->number);
		}
	      speakers_list = this_channel->speakers;
	      while (speakers_list != NULL)
		{
		  this_speaker = speakers_list->data;
		  speaker_name = this_speaker->name;
		  /* If the name starts with a digit, consider it a number.  */
		  if (g_ascii_isdigit(speaker_name[0]))
		    {
		      speaker_code = g_ascii_strtoll (speaker_name,
						      NULL, 10);
		      if ((speaker_code < 0) || (speaker_code > 63))
			{
			  g_printerr ("Speaker name %d is invalid.\n",
				      speaker_code);
			}
		      else
			{
			  this_speaker->speaker_code = speaker_code;
			  if (speaker_code > max_speaker_code)
			    max_speaker_code = speaker_code;
			  speaker_bit = 1U << speaker_code;
			  if ((channel_mask & speaker_bit) == 0)
			    {
			      unique_speakers = unique_speakers + 1;
			      channel_mask = channel_mask | speaker_bit;
			    }
			}
		    }
		  else
		    /* Otherwise consider it a name.  */
		    {
		      p = g_hash_table_lookup (hash_table, this_speaker->name);
		      if (p != NULL)
			{
			  speaker_code = (enum speaker_codes) *p;
			  if (speaker_code > max_speaker_code)
			    max_speaker_code = speaker_code;
			  this_speaker->speaker_code = speaker_code;
			  if ((speaker_code != speaker_all) &&
			      (speaker_code != speaker_none))
			    {
			      speaker_bit = 1U << speaker_code;
			      if ((channel_mask & speaker_bit) == 0)
				{
				  unique_speakers = unique_speakers + 1;
				  channel_mask = channel_mask | speaker_bit;
				}
			    }
			}
		      else
			{
			  g_printerr ("Speaker name %s is invalid.\n",
				      this_speaker->name);
			}
		    }
		  speakers_list = speakers_list->next;
		}
	      channels_list = channels_list->next;
	    }
	  if (TRACE_SOUND)
	    {
	      g_print ("Channel mask = %016lx.\n", sound_effect->channel_mask);
	    }
	}
      sounds_list = sounds_list->next;
    }

  if (TRACE_SOUND)
    {
      g_print ("Overall channel mask 1 = %016lx.\n", channel_mask);
    }

  /* Add more bits to the channel mask based on the speaker names
   * implied by the number of speakers specified by the sound designer.  */

  if (TRACE_SOUND)
    {
      g_print ("Speaker count = %d; unique speakers = %d, max channels = %d,"
	       " max speaker code = %d.\n",
	       speaker_count, unique_speakers, max_channels, max_speaker_code);
    }

  /* If the sound designer specified a number of speakers greater than
   * the number seen in the descriptions of the sounds, add the additional
   * speakers.  Even though they will not be used, they are probably
   * wired to the mixer and so have an output channel.  */
  if ((speaker_count > max_channels) && (speaker_count > unique_speakers))
    {
  
      switch (speaker_count) {

      case 1:
	/* When there is just one speaker all outputs will go there.
	 * If we don't have any speakers yet, Call it front_center.  */
	if (channel_mask == 0)
	  channel_mask = 1U << speaker_front_center;
	break;
    
      case 2:
	/* Stereo implies front_left and front_right.  */
	channel_mask = (channel_mask | 1U << speaker_front_left |
			1U << speaker_front_right);
	break;
    
      case 3:
	/* Three speakers is probably 2.1, meaning add an LFE channel.  */
	channel_mask = (channel_mask | 1U << speaker_front_left |
			1U << speaker_front_right | 1U << speaker_LFE1);
	break;
    
      case 4:
	/* Four speakers are front and read left and right.  */
	channel_mask = (channel_mask | 1U << speaker_front_left |
			1U << speaker_front_right | 1U << speaker_rear_left |
			1U << speaker_rear_right);
	break;
    
      case 5:
	/* Add front_center.  */
	channel_mask = (channel_mask | 1U << speaker_front_left |
			1U << speaker_front_right | 1U << speaker_rear_left |
			1U << speaker_rear_right | 1U << speaker_front_center);
	break;
    
      case 6:
	/* This is a probably a 5.1 surround sound system.  
	 * Add an LFE channel.  */
	channel_mask = (channel_mask | 1U << speaker_front_left |
			1U << speaker_front_right | 1U << speaker_rear_left |
			1U << speaker_rear_right | 1U << speaker_front_center |
			1U << speaker_LFE1);
	break;
    
      case 7:
	/* Add rear center.  */
	channel_mask = (channel_mask | 1U << speaker_front_left |
			1U << speaker_front_right | 1U << speaker_rear_left |
			1U << speaker_rear_right | 1U << speaker_front_center |
			1U << speaker_LFE1 | 1U << speaker_rear_center);
	break;
	
      case 8:
	/* This is probably a 7.1 surround system.  */
	channel_mask = (channel_mask | 1U << speaker_front_left |
			1U << speaker_front_right | 1U << speaker_rear_left |
			1U << speaker_rear_right | 1U << speaker_front_center |
			1U << speaker_LFE1 | 1U << speaker_side_left |
			1U << speaker_side_right);
	break;
    
      default:
	/* If we have more than eight speakers the output channels are mapped
	 * 1-to-1 onto speakers.  */
	for (i = 0; i < speaker_count; i++)
	  {
	    channel_mask = channel_mask | 1U << i;
	  }
	break;
    
      }
    }

  if (TRACE_SOUND)
    {
      g_print ("Overall channel mask 2 = %016lx.\n", channel_mask);
    }

  /* We will use this global channel mask to route after the sound effects
   * are mixed together.  */
  sounds_data->channel_mask = channel_mask;

  /* We need to set the speaker count high enough that every speaker
   * has a channel.  */
  local_speaker_count = 0;
  for (i = 0; i <= max_speaker_code; i++)
    {
      speaker_bit = 1U << i;
      if ((channel_mask & speaker_bit) != 0)
	local_speaker_count = local_speaker_count + 1;
    }
  if (TRACE_SOUND)
    {
      g_print ("Local speaker count = %d.\n", local_speaker_count);
    }
  if (local_speaker_count > speaker_count)
    speaker_count = local_speaker_count;
  
  if (TRACE_SOUND)
    {
      g_print ("Speaker count = %d.\n", speaker_count);
    }
  sep_set_speaker_count (speaker_count, app);
    
  /* Now that we have the global channel mask, we can create a map from
   * speaker number to output channel.  We will use this when
   * constructing the mix-matrix for the "final" bin.  Also, create
   * the reverse map, from output channel to an abbreviated speaker
   * name, for labeling the channels on the VU meter.  */
  
  out_chan = 0;
  map_table = g_malloc ((max_speaker_code + 1) * (sizeof (gint)));
  speaker_abbreviations = g_malloc (speaker_count * (sizeof (gpointer)));
  for (i=0; i < speaker_count; i++)
    speaker_abbreviations[i] = NULL;
				    
  for (i=0; i <= max_speaker_code; i++)
    {
      speaker_bit = 1U << i;
      if ((channel_mask & speaker_bit) != 0)
	{
	  map_table[i] = out_chan;
	  if (out_chan < speaker_count)
	    {
	      switch (i) {
	      case speaker_front_left:
		speaker_abbreviations[out_chan] = "FL  ";
		break;
	      case speaker_front_right:
		speaker_abbreviations[out_chan] = "FR  ";
		break;
	      case speaker_front_center:
		speaker_abbreviations[out_chan] = "FC  ";
		break;
	      case speaker_LFE1:
		speaker_abbreviations[out_chan] = "LFE1";
		break;
	      case speaker_rear_left:
		speaker_abbreviations[out_chan] = "RL  ";
		break;
	      case speaker_rear_right:
		speaker_abbreviations[out_chan] = "RR  ";
		break;
	      case speaker_front_left_of_center:
		speaker_abbreviations[out_chan] = "FLC ";
		break;
	      case speaker_front_right_of_center:
		speaker_abbreviations[out_chan] = "FRC ";
		break;
	      case speaker_rear_center:
		speaker_abbreviations[out_chan] = "RC  ";
		break;
	      case speaker_LFE2:
		speaker_abbreviations[out_chan] = "LFE2";
		break;
	      case speaker_side_left:
		speaker_abbreviations[out_chan] = "SL  ";
		break;
	      case speaker_side_right:
		speaker_abbreviations[out_chan] = "SR  ";
		break;
	      case speaker_top_front_left:
		speaker_abbreviations[out_chan] = "TFL ";
		break;
	      case speaker_top_front_right:
		speaker_abbreviations[out_chan] = "TFR ";
		break;
	      case speaker_top_front_center:
		speaker_abbreviations[out_chan] = "TFC ";
		break;
	      case speaker_top_center:
		speaker_abbreviations[out_chan] = "TC  ";
		break;
	      case speaker_top_rear_left:
		speaker_abbreviations[out_chan] = "TRL ";
		break;
	      case speaker_top_rear_right:
		speaker_abbreviations[out_chan] = "TRR ";
		break;
	      case speaker_top_side_left:
		speaker_abbreviations[out_chan] = "TSL ";
		break;
	      case speaker_top_side_right:
		speaker_abbreviations[out_chan] = "TSR ";
		break;
	      case speaker_top_rear_center:
		speaker_abbreviations[out_chan] = "TRC ";
		break;
	      case speaker_bottom_front_center:
		speaker_abbreviations[out_chan] = "BFC ";
		break;
	      case speaker_bottom_front_left:
		speaker_abbreviations[out_chan] = "BFL ";
		break;
	      case speaker_bottom_front_right:
		speaker_abbreviations[out_chan] = "BFR ";
		break;
	      case speaker_wide_left:
		speaker_abbreviations[out_chan] = "WL  ";
		break;
	      case speaker_wide_right:
		speaker_abbreviations[out_chan] = "WR  ";
		break;
	      case speaker_surround_left:
		speaker_abbreviations[out_chan] = "SL  ";
		break;
	      case speaker_surround_right:
		speaker_abbreviations[out_chan] = "SR  ";
		break;
	      default:
		speaker_abbreviations[out_chan] = NULL;
	      }
	    }
	  out_chan = out_chan + 1;
	}
    }

  sounds_data->speaker_abbreviations = speaker_abbreviations;
  sounds_data->speaker_count = speaker_count;
  
  if (TRACE_SOUND)
    {
      printf ("Output channel to speaker correspondence:\n");
      for (i=0; i < speaker_count; i++)
	{
	  speaker_abbreviation = speaker_abbreviations[i];
	  printf ("%d: %s.\n", i, speaker_abbreviation);
	}
    } 
  
  /* Tell each reference to a speaker which "final" output channel it is on.
   * This data structure is used in sound_mix_matrix_volume to construct
   * the mix matrix for the audioconvert element which feeds the
   * "final" bin.  */
  sounds_list = sounds_data->sounds_list;
  while (sounds_list != NULL)
    {
      sound_effect = sounds_list->data;
      if (!sound_effect->disabled)
	{
	  if (TRACE_SOUND)
	    {
	      g_print ("Sound %s.\n", sound_effect->name);
	    }
	  
	  channels_list = sound_effect->channels;
	  while (channels_list != NULL)
	    {
	      this_channel = channels_list->data;
	      if (TRACE_SOUND)
		{
		  g_print ("Channel %d.\n", this_channel->number);
		}
	      speakers_list = this_channel->speakers;
	      while (speakers_list != NULL)
		{
		  this_speaker = speakers_list->data;
		  speaker_code = this_speaker->speaker_code;
		  if (speaker_code >= 0)
		    {
		      this_speaker->output_channel = map_table[speaker_code];
		      if (TRACE_SOUND)
			{
			  g_print ("Speaker %s, code %d, channel %d.\n",
				   this_speaker->name,
				   this_speaker->speaker_code,
				   this_speaker->output_channel);
			}
		    }
		  speakers_list = speakers_list->next;
		}
	      channels_list = channels_list->next;
	    }
	}
      sounds_list = sounds_list->next;
    }

  g_free (map_table);  
  g_hash_table_destroy (hash_table);
  
  return;
}

/* Start the sound system.  We have already read an XML file
 * containing sound definitions and put the results in the sound list.  */
GstPipeline *
sound_start (GApplication *app)
{
  GstPipeline *pipeline_element;
  GstBin *bin_element;
  GList *sound_list;
  gint sound_number, sound_count;
  GList *l;
  struct sound_info *sound_data;
  struct sounds_info *sounds_data;
  gint success;

  sounds_data = sep_get_sounds_data (app);
  sound_list = sounds_data->sounds_list;

  /* Handle the speakers.  */
  sound_process_speakers (sounds_data, app);

  /* Count the non-disabled sounds.  */
  sound_count = 0;
  for (l = sound_list; l != NULL; l = l->next)
    {
      sound_data = l->data;
      if (!sound_data->disabled)
	{
	  sound_count = sound_count + 1;
	}
    }

  /* If we have any sounds, create the gstreamer pipeline and place the 
   * sound effects bins in it.  */
  if (sound_count == 0)
    {
      return NULL;
    }

  pipeline_element = gstreamer_init (sound_count, app);
  if (pipeline_element == NULL)
    {
      /* We are unable to create the gstreamer pipeline.  */
      return pipeline_element;
    }

  /* Create a gstreamer bin for each enabled sound effect and place it in
   * the gstreamer pipeline.  */
  sound_number = 0;
  for (l = sound_list; l != NULL; l = l->next)
    {
      sound_data = l->data;
      if (!sound_data->disabled)
        {
          bin_element =
            gstreamer_create_bin (sound_data, sound_number, pipeline_element,
                                  app);

          if (bin_element == NULL)
            {
              /* We are unable to create the gstreamer bin.  This might
               * be because an element is unavailable.  */
              sound_data->disabled = TRUE;
              sound_count = sound_count - 1;
              continue;
            }

          sound_data->sound_control = bin_element;
          sound_number = sound_number + 1;
        }
    }

  /* If we have any sound effects, complete the gstreamer pipeline.  */
  if (sound_count > 0)
    {
      success = gstreamer_complete_pipeline (pipeline_element, app);
      if (success == 0)
        {
          pipeline_element = NULL;
        }
    }
  else
    {
      /* Since we have no sound effects, we don't need a pipeline.  */
      g_object_unref (pipeline_element);
      pipeline_element = NULL;
    }

  return pipeline_element;
}

/* Set the name displayed in a cluster.  */
void
sound_cluster_set_name (gchar *sound_name, guint cluster_number,
                        GApplication *app)
{
  GList *children_list;
  const char *child_name;
  GtkLabel *title_label;
  GtkWidget *cluster;

  /* find the cluster */
  cluster = sep_get_cluster_from_number (cluster_number, app);

  if (cluster == NULL)
    return;

  /* Set the name in the cluster.  */
  title_label = NULL;
  children_list = gtk_container_get_children (GTK_CONTAINER (cluster));
  while (children_list != NULL)
    {
      child_name = gtk_widget_get_name (children_list->data);
      if (g_strcmp0 (child_name, "title") == 0)
        {
          title_label = children_list->data;
          break;
        }
      children_list = children_list->next;
    }
  g_list_free (children_list);

  if (title_label != NULL)
    {
      gtk_label_set_label (title_label, sound_name);
    }
}

/* Append a sound to the list of sounds. */
void
sound_append_sound (struct sound_info *sound_effect, GApplication *app)
{
  struct sounds_info *sounds_data;

  sounds_data = sep_get_sounds_data (app);
  sounds_data->sounds_list = g_list_append (sounds_data->sounds_list,
					    sound_effect);
  return;
}

/* Associate a sound with a specified cluster.  */
struct sound_info *
sound_bind_to_cluster (gchar *sound_name, guint cluster_number,
                       GApplication *app)
{
  struct sounds_info *sounds_data;
  GList *sound_effect_list;
  GtkWidget *cluster_widget;
  struct sound_info *sound_effect;
  gboolean sound_effect_found;

  sounds_data = sep_get_sounds_data (app);
  sound_effect_list = sounds_data->sounds_list;
  
  sound_effect_found = FALSE;
  while (sound_effect_list != NULL)
    {
      sound_effect = sound_effect_list->data;
      if ((g_strcmp0 (sound_name, sound_effect->name)) == 0)
        {
          sound_effect_found = TRUE;
          break;
        }
      sound_effect_list = sound_effect_list->next;
    }

  if (!sound_effect_found)
    return NULL;

  cluster_widget = sep_get_cluster_from_number (cluster_number, app);
  sound_effect->cluster_number = cluster_number;
  sound_effect->cluster_widget = cluster_widget;

  return sound_effect;

}

/* Disassociate a sound from its cluster.  */
void
sound_unbind_from_cluster (struct sound_info *sound_effect,
                           GApplication *app)
{
  sound_effect->cluster_number = -1;
  sound_effect->cluster_widget = NULL;

  return;
}

/* Given a cluster number, find the corresponding sound effect.  */
struct sound_info *
sound_get_sound_effect_from_widget (GtkWidget *cluster_widget,
				    GApplication *app)
{
  struct sounds_info *sounds_data;
  GList *sounds_list;
  struct sound_info *sound_effect;
  gboolean sound_effect_found = FALSE;
  
  sounds_data = sep_get_sounds_data (app);
  sounds_list = sounds_data->sounds_list;
  
  /* Find the sound effect attached to the specified widget.  */
  while (sounds_list != NULL)
    {
      sound_effect = sounds_list->data;
      if (sound_effect->cluster_widget == cluster_widget)
	{
	  sound_effect_found = TRUE;
	  break;
	}
      sounds_list = sounds_list->next;
    }
  
  if (sound_effect_found)
    return (sound_effect);
  
  return NULL;
}

/* Start playing a sound effect.  */
void
sound_start_playing (struct sound_info *sound_data, GApplication *app)
{
  GstBin *bin_element;
  GstEvent *event;
  GstStructure *structure;

  bin_element = sound_data->sound_control;
  if (bin_element == NULL)
    return;

  /* If the sound has already been started, and is not yet releasing, 
   * don't try to start it again.  A sound is releasing if we have sent
   * a release message or if it has entered its release stage on its own.  */
  if (sound_data->running && !sound_data->release_sent
      && !sound_data->release_has_started)
    {
      return;
    }

  /* Send a start message to the bin.  It will be routed to the source, and
   * flow from there downstream through the looper and envelope.  
   * The looper element will start sending its local buffer
   * and the envelope element will start to shape the volume.  */
  sound_data->running = TRUE;
  sound_data->starting_time = g_get_monotonic_time () * 1e3;
  sound_data->release_sent = FALSE;
  sound_data->release_has_started = FALSE;
  structure = gst_structure_new_empty ((gchar *) "start");
  event = gst_event_new_custom (GST_EVENT_CUSTOM_UPSTREAM, structure);
  gst_element_send_event (GST_ELEMENT (bin_element), event);

  return;
}

/* Stop playing a sound effect.  */
void
sound_stop_playing (struct sound_info *sound_data, GApplication *app)
{
  GstBin *bin_element;
  GstEvent *event;
  GstStructure *structure;

  bin_element = sound_data->sound_control;

  /* Send a release message to the bin.  The looper element will stop
   * looping, and the envelope element will start shutting down the sound.
   * If the sound has a non-zero release time we should get a call to 
   * release_started shortly, unless the sound has already completed
   * and the message is still on its way down the pipeline.  */
  structure = gst_structure_new_empty ((gchar *) "release");
  event = gst_event_new_custom (GST_EVENT_CUSTOM_UPSTREAM, structure);
  gst_element_send_event (GST_ELEMENT (bin_element), event);

  sound_data->release_sent = TRUE;

  return;
}

/* Get the elapsed time of a playing sound.  */
guint64
sound_get_elapsed_time (struct sound_info * sound_data, GApplication *app)
{
  GstElement *looper_element;
  guint64 elapsed_time;

  looper_element = gstreamer_get_looper (sound_data->sound_control);
  g_object_get (looper_element, (gchar *) "elapsed-time", &elapsed_time,
                NULL);
  return elapsed_time;
}

/* Calculate the amount of time allowed by the envelope.
 * G_MAXUINT64 means no limit.  */
static guint64
calculate_envelope_duration_time (struct sound_info *sound_data,
                                  GApplication *app)
{
  /* If the release duration is infinite, the envelope does not limit
   * the duration of the sound.  */
  if (sound_data->release_duration_infinite)
    return G_MAXUINT64;

  /* Otherwise, if the release is scheduled for a particular time, 
   * the envelope duration time is the release start time plus the 
   * release duration time.  */
  if (sound_data->release_start_time > 0)
    {
      return (sound_data->release_start_time +
              sound_data->release_duration_time);
    }

  /* Otherwise, if the release has started, the envelope time is the 
   * amount of time the sound ran until release, plus the release 
   * duration time.  */
  if (sound_data->release_has_started)
    {
      return (sound_data->releasing_time - sound_data->starting_time +
              sound_data->release_duration_time);
    }

  /* The release has not yet started and is not scheduled to start.
   * We won't know the envelope duration until an external signal
   * triggers the release (and not even then if the release duration is 
   * infinite).  Until the trigger arrives, the envelope does not limit 
   * the amount of time for the sound.  */
  return G_MAXUINT64;
}

/* Get the remaining run time of a playing sound.  
 * G_MAXUINT64 means infinity.  */
guint64
sound_get_remaining_time (struct sound_info * sound_data, GApplication *app)
{
  GstElement *looper_element;
  guint64 looper_remaining_time;
  guint64 envelope_duration_time, envelope_remaining_time, elapsed_time;

  /* Calculate the amount of time left in the looper element.  */
  looper_element = gstreamer_get_looper (sound_data->sound_control);
  g_object_get (looper_element, (gchar *) "remaining-time",
                &looper_remaining_time, NULL);

  /* Calculate how long the envelope will allow the sound to run.  */
  envelope_duration_time = calculate_envelope_duration_time (sound_data, app);

  /* If the envelope doesn't limit the duration of the sound, return
   * the looper's limit, which might also not be limited due to indefinite
   * looping.  */
  if (envelope_duration_time == G_MAXUINT64)
    {
      return looper_remaining_time;
    }

  /* Calculate how long until the envelope ends the sound.  */
  g_object_get (looper_element, (gchar *) "elapsed-time", &elapsed_time,
                NULL);
  envelope_remaining_time = envelope_duration_time - elapsed_time;

  /* Return the limit that will end the sound sooner.  */
  if (envelope_remaining_time < looper_remaining_time)
    return envelope_remaining_time;
  return looper_remaining_time;
}

/* Receive a completed message, which indicates that a sound has finished.  */
void
sound_completed (const gchar * sound_name, GApplication *app)
{
  struct sounds_info *sounds_data;
  GList *sound_effect_list;
  struct sound_info *sound_effect = NULL;
  gboolean sound_effect_found;
  gboolean terminated;
  
  /* Search through the sound effects for the one with this name.  */
  sounds_data = sep_get_sounds_data (app);
  sound_effect_list = sounds_data->sounds_list;
  sound_effect_found = FALSE;
  while (sound_effect_list != NULL)
    {
      sound_effect = sound_effect_list->data;
      if (g_strcmp0 (sound_effect->name, sound_name) == 0)
        {
          sound_effect_found = TRUE;
          break;
        }
      sound_effect_list = sound_effect_list->next;
    }

  /* There isn't one--ignore the completion.  */
  if (!sound_effect_found)
    return;

  /* Flag that the sound is no longer playing.  */
  sound_effect->running = FALSE;

  /* Let the internal sequencer distinguish a sound that has completed
   * normally from one that has been stopped.  */
  terminated = sound_effect->release_sent;
  sound_effect->release_sent = FALSE;
  sequence_sound_completion (sound_effect, terminated, app);
  return;
}

/* Receive a release_started message, which indicates that a sound has entered
 * its release stage.  */
void
sound_release_started (const gchar * sound_name, GApplication *app)
{
  struct sounds_info *sounds_data;
  GList *sound_effect_list;
  struct sound_info *sound_effect = NULL;
  gboolean sound_effect_found;

  /* Search through the sound effects for the one with this name.  */
  sounds_data = sep_get_sounds_data (app);
  sound_effect_list = sounds_data->sounds_list;
  sound_effect_found = FALSE;
  while (sound_effect_list != NULL)
    {
      sound_effect = sound_effect_list->data;
      if (g_strcmp0 (sound_effect->name, sound_name) == 0)
        {
          sound_effect_found = TRUE;
          break;
        }
      sound_effect_list = sound_effect_list->next;
    }

  /* If there isn't one, ignore the termination message.  */
  if (!sound_effect_found)
    return;

  /* Remember that the sound is in its release stage.  */
  sound_effect->releasing_time = g_get_monotonic_time () * 1e3;
  sound_effect->release_has_started = TRUE;

  /* Let the internal sequencer handle it.  */
  sequence_sound_release_started (sound_effect, app);

  return;
}

/* The Pause button was pushed.  */
void
sound_button_pause (GApplication *app)
{
  struct sounds_info *sounds_data;
  GList *sound_list;
  GList *l;
  struct sound_info *sound_data;
  GstBin *bin_element;
  GstEvent *event;
  GstStructure *structure;

  sounds_data = sep_get_sounds_data (app);
  sound_list = sounds_data->sounds_list;

  /* Go through the non-disabled sounds, sending each a pause command.  */
  for (l = sound_list; l != NULL; l = l->next)
    {
      sound_data = l->data;
      if (!sound_data->disabled)
        {
          bin_element = sound_data->sound_control;

          /* Send a pause message to the bin.  The looper element will stop
           * advancing its pointer, sending silence instead, and the envelope
           * element will stop advancing through its timeline.  */
          structure = gst_structure_new_empty ((gchar *) "pause");
          event = gst_event_new_custom (GST_EVENT_CUSTOM_UPSTREAM, structure);
          gst_element_send_event (GST_ELEMENT (bin_element), event);
        }

    }
  return;
}

/* The Continue button was pushed.  */
void
sound_button_continue (GApplication *app)
{
  struct sounds_info *sounds_data;
  GList *sound_list;
  GList *l;
  struct sound_info *sound_data;
  GstBin *bin_element;
  GstEvent *event;
  GstStructure *structure;

  sounds_data = sep_get_sounds_data (app);
  sound_list = sounds_data->sounds_list;

  /* Go through the non-disabled sounds, sending each a continue command.  */
  for (l = sound_list; l != NULL; l = l->next)
    {
      sound_data = l->data;
      if (!sound_data->disabled)
        {
          bin_element = sound_data->sound_control;

          /* Send a continue message to the bin.  The looper element will 
           * return to advancing its pointer, and the envelope element will 
           * return to advancing through its timeline.  */
          structure = gst_structure_new_empty ((gchar *) "continue");
          event = gst_event_new_custom (GST_EVENT_CUSTOM_UPSTREAM, structure);
          gst_element_send_event (GST_ELEMENT (bin_element), event);
        }

    }

  return;
}

/* Count the bit depth and number of sound channels in a WAV file.
 * Return 1 on success, 0 on failure.  */
gint
sound_parse_wav_file_header (const gchar *wav_file_name,
			     struct sound_info *sound_effect,
			     GApplication *app)
{
  FILE *file_stream;
  size_t amount_read;
  gint stream_status;
  gboolean file_open = FALSE;
  gint channel_count;
  gint format_code;
  gchar *format_name;
  gint bits_per_sample;
  gint return_value = 0;
  gchar header [36];
  
  file_stream = fopen (wav_file_name, "rb");
  if (file_stream == NULL)
    {
      g_printerr ("Failed to open file \"%s\": %s.\n",
		  wav_file_name, strerror (errno));
      goto common_exit;
    }
  file_open = TRUE;

  /* Read the first 36 bytes of the file, which will give us everything
   * we need to do some validation and find the information we need.  */
  amount_read = fread (&header, 1, 36, file_stream);
  if (amount_read != 36)
    {
      g_printerr ("Failed to read the first 36 bytes of file \"%s\"; "
		  "got %lu bytes.", wav_file_name, amount_read);
      goto common_exit;
    }

  if (memcmp (&header[0], "RIFF", 4) != 0)
    {
      g_printerr ("File \"%s\" is not a RIFF file.",
		  wav_file_name);
      goto common_exit;
    }

  if (memcmp (&header[8], "WAVE", 4) != 0)
    {
      g_printerr ("File \"%s\" is not a WAVE file.",
		  wav_file_name);
      goto common_exit;
    }

  /* the number of channels is a 2-byte integer at offset 22.  */
  channel_count = header[22] + (header[23] * 256);

  /* To find the format, we need the number of bits per sample
   * at offset 34, and the format code at offset 20.  */
  bits_per_sample = header[34] + (header[35] * 256);
  format_code = header[20] + (header[21] * 256);

  /* Note that whether a sample is signed or unsigned is not indicated
   * in the WAV file format, so the convention is that 8-bit samples
   * are unsigned and all the rest are signed.  */
  format_name = NULL;
  switch (bits_per_sample)
    {
    case 8:
      format_name = "U8";
      break;
    case 16:
      format_name = "S16LE";
      break;
    case 24:
      format_name = "S24LE";
      break;
    case 32:
      if (format_code == 1)
	{
	  format_name = "S32LE";
	  break;
	}
      if (format_code == 3)
	{
	  format_name = "F32LE";
	  break;
	}
      break;
    case 64:
      format_name = "F64LE";
      break;
    default:
      break;
    }

  if (format_name == NULL)
    {
      g_printerr ("Unparsable WAV file: "
		  "format code %d, bits per sample %d, file name %s.\n",
		  format_code, bits_per_sample, wav_file_name);
      goto common_exit;
    }

  sound_effect->format_name = format_name;
  sound_effect->channel_count = channel_count;
  
  if (TRACE_SOUND)
    {
      g_print ("File %s is %s with %d channels.\n",
	       wav_file_name, format_name, channel_count);
      g_print ("Byte 20: %d, 21: %d, 22: %d, 23: %d, 34: %d, 35: %d.\n",
	       header[20], header[21], header[22], header[23],
	       header[34], header[35]);
    }
  return_value = 1;

 common_exit:
  if (file_open)
    {
      stream_status = fclose (file_stream);
      if (stream_status == EOF)
        {
          g_printerr ("Failed to close file \"%s\".",
		      wav_file_name);
	  return_value = 0;
        }
      file_open = FALSE;
    }
  return return_value;
}

/* Add a channel description to a sound.  */
void
sound_append_channel (struct channel_info *channel_data,
		      struct sound_info *sound_data,
		      GApplication *app)
{
  if (TRACE_SOUND)
    {
      g_print ("Channel %d in sound %s.\n",
	       channel_data->number, sound_data->name);
    }
  if (channel_data->number >= sound_data->channel_count)
    {
      g_printerr ("Sound %s has %d channels but specifies more.\n",
		  sound_data->wav_file_name, sound_data->channel_count);
      sound_data->disabled = TRUE;
    }
  sound_data->channels = g_list_prepend (sound_data->channels,
					 channel_data);
  
  return;
}

/* Add a speaker description to a channel.  */
void
sound_append_speaker (struct speaker_info *speaker_data,
		      struct channel_info *channel_data,
		      struct sound_info *sound_data,
		      GApplication *app)
{
  if (TRACE_SOUND)
    {
      g_print ("Speaker %s in channel %d volume %f in sound %s.\n",
	       speaker_data->name, channel_data->number,
	       speaker_data->volume_level, sound_data->name);
    }
  channel_data->speakers = g_list_prepend (channel_data->speakers,
					   speaker_data);
  
  return;
}

/* Find the channel mask.  It has a bit set for each active speaker.  */
guint64
sound_get_channel_mask (GApplication *app)
{
  struct sounds_info *sounds_data;

  sounds_data = sep_get_sounds_data (app);
  return (sounds_data->channel_mask);  
}

/* Determine the volume of sound from the specified input channel
 * to the specified output channel.  This is used to construct the mix matrix
 * that feeds from this sound effect to the "final" gstreamer bin.  */
gfloat
sound_mix_matrix_volume (gint in_chan, gint out_chan,
			 struct sound_info *sound_effect, GApplication *app)
{
  GList *channel_list, *speaker_list;
  struct channel_info *this_channel;
  struct speaker_info *this_speaker;

  channel_list = sound_effect->channels;
  while (channel_list != NULL)
    {
      this_channel = channel_list->data;
      if (this_channel->number == in_chan)
	{
	  speaker_list = this_channel->speakers;
	  while (speaker_list != NULL)
	    {
	      this_speaker = speaker_list->data;
	      /* Speaker all means all output channels, thus all speakers.  */
	      if (g_strcmp0 (this_speaker->name, "all") == 0)
		return (this_speaker->volume_level);
	      /* Speaker none means send the sound nowhere.  */
	      if (g_strcmp0 (this_speaker->name, "none") == 0)
		return ((gfloat) 0.0);
	      
	      if (this_speaker->output_channel == out_chan)
		{
		  if (TRACE_SOUND)
		    {
		      g_print ("sound %s, in %d, out %d: vol %f.\n",
			       sound_effect->name, in_chan, out_chan,
			       this_speaker->volume_level);
		    }
		  return (this_speaker->volume_level);
		}
	      speaker_list = speaker_list->next;
	    }
	}
      channel_list = channel_list->next;
    }

  /* If we don't find the channel, send silence.  */
  return ((gfloat) 0.0);
}

/* Return the abbreviated speaker name for the requested output channel.
 * This is used to label the level bars in the VU meter.  */
gchar *
sound_output_channel_name (gint output_channel, GApplication *app)
{
  struct sounds_info *sounds_data;
  gpointer *speaker_abbreviations;
  gchar *speaker_abbreviation;

  sounds_data = sep_get_sounds_data (app);

  /* If an output channel does not have a speaker assigned to it,
   * the label is blank.  */
  if (output_channel >= sounds_data->speaker_count)
    return ("    ");
  
  speaker_abbreviations = sounds_data->speaker_abbreviations;
  speaker_abbreviation = speaker_abbreviations[output_channel];
  return (speaker_abbreviation);
}

/* End of file sound_subroutines.c */
