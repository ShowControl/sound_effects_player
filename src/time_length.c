/*
 * File: time_length.c, author: John Sauter, date: January 14, 2017.
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

/* Return the length of a month.  Most months have fixed lengths, 
 * but February has either 28 or 29 days, depending on the year.  
 */
int
time_length_month (struct tm *time_tm)
{
  int year;

  switch (time_tm->tm_mon)
    {
    case 0: /* January */
      return (31);
      break;
      
    case 1: /* February */
      year = time_tm->tm_year + 1900;
      if ((year % 4) != 0)
	return (28);

      /* The year is a multiple of 4.  */
      if ((year % 100) != 0)
	{
	  /* The year is not a century year, so it is a leap year.  
	   */
	  return (29);
	}

      /* The year is a century year.  */
      if ((year % 400) == 0)
	{
	  /* The four-century years are leap years.  */
	  return (29);
	}
      else
	{
	  /* The century years that are not four-century years
	   * are not leap years.  */
	  return (28);
	}
      break;
      
    case 2: /* March */
      return (31);
      break;
      
    case 3: /* April */
      return (30);
      break;
      
    case 4: /* May */
      return (31);
      break;
      
    case 5: /* June */
      return (30);
      break;
      
    case 6: /* July */
      return (31);
      break;
      
    case 7: /* August */
      return (31);
      break;
      
    case 8: /* September */
      return (30);
      break;
      
    case 9: /* October */
      return (31);
      break;
      
    case 10: /* November */
      return (30);
      break;
      
    case 11: /* December */
      return (31);
      break;
      
    default:
      return (0);
      break;
    }
  
  return (0);
}

/* Return the length of the previous month, which might be 
 * the last month of the previous year.  */
int
time_length_prev_month (struct tm *time_tm)
{

  struct tm prev_month_tm;
  int return_value;

  /* Compute the previous month.  */
  time_copy_tm (time_tm, &prev_month_tm);
  prev_month_tm.tm_mon = time_tm->tm_mon - 1;
  if (prev_month_tm.tm_mon < 0)
    {
      prev_month_tm.tm_mon = prev_month_tm.tm_mon + 12;
      prev_month_tm.tm_year = prev_month_tm.tm_year - 1;
    }
  return_value = time_length_month (&prev_month_tm);
  return (return_value);
}

/* Return the length of this UTC minute.  */
int
time_length_UTC_minute (struct tm *time_tm,
			int variable_length_seconds_before_1972)
{

  int JDN_today, JDN_tomorrow, DTAI_today, DTAI_tomorrow;

  /* All minutes are 60 seconds in length except possibly
   * the last minute of the UTC day.  */
  if ((time_tm->tm_hour != 23) || (time_tm->tm_min != 59))
    return (60);
  
  /* Calculate the Julian Day Number of the specified date.  */
  JDN_today = time_Julian_day_number (time_tm->tm_year + 1900,
				      time_tm->tm_mon + 1,
				      time_tm->tm_mday);

  /* Calculate the number of seconds between UTC and TAI, 
   * known as DTAI, for this date.  */
  DTAI_today = time_DTAI (JDN_today,
			  variable_length_seconds_before_1972);

  /* Do the same for tomorrow.  */
  JDN_tomorrow = JDN_today + 1;
  DTAI_tomorrow = time_DTAI (JDN_tomorrow,
			     variable_length_seconds_before_1972);

  /* If the value of DTAI did not change between the beginning 
   * of the day today and the beginning of the day tomorrow, 
   * then today has 86,400 seconds and the last minute of the day 
   * has 60 seconds.
   */
  if (DTAI_today == DTAI_tomorrow)
    return (60);

  /* Otherwise, modify the length of last minute of the day to 
   * account for the increase or decrease in DTAI.  */
  return (60 + DTAI_tomorrow - DTAI_today);
}

