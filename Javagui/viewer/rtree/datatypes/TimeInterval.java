package viewer.rtree.datatypes;
import sj.lang.*;

/**
 * The TimeIntervall class is used to convert a secondo-period list to a time coordinate.
 * 
 * It uses the DatePixelCalculator to do the transformation from date to coordinates.
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 23.01.2010
 *
 */
public class TimeInterval {

	private Instant startDate;
	private Instant endDate;
	
	public TimeInterval(Instant startDate, Instant endDate)
	{
		this.startDate = startDate;
		this.endDate = endDate;
	}
	
	public String toString()
	{
		return "(" + startDate.toString() + "\n" + endDate.toString() + ")";
	}
		
	public static TimeInterval fromListExpr(ListExpr le)
	{
		Instant startDate = Instant.readInstant(le.first());
		Instant endDate = Instant.readInstant(le.second());
		
		return new TimeInterval(startDate, endDate);
	}
	
	public int getStartCoordinate(double scaleFactor)
	{
		return startDate.toPixel();
	}
	
	public int getEndCoordinate(double scaleFactor)
	{
		return endDate.toPixel();
	}
}
