import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;
import java.util.Scanner;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

import sj.lang.ListExpr;

public class ReduceAggre extends Reducer<IntWritable, Text, Text, Text>
implements Constant{


	public void reduce(IntWritable key, Iterable<Text> values, Context context) throws IOException {
		int tripID_Start = 1;
		String tpName = "xxxjmTP";
		String tpName2[] = {"xxxR1", "xxxR2"};
		String secIP = "", dbName = "";
		int secPort = -1;
		int csIndex = 0; //current slave index
		
		//Get the reduce step generate script
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

		String localReducePath = "";
		String slFile = System.getenv().get("PARALLEL_SECONDO_SLAVES");
		if (slFile.length() == 0)
		{
			throw new RuntimeException(
				"Undefined PARALLEL_SECONDO_SLAVES in " + localAddr);
		}
		String tmpFileName = "/gen_reduce_" + (Math.random()*1000) + ".sec";
		try {
			Scanner scanner;
			scanner = new Scanner(new FileInputStream(slFile));
			while (scanner.hasNextLine()){
				String[] slave = scanner.nextLine().split(sysDim);
				if (localAddr.compareTo(slave[0]) == 0){
					localReducePath = slave[1] + tmpFileName;
					break;
				}
			}
		} catch (FileNotFoundException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}

	    System.out.println("The local reduce file Path is: " + localReducePath);
	    FileSystem.get(context.getConfiguration()).
	    	copyToLocalFile(new Path(reduceGenFileGPath), new Path(localReducePath));
		
		//Collect partial rectangles
		//The value of files' locations
		int itTimes = 0;
		ListExpr suffixList = ListExpr.theEmptyList();
		ListExpr last = new ListExpr();
		for (Text value: values){
			System.out.println("GET: " + value.toString());
			String parameters[] = value.toString().split(inDim);
			int mapIdx = Integer.parseInt(parameters[0]);
			int tripNum = Integer.parseInt(parameters[1]);
			
			if (itTimes++ == 0){
				suffixList = 
					ListExpr.oneElemList(
						ListExpr.twoElemList(
							ListExpr.intAtom(mapIdx) , 
							ListExpr.intAtom(mapIdx)));
				last = suffixList; 
			}
			else{
				last = ListExpr.append(last, 
						ListExpr.twoElemList(
							ListExpr.intAtom(mapIdx) , 
							ListExpr.intAtom(mapIdx)));
			}
			
			System.out.println("key: " + key.get());
			System.out.println("mapIdx: " + mapIdx);
			
			if (key.get() > mapIdx)
				tripID_Start += tripNum;
			else if (key.get() == mapIdx)
			{
				csIndex = key.get();
				secIP = parameters[2];
				secPort = Integer.parseInt(parameters[3]);
				dbName = parameters[4];
			}
			
		}
		
		System.out.println("sceIP: " + secIP);
		System.out.println("secPort: " + secPort);
		System.out.println("dbName: " + dbName);
		
		suffixList = ListExpr.twoElemList(
				ListExpr.twoElemList(
					ListExpr.symbolAtom("rel"), 
					ListExpr.twoElemList(
						ListExpr.symbolAtom("tuple"), 
						ListExpr.twoElemList(
							ListExpr.twoElemList(
								ListExpr.symbolAtom(frsName),  //Machine ID 
								ListExpr.symbolAtom("int")), 
							ListExpr.twoElemList(
								ListExpr.symbolAtom(fssName),  //File ID
								ListExpr.symbolAtom("int"))))), 
				suffixList);
		suffixList = ListExpr.twoElemList(
				ListExpr.symbolAtom("feed"),
				suffixList);
		
		ListExpr ffeedList = ListExpr.fiveElemList(
								ListExpr.symbolAtom("ffeed"), 
								ListExpr.stringAtom(partRectFileName), 
								ListExpr.twoElemList(
									ListExpr.textAtom(""), 
									ListExpr.threeElemList(
										ListExpr.symbolAtom("attr"), 
										ListExpr.symbolAtom(tpName), 
										ListExpr.symbolAtom(frsName))), 
									ListExpr.theEmptyList(), 
									ListExpr.threeElemList(
										ListExpr.threeElemList(
											ListExpr.symbolAtom("attr"),
											ListExpr.symbolAtom(tpName), 
											ListExpr.symbolAtom(fssName)), 
									ListExpr.threeElemList(
											ListExpr.symbolAtom("attr"),
											ListExpr.symbolAtom(tpName),
											ListExpr.symbolAtom(fssName)), 
									ListExpr.intAtom(1)));

		ListExpr loopselList = ListExpr.threeElemList(
				ListExpr.symbolAtom("loopsel"), 
				suffixList, 
				ListExpr.threeElemList(
					ListExpr.symbolAtom("fun"), 
					ListExpr.twoElemList(
						ListExpr.symbolAtom(tpName), 
						ListExpr.symbolAtom("TUPLE")),
					ffeedList));
		
		
		ListExpr aggregateList =
			ListExpr.fiveElemList(
				ListExpr.symbolAtom("aggregateB"),
				loopselList,
				ListExpr.symbolAtom(partRectAttr),
				ListExpr.fourElemList(
					ListExpr.symbolAtom("fun"),
					ListExpr.twoElemList(
						ListExpr.symbolAtom(tpName2[0]), 
						ListExpr.symbolAtom("rect3")),
					ListExpr.twoElemList(
						ListExpr.symbolAtom(tpName2[1]), 
						ListExpr.symbolAtom("rect3")),
					ListExpr.threeElemList(
						ListExpr.symbolAtom("union"),
						ListExpr.symbolAtom(tpName2[0]),
						ListExpr.symbolAtom(tpName2[1]))),
					ListExpr.twoElemList(
						ListExpr.symbolAtom("rect3"), 
						ListExpr.symbolAtom("undef")));
		
		ListExpr createRect = 
			ListExpr.fourElemList(
				ListExpr.symbolAtom("let "),
				ListExpr.symbolAtom(GlobalRectOBJName),
				ListExpr.symbolAtom(" = "),
				aggregateList);
		
	    QuerySecondo secEntity = new QuerySecondo();
		ListExpr resultList = new ListExpr();
		try {
			secEntity.open(secIP, dbName,secPort, false);
			
			secEntity.query(createRect.toString(), resultList);
			secEntity.query("if (isDBObject(\"TRIPID_START\")) then delete SCALEFACTOR endif", resultList);
			secEntity.query("let TRIPID_START = " 	+ tripID_Start, resultList);
			secEntity.query("let P_DS_INDEX = " + csIndex, resultList);
			
			
	    	//Read and execute the generator file.
	    	Scanner genFile= new Scanner(new FileInputStream(localReducePath));
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
	    					secEntity.query(queryStr, resultList, true);
	    					System.out.println("***:" + resultList.toString());
	    					queryStr = "";
	    				}
	    			}
	    		}
	    	}

	    	secEntity.close();
			
			context.write(new Text("" + csIndex),
					new Text("" + csIndex + " " + 1));
			
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		
		
	}

}
