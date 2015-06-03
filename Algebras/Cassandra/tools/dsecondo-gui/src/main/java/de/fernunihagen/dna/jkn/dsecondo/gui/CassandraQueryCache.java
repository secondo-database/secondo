package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.datastax.driver.core.ResultSet;
import com.datastax.driver.core.Row;

public class CassandraQueryCache {
	protected List<CassandraQuery> queries;
	protected Map<String, Integer> tokenRanges; 
	
	protected CassandraClient client;
	
	public CassandraQueryCache(CassandraClient client) {
		super();
		this.client = client;
	}
	
	/**
	 * A Cache for cassandra data, e.g. the GEP and 
	 * the heartbeat data
	 */
	public synchronized void updateCache() {
		queries = new ArrayList<CassandraQuery>();
		tokenRanges = new HashMap<String, Integer>();
		
		ResultSet queryResult = client.getQueries();
		for(Row row : queryResult) {
			queries.add(new CassandraQuery(row.getInt(0), row.getString(1), row.getLong(2)));
		}
		
		Collections.sort(queries, new Comparator<CassandraQuery>() {
			public int compare(CassandraQuery o1, CassandraQuery o2) {
				return Integer.compare(o1.getId(), o2.getId());
			}
		});
		
		ResultSet tokenResult = client.getTokenProcessedRangesForQuery(GuiRegistry.getInstance().getObservedQueryId());
		for(Row row : tokenResult) {
			String ip = row.getString(0);
			if(tokenRanges.get(ip) == null) {
				tokenRanges.put(ip, 1);
			} else {
				tokenRanges.put(ip, (tokenRanges.get(ip)+1));
			}
		}
	}
	
	/**
	 * Get the scheduled queries from cache
	 * @return scheduled queries
	 */
	public List<CassandraQuery> getQueryCache() {
		return Collections.unmodifiableList(queries);
	}
	
	/**
	 * Get the token ranges for the current observed query
	 * @return
	 */
	public Map<String, Integer> getTokenCache() {
		return Collections.unmodifiableMap(tokenRanges);
	}
}
