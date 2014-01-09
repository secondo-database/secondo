

public class PSNode {

	String ipAddr, fileLoc;
	int sn, portNum;  //serial number
	
	public PSNode(int sn, String ipAddr, String fileLoc, int portNum) {
		super();
		this.sn = sn;
		this.ipAddr = ipAddr;
		this.fileLoc = fileLoc;
		this.portNum = portNum;
	}

	public String getIpAddr() {
		return ipAddr;
	}

	public String getFileLoc() {
		return fileLoc;
	}

	public int getPortNum() {
		return portNum;
	}

	public int getSn() {
		return sn;
	}
	
	
	
}
