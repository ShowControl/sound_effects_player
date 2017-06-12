/*
 * File: time_utc_add.c, author: John Sauter, date: January 14, 2017.
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

/* Add to a field of a tm structure, and then make all the fields
 * have valid values.  */

/* Rounding Mode is used when a modification to a field has caused
 * another field to be invalid.  For example, if you start with
 * October 31 and increase the month by one, you have November 31,
 * which does not exist.  If Rounding Mode is -1, the result is
 * November 30.  If Rounding Mode is +1, the result is December 1.
 * Another example is birthdays on February 29.  If you were born
 * on February 29, 1996, your twentieth birthday is on February 29,
 * 2016.  What day is your twenty-first birthday?  Start with 
 * February 29, 1996 and increase the year by twenty-one
 * to get February 29, 2017, then set Rounding Mode to +1 
 * if you want March 1, 2017, or -1 if you want February 28, 2017.
 *
 * Like months, not all minutes are the same length.  If you start
 * with December 31, 2016, 23:59:60, and decrease the minutes by one
 * you get December 31, 2016, 23:58:60, which does not exist.
 * If Rounding Mode is +1 you get December 31, 2016, 23:59:00.  If
 * Rounding Mode is -1 you get December 31, 2016, 23:58:59.
 */

/* After changing the day, make sure the seconds are correct.  */
static int
time_UTC_adjust_seconds (struct tm *time_tm, int rounding_mode,
			 int variable_length_seconds_before_1972)
{
  /* The seconds can be invalid if we are at the end of the day and
   * the day we came from was longer than this day.  This can happen
   * if the day we came from was 86,401 seconds long, or if the
   * day we came to was 86,399 seconds long, or both.  */
  if (time_tm->tm_sec >=
      time_length_UTC_minute (time_tm,
			      variable_length_seconds_before_1972))
    {
      switch (rounding_mode)
	{
	case 1:
	  /* Round up to the first second of the next day.  
	   * This will often be the first second of the next year.  
	   */
	  time_tm->tm_sec = 0;
	  time_tm->tm_min = time_tm->tm_min + 1;
	  if (time_tm->tm_min > 59)
	    {
	      time_tm->tm_min = 0;
	      time_tm->tm_hour = time_tm->tm_hour + 1;
	      if (time_tm->tm_hour > 23)
		{
		  time_tm->tm_hour = 0;
		  time_tm->tm_mday = time_tm->tm_mday + 1;
		  if (time_tm->tm_mday > time_length_month (time_tm))
		    {
		      time_tm->tm_mday = 1;
		      time_tm->tm_mon = time_tm->tm_mon + 1;
		      if (time_tm->tm_mon > 11)
			{
			  time_tm->tm_mon = 0;
			  time_tm->tm_year = time_tm->tm_year + 1;
			}
		    }
		}
	    }
	  break;
	  
	case -1:
	  /* Round down to the last second of this day.  */
	  time_tm->tm_sec =
	    time_length_UTC_minute (time_tm,
				    variable_length_seconds_before_1972)
	    - 1;
	  break;
	  
	default:
	  break;
	}
    }
  return (0);
}

/* Add a specified number of years to a specified time.  */
int
time_UTC_add_years (struct tm *time_tm, int addend,
		    int rounding_mode,
		    int variable_length_seconds_before_1972)
{
  int return_value;
  
  time_tm->tm_year = time_tm->tm_year + addend;

  /* We have just changed the year.  If the day of the month is 
   * outside its valid range, which can only be the case for 
   * February 29, round it up to March 1 or down to February 28, 
   * depending on the rounding mode.
   */
  if (time_tm->tm_mday > time_length_month (time_tm))
    {
      switch (rounding_mode)
	{
	case 1:
	  /* Round up to March 1.  */
	  time_tm->tm_mday = 1;
	  time_tm->tm_mon = time_tm->tm_mon + 1;
	  while (time_tm->tm_mon > 11)
	    {
	      time_tm->tm_mon = time_tm->tm_mon - 12;
	      time_tm->tm_year = time_tm->tm_year + 1;
	    }
	  break;
	  
	case -1:
	  /* Round down to February 28.  */
	  time_tm->tm_mday = time_length_month (time_tm);
	  break;
	  
	default:
	  break;
	}
    }

  /* Adjust the value of the seconds field to be valid.  */
  return_value =
    time_UTC_adjust_seconds (time_tm, rounding_mode,
			     variable_length_seconds_before_1972);
  if (return_value != 0)
    return (return_value);
    
  return_value =
    time_UTC_normalize (time_tm, time_tm->tm_sec,
			variable_length_seconds_before_1972);
  return (return_value);
}

/* Add a specified number of months to the time.  
 * To subtract months, make the amount to add negative.  */
