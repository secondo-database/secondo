package ParallelSecondo;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Scanner;

import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;

import sj.lang.ListExpr;

public class PS_HadoopReduce2_HDJ_QMap 
extends Mapper<LongWritable, Text, IntWritable, BytesWritable> 
implements Constant
{

	Map<String, Integer> paraNames;
	
	@Override
	protected void map(LongWritable key, Text value, Context context) throws IOException,
			InterruptedException {

		String parameters[] = value.toString().split(inDim);
		int mapperIdx = Integer.parseInt(parameters[0]);
		String databaseName = parameters[1];
		String AcceptFileNameList = parameters[6];
		String AcceptFileLocList = parameters[7];
		String AcceptDLOName = parameters[8];
		String AcceptDLOLoc = parameters[9];
		//------------------------------------------------------------------
		String[] 	InputObjectName		= { parameters[12], parameters[21]} ;
		int[] 		slaveIdx					= 
									{ Integer.parseInt(parameters[13]), Integer.parseInt(parameters[22])};							 							
		int[] 		duplicateTimes    = 
									{ Integer.parseInt(parameters[14]), Integer.parseInt(parameters[23])};
		String[] 	PAName						= { parameters[15], parameters[24]} ;
		String[] 	mapCreateQuery 		= { parameters[16], parameters[25]} ;
		String[] 	mapFileName 			= { parameters[17], parameters[26]} ;
		String[] 	mapFileLoc				= { parameters[18], parameters[27]} ;

		ListExpr recvFileList = new ListExpr();
		recvFileList.readFromString(AcceptFileLocList);

		String[] interResultName = { 
				"P" + 1 + "_" + context.getJobName(),
				"P" + 2 + "_" + context.getJobName()};
		int secondoSlaveIdx = mapperIdx + 1;

		String slFile = System.getenv().get("PARALLEL_SECONDO_SLAVES");
		if (slFile == null)
			throw new RuntimeException(
				"Undefined PARALLEL_SECONDO_SLAVES in " + 
				InetAddress.getLocalHost().getHostAddress());
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
		String mapperIPAddr = slaves.get(mapperIdx).getIpAddr();
		int mapperPortNum = slaves.get(mapperIdx).getPortNum();

		QuerySecondo secEntity = new QuerySecondo();
		ListExpr resultList = new ListExpr();
		try
		{
			// First, process the map query like the QMap class 
			// Output the type file, and then use the ~doubleexport~ and ~send~ 
			// operators to deliver the result into HDFS.
			
			String[] SIDESTREAMNL = {"XXXSIDESTREAM1", "XXXSIDESTREAM2"};
			String[] SIDEKeyNAME  = {"XXXSIDEKEY1", "XXXSIDEKEY2"};
			
			//Create an empty stream, in case one side tuple stream doesn't exist. 
			ListExpr emptyStream = ListExpr.threeElemList(ListExpr.symbolAtom("head"), 
					ListExpr.twoElemList(ListExpr.symbolAtom("transformstream"), 
							ListExpr.threeElemList(ListExpr.symbolAtom("intstream"), 
									ListExpr.intAtom(0), ListExpr.intAtom(0))), 
					ListExpr.intAtom(0));
			ListExpr emptyStreamPA = ListExpr.symbolAtom("Elem");
			
			int sendPort = HPA_AuxFunctions.getPort();
			ListExpr comMapQuery = ListExpr.twoElemList(ListExpr.symbolAtom("query"),
					ListExpr.fourElemList(ListExpr.symbolAtom("send"), 
							ListExpr.fiveElemList(ListExpr.symbolAtom("doubleexport"), 
									ListExpr.symbolAtom(SIDESTREAMNL[0]), 
									ListExpr.symbolAtom(SIDESTREAMNL[1]),
									ListExpr.symbolAtom(SIDEKeyNAME[0]),
									ListExpr.symbolAtom(SIDEKeyNAME[1])),
							ListExpr.intAtom(sendPort), 
							ListExpr.symbolAtom("KeyT")));
			
			
			ListExpr InterSymbol = ListExpr.symbolAtom(QUERYNLSTR);
			ListExpr[] mapQuery = 
				{ListExpr.theEmptyList(), ListExpr.theEmptyList()};
			
			secEntity.open(mapperIPAddr, databaseName, mapperPortNum, true);
			for (int side = 0; side < 2; side++)
			{
				ListExpr typeMapQuery = ListExpr.twoElemList(ListExpr.symbolAtom("query"), 
						ListExpr.fiveElemList(ListExpr.symbolAtom("fconsume"), 
								ListExpr.threeElemList(ListExpr.symbolAtom("head"), 
										InterSymbol, 
										ListExpr.intAtom(0)),
								ListExpr.twoElemList(ListExpr.stringAtom(interResultName[side]), 
										ListExpr.textAtom("")), 
								ListExpr.oneElemList(ListExpr.intAtom(0)), 
								ListExpr.theEmptyList()));
				
				boolean replaced = true, isInputFile = true;
				ListExpr inputStream = null;
				ListExpr mapQueryList = new ListExpr(),
								 mapDLFNameList = new ListExpr(),
								 mapDLFLocsList = new ListExpr();
				mapQueryList.readFromString(mapCreateQuery[side]);
				mapDLFNameList.readFromString(mapFileName[side]);
				mapDLFLocsList.readFromString(mapFileLoc[side]);
				
				if (!mapQueryList.isEmpty())
				{
					//Embed the unexecuted map query
					if (!mapDLFNameList.isEmpty())
					{
						inputStream = mapDLFNameList.first();
						typeMapQuery = ExtListExpr.replace(typeMapQuery, InterSymbol, mapQueryList);
						typeMapQuery = HPA_AuxFunctions.loc2Ffeed(typeMapQuery, 
								ListExpr.oneElemList(inputStream), mapDLFLocsList, 
								duplicateTimes[side]);
						replaced = (!typeMapQuery.isEmpty());
					}
					else
					{
						//Impossible happens
						throw new IOException("Error in an unexpected brach.");
					}
				}
				else
				{
					//Follows the old way
					if (InputObjectName[side].matches(INDLFPattern))
					{
						inputStream = ListExpr.stringAtom(InputObjectName[side]);
						typeMapQuery = ExtListExpr.replace(typeMapQuery, InterSymbol, inputStream);
						ListExpr mapFileLocList = null;
						if (side == 0)
							mapFileLocList = recvFileList.first();
						else 
							mapFileLocList = recvFileList.second();
						typeMapQuery = HPA_AuxFunctions.loc2Ffeed(
								typeMapQuery, ListExpr.oneElemList(inputStream),
								ListExpr.oneElemList(mapFileLocList), 
								duplicateTimes[side]);
						replaced = (!mapFileLocList.isEmpty());
					}
					else
					{
						isInputFile = false;
						inputStream = ListExpr.twoElemList(ListExpr.symbolAtom("feed"), 
								ListExpr.symbolAtom(InputObjectName[side]));
						ListExpr omnList = new ListExpr();
						ListExpr omlList = new ListExpr();
						omnList.readFromString(AcceptDLOName);
						omlList.readFromString(AcceptDLOLoc);
						boolean isObjExist = HPA_AuxFunctions.objectExist(InputObjectName[side], omnList, omlList);
						if (isObjExist){
							typeMapQuery = ExtListExpr.replace(typeMapQuery, InterSymbol, inputStream);
						}
						else{
							typeMapQuery = ListExpr.theEmptyList();
						}
					}
				}

				if (!typeMapQuery.isEmpty())
				{
					secEntity.query(typeMapQuery.toString(), resultList);
					//Replace the input stream
					comMapQuery = ExtListExpr.replace(comMapQuery, 
							ListExpr.symbolAtom(SIDESTREAMNL[side]), 
							typeMapQuery.second().second().second());
					//Replace the Partition attribute (Key)
					comMapQuery = ExtListExpr.replace(comMapQuery,
							ListExpr.symbolAtom(SIDEKeyNAME[side]),
							ListExpr.symbolAtom(PAName[side]));
				}
				else
				{
					//Replace the input stream
					comMapQuery = ExtListExpr.replace(comMapQuery, 
							ListExpr.symbolAtom(SIDESTREAMNL[side]), 
							emptyStream);
					//Replace the Partition attribute (Key)
					comMapQuery = ExtListExpr.replace(comMapQuery,
							ListExpr.symbolAtom(SIDEKeyNAME[side]),
							emptyStreamPA);
				}
			}
			secEntity.close();
			
			// ----------------------------------------------------------------------
			//Remove duplicate parameter names, since the two input stream is 
			//created independently. 
			comMapQuery = rdupParameters(comMapQuery);
			// Second, use the doubleexport operation to send the result into HDFS
			String[] queries = {comMapQuery.toString()};
			SecExRunnable sender = 
				new SecExRunnable(mapperIPAddr, databaseName, mapperPortNum, queries);
			if (!sender.isInitialized())
				throw new RemoteStreamException(
						"Error! Exception while setting up sender in : " + mapperIdx);
			Thread senderThread = new Thread(sender);
			senderThread.start();
			
			
			RemoteStream receiver = new RemoteStream("client", mapperIPAddr, sendPort);
			receiver.Connect();
			if (receiver.getConnected()){

        // Read the typeInfo, especially get the key attribute type
        String typeInfo = receiver.readLine();
        receiver.writeLine("<GET TYPE/>");
        int keyLoc = typeInfo.indexOf("APPEND");
        if (keyLoc < 0) {
          System.err.println("Get typeInfo: " + typeInfo);
          throw new IOException("Error: Expect appended key attribute");
        }
        String keyType = typeInfo.substring(keyLoc + 7);
        if (!keyType.equals("string"))
          throw new IOException("Error: Expect string type key attribute");
        receiver.setKeyType(keyType);

        int loadTupleNum = 0;
        int sockCount = 0;
        int loadTupleSize = 0;
        while (!receiver.getTheLastSocket()) {
          byte[] tupleBuffer = new byte[RemoteStream.MAX_TUPLESIZE];
          int tbOffset = 0;
          while (receiver.receiveSocket(tupleBuffer, tbOffset)) {
            tbOffset += RemoteStream.SOCKTUP_SIZE;
            sockCount++;
          }
          
          if (!receiver.getTheLastSocket()) {
            sockCount++;
            tbOffset += RemoteStream.SOCKTUP_SIZE;
            loadTupleNum += LoadTuples(tupleBuffer, tbOffset, context);
            loadTupleSize += tbOffset;
          }
        }
        senderThread.join(); //wait until the Secondo thread close
        
        String hostName = InetAddress.getLocalHost().getHostName();
        
        System.out.print(hostName + " through port " + sendPort);
        System.out.println(" total got " + loadTupleNum + " tuples");
        System.out.println(" total got " + loadTupleSize / (1024*1024) + "MB tuples");
        receiver.close();
        receiver = null;
      
			}
			else{
				throw new IOException("Error! Unconnected to sender " 
						+ mapperIPAddr + ":" + sendPort);
			}
		}
		catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RemoteStreamException("Catch IOException in Map task");
		}
	}
	
  private int LoadTuples(byte[] srcBuf, int curPos, Context context)
  throws IOException, InterruptedException{
  int offset = 0;
  int loadTupleNum = 0;
  
  while(offset < curPos) {
   
    //Read key attribute size;
    int keySize = RemoteStream.Byte2Int(srcBuf, offset);
    if (keySize < 0)
      throw new IOException("Error: invalid key attribute size");
    else if (keySize == 0)
      break;
    offset += 4;
    
    //Read key attribute value;
    String keyValue = RemoteStream.Byte2String(srcBuf, offset, keySize);
    offset += keySize;
    
    //Read tuple block size;
    //The tuple block size contains a complete tuple block, 
    //outputted from Tuple::WriteToBin. 
    int tupleSize = RemoteStream.Byte2Int(srcBuf, offset);
    //short tupleSize = RemoteStream.Byte2UnsignedShort(srcBuf, offset);
    //tupleSize += 4/*2*/; //The whole tuple size should contain the short length
    
    BytesWritable byteValue = new BytesWritable();
    byteValue.set(srcBuf, offset, tupleSize);
    offset += tupleSize;
    
    context.write(new IntWritable(Integer.parseInt(keyValue)), byteValue);
    loadTupleNum++;      
  }
  
  return loadTupleNum;
}
	/**
	 * This function scans the given nested list, 
	 * if it is start with fun, finds out all its parameters, 
	 * if a parameter's name is already used before, 
	 * replace it with a new name. 
	 * 
	 * 
	 * @param queryList
	 * @return
	 */
  private ListExpr rdupParameters(ListExpr queryList)
  {
  	if (paraNames == null){
  		paraNames = new HashMap<String, Integer>();
  	}
  	
  	if (queryList.isEmpty())
  		return ListExpr.theEmptyList();
  	
  	if (queryList.isAtom())
  		return queryList;
  	
  	if (queryList.first().isAtom())
  	{
  		if (queryList.first().equals(ListExpr.symbolAtom("fun"))){
  			//This is a function list 
  			ListExpr rest = queryList.rest();
  			
  			List<String[]> dupPNPairs = new ArrayList<String[]>();
  			while (rest.listLength() > 1)
  			{
  				//read parameter names
					ListExpr parameter = rest.first();
  				String pName = parameter.first().symbolValue();
  				if (paraNames.containsKey(pName))
  				{
  					int Cnt = paraNames.get(pName);
  					Cnt++;
  					String newPName = pName + Cnt;
  					paraNames.put(pName, Cnt);
  					String[] pnPairs = {pName, newPName};
  					dupPNPairs.add(pnPairs);
  				}
  				else
  				{
  					paraNames.put(pName, 0);
  				}
  				
  				rest = rest.rest();
  			}
  			
  			if (dupPNPairs.size() > 0)
  			{
  				ListIterator<String[]> it = dupPNPairs.listIterator();
  				while (it.hasNext()){
  					String[] pnPairs = (String[]) it.next();
  					ListExpr oldName = ListExpr.symbolAtom(pnPairs[0]);
  					ListExpr newName = ListExpr.symbolAtom(pnPairs[1]);
  					queryList = ExtListExpr.replace(queryList, oldName, newName);
  				}
  			}
  			
  			return queryList;
  		}
  	}
  	
  	return ListExpr.cons(rdupParameters(queryList.first()), 
  			rdupParameters(queryList.rest()));
  }
	
}

