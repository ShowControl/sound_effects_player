/*
 * menu_subroutines.c
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

#include "config.h"
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "menu_subroutines.h"
#include "parse_xml_subroutines.h"
#include "network_subroutines.h"
#include "sound_subroutines.h"
#include "sound_effects_player.h"
#include "gstreamer_subroutines.h"

/* Subroutines used by the menus. */

/* The user has invoked the preferences dialogue from a menu. */
static void
preferences_activated (GSimpleAction * action, GVariant * parameter,
                       gpointer app)
{
  GtkBuilder *builder;
  GError *error = NULL;
  gchar *preferences_file_name;
  GtkWindow *parent_window;
  GtkDialog *dialog;
  gchar *ui_path;

  /* Load the preferences user interface definition from its file. */
  builder = gtk_builder_new ();
  ui_path = sep_get_ui_path (app);
  preferences_file_name = g_strconcat (ui_path, "preferences.ui", NULL);
  if (!gtk_builder_add_from_file (builder, preferences_file_name, &error))
    {
      g_critical ("Couldn't load builder file %s: %s", preferences_file_name,
                  error->message);
      g_error_free (error);
      return;
    }

  /* Auto-connect signal handlers. */
  gtk_builder_connect_signals (builder, app);

  /* Get the dialog object from the UI file. */
  dialog = GTK_DIALOG (gtk_builder_get_object (builder, "dialog1"));
  if (dialog == NULL)
    {
      g_critical ("Widget \"dialog1\" is missing in file %s.\n",
                  preferences_file_name);
      return;
    }

  /* We are done with the name of the preferences user interface file
   * and the user interface builder. */
  g_free (preferences_file_name);
  g_object_unref (G_OBJECT (builder));

  /* Set the dialog window's application, so we can retrieve it later. */
  gtk_window_set_application (GTK_WINDOW (dialog), app);

  /* Get the top-level window to use as the transient parent for
   * the dialog.  This makes sure the dialog appears over the
   * application window.  Also, destroy the dialog if the application
   * is closed, and propagate information such as styling and accessibility
   * from the top-level window to the dialog.  */
  parent_window = sep_get_top_window ((GApplication *) app);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), parent_window);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_attached_to (GTK_WINDOW (dialog),
                              GTK_WIDGET (parent_window));

  /* Run the dialog and wait for it to complete. */
  gtk_dialog_run (dialog);
  gtk_widget_destroy (GTK_WIDGET (dialog));

  return;
}

/* Subroutine called when the preferences menu changes the network port. */
gboolean
menu_network_port_changed (GtkEntry * port_entry, GtkWidget * dialog_widget)
{
  GApplication *app;
  const gchar *port_text;
  long int port_number;

  /* We are passed the dialogue widget.  We previously set its application
   * so we can retrieve it here. */
  app =
    G_APPLICATION (gtk_window_get_application (GTK_WINDOW (dialog_widget)));

  /* Extract the port number from the entry widget. */
  port_text = gtk_entry_get_text (port_entry);
  port_number = strtoll (port_text, NULL, 10);

  /* Tell the network module to change its port number. */
  network_unbind_port (app);
  network_set_port (port_number, app);
  network_bind_port (app);

  return TRUE;
}

/* Subroutine called when the preferences dialog is closed. */
gboolean
menu_preferences_close_clicked (GtkButton * close_button,
                                GtkWidget * dialog_widget)
{
  gtk_dialog_response (GTK_DIALOG (dialog_widget), 0);
  return FALSE;
}

/* Subroutine called when the application's "quit" menu item is selected. */
static void
quit_activated (GSimpleAction * action, GVariant * parameter, gpointer app)
{
  /* Shut down the gstreamer pipeline, then terminate the application.  */
  gstreamer_shutdown (app);
  return;
}

/* Subroutine called when the top-level window is closed.  */
gboolean
menu_delete_top_window (GtkButton * close_button, GdkEvent * event,
                        GtkWidget * top_box)
{
  GApplication *app;

  app = sep_get_application_from_widget (top_box);
  gstreamer_shutdown (app);

  /* The gstreamer shutdown process is asynchronous, and will terminate
   * the application when it completes, so don't do the termination here.
   */
  return TRUE;
}

/* Reset the application to its defaults. */
static void
new_activated (GSimpleAction * action, GVariant * parameter, gpointer app)
{
  network_unbind_port (app);
  sep_set_configuration_file (NULL, app);
  network_set_port (1500, app);
  network_bind_port (app);

  return;
}

/* Open a configuration file and read its contents.  
 * The file is assumed to be in XML format. */
