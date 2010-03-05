package viewer.rtree.datatypes;
import java.util.Date;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.StringTokenizer;

/**
 * The DatePixelCalculator class offers a method to convert secondo dates to pixels.
 * This is necessary to display Temporal and SpatialTemporal objects.
 * 
 * Temporal objects always have temporal attributes which can be choosen as projection axis.
 * In order to display the temporal axis, the string-dates have to be converted to pixels.
 *
 * To transform dates to pixels, DatePixelCalculator calculates the seconds between
 * the date and the date 1.1.2000. The difference in seconds between the dates
 * is the result of the transformation. 
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 21.01.2010
 */
public class DatePixelCalculator {

	private static Date pixel0Date;
	
	public DatePixelCalculator()
	{
		pixel0Date = getPixel0Date();
	}
	
	/**
	 * Converts a date to pixels
	 * @param secondoDate Secondo date string YYYY-MM-DD-HH:MM:SS.xxx.. (x arbritrary precision)
	 * @param scaleFactor The result is scaled by this result
	 * @return Result of transformation from date to pixel (see above)
	 */
	public int dateToPixel(String secondoDate)
	{
		Date date = secondoDateToDate(secondoDate);
		
		return dateToPixel(date);
	}
	
	/**
	 * Converts a date to pixels
	 * @param date A java date
	 * @param scaleFactor The result is scaled by this result
	 * @return Result of transformation from date to pixel (see above)
	 */
	public int dateToPixel(Date date)
	{
		int distance = (int) (( date.getTime() - pixel0Date.getTime()) / 1000);

		return distance;
	}
	
	
	/**
	 * This method returns the java date 2000.01.01.00.00.00
	 * @return see above
	 */
	private Date getPixel0Date()
	{
		// 0 Pixel Date:
		Calendar calendar = new GregorianCalendar(2000, 00, 01, 00, 00, 00);
		return calendar.getTime();
	}
	
	
	/**
	 * Converts a secondo date to a java date
	 * @param secondoDate Format YYYY-MM-DD-HH:MM:SS.xxx.. (x arbritrary precision)
	 * @return
	 */
	public Date secondoDateToDate(String secondoDate)
	{
		// Transform secondoDate to Date
		int year, month, dayOfMonth; 
		int hourOfDay = 0;
		int minute = 0;
		int second = 0;
		
		StringTokenizer tokenizer = new StringTokenizer(secondoDate, "-:.");
		
		// Extract first three tokens: Year, Month, Day
		year 	= Integer.valueOf( tokenizer.nextToken() );
		month 	= Integer.valueOf( tokenizer.nextToken() ) - 1; // Java beginnt bei Monat 0
		dayOfMonth 	= Integer.valueOf( tokenizer.nextToken() );
		
		// Extract hour, minute, second, if avaiable
		if ( tokenizer.hasMoreElements() ) 
		{
			hourOfDay = Integer.valueOf( tokenizer.nextToken() );
		}
		
		if ( tokenizer.hasMoreElements() ) 
		{
			minute = Integer.valueOf( tokenizer.nextToken() );
		}
		
		if ( tokenizer.hasMoreElements() ) 
		{
			second = Integer.valueOf( tokenizer.nextToken() );
		}
		
		
		// Construct date
		Calendar calendar = new GregorianCalendar(year, month, dayOfMonth, 
													hourOfDay, minute, second);
		
		return calendar.getTime();
		
	}
}
