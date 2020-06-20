/*
 * button_subroutines.c
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
#include "button_subroutines.h"
#include "gstreamer_subroutines.h"
#include "sound_effects_player.h"
#include "sound_subroutines.h"
#include "sound_structure.h"
#include "sequence_subroutines.h"

#define BUTTON_TRACE FALSE

/* The Mute button has been toggled.  */
void
button_mute_toggled (GtkToggleButton *button, gpointer user_data)
{
  GApplication *app;
  gboolean button_state;
  GstPipeline *pipeline_element;
  GstElement *volume_element;
  GstElement *final_bin_element;

  app = sep_get_application_from_widget (user_data);
  button_state = gtk_toggle_button_get_active (button);
  pipeline_element = sep_get_pipeline_from_app (app);
  if (pipeline_element == NULL)
    return;

  /* Find the final bin.  */
  final_bin_element =
    gst_bin_get_by_name (GST_BIN (pipeline_element), (gchar *) "final");
  if (final_bin_element == NULL)
    return;

  /* Find the volume element in the final bin */
  volume_element = gstreamer_get_volume (GST_BIN (final_bin_element));
  if (volume_element == NULL)
    return;

  /* Set the mute property of the volume element based on whether
   * the mute button has been activated or deactivated.  */
  g_object_set (volume_element, "mute", button_state, NULL);

  return;
}

/* The master volume slider has been moved.  Update the volume and the display. 
 * The user data is the widget being controlled. */
void
button_master_volume_changed (GtkButton *button, gpointer user_data)
{
  GApplication *app;
  GstPipeline *pipeline_element;
  GstElement *volume_element;
  GstElement *final_bin_element;
  GtkLabel *volume_label;
  gdouble new_value;
  gchar *value_string;
  GtkWidget *parent_container;
  GList *children_list = NULL;
  GList *l;
  const gchar *child_name = NULL;

  app = sep_get_application_from_widget (user_data);
  pipeline_element = sep_get_pipeline_from_app (app);

  /* If we don't have a Gstreamer pipeline, do nothing.  */
  if (pipeline_element == NULL)
    return;

  /* Find the final bin.  If we don't have one, do nothing.  */
  final_bin_element =
    gst_bin_get_by_name (GST_BIN (pipeline_element), (gchar *) "final");
  if (final_bin_element == NULL)
    return;

  /* Find the volume element in the final bin.  If it isn't there,
   * do nothing.  */
  volume_element = gstreamer_get_volume (GST_BIN (final_bin_element));
  if (volume_element == NULL)
    return;

  /* Find the volume label associated with this volume widget.
   * It will be a child of this widget's parent. */
  parent_container = gtk_widget_get_parent (GTK_WIDGET (button));
  children_list =
    gtk_container_get_children (GTK_CONTAINER (parent_container));
  l = children_list;
  volume_label = NULL;
  while (l != NULL)
    {
      child_name = gtk_widget_get_name (l->data);
      if (g_strcmp0 (child_name, "volume_label") == 0)
        {
          volume_label = l->data;
          break;
        }
      l = l->next;
    }
  g_list_free (children_list);

  /* If there is no volume label, do nothing.  */
  if (volume_label == NULL)
    return;

  new_value = gtk_scale_button_get_value (GTK_SCALE_BUTTON (button));
  /* Set the volume of the sound. */
  g_object_set (volume_element, "volume", new_value, NULL);

  /* Update the text in the volume label. */
  value_string = g_strdup_printf ("Vol%4.0f%%", new_value * 100.0);
  gtk_label_set_text (volume_label, value_string);
  g_free (value_string);

  return;
}

/* The Pause button has been pushed.  */
void
button_pause_clicked (GtkButton *button, gpointer user_data)
{
  GApplication *app;

  app = sep_get_application_from_widget (user_data);
  sound_button_pause (app);

  return;
}

/* The Continue button has been pushed.  */
void
button_continue_clicked (GtkButton *button, gpointer user_data)
{
  GApplication *app;

  app = sep_get_application_from_widget (user_data);
  sound_button_continue (app);

  return;
}

/* The Play button has been pushed.  */
void
button_play_clicked (GtkButton *button, gpointer user_data)
{
  GApplication *app;

  /* Let the internal sequencer handle it.  */
  app = sep_get_application_from_widget (user_data);
  sequence_button_play (app);

  return;
}

