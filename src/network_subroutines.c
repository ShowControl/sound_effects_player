/*
 * network_subroutines.c
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
#include <gio/gio.h>
#include "sound_effects_player.h"
#include "parse_net_subroutines.h"
#include "network_subroutines.h"

/* The persistent data used by the network subroutines. */

struct network_info
{
  gchar *network_buffer;
  gint port_number;
  gboolean bound;
  GSource *source_IPv4, *source_IPv6;
  GSocket *socket_IPv4, *socket_IPv6;
};

/* Subroutines to handle network messages */

/* Receive incoming data. This is called from the main loop whenever
 * there is data or a disconnect on a port. */
static gboolean
receive_data_callback (GSocket *socket, GIOCondition condition,
                       gpointer user_data)
{
  struct network_info *network_data;
  gchar *network_buffer;
  GError *error = NULL;
  gssize nread;

  /* Find the network buffer */
  network_data = sep_get_network_data ((GApplication *) user_data);
  network_buffer = network_data->network_buffer;

  /* If we have data, process it. */
  if ((condition & G_IO_IN) != 0)
    {
      nread =
        g_socket_receive (socket, network_buffer, network_buffer_size, NULL,
                          &error);

      if (error != NULL)
        {
          printf ("Error in network socket receive: %s.\n", error->message);
          g_error_free (error);
          return G_SOURCE_REMOVE;
        }
      if (nread != 0)
        {
          /* Parse the received datagram.  */
          parse_net_text (nread, network_buffer, user_data);
        }

    }

  /* If we have received the hangup condition, stop listening for data. */
  if ((condition & G_IO_HUP) != 0)
    return G_SOURCE_REMOVE;

  /* Otherwise, continue listening. */
  return G_SOURCE_CONTINUE;
}

/* Initialize the network subroutines.  The return value is the 
 * persistent data.  */
void *
network_init (GApplication *app)
{
  gchar *network_buffer;
  struct network_info *network_data;

  /* Allocate the persistent information. */
  network_data = g_malloc (sizeof (struct network_info));

  /* Allocate the network buffer. */
  network_buffer = g_malloc0 (network_buffer_size);
  network_data->network_buffer = network_buffer;

  /* Set the default port. */
  network_data->port_number = 1500;

  /* The port is not yet bound.  */
  network_data->bound = FALSE;

  return network_data;
}

/* Subroutine to bind the network port so we can listen on it.  */
void
network_bind_port (GApplication *app)
{
  GError *error = NULL;
  GSocket *socket_IPv4, *socket_IPv6;
  GInetAddress *inet_IPv4_address, *inet_IPv6_address;
  GSocketAddress *socket_IPv4_address, *socket_IPv6_address;
  GSource *source_IPv4, *source_IPv6;
  struct network_info *network_data;

  network_data = sep_get_network_data (app);

  /* Create a socket to listen for UDP messages on IPv6 and, if necessary,
   * another to listen for UDP messages on IPv4. */

  socket_IPv6 =
    g_socket_new (G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_DATAGRAM,
                  G_SOCKET_PROTOCOL_UDP, &error);
  if (error != NULL)
    {
      printf ("Failed to create network socket: %s.\n", error->message);
      g_error_free (error);
      return;
    }

  inet_IPv6_address = g_inet_address_new_any (G_SOCKET_FAMILY_IPV6);
  socket_IPv6_address =
    g_inet_socket_address_new (inet_IPv6_address, network_data->port_number);
  g_socket_bind (socket_IPv6, socket_IPv6_address, FALSE, &error);
  if (error != NULL)
    {
      printf ("Network socket bind failed: %s.\n", error->message);
      g_error_free (error);
      return;
    }
  source_IPv6 =
    g_socket_create_source (socket_IPv6, G_IO_IN | G_IO_HUP, NULL);
  g_source_set_callback (source_IPv6, (GSourceFunc) receive_data_callback,
                         app, NULL);
  g_source_attach (source_IPv6, NULL);
  network_data->source_IPv6 = source_IPv6;
  network_data->socket_IPv6 = socket_IPv6;

  if (g_socket_speaks_ipv4 (socket_IPv6))
    {
      network_data->source_IPv4 = NULL;
      network_data->socket_IPv4 = NULL;
      network_data->bound = TRUE;
      return;
    }

  /* The IPv6 socket we just created doesn't speak IPv4, so create
   * a socket that does. */

  socket_IPv4 =
    g_socket_new (G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
                  G_SOCKET_PROTOCOL_UDP, &error);
  if (error != NULL)
    {
      printf ("Failed to create network IPV4 socket: %s.\n", error->message);
      g_error_free (error);
      return;
    }
  inet_IPv4_address = g_inet_address_new_any (G_SOCKET_FAMILY_IPV4);
  socket_IPv4_address =
    g_inet_socket_address_new (inet_IPv4_address, network_data->port_number);
  g_socket_bind (socket_IPv4, socket_IPv4_address, FALSE, &error);
  if (error != NULL)
    {
      printf ("Failed to bind IPv4 socket: %s.\n", error->message);
      g_error_free (error);
      return;
    }
  source_IPv4 =
    g_socket_create_source (socket_IPv4, G_IO_IN | G_IO_HUP, NULL);
  g_source_set_callback (source_IPv4, (GSourceFunc) receive_data_callback,
                         app, NULL);
  g_source_attach (source_IPv4, NULL);
  network_data->source_IPv4 = source_IPv4;
  network_data->socket_IPv4 = socket_IPv4;
  network_data->bound = TRUE;

  return;
}

/* Set the network port number. */
void
network_set_port (int port_number, GApplication *app)
{
  struct network_info *network_data;

  network_data = sep_get_network_data (app);

  network_data->port_number = port_number;
  return;
}

/* Unbind from the network port.  */
void
network_unbind_port (GApplication *app)
{
  GError *error = NULL;
  struct network_info *network_data;

  network_data = sep_get_network_data (app);

  /* Stop network processing.  */
  if (network_data->source_IPv4 != NULL)
    {
      g_socket_close (network_data->socket_IPv4, &error);
      if (error != NULL)
        {
          printf ("Network unbind failed: %s.\n", error->message);
          g_error_free (error);
        }
      g_object_unref (network_data->socket_IPv4);
      network_data->socket_IPv4 = NULL;

      g_source_destroy (network_data->source_IPv4);
      g_source_unref (network_data->source_IPv4);
      network_data->source_IPv4 = NULL;
    }

  if (network_data->source_IPv6 != NULL)
    {
      g_socket_close (network_data->socket_IPv6, &error);
      if (error != NULL)
        {
          printf ("Network socket close failed: %s.\n", error->message);
          g_error_free (error);
        }
      g_object_unref (network_data->socket_IPv6);
      network_data->socket_IPv6 = NULL;

      g_source_destroy (network_data->source_IPv6);
      g_source_unref (network_data->source_IPv6);
      network_data->source_IPv6 = NULL;
    }

  network_data->bound = FALSE;

  return;
}

/* Find the network port number. */
gint
network_get_port (GApplication *app)
{
  struct network_info *network_data;
  gint port_number;

  network_data = sep_get_network_data (app);
  port_number = network_data->port_number;
  return (port_number);
}

/* End of file network_subroutines.c  */
