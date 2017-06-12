/*
 * trace_subroutines.h
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

#include <gtk/gtk.h>
#include <gst/gst.h>
#include "sound_effects_player.h"

/* Subroutines defined in trace_subroutines.c */

void *trace_init (GApplication * app);

void trace_finalize (GApplication * app);

gint trace_sequencer_level (GApplication * app);

void trace_sequencer_write (gchar * line, GApplication * app);

/* End of file trace_subroutines.h */
