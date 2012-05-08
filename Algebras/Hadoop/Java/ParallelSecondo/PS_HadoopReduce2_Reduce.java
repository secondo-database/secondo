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


public class PS_HadoopReduce2_Reduce 
	extends Reducer<IntWritable, Text, IntWritable, Text>  
	implements Constant
{

	String IPAddr = "";
	int portNum = -1;
	int slaveIndex = -1;  
	//The index of the local machine inside the Machines array

	
	@Override
	protected void reduce(IntWritable key, Iterable<Text> values,
			Context context) throws IOException, InterruptedException 
	{
		int columnNo = key.get();
		//List[] bothRows = { new ArrayList<Integer>(), new ArrayList<Integer>()};
		List<ArrayList<Integer>> bothRows = new ArrayList<ArrayList<Integer>>();
		bothRows.add(new ArrayList<Integer>());
		bothRows.add(new ArrayList<Integer>());
		
		String databaseName = "";
		String CreateObjectName = "";
		String CreateFilePath   = "";
		String reduceQuery = "";
		String[] interResultName = {"",""};
		String DLF_Names = "";           //Other DLF flist parameters
		String DLF_Locs  = "";
		FListKind outputKind = FListKind.UNDEF;
		int candSlaveIndex = -1;
		int resultRowNo = context.getTaskAttemptID().getTaskID().getId() + 1;
		
		for (Text value : values)
		{
			String parameters[] = value.toString().split(inDim);
			
			int side  = Integer.parseInt(parameters[0]);
			int slave = Integer.parseInt(parameters[1]);
			
			bothRows.get(side).add(slave);
			
			
			if ( interResultName[0].length() == 0 
				|| interResultName[1].length() == 0)
			{
				interResultName[side] = parameters[2];
			}
			
			if (databaseName.length() == 0)
			{
				databaseName 					= parameters[3];
				CreateObjectName 			= parameters[4];
				CreateFilePath	 			= parameters[5];
				DLF_Names 						= parameters[7];
				DLF_Locs	 						= parameters[8];
				outputKind						= 
					FListKind.values()[Integer.parseInt(parameters[9])];
			}
			
			if (reduceQuery.length() == 0)
			{ 
				//In some mappers, the reduceQuery may be empty,
				//if not both inputs are replaced by intermediate results
				reduceQuery						= parameters[6];
			}
		}
		
		for (int side = 0; side < 2; side++){
	  	if (bothRows.get(side).size() == 0){
				return;
			}
		}
		
		ListExpr reduceQueryList = new ListExpr();
		reduceQueryList.readFromString(reduceQuery);

		if (outputKind == FListKind.DLO){
			candSlaveIndex = resultRowNo;
		}

		PSNode node = PSNode.SelectDataServer(candSlaveIndex);
		if (node == null)
			throw new IOException("Cannot find proper data server");
		else{
			slaveIndex = node.getSn() + 1;
			IPAddr 		 = node.getIpAddr();
			portNum		 = node.getPortNum();
		}
		
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
		
		ListExpr[] irSuffixList = {ListExpr.theEmptyList(), ListExpr.theEmptyList()};
		String[] iftName = {"xxxjmIFTP1", "xxxjmIFTP2"};
		ListExpr[] lfInterList = {ListExpr.theEmptyList(), ListExpr.theEmptyList()};
		for (int side = 0; side < 2; side++)
		{
			irSuffixList[side] = HPA_AuxFunctions.feedRows(bothRows.get(side).iterator());
			
			int typeNodeIdx = bothRows.get(side).get(new Random().nextInt(bothRows.get(side).size()));
			lfInterList[side] = HPA_AuxFunctions.feedInterResult(
					columnNo, interResultName[side], iftName[side], 
					typeNodeIdx, irSuffixList[side]);

			ListExpr interPattern = ListExpr.stringAtom(interResultName[side]);
			reduceQueryList = 
				ExtListExpr.replace(reduceQueryList, interPattern, lfInterList[side]);
		}
		
		//Set other flists
		ListExpr nameList = new ListExpr();
		ListExpr locsList = new ListExpr();

		nameList.readFromString(DLF_Names);
		if (nameList.isAtom()) nameList = ListExpr.oneElemList(nameList);
		locsList.readFromString(DLF_Locs);
		locsList = ListExpr.oneElemList(locsList);

		boolean replaced = true;
		if (!nameList.isEmpty())
		{
			reduceQueryList = HPA_AuxFunctions.loc2Ffeed(
					reduceQueryList, nameList, locsList, 1);
			replaced = (!reduceQuery.isEmpty());
		}

		if (replaced)
		{
			comCreateQuery = 
				ExtListExpr.replaceFirst(comCreateQuery, QUERYNLSTR, reduceQueryList);
			
			QuerySecondo secEntity = new QuerySecondo();
			try
			{
				secEntity.open(IPAddr, databaseName, portNum, true);
				ListExpr resultList = new ListExpr();
				
				if (outputKind == FListKind.DLO){
					secEntity.query("delete " + CreateObjectName, resultList, true);
				}
				secEntity.query(comCreateQuery.toString(), resultList);
				
/*
The reduce step produces a parse relation matrix, 
both the slaveIndex and columnNo plus 1, since they start from 1.

*/
				context.write(new IntWritable(resultRowNo), 
						new Text("" + (columnNo + 1) + " " + slaveIndex));
				
				secEntity.close();
			}
			catch(IOException ie){
				ie.printStackTrace();
				throw new RuntimeException("Secondo runtime error");
			}
		}
	}

}
