package viewer.rtree.secondotypes;

import sj.lang.ListExpr;
import viewer.rtree.datatypes.Segment;

import java.util.*;

/**
 * The Line class represents the Secondo datatype line from the SpatialAlgebra.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 21.1.2010
 */
public class Line extends LineBase 
{
	// constructors
	
	/**
	 * Creates a new Line object from a set of segments.
	 * @param segments Set of segments
	 */
	public Line( LinkedList<Segment> segments)
	{
		super(segments);
	}
	
	// public methods
	
	/**
	 * Returns the string representation of a Line object.
	 * @return String representation of a Line object
	 */
	public String toString()
	{
		String result = "Line (";
		result += segmentsToString();
		result += ")";
		
		return result;
	}

	/**
	 * Converts a given list expression into a Line object.
	 * @param le List representation of a Line object
	 * @return Line object
	 */
	public static Line fromListExpr(ListExpr le)
	{
		LinkedList<Segment> segments = readSegmentsFromListExpr(le);
		return new Line(segments);
	}
}
