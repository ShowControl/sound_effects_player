/*
 * File: time_utc_to_local.c, author: John Sauter, date: January 14, 2017.
 */

/*
 * Copyright © 2017 by John Sauter <John_Sauter@systemeyescomputerstore.com>

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

#include "time_subroutines.h"

/* Convert a tm structure containing Coordinated Universal Time
 * to one containing local time.  */
int
time_UTC_to_local (struct tm *coordinated_universal_time,
		   struct tm *local_time_tm,
		   int variable_length_seconds_before_1972)
{
  struct tm utc_time_tm;
  time_t seconds_since_epoch;
  int gmt_offset, hours_offset, minutes_offset, seconds_offset;
  int prev_minute_length;
  
  /* Compute the number of seconds from the epoch until the 
   * beginning of the current minute, since the timegm and 
   * localtime functions will not work correctly during a 
   * leap second.  */
  time_copy_tm (coordinated_universal_time, &utc_time_tm);
  utc_time_tm.tm_sec = 0;
  seconds_since_epoch = timegm (&utc_time_tm);

  /* Convert to local time in a tm structure.  */
  localtime_r (&seconds_since_epoch, local_time_tm);

  /* Now that we have access to the GMT offset, we do the
   * conversion again, this time taking leap seconds into 
   * account.  */
  time_copy_tm (coordinated_universal_time, &utc_time_tm);
  gmt_offset = local_time_tm->tm_gmtoff;

  /* When converting to local time, handle hours, minutes and
   * seconds separately.  That way leap seconds occur at
   * about the same time everywhere, regardless of time zone.  
   */
  hours_offset = gmt_offset / 3600;
  minutes_offset = (gmt_offset / 60) - (hours_offset * 60);
  seconds_offset =
    gmt_offset - (hours_offset * 3600) - (minutes_offset * 60);

  /* Special processing if the time zone is not on a minute
   * boundary.  We delay the leap second until the end of
   * the local minute.  */
  if (seconds_offset != 0)
    {
      /* If the time zone is not on a minute boundary,
       * and the minute we are leaving does not have
       * 60 seconds, compensate for it.  */
      prev_minute_length =
	time_length_prev_UTC_minute (&utc_time_tm,
			    variable_length_seconds_before_1972);
      if (prev_minute_length != 60)
	{
	  seconds_offset =
	    seconds_offset + prev_minute_length - 60;
	}
      
      /* Always add seconds, rather than subtract.  */
      if (seconds_offset < 0)
	{
	  seconds_offset = seconds_offset + 60;
	  minutes_offset = minutes_offset - 1;
	  if (minutes_offset < 0)
	    {
	      minutes_offset = minutes_offset + 60;
	      hours_offset = hours_offset - 1;
	    }
	}
    }
  
  utc_time_tm.tm_hour = utc_time_tm.tm_hour + hours_offset;
  utc_time_tm.tm_min = utc_time_tm.tm_min + minutes_offset;
  utc_time_tm.tm_sec = utc_time_tm.tm_sec + seconds_offset;

  /* Copy the year, month, day, hour, minute and second
   * back to the caller.  */
  local_time_tm->tm_year = utc_time_tm.tm_year;
  local_time_tm->tm_mon = utc_time_tm.tm_mon;
  local_time_tm->tm_mday = utc_time_tm.tm_mday;
  local_time_tm->tm_hour = utc_time_tm.tm_hour;
  local_time_tm->tm_min = utc_time_tm.tm_min;
  local_time_tm->tm_sec = utc_time_tm.tm_sec;

  /* Make sure all the fields are in their valid ranges.  */
  time_local_normalize (local_time_tm, local_time_tm->tm_sec,
			variable_length_seconds_before_1972);
    
  return (0);
}
