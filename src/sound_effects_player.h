/*
 * sound_effects_player.h
 *
 * Copyright © 2020 by John Sauter <John_Sauter@systemeyescomputerstore.com>
 * 
 * Sound_effects_player is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Sound_effects_player is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SOUND_EFFECTS_PLAYER_H_
#define _SOUND_EFFECTS_PLAYER_H_

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

G_BEGIN_DECLS
#define SOUND_EFFECTS_PLAYER_TYPE_APPLICATION (sound_effects_player_get_type ())
#define SOUND_EFFECTS_PLAYER_APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUND_EFFECTS_PLAYER_TYPE_APPLICATION, Sound_Effects_Player))
#define SOUND_EFFECTS_PLAYER_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SOUND_EFFECTS_PLAYER_TYPE_APPLICATION, Sound_Effects_PlayerClass))
#define SOUND_EFFECTS_PLAYER_IS_APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUND_EFFECTS_PLAYER_TYPE_APPLICATION))
#define SOUND_EFFECTS_PLAYER_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUND_EFFECTS_PLAYER_TYPE_APPLICATION))
#define SOUND_EFFECTS_PLAYER_APPLICATION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUND_EFFECTS_PLAYER_TYPE_APPLICATION, Sound_Effects_PlayerClass))
typedef struct _Sound_Effects_PlayerClass Sound_Effects_PlayerClass;
typedef struct _Sound_Effects_Player Sound_Effects_Player;
typedef struct _Sound_Effects_PlayerPrivate Sound_Effects_PlayerPrivate;

struct _Sound_Effects_PlayerClass
{
  GtkApplicationClass parent_class;
};

struct _Sound_Effects_Player
{
  GtkApplication parent_instance;

  Sound_Effects_PlayerPrivate *priv;

};

GType
sound_effects_player_get_type (void)
  G_GNUC_CONST;
     Sound_Effects_Player *sound_effects_player_new (void);

/* Callbacks */

/* The gstreamer pipeline has completed initialization; we can show
 * the top-level window now.  */
void sep_gstreamer_ready (GApplication *app);

/* Create the gstreamer pipeline by reading an XML file.  */
void sep_create_pipeline (gchar *filename, GApplication *app);

/* Find the gstreamer pipeline.  */
GstPipeline *sep_get_pipeline_from_app (GApplication *app);

/* Given a widget, get the app.  */
GApplication *sep_get_application_from_widget (GtkWidget *object);

/* Given a widget within a cluster, find the cluster.  */
GtkWidget *sep_get_cluster_from_widget (GtkWidget *the_widget);

/* Given a widget in a cluster, get its sound_effect structure. */
struct sound_info *sep_get_sound_effect (GtkWidget *object);

/* Given a cluster number, get the cluster.  */
GtkWidget *sep_get_cluster_from_number (guint cluster_number,
                                             GApplication *app);

/* Given a cluster, get its number.  */
guint sep_get_cluster_number (GtkWidget *cluster_widget);

/* Find the common area above the clusters. */
GtkWidget *sep_get_common_area (GApplication *app);

/* Find the network information. */
void *sep_get_network_data (GApplication *app);

/* Find the trace information.  */
void *sep_get_trace_data (GApplication *app);

/* Find the network messages parser information. */
void *sep_get_parse_net_data (GApplication *app);

/* Find the top-level window. */
GtkWindow *sep_get_top_window (GApplication *app);

/* Find the operator text label widget.  */
GtkLabel *sep_get_operator_text (GApplication *app);

/* Find the status bar.  */
GtkStatusbar *sep_get_status_bar (GApplication *app);

/* Find the context ID.  */
guint sep_get_context_id (GApplication *app);

/* Find the configuration file. */
xmlDocPtr sep_get_configuration_file (GApplication *app);

/* Remember the configuration file. */
void sep_set_configuration_file (xmlDocPtr configuration_file,
				 GApplication *app);

/* Find the name of the configuration file. */
gchar *sep_get_configuration_filename (GApplication *app);

/* Remember the name of the configuration file. */
void sep_set_configuration_filename (gchar *configuration_filename,
				     GApplication *app);

/* Find the name of the file from which the network port was set.  */
gchar *sep_get_network_port_filename (GApplication *app);

/* Set the name of the file from which the network port was set.  */
void sep_set_network_port_filename (gchar *network_port_filename,
				    GApplication *app);

/* Find the name of the file from which the speaker count was set.  */
gchar *sep_get_speaker_count_filename (GApplication *app);

/* Set the name of the file from which the speaker count was set.  */
void sep_set_speaker_count_filename (gchar *speaker_count_filename,
				    GApplication *app);

/* Set the speaker count.  */
void sep_set_speaker_count (gint64 speaker_count, GApplication *app);

/* Get the speaker count.  */
gint64 sep_get_speaker_count (GApplication *app);

/* Find the folder of the project file.  */
gchar *sep_get_project_folder_name (GApplication *app);

/* Find the name of the project file.  */
gchar *sep_get_project_file_name (GApplication *app);

/* Remember the folder of the project file.  */
void sep_set_project_folder_name (gchar * project_folder_name,
				  GApplication *app);

/* Remember the name of the project file.  */
void sep_set_project_file_name (gchar * project_file_name,
				GApplication *app);

/* Find the path to the user interface files. */
gchar *sep_get_ui_path (GApplication *app);

/* Find the persistent data for the sound subroutines.  */
void *sep_get_sounds_data (GApplication *app);

/* Find the persistent data for the display subroutines.  */
void *sep_get_display_data (GApplication *app);

/* Find the sequence information.  */
void *sep_get_sequence_data (GApplication *app);

/* Find the signal handler information.  */
void *sep_get_signal_data (GApplication *app);

/* Find the timer information.  */
void *sep_get_timer_data (GApplication *app);

G_END_DECLS
#endif /* _SOUND_EFFECTS_PLAYER_H_ */
