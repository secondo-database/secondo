package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import java.util.Set;

import com.datastax.driver.core.Cluster;
import com.datastax.driver.core.ConsistencyLevel;
import com.datastax.driver.core.Host;
import com.datastax.driver.core.Metadata;
import com.datastax.driver.core.ResultSet;
import com.datastax.driver.core.Session;
import com.datastax.driver.core.SimpleStatement;
import com.datastax.driver.core.Statement;

public class CassandraClient implements AutoCloseable {

	private Cluster cluster;
	private Session session;

	/**
	 * Connect to the cassandra cluster
	 * @throws Exception
	 */
	public void connect() throws Exception {
		
		Properties prop = loadProperties();		
		String node = prop.getProperty("node");
		String keyspace = prop.getProperty("keyspace");
		
		cluster = Cluster.builder().addContactPoint(node).build();
	    session = cluster.connect();
	    
	    session.execute("USE " + keyspace + ";");
	}

	/**
	 * Load properties data with the connection data for cassandra
	 * @return Properties
	 * @throws IOException
	 * @throws FileNotFoundException
	 */
	protected Properties loadProperties() throws IOException,
			FileNotFoundException {
		Properties prop = new Properties();
		String propFileName = "cassandra.properties";
 
		InputStream inputStream = getClass().getClassLoader().getResourceAsStream(propFileName);
 
		if (inputStream != null) {
			prop.load(inputStream);
		} else {
			throw new FileNotFoundException("property file '" + propFileName + "' not found in the classpath");
		}
		return prop;
	}
	
	/**
	 * Get all Hosts of the Cassandra cluster
	 * @return Hosts of the Cassandra cluster
	 */
	Set<Host> getAllHosts() {		
		Metadata metadata = cluster.getMetadata();
		return metadata.getAllHosts();
	}

	/** 
	 * Get Heartbeat data of the DSECONDO nodes
	 * @return heartbeat data
	 */
	ResultSet getNodeHearbeat() {
		ResultSet result = session.execute("SELECT ip, heartbeat, lastquery from system_state");
		return result;
	}
	
	/**
	 * Get all schedules quries
	 * @return The global execution plan of DSECONDO
	 */
	ResultSet getQueries() {
		ResultSet result = session.execute("SELECT id, query, version from system_queries");
		return result;
	}
	
	/**
	 * Get the processing status for a specific query
	 * @param queryId
	 * @return Processed Token Ranges for the query
	 */
	ResultSet getTokenProcessedRangesForQuery(int queryId) {
		Statement statement = new SimpleStatement("SELECT ip, begintoken, endtoken from system_progress where queryid = " + queryId)
		                             .setConsistencyLevel(ConsistencyLevel.QUORUM);
		ResultSet result = session.execute(statement);
		return result;
	}
	
	/**
	 * Get the total amount of token ranges
	 * @return Amount of token ranges
	 */
	int getTotalTokenRanges() {
		ResultSet result = session.execute("SELECT ip from system_tokenranges");
		return result.getAvailableWithoutFetching();
	}

	/**
	 * Close the connection to the cassandra cluster
	 */
	public void close() {
	      session.close();
	      cluster.close();
	}

}
