/*
 * sequence_subroutines.c
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
#include "sequence_subroutines.h"
#include "sequence_structure.h"
#include "button_subroutines.h"
#include "display_subroutines.h"
#include "sound_effects_player.h"
#include "sound_structure.h"
#include "sound_subroutines.h"
#include "timer_subroutines.h"
#include "trace_subroutines.h"

/* the persistent data used by the internal sequencer */
struct sequence_info
{
  GList *item_list;             /* The sequence  */
  gchar *next_item_name;        /* The name of the next sequence item
                                 * to be executed.  */
  GList *running;               /* The list of Start Sound items
                                 * that are still attached to a cluster  */
  GList *offering;              /* The list of Offer Sound items 
                                 * that are still attached to a cluster  */
  struct remember_info *current_operator_wait;  /* The Operator Wait sequence 
                                                 * item that is currently 
                                                 * displaying its text to the 
                                                 * operator.  */
  GList *operator_waiting;      /* The Operator Wait sequence items that are
                                 * waiting for their turn at the operator.  */
  GList *waiting;               /* The Wait sequence items that are still 
                                 * pending.  */
  gboolean message_displaying;  /* TRUE if the sequencer is displaying a
                                 * message to the operator.  */
  guint message_id;             /* The ID of the message being displayed by the
                                 * sequencer.  */
};

/* an entry on the running, offering or operator waiting lists */
struct remember_info
{
  guint cluster_number;
  struct sound_info *sound_effect;
  struct sequence_item_info *sequence_item;
  gboolean active;
  gboolean being_displayed;
  gboolean release_sent;
  gboolean release_seen;
  gboolean off_cluster;
  gboolean stopped_by_operator;
};

/* Forward declarations, so I can call these subroutines before I define them.  
 */

static struct sequence_item_info *find_item_by_name (gchar * item_name,
                                                     struct sequence_info
                                                     *sequence_data,
                                                     GApplication *app);

static void execute_items (struct sequence_info *sequence_data,
                           GApplication *app);

static void execute_item (struct sequence_item_info *the_item,
                          struct sequence_info *sequence_data,
                          GApplication *app);

static void execute_start_sound (struct sequence_item_info *the_item,
                                 struct sequence_info *sequence_data,
                                 GApplication *app);

static void execute_stop_sound (struct sequence_item_info *the_item,
                                struct sequence_info *sequence_data,
                                GApplication *app);

static void execute_wait (struct sequence_item_info *the_item,
                          struct sequence_info *sequence_data,
                          GApplication *app);

static void wait_completed (void *remember_data, GApplication *app);

static void execute_offer_sound (struct sequence_item_info *the_item,
                                 struct sequence_info *sequence_data,
                                 GApplication *app);

static void execute_cease_offering_sound (struct sequence_item_info *the_item,
                                          struct sequence_info *sequence_data,
                                          GApplication *app);

static void execute_operator_wait (struct sequence_item_info *the_item,
                                   struct sequence_info *sequence_data,
                                   GApplication *app);

static void execute_cancel_wait (struct sequence_item_info *the_item,
                                 struct sequence_info *sequence_data,
                                 GApplication *app);

static void update_operator_display (struct sequence_info *sequence_data,
                                     GApplication *app);
static void clock_tick (void *sequence_data, GApplication *app);

/* Subroutines for handling sequence items.  */

/* Initialize the internal sequencer.  */
void *
sequence_init (GApplication *app)
{
  struct sequence_info *sequence_data;

  sequence_data = g_malloc (sizeof (struct sequence_info));
  sequence_data->item_list = NULL;
  sequence_data->offering = NULL;
  sequence_data->running = NULL;
  sequence_data->current_operator_wait = NULL;
  sequence_data->operator_waiting = NULL;
  sequence_data->waiting = NULL;
  sequence_data->message_displaying = FALSE;
  sequence_data->message_id = 0;
  return (sequence_data);
}

/* Append a sequence item to the sequence. */
void
sequence_append_item (struct sequence_item_info *item, GApplication *app)
{
  struct sequence_info *sequence_data;

  sequence_data = sep_get_sequence_data (app);
  sequence_data->item_list = g_list_append (sequence_data->item_list, item);
  return;
}

/* Start running the sequencer.  */
void
sequence_start (GApplication *app)
{
  struct sequence_info *sequence_data;
  GList *item_list;
  struct sequence_item_info *item, *start_item;
  sequence_data = sep_get_sequence_data (app);

  /* Find the Start Sequence item in the sequence.  */
  start_item = NULL;
  for (item_list = sequence_data->item_list; item_list != NULL;
       item_list = item_list->next)
    {
      item = item_list->data;
      if (item->type == start_sequence)
        {
          start_item = item;
          break;
        }
    }
  if (start_item == NULL)
    {
      display_show_message ("No Start Sequence item.", app);
      return;
    }

  /* We have a sequence which contains a Start Sequence item.  Proceed to
   * the specified next item.  */
  if (start_item->next == NULL)
    {
      display_show_message ("Sequence Start has no next item.", app);
      return;
    }

  if (trace_sequencer_level (app) > 0)
    {
      trace_sequencer_write ("sequencer started.", app);
    }
  sequence_data->next_item_name = start_item->next;

  /* Run sequence items starting at next_item_name.  */
  execute_items (sequence_data, app);

  return;
}

/* Execute the named next item, and continue execution until we must
 * wait for something or we run out of items to execute.  */
