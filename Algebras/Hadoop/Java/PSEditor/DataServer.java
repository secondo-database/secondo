package PSEditor;

public class DataServer {

	public enum ROLES{MASTER, SLAVE, MASLAVE;

	/* (non-Javadoc)
	 * @see java.lang.Enum#toString()
	 */
	@Override
	public String toString() {
		// TODO Auto-generated method stub
		switch (this){
		case MASTER: 
			return "Master";
		case SLAVE:
			return "Slave";
		case MASLAVE: 
			return "Master & Slave";
		default:
			return "Unknown";
		}
	} 
	};

	private String ip, path;
	private int port;
	private ROLES role; 
	
	public DataServer(String i, String p, int t, ROLES r)
	{
		ip = i;
		path = p;
		port = t;
		role = r;
	}
	
	
	
	/**
	 * @return the ip
	 */
	public String getIp() {
		return ip;
	}



	/**
	 * @return the path
	 */
	public String getPath() {
		return path;
	}



	/**
	 * @return the port
	 */
	public int getPort() {
		return port;
	}



	/**
	 * @return the role
	 */
	public ROLES getRole() {
		return role;
	}
	
	public String toString()
	{
		return (ip + ":" + path + ":" + port);
	}

	@Override
	public boolean equals(Object v){
		DataServer value = (DataServer)v;
		if ((ip.compareTo(value.getIp()) == 0) 
				&& (path.trim().compareTo(value.getPath().trim()) == 0)
				&& (port == value.getPort())){
			
			if ( role == value.getRole()
					|| role == ROLES.MASLAVE 
					|| value.getRole() == ROLES.MASLAVE )
				return true;
		}

		return false;
	}
	
	public boolean isMaster()
	{
		if (role != ROLES.SLAVE)
			return true;
		else
			return false;
	}
	
	public boolean isSlave()
	{
		if (role != ROLES.MASTER)
			return true;
		else
			return false;
	}
}