/* Return the number of seconds in the previous UTC minute.  */
int
time_length_prev_UTC_minute (struct tm *time_tm,
			     int variable_length_seconds_before_1972)
{
  struct tm prev_minute_tm;
  int return_value;

  /* Compute the previous minute.  If this minute is the first 
   * minute of he year, the previous minute will be the last 
   * minute of the previous year.  */
  time_copy_tm (time_tm, &prev_minute_tm);
  prev_minute_tm.tm_min = time_tm->tm_min - 1;
  if (prev_minute_tm.tm_min < 0)
    {
      prev_minute_tm.tm_min = prev_minute_tm.tm_min + 60;
      prev_minute_tm.tm_hour = prev_minute_tm.tm_hour - 1;
      if (prev_minute_tm.tm_hour < 0)
	{
	  prev_minute_tm.tm_hour = prev_minute_tm.tm_hour + 24;
	  prev_minute_tm.tm_mday = prev_minute_tm.tm_mday - 1;
	  if (prev_minute_tm.tm_mday < 1)
	    {
	      prev_minute_tm.tm_mday =
		time_length_prev_month (time_tm);
	      prev_minute_tm.tm_mon = prev_minute_tm.tm_mon - 1;
	      if (prev_minute_tm.tm_mon < 0)
		{
		  prev_minute_tm.tm_mon =
		    prev_minute_tm.tm_mon + 12;
		  prev_minute_tm.tm_year =
		    prev_minute_tm.tm_year - 1;
		}
	    }
	}
    }

  return_value =
    time_length_UTC_minute (&prev_minute_tm,
			    variable_length_seconds_before_1972);
  return (return_value);
}

/* Return the length of this local minute.  */
int
time_length_local_minute (struct tm *time_tm,
			  int variable_length_seconds_before_1972)
{
  /* Modern time zones are mostly 60 minutes offset from UTC.  
   * Those that are not are mostly 30 minutes offset.  All are 
   * offset by an integral number of minutes.  This makes it 
   * easy to determine which local minute contains a 
   * leap second: it is the one that starts at the same time 
   * as the UTC minute that contains a leap second.  
   * However, in the past there were some time zones that
   * were not offset from UTC by an integral number of minutes.
   * Since we are extending leap seconds into the past, we are
   * forced to decide which local minute, not starting at the 
   * same time as any UTC minute, should contain the leap second.
   * I have chosen the minute containing the leap second to be
   * the local minute that starts during the UTC minute 
   * containing the leap second.  However, I defer the leap 
   * to the end of the local minute, since it is clear that 
   * second number 60 is an additional second, but it is not 
   * clear what to call a second that is added earlier in the 
   * minute.
   *
   * This problem is somewhat artificial, since it is only
   * because we are pretending that leap seconds existed before
   * time zones that the issue even arises.
   */
  struct tm utc_time_tm;
  struct tm local_tm;
  int return_val;
  
  /* Convert the local time to UTC and return the length of
   * the UTC minute corresponding to the local
   * time minute.  */
  time_copy_tm (time_tm, &local_tm);
  local_tm.tm_sec = 0;
  time_local_to_UTC (&local_tm, &utc_time_tm,
		     variable_length_seconds_before_1972);
  return_val =
    time_length_UTC_minute (&utc_time_tm,
			    variable_length_seconds_before_1972);
  return (return_val);
}

/* Return the number of seconds in the previous local minute.  */
int
time_length_prev_local_minute (struct tm *time_tm,
			       int variable_length_seconds_before_1972)
{
  struct tm prev_minute_tm;
  int return_value;

  /* Compute the previous minute.  If this minute is the first 
   * minute of the year, the previous minute will be the last 
   * minute of the previous year.  */
  time_copy_tm (time_tm, &prev_minute_tm);
  prev_minute_tm.tm_min = time_tm->tm_min - 1;
  if (prev_minute_tm.tm_min < 0)
    {
      prev_minute_tm.tm_min = prev_minute_tm.tm_min + 60;
      prev_minute_tm.tm_hour = prev_minute_tm.tm_hour - 1;
      if (prev_minute_tm.tm_hour < 0)
	{
	  prev_minute_tm.tm_hour = prev_minute_tm.tm_hour + 24;
	  prev_minute_tm.tm_mday = prev_minute_tm.tm_mday - 1;
	  if (prev_minute_tm.tm_mday < 1)
	    {
	      prev_minute_tm.tm_mday =
		time_length_prev_month (time_tm);
	      prev_minute_tm.tm_mon = prev_minute_tm.tm_mon - 1;
	      if (prev_minute_tm.tm_mon < 0)
		{
		  prev_minute_tm.tm_mon =
		    prev_minute_tm.tm_mon + 12;
		  prev_minute_tm.tm_year =
		    prev_minute_tm.tm_year - 1;
		}
	    }
	}
    }

  return_value =
    time_length_local_minute (&prev_minute_tm,
			      variable_length_seconds_before_1972);
  return (return_value);
}