static void
execute_items (struct sequence_info *sequence_data, GApplication *app)
{
  struct sequence_item_info *next_item;
  gchar *display_text;
  gchar *trace_text;

  while (sequence_data->next_item_name != NULL)
    {
      next_item =
        find_item_by_name (sequence_data->next_item_name, sequence_data, app);

      if (next_item == NULL)
        {
          display_text =
            g_strdup_printf ("Next item, %s, not found.",
                             sequence_data->next_item_name);
          display_show_message (display_text, app);
	  if (trace_sequencer_level (app) > 0)
	    {
	      trace_sequencer_write (display_text, app);
	    }
          g_free (display_text);
          display_text = NULL;
          break;
        }

      sequence_data->next_item_name = NULL;
      execute_item (next_item, sequence_data, app);
    }

  /* There are no more items to execute.  If nothing is waiting to
   * execute later, we are done.  */
  if ((sequence_data->offering == NULL) && (sequence_data->running == NULL)
      && (sequence_data->current_operator_wait == NULL)
      && (sequence_data->operator_waiting == NULL)
      && (sequence_data->waiting == NULL))
    {
      /* Signal the application to quit.  This first shuts down the
       * Gstreamer pipeline, then exits.  */
      if (trace_sequencer_level (app) > 0)
	{
	  trace_text = g_strdup_printf ("Nothing left to do; exiting.");
	  trace_sequencer_write (trace_text, app);
	  g_free (trace_text);
	  trace_text = NULL;
	}

      g_application_quit (app);
    }

  return;
}

/* Find a sequence item, given its name.  If the item is not found,
 * returns NULL.  */
struct sequence_item_info *
find_item_by_name (gchar * item_name, struct sequence_info *sequence_data,
                   GApplication *app)
{

  struct sequence_item_info *item, *found_item;
  GList *item_list;
  gchar *trace_text;

  found_item = NULL;
  for (item_list = sequence_data->item_list; item_list != NULL;
       item_list = item_list->next)
    {
      item = item_list->data;

      if (g_strcmp0 (item_name, item->name) == 0)
        {
          found_item = item;
          break;
        }
    }

  if ((found_item == NULL) && (trace_sequencer_level (app) > 0))
    {
      trace_text =
        g_strdup_printf ("Item with name %s not found.", item_name);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  return (found_item);
}

/* Execute a sequence item.  */
void
execute_item (struct sequence_item_info *the_item,
              struct sequence_info *sequence_data, GApplication *app)
{
  gchar *display_text;
  gchar *trace_text;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text = g_strdup_printf ("Executing item %s.", the_item->name);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  switch (the_item->type)
    {
    case unknown:
      display_text =
        g_strdup_printf ("Unknown type of sequence item: %s.", the_item->name);
      display_show_message (display_text, app);
      if (trace_sequencer_level (app) > 0)
	{
	  trace_sequencer_write (display_text, app);
	}
      g_free (display_text);
      display_text = NULL;
      break;

    case start_sound:
      execute_start_sound (the_item, sequence_data, app);
      break;

    case stop_sound:
      execute_stop_sound (the_item, sequence_data, app);
      break;

    case wait:
      execute_wait (the_item, sequence_data, app);
      break;

    case offer_sound:
      execute_offer_sound (the_item, sequence_data, app);
      break;

    case cease_offering_sound:
      execute_cease_offering_sound (the_item, sequence_data, app);
      break;

    case operator_wait:
      execute_operator_wait (the_item, sequence_data, app);
      break;

    case cancel_wait:
      execute_cancel_wait (the_item, sequence_data, app);

    case start_sequence:
      display_show_message ("Start sequence", app);
      break;
    }

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
	g_strdup_printf ("Finished executing item %s, next is %s.",
			 the_item->name, sequence_data->next_item_name);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  return;
}

/* Execute a Start Sound sequence item.  */
void
execute_start_sound (struct sequence_item_info *the_item,
                     struct sequence_info *sequence_data, GApplication *app)
{
  gint cluster_number;
  struct sound_info *sound_effect;
  struct sound_info *old_sound_effect;
  struct remember_info *remember_data;
  gboolean item_found;
  GList *item_list;
  gchar *display_text;
  gchar *trace_text;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Start Sound, name = %s, cluster = %d, "
                         "sound name = %s, next = %s, "
                         "tag %s, stopped = %s, release = %s, complete = %s, "
                         "terminate = %s.", the_item->name,
                         the_item->cluster_number, the_item->sound_name,
                         the_item->next_starts, the_item->tag,
			 the_item->next_sound_stopped,
                         the_item->next_release_started,
                         the_item->next_completion,
                         the_item->next_termination);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  cluster_number = the_item->cluster_number;

  /* See if there is already a sound on this cluster.  */
  item_found = FALSE;
  for (item_list = sequence_data->running; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      old_sound_effect = remember_data->sound_effect;
      if ((remember_data->cluster_number == cluster_number)
          && (remember_data->off_cluster == FALSE))
        {
          item_found = TRUE;
          break;
        }
    }
  if (item_found)
    {
      if ((!old_sound_effect->release_has_started) &&
	  (!old_sound_effect->release_sent))
        {
          display_text =
            g_strdup_printf ("Cannot start sound %s on busy"
                             " cluster %d, occupied by %s.",
                             the_item->sound_name, the_item->cluster_number,
                             old_sound_effect->name);
          display_show_message (display_text, app);
	  if (trace_sequencer_level (app) > 0)
	    trace_sequencer_write (display_text, app);
          g_free (display_text);
          display_text = NULL;
          return;
        }
      /* There is a sound on this cluster, but it is releasing.
       * Remove it from the cluster in favor of this new sound.  */
      button_reset_cluster (old_sound_effect, app);
      remember_data->off_cluster = TRUE;
      sound_unbind_from_cluster (old_sound_effect, app);
    }

  /* Set the name of the cluster to the specified text.  */
  sound_cluster_set_name (the_item->text_to_display, cluster_number, app);

  /* Associate the sound with the cluster.  */
  sound_effect =
    sound_bind_to_cluster (the_item->sound_name, cluster_number, app);

  /* If the sound does not exist, don't try to start it.  */
  if (sound_effect != NULL)
    {
      /* Start that sound.  */
      sound_start_playing (sound_effect, app);

      /* Show the operator that a sound is playing on this cluster.  */
      button_set_cluster_playing (sound_effect, app);

      /* Remember that the sound is running.  */
      remember_data = g_malloc (sizeof (struct remember_info));
      remember_data->cluster_number = cluster_number;
      remember_data->sequence_item = the_item;
      remember_data->sound_effect = sound_effect;
      remember_data->active = TRUE;
      remember_data->being_displayed = FALSE;
      remember_data->release_seen = FALSE;
      remember_data->release_sent = FALSE;
      remember_data->off_cluster = FALSE;
      remember_data->stopped_by_operator = FALSE;
      sequence_data->running =
        g_list_append (sequence_data->running, remember_data);
    }
  else
    {
      trace_text =
        g_strdup_printf ("Sound %s not defined.", the_item->sound_name);
      display_show_message (trace_text, app);
      if (trace_sequencer_level (app) > 0)
        {
          trace_sequencer_write (trace_text, app);
        }
      g_free (trace_text);
      trace_text = NULL;
    }

  /* In case this is the most important text to be displayed to the operator,
   * update the operator display.  */
  update_operator_display (sequence_data, app);

  /* Advance to the next sequence item.  */
  sequence_data->next_item_name = the_item->next_starts;

  return;
}

