package de.fernunihagen.dna.data;

import java.io.Serializable;
import java.util.Date;

/**
 * Datatransferobject for databaseserverproperties
 * @author Michael Küpper
 *
 */
public class DatabaseServerDto implements Serializable {

	private static final long serialVersionUID = 4155078262997831502L;

	private Integer id;
	private String databaseServer;
	private String userName;
	private String password;
	private Integer port;
	private String optServer;
	private Integer optPort;
	private boolean usingOptimizer;
	private Long lastUsing;
	
	public DatabaseServerDto() {
	}
	
	public DatabaseServerDto(String databaseServer,
			String userName, String password, Integer port, String optServer,
			Integer optPort, boolean usingOptimizer) {
		super();
		this.databaseServer = databaseServer;
		this.userName = userName;
		this.password = password;
		this.port = port;
		this.optServer = optServer;
		this.optPort = optPort;
		this.usingOptimizer = usingOptimizer;
		this.lastUsing = new Date().getTime();
	}

	
	public Integer getId() {
		return id;
	}
	public void setId(Integer id) {
		this.id = id;
	}
	
	public String getDatabaseServer() {
		return databaseServer;
	}
	public void setDatabaseServer(String databaseServer) {
		this.databaseServer = databaseServer;
	}
	public String getUserName() {
		return userName;
	}
	public void setUserName(String userName) {
		this.userName = userName;
	}
	public String getPassword() {
		return password;
	}
	public void setPassword(String password) {
		this.password = password;
	}
	public Integer getPort() {
		return port;
	}
	public void setPort(Integer port) {
		this.port = port;
	}
	public String getOptServer() {
		return optServer;
	}
	public void setOptServer(String optServer) {
		this.optServer = optServer;
	}
	public Integer getOptPort() {
		return optPort;
	}
	public void setOptPort(Integer optPort) {
		this.optPort = optPort;
	}
	public boolean isUsingOptimizer() {
		return usingOptimizer;
	}
	public void setUsingOptimizer(boolean usingOptimizer) {
		this.usingOptimizer = usingOptimizer;
	}
	
	@Override
	public String toString() {
		return getDatabaseServer() + " / " + getUserName();
	}
	public Long getLastUsing() {
		return lastUsing;
	}
	public void setLastUsing(Long lastUsing) {
		this.lastUsing = lastUsing;
	}
}
