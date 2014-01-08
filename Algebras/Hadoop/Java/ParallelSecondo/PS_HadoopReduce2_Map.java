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


public class PS_HadoopReduce2_Map 
	extends Mapper<LongWritable, Text, IntWritable, Text> 
	implements Constant
{

	@Override
	protected void map(LongWritable key, Text value, Context context) 
		throws IOException, InterruptedException 
	{
		String parameters[] = value.toString().split(inDim);
		int 		slaveIdx					= Integer.parseInt(parameters[0]);							 							
		String 	databaseName 			= parameters[1];		
		String 	CreateObjectName 	= parameters[2];		
		String 	reduceQuery 			= parameters[3];
		String 	mapFileNames	 		= parameters[4];
		String 	mapFileLocs				= parameters[5];
		String 	mapObjNames	 			= parameters[6];
		String 	mapObjLocs				= parameters[7];
		//The parameters[8-9] is prepared for reduce tasks
		String CreateFilePath 		= parameters[10];
		FListKind outputKind 			= 
			FListKind.values()[Integer.parseInt(parameters[11])];
		String[] InputObjectName	= {parameters[12], parameters[15]};
		int[] 	 duplicateTimes   = {Integer.parseInt(parameters[13]), 
																 Integer.parseInt(parameters[16])};
		String[] PAName						= {parameters[14], parameters[17]};
		int PSFSMode              = Integer.parseInt(parameters[18]);
		
		
		ListExpr fpList = new ListExpr();
		fpList.readFromString(CreateFilePath);
		CreateFilePath = fpList.first().textValue();
		
		int secondoSlaveIdx = slaveIdx + 1;
		String[] interResultName = { "P" + 1 + "_" + context.getJobName(),
																 "P" + 2 + "_" + context.getJobName()};
		
		ListExpr fileNameList = new ListExpr(), fileLocList = new ListExpr();
		fileNameList.readFromString(mapFileNames);
		fileLocList.readFromString(mapFileLocs);
		ListExpr ObjNameList = new ListExpr(), ObjLocList = new ListExpr();
		ObjNameList.readFromString(mapObjNames);
		ObjLocList.readFromString(mapObjLocs);
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
		try
		{
			ListExpr InterSymbol = ListExpr.symbolAtom(QUERYNLSTR);
			ListExpr[] comMapQuery = 
				{ListExpr.theEmptyList(), ListExpr.theEmptyList()};
			boolean bothReplaced = true;
			for (int side = 0; side < 2; side++)
			{
				String fdoName = "fdistribute";
				if (PSFSMode == 3)
					fdoName = "fdistribute3";

				//Each time process one side input
				comMapQuery[side] = ListExpr.twoElemList(
						ListExpr.symbolAtom("query"),
						ListExpr.twoElemList(
								ListExpr.symbolAtom("tconsume"), 
								ListExpr.threeElemList(
										ListExpr.symbolAtom("project"), 
										ListExpr.sixElemList(
												ListExpr.symbolAtom(fdoName),
												InterSymbol, 
												ListExpr.fourElemList(
														ListExpr.stringAtom(interResultName[side]), 
														ListExpr.textAtom(CreateFilePath),
														ListExpr.symbolAtom(PAName[side]),
														ListExpr.intAtom(secondoSlaveIdx)), 
														ListExpr.twoElemList(
																ListExpr.intAtom(context.getNumReduceTasks()), 
																ListExpr.boolAtom(true)),
																ListExpr.theEmptyList(), 
																ListExpr.theEmptyList()), 
																ListExpr.oneElemList(
																		ListExpr.symbolAtom("Suffix")))));

				ListExpr restNameList = fileNameList;
				ListExpr restLocList  = fileLocList;
				ListExpr inputStreamList = null;
				boolean replaced = true;
				ListExpr pattern = null;
				boolean isInputFile = false;
				
				if (!fileNameList.isEmpty())
				{
					//DLF input
					restNameList = fileNameList;
					while(!restNameList.isEmpty())
					{
						String 	 fileName = restNameList.first().first().stringValue();
						ListExpr fileLoc  = restLocList.first();
						if (fileName.compareTo(InputObjectName[side]) == 0)
						{
							inputStreamList = restNameList.first().first();
							
							comMapQuery[side] = ExtListExpr.replace(
									comMapQuery[side], InterSymbol, inputStreamList);
							comMapQuery[side] = HPA_AuxFunctions.loc2Ffeed(comMapQuery[side], 
									ListExpr.oneElemList(inputStreamList), 
									ListExpr.oneElemList(fileLoc), duplicateTimes[side]);
							
							replaced = (!comMapQuery[side].isEmpty());
							pattern = restNameList.first().first();
							
							isInputFile = true;
							break;
						}
						
						restNameList = restNameList.rest();
						restLocList  = restLocList.rest();
					}
				}
				
				if (!isInputFile)
				{
					//DLO input
					inputStreamList = ListExpr.twoElemList(
							ListExpr.symbolAtom("feed"),
							ListExpr.symbolAtom(InputObjectName[side]));
					
					boolean isObjExist = HPA_AuxFunctions.objectExist(
							InputObjectName[side] ,ObjNameList, ObjLocList);
					if (isObjExist){
						comMapQuery[side] = ExtListExpr.replace(
								comMapQuery[side], InterSymbol, inputStreamList);
					}
					else{
						//Not execute the query if the required DLO sub-object doesn't exist.
						comMapQuery[side] = ListExpr.theEmptyList();
					}
					
					pattern = inputStreamList;
					replaced = true;
				}
				
				//Use the intermediate result to replace the input flist 
				interResultName[side] = "<DLFMark:" + interResultName[side] + "/>";
				ListExpr interPattern  = ListExpr.stringAtom(interResultName[side]);
				
				if (replaced){

					//In case a same object is used several times inside the query.
					reduceQueryList = ExtListExpr.replaceFirst( reduceQueryList, pattern, interPattern);
					reduceQuery = HPA_AuxFunctions.plainStr(reduceQueryList);
					
					//Build up the map query
					comMapQuery[side] = ExtListExpr.replace(comMapQuery[side], InterSymbol, inputStreamList);
				}

				bothReplaced &= replaced;
			}
			
			secEntity.open(mapperIPAddr, databaseName, mapperPortNum, true);
			for (int side = 0; side < 2; side++) {
				if (!comMapQuery[side].isEmpty()) {
					List<Integer> rColumns = new ArrayList<Integer>();
					ListExpr resultList = new ListExpr();
					secEntity.query(comMapQuery[side].toString(), resultList);
					resultList = resultList.second();
					ListExpr rest = resultList;
					while (!rest.isEmpty()) {
						ListExpr item = rest.first();
						rColumns.add(item.first().intValue());
						rest = rest.rest();
					}

					if (!bothReplaced) {
						// Empty the reduce query, if not both input been replaced by  inter-result-name
						reduceQuery = "";
					}

					for (int column : rColumns) {
						context.write(new IntWritable(column), 
								new Text("" + 
										side 										+inDim+ 
										secondoSlaveIdx 				+inDim+ 
										interResultName[side] 	+inDim+ 
										databaseName 						+inDim+ 
										CreateObjectName 				+inDim+ 
										CreateFilePath 					+inDim+ 
										reduceQuery 						+inDim+ 
										parameters[8] 					+inDim+ 
										parameters[9] 					+inDim+ 
										outputKind.ordinal()    +inDim+
										PSFSMode                +inDim+
										""));
					}
				} else {
					System.err.println("2: The construction of map query fails");
				}
			}
			secEntity.close();

		}  catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RemoteStreamException("Catch IOException in Map task");
		}
		
		
	}

	
	
}
