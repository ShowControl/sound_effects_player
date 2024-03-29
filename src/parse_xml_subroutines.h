/*
 * parse_xml_subroutines.h
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
/* Subroutines defined in parse_xml_subroutines.c */

void parse_xml_read_configuration_file (gchar *configuration_file_name,
                                        GApplication *app);

void parse_xml_write_configuration_file (gchar *configuration_file_name,
                                         GApplication *app);
void parse_xml_read_project_file (gchar *project_folder_name,
                                  gchar *project_file_name,
                                  GApplication *app);

/* End of file parse_xml_subroutines.h  */