/* The Start button in a cluster has been pushed.  */
void
button_start_clicked (GtkButton *button, gpointer user_data)
{
  GApplication *app;
  GtkWidget *cluster_widget;
  guint cluster_number;

  /* Let the internal sequencer handle it.  */
  app = sep_get_application_from_widget (user_data);
  cluster_widget = sep_get_cluster_from_widget (user_data);
  cluster_number = sep_get_cluster_number (cluster_widget);
  sequence_cluster_start (cluster_number, app);

  return;
}

/* The Stop button in a cluster has been pushed.  */
void
button_stop_clicked (GtkButton *button, gpointer user_data)
{
  GApplication *app;
  GtkWidget *cluster_widget;
  guint cluster_number;

  /* Let the internal sequencer handle it.  */
  app = sep_get_application_from_widget (user_data);
  cluster_widget = sep_get_cluster_from_widget (user_data);
  cluster_number = sep_get_cluster_number (cluster_widget);
  sequence_cluster_stop (cluster_number, app);

  return;
}

/* Show that the Start button has been pushed.  */
void
button_set_cluster_playing (struct sound_info *sound_data, GApplication *app)
{
  GtkButton *start_button;
  GtkLabel *volume_label, *pan_label;
  GstElement *volume_element, *pan_element;
  gdouble volume_level_value, pan_value;
  GstBin *bin_element;
  GtkScaleButton *volume_button, *pan_button;
  GtkWidget *parent_container;
  GList *children_list, *grandchildren_list;
  GList *l, *ll;
  const gchar *child_name, *grandchild_name;

  if (BUTTON_TRACE)
    {
      g_print ("Set sound %s to playing.\n", sound_data->name);
    }

  /* Find the start button and set its text to "Playing...". 
   * The start button will be a child of the cluster, and will be named
   * "start_button".  */
  parent_container = sound_data->cluster_widget;

  /* It is possible, though unlikely, that the sound will no longer
   * be in a cluster.  */
  if (parent_container == NULL)
    return;
    
  start_button = NULL;
  pan_label = NULL;
  volume_label = NULL;
  pan_button = NULL;
  volume_button = NULL;
  children_list = gtk_container_get_children (GTK_CONTAINER (parent_container));
  l = children_list;
  while (l != NULL)
    {
      child_name = gtk_widget_get_name (l->data);
      if (BUTTON_TRACE)
	g_print (" child name is %s\n", child_name);
      if (GTK_IS_CONTAINER (l->data))
	{
	  if (BUTTON_TRACE)
	    g_print (" container\n");
	  grandchildren_list = gtk_container_get_children (l->data);
	  ll = grandchildren_list;
	  while (ll != NULL)
	    {
	      grandchild_name = gtk_widget_get_name (ll->data);
	      if (BUTTON_TRACE)
		g_print (" grandchild name is %s\n", grandchild_name);
	      if (g_strcmp0 (grandchild_name, "pan_label") == 0)
		pan_label = ll->data;
	      if (g_strcmp0 (grandchild_name, "pan_button") == 0)
		pan_button = ll->data;
	      if (g_strcmp0 (grandchild_name, "volume_label") == 0)
		volume_label = ll->data;
	      if (g_strcmp0 (grandchild_name, "volume") == 0)
		volume_button = ll->data;
	      ll = ll->next;
	    }
	  g_list_free (grandchildren_list);
	  grandchildren_list = NULL;
	}
      if (g_strcmp0 (child_name, "start_button") == 0)
	  start_button = l->data;
      l = l->next;
    }
  g_list_free (children_list);
  children_list = NULL;
    
  if (BUTTON_TRACE)
    {
      if (start_button != NULL)
	g_print (" found start button\n");
      if (pan_label != NULL)
	g_print (" found pan label\n");
      if (pan_button != NULL)
	g_print (" found pan button\n");
      if (volume_label != NULL)
	g_print (" found volume label\n");
      if (volume_button != NULL)
	g_print (" found volume button\n");
    }
  
  if (start_button != NULL)
    gtk_button_set_label (start_button, "Playing...");
  
  /* The sound_effect structure records where the Gstreamer bin is
   * for this sound effect.  That bin contains the volume and pan
   * controls. */
  bin_element = sound_data->sound_control;
  volume_element = gstreamer_get_volume (bin_element);
  if ((volume_element != NULL) && (volume_button != NULL))
    {
      volume_level_value = sound_data->default_volume_level;
      if (BUTTON_TRACE)
	{
	  g_print (" set volume to %4.3f.\n", volume_level_value);
	}
      gtk_scale_button_set_value (volume_button, volume_level_value);
    }
  
  pan_element = gstreamer_get_pan (bin_element);
  if ((pan_element != NULL) && (pan_button != NULL))
    {
      pan_value = sound_data->designer_pan;
      /* -1.0 is full left, 0.0 is center, 1.0 is full right.
       * convert to 0.0 to 100.0  */
      pan_value = (pan_value * 50.0) + 50.0;
  
      if (BUTTON_TRACE)
	{
	  g_print (" set pan to %4.0f.\n", pan_value);
	}
      gtk_scale_button_set_value (pan_button, pan_value);
    }

  return;
}

