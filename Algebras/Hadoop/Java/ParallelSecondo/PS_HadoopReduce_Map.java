package ParallelSecondo;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;

import sj.lang.ListExpr;

public class PS_HadoopReduce_Map 
	extends Mapper<LongWritable, Text, IntWritable, Text> 
	implements Constant
{

	@Override
	protected void map(LongWritable key, Text value, Context context) 
		throws IOException, InterruptedException {

		String parameters[] = value.toString().split(inDim);
		int 		slaveIdx					= Integer.parseInt(parameters[0]);							 							
		String 	databaseName 			= parameters[1];		
		String 	CreateObjectName 	= parameters[2];		
		String 	reduceQuery 			= parameters[3];
		String 	mapFileNames	 		= parameters[4];
		String 	mapFileLocs				= parameters[5];
		//The parameters[6-7] is prepared for reduce tasks
		int 		duplicateTimes    = Integer.parseInt(parameters[8]);
		FListKind outputKind 			= 
			FListKind.values()[Integer.parseInt(parameters[9])];
		
		String CreateFilePath 		= parameters[10];
		ListExpr fpList = new ListExpr();
		fpList.readFromString(CreateFilePath);
		CreateFilePath = fpList.first().textValue();
		
		String 	InputObjectName		= parameters[11];
		String 	PAName						= parameters[12];
		int     PSFSMode          = Integer.parseInt(parameters[13]);

		int secondoSlaveIdx = slaveIdx + 1;
		String interResultName = "P_"+ context.getJobName();
		
		ListExpr fileNameList = new ListExpr(), fileLocList = new ListExpr();
		fileNameList.readFromString(mapFileNames);
		fileLocList.readFromString(mapFileLocs);

		//replace the input argument in the reduce query
		ListExpr reduceQueryList = new ListExpr();
		reduceQueryList.readFromString(reduceQuery);

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
		String mapperIPAddr = slaves.get(slaveIdx).getIpAddr();
		int mapperPortNum = slaves.get(slaveIdx).getPortNum();

		QuerySecondo secEntity = new QuerySecondo();
		List<Integer> rColumns = new ArrayList<Integer>();
		try
		{
			String fdoName = "fdistribute";       //The name of the distribute operator
			if (PSFSMode == 3)
				fdoName = "fdistribute3";
			
			ListExpr comMapQuery;
			ListExpr InterSymbol = ListExpr.symbolAtom(QUERYNLSTR);
			comMapQuery = ListExpr.twoElemList(
					ListExpr.symbolAtom("query"),
						ListExpr.twoElemList(
								ListExpr.symbolAtom("tconsume"), 
								ListExpr.threeElemList(
										ListExpr.symbolAtom("project"), 
										ListExpr.sixElemList(
												ListExpr.symbolAtom(fdoName),
												InterSymbol,
												ListExpr.fourElemList(
														ListExpr.stringAtom(interResultName), 
														ListExpr.textAtom(CreateFilePath),
														ListExpr.symbolAtom(PAName),
														ListExpr.intAtom(secondoSlaveIdx)), 
												ListExpr.twoElemList(
														ListExpr.intAtom(context.getNumReduceTasks()), 
														ListExpr.boolAtom(true)),
												ListExpr.theEmptyList(), 
												ListExpr.theEmptyList()), 
											ListExpr.oneElemList(ListExpr.symbolAtom("Suffix")))));
			
			boolean replaced = true;
			ListExpr pattern = null;
			if (!fileNameList.isEmpty())
			{
				
				//Only exist one element
				ListExpr inputStreamList = fileNameList.first().first();
  			comMapQuery = ExtListExpr.replace(comMapQuery, InterSymbol, inputStreamList);
				
				comMapQuery = HPA_AuxFunctions.loc2Ffeed(comMapQuery,
						ListExpr.oneElemList(inputStreamList), 
						fileLocList, duplicateTimes);

				replaced = (!comMapQuery.isEmpty());
				pattern = fileNameList.first().first();
			}
			else
			{
				ListExpr inputStreamList = ListExpr.twoElemList(
						ListExpr.symbolAtom("feed"),
						ListExpr.symbolAtom(InputObjectName));
				comMapQuery = ExtListExpr.replace(comMapQuery, InterSymbol, inputStreamList);
				pattern = inputStreamList;
			}
			
			
			
			//Build up the map query
			secEntity.open(mapperIPAddr, databaseName, mapperPortNum, true);
			if (replaced)
			{
				//Use the intermediate result to replace the input flist 
				interResultName = "<DLFMark:" + interResultName + "/>";
				ListExpr interPattern  = ListExpr.stringAtom(interResultName);
				
				reduceQueryList = 
					ExtListExpr.replace(reduceQueryList, pattern, interPattern);
				reduceQuery = HPA_AuxFunctions.plainStr(reduceQueryList);
				
				ListExpr resultList = new ListExpr();
				secEntity.query(comMapQuery.toString(), resultList);
				
				resultList = resultList.second();
				ListExpr rest = resultList;
				while (!rest.isEmpty())
				{
					ListExpr item = rest.first();
					rColumns.add(item.first().intValue());
					rest = rest.rest();
				}

				for (int column : rColumns)
				{
					context.write(new IntWritable(column), 
							new Text(
									secondoSlaveIdx 	+ inDim + 
									databaseName    	+ inDim +
									CreateObjectName  + inDim +
									CreateFilePath    + inDim +
									reduceQuery       + inDim +
									interResultName	  + inDim +
									parameters[6]			+ inDim +
									parameters[7]			+ inDim +
									outputKind.ordinal() + inDim +
									PSFSMode
							));
				}
			}
			else{
				System.err.println("The construction of map query fails");
			}
			
			secEntity.close();
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RemoteStreamException("Catch IOException in Map task");
		}
	}
}
