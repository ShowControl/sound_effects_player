/*
 * File: time_tm_to_string.c, author: John Sauter, date: December 31, 2016.
 */

/*
 * Copyright © 2016 by John Sauter <John_Sauter@systemeyescomputerstore.com>

 * This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *  The author's contact information is as follows:
 *    John Sauter
 *    System Eyes Computer Store
 *    20A Northwest Blvd.  Ste 345
 *    Nashua, NH  03063-4066
 *    telephone: (603) 424-1188
 *    e-mail: John_Sauter@systemeyescomputerstore.com
 */

#include <stdio.h>

#include "time_subroutines.h"

/* Subroutine to convert a time in tm format to a string, 
 * in RFC 3339 (ISO 8601) format.  Returns the number of 
 * characters written into the string, not counting the 
 * trailing NUL byte.  If the time zone is not an integral
 * number of minutes offset from UTC, violates RFC 3339 
 * by appending ":ss" to the offset.  */
int
time_tm_to_string (struct tm *input_tm,
		   char *current_time_string,
		   int current_time_string_length)
{
  int return_val;
  int gmt_offset, offset_hours, offset_minutes, offset_seconds;
  int pos_offset;
  char *timezone;
  char buffer [32];

  /* Compute the time zone information. We use Z for UTC, 
   * otherwise + or - hh:mm or hh:mm:ss.  */
  gmt_offset = input_tm->tm_gmtoff;
  if (gmt_offset == 0)
    {
      timezone = "Z";
    }
  else
    {
      if (gmt_offset < 0)
	{
	  pos_offset = -gmt_offset;
	}
      else
	{
	  pos_offset = gmt_offset;
	}
      
      offset_hours = pos_offset / 3600;
      offset_minutes = ((pos_offset / 60) - (offset_hours * 60));
      offset_seconds =
	pos_offset - (offset_hours * 3600) - (offset_minutes * 60);
      if (gmt_offset < 0)
	{
	  offset_hours = -offset_hours;
	}

      if (offset_seconds == 0)
	{
	  snprintf (&buffer [0], sizeof (buffer),
		    "%+03d:%02d", offset_hours, offset_minutes);
	}
      else
	{
	  snprintf (&buffer [0], sizeof (buffer),
		    "%+03d:%02d:%02d", offset_hours, offset_minutes,
		    offset_seconds);
	}
      timezone = &buffer [0];
    }
  
  return_val = snprintf (current_time_string,
			 current_time_string_length,
			 "%04d-%02d-%02dT%02d:%02d:%02d%s",
			 input_tm->tm_year + 1900,
			 input_tm->tm_mon + 1,
			 input_tm->tm_mday,
			 input_tm->tm_hour,
			 input_tm->tm_min,
			 input_tm->tm_sec,
			 timezone);
  
  return (return_val);
}
