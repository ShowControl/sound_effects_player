/*
 * sound_effects_player.c
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
#include <glib/gi18n.h>
#include <libxml/xmlmemory.h>
#include "display_subroutines.h"
#include "gstreamer_subroutines.h"
#include "main.h"
#include "menu_subroutines.h"
#include "network_subroutines.h"
#include "parse_net_subroutines.h"
#include "parse_xml_subroutines.h"
#include "sequence_subroutines.h"
#include "signal_subroutines.h"
#include "sound_effects_player.h"
#include "sound_structure.h"
#include "sound_subroutines.h"
#include "timer_subroutines.h"
#include "time_subroutines.h"
#include "trace_subroutines.h"

/* The private data associated with the top-level window. */
struct _Sound_Effects_PlayerPrivate
{
  /* The Gstreamer pipeline. */
  GstPipeline *gstreamer_pipeline;

  /* A flag that is set by the Gstreamer startup process when it is
   * complete.  */
  gboolean gstreamer_ready;

  /* The top-level gtk window. */
  GtkWindow *top_window;

  /* A flag that is set to indicate that we have told GTK to start
   * showing the top-level window.  */
  gboolean windows_showing;

  /* The common area, needed for updating the display asynchronously. */
  GtkWidget *common_area;

  /* The place where messages to the operator are shown.  */
  GtkLabel *operator_text;

  /* The status bar and context ID, used for showing messages.  */
  GtkStatusbar *status_bar;
  guint context_id;

  /* The list of sounds we can make.  Each item of the GList points
   * to a sound_info structure. */
  GList *sound_list;

  /* The persistent information for the internal sequencer.  */
  void *sequence_data;

  /* The persistent information for the signal handler.  */
  void *signal_data;

  /* The persistent information for the timer.  */
  void *timer_data;

  /* The list of clusters that might contain sound effects. */
  GList *clusters;

  /* The persistent network information. */
  void *network_data;

  /* The persistent information for the network commands parser. */
  void *parse_net_data;

  /* The XML file that holds parameters for the program. */
  xmlDocPtr configuration_file;

  /* The name of that file, for use in Save and as the default file
   * name for Save As. */
  gchar *configuration_filename;

  /* The name of the file from which the network port number was set,
   * for warning messages about duplicate settings.  */
  gchar *network_port_filename;

  /* The folder that holds the project file.  */
  gchar *project_folder_name;

  /* The file within that folder that holds the project.  */
  gchar *project_file_name;

  /* The path to the user interface files. */
  gchar *ui_path;

  /* The persistent information for the trace subroutines.  */
  void *trace_data;

};

G_DEFINE_TYPE_WITH_PRIVATE (Sound_Effects_Player, sound_effects_player,
                            GTK_TYPE_APPLICATION);