/* Execute a Stop Sound sequence item.  */
void
execute_stop_sound (struct sequence_item_info *the_item,
                    struct sequence_info *sequence_data, GApplication *app)
{

  struct sound_info *sound_effect;
  struct remember_info *remember_data;
  struct sequence_item_info *sequence_item;
  gboolean item_found, still_searching;
  GList *item_list;
  gchar *trace_text;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Stop Sound, name = %s, tag = %s, next = %s.",
                         the_item->name, the_item->tag, the_item->next);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Stop all running sounds with the specified tag.  */
  still_searching = TRUE;

  while (still_searching)
    {
      /* Find all running sounds whose Start Sound sequence item has 
       * the specified tag.  */
      item_found = FALSE;
      for (item_list = sequence_data->running; item_list != NULL;
           item_list = item_list->next)
        {
          remember_data = item_list->data;
          sequence_item = remember_data->sequence_item;
          if ((g_strcmp0 (the_item->tag, sequence_item->tag) == 0)
              && remember_data->active && !remember_data->release_sent)
            {
              item_found = TRUE;
              break;
            }
        }
      if (item_found)
        {
          /* Stop the sound.  When the sound terminates we will clean up.  */
	  sound_effect = remember_data->sound_effect;
	  if (trace_sequencer_level (app) > 0)
	    {
	      trace_text = g_strdup_printf ("Stopping sound %s.",
					    sound_effect->name);
	      trace_sequencer_write (trace_text, app);
	      g_free (trace_text);
	      trace_text = NULL;
	    }
          remember_data->release_sent = TRUE;
          sound_stop_playing (sound_effect, app);
        }
      else
        {
          still_searching = FALSE;
        }
    }

  /* Advance to the next sequence item.  */
  sequence_data->next_item_name = the_item->next;

  return;
}

/* Execute a Wait sequence item.  */
void
execute_wait (struct sequence_item_info *the_item,
              struct sequence_info *sequence_data, GApplication *app)
{
  struct remember_info *remember_data;
  gchar *trace_text;
  gdouble wait_seconds;

