package ParallelSecondo;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.util.Scanner;

/**
 * 
 * Create and get zeroTuple for HDJ. 
 * Since this file must be encoded in Java program.  
 * 
 * @author Jiamin
 *
 */

public class HDJ_GetZTuple 
implements Constant{

	public static void getZeroTuple(String zTupleFileName)
	{
		try{
		int sendPort = HPA_AuxFunctions.getPort();
		System.out.println("Get send port is: " + sendPort );
		
		// 3. Get the binary data of 0-tuple from master's meta database
		String[] queries = { 
				"query intstream(0,1) transformstream intstream(1,0) transformstream " +
				"doubleexport[Elem, Elem] add0Tuple head[2] tail[1] send[" + sendPort
				+ ", KeyT]" };
		
		PSNode masterNode = PSNode.getMasterNode();
		String masterIP  	= masterNode.getIpAddr(), 
					 masterLoc 	= masterNode.getFileLoc();
		int    masterPort = masterNode.getPortNum();
		
		
		SecExRunnable ztupSender = new SecExRunnable(masterIP, metaDBName,
				masterPort, queries);  //Change the port if necessary
		if (!ztupSender.isInitialized())
			throw new RemoteStreamException("Get Exception while get 0Tuple");

		new Thread(ztupSender).start();

		byte[] tupleBuffer = new byte[RemoteStream.MAX_TUPLESIZE];
		byte[] zTuple = new byte[RemoteStream.SOCKTUP_SIZE];
		int offset = 0, zTupleSize = 0;
		RemoteStream ztupRecv = new RemoteStream("client", "localhost",
				sendPort);
		ztupRecv.Connect();
		if (!ztupRecv.getConnected()) {
			System.err.println("Error: Can't access to the 0tuple Sender");
			System.exit(-1);
		} else {
			String typeInfo = ztupRecv.readLine();
			ztupRecv.writeLine("<GET TYPE/>");
			int keyLoc = typeInfo.indexOf("APPEND");
			if (keyLoc < 0) {
				System.err.println("Get typeInfo: " + typeInfo);
				throw new RemoteStreamException(
						"Error: Expect appended key attribute");
			}
			String keyType = typeInfo.substring(keyLoc + 7);
			if (!keyType.equals("string"))
				throw new RemoteStreamException(
						"Error: Expect string type key attribute");
			ztupRecv.setKeyType(keyType);

			if (ztupRecv.receiveSocket(tupleBuffer, 0))
				throw new RemoteStreamException(
						"Error: Overmuch zero_Tuple data.");


			int keySize = RemoteStream.Byte2Int(tupleBuffer, offset);
			offset += 4;
			if (keySize <= 0)
				throw new RemoteStreamException(
						"Error: invalide zero_Tuple key size.");
			String zTupleKeyValue = RemoteStream.Byte2String(tupleBuffer,
					offset, keySize);
			offset += keySize;
			if (!zTupleKeyValue.equals("0Tuple")) {
				System.err.println("zTupleKeyValue is:" + zTupleKeyValue);
				throw new RemoteStreamException(
						"Error: invalide zero_Tuple key value.");
			}
			// short zTupleSize =
			RemoteStream.Byte2UnsignedShort(tupleBuffer, offset);
			zTupleSize = RemoteStream.Byte2Int(tupleBuffer, offset);
			// zTupleSize += 4; 
			if( zTupleSize > RemoteStream.SOCKTUP_SIZE ){
				System.err.println("zTupleSize is:" + zTupleSize);
				throw new RemoteStreamException("Error: Overfill zero_Tuple size.");
			}
		System.arraycopy(tupleBuffer, offset, zTuple, 0, zTupleSize);
		File f = new File(masterLoc + "/" + zTupleFileName);
		FileOutputStream fout = new FileOutputStream(f);
		fout.write(zTuple, 0, zTupleSize);
		fout.close();
		
		String hostName = InetAddress.getLocalHost().getHostName();
		System.out.print(hostName + " through port " + sendPort);
		System.out.println(" get zero_tuple with size: " + zTupleSize);
		ztupRecv.receiveSocket(tupleBuffer, 0);
		if (!ztupRecv.getTheLastSocket())
			throw new RemoteStreamException(
					"Error: can't accept the END socket");

		ztupRecv.close();
		ztupRecv = null;
		}
	}
		catch(IOException e){
			e.printStackTrace();
			System.exit(-1);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(-1);
		}
	}
}
