/*
 * File: time_diff.c, author: John Sauter, date: January 27, 2017.
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

/* Compute the time in seconds between the beginning of day A and
 * the beginning of day B.  */
static long long int
diff_day_seconds (struct tm *A_tm, struct tm *B_tm,
		  int variable_length_seconds_before_1972);

/* Subroutine to compute the difference, in seconds, between two 
 * instants of Coordinated Universal time.
 *
 * The parameter variable_length_seconds_before_1972 controls
 * the interpretation of seconds before 1972.  Before the invention
 * of the atomic clock in 1955, it was adequate to define the 
 * second as 1/86,400 of the length of a mean solar day.  
 * By 1960 it had become clear to the scientific world that the
 * Earth's rotation was no longer steady enough for the most
 * accurate clocks, so the definition of a second was changed 
 * from Earth-based to atomic-based.
 *
 * Civil time has always been based on the days and the seasons,
 * so it was necessary to make the new second fit into the
 * variable-length day.  By 1972 the current procedure was settled
 * on, in which there are a varying number of fixed-length seconds
 * in each day.
 *
 * Depending on your application's needs, you will want time before
 * 1972 to be measured using modern fixed-length seconds, or
 * the variable-length seconds that were being used at the time.
 * For example, consider this problem: "A beam of light left the 
 * Earth on January 1, 1970, at 00:00 UTC.  How far from Earth was 
 * it on January 1, 2017, at 00:00 UTC?"  To answer that question 
 * you would use modern, fixed-length seconds.  If the problem is
 * "Fredrick was born on February 29, 1856.  How old was he
 * on December 1, 1879?", you would use contemporary seconds.  
 *
 * You should use variable-length seconds when you are concerned
 * with the time as told by a clock, such as reckoning birthdays, 
 * and fixed-length seconds when you are concerned with activities 
 * that are not based on a clock, such as the time it takes an
 * object to fall through a given distance.
 */
long long int
time_diff (struct tm *A_tm, struct tm *B_tm,
	   int variable_length_seconds_before_1972)
{
  long long int day_seconds;
  long long int A_exclude, B_include;
  
  /* Are the two times on the same day?  */
  day_seconds = 0;
  if ((A_tm->tm_year != B_tm->tm_year) ||
      (A_tm->tm_mon != B_tm->tm_mon) ||
      (A_tm->tm_mday != B_tm->tm_mday))
    {
      /* They are not.  Compute the time in seconds between
       * the beginning of day A and the beginning of day B.  */
      day_seconds =
	diff_day_seconds (A_tm, B_tm,
			  variable_length_seconds_before_1972);
    }
  
  /* Compute the part of day A that we skip.  */
  A_exclude =
    (A_tm->tm_hour * 3600) + (A_tm->tm_min * 60) + A_tm->tm_sec;

  /* Compute the part of day B that we add.  */
  B_include =
    (B_tm->tm_hour * 3600) + (B_tm->tm_min * 60) + B_tm->tm_sec;

  /* Return the seconds between the days, minus the part of day A
   * that we are not counting, plus the part of day B that we add.  
   */
  return (day_seconds - A_exclude + B_include);
}

/* Compute the Julian Day Number corresponding to a specified
 * year, month and day in the Gregorian calender.  
 * Here, a Julian Day Number refers to a day as a whole, rather 
 * than to a particular moment within a day.  Day 0 is November 24,
 * 4714 BC, and Julian Day Numbers are always integers.  */
int
time_Julian_day_number (int year, int month, int day)
{
  int m, y, prev_days, year_days, leap_days, result;
  
  /* Adjust the year to start on March 1 to make leap day the last
   * day of the year.  */
  /* m is 0-based, but month is 1-based.  */
  m = month - 3;
  if (m < 0)
    m = m + 12;
  /* We now have m==0 => March, m==1 => April ... m==10 => January,
   * m==11 => February.  */

  /* Count years from -4800.  */
  y = year + 4800;
  
  /* If the month is January or February, we are in the previous
   * (March-based) year.  */
  if (month < 3)
    y = y - 1;

  /* Calculate the number of days in the previous months of this 
   * year.  */
  prev_days = ((153 * m) + 2) / 5;

  /* Calculate the number of days in previous years excluding 
   * leap days.  */
  year_days = 365 * y;

  /* Adjust for an additional day each leap year.
   * We use the Gregorian rule, which means dates before 
   * October 15, 1582 are named according to the 
   * Proleptic Gregorian calendar.  */
  leap_days = (y / 4) - (y / 100) + (y / 400);

  /* Adjust the base date to be November 24, 4714 BC.  */
  result = year_days + leap_days + prev_days + day - 32045;
  return (result);
}

