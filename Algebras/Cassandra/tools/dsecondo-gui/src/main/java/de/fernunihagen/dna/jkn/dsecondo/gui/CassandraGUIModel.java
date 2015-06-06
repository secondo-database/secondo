package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.datastax.driver.core.Host;
import com.datastax.driver.core.ResultSet;
import com.datastax.driver.core.Row;

public class CassandraGUIModel {
	protected List<CassandraQuery> queries;
	protected Map<String, Integer> tokenRanges; 
	protected Map<String, CassandraSystemState> systemState;
	protected int observedQueryId = 1;
	protected CassandraClient client;
	
	public CassandraGUIModel(CassandraClient client) {
		super();
		this.client = client;
		
		queries = new ArrayList<CassandraQuery>();
		tokenRanges = new HashMap<String, Integer>();
		systemState = new HashMap<String, CassandraSystemState>();
	}
	
	/**
	 * A Cache for cassandra data, e.g. the GEP and 
	 * the heartbeat data
	 */
	public synchronized void updateModel() {
		updateQueries();
		updateTokenranges();
		updateSystemState();
	}

	/**
	 * Update query data
	 */
	protected void updateQueries() {
		final ArrayList<CassandraQuery> newQueries = new ArrayList<CassandraQuery>();
		ResultSet queryResult = client.getQueries();
		for(Row row : queryResult) {
			newQueries.add(new CassandraQuery(row.getInt(0), row.getString(1), row.getLong(2)));
		}
		
		Collections.sort(newQueries, new Comparator<CassandraQuery>() {
			public int compare(CassandraQuery o1, CassandraQuery o2) {
				return Integer.compare(o1.getId(), o2.getId());
			}
		});
		queries = newQueries;
	}

	/**
	 * Update token range data
	 */
	protected void updateTokenranges() {
		final HashMap<String, Integer> newTokenRanges = new HashMap<String, Integer>();
		ResultSet tokenResult = client.getTokenProcessedRangesForQuery(observedQueryId);
		for(Row row : tokenResult) {
			String ip = row.getString(0);
			if(newTokenRanges.get(ip) == null) {
				newTokenRanges.put(ip, 1);
			} else {
				newTokenRanges.put(ip, (newTokenRanges.get(ip)+1));
			}
		}
		tokenRanges = newTokenRanges;
	}

	/**
	 * Update heartbeat data
	 */
	protected void updateSystemState() {
		final HashMap<String, CassandraSystemState> newSystemState = new HashMap<String, CassandraSystemState>();
		ResultSet heartbeatSet = client.getSystemState();
		for(Row row: heartbeatSet) {
			String ip = row.getString(0);
			long heartbeatValue = row.getLong(1);
			String cputype = row.getString(2);
			int memory = row.getInt(3);
			int threads = row.getInt(4);
			
			final CassandraSystemState systemState = new CassandraSystemState(heartbeatValue, cputype, memory, threads);
			newSystemState.put(ip, systemState);
		}
		systemState = newSystemState;
	}
	
	/**
	 * Get all hosts
	 * @return
	 */
	Set<Host> getAllHosts() {
		return client.getAllHosts();
	}
	
	
	/**
	 * Get the scheduled queries from cache
	 * @return scheduled queries
	 */
	public List<CassandraQuery> getQueries() {
		return Collections.unmodifiableList(queries);
	}
	
	/**
	 * Get the token ranges for the current observed query
	 * @return
	 */
	public Map<String, Integer> getTokenCache() {
		return Collections.unmodifiableMap(tokenRanges);
	}

	/**
	 * Get the observed query id
	 */
	public int getObservedQueryId() {
		return observedQueryId;
	}

	/**
	 * Set the observed query id
	 * @param observedQueryId
	 */
	public void setObservedQueryId(int observedQueryId) {
		this.observedQueryId = observedQueryId;
	}

	/**
	 * Get the node heartbeat
	 * @return
	 */
	public Map<String, CassandraSystemState> getNodeHeartbeat() {
		return systemState;
	}

	/**
	 * Get all token ranges
	 * @return
	 */
	public int getTotalTokenRanges() {
		return client.getTotalTokenRanges();
	}
}
