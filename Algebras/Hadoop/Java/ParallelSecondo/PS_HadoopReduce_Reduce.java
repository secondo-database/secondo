package ParallelSecondo;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

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
		int PSFSMode = 1;
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
				PSFSMode          = Integer.parseInt(parameters[9]);
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

		PSNode node = PSNode.SelectDataServer(candSlaveIndex);
		if (node == null)
			throw new IOException("Cannot find proper data server");
		else{
			slaveIndex 	= node.getSn() + 1;
			IPAddr 			= node.getIpAddr();
			portNum	 		= node.getPortNum();
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

/* 
0.1 Collect intermediate results, only one

Use a loopsel+ffeed operation to replace the inter-result files 
set in the reduceQuery, 
Build up the nested-list of the intermediate result locations
 
*/
		ListExpr irSuffixList = HPA_AuxFunctions.feedColumn(rows.iterator(), columnNo);
		
		String iftName = "xxxjmIFTP";
		int typeNodeIdx = rows.get(new Random().nextInt(rows.size()));
		ListExpr lfInterList = HPA_AuxFunctions.feedInterResult2(
				interResultName,typeNodeIdx, irSuffixList, PSFSMode);
		
		
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
		
		boolean replaced = true;
		if (!nameList.isEmpty())
		{
			nameList = nameList.first();
			reduceQueryList = HPA_AuxFunctions.loc2Ffeed(
					reduceQueryList, nameList, locsList,1);
			replaced = (!reduceQueryList.isEmpty());
		}

		if (replaced)
		{
			comCreateQuery = ExtListExpr.replaceFirst(
					comCreateQuery, QUERYNLSTR,	 reduceQueryList);
			
			//Process the query and process it.
			QuerySecondo secEntity = new QuerySecondo();
			try
			{
				secEntity.open(IPAddr, databaseName, portNum, true);
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
	}
}