  if (trace_sequencer_level (app) > 0)
    {
      wait_seconds = the_item->time_to_wait / 1e9;
      trace_text =
        g_strdup_printf ("Wait, name = %s, time = %f, "
                         "when complete = %s, operator text = %s, "
                         "tag = %s, next = %s.", the_item->name, wait_seconds,
                         the_item->next_completion, the_item->text_to_display,
                         the_item->tag, the_item->next);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Record information about the wait, since we will need it when
   * the wait is over.  */
  remember_data = g_malloc (sizeof (struct remember_info));
  remember_data->cluster_number = 0;
  remember_data->sound_effect = NULL;
  remember_data->sequence_item = the_item;

  if ((sequence_data->waiting == NULL)
      && (sequence_data->current_operator_wait == NULL))
    {
      /* There are no prior Wait or Operator Wait commands running.  */
      /* TODO: display the Wait that will end soonest.  */
      remember_data->active = TRUE;
      display_set_operator_text (the_item->text_to_display, app);
    }
  else
    {
      remember_data->active = FALSE;
    }
  remember_data->release_seen = FALSE;
  remember_data->release_sent = FALSE;
  remember_data->stopped_by_operator = FALSE;
  remember_data->off_cluster = FALSE;

  /* Place this item on the wait list.  */
  sequence_data->waiting =
    g_list_append (sequence_data->waiting, remember_data);

  /* Arrange to call wait_completed when the wait is over.  */
  timer_create_entry (wait_completed, (the_item->time_to_wait / 1e9),
                      remember_data, app);

  /* Advance to the next sequence item.  */
  sequence_data->next_item_name = the_item->next;

  return;
}

/* Call here from the timer when a wait is completed.  */
static void
wait_completed (void *user_data, GApplication *app)
{
  struct remember_info *remember_data = user_data;
  struct sequence_info *sequence_data;
  struct sequence_item_info *current_sequence_item;
  GList *list_element, *next_list_element;
  gchar *trace_text;
  gdouble wait_seconds;

  sequence_data = sep_get_sequence_data (app);
  current_sequence_item = NULL;

  /* Find this item on the wait list so we can remove it.  */
  list_element = g_list_first (sequence_data->waiting);
  while (list_element != NULL)
    {
      next_list_element = list_element->next;
      if (remember_data == list_element->data)
        {
          /* This is the item on the list.  Remove it.  */
          remember_data->active = FALSE;
          current_sequence_item = remember_data->sequence_item;
          g_free (remember_data);
          sequence_data->waiting =
            g_list_delete_link (sequence_data->waiting, list_element);
        }
      list_element = next_list_element;
    }

  /* If we didn't find the item on the list, do nothing.  */
  if (current_sequence_item == NULL)
    {
      return;
    }

  if (trace_sequencer_level (app) > 0)
    {
      wait_seconds = current_sequence_item->time_to_wait / 1e9;
      trace_text =
        g_strdup_printf ("Wait completed, name = %s, time = %f,"
                         " when complete = %s, operator text = %s, "
                         "next = %s.", current_sequence_item->name,
                         wait_seconds, current_sequence_item->next_completion,
                         current_sequence_item->text_to_display,
                         current_sequence_item->next);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* TODO: display the wait list item that will end soonest,
   * or the current operator wait if there is one.  */

  /* Tell the sequencer to proceed from the specified item.  */
  sequence_data->next_item_name = current_sequence_item->next_completion;
  execute_items (sequence_data, app);

  return;
}

/* Execute an Offer Sound sequence item.  */
void
execute_offer_sound (struct sequence_item_info *the_item,
                     struct sequence_info *sequence_data, GApplication *app)
{
  struct remember_info *remember_data;
  gchar *trace_text;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Offer sound, name = %s, " "cluster = %d, "
                         "Q number = %s, OSC cue number = %d, "
                         "OSC cue string = %s, "
                         "next = %s, next_to_start = %s, tag = %s.",
                         the_item->name, the_item->cluster_number,
                         the_item->Q_number, the_item->OSC_cue_number,
                         the_item->OSC_cue_string, the_item->next,
                         the_item->next_to_start, the_item->tag);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Set the name of the cluster to the specified text.  */
  sound_cluster_set_name (the_item->text_to_display, the_item->cluster_number,
                          app);

  /* Remember that the sound is being offered.  */

  remember_data = g_malloc (sizeof (struct remember_info));
  remember_data->cluster_number = the_item->cluster_number;
  remember_data->sound_effect = NULL;
  remember_data->sequence_item = the_item;
  remember_data->active = TRUE;
  remember_data->release_seen = FALSE;
  remember_data->release_sent = FALSE;
  remember_data->stopped_by_operator = FALSE;
  remember_data->off_cluster = FALSE;

  sequence_data->offering =
    g_list_append (sequence_data->offering, remember_data);

  /* Advance to the next sequence item.  */
  sequence_data->next_item_name = the_item->next;

  return;
}

/* Execute a Cease Offering Sound sequence item.  */
void
execute_cease_offering_sound (struct sequence_item_info *the_item,
                              struct sequence_info *sequence_data,
                              GApplication *app)
{
  gint cluster_number;
  struct remember_info *remember_data;
  struct sequence_item_info *sequence_item;
  GList *list_element, *next_list_element;
  gchar *trace_text;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Cease offering sound, name = %s, tag = %s, "
                         "next = %s.", the_item->name, the_item->tag,
                         the_item->next);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Process every Offer Sound sequence item with the same tag.  */
  list_element = sequence_data->offering;
  while (list_element != NULL)
    {
      next_list_element = list_element->next;
      remember_data = list_element->data;
      sequence_item = remember_data->sequence_item;
      if ((g_strcmp0 (the_item->tag, sequence_item->tag) == 0)
          && (remember_data->active))
        {
          /* We have a match.  */
          if (trace_sequencer_level (app) > 0)
            {
              trace_text =
                g_strdup_printf ("Canceling Offer Sound on cluster %d.",
                                 remember_data->cluster_number);
              trace_sequencer_write (trace_text, app);
              g_free (trace_text);
              trace_text = NULL;
            }
          remember_data->active = FALSE;

          /* Remove the Offer Sound from the cluster.  */
          sequence_data->offering =
            g_list_remove_link (sequence_data->offering, list_element);

          /* Remove the Offer Sound's text from the cluster.  */
          cluster_number = remember_data->cluster_number;
          sound_cluster_set_name ((gchar *) "", cluster_number, app);

          g_list_free (list_element);
          g_free (remember_data);
	  remember_data = NULL;
        }
      list_element = next_list_element;
    }

  /* Advance to the next sequence item.  */
  sequence_data->next_item_name = the_item->next;

  return;
}

/* Execute an Operator Wait sequence item.  */
void
execute_operator_wait (struct sequence_item_info *the_item,
                       struct sequence_info *sequence_data,
                       GApplication *app)
{
  struct remember_info *remember_data;
  gchar *trace_text;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Operator Wait, name = %s, next play = %s, "
                         "operator text = %s, tag= %s, next = %s.",
                         the_item->name, the_item->next_play,
                         the_item->text_to_display, the_item->tag,
                         the_item->next);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Record information about the operator wait, since we will need it when
   * the operator wait is over.  */
  remember_data = g_malloc (sizeof (struct remember_info));
  remember_data->cluster_number = 0;
  remember_data->sound_effect = NULL;
  remember_data->sequence_item = the_item;
  remember_data->release_seen = FALSE;
  remember_data->release_sent = FALSE;
  remember_data->stopped_by_operator = FALSE;
  remember_data->off_cluster = FALSE;

  if (sequence_data->current_operator_wait == NULL)
    {
      /* There are no prior operator wait commands running.  */
      remember_data->active = TRUE;
      sequence_data->current_operator_wait = remember_data;
      display_set_operator_text (the_item->text_to_display, app);
    }
  else
    {
      /* There is an Operator Wait already running; place this one
       * on the list to be executed later.  */
      remember_data->active = FALSE;
      sequence_data->operator_waiting =
        g_list_append (sequence_data->operator_waiting, remember_data);
    }

  /* Advance to the next sequence item.  */
  sequence_data->next_item_name = the_item->next;

  return;
}