/* Create a new window loading a file. */
static void
sound_effects_player_new_window (GApplication * app, GFile * file)
{
  GtkWindow *top_window;
  GtkWidget *common_area;
  GtkLabel *operator_text;
  GtkStatusbar *status_bar;
  guint context_id;
  GtkBuilder *builder;
  GError *error = NULL;
  gint cluster_number;
  gchar *cluster_name;
  GtkWidget *cluster_widget;
  gchar *filename;
  guint message_code;
  GFile *parent_file;

  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  /* Initialize the trace subroutines.  */
  priv->trace_data = trace_init (app);

  /* Initialize the signal handler.  */
  priv->signal_data = signal_init (app);

  /* Initialize the timer.  */
  priv->timer_data = timer_init (app);

  /* Remember the path to the user interface files. */
  priv->ui_path = g_strdup (PACKAGE_DATA_DIR "/ui/");

  /* Load the main user interface definition from its file. */
  builder = gtk_builder_new ();
  filename =
    g_build_filename (priv->ui_path, "sound_effects_player.ui", NULL);
  if (!gtk_builder_add_from_file (builder, filename, &error))
    {
      g_critical ("Couldn't load builder file %s: %s", filename,
                  error->message);
      g_error_free (error);
    }

  /* Auto-connect signal handlers. */
  gtk_builder_connect_signals (builder, app);

  /* Get the top-level window object from the user interface file. */
  top_window =
    GTK_WINDOW (gtk_builder_get_object (builder, "top_level_window"));
  priv->top_window = top_window;
  if (top_window == NULL)
    {
      g_critical ("Widget \"top_level_window\" is missing in file %s.",
                  filename);
    }

  /* Also get the common area, operator text and status bar. */
  common_area = GTK_WIDGET (gtk_builder_get_object (builder, "common_area"));
  priv->common_area = common_area;
  if (common_area == NULL)
    {
      g_critical ("Widget \"common_area\" is missing in file %s.", filename);
    }

  operator_text =
    GTK_LABEL (gtk_builder_get_object (builder, "operator_text"));
  priv->operator_text = operator_text;
  if (operator_text == NULL)
    {
      g_critical ("Operator text is missing in file %s.", filename);
    }

  status_bar = GTK_STATUSBAR (gtk_builder_get_object (builder, "status_bar"));
  priv->status_bar = status_bar;
  if (status_bar == NULL)
    {
      g_critical ("Status bar is missing in file %s.", filename);
    }

  /* Generate a context ID for the status bar.  */
  context_id =
    gtk_statusbar_get_context_id (status_bar, (gchar *) "messages");
  priv->context_id = context_id;

  /* We are done with the name of the user interface file. */
  g_free (filename);

  /* Remember where the clusters are. Each cluster has a name identifying it. */
  priv->clusters = NULL;
  for (cluster_number = 0; cluster_number < 16; ++cluster_number)
    {
      cluster_name = g_strdup_printf ("cluster_%2.2d", cluster_number);
      cluster_widget =
        GTK_WIDGET (gtk_builder_get_object (builder, cluster_name));
      if (cluster_widget != NULL)
        {
          priv->clusters = g_list_prepend (priv->clusters, cluster_widget);
        }
      g_free (cluster_name);
    }

  /* ANJUTA: Widgets initialization for sound_effects_player.ui 
   * - DO NOT REMOVE */

  g_object_unref (builder);

  gtk_window_set_application (top_window, GTK_APPLICATION (app));

  /* Set up the menu. */
  filename = g_build_filename (priv->ui_path, "app-menu.ui", NULL);
  menu_init (app, filename);
  g_free (filename);

  /* Set up the remainder of the private data. */
  priv->gstreamer_pipeline = NULL;
  priv->gstreamer_ready = FALSE;
  priv->sound_list = NULL;

  /* Initialize the internal sequencer.  */
  priv->sequence_data = sequence_init (app);

  /* Initialize the network message parser. */
  priv->parse_net_data = parse_net_init (app);

  /* Initialize the network handler so we can set the network port number. */
  priv->network_data = network_init (app);

  /* The display is initialized; time to show it. */
  gtk_widget_show_all (GTK_WIDGET (top_window));
  priv->windows_showing = TRUE;

  /* Read the configuration file, which will contain the preferences
   * and the default project file.  If the configuration file has not
   * been specified on the command line, read the default configuration
   * file.  If it does not exist, create a default configuration file.  */
  priv->configuration_filename =
    g_strdup (main_get_configuration_file_name ());
  if (priv->configuration_filename == NULL)
    {
      priv->configuration_filename =
        g_build_filename (g_get_user_config_dir (), (gchar *) "ShowControl",
                          (gchar *) "ShowControl_config.xml", NULL);
    }
  parse_xml_read_configuration_file (priv->configuration_filename, app);

  /* If the invocation of sound_effects_player included a parameter,
   * that parameter is the name of the project file to load before
   * starting the user interface, overriding the value in the
   * configuration file.  */
  if (file != NULL)
    {
      parent_file = g_file_get_parent (file);
      priv->project_folder_name = g_file_get_path (parent_file);
      priv->project_file_name = g_file_get_basename (file);

      /* Write back the configuration file, updated with the new
       * project folder and file names.  */
      parse_xml_write_configuration_file (priv->configuration_filename, app);
      g_object_unref (parent_file);
      parent_file = NULL;
    }

  /* If we have a parameter, it is the project XML file to read 
   * for our sounds.  If we don't, read the most recent project file,
   * whose folder and name were saved in the configuration file.
   * If there is no project file the user will have to read one
   * using the menu.  */
  if (priv->project_file_name != NULL)
    {
      message_code = display_show_message ("Loading...", app);
      parse_xml_read_project_file (priv->project_folder_name,
                                   priv->project_file_name, app);
      priv->gstreamer_pipeline = sound_init (app);
      display_remove_message (message_code, app);
      if (priv->gstreamer_pipeline != NULL)
        {
          message_code = display_show_message ("Starting...", app);
        }
      else
        {
          message_code = display_show_message ("Failed.", app);
        }
    }
  else
    {
      message_code = display_show_message ("No sounds.", app);
    }

  /* Listen for network messages, since the network port is
   * now determined.  */
  network_bind_port (app);

  return;
}

