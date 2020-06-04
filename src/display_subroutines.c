/*
 * display_subroutines.c
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
#include "display_subroutines.h"
#include "sound_subroutines.h"
#include "sound_effects_player.h"

#define TRACE_DISPLAY FALSE

/* The persistent data used by the display subroutines.  */
struct display_info
{
  GtkWidget *VU_meter;
  gint initialized;
};

/* Subroutines for display progessing.  */
void *
display_init (GApplication *app)
{
  struct display_info *display_data;

  display_data = g_malloc (sizeof (struct display_info));
  display_data->VU_meter = NULL;
  display_data->initialized = 0;
  return (display_data);
}

void
display_finish (GApplication *app)
{
  struct display_info *display_data;

  display_data = sep_get_display_data (app);
  g_free (display_data);
  display_data = NULL;
  return;
}

/* Initialize the display data.  We need the sound data to be complete,
 * so we defer this until the first time we update the VU meter.  */
void
initialize_display_data (GApplication *app)
{
  struct display_info *display_data;
  GtkWidget *common_area;
  GList *children_list, *l1;
  GList *grandchildren_list, *l2;
  GtkWidget *child_widget;
  GtkWidget *grandchild_widget;
  const gchar *child_name;
  const gchar *grandchild_name;
  GtkWidget *VU_meter = NULL;
  GtkLabel *text_label;
  gint64 channel_number;
  gchar *speaker_abbreviation;

  display_data = sep_get_display_data (app);
  
  /* Find the VU meter in the common area. */
  common_area = sep_get_common_area (app);
  children_list = gtk_container_get_children (GTK_CONTAINER (common_area));
  l1 = children_list;
  while (l1 != NULL)
    {
      child_widget = l1->data;
      child_name = gtk_widget_get_name (child_widget);
      if (g_ascii_strcasecmp (child_name, "global display") == 0)
        {
          grandchildren_list =
	    gtk_container_get_children (GTK_CONTAINER (child_widget));
	  l2 = grandchildren_list;
	  
          while (l2 != NULL)
            {
	      grandchild_widget = l2->data;
              grandchild_name = gtk_widget_get_name (grandchild_widget);
              if (g_ascii_strcasecmp (grandchild_name, "VU_meter") == 0)
                {
                  VU_meter = l2->data;
                  break;
                }
              l2 = l2->next;
            }
          g_list_free (grandchildren_list);
	  grandchildren_list = NULL;
        }

      if (VU_meter != NULL)
        break;
      l1 = l1->next;
    }
  g_list_free (children_list);
  children_list = NULL;

  /* If we didn't find the VU meter, do nothing.  */
  if (VU_meter == NULL)
    return;

  /* Within the VU_meter box is a label and a level bar for each channel.  
   * Set the name of each output channel.  */
  children_list = gtk_container_get_children (GTK_CONTAINER (VU_meter));
  l1 = children_list;

  while (l1 != NULL)
    {
      child_widget = l1->data;
      child_name = gtk_widget_get_name (child_widget);
      channel_number = g_ascii_strtoll (child_name, NULL, 10);
      speaker_abbreviation = sound_output_channel_name (channel_number, app);
      grandchildren_list =
	gtk_container_get_children (GTK_CONTAINER (child_widget));
      l2 = grandchildren_list;
      while (l2 != NULL)
	{
	  grandchild_widget = l2->data;
	  grandchild_name = gtk_widget_get_name (grandchild_widget);
	  if (g_ascii_strcasecmp (grandchild_name, "label") == 0)
	    {
	      if (TRACE_DISPLAY)
		{
		  g_print ("Setting the name of channel %ld to \"%s\".\n",
			   channel_number, speaker_abbreviation);
		}
	      text_label = GTK_LABEL (grandchild_widget);
	      gtk_label_set_text (text_label, speaker_abbreviation);
	    }
	  l2 = l2->next;
	}
      g_list_free (grandchildren_list);
      grandchildren_list = NULL;
      l1 = l1->next;
    }
  g_list_free (children_list);
  children_list = NULL;

  /* Remember where the VU meter is.  */
  display_data->VU_meter = VU_meter;
  
  /* We don't need to do this again.  */
  display_data->initialized = 1;
}