static void
open_activated (GSimpleAction * action, GVariant * parameter, gpointer app)
{
  GtkWidget *dialog;
  GtkFileChooser *chooser;
  GtkFileChooserAction file_action = GTK_FILE_CHOOSER_ACTION_OPEN;
  GtkWindow *parent_window;
  gint res;
  gchar *configuration_file_name;

  /* Get the top-level window to use as the transient parent for
   * the file open dialog.  This makes sure the dialog appears over the
   * application window. */
  parent_window = sep_get_top_window ((GApplication *) app);

  /* Configure the dialogue: choosing multiple files is not permitted. */
  dialog =
    gtk_file_chooser_dialog_new ("Open Configuration File", parent_window,
                                 file_action, "_Cancel", GTK_RESPONSE_CANCEL,
                                 "_Open", GTK_RESPONSE_ACCEPT, NULL);
  chooser = GTK_FILE_CHOOSER (dialog);
  gtk_file_chooser_set_select_multiple (chooser, FALSE);

  /* Use the file dialog to ask for a file to read. */
  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      /* We have a file name. */
      configuration_file_name = gtk_file_chooser_get_filename (chooser);
      gtk_widget_destroy (dialog);
      /* Parse the file as an XML file and create the gstreamer pipeline.  */
      sep_create_pipeline (configuration_file_name, (GApplication *) app);
    }
  else
    {
      gtk_widget_destroy (dialog);
    }

  return;
}

static void
save_activated (GSimpleAction * action, GVariant * parameter, gpointer app)
{
}

/* Write the configuration information to an XML file. */
static void
saveas_activated (GSimpleAction * action, GVariant * parameter, gpointer app)
{

  GtkWidget *dialog;
  GtkFileChooser *chooser;
  GtkFileChooserAction file_action = GTK_FILE_CHOOSER_ACTION_SAVE;
  GtkWindow *parent_window;
  gint res;
  gchar *configuration_file_name;

  /* Get the top-level window to use as the transient parent for
   * the dialog.  This makes sure the dialog appears over the
   * application window. */
  parent_window = sep_get_top_window ((GApplication *) app);

  /* Configure the dialogue: prompt on specifying an existing file,
   * allow folder creation, and specify the current configuration file
   * name if one exists, or a default name if not. */
  dialog =
    gtk_file_chooser_dialog_new ("Save Configuration File", parent_window,
                                 file_action, "_Cancel", GTK_RESPONSE_CANCEL,
                                 "_Save", GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_application (GTK_WINDOW (dialog), app);
  chooser = GTK_FILE_CHOOSER (dialog);
  gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);
  gtk_file_chooser_set_create_folders (chooser, TRUE);
  configuration_file_name = sep_get_configuration_filename (app);
  if (configuration_file_name == NULL)
    {
      configuration_file_name =
        g_build_filename ("~/.ShowControl/", "ShowControl_config.xml", NULL);
      sep_set_configuration_filename (configuration_file_name, app);
    }
  gtk_file_chooser_set_filename (chooser, configuration_file_name);

  /* Use the file dialog to ask for a file to write. */
  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      /* We have a file name. */
      configuration_file_name = gtk_file_chooser_get_filename (chooser);
      gtk_widget_destroy (dialog);
      parse_xml_write_configuration_file (configuration_file_name, app);
    }
  else
    {
      gtk_widget_destroy (dialog);
    }

  return;
}


static void
copy_activated (GSimpleAction * action, GVariant * parameter, gpointer app)
{
}

static void
cut_activated (GSimpleAction * action, GVariant * parameter, gpointer app)
{
}

static void
paste_activated (GSimpleAction * action, GVariant * parameter, gpointer app)
{
}