/* Show that the release stage of a sound is running.  */
void
button_set_cluster_releasing (struct sound_info *sound_data,
                              GApplication *app)
{
  GtkButton *start_button;
  GtkWidget *parent_container;
  GList *children_list;
  GList *l;
  const gchar *child_name;

    if (BUTTON_TRACE)
    {
      g_print ("Set sound %s to releasing.\n", sound_data->name);
    }

  /* Find the start button and set its text to "Releasing...". 
   * The start button will be a child of the cluster, and will be named
   * "start_button".  */
  parent_container = sound_data->cluster_widget;

  /* It is possible that the sound will no longer be in a cluster.  */
  if (parent_container != NULL)
    {
      start_button = NULL;
      children_list =
        gtk_container_get_children (GTK_CONTAINER (parent_container));
      l = children_list;
      while (l != NULL)
        {
          child_name = gtk_widget_get_name (l->data);
          if (g_strcmp0 (child_name, "start_button") == 0)
            {
              start_button = l->data;
              break;
            }
          l = l->next;
        }
      g_list_free (children_list);
      if (start_button != NULL)
	gtk_button_set_label (start_button, "Releasing...");
    }

  return;
}

/* Reset the appearance of a cluster after its sound has finished playing. */
void
button_reset_cluster (struct sound_info *sound_data, GApplication *app)
{
  GtkButton *start_button;
  GtkWidget *parent_container;
  GList *children_list;
  GList *l;
  const gchar *child_name;

  if (BUTTON_TRACE)
    {
      g_print ("Set sound %s to initial appearance.\n", sound_data->name);
    }

  /* Find the start button and set its text back to "Start". 
   * The start button will be a child of the cluster, and will be named
   * "start_button".  */
  parent_container = sound_data->cluster_widget;

  /* It is possible that the sound will no longer be in a cluster.  */
  if (parent_container != NULL)
    {
      start_button = NULL;
      children_list =
        gtk_container_get_children (GTK_CONTAINER (parent_container));
      l = children_list;
      while (l != NULL)
        {
          child_name = gtk_widget_get_name (l->data);
          if (g_strcmp0 (child_name, "start_button") == 0)
            {
              start_button = l->data;
              break;
            }
          l = l->next;
        }
      g_list_free (children_list);
      if (start_button != NULL)
	gtk_button_set_label (start_button, "Start");
    }

  return;
}

/* The volume slider has been moved.  Update the volume and the display. 
 * The user data is the widget being controlled. */
