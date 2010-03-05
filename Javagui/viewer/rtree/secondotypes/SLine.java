package viewer.rtree.secondotypes;

import sj.lang.ListExpr;
import viewer.rtree.datatypes.Segment;

import java.util.*;

/**
 * The SLine class represents the Secondo datatype sline from the SpatialAlgebra.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 21.1.2010
 */
public class SLine extends LineBase 
{
	// constructor
	
	/**
	 * Creates a new SLine object from a given list of segments.
	 * @param segments List of segments
	 */
	public SLine(LinkedList<Segment> segments)
	{
		super(segments);
	}
	
	// public methods
	
	/**
	 * Returns the string representation of an SLine object.
	 * @return String representation of an SLine object
	 */
	public String toString()
	{
		String result = "SLine(";
		result += segmentsToString();
		result += ")";
		
		return result;
	}
	
	/**
	 * Converts a given list expression into an SLine object.
	 * @param le List representation of an SLine object
	 * @return SLine object
	 */
	public static SLine fromListExpr(ListExpr le)
	{
		LinkedList<Segment> segments = readSegmentsFromListExpr(le);
		return new SLine(segments);
	}
}
