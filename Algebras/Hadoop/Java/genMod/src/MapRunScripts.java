import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;
import java.util.Map;
import java.util.Scanner;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;

import sj.lang.*;


public class MapRunScripts extends Mapper<LongWritable, Text, IntWritable, Text> 
implements Constant{

  @Override
  protected void map(LongWritable key, Text value,
      Context context) throws IOException,
      InterruptedException
  {
    
    //Get the parameters set in the file
    String line = value.toString();
    String para[] = line.split(inDim);
    if (para.length != 11)
      throw new IOException("Error: Incorrect paramter amount");
    
    String secIP = para[0];
    int    secPt = Integer.parseInt(para[1]);
    String dbName = para[2];
    String sfs = para[3];
//    String hrs = para[4];
//    String trs = para[5];
    String stl = para[4];
    String edl = para[5];
    String mapIdx = para[6];
    String sts = para[7];  //start sample id
    String eds = para[8]; //end sample id
    String avn = para[9]; //number of all vehicles
    int sdn = Integer.parseInt(para[10]); //number of simulate days
    
    //Get the local IP Address
    String localAddr = "";
  	Enumeration<NetworkInterface> interfaces = 
  		NetworkInterface.getNetworkInterfaces();
  	while (interfaces.hasMoreElements())
  	{
  		NetworkInterface current = interfaces.nextElement();
//  		System.out.println(current);
  		if (!current.isUp() || current.isLoopback() || current.isVirtual()) continue;
  		
  		Enumeration<InetAddress> addresses = current.getInetAddresses();
  		while (addresses.hasMoreElements()){
  			InetAddress current_addr = addresses.nextElement();
  			if (current_addr.isLoopbackAddress()) continue;
  			if (current_addr instanceof Inet4Address){
//  				System.out.println("The IP address is: " + current_addr.getHostAddress());
  				if (!localAddr.isEmpty()){
  					System.err.println("Warning! There exist multiple addresses, " +
  							"including : " + localAddr + " and " + current_addr.getHostAddress());
  				}
  				localAddr = current_addr.getHostAddress();
  				
  			}
  		}
  	}

  String localMapPath = "", localDataPath = "";
	String slFile = System.getenv().get("PARALLEL_SECONDO_SLAVES");
	if (slFile.length() == 0)
	{
		throw new RuntimeException(
			"Undefined PARALLEL_SECONDO_SLAVES in " + localAddr);
	}
	String tmpFileName = "/gen_map_" + (Math.random()*1000) + ".sec";
	try {
		Scanner scanner;
		scanner = new Scanner(new FileInputStream(slFile));
		while (scanner.hasNextLine()){
			String[] slave = scanner.nextLine().split(sysDim);
			if (localAddr.compareTo(slave[0]) == 0){
				localMapPath = slave[1] + tmpFileName;
				localDataPath = slave[1].substring(0, slave[1].lastIndexOf("/")) + "/msec/bin";
				break;
			}
		}
	} catch (FileNotFoundException e1) {
		// TODO Auto-generated catch block
		e1.printStackTrace();
	}

		System.err.println("The local address is: " + localAddr);
    System.err.println("The local map file Path is: " + localMapPath);
    System.err.println("The local data file Path is: " + localDataPath);
    FileSystem.get(context.getConfiguration()).
    	copyToLocalFile(new Path(mapGenFileGPath), new Path(localMapPath));
    

    //Remote access to a Secondo database
    QuerySecondo secEntity = new QuerySecondo();
    try {
    	//Connect and create the database
    	secEntity.open(secIP, dbName, secPt, true);
    	ListExpr resultList = new ListExpr();
    	
    	//Prepare parameters
    	secEntity.query("let SCALEFACTOR = " 	+ sfs, resultList);
    	secEntity.query("let L_START = " 		+ stl, resultList);
    	secEntity.query("let L_END = " 			+ edl, resultList);
    	secEntity.query("let S_START = " 		+ sts, resultList);
    	secEntity.query("let S_END = " 			+ eds, resultList);
    	secEntity.query("let " + allVehicleNumOBJName + " = " + avn, resultList);
    	if ( sdn > 0 ){
    		secEntity.query("let P_NUMDAYS = " 			+ sdn, resultList);
    	}
    	
    	//Initialize the basic data files
    	secEntity.query("restore streets from '" + localDataPath + "/streets.data'", resultList);
    	secEntity.query("restore homeRegions from '" + localDataPath + "/homeRegions.data'", resultList);
    	secEntity.query("restore workRegions from '" + localDataPath + "/workRegions.data'", resultList);
    	
    	//Read and execute the generator file.
    	Scanner genFile= new Scanner(new FileInputStream(localMapPath));
    	String queryStr = "";
    	while (genFile.hasNextLine()){
    		String aLine = genFile.nextLine();
    		if (aLine.startsWith("#"))
    			continue;
    		else{
    			queryStr += " " + aLine;
    			if (aLine.endsWith(";") || aLine.length() == 0)
    			{
    				if (queryStr.trim().length() > 0)
    				{
    					System.out.println("###: " + queryStr);
    					secEntity.query(queryStr, resultList, false);
    					System.out.println("***:" + resultList.toString());
    					queryStr = "";
    				}
    			}
    		}
    	}
    	
    	//Prepare data for consequent steps.
    	secEntity.query("query dataMtrip1 count", resultList);
    	int tripNum = resultList.second().intValue();
    	
    	secEntity.query("query " + partRectOBJName + " feed namedtransformstream[" + partRectAttr + "] " +
    			"fconsume[\"" + partRectFileName + "\",''," + mapIdx + ";;] ", resultList);
//    	secEntity.query("query dataScar feed fconsume[\"DSC\",''," + mapIdx + ";;]", resultList);
    	
    	secEntity.close();
    	
    	for (int i = 1; i <= context.getNumReduceTasks(); i++){
    	
    		context.write(new IntWritable(i), 
        		new Text(mapIdx + inDim +
        				tripNum + inDim +
        				secIP + inDim +
        				secPt + inDim +
        				dbName + inDim +
        				stl + "_" + edl));	
    	}
    }
    catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RemoteStreamException("Catch IOException in Map task");
    }

    boolean ok = new File(localMapPath).delete();
  }

  private String getEnv(String varName){
    String varVal = "";
    for(Map.Entry entry: System.getenv().entrySet()){
      if(entry.getKey().equals(varName))
        varVal  = entry.getValue().toString();
    }
    return varVal;
  }
  
  
}

class RunCommand extends Thread{

  InputStream is;
  RunCommand(InputStream is) {
    this.is = is;
  }

  public void run()
  {
    try {
      InputStreamReader isr = new InputStreamReader(is);
      BufferedReader br = new BufferedReader(isr);
      String line = null;
      while ((line = br.readLine()) != null) {
        System.err.println(line);
      }
    } catch (IOException ioe) {
      ioe.printStackTrace();
    }
  } 
}