void
button_volume_changed (GtkButton *button, gpointer user_data)
{
  GtkLabel *volume_label = NULL;
  GtkWidget *parent_container;
  GList *children_list = NULL;
  GList *l;
  const gchar *child_name = NULL;
  struct sound_info *sound_data;
  GstBin *bin_element;
  GstElement *volume_element;
  gdouble old_value, new_value;
  gchar *value_string;

  if (BUTTON_TRACE)
    {
      g_print ("Volume slider changed.\n");
    }

  /* Find the volume label associated with this volume widget.
   * It will be a child of this widget's parent. */
  volume_label = NULL;
  parent_container = gtk_widget_get_parent (GTK_WIDGET (button));
  children_list =
    gtk_container_get_children (GTK_CONTAINER (parent_container));
  l = children_list;
  while (l != NULL)
    {
      child_name = gtk_widget_get_name (l->data);
      if (g_strcmp0 (child_name, "volume_label") == 0)
        {
          volume_label = l->data;
          break;
        }
      l = l->next;
    }
  g_list_free (children_list);

  if (volume_label != NULL)
    {
      /* There should be a sound effect associated with this cluster.
       * If there isn't, do nothing. */
      sound_data = sep_get_sound_effect (user_data);
      if (sound_data == NULL)
        return;

      /* The sound_effect structure records where the Gstreamer bin is
       * for this sound effect.  That bin contains the volume control.
       */
      bin_element = sound_data->sound_control;
      volume_element = gstreamer_get_volume (bin_element);
      if (volume_element == NULL)
        return;

      new_value = gtk_scale_button_get_value (GTK_SCALE_BUTTON (button));
      if (BUTTON_TRACE)
	{
	  g_print ("Raw value for volume slider is %4.3f.\n", new_value);
	}
      g_object_get (volume_element, "volume", &old_value, NULL);
      /* Set the volume of the sound. */
      g_object_set (volume_element, "volume", new_value, NULL);

      /* Update the text in the volume label. */
      value_string = g_strdup_printf ("Vol %4.0f%%", new_value * 100.0);
      gtk_label_set_text (volume_label, value_string);
      if (BUTTON_TRACE)
	{
	  g_print ("Volume of %s changed: %s.\n",
		   sound_data->name, value_string);
	}
      g_free (value_string);

    }

  return;
}

/* The pan slider has been moved.  Update the pan and display. */
void
button_pan_changed (GtkButton *button, gpointer user_data)
{
  GtkLabel *pan_label;
  GtkWidget *parent_container;
  GList *children_list;
  GList *l;
  const gchar *child_name;
  struct sound_info *sound_data;
  GstBin *bin_element;
  GstElement *pan_element;
  gdouble old_value, new_value;
  gchar *value_string;

  if (BUTTON_TRACE)
    {
      g_print ("Pan slider changed.\n");
    }

  /* Find the pan label associated with this pan widget.
   * It will be a child of this widget's parent. */
  parent_container = gtk_widget_get_parent (GTK_WIDGET (button));
  pan_label = NULL;
  children_list =
    gtk_container_get_children (GTK_CONTAINER (parent_container));
  l = children_list;
  while (l != NULL)
    {
      child_name = gtk_widget_get_name (l->data);
      if (g_strcmp0 (child_name, "pan_label") == 0)
        {
          pan_label = l->data;
          break;
        }
      l = l->next;
    }
  g_list_free (children_list);

  if (pan_label != NULL)
    {
      /* There should be a sound effect associated with this cluster.
       * If there isn't, do nothing. */
      sound_data = sep_get_sound_effect (user_data);
      if (sound_data == NULL)
        return;

      /* The sound_effect structure records where the Gstreamer bin is
       * for this sound effect.  That bin contains the pan control.
       */
      bin_element = sound_data->sound_control;
      pan_element = gstreamer_get_pan (bin_element);
      /* The pan control may have been omitted by the sound designer.  */
      if (pan_element == NULL)
        return;

      new_value = gtk_scale_button_get_value (GTK_SCALE_BUTTON (button));
      if (BUTTON_TRACE)
	{
	  g_print ("Raw value from pan button is %4.3f.\n", new_value);
	}
      new_value = (new_value - 50.0) / 50.0;
      g_object_get (pan_element, "panorama", &old_value, NULL);
      /* Set the panorama position of the sound. */
      g_object_set (pan_element, "panorama", new_value, NULL);

      /* Update the text of the pan label.  0.0 corresponds to Center, 
       * negative numbers to left, and positive numbers to right. */
      if (new_value == 0.0)
        gtk_label_set_text (pan_label, "Center");
      else
        {
          if (new_value < 0.0)
            value_string =
              g_strdup_printf ("Left %4.0f%%", -(new_value * 100.0));
          else
            value_string =
              g_strdup_printf ("Right%4.0f%%", new_value * 100.0);
          gtk_label_set_text (pan_label, value_string);
          g_free (value_string);
        }
      if (BUTTON_TRACE)
	{
	  g_print ("Pan value of %s changed from %4.0f to %4.0f.\n",
		   sound_data->name, old_value, new_value);
	}
    }

  return;
}
