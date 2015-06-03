package de.fernunihagen.dna.jkn.dsecondo.gui;

public class CassandraQuery {
	protected int id;
	protected String query;
	protected long version;
	
	public CassandraQuery(int id, String query, long version) {
		super();
		this.id = id;
		this.query = query;
		this.version = version;
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public String getQuery() {
		return query;
	}

	public void setQuery(String query) {
		this.query = query;
	}

	public long getVersion() {
		return version;
	}

	public void setVersion(long version) {
		this.version = version;
	}
}