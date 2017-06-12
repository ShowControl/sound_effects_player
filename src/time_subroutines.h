/*
 * File: time_subroutines.h, author: John Sauter, date: May 7, 2017.
 * Header file for subroutines to deal with UTC time.
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

#ifndef TIME_SUBROUTINES_H
#define TIME_SUBROUTINES_H

#include <time.h>
#include <stdarg.h>

/* Convert a 128-bit integer to a string.  */
int
int128_to_string (__int128 value,
		  char *result, int result_size);

/* Copy a tm structure.  */
int
time_copy_tm (struct tm *in_tm, struct tm *out_tm);

/* Fetch the current time, without nanoseconds.  */
int
time_current_tm (struct tm *current_tm);

/* Fetch the current time, including nanoseconds.  */
int
time_current_tm_nano (struct tm *current_tm,
		      int *nanoseconds);

/* Compute the difference between two times in seconds.  */
long long int
time_diff (struct tm *A_tm, struct tm *B_tm,
	   int variable_length_seconds_before_1972)
  __attribute__ ((pure));

/* Fetch the difference between TAI and UTC at the beginning
 * of the specified Julian Day Number.  */
int
time_DTAI (int Julian_day_number,
	   int variable_length_seconds_before_1972)
  __attribute__ ((const));

/* Compute the Julian Day Number corresponding to a date.  */
int
time_Julian_day_number (int year, int month, int day)
  __attribute__ ((const));

/* Compute the length of the current minute of local time.
 */
int
time_length_local_minute (struct tm *time_tm,
			  int variable_length_seconds_before_1972)
  __attribute__ ((pure));

/* Compute the length of the current month.  */
int
time_length_month (struct tm *time_tm)
  __attribute__ ((pure));

/* Compute the length of the previous minute of local time.  
 */
int
time_length_prev_local_minute (struct tm *time_tm,
			       int variable_length_seconds_before_1972)
  __attribute__ ((pure));

/* Compute the length of the previous month.  */
int
time_length_prev_month (struct tm *time_tm)
  __attribute__ ((pure));

/* Compute the length of the previous minute of
 * Coordinated Universal Time.  */
int
time_length_prev_UTC_minute (struct tm *time_tm,
			     int variable_length_seconds_before_1972)
  __attribute__ ((pure));

/* Compute the length of the current minute of
 * Coordinated Universal Time.  */
int
time_length_UTC_minute (struct tm *time_tm,
			int variable_length_seconds_before_1972)
  __attribute__ ((pure));

/* Add days to a local time.  */
int
time_local_add_days (struct tm *time_tm, int addend,
		     int rounding_mode,
		     int variable_length_seconds_before_1972);

/* Add hours to a local time.  */
int
time_local_add_hours (struct tm *time_tm, int addend,
		      int rounding_mode,
		      int variable_length_seconds_before_1972);

/* Add minutes to a local time.  */
int
time_local_add_minutes (struct tm *time_tm, int addend,
			int rounding_mode,
			int variable_length_seconds_before_1972);

/* Add months to a local time.  */
int
time_local_add_months (struct tm *time_tm, int addend,
		       int rounding_mode,
		       int variable_length_seconds_before_1972);

/* Add seconds to a local time.  */
int
time_local_add_seconds (struct tm *time_tm,
			long long int add_seconds,
			int variable_length_seconds_before_1972);

/* Add seconds and nanoseconds to a local time.  */
int
time_local_add_seconds_ns (struct tm *time_tm,
			   long long int *nanoseconds,
			   long long int add_seconds,
			   long long int add_nanoseconds,
			   int variable_length_seconds_before_1972);

/* Add years to a local time.  */
int
time_local_add_years (struct tm *time_tm, int addend,
		      int rounding_mode,
		      int variable_length_seconds_before_1972);

/* Make sure all of the fields of a tm structure containing 
 * local time are within their valid ranges.  */
int
time_local_normalize (struct tm *time_tm,
		      long long int seconds,
		      int variable_length_seconds_before_1972);

/* Convert local time to Coordinated Universal Time.  */
int
time_local_to_UTC (struct tm *local_time,
		   struct tm *coordinated_universal_time,
		   int variable_length_seconds_before_1972);

/* Sleep until a specified Coordinated Universal Time.  */
int
time_sleep_until (struct tm *time_tm, int nanoseconds,
		  int variable_length_seconds_before_1972);

/* Convert the time and nanoseconds to a 128-bit integer.  
 */
int
time_tm_nano_to_integer (struct tm *input_tm,
			 int input_nanoseconds,
			 __int128 *result);

/* Convert the time and nanoseconds to a string.  */
int
time_tm_nano_to_string (struct tm *input_tm,
			int input_nanoseconds,
			char *current_time_string,
			int current_time_string_length);

/* Convert the time to a long long integer.  */
int
time_tm_to_integer (struct tm *input_tm,
		    long long int *result);

/* Convert the time to a string.  */
int
time_tm_to_string (struct tm *input_tm,
		   char *current_time_string,
		   int current_time_string_length);

/* Add days to a Coordinated Universal Time.  */
int
time_UTC_add_days (struct tm *time_tm, int addend,
		   int rounding_mode,
		   int variable_length_seconds_before_1972);

/* Add hours to a Coordinated Universal Time.  */
int
time_UTC_add_hours (struct tm *time_tm, int addend,
		    int rounding_mode,
		    int variable_length_seconds_after_1972);

/* Add minutes to a Coordinated Universal Time.  */
int
time_UTC_add_minutes (struct tm *time_tm, int addend,
		      int rounding_mode,
		      int variable_length_seconds_before_1972);

/* Add months to a Coordinated Universal Time.  */
int
time_UTC_add_months (struct tm *time_tm, int addend,
		     int rounding_mode,
		     int variable_length_seconds_before_1972);

/* Add seconds to a Coordinated Universal Time.  */
int
time_UTC_add_seconds (struct tm *time_tm,
		      long long int add_seconds,
		      int variable_length_seconds_before_1972);

/* Add seconds and nanoseconds to a Coordinated Universal Time.  
 */
int
time_UTC_add_seconds_ns (struct tm *time_tm,
			 long long int *nanoseconds,
			 long long int add_seconds,
			 long long int add_nanoseconds,
			 int variable_length_seconds_before_1972);

/* Add years to a Coordinated Universal Time.  */
int
time_UTC_add_years (struct tm *time_tm, int addend,
		    int rounding_mode,
		    int variable_length_seconds_before_1972);

/* Make sure all of the fields of a tm structure containing a
 * Coordinated Universal Time are within their valid ranges.  
 */
int
time_UTC_normalize (struct tm *time_tm,
		    long long int seconds,
		    int variable_length_seconds_before_1972);

/* Convert Coordinated Universal Time to local time.  */
int
time_UTC_to_local (struct tm *coordinated_universal_time,
		   struct tm *local_time,
		   int variable_length_seconds_before_1972);

#endif /* TIME_SUBROUTINES_H */

/* End of file time_subroutines.h */
