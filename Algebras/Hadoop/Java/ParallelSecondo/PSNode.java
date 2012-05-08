package ParallelSecondo;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

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
		throws IOException, InterruptedException
	{
		String localAddr = InetAddress.getLocalHost().getHostAddress();
		String slFile = System.getenv().get("PARALLEL_SECONDO_SLAVES");
		
		if (slFile.length() == 0)
		{
			throw new RuntimeException(
					"Undefined PARALLEL_SECONDO_SLAVES in " + localAddr);
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

		if ( candidate > 0 ){
			return slaves.get(candidate -1 );
		}
		else {
			int pCnt = Integer.MAX_VALUE;
			for (PSNode slave : slaves){
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
	
}
