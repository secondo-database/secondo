package viewer.rtree.datatypes;

import sj.lang.*;
import java.util.Date;

/**
 * The instant class is used to convert a secondo-instant list to a time coordinate.
 * 
 * It uses the DatePixelCalculator to do the transformation from date to coordinates.
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 23.01.2010
 *
 */
public class Instant {

	private DatePixelCalculator calc;
	private Date date;

	public Instant( String secondoDate )
	{
		calc = new DatePixelCalculator();
		date = calc.secondoDateToDate(secondoDate);
	}
	
	public String toString()
	{
		 return "(" + getDate() + " " + toPixel() + ")";
	}

	
	public static boolean isInstant(ListExpr le)
	{
		if ( le.isAtom() && 
			 le.atomType() == ListExpr.STRING_ATOM )
		{
			return true;
		}
		
		return false;
	}
	
	
	public static Instant readInstant(ListExpr le)
	{
		return new Instant( le.stringValue() );
	}
	
	public static Instant readInstantDatatype(ListExpr le)
	{
		return readInstantDatatype(le.second());
	}
	
	public int toPixel()
	{
		return calc.dateToPixel(date);
	}
	
	public Date getDate()
	{
		return date;
	}
}
