package ParallelSecondo;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.Random;

public class PSNode implements Constant{

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
	
	
	public void setNode(int sn, String ipAddr, String fileLoc, int portNum){
		this.sn = sn;
		this.ipAddr = ipAddr;
		this.fileLoc = fileLoc;
		this.portNum = portNum;
	}
	
	public PSNode(PSNode rhg){
		this(rhg.getSn(), rhg.getIpAddr(), rhg.getFileLoc(), rhg.getPortNum());
	}
	
	/**
	 * Find a light weight data server on one node, 
	 * in order to get a even overload on cores inside one computer.
	 * 
	 * @param candidate
	 * @return
	 * @throws IOException 
	 * @throws InterruptedException 
	 */
	public static PSNode SelectDataServer(int candidate) 
		throws IOException, InterruptedException{
            return SelectDataServer(candidate, -1);
        }

	public static PSNode SelectDataServer(int candidate, int taskId) 
		throws IOException, InterruptedException
	{
		String slFile = System.getenv().get("PARALLEL_SECONDO_SLAVES");
		String localHost = InetAddress.getLocalHost().getHostName();
		if (slFile.length() == 0)
		{
			throw new RuntimeException(
					"Undefined PARALLEL_SECONDO_SLAVES in " + localHost);
		}
		
		List<PSNode> slaves = new ArrayList<PSNode>();
		try {		
			Scanner scanner;
			scanner = new Scanner(new FileInputStream(slFile));
			int lineNum = 0;
			while (scanner.hasNextLine()){
				String[] line = scanner.nextLine().split(sysDim);
				slaves.add(new PSNode(lineNum++, line[0], line[1], 
						Integer.parseInt(line[2])));
			}
		} catch (FileNotFoundException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		String localAddr = guessLocalIP(slaves);
		if (localAddr.length() == 0)
			return null;

		if ( candidate > 0 ){
			return slaves.get(candidate -1 );
		}
                else if(taskId >= 0){
                  int localSlavesNum = 0;
                  for (PSNode slave : slaves){
                    if (localAddr.compareTo(slave.getIpAddr()) == 0){
                      localSlavesNum++;
                    }
                  } 
                  Random r = new Random();
                  int localDSId = (taskId + r.nextInt(slaves.size())) % localSlavesNum;
                  int locDSCnt = 0;
                  for (PSNode slave : slaves){
                    if ((localAddr.compareTo(slave.getIpAddr()) == 0) 
                        && (locDSCnt++ == localDSId )){
                      return slave;
                    }                     
                  }
                  return null;
                }
		else {
			int pCnt = Integer.MAX_VALUE;
			for (PSNode slave : slaves){
				if (localAddr.compareTo(slave.getIpAddr()) != 0)
			          continue;
				int port = slave.getPortNum();
				Runtime rt = Runtime.getRuntime();
				Process proc = rt.exec(new String[]{"lsof", "-i", ":" + port});
				BufferedReader ibr = 
					new BufferedReader(new InputStreamReader(proc.getInputStream()));
				String line = null;
				int counter = 0;
				while ((line = ibr.readLine()) != null){
					if (line.contains("ESTABLISHED")){
						counter++;
					}
				}
				proc.waitFor();
				
				if (counter <= pCnt){
					return new PSNode(slave.getSn(), localAddr, slave.getFileLoc(), port);
				}
			}
			return null;
		}
	}
	
	/**
	 * Scan all the IP addresses of the current node, 
	 * and if it is shown in the slave list, 
	 * then return it as the localIP of the current node. 
	 * Or else, an empty string is returned. 
	 * @throws SocketException 
	 * 
	 * 
	 */
	public static String guessLocalIP(List<PSNode> slaves) 
		throws SocketException
	{
		List<String> allAvailableIP = new ArrayList<String>();
		
		for (Enumeration<NetworkInterface> ifaces = 
					NetworkInterface.getNetworkInterfaces();
    		ifaces.hasMoreElements(); )
		{
			NetworkInterface iface = ifaces.nextElement();
      for (Enumeration<InetAddress> addresses = 
      			iface.getInetAddresses(); 
      		addresses.hasMoreElements(); )
      {
      	InetAddress address = addresses.nextElement();
      	allAvailableIP.add(address.toString().substring(1));
      }
		}
		
		for (PSNode node : slaves)
		{
			if (allAvailableIP.contains(node.getIpAddr())){
				return node.getIpAddr();
			}
		}
		
		return "";
	}

	public static PSNode getMasterNode() throws FileNotFoundException
	{
		String smFile = System.getenv().get("PARALLEL_SECONDO_MASTER");
		if (smFile.length() == 0)
		{
			System.err.println(
				"The Master list PARALLEL_SECONDO_MASTER is not defined at current node.");
			System.exit(-1);
		}
		Scanner scanner = new Scanner(new FileInputStream(smFile));
		String masterIP = "", masterLoc = "";
		int masterPort = -1;
		if (scanner.hasNext())
		{
			String[] pms = scanner.nextLine().split(sysDim);
			masterIP 		 = pms[0];
			masterLoc		 = pms[1];
			masterPort   = Integer.parseInt(pms[2]);
		}

		return new PSNode(0, masterIP, masterLoc, masterPort);
	}
	
	
}