/* Execute a Cancel Wait sequence item.  */
void
execute_cancel_wait (struct sequence_item_info *the_item,
                     struct sequence_info *sequence_data, GApplication *app)
{
  struct remember_info *remember_data;
  struct sequence_item_info *current_sequence_item;
  struct sequence_item_info *next_sequence_item;
  GList *list_element, *next_list_element;
  gchar *trace_text;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Cancel Wait, name = %s, tag = %s, next = %s.",
                         the_item->name, the_item->tag, the_item->next);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Find all Waits with this tag that are in the remember queue and 
   * remove them.  */
  current_sequence_item = NULL;

  list_element = g_list_first (sequence_data->waiting);
  while (list_element != NULL)
    {
      next_list_element = list_element->next;
      remember_data = list_element->data;
      current_sequence_item = remember_data->sequence_item;
      if (g_strcmp0 (current_sequence_item->tag, the_item->tag) == 0)
        {
          /* The tag matches.  Remove the item.  */
          remember_data->active = FALSE;
          g_free (remember_data);
          sequence_data->waiting =
            g_list_delete_link (sequence_data->waiting, list_element);
        }
      list_element = next_list_element;
    }

  /* Find all Operator Waits with this tag that are in the remember queue and 
   * remove them also.  */
  current_sequence_item = NULL;

  list_element = g_list_first (sequence_data->operator_waiting);
  while (list_element != NULL)
    {
      next_list_element = list_element->next;
      remember_data = list_element->data;
      current_sequence_item = remember_data->sequence_item;
      if (g_strcmp0 (current_sequence_item->tag, the_item->tag) == 0)
        {
          /* The tag matches.  Remove the item.  */
          remember_data->active = FALSE;
          g_free (remember_data);
          sequence_data->waiting =
            g_list_delete_link (sequence_data->waiting, list_element);
        }
      list_element = next_list_element;
    }

  /* If there is an Operator Wait that is in execution, a common case,
   * and if it has the specified tag, cancel it.  */

  remember_data = sequence_data->current_operator_wait;

  /* If we are not waiting for the operator to press the key,
   * there is nothing to cancel.  */
  if (remember_data != NULL)
    {
      current_sequence_item = remember_data->sequence_item;

      /* This Operator Wait sequence item is no longer waiting.  */
      sequence_data->current_operator_wait = NULL;
      g_free (remember_data);
      remember_data = NULL;

      /* See if there is another one ready to wait.
       * We have already deleted those with the matching tag,
       * so any left will be allowed to execute.  */
      list_element = g_list_first (sequence_data->operator_waiting);
      if (list_element != NULL)
        {
          /* There is, give it its chance to display for the opeaator.  */
          sequence_data->operator_waiting =
            g_list_remove_link (sequence_data->operator_waiting,
                                list_element);
          remember_data = list_element->data;
          remember_data->active = TRUE;
          next_sequence_item = remember_data->sequence_item;
          display_set_operator_text (next_sequence_item->text_to_display,
                                     app);
          sequence_data->current_operator_wait = remember_data;
          g_list_free (list_element);
        }
      else
        {
          display_clear_operator_text (app);
        }
    }

  /* Advance to the next sequence item.  */
  sequence_data->next_item_name = the_item->next;

  return;
}

/* Update the operator display.  Show the most important item, preferring
 * the current item in case of a tie.  */
static void
update_operator_display (struct sequence_info *sequence_data,
                         GApplication *app)
{
  struct remember_info *remember_data;
  struct sequence_item_info *sequence_item;
  struct remember_info *most_important;
  struct remember_info *current_display;
  struct sound_info *sound_effect;
  gboolean found_item;
  guint most_importance;
  GList *item_list;
  guint64 elapsed_time, remaining_time;
  gchar *display_text;

  found_item = FALSE;
  most_importance = 0;
  most_important = NULL;
  current_display = NULL;
  for (item_list = sequence_data->running; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;

      /* Note which item is currently being displayed.  */
      if (remember_data->being_displayed)
        {
          current_display = remember_data;
        }

      /* Find the item that should be displayed.  */
      sequence_item = remember_data->sequence_item;
      if ((remember_data->active) && (sequence_item->importance > 0))
        {
          if (found_item == FALSE)
            {
              most_important = remember_data;
              most_importance = sequence_item->importance;
              found_item = TRUE;
            }
          else
            {
              if (sequence_item->importance > most_importance)
                {
                  most_important = remember_data;
                  most_importance = sequence_item->importance;
                }
              else
                {
                  if ((sequence_item->importance == most_importance)
                      && (remember_data->being_displayed == TRUE))
                    {
                      most_important = remember_data;
                    }
                }
            }
        }
    }

  if (found_item == TRUE)
    {
      /* "most_important" is the item we should be displaying.  
       * "current_display" is the item we are currently displaying, if any.  
       * These may be the same item.  */
      sequence_item = most_important->sequence_item;
      sound_effect = most_important->sound_effect;
      /* Prepend the elapsed time to the activity information, and append
       * the remaining time, if known.  */
      elapsed_time = sound_get_elapsed_time (sound_effect, app);
      remaining_time = sound_get_remaining_time (sound_effect, app);
      if (remaining_time == G_MAXUINT64)
        {
          display_text =
            g_strdup_printf ("%4.1f %s", (gdouble) elapsed_time / 1e9,
                             sequence_item->text_to_display);
        }
      else
        {
          display_text =
            g_strdup_printf ("%4.1f %s (%4.1f)", (gdouble) elapsed_time / 1e9,
                             sequence_item->text_to_display,
                             (gdouble) remaining_time / 1e9);
        }

      /* Display the most important activity.  */
      display_current_activity (display_text, app);
      g_free (display_text);
      display_text = NULL;

      /* Mark the most important item as the one currently being displayed.  */
      if (current_display != NULL)
        {
          current_display->being_displayed = FALSE;
        }
      most_important->being_displayed = TRUE;

      /* Keep updating the display every 0.1 second until there is nothing
       * to show.  */
      timer_create_entry (clock_tick, 0.1, sequence_data, app);
    }
  else
    {
      /* There is nothing happening.  */
      display_current_activity ((gchar *) "", app);
    }
  return;
}

