/*
 * File: time_local_to_utc.c, author: John Sauter, date: January 14, 2017.
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

/* Convert a tm structure containing local time
 * to one containing Coordinated Universal Time.  
 * As a side effect, update the tm_wday, tm_yday,
 * tm_isdst, tm_zone and tm_gmtoff fields in
 * the local_time parameter.  */
int
time_local_to_UTC (struct tm *local_time,
		   struct tm *coordinated_universal_time,
		   int variable_length_seconds_before_1972)
{
  struct tm local_tm;
  struct tm utc_time_tm;
  time_t seconds_since_epoch;
  int gmt_offset;
  int hours_offset, minutes_offset, seconds_offset;
  
  /* Compute the number of seconds from the epoch to the 
   * beginning of the current minute, since the mktime and 
   * gmtime functions will not work correctly during a 
   * leap second.  */
  time_copy_tm (local_time, &local_tm);
  local_tm.tm_sec = 0;
  seconds_since_epoch = mktime (&local_tm);

  /* Convert to UTC in a tm structure.  */
  gmtime_r (&seconds_since_epoch, &utc_time_tm);

  /* Now that we have access to the GMT offset, 
   * do the conversion again, taking leap seconds 
   * into account.  */
  gmt_offset = local_tm.tm_gmtoff;
  utc_time_tm.tm_year = local_time->tm_year;
  utc_time_tm.tm_mon = local_time->tm_mon;
  utc_time_tm.tm_mday = local_time->tm_mday;
  utc_time_tm.tm_hour = local_time->tm_hour;
  utc_time_tm.tm_min = local_time->tm_min;
  utc_time_tm.tm_sec = local_time->tm_sec;

  /* Add in the hours, minutes and seconds of the offset 
   * separately.  That way leap seconds occur at about 
   * the same time everywhere.  */
  hours_offset = gmt_offset / 3600;
  minutes_offset = (gmt_offset / 60) - (hours_offset * 60);
  seconds_offset =
    gmt_offset - (minutes_offset * 60) - (hours_offset * 3600);

  /* Always add seconds.  */
  if (seconds_offset > 0)
    {
      seconds_offset = seconds_offset - 60;
      minutes_offset = minutes_offset + 1;
      if (minutes_offset > 59)
	{
	  minutes_offset = minutes_offset - 60;
	  hours_offset = hours_offset + 1;
	}
    }
  
  utc_time_tm.tm_hour = utc_time_tm.tm_hour - hours_offset;
  utc_time_tm.tm_min = utc_time_tm.tm_min - minutes_offset;
  utc_time_tm.tm_sec = utc_time_tm.tm_sec - seconds_offset;

  /* Make sure the fields of the tm structure are all in their 
   * valid ranges.  */
  time_UTC_normalize (&utc_time_tm, utc_time_tm.tm_sec,
		      variable_length_seconds_before_1972);

  /* Return the result to the caller.  */
  time_copy_tm (&utc_time_tm, coordinated_universal_time);
  
  /* Also return the additional information calculated by mktime.  
   */
  local_time->tm_wday = local_tm.tm_wday;
  local_time->tm_yday = local_tm.tm_yday;
  local_time->tm_isdst = local_tm.tm_isdst;
  local_time->tm_zone = local_tm.tm_zone;
  local_time->tm_gmtoff = local_tm.tm_gmtoff;
  
  return (0);
}
