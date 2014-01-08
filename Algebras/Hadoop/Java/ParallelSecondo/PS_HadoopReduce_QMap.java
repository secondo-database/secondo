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

public class PS_HadoopReduce_QMap 
	extends Mapper<LongWritable, Text, IntWritable, Text> 
	implements Constant
{

	@Override
	protected void map(LongWritable key, Text value,
			Context context) throws IOException, InterruptedException 
	{

		String parameters[] = value.toString().split(inDim);
		int 		slaveIdx					= Integer.parseInt(parameters[0]);							 							
		int 		mapperIdx					= Integer.parseInt(parameters[1]);							 							
		String 	databaseName 			= parameters[2];
		String 	mapCreateQuery 		= parameters[3];
		String 	mapFileName 			= parameters[4];
		String 	mapFileLoc				= parameters[5];
		String 	reduceQuery 			= parameters[7];
		String  paraMapFileName   = parameters[8];
		String 	paraMapFileLocs		= parameters[9];
		int 		duplicateTimes    = Integer.parseInt(parameters[12]);
		String CreateFilePath 		= parameters[14];
		ListExpr fpList = new ListExpr();
		fpList.readFromString(CreateFilePath);
		CreateFilePath = fpList.first().textValue();
		String 	InputObjectName		= parameters[15];
		String 	PAName						= parameters[16];
		int     PSFSMode          = Integer.parseInt(parameters[17]);
		
		String 	interResultName = "P_"+ context.getJobName();
		int secondoSlaveIdx = mapperIdx + 1;
		List<Integer> rColumns = new ArrayList<Integer>();
		
		//----------- Prepare All Parameters
		
		ListExpr fileNameList = new ListExpr(), fileLocList = new ListExpr(), 
						 mapQueryList = new ListExpr(), reduceQueryList = new ListExpr();
		fileNameList.readFromString(mapFileName);
		fileLocList.readFromString(mapFileLoc);
		mapQueryList.readFromString(mapCreateQuery);
		reduceQueryList.readFromString(reduceQuery);
		
		ListExpr paraFileNameList = new ListExpr(), paraFileLocsList = new ListExpr();
		paraFileNameList.readFromString(paraMapFileName);
		paraFileLocsList.readFromString(paraMapFileLocs);

		//Locate the mapper node
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
			e1.printStackTrace();
		}
		String mapperIPAddr = slaves.get(slaveIdx - 1).getIpAddr();
		int mapperPortNum = slaves.get(slaveIdx - 1).getPortNum();
		QuerySecondo secEntity = new QuerySecondo();
		
		try
		{
			ListExpr comMapQuery;
			
			String fdoName = "fdistribute";       //The name of the distribute operator
			if (PSFSMode == 3)
				fdoName = "fdistribute3";
			
			//DLF
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
				pattern = fileNameList.first();
				comMapQuery = ExtListExpr.replace(comMapQuery, InterSymbol, mapQueryList);
				comMapQuery = HPA_AuxFunctions.loc2Ffeed(comMapQuery, fileNameList, fileLocList, duplicateTimes);
				replaced = (!comMapQuery.isEmpty());
			}
			else
			{
				ListExpr inputStreamList = ListExpr.twoElemList(
						ListExpr.symbolAtom("feed"), 
						ListExpr.symbolAtom(InputObjectName));
				comMapQuery = ExtListExpr.replace(comMapQuery, InterSymbol, mapQueryList);
//				pattern = inputStreamList;
			}
			
			secEntity.open(mapperIPAddr, databaseName, mapperPortNum, true);
			if (replaced)
			{
				//Replace the reduce query
				interResultName = "<DLFMark:" + interResultName + "/>";
				ListExpr interPattern  = ListExpr.stringAtom(interResultName);
				ListExpr orgnlPattern  = ListExpr.stringAtom(InputObjectName);
				reduceQueryList = ExtListExpr.replace(reduceQueryList, orgnlPattern, interPattern);
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
									secondoSlaveIdx			+ inDim +
									databaseName				+ inDim +
									parameters[6]				+ inDim +		//create Object Name
									CreateFilePath			+ inDim +
									reduceQuery					+ inDim +
									interResultName			+ inDim +
									parameters[10]			+ inDim +		// reducer DLF name list
									parameters[11]			+ inDim +		// reduce DLF loc list
									parameters[13]			+ inDim +   // output kind
									PSFSMode                        // PSFS mode
							));
				}
			}
			else
			{
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
