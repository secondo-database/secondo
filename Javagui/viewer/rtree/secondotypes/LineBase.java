package viewer.rtree.secondotypes;
import java.awt.Graphics;
import java.util.*;
import sj.lang.*;
import viewer.rtree.datatypes.Segment;
import viewer.rtree.datatypes.TypeBase;

/**
 * This is an abstract base class for the Line and SLine Secondo datatypes.
 * 
 * It provides functionality to extract segments from a list expression and
 * a way to draw a set of segments.
 * 
 * @author Benedikt Buer
 * @version 1.1
 * @author Christian Oevermann
 * @since 21.01.2010
 */
public abstract class LineBase extends TypeBase 
{
	protected LinkedList<Segment> segments; 
	
	// constructors
	
	/**
	 * Creates a new LineBase object from a given list of segments.
	 * @param segments List of segments
	 */
	public LineBase(LinkedList<Segment> segments)
	{
		this.segments = segments;
	}
	
	// public methods
	
	/**
	 * Returns the string representation of a list of segments.
	 * @return String representation of a list of segments
	 */
	public String segmentsToString()
	{
		String result = "";
		
		for (Segment s: segments)
		{
			result += s.toString();
		}
		
		return result;
	}
	
	/**
	 * Checks whether a list expression represents a set of segments.
	 * @param le List expression
	 * @return True if the list contains only segments, otherwise false
	 */
	public static boolean isListOfSegments(ListExpr le)
	{
		ListExpr listOfSegments = le;
	
		while (!listOfSegments.isEmpty())
		{
			if (!Segment.isSegment(listOfSegments.first()))
			{
				return false;
			}

			listOfSegments = listOfSegments.rest();
		}
		
		return true;
	}
	
	/**
	 * Extracts a list of segments from the given list expression.
	 * @param le List expression
	 * @return List of segments
	 */
	public static LinkedList<Segment> readSegmentsFromListExpr(ListExpr le)
	{
		ListExpr listOfSegments = le;
		LinkedList<Segment> result = new LinkedList<Segment>();
		
		while (!listOfSegments.isEmpty())
		{
			result.add(Segment.fromListExpr(listOfSegments.first()));
			
			listOfSegments = listOfSegments.rest();
		}
		
		return result;
	}
	
	/**
	 * Draws the segments.
	 * @param g Graphic context
	 */
	public void draw(Graphics g)
	{	
		for (Segment s : segments)
		{
			s.draw(g, projectionParameters, renderParameters, nodeStatus);
		}
	}
}