static void
helpabout_activated (GSimpleAction * action, GVariant * parameter,
                     gpointer app)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *content_area;
  GtkWindow *parent_window;
  GtkDialogFlags flags;
  gchar *text;
  const gchar *gst_nano_str;
  guint gtk_major, gtk_minor, gtk_micro;
  guint gst_major, gst_minor, gst_micro, gst_nano;
  extern const guint glib_major_version;
  extern const guint glib_minor_version;
  extern const guint glib_micro_version;
  extern const guint glib_binary_age;
  extern const guint glib_interface_age;


  /* Get the top-level window to use as the transient parent for
   * the dialog.  This makes sure the dialog appears over the
   * application window. */
  parent_window = sep_get_top_window ((GApplication *) app);

  flags = GTK_DIALOG_DESTROY_WITH_PARENT;
  dialog =
    gtk_dialog_new_with_buttons ("About Sound Effects Player", parent_window,
                                 flags, "_OK", GTK_RESPONSE_NONE, NULL);
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  /* Construct the text for the "Help About" message.  */
  gtk_major = gtk_get_major_version ();
  gtk_minor = gtk_get_minor_version ();
  gtk_micro = gtk_get_micro_version ();
  gst_version (&gst_major, &gst_minor, &gst_micro, &gst_nano);
  if (gst_nano == 1)
    gst_nano_str = "(CVS)";
  else if (gst_nano == 2)
    gst_nano_str = "(Prerelease)";
  else
    gst_nano_str = "";
  text =
    g_strdup_printf ("%s is a component of SoundControl, "
                     "an application to control lighting and sound "
                     "in theatrical productions.  This component "
                     "plays music and sound effects.\n\n"
                     "Copyright &#169; %s by John Sauter.\n"
                     "This is %s version %s.\n" "Send bug reports to %s.\n"
                     "Sources are on github at %s.\n\n"
                     "This program comes with ABSOLUTELY NO WARRANTY;"
                     " for details see the GNU General Public License.  "
                     "This is free software, and you are welcome to"
                     " redistribute it under certain conditions,"
                     " as described in the GNU General Public License.\n\n"
                     "This program is linked against glib"
                     " %d.%d.%d ages %d and %d, against gtk %d.%d.%d and"
                     " against Gstreamer %d.%d.%d%s.", PACKAGE,
                     MODIFICATION_DATE, PACKAGE_NAME, PACKAGE_VERSION,
                     PACKAGE_BUGREPORT, PACKAGE_URL, glib_major_version,
                     glib_minor_version, glib_micro_version, glib_binary_age,
                     glib_interface_age, gtk_major, gtk_minor, gtk_micro,
                     gst_major, gst_minor, gst_micro, gst_nano_str);

  /* Create the label widget.  */
  label = gtk_label_new (NULL);

  /* Tell it we want to wrap lines that are too long.  */
  gtk_label_set_line_wrap ((GtkLabel *) label, TRUE);

  /* Minimum width of 100 pixels.  */
  gtk_widget_set_size_request (label, 100, -1);

  /* Start at 450 pixels wide.  */
  gtk_window_set_default_size ((GtkWindow *) dialog, 450, -1);

  /* Give the label the text we constructed, expanding &#169; into the
   * copyright symbol.  */
  gtk_label_set_markup ((GtkLabel *) label, text);
  g_free (text);
  text = NULL;

  /* Specify that the dialog box is to be destroyed when the user responds.  */
  g_signal_connect_swapped (dialog, "response",
                            G_CALLBACK (gtk_widget_destroy), dialog);

  /* Add the label, and show everything we've added.  */
  gtk_container_add (GTK_CONTAINER (content_area), label);
  gtk_widget_show_all (dialog);

  return;
}

/* Actions table to link menu items to subroutines. */
static GActionEntry app_entries[] = {
  {"preferences", preferences_activated, NULL, NULL, NULL},
  {"quit", quit_activated, NULL, NULL, NULL},
  {"new", new_activated, NULL, NULL, NULL},
  {"open", open_activated, NULL, NULL, NULL},
  {"save", save_activated, NULL, NULL, NULL},
  {"saveas", saveas_activated, NULL, NULL, NULL},
  {"copy", copy_activated, NULL, NULL, NULL},
  {"cut", cut_activated, NULL, NULL, NULL},
  {"paste", paste_activated, NULL, NULL, NULL},
  {"helpabout", helpabout_activated, NULL, NULL, NULL}
};

/* Initialize the menu. */
void
menu_init (GApplication * app, gchar * file_name)
{
  GtkBuilder *builder;
  GError *error = NULL;
  GMenuModel *app_menu;
  GMenuModel *menu_bar;

  const gchar *quit_accels[2] = { "<Ctrl>Q", NULL };

  g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries,
                                   G_N_ELEMENTS (app_entries), app);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.quit",
                                         quit_accels);

  /* Load UI from file */
  builder = gtk_builder_new ();
  if (!gtk_builder_add_from_file (builder, file_name, &error))
    {
      g_critical ("Couldn't load menu file %s: %s", file_name,
                  error->message);
      g_error_free (error);
    }

  /* Auto-connect signal handlers. */
  gtk_builder_connect_signals (builder, app);

  /* Specify the application menu and the menu bar. */
  app_menu = (GMenuModel *) gtk_builder_get_object (builder, "appmenu");
  menu_bar = (GMenuModel *) gtk_builder_get_object (builder, "menubar");
  gtk_application_set_app_menu (GTK_APPLICATION (app), app_menu);
  gtk_application_set_menubar (GTK_APPLICATION (app), menu_bar);

  /* We are finished with the builder.  */
  g_object_unref (builder);

  return;
}