/* GApplication implementation */
static void
sound_effects_player_activate (GApplication * application)
{
  /* Some high security environments disable the adjtimex function,
   * even when it is just fetching information.  
   * The adjtimex function is the only way to determine that
   * there is a leap second in progress, so do not run if it
   * doesn't work.  */
  if (time_test_for_disabled_adjtimex() == 1)
    {
      g_critical ("\n"
		  "The current time will not be correct during a leap second\n"
		  "because the Linux adjtimex function is not working.\n"
		  "See Fedora Bugzilla 1778298.\n");
      exit (EXIT_FAILURE);
    }

  sound_effects_player_new_window (application, NULL);
}

static void
sound_effects_player_open (GApplication * application, GFile ** files,
                           gint n_files, const gchar * hint)
{
  gint i;

  /* Some high security environments disable the adjtimex function,
   * even when it is just fetching information.  
   * The adjtimex function is the only way to determine that
   * there is a leap second in progress, so do not run if it
   * doesn't work.  */
  if (time_test_for_disabled_adjtimex() == 1)
    {
      g_critical ("\n"
		  "The current time will not be correct during a leap second\n"
		  "because the Linux adjtimex function is not working.\n"
		  "See Fedora Bugzilla 1778298.\n");
      exit (EXIT_FAILURE);
    }

  for (i = 0; i < n_files; i++)
    sound_effects_player_new_window (application, files[i]);
}

static void
sound_effects_player_init (Sound_Effects_Player * object)
{
  object->priv = sound_effects_player_get_instance_private (object);
}

static void
sound_effects_player_dispose (GObject * object)
{
  GApplication *app = (GApplication *) object;
  Sound_Effects_Player *self = (Sound_Effects_Player *) object;
  GList *sound_effect_list;
  GList *next_sound_effect;
  struct sound_info *sound_effect;

  /* Deallocate the gstreamer pipeline.  */
  if (self->priv->gstreamer_pipeline != NULL)
    {
      self->priv->gstreamer_pipeline = gstreamer_dispose (app);
    }

  /* Deallocate the list of sound effects. */
  sound_effect_list = self->priv->sound_list;

  while (sound_effect_list != NULL)
    {
      sound_effect = sound_effect_list->data;
      next_sound_effect = sound_effect_list->next;
      g_free (sound_effect->name);
      g_free (sound_effect->wav_file_name);
      g_free (sound_effect->wav_file_name_full);
      g_free (sound_effect->function_key);
      g_free (sound_effect);
      self->priv->sound_list =
        g_list_delete_link (self->priv->sound_list, sound_effect_list);
      sound_effect_list = next_sound_effect;
    }

  if (self->priv->project_folder_name != NULL)
    {
      g_free (self->priv->project_folder_name);
      self->priv->project_folder_name = NULL;
    }
  if (self->priv->project_file_name != NULL)
    {
      g_free (self->priv->project_file_name);
      self->priv->project_file_name = NULL;
    }
  if (self->priv->network_port_filename != NULL)
    {
      g_free (self->priv->network_port_filename);
      self->priv->network_port_filename = NULL;
    }

  /* Deallocate the configuration file.  */
  if (self->priv->configuration_file != NULL)
    {
      xmlFree (self->priv->configuration_file);
      self->priv->configuration_file = NULL;
    }

  if (self->priv->configuration_filename != NULL)
    {
      g_free (self->priv->configuration_filename);
      self->priv->configuration_filename = NULL;
    }

  /* Shut down the trace subroutines.  */
  trace_finalize (app);
  self->priv->trace_data = NULL;

  G_OBJECT_CLASS (sound_effects_player_parent_class)->dispose (object);
  return;
}