/* Compute the number of seconds between two days,
 * figuring from the start of day A to the start of day B.  */
static long long int
diff_day_seconds (struct tm *A_tm, struct tm *B_tm,
		  int variable_length_seconds_before_1972)
{
  int A_jdn, B_jdn, A_dtai, B_dtai;
  long long int result;

  /* Compute the Julian Day Number for the two days.  */
  A_jdn = time_Julian_day_number (A_tm->tm_year + 1900,
				  A_tm->tm_mon + 1,
				  A_tm->tm_mday);
  B_jdn = time_Julian_day_number (B_tm->tm_year + 1900,
				  B_tm->tm_mon + 1,
				  B_tm->tm_mday);

  /* Compute the value of DTAI on each day.  */
  A_dtai = time_DTAI (A_jdn,
		      variable_length_seconds_before_1972);
  B_dtai = time_DTAI (B_jdn,
		      variable_length_seconds_before_1972);
  
  /* The result is the number of days between the dates, 
   * times 86,400, plus the increase in DTAI.  */
  result =
    ((B_jdn - A_jdn) * (long long int) 86400) +
    (B_dtai - A_dtai);
  return (result);
}

/* List of days which start with a different value of DTAI
 * than the previous day.  Each entry is two integers:
 * the Julian Day Number and the value of DTAI.  */
#include "dtai_table.h"
static int DTAI_table[DTAI_ENTRY_COUNT][2] = {
#include "dtai_table.tab"
};
  
/* Determine the value of DTAI at the beginning of a day,
 * specified by its Julian Day Number.  */
int
time_DTAI (int day_number,
	   int variable_length_seconds_before_1972)
{
  int index, past_limit, future_limit, return_value;
  double days;
  int the_day;

  past_limit = 0;
  future_limit = DTAI_ENTRY_COUNT - 1;

  /* If the application prefers variable-length seconds
   * before 1972, then the value of DTAI for all dates
   * before 1972 is the same as the value for January 1,
   * 1972.  */
  the_day = day_number;
  if ((variable_length_seconds_before_1972 == 1) &&
      (the_day < time_Julian_day_number (1972,1,1)))
    {
      the_day = time_Julian_day_number (1972,1,1);
    }

  if (the_day < DTAI_table[past_limit][0])
    {
      /* The date is before the beginning of the table.
       * Assume 25.2 negative leap seconds per year.  */
      days = DTAI_table[past_limit][0] - the_day;
      return_value =
	((int)((double) (days * 25.2) / 365.2425) +
	 (double) DTAI_table[past_limit][1]);
      return (return_value);
    }

  if (the_day > DTAI_table[future_limit][0])
    {
      /* The date is after the end of the table.
       * Assume 3.34 positive leap seconds per year.  */
      days = the_day - DTAI_table[future_limit][0];
      return_value =
	((int)((double) (days * 3.34) / 365.2425) +
	 (double) DTAI_table[future_limit][1]);
      return (return_value);
    }
  
  /* Do a binary search for the day.  */
  for (;;)
    {
      /* Set our index to the midpoint of the current range.  */
      index = (future_limit + past_limit) / 2;
      if (future_limit < past_limit)
	{
	  /* The entry is not in the table.
	   * Return the entry immediately pastward of
	   * the requested day.  */
	  return_value = DTAI_table [index][1];
	  return (return_value);
	}
      
      if (the_day < DTAI_table [index][0])
	{
	  future_limit = index - 1;
	  continue;
	}

      if (the_day > DTAI_table [index][0])
	{
	  past_limit = index + 1;
	  continue;
	}
      
      /* We have an exact match.  */
      return_value = DTAI_table [index][1];
      return (return_value);
    }
  
  return (0);
}
