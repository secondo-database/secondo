
package viewer.hoese;


/** this class provides transformations between julian and
    gregorian calendar */


public class JulianDate{


public static int toJulian(int year, int month, int day)
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
   {  int jy = year;
      if (year < 0) jy++;
      int jm = month;
      if (month > 2) jm++;
      else
      {  jy--;
         jm += 13;
      }
      int jul = (int) (java.lang.Math.floor(365.25 * jy)
      + java.lang.Math.floor(30.6001*jm) + day + 1720995.0);
      int IGREG = 15 + 31*(10+12*1582);
         // Gregorian Calendar adopted Oct. 15, 1582
      if (day + 31 * (month + 12 * year) >= IGREG)
         // change over to Gregorian calendar
      {  int ja = (int)(0.01 * jy);
         jul += 2 - ja + (int)(0.25 * ja);
      }
      return jul-NULL_DAY;
   }


/** returns an array [year,month,day] */

public static int[] fromJulian(int j)
   /**
    * Converts a Julian day to a calendar date
    * @param j  the Julian date
    * This algorithm is from Press et al., Numerical Recipes
    * in C, 2nd ed., Cambridge University Press 1992
    */
   {  j=j+NULL_DAY;
      int ja = j;
      int JGREG = 2299161;
         /* the Julian date of the adoption of the Gregorian
            calendar
         */
      if (j >= JGREG)
      /* cross-over to Gregorian Calendar produces this
         correction
      */
      {  int jalpha = (int)(((float)(j - 1867216) - 0.25)
             / 36524.25);
         ja += 1 + jalpha - (int)(0.25 * jalpha);
      }
      int jb = ja + 1524;
      int jc = (int)(6680.0 + ((float)(jb-2439870) - 122.1)
          /365.25);
      int jd = (int)(365 * jc + (0.25 * jc));
      int je = (int)((jb - jd)/30.6001);
      int day = jb - jd - (int)(30.6001 * je);
      int month = je - 1;
      if (month > 12) month -= 12;
      int year = jc - 4715;
      if (month > 2) --year;
      if (year <= 0) --year;
      int[] res ={year,month,day};
      return res;
   }

public static int NULL_DAY = 2451544; // make compatible with old hoese representation
}