static void
sound_effects_player_finalize (GObject * object)
{
  G_OBJECT_CLASS (sound_effects_player_parent_class)->finalize (object);
  return;
}

static void
sound_effects_player_class_init (Sound_Effects_PlayerClass * klass)
{
  G_APPLICATION_CLASS (klass)->activate = sound_effects_player_activate;
  G_APPLICATION_CLASS (klass)->open = sound_effects_player_open;
  G_OBJECT_CLASS (klass)->dispose = sound_effects_player_dispose;
  G_OBJECT_CLASS (klass)->finalize = sound_effects_player_finalize;
}

Sound_Effects_Player *
sound_effects_player_new (void)
{
  return g_object_new (sound_effects_player_get_type (), "application-id",
                       "org.gnome.show_control.sound_effects_player", "flags",
                       G_APPLICATION_HANDLES_OPEN, NULL);
}

/* Callbacks from other modules.  The names of the callbacks are prefixed
 * with sep_ rather than sound_effects_player_ for readability. */

/* This is called when the gstreamer pipeline has completed initialization.  */
void
sep_gstreamer_ready (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  priv->gstreamer_ready = TRUE;

  /* If we aren't yet showing the top-level window, show it now.  */
  if (!priv->windows_showing)
    {
      gtk_widget_show_all (GTK_WIDGET (priv->top_window));
      priv->windows_showing = TRUE;
    }

  /* Tell the operator we are running.  */
  display_show_message ("Running.", app);

  /* Start the internal sequencer.  */
  sequence_start (app);
  return;
}

/* Create the gstreamer pipeline by reading an XML file.  */
void
sep_create_pipeline (gchar * filename, GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  gchar *local_filename;

  local_filename = g_strdup (filename);
  parse_xml_read_configuration_file (local_filename, app);
  priv->gstreamer_pipeline = sound_init (app);
  local_filename = NULL;

  return;
}

/* Find the gstreamer pipeline.  */
GstPipeline *
sep_get_pipeline_from_app (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  GstPipeline *pipeline_element;

  pipeline_element = priv->gstreamer_pipeline;

  return (pipeline_element);
}

/* Find the application, given any widget in the application.  */
GApplication *
sep_get_application_from_widget (GtkWidget * object)
{
  GtkWidget *toplevel_widget;
  GtkWindow *toplevel_window;
  GtkApplication *gtk_app;
  GApplication *app;

  /* Find the top-level window.  */
  toplevel_widget = gtk_widget_get_toplevel (object);
  toplevel_window = GTK_WINDOW (toplevel_widget);
  /* The top level window knows where to find the application.  */
  gtk_app = gtk_window_get_application (toplevel_window);
  app = (GApplication *) gtk_app;
  return (app);
}

/* Find the cluster which contains the given widget.  */
GtkWidget *
sep_get_cluster_from_widget (GtkWidget * object)
{
  GtkWidget *this_object;
  const gchar *widget_name;
  GtkWidget *cluster_widget;

  /* Work up from the given widget until we find one whose name starts
   * with "cluster_". */

  cluster_widget = NULL;
  this_object = object;
  do
    {
      widget_name = gtk_widget_get_name (this_object);
      if (g_str_has_prefix (widget_name, "cluster_"))
        {
          cluster_widget = this_object;
          break;
        }
      this_object = gtk_widget_get_parent (this_object);
    }
  while (this_object != NULL);

  return (cluster_widget);
}

/* Find the sound effect information corresponding to a cluster, 
 * given a widget inside that cluster. Return NULL if
 * the cluster is not running a sound effect. */
