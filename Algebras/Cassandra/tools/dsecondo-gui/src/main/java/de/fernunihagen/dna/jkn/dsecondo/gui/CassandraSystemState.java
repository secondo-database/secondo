package de.fernunihagen.dna.jkn.dsecondo.gui;

public class CassandraSystemState {
	protected long heartbeat;
	protected String cputype;
	protected int memory;
	protected int threads;
	
	public CassandraSystemState(long heartbeat, String cputype, int memory,
			int threads) {
		super();
		this.heartbeat = heartbeat;
		this.cputype = cputype;
		this.memory = memory;
		this.threads = threads;
	}

	public int getThreads() {
		return threads;
	}

	public void setThreads(int threads) {
		this.threads = threads;
	}



	public long getHeartbeat() {
		return heartbeat;
	}

	public void setHeartbeat(long heartbeat) {
		this.heartbeat = heartbeat;
	}

	public String getCputype() {
		return cputype;
	}

	public void setCputype(String cputype) {
		this.cputype = cputype;
	}

	public int getMemory() {
		return memory;
	}

	public void setMemory(int memory) {
		this.memory = memory;
	}
	
}
