package com.secondo.webgui.utils;

/** this class provides transformations between julian and
gregorian calendar */


public class JulianDate{


public static long toJulian(int year, int month, int day)
/**
* @return The Julian day number that begins at noon of
* this day
* Positive year signifies A.D., negative year B.C.
* Remember that the year after 1 B.C. was 1 A.D.
*
* A convenient reference point is that May 23, 1968 noon
* is Julian day 2440000.
*
* Julian day 0 is a Monday.
*
* This algorithm is from Press et al., Numerical Recipes
* in C, 2nd ed., Cambridge University Press 1992
*/
{  long jy = year;
  if (year < 0) jy++;
  long jm = month;
  if (month > 2) jm++;
  else
  {  jy--;
     jm += 13;
  }
  long jul = (long) (java.lang.Math.floor(365.25 * jy)
  + java.lang.Math.floor(30.6001*jm) + day + 1720995.0);
  long IGREG = 15 + 31*(10+12*1582);
     // Gregorian Calendar adopted Oct. 15, 1582
  if (day + 31 * (month + 12 * year) >= IGREG)
     // change over to Gregorian calendar
  {  long ja = (long)(0.01 * jy);
     jul += 2 - ja + (long)(0.25 * ja);
  }
  return jul-NULL_DAY;
}


/** returns an array [year,month,day] */

public static int[] fromJulian(long j)
/**
* Converts a Julian day to a calendar date
* @param j  the Julian date
* This algorithm is from Press et al., Numerical Recipes
* in C, 2nd ed., Cambridge University Press 1992
*/
{  j=j+NULL_DAY;
  long ja = j;
  long JGREG = 2299161;
     /* the Julian date of the adoption of the Gregorian
        calendar
     */
  if (j >= JGREG)
  /* cross-over to Gregorian Calendar produces this
     correction
  */
  {  long jalpha = (long)(((float)(j - 1867216) - 0.25)
         / 36524.25);
     ja += 1 + jalpha - (long)(0.25 * jalpha);
  }
  long jb = ja + 1524;
  long jc = (long)(6680.0 + ((float)(jb-2439870) - 122.1)
      /365.25);
  long jd = (long)(365 * jc + (0.25 * jc));
  long je = (long)((jb - jd)/30.6001);
  long day = jb - jd - (long)(30.6001 * je);
  long month = je - 1;
  if (month > 12) month -= 12;
  long year = jc - 4715;
  if (month > 2) --year;
  if (year <= 0) --year;
  int[] res ={(int)year,(int)month,(int)day};
  return res;
}

public static int NULL_DAY = 2451547; // make compatible with old hoese representation
}