struct sound_info *
sep_get_sound_effect (GtkWidget * object)
{
  GtkWidget *this_object;
  const gchar *widget_name;
  GtkWidget *cluster_widget = NULL;
  GtkWidget *toplevel_widget;
  GtkWindow *toplevel_window;
  GtkApplication *app;
  Sound_Effects_Player *self;
  Sound_Effects_PlayerPrivate *priv;
  GList *sound_effect_list;
  struct sound_info *sound_effect = NULL;
  gboolean sound_effect_found;

  /* Work up from the given widget until we find one whose name starts
   * with "cluster_". */

  this_object = object;
  do
    {
      widget_name = gtk_widget_get_name (this_object);
      if (g_str_has_prefix (widget_name, "cluster_"))
        {
          cluster_widget = this_object;
          break;
        }
      this_object = gtk_widget_get_parent (this_object);
    }
  while (this_object != NULL);

  /* Find the application's private data, where the sound effects
   * information is kept.  First we find the top-level window, 
   * which has the private data. */
  toplevel_widget = gtk_widget_get_toplevel (object);
  toplevel_window = GTK_WINDOW (toplevel_widget);
  /* Work through the pointer structure to the private data. */
  app = gtk_window_get_application (toplevel_window);
  self = SOUND_EFFECTS_PLAYER_APPLICATION (app);
  priv = self->priv;

  /* Then we search through the sound effects for the one attached
   * to this cluster. */
  sound_effect_list = priv->sound_list;
  sound_effect_found = FALSE;
  while (sound_effect_list != NULL)
    {
      sound_effect = sound_effect_list->data;
      if (sound_effect->cluster_widget == cluster_widget)
        {
          sound_effect_found = TRUE;
          break;
        }
      sound_effect_list = sound_effect_list->next;
    }
  if (sound_effect_found)
    return (sound_effect);

  return NULL;
}

/* Find a cluster, given its number.  */
GtkWidget *
sep_get_cluster_from_number (guint cluster_number, GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  GtkWidget *cluster_widget;
  GList *cluster_list;
  const gchar *widget_name;
  gchar *cluster_name;

  /* Go through the list of clusters looking for one with the name
   * "cluster_" followed by the cluster number.  */
  for (cluster_list = priv->clusters; cluster_list != NULL;
       cluster_list = cluster_list->next)
    {
      cluster_widget = cluster_list->data;
      widget_name = gtk_widget_get_name (cluster_widget);
      cluster_name = g_strdup_printf ("cluster_%2.2d", cluster_number);
      if (g_ascii_strcasecmp (widget_name, cluster_name) == 0)
        {
          g_free (cluster_name);
          return (cluster_widget);
        }
      g_free (cluster_name);
    }

  return NULL;
}

/* Given a cluster, find its cluster number.  */
guint
sep_get_cluster_number (GtkWidget * cluster_widget)
{
  const gchar *widget_name;
  guint cluster_number;
  gint result;

  /* Extract the cluster number from its name.  */
  widget_name = gtk_widget_get_name (cluster_widget);
  result = sscanf (widget_name, "cluster_%u", &cluster_number);
  if (result != 1)
    {
      g_print ("result = %d, cluster number = %d.\n", result, cluster_number);
    }

  return (cluster_number);
}

/* Find the area above the top of the clusters, 
 * so it can be updated.  The parameter passed is the application, which
 * was passed through gstreamer_setup and the gstreamer signaling system
 * as an opaque value.  */
GtkWidget *
sep_get_common_area (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  GtkWidget *common_area;

  common_area = priv->common_area;

  return (common_area);
}

/* Find the network information.  The parameter passed is the application, which
 * was passed through the various gio callbacks as an opaque value.  */
void *
sep_get_network_data (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  void *network_data;

  network_data = priv->network_data;
  return (network_data);
}

/* Find the trace subroutines' persistent data.  */
void *
sep_get_trace_data (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  void *trace_data;

  trace_data = priv->trace_data;
  return (trace_data);
}

/* Find the network commands parser information.  
 * The parameter passed is the application.  */
void *
sep_get_parse_net_data (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  void *parse_net_data;

  parse_net_data = priv->parse_net_data;
  return (parse_net_data);
}

/* Find the top-level window, to use as the transient parent for
 * dialogs. */
GtkWindow *
sep_get_top_window (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  GtkWindow *top_window;

  top_window = priv->top_window;
  return (top_window);
}

/* Find the operator text label widget, which is used to display
 * text from the sequencer to the operator.  */
GtkLabel *
sep_get_operator_text (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  GtkLabel *operator_text;

  operator_text = priv->operator_text;
  return operator_text;
}