int
time_UTC_add_months (struct tm *time_tm, int addend,
		     int rounding_mode,
		     int variable_length_seconds_before_1972)
{
  int return_value;
  
  time_tm->tm_mon = time_tm->tm_mon + addend;
  
  /* Get the months value into its valid range, compensating by
   * adjusting the years.  */
  while (time_tm->tm_mon < 0)
    {
      time_tm->tm_mon = time_tm->tm_mon + 12;
      time_tm->tm_year = time_tm->tm_year - 1;
    }
  while (time_tm->tm_mon > 11)
    {
      time_tm->tm_mon = time_tm->tm_mon - 12;
      time_tm->tm_year = time_tm->tm_year + 1;
    }

  /* If we are at the end of the month, we may have to adjust 
   * the day to the last day of this month or the first day 
   * of the next.  */
  if (time_tm->tm_mday > time_length_month (time_tm))
    {
      switch (rounding_mode)
	{
	case 1:
	  time_tm->tm_mday = 1;
	  time_tm->tm_mon = time_tm->tm_mon + 1;
	  if (time_tm->tm_mon > 11)
	    {
	      /* This is only a formality--December has 31 days
	       * so we will never have to round up to January of
	       * the next year.  */
	      time_tm->tm_mon = time_tm->tm_mon - 12;
	      time_tm->tm_year = time_tm->tm_year + 1;
	    }
	  break;

	case -1:
	  time_tm->tm_mday = time_length_month (time_tm);
	  break;

	default:
	  break;
	}
    }
  
  /* Make sure the value of the seconds field is valid.  */
  return_value =
    time_UTC_adjust_seconds (time_tm, rounding_mode,
			     variable_length_seconds_before_1972);
  if (return_value != 0)
    return (return_value);
  
  return_value =
    time_UTC_normalize (time_tm, time_tm->tm_sec,
			variable_length_seconds_before_1972);
  return (return_value);
}

/* Add a specified number of days to a specified time.  */
int
time_UTC_add_days (struct tm *time_tm, int addend,
		   int rounding_mode,
		   int variable_length_seconds_before_1972)
{
  int return_value;
  
  time_tm->tm_mday = time_tm->tm_mday + addend;

  /* Go through the months one at a time because they have 
   * non-uniform lengths.  */
  while (time_tm->tm_mday < 1)
    {
      /* We are backing into the previous month.  */
      time_tm->tm_mon = time_tm->tm_mon - 1;
      if (time_tm->tm_mon < 0)
	{
	  time_tm->tm_mon = time_tm->tm_mon + 12;
	  time_tm->tm_year = time_tm->tm_year - 1;
	}
      time_tm->tm_mday =
	time_tm->tm_mday + time_length_month (time_tm);
    }

  while (time_tm->tm_mday > time_length_month (time_tm))
    {
      /* We are moving forward into future months.  */
      time_tm->tm_mday =
	time_tm->tm_mday - time_length_month (time_tm);
      time_tm->tm_mon = time_tm->tm_mon + 1;
      if (time_tm->tm_mon > 11)
	{
	  time_tm->tm_mon = time_tm->tm_mon - 12;
	  time_tm->tm_year = time_tm->tm_year + 1;
	}
    }
  
  /* Make sure the value of the seconds field is valid.  */
  return_value =
    time_UTC_adjust_seconds (time_tm, rounding_mode,
			     variable_length_seconds_before_1972);
  if (return_value != 0)
    return (return_value);
  
  return_value =
    time_UTC_normalize (time_tm, time_tm->tm_sec,
			variable_length_seconds_before_1972);
  return (return_value);
}

/* Add a specified number of hours to the specified time.  */
int
time_UTC_add_hours (struct tm *time_tm, int addend,
		    int rounding_mode,
		    int variable_length_seconds_before_1972)
{
  int return_value;

  time_tm->tm_hour = time_tm->tm_hour + addend;

  /* If the hours field is out of range, adjust it.  */
  while (time_tm->tm_hour < 0)
    {
      time_tm->tm_hour = time_tm->tm_hour + 24;
      time_tm->tm_mday = time_tm->tm_mday - 1;
      if (time_tm->tm_mday < 1)
	{
	  time_tm->tm_mday =
	    time_tm->tm_mday + time_length_prev_month (time_tm);
	  time_tm->tm_mon = time_tm->tm_mon - 1;
	  if (time_tm->tm_mon < 0)
	    {
	      time_tm->tm_mon = time_tm->tm_mon + 12;
	      time_tm->tm_year = time_tm->tm_year - 1;
	    }
	}
    }

  while (time_tm->tm_hour > 23)
    {
      time_tm->tm_hour = time_tm->tm_hour - 24;
      time_tm->tm_mday = time_tm->tm_mday + 1;
      if (time_tm->tm_mday > time_length_month (time_tm))
	{
	  time_tm->tm_mday =
	    time_tm->tm_mday - time_length_month (time_tm);
	  time_tm->tm_mon = time_tm->tm_mon + 1;
	  if (time_tm->tm_mon > 11)
	    {
	      time_tm->tm_mon = time_tm->tm_mon - 12;
	      time_tm->tm_year = time_tm->tm_year + 1;
	    }
	}
    }
  
  /* Make sure the seconds field is valid.  */
  return_value =
    time_UTC_adjust_seconds (time_tm, rounding_mode,
			     variable_length_seconds_before_1972);
  if (return_value != 0)
    return (return_value);

  return_value =
    time_UTC_normalize (time_tm, time_tm->tm_sec,
			variable_length_seconds_before_1972);
  return (return_value);
}

