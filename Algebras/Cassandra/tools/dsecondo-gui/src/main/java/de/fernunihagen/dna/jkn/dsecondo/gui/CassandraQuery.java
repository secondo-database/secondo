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

	@Override
	public String toString() {
		return "CassandraQuery [id=" + id + ", query=" + query + ", version="
				+ version + "]";
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + id;
		result = prime * result + ((query == null) ? 0 : query.hashCode());
		result = prime * result + (int) (version ^ (version >>> 32));
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		CassandraQuery other = (CassandraQuery) obj;
		if (id != other.id)
			return false;
		if (query == null) {
			if (other.query != null)
				return false;
		} else if (!query.equals(other.query))
			return false;
		if (version != other.version)
			return false;
		return true;
	}
	
	
}