/* Update the VU meter. */
void
display_update_vu_meter (gpointer *user_data, gint channel,
                         gdouble new_value, gdouble peak_dB, gdouble decay_dB)
{
  struct display_info *display_data;
  GList *children_list, *l1;
  GtkWidget *child_widget;
  const gchar *child_name;
  GtkWidget *VU_meter = NULL;
  GtkWidget *this_meter = NULL;
  GtkLevelBar *channel_level_bar = NULL;
  gint64 channel_number;
  gdouble channel_value;
  GApplication *app;

  app = G_APPLICATION (user_data);
  display_data = sep_get_display_data (app);

  /* If we haven't done so already, find the VU meter and set the names
   * of the output channels.  */
  if (display_data->initialized == 0)
    {
      initialize_display_data (app);
    }
  VU_meter = display_data->VU_meter;
  
  /* Find the box for this channel. */
  children_list = gtk_container_get_children (GTK_CONTAINER (VU_meter));
  this_meter = NULL;
  l1 = children_list;
  
  while (l1 != NULL)
    {
      child_widget = l1->data;
      child_name = gtk_widget_get_name (child_widget);
      channel_number = g_ascii_strtoll (child_name, NULL, 10);
      if (channel_number == channel)
	{
	  this_meter = child_widget;
	  break;
	}
      l1 = l1->next;
    }
  g_list_free (children_list);
  children_list = NULL;
  
  if (this_meter == NULL)
    return;

  /* Now find the level bar within the box for this output channel.  */
  children_list = gtk_container_get_children (GTK_CONTAINER (this_meter));
  channel_level_bar = NULL;
  l1 = children_list;
  
  while (l1 != NULL)
    {
      child_widget = l1->data;
      child_name = gtk_widget_get_name (child_widget);
      if (g_ascii_strcasecmp (child_name, "LEVEL_BAR") == 0)
	{
	  channel_level_bar = GTK_LEVEL_BAR (child_widget);
	  break;
	}
      l1 = l1->next;
    }
  g_list_free (children_list);
  children_list = NULL;
  
  if (channel_level_bar == NULL)
    return;
  	  
  /* Set the value of the level bar, between 0 and 1.  
   * Due to what appears to be a bug in level_bar, do not set it very small.  
   */
  channel_value = new_value;
  if (channel_value < 0.01)
    channel_value = 0.01;

  if (TRACE_DISPLAY)
    {
      g_print ("VU meter %d set to %f.\n", channel, channel_value);
    }
  gtk_level_bar_set_value (channel_level_bar, channel_value);

  return;
}

/* Show the user a message.  The return value is a message ID, which
 * can be used to remove the message.  */
guint
display_show_message (gchar * message_text, GApplication *app)
{
  GtkStatusbar *status_bar;
  guint context_id;
  guint message_id;

  /* Find the GUI's status display area.  */
  status_bar = sep_get_status_bar (app);

  /* Use the regular context for messages.  */
  context_id = sep_get_context_id (app);

  /* Show the message.  */
  message_id = gtk_statusbar_push (status_bar, context_id, message_text);

  return message_id;
}

/* Remove a previously-displayed message.  */
void
display_remove_message (guint message_id, GApplication *app)
{
  GtkStatusbar *status_bar;
  guint context_id;

  /* Find the GUI's status display area.  */
  status_bar = sep_get_status_bar (app);

  /* Get the message context ID.  */
  context_id = sep_get_context_id (app);

  /* Remove the specified message.  */
  gtk_statusbar_remove (status_bar, context_id, message_id);

  return;
}

/* Display a message to the operator.  */
void
display_set_operator_text (gchar *text_to_display, GApplication *app)
{
  GtkLabel *text_label;

  /* Find the GUI's operator text area.  */
  text_label = sep_get_operator_text (app);

  /* Set the text.  */
  gtk_label_set_text (text_label, text_to_display);

  return;
}

/* Erase any operator message.  */
void
display_clear_operator_text (GApplication *app)
{
  GtkLabel *text_label;

  /*Find the GUI's operator text area.  */
  text_label = sep_get_operator_text (app);

  /* Clear the text.  */
  gtk_label_set_text (text_label, (gchar *) "");

  return;
}

/* Update the current activity information.  */
void
display_current_activity (gchar * activity_text, GApplication *app)
{
  GtkLabel *activity_label;
  GtkWidget *common_area;
  GtkWidget *child_widget;
  GtkWidget *grandchild_widget;
  GList *children_list, *l1;
  GList *grandchildren_list, *l2;
  const gchar *child_name;
  const gchar *grandchild_name;

  common_area = sep_get_common_area (app);

  /* Find the activity information in the common area. */
  activity_label = NULL;
  children_list = gtk_container_get_children (GTK_CONTAINER (common_area));
  l1 = children_list;
  while (l1 != NULL)
    {
      child_widget = l1->data;
      child_name = gtk_widget_get_name (child_widget);
      if (g_ascii_strcasecmp (child_name, "global display") == 0)
        {
          grandchildren_list =
            gtk_container_get_children (GTK_CONTAINER (child_widget));
	  l2 = grandchildren_list;
	  
          while (l2 != NULL)
            {
	      grandchild_widget = l2->data;
              grandchild_name =
                gtk_widget_get_name (grandchild_widget);
              if (g_ascii_strcasecmp (grandchild_name, "activity") == 0)
                {
                  activity_label = GTK_LABEL (grandchild_widget);
                  break;
                }
              l2 = l2->next;
            }
          g_list_free (grandchildren_list);
        }

      if (activity_label != NULL)
        break;
      l1 = l1->next;
    }
  g_list_free (children_list);

  if (activity_label == NULL)
    return;

  gtk_label_set_text (activity_label, activity_text);
  return;

}

/* End of file display_subroutines.c  */