/* Add a specified number of minutes to the specified time.  */
int
time_UTC_add_minutes (struct tm *time_tm, int addend,
		      int rounding_mode,
		      int variable_length_seconds_before_1972)
{
  int return_value;

  time_tm->tm_min = time_tm->tm_min + addend;

  /* If the minutes field is out of range, adjust it.  */
  while (time_tm->tm_min < 0)
    {
      time_tm->tm_min = time_tm->tm_min + 60;
      time_tm->tm_hour = time_tm->tm_hour - 1;
      if (time_tm->tm_hour < 0)
	{
	  time_tm->tm_hour = time_tm->tm_hour + 24;
	  time_tm->tm_mday = time_tm->tm_mday - 1;
	  if (time_tm->tm_mday < 1)
	    {
	      time_tm->tm_mday =
		time_tm->tm_mday + time_length_prev_month (time_tm);
	      time_tm->tm_mon = time_tm->tm_mon - 1;
	      if (time_tm->tm_mon < 0)
		{
		  time_tm->tm_mon = time_tm->tm_mon + 12;
		  time_tm->tm_year = time_tm->tm_year - 1;
		}
	    }
	}
    }

  while (time_tm->tm_min > 59)
    {
      time_tm->tm_min = time_tm->tm_min - 60;
      time_tm->tm_hour = time_tm->tm_hour + 1;
      if (time_tm->tm_hour > 23)
	{
	  time_tm->tm_hour = time_tm->tm_hour - 24;
	  time_tm->tm_mday = time_tm->tm_mday + 1;
	  if (time_tm->tm_mday > time_length_month (time_tm))
	    {
	      time_tm->tm_mday =
		time_tm->tm_mday - time_length_month (time_tm);
	      time_tm->tm_mon = time_tm->tm_mon + 1;
	      if (time_tm->tm_mon > 11)
		{
		  time_tm->tm_mon = time_tm->tm_mon - 12;
		  time_tm->tm_year = time_tm->tm_year + 1;
		}
	    }
	}
    }
  
  /* Make sure the seconds field is valid.  */
  return_value =
    time_UTC_adjust_seconds (time_tm, rounding_mode,
			     variable_length_seconds_before_1972);
  if (return_value != 0)
    return (return_value);

  return_value =
    time_UTC_normalize (time_tm, time_tm->tm_sec,
			variable_length_seconds_before_1972);
  return (return_value);
}

/* Add a specified number of seconds to a time.  */
int
time_UTC_add_seconds (struct tm *time_tm,
		      long long int add_seconds,
		      int variable_length_seconds_before_1972)
{
  long long int add_nanoseconds;
  long long int nanoseconds;
  int return_value;

  /* Use time_UTC_add_seconds_ns with nanoseconds == 0.  */
  nanoseconds = 0;
  add_nanoseconds = 0;
  return_value =
    time_UTC_add_seconds_ns (time_tm, &nanoseconds,
			     add_seconds, add_nanoseconds,
			     variable_length_seconds_before_1972);
  return (return_value);
}

/* Add a specified number of seconds and nanoseconds to a time.  
 */
int
time_UTC_add_seconds_ns (struct tm *time_tm,
			 long long int *nanoseconds,
			 long long int add_seconds,
			 long long int add_nanoseconds,
			 int variable_length_seconds_before_1972)
{
  long long int accumulated_seconds;
  long long int accumulated_nanoseconds;
  int return_value;

  /* Bring the nanoseconds into its proper range, compensating
   * by adjusting seconds.  */
  accumulated_seconds = time_tm->tm_sec + add_seconds;
  accumulated_nanoseconds = *nanoseconds + add_nanoseconds;
  while (accumulated_nanoseconds >= 1e9)
    {
      accumulated_seconds = accumulated_seconds + 1;
      accumulated_nanoseconds = accumulated_nanoseconds - 1e9;
    }
  while (accumulated_nanoseconds < 0)
    {
      accumulated_seconds = accumulated_seconds - 1;
      accumulated_nanoseconds = accumulated_nanoseconds + 1e9;
    }

  /* The caller can increase or decrease the seconds by a large
   * amount to navigate the calendar by seconds.  Adjust the year,
   * month, day of the month, hour and minute fields to get the
   * seconds field into its valid range, and thereby name the 
   * designated second using the Gregorian calendar.  
   * Note that time_UTC_normalize takes the seconds value in a 
   * long long int parameter and ignores the tm_sec field of time_tm.  
   * This is to allow the number of seconds to be very large or 
   * very negative.  When time_UTC_normalize is complete
   * it stores the adjusted value of the seconds parameter into 
   * time_tm->tm_sec.  */
  return_value =
    time_UTC_normalize (time_tm, accumulated_seconds,
			variable_length_seconds_before_1972);
  *nanoseconds = accumulated_nanoseconds;
  return (return_value);
}
