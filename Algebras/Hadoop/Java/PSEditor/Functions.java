package PSEditor;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

public class Functions {

	static public String get_localIP()
	{
    //Get the local IP Address
    String localAddr = "";
  	Enumeration<NetworkInterface> interfaces;
		try {
			interfaces = NetworkInterface.getNetworkInterfaces();
			while (interfaces.hasMoreElements())
			{
				NetworkInterface current = interfaces.nextElement();
				if (!current.isUp() || current.isLoopback() || current.isVirtual()) continue;
				
				Enumeration<InetAddress> addresses = current.getInetAddresses();
				while (addresses.hasMoreElements()){
					InetAddress current_addr = addresses.nextElement();
					if (current_addr.isLoopbackAddress()) continue;
					if (current_addr instanceof Inet4Address){
						if (!localAddr.isEmpty()){
							System.err.println("Warning! There exist multiple addresses, " +
									"including : " + localAddr + " and " + current_addr.getHostAddress());
						}
						localAddr = current_addr.getHostAddress();
						
					}
				}
			}
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(-1);
		}
		
		return localAddr;
	}

	
}
