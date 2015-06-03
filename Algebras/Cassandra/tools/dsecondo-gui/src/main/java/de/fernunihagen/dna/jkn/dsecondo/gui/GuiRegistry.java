package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.util.Map;

public class GuiRegistry {
	protected static GuiRegistry instance;
	
	protected Map<String, CassandraNode> cassandraNodes;
	protected int observedQueryId = 1;
	
	public static synchronized GuiRegistry getInstance() {
		if(instance == null) {
			instance = new GuiRegistry();
		}
		
		return instance;
	}
	
	@Override
	protected Object clone() throws CloneNotSupportedException {
		throw new UnsupportedOperationException("Cloning a singleton is not supported");
	}
	
	protected GuiRegistry() {
		// Protected constructor
	}

	public Map<String, CassandraNode> getCassandraNodes() {
		return cassandraNodes;
	}

	public void setCassandraNodes(Map<String, CassandraNode> cassandraNodes) {
		this.cassandraNodes = cassandraNodes;
	}

	public int getObservedQueryId() {
		return observedQueryId;
	}

	public void setObservedQueryId(int observedQueryId) {
		this.observedQueryId = observedQueryId;
	}
	
}
