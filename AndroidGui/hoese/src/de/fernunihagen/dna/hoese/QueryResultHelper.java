package de.fernunihagen.dna.hoese;

import java.util.List;

public class QueryResultHelper {
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
}
