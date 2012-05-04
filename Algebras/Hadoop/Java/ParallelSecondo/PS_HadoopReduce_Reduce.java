package ParallelSecondo;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Scanner;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

import sj.lang.ListExpr;

public class PS_HadoopReduce_Reduce 
	extends Reducer<IntWritable, Text, IntWritable, Text>  
	implements Constant
{
	String IPAddr = "";
	int portNum = -1;
	int slaveIndex = -1;  
		//The index of the local machine inside the Machines array

	@Override
	protected void reduce(IntWritable key, Iterable<Text> values,
			Context context) throws IOException,
			InterruptedException {

		int columnNo = key.get();
		List<Integer> rows = new ArrayList<Integer>();
		String databaseName = "";
		String CreateObjectName = "";
		String CreateFilePath   = "";
		String reduceQuery = "";
		String interResultName = "";
		String DLF_Names = "";           //Other DLF flist parameters
		String DLF_Locs  = "";
		FListKind outputKind = FListKind.UNDEF;
		int candSlaveIndex = -1;
		int resultRowNo = context.getTaskAttemptID().getTaskID().getId() + 1;
		
		for (Text value : values)
		{
			String parameters[] = value.toString().split(inDim);
			
			rows.add(Integer.parseInt(parameters[0]));
			
			if (databaseName.length() == 0)
			{
				databaseName 			= parameters[1];
				CreateObjectName 	= parameters[2];
				CreateFilePath	 	= parameters[3];
				reduceQuery				= parameters[4];
				interResultName   = parameters[5];
				DLF_Names 				= parameters[6];
				DLF_Locs	 				= parameters[7];
				outputKind				= FListKind.values()[Integer.parseInt(parameters[8])];
			}
		}
		
		ListExpr reduceQueryList = new ListExpr();
		reduceQueryList.readFromString(reduceQuery);
		
		if (outputKind == FListKind.DLO){
/*
A data server process one column data, and each column is transposed to one 
row of the output matrix relation.
 
*/
			candSlaveIndex = resultRowNo;
		}

		if (!selectDataServer(candSlaveIndex))
			throw new IOException("Cannot find proper data server");
		
		//Prepare the creation query first
		ListExpr comCreateQuery;
		if (outputKind == FListKind.DLO)
		{
			//DLO
			comCreateQuery = ListExpr.fourElemList(
					ListExpr.symbolAtom("let"), 
					ListExpr.symbolAtom(CreateObjectName), 
					ListExpr.symbolAtom("="), 
					ListExpr.symbolAtom(QUERYNLSTR));  
		}
		else
		{
			//DLF
			ListExpr InterSymbol = ListExpr.symbolAtom(QUERYNLSTR);
			comCreateQuery = ListExpr.twoElemList(
					ListExpr.symbolAtom("query"), 
					ListExpr.fiveElemList(
							ListExpr.symbolAtom("fconsume"),
							InterSymbol, 
							ListExpr.fourElemList(
									ListExpr.stringAtom(CreateObjectName), 
									ListExpr.textAtom(CreateFilePath),
								ListExpr.intAtom(resultRowNo),
								ListExpr.intAtom(columnNo + 1)), 
						ListExpr.theEmptyList(), 
						ListExpr.theEmptyList()));
		}
		

/* 
0.1 Collect intermediate results, only one

Use a loopsel+ffeed operation to replace the inter-result files 
set in the reduceQuery, 
Build up the nested-list of the intermediate result locations
 
*/
		ListExpr irSuffixList = ListExpr.theEmptyList();
		Iterator<Integer> it = rows.iterator();
		if (it.hasNext())
		{
			int row = it.next();
			irSuffixList = ListExpr.oneElemList(
				ListExpr.twoElemList(
					ListExpr.intAtom(row),					//Row ID 
					ListExpr.intAtom(row)));				//Slave ID
			ListExpr last = irSuffixList;
			while (it.hasNext())
			{
				row = it.next();
				last = ListExpr.append(last, 
						ListExpr.twoElemList(
							ListExpr.intAtom(row), 
							ListExpr.intAtom(row)));
			}
		}
		
		irSuffixList = ListExpr.twoElemList(
				ListExpr.twoElemList(
						ListExpr.symbolAtom("trel"), 
						ListExpr.twoElemList(
								ListExpr.symbolAtom("tuple"),
										ListExpr.twoElemList(
												ListExpr.twoElemList(
														ListExpr.symbolAtom(frsName), 
														ListExpr.symbolAtom("int")), 
												ListExpr.twoElemList(
														ListExpr.symbolAtom(fssName), 
														ListExpr.symbolAtom("int"))))), 	//TRel Definition 
						irSuffixList																			//TRel Value
			);
		irSuffixList = 
			ListExpr.twoElemList( ListExpr.symbolAtom("feed"), irSuffixList);
		
		
		String iftName = "xxxjmIFTP";
		ListExpr lfInterList = ListExpr.oneElemList(ListExpr.symbolAtom("ffeed"));
		ListExpr last = lfInterList;
		last = ListExpr.append(last, ListExpr.stringAtom(
				interResultName.substring(
						interResultName.lastIndexOf(":") + 1, 
						interResultName.lastIndexOf("/>"))));
		last = ListExpr.append(last, ListExpr.threeElemList(
				ListExpr.textAtom(""), 
				ListExpr.threeElemList(
					ListExpr.symbolAtom("attr"), 
					ListExpr.symbolAtom(iftName), 
					ListExpr.symbolAtom(frsName)),  //get file row_columnNo   
				ListExpr.intAtom(columnNo)));
		int typeNodeIdx = rows.get(new Random().nextInt(rows.size()));
		last = ListExpr.append(last, ListExpr.oneElemList(
				ListExpr.intAtom(typeNodeIdx))); //Type Remote node

		last = ListExpr.append(last, ListExpr.threeElemList(
				ListExpr.threeElemList(
					ListExpr.symbolAtom("attr"), 
					ListExpr.symbolAtom(iftName), 
					ListExpr.symbolAtom(fssName)),  //create by a slave  
				ListExpr.threeElemList(
					ListExpr.symbolAtom("attr"), 
					ListExpr.symbolAtom(iftName), 
					ListExpr.symbolAtom(fssName)), 	//also locate at it
				ListExpr.intAtom(1)));				//without duplication
		lfInterList = 					
			ListExpr.threeElemList(
				ListExpr.symbolAtom("loopsel"), 
				irSuffixList, 
				ListExpr.threeElemList(
					ListExpr.symbolAtom("fun"), 
						ListExpr.twoElemList(
							ListExpr.symbolAtom(iftName), 
							ListExpr.symbolAtom("TUPLE")),
						lfInterList));
		
		ListExpr interPattern = ListExpr.stringAtom(interResultName);
		reduceQueryList = ExtListExpr.replace(reduceQueryList, interPattern, lfInterList);
		
		
/*
0.2. Collect other DLF data 

(from the whole cluster), and may exist several of it. 

*/
		//TODO But for DLO data, it is hard to be collected. 
		ListExpr nameList = new ListExpr();
		ListExpr locsList = new ListExpr();
		nameList.readFromString(DLF_Names);
		locsList.readFromString(DLF_Locs);
		
		ListExpr restName = nameList;
		ListExpr restLocs = locsList;
		int dlfIndex = 0;
		while(!restLocs.isEmpty())
		{
			ListExpr dlfNameList = restName.first();
			ListExpr dlfLocsList = restLocs.first();
			
			int rowNum = 1;
			ListExpr restDlfLocs = dlfLocsList;
			ListExpr dlfSuffixList = new ListExpr(), ffeedList_last = null;
			while (!restDlfLocs.isEmpty())
			{
				ListExpr row = restDlfLocs.first();
				int slaveIdx = row.first().intValue();
				ListExpr columns = row.second();
				
				ListExpr restColumns = columns;
				while (!restColumns.isEmpty())
				{
					int colNum = restColumns.first().intValue();
					
					if (dlfSuffixList.isEmpty()){
						dlfSuffixList = ListExpr.oneElemList(
								ListExpr.fourElemList(
										ListExpr.intAtom(rowNum), 
										ListExpr.intAtom(colNum), 
										ListExpr.intAtom(slaveIdx), 
										row.third()));
						ffeedList_last = dlfSuffixList;
					}
					else{
						ffeedList_last = ListExpr.append(ffeedList_last, 
								ListExpr.fourElemList(
									ListExpr.intAtom(rowNum), 
									ListExpr.intAtom(colNum), 
									ListExpr.intAtom(slaveIdx),
									row.third()));
					}
					restColumns = restColumns.rest();
				}
				
				rowNum++;
				restDlfLocs = restDlfLocs.rest();
			}
			dlfSuffixList = ListExpr.twoElemList(
					ListExpr.twoElemList(
							ListExpr.symbolAtom("trel"),
							ListExpr.twoElemList(
									ListExpr.symbolAtom("tuple"), 
											ListExpr.fourElemList(
													ListExpr.twoElemList(
															ListExpr.symbolAtom(frsName), 
															ListExpr.symbolAtom("int")),
													ListExpr.twoElemList(
															ListExpr.symbolAtom(fcsName), 
															ListExpr.symbolAtom("int")),
													ListExpr.twoElemList(
															ListExpr.symbolAtom(fssName), 
															ListExpr.symbolAtom("int")),
													ListExpr.twoElemList(
															ListExpr.symbolAtom(fptName), 
															ListExpr.symbolAtom("text"))))),
					dlfSuffixList);
			dlfSuffixList = ListExpr.twoElemList( ListExpr.symbolAtom("feed"), dlfSuffixList);
			String dtpName = "xxxjmDlfTP" + dlfIndex;
			String dlfName = dlfNameList.stringValue();
			dlfName  = dlfName.substring(dlfName.lastIndexOf(':') + 1, dlfName.lastIndexOf("/>"));
			ListExpr collectOneFile = ListExpr.oneElemList(ListExpr.symbolAtom("ffeed"));
			ListExpr ffeed_last = collectOneFile;
			ffeed_last = ListExpr.append(ffeed_last, ListExpr.stringAtom(dlfName));
			ffeed_last = ListExpr.append(ffeed_last, 
					ListExpr.threeElemList(
							ListExpr.textAtom(""),

							ListExpr.threeElemList(
									ListExpr.symbolAtom("attr"),
									ListExpr.symbolAtom(dtpName),
									ListExpr.symbolAtom(frsName)),
							ListExpr.threeElemList(
									ListExpr.symbolAtom("attr"), 
									ListExpr.symbolAtom(dtpName), 
									ListExpr.symbolAtom(fcsName))));
			
			ffeed_last = ListExpr.append(ffeed_last, ListExpr.theEmptyList()); 
			//Search type file at local disk

			ffeed_last = ListExpr.append(ffeed_last, 
					ListExpr.threeElemList(
							ListExpr.threeElemList(
									ListExpr.symbolAtom("attr"),	
									ListExpr.symbolAtom(dtpName),	
									ListExpr.symbolAtom(fssName)), 
							ListExpr.threeElemList(
									ListExpr.symbolAtom("attr"),	
									ListExpr.symbolAtom(dtpName),	
									ListExpr.symbolAtom(fssName)), 
							ListExpr.intAtom(1)));
			//Search data file at remote data servers

			collectOneFile = ListExpr.threeElemList(
					ListExpr.symbolAtom("loopsel"), 
					dlfSuffixList, 
					ListExpr.threeElemList(
						ListExpr.symbolAtom("fun"), 
							ListExpr.twoElemList(
								ListExpr.symbolAtom(dtpName), 
								ListExpr.symbolAtom("TUPLE")),
								collectOneFile));
			reduceQueryList = ExtListExpr.replace(
					reduceQueryList, dlfNameList, collectOneFile);
			
			dlfIndex++;
			restName = restName.rest();
			restLocs = restLocs.rest();
		}

		
		comCreateQuery = ExtListExpr.replaceFirst(
				comCreateQuery, QUERYNLSTR,	 reduceQueryList);
		
		//Process the query and process it.
		QuerySecondo secEntity = new QuerySecondo();
		try
		{
			secEntity.open(IPAddr, databaseName, portNum);
			ListExpr resultList = new ListExpr();
			
			if (outputKind == FListKind.DLO){
				secEntity.query("delete " + CreateObjectName, resultList, true);
			}
			secEntity.query(comCreateQuery.toString(), resultList);
			
			//The reduce step produces a parse relation matrix
			//both the slaveIndex and columnNo plus 1, since they start from 1.
			
			context.write(new IntWritable(resultRowNo), 
					new Text("" + (columnNo + 1) + " " + slaveIndex));
			
			secEntity.close();
		}
		catch(IOException ie){
			ie.printStackTrace();
			throw new RuntimeException("Secondo runtime error");
		}
		
	}
	
	boolean selectDataServer(int candidate)	
		throws IOException, InterruptedException
	{
		//Select a proper slave to process the query
		String localAddr = InetAddress.getLocalHost().getHostAddress();
		String slFile = System.getenv().get("PARALLEL_SECONDO_SLAVES");
		if (slFile.length() == 0)
		{
			throw new RuntimeException(
					"Undefined PARALLEL_SECONDO_SLAVES in " + localAddr);
		}
		
		if ( candidate > 0 ){
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
				slaveIndex = candidate;
				IPAddr = slaves.get(slaveIndex - 1).getIpAddr();
				portNum = slaves.get(slaveIndex - 1).getPortNum();
				
				return true;
			} catch (FileNotFoundException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
		}
			
		
		int rowSize = 0;
		List<PSNode> slaves = new ArrayList<PSNode>();
		try {
			Scanner scanner;
			scanner = new Scanner(new FileInputStream(slFile));
			while (scanner.hasNextLine()){
				rowSize++;  //Get the rows scale of the matrix
				String[] line = scanner.nextLine().split(sysDim);
				if (localAddr.compareTo(line[0]) == 0){
					slaves.add(new PSNode(rowSize, line[0], line[1], 
					Integer.parseInt(line[2])));
				}
			}
		} catch (FileNotFoundException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}

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
				pCnt = counter;
				IPAddr = localAddr;
				portNum = port;
				slaveIndex = slave.getSn();
				return true;
			}
		}
		return false;
	}
	
	
}