/* Come here when the 0.1-second clock ticks to update the activity display.  */
static void
clock_tick (void *user_data, GApplication *app)
{
  struct sequence_info *sequence_data = user_data;
  update_operator_display (sequence_data, app);
  return;
}

/* Execute the Go command from an external sequencer issuing MIDI Show Control
 * commands.  */
void
sequence_MIDI_show_control_go (gchar * Q_number, GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct sequence_item_info *sequence_item;
  gboolean found_item;
  GList *item_list;
  gchar *display_text;
  gchar *trace_text;

  sequence_data = sep_get_sequence_data (app);

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("MIDI show control go, Q_number = %s.", Q_number);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Find the cluster whose Offer Sound sequence item has the specified
   * Q_number.  */
  found_item = FALSE;
  for (item_list = sequence_data->offering; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      sequence_item = remember_data->sequence_item;
      if ((g_strcmp0 (Q_number, sequence_item->Q_number) == 0)
          && (remember_data->active))
        {
          found_item = TRUE;
          break;
        }
    }

  /* If no cluster has been marked by Offer Sound with that
   * sequence item, check for "quit" as a special case.  
   * Placing this test after the search for a cluster
   * lets the sequence capture the quit command and do a
   * controlled shutdown.  */
  if (!found_item)
    {
      if (g_strcmp0 (Q_number, (gchar *) "quit") == 0)
        {
          /* Terminate the sound effects player.  */
          g_application_quit (app);
          return;
        }
    }

  if (!found_item)
    {
      display_text =
        g_strdup_printf ("There is no cluster holding an"
                         " Offer Sound with Q_number %s.", Q_number);
      display_show_message (display_text, app);
      if (trace_sequencer_level (app) > 0)
        {
          trace_sequencer_write (display_text, app);
        }
      g_free (display_text);
      display_text = NULL;
      return;
    }

  /* Run the sequencer.  A subsequent Start Sound sequence item
   * which names this same cluster will take posession of the cluster
   * until it completes or is terminated.  */
  sequence_data->next_item_name = sequence_item->next_to_start;
  execute_items (sequence_data, app);

  return;
}

/* Execute the Go_off command from an external sequencer issuing MIDI Show 
 * Control commands.  */
void
sequence_MIDI_show_control_go_off (gchar * Q_number, GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct sequence_item_info *sequence_item;
  GList *item_list;
  gchar *trace_text;

  sequence_data = sep_get_sequence_data (app);

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("MIDI show control go_off, Q_number = %s.",
                         Q_number);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }
  /* Stop the running sounds whose Start Sound sequence item has the specified
   * Q_number.  If there is no Q number, stop all sounds.  */
  for (item_list = sequence_data->running; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      sequence_item = remember_data->sequence_item;
      if (((Q_number == NULL) || (g_strcmp0 (Q_number, (gchar *) ""))
           || (g_strcmp0 (Q_number, sequence_item->Q_number) == 0))
          && remember_data->active && !remember_data->release_sent)
        {
          /* Stop the sound.  When the sound terminates we will clean up.  */
          remember_data->release_sent = TRUE;
	  remember_data->stopped_by_operator = TRUE;
          sound_stop_playing (remember_data->sound_effect, app);
	  /* Run the sequencer starting from the next_sound_stopped label.  */
	  sequence_data->next_item_name = sequence_item->next_sound_stopped;
	  execute_items (sequence_data, app);
        }
    }

  return;
}

/* Execute the Open Sound Control (OSC) cue command
 * when the operand is a number.  */
void
sequence_OSC_cue_number (guint osc_cue_number, GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct sequence_item_info *sequence_item;
  gboolean found_item;
  GList *item_list;
  gchar *display_text;
  gchar *trace_text;

  sequence_data = sep_get_sequence_data (app);

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("OSC cue number, operand = %d.", osc_cue_number);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Find the cluster whose Offer Sound sequence item has the specified
   * OSC_cue number.  */
  found_item = FALSE;
  for (item_list = sequence_data->offering; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      sequence_item = remember_data->sequence_item;
      if (sequence_item->OSC_cue_number_specified
          && (sequence_item->OSC_cue_number == osc_cue_number)
          && (remember_data->active))
        {
          found_item = TRUE;
          break;
        }
    }

  if (!found_item)
    {
      display_text =
        g_strdup_printf ("There is no cluster with OSC cue number %d.",
                         osc_cue_number);
      display_show_message (display_text, app);
      if (trace_sequencer_level (app) > 0)
        {
          trace_sequencer_write (display_text, app);
        }
      g_free (display_text);
      display_text = NULL;
      return;
    }

  /* Run the sequencer.  A subsequent Start Sound sequence item
   * which names this same cluster will take posession of the cluster
   * until it completes or is terminated.  */
  sequence_data->next_item_name = sequence_item->next_to_start;
  execute_items (sequence_data, app);

  return;
}

/* Execute the Open Sound Control (OSC) cue command
 * when the operand is a string.  */