/* Find the status bar, which is used for messages.  */
GtkStatusbar *
sep_get_status_bar (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  GtkStatusbar *status_bar;

  status_bar = priv->status_bar;
  return status_bar;
}

/* Find the context ID, which is also needed for messages.  */
guint
sep_get_context_id (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  guint context_id;

  context_id = priv->context_id;
  return context_id;
}

/* Find the configuration file. */
xmlDocPtr
sep_get_configuration_file (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  xmlDocPtr configuration_file;

  configuration_file = priv->configuration_file;
  return (configuration_file);
}

/* Remember the configuration file. */
void
sep_set_configuration_file (xmlDocPtr configuration_file, GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  if (priv->configuration_file != NULL)
    {
      xmlFreeDoc (priv->configuration_file);
      priv->configuration_file = NULL;
    }
  priv->configuration_file = configuration_file;

  return;
}

/* Find the file where the network port was set. */
gchar *
sep_get_network_port_filename (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  gchar *network_port_filename;

  network_port_filename = priv->network_port_filename;
  return (network_port_filename);
}

/* Remember the file name where the network port was set. */
void
sep_set_network_port_filename (gchar * network_port_filename,
                               GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  if (priv->network_port_filename != NULL)
    {
      g_free (priv->network_port_filename);
      priv->network_port_filename = NULL;
    }
  priv->network_port_filename = g_strdup (network_port_filename);
  return;
}

/* Find the name of the project folder. */
gchar *
sep_get_project_folder_name (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  gchar *project_folder_name;

  project_folder_name = priv->project_folder_name;
  return (project_folder_name);
}

/* Remember the name of the project folder. */
void
sep_set_project_folder_name (gchar * project_folder_name, GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  if (priv->project_folder_name != NULL)
    {
      g_free (priv->project_folder_name);
      priv->project_folder_name = NULL;
    }
  priv->project_folder_name = project_folder_name;

  return;
}

/* Find the name of the project file. */
gchar *
sep_get_project_file_name (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  gchar *project_file_name;

  project_file_name = priv->project_file_name;
  return (project_file_name);
}

/* Remember the name of the project file. */
void
sep_set_project_file_name (gchar * project_file_name, GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  if (priv->project_file_name != NULL)
    {
      g_free (priv->project_file_name);
      priv->project_file_name = NULL;
    }
  priv->project_file_name = project_file_name;

  return;
}

/* Find the name of the configuration file. */
gchar *
sep_get_configuration_filename (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  gchar *configuration_filename;

  configuration_filename = priv->configuration_filename;
  return (configuration_filename);
}

/* Find the path to the user interface files. */
gchar *
sep_get_ui_path (GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;
  gchar *ui_path;

  ui_path = priv->ui_path;
  return (ui_path);
}

/* Set the name of the configuration file. */
void
sep_set_configuration_filename (gchar * filename, GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  if (priv->configuration_filename != NULL)
    {
      g_free (priv->configuration_filename);
      priv->configuration_filename = NULL;
    }
  priv->configuration_filename = filename;

  return;
}

/* Find the list of sound effects.  */
GList *
sep_get_sound_list (GApplication * app)
{
  GList *sound_list;

  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  sound_list = priv->sound_list;
  return (sound_list);
}

/* Update the list of sound effects.  */
void
sep_set_sound_list (GList * sound_list, GApplication * app)
{
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  priv->sound_list = sound_list;
  return;
}

/* Find the persistent data for the internal sequencer.  */
void *
sep_get_sequence_data (GApplication * app)
{
  void *sequence_data;
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  sequence_data = priv->sequence_data;
  return (sequence_data);
}

/* Find the persistent data for the signal handler.  */
void *
sep_get_signal_data (GApplication * app)
{
  void *signal_data;
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  signal_data = priv->signal_data;
  return (signal_data);
}

/* Find the persistent data for the timer.  */
void *
sep_get_timer_data (GApplication * app)
{
  void *timer_data;
  Sound_Effects_PlayerPrivate *priv =
    SOUND_EFFECTS_PLAYER_APPLICATION (app)->priv;

  timer_data = priv->timer_data;
  return (timer_data);
}
