package de.fernunihagen.dna.hoese;

import java.util.List;
import java.util.Vector;

import javamini.awt.geom.Rectangle2D;

public class QueryResultHelper {
	
	public enum CoordinateSystem {
		GEO,
		WORLD
	}
	
	public static Interval getBoundingInterval(List<QueryResult> queryResults) {
		Interval interval = null;

		for (QueryResult queryResult : queryResults) {
			if (interval == null) {
				interval = queryResult.getBoundingInterval();
			} else {
				interval.unionInternal(queryResult.getBoundingInterval());
			}
		}

		return interval;
	}

	public static  int getMaxIntervalCount(List<QueryResult> queryResults) {
		int maxCount = 0;

		for (QueryResult queryResult : queryResults) {
			maxCount = Math.max(maxCount, queryResult.getIntervalCount());
		}
		return maxCount;
	}

	public static Rectangle2D.Double getBounds(List<QueryResult> queryResults) { 
		Rectangle2D.Double totalBound = null;
		
		for (QueryResult queryResult : queryResults) {
			for (DsplBase base : queryResult.getEntries()) {
				if (base instanceof DsplGraph) {
					Rectangle2D.Double bound = ((DsplGraph) base).getBounds();
					if (totalBound == null)
						totalBound = bound;
					else
						Rectangle2D.Double.union(bound, totalBound, totalBound);
				}
			}
		}
		
	if (totalBound == null) {
		return new Rectangle2D.Double(); // Empty bounding Rectangle
	}
	
	return totalBound;
	}

	
	public static CoordinateSystem getCoordinateSystem(List<QueryResult> queryresults) {
		Rectangle2D.Double bound = QueryResultHelper.getBounds(queryresults);
		Rectangle2D.Double geoBound = new Rectangle2D.Double(-180, -90, 360, 180);
		boolean geo = geoBound.contains(bound);
		return geo ? CoordinateSystem.GEO : CoordinateSystem.WORLD; // Graphicmode
	}

}