void
sequence_OSC_cue_string (gchar * osc_cue_string, GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct sequence_item_info *sequence_item;
  gboolean found_item;
  GList *item_list;
  gchar *display_text;
  gchar *trace_text;

  sequence_data = sep_get_sequence_data (app);

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("OSC cue string, operand = %s.", osc_cue_string);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* Find the cluster whose Offer Sound sequence item has the specified
   * OSC_cue string.  */
  found_item = FALSE;
  for (item_list = sequence_data->offering; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      sequence_item = remember_data->sequence_item;
      if (sequence_item->OSC_cue_string_specified
          && (g_strcmp0 (sequence_item->OSC_cue_string, osc_cue_string) == 0)
          && (remember_data->active))
        {
          found_item = TRUE;
          break;
        }
    }

  if (!found_item)
    {
      display_text =
        g_strdup_printf ("There is no cluster with OSC cue string %s.",
                         osc_cue_string);
      display_show_message (display_text, app);
      if (trace_sequencer_level (app) > 0)
        {
          trace_sequencer_write (display_text, app);
        }
      g_free (display_text);
      display_text = NULL;
      return;
    }

  /* Run the sequencer.  A subsequent Start Sound sequence item
   * which names this same cluster will take posession of the cluster
   * until it completes or is terminated.  */
  sequence_data->next_item_name = sequence_item->next_to_start;
  execute_items (sequence_data, app);

  return;
}

/* Process the Start button on a cluster.  */
void
sequence_cluster_start (guint cluster_number, GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct sequence_item_info *sequence_item;
  gboolean found_item;
  GList *item_list;
  gchar *display_text;
  gchar *trace_text;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Start button pressed on cluster %d.",
                         cluster_number);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  sequence_data = sep_get_sequence_data (app);

  /* See if there is an Offer Sound sequence item outstanding which names
   * this cluster.  */
  found_item = FALSE;
  for (item_list = sequence_data->offering; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      if ((remember_data->cluster_number == cluster_number) &&
	  (!remember_data->off_cluster))
        {
          found_item = TRUE;
          break;
        }
    }
  if (!found_item)
    {
      display_text =
        g_strdup_printf ("No Offer Sound outstanding on cluster %d",
                         cluster_number);
      display_show_message (display_text, app);
      if (trace_sequencer_level (app) > 0)
        {
          trace_sequencer_write (display_text, app);
        }
      g_free (display_text);
      display_text = NULL;
      return;
    }

  /* We have an Offer Sound sequence item on this cluster.
   * Run the sequencer starting at its specified sequence item.  */
  sequence_item = remember_data->sequence_item;
  sequence_data->next_item_name = sequence_item->next_to_start;
  execute_items (sequence_data, app);

  return;
}

/* Process the Stop button on a cluster.  */
void
sequence_cluster_stop (guint cluster_number, GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct sequence_item_info *start_sound_sequence_item;
  gboolean item_found;
  GList *item_list;
  gchar *display_text;
  gchar *trace_text;

  sequence_data = sep_get_sequence_data (app);

  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Stop button pressed on cluster %d.",
                         cluster_number);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* See if there is a Start Sound sequence item outstanding which names
   * this cluster.  */
  item_found = FALSE;
  for (item_list = sequence_data->running; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      if ((remember_data->cluster_number == cluster_number)
          && remember_data->active && !remember_data->release_sent)
        {
          item_found = TRUE;
          break;
        }
    }
  if (!item_found)
    {
      /* There isn't.  Ignore the stop button.  */
      display_text =
        g_strdup_printf ("No sound to stop on cluster %d.", cluster_number);
      display_show_message (display_text, app);
      if (trace_sequencer_level (app) > 0)
        {
          trace_sequencer_write (display_text, app);
        }
      g_free (display_text);
      display_text = NULL;
      return;
    }

  start_sound_sequence_item = remember_data->sequence_item;
  
  /* We have a Start Sound sequence item on this cluster.
   * Tell the sound to stop.  When it has stopped, the termination
   * process will be invoked.  */
  remember_data->release_sent = TRUE;
  remember_data->stopped_by_operator = TRUE;
  sound_stop_playing (remember_data->sound_effect, app);

  /* Run the sequencer starting from the next_sound_stopped label.  */
  sequence_data->next_item_name =
    start_sound_sequence_item->next_sound_stopped;
  execute_items (sequence_data, app);
    
  return;
}

/* Process the Play button.  */
void
sequence_button_play (GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct sequence_item_info *current_sequence_item;
  struct sequence_item_info *next_sequence_item;
  GList *list_element;
  gchar *trace_text;

  sequence_data = sep_get_sequence_data (app);
  remember_data = sequence_data->current_operator_wait;

  if (trace_sequencer_level (app) > 0)
    {
      trace_text = g_strdup_printf ("Play button pressed.");
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  /* If we are not waiting for the operator to press the key,
   * do nothing.  */
  if (remember_data == NULL)
    return;

  current_sequence_item = remember_data->sequence_item;

  /* This Operator Wait sequence item is no longer waiting.  */
  sequence_data->current_operator_wait = NULL;
  g_free (remember_data);
  remember_data = NULL;

  /* See if there is another one ready to wait.  */
  list_element = g_list_first (sequence_data->operator_waiting);
  if (list_element != NULL)
    {
      /* There is, give it its chance to display for the opeaator.  */
      sequence_data->operator_waiting =
        g_list_remove_link (sequence_data->operator_waiting, list_element);
      remember_data = list_element->data;
      remember_data->active = TRUE;
      next_sequence_item = remember_data->sequence_item;
      display_set_operator_text (next_sequence_item->text_to_display, app);
      sequence_data->current_operator_wait = remember_data;
      g_list_free (list_element);
    }
  else
    {
      display_clear_operator_text (app);
    }

  /* Run the sequencer starting from the Operator Wait's specified label.  */
  sequence_data->next_item_name = current_sequence_item->next_play;
  execute_items (sequence_data, app);

  return;
}

/* Process the completion or termination of a sound.  */
void
sequence_sound_completion (struct sound_info *sound_effect,
                           gboolean terminated, GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct remember_info *offer_remember_data;
  struct sequence_item_info *start_sound_sequence_item;
  struct sequence_item_info *offer_sound_sequence_item;
  gboolean item_found;
  GList *item_list, *found_item;
  gchar *trace_text;
  guint cluster_number;

  sequence_data = sep_get_sequence_data (app);
  cluster_number = sound_effect->cluster_number;
  
  if (trace_sequencer_level (app) > 0)
    {
      if (terminated)
	{
	  trace_text =
	    g_strdup_printf ("Termination of sound %s on cluster %d.",
			     sound_effect->name, cluster_number);
	}
      else
	{
	  trace_text =
	    g_strdup_printf ("Completion of sound %s on cluster %d.",
			     sound_effect->name, cluster_number);
	}
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }
  
  /* See if there is a Start Sound sequence item outstanding which names
   * this sound.  */
  item_found = FALSE;
  for (item_list = sequence_data->running; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      if ((remember_data->sound_effect == sound_effect)
          && remember_data->active)
        {
          found_item = item_list;
          item_found = TRUE;
          break;
        }
    }

  if (!item_found)
    {
      /* There isn't.  Ignore the completion.  */
      display_show_message ("Sound is not running.", app);
      if (trace_sequencer_level (app) > 0)
        {
          trace_sequencer_write ("Sound is not running.", app);
        }
      return;
    }

  /* We have a Start Sound sequence item for this sound.  It has
   * completed or terminated.  */
  start_sound_sequence_item = remember_data->sequence_item;

  /* If this sound is still showing on the cluster, set the start label 
   * back to "Start".  */
  if (!remember_data->off_cluster)
    {
      button_reset_cluster (sound_effect, app);
      remember_data->off_cluster = TRUE;
      sound_unbind_from_cluster (sound_effect, app);
    }
  
  /* See if there is an Offer Sound sequence item outstanding which names
   * this cluster.  */
  item_found = FALSE;
  for (item_list = sequence_data->offering; item_list != NULL;
       item_list = item_list->next)
    {
      offer_remember_data = item_list->data;
      if ((offer_remember_data->active)
	  && (offer_remember_data->cluster_number == cluster_number))
	{
	  offer_sound_sequence_item = offer_remember_data->sequence_item;
	  item_found = TRUE;
	  break;
	}
    }
  
  /* If there is, restore its text to the cluster.  If there isn't, clear
   * the text field.  */
  if (item_found)
    {
      if (trace_sequencer_level (app) > 0)
	{
	  trace_text =
	    g_strdup_printf ("Offer Sound item %s reinstated "
			     "on cluster %d.",
			     offer_sound_sequence_item->name,
			     cluster_number);
	  trace_sequencer_write (trace_text, app);
	  g_free (trace_text);
	  trace_text = NULL;
	}
      sound_cluster_set_name (offer_sound_sequence_item->text_to_display,
			      cluster_number, app);
    }
  else
    {
      sound_cluster_set_name ((gchar *) "", cluster_number,
			      app);
    }

  /* Remove the sequence item from the running list.  */
  sequence_data->running =
    g_list_remove_link (sequence_data->running, found_item);
  g_free (remember_data);
  g_list_free (found_item);

  /* If there is another sound running, show its status.  */
  update_operator_display (sequence_data, app);

  /* Now that the Start Sound has completed, run the sequencer
   * from its completion or termination label.  */
  if (terminated)
    {
      sequence_data->next_item_name =
        start_sound_sequence_item->next_termination;
    }
  else
    {
      sequence_data->next_item_name =
        start_sound_sequence_item->next_completion;
    }
  execute_items (sequence_data, app);

  return;
}

/* Process the start of the release stage of a sound.  */
void
sequence_sound_release_started (struct sound_info *sound_effect,
                                GApplication *app)
{
  struct sequence_info *sequence_data;
  struct remember_info *remember_data;
  struct sequence_item_info *start_sound_sequence_item;
  gboolean item_found;
  GList *item_list;
  gchar *trace_text;
  guint cluster_number;
  
  sequence_data = sep_get_sequence_data (app);
  cluster_number = sound_effect->cluster_number;
  
  if (trace_sequencer_level (app) > 0)
    {
      trace_text =
        g_strdup_printf ("Release started for sound %s on cluster %d.",
                         sound_effect->name, cluster_number);
      trace_sequencer_write (trace_text, app);
      g_free (trace_text);
      trace_text = NULL;
    }

  sound_effect->release_has_started = TRUE;

  /* See if there is a Start Sound sequence item outstanding for this
   * sound effect.  */
  item_found = FALSE;
  for (item_list = sequence_data->running; item_list != NULL;
       item_list = item_list->next)
    {
      remember_data = item_list->data;
      if ((remember_data->sound_effect == sound_effect)
          && (remember_data->active))
        {
          item_found = TRUE;
          break;
        }
    }

  if (!item_found)
    {
      /* There isn't.  Ignore the release.  */
      display_show_message ("Release started but sound not running.", app);
      if (trace_sequencer_level (app) > 0)
        {
          trace_sequencer_write ("release started but sound not running",
                                 app);
        }
      return;
    }

  remember_data->release_seen = TRUE;

  /* We have a Start Sound sequence item for this sound.  It has
   * started the release stage of its amplitude envelope.  */
  start_sound_sequence_item = remember_data->sequence_item;

  /* Show the operator that the sound is now releasing, if it is still
   * attached to its cluster.  */
  if (!remember_data->off_cluster)
    button_set_cluster_releasing (sound_effect, app);

  /* If there is another sound running, show its status.  */
  update_operator_display (sequence_data, app);

  /* Run the sequencer from the release_started label unless the
   * sound was stopped by the operator.  If we have already sent
   * a release command to the sound, don't run from the release started
   * label but instead wait for the termination or completion
   * of the sound and run from the appropriate label at that time.
   */

  if ((!remember_data->release_sent) && (!remember_data->stopped_by_operator))
    {
      sequence_data->next_item_name =
        start_sound_sequence_item->next_release_started;
      execute_items (sequence_data, app);
    }

  return;
}
