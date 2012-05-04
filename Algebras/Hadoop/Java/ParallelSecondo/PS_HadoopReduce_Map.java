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

		int secondoSlaveIdx = slaveIdx + 1;
		String interResultName = "Partition_"+ context.getJobName();
		
		ListExpr fileNameList = new ListExpr(), fileLocList = new ListExpr();
		fileNameList.readFromString(mapFileNames);
		fileLocList.readFromString(mapFileLocs);
		ListExpr reduceQueryList = new ListExpr();

		//replace the input argument in the reduce query
		ListExpr newFileNameList = new ListExpr(), newFileLocList = new ListExpr();
		ListExpr newFileNameList_last = null, newFileLocList_last = null;
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
			ListExpr comMapQuery;
			ListExpr InterSymbol = ListExpr.symbolAtom(QUERYNLSTR);
			comMapQuery = ListExpr.twoElemList(
					ListExpr.symbolAtom("query"),
						ListExpr.twoElemList(
								ListExpr.symbolAtom("tconsume"), 
								ListExpr.threeElemList(
										ListExpr.symbolAtom("project"), 
										ListExpr.sixElemList(
												ListExpr.symbolAtom("fdistribute"),
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
	
			boolean isInputFile = false;
			ListExpr restNameList = fileNameList;
			ListExpr restLocList  = fileLocList;
			ListExpr inputStreamList = null;
			boolean replaced = true;
			
			while (!restNameList.isEmpty())
			{
				String 	 fileName = restNameList.first().stringValue();
				ListExpr fileLoc  = restLocList.first();
				
				System.out.println("The fileName is : " + fileName);
				System.out.println("The InputObjectName is : " + InputObjectName);
				if (fileName.compareTo(InputObjectName) == 0)
				{
					if (fileLoc.isEmpty())
					{
						System.out.println("The fileLoc is empty: " + fileName );
						replaced = false;
						break;
					}
/*
The input flist is DLF kind, since its object name is listed in the DLF name list
Therefore, it will build up a loopsel+ffeed query to get its value
 
*/
					fileName = fileName.substring(
							fileName.lastIndexOf(':') + 1, fileName.lastIndexOf("/>"));
					
					isInputFile = true;

					int rowNum = 0;
					ListExpr restFileLoc = fileLoc;
					while (!restFileLoc.isEmpty())
					{
						int 			row 			= restFileLoc.first().first().intValue();
						ListExpr 	columns 	= restFileLoc.first().second();
						String   	filePath	= restFileLoc.first().third().textValue();

						//Build up a temporal relation of column numbers, 
						//prepared for the later loopsel operation
						ListExpr suffixTRel = null, trelLast = null;
						while (!columns.isEmpty()){
							int column = columns.first().intValue();
							
							if (suffixTRel == null){
								suffixTRel = ListExpr.oneElemList(
										ListExpr.oneElemList(ListExpr.intAtom(column)));
								trelLast = suffixTRel;
							}
							else{
								trelLast = ListExpr.append( trelLast, 
												ListExpr.oneElemList(ListExpr.intAtom(column)));
							}
							columns = columns.rest();
						}
						suffixTRel = ListExpr.twoElemList(
								ListExpr.twoElemList(
									ListExpr.symbolAtom("trel"), 
									ListExpr.twoElemList(
										ListExpr.symbolAtom("tuple"), 
										ListExpr.oneElemList(
											ListExpr.twoElemList(
												ListExpr.symbolAtom(fsName), 
												ListExpr.symbolAtom("int"))))), 
									suffixTRel);
						suffixTRel = ListExpr.twoElemList(
								ListExpr.symbolAtom("feed"),
								suffixTRel);

						//Add with append operator
						String tupleParaName = "XxxTP" + rowNum;
						ListExpr ffeedOneRow = 
							ListExpr.oneElemList(ListExpr.symbolAtom("ffeed"));
						ListExpr ffeed_last = ffeedOneRow;
						ffeed_last = 
							ListExpr.append(ffeed_last, ListExpr.stringAtom(fileName));
						ffeed_last = ListExpr.append(ffeed_last, 
								ListExpr.threeElemList(ListExpr.textAtom(filePath), 
										ListExpr.intAtom(row),
										ListExpr.threeElemList(
												ListExpr.symbolAtom("attr"), 
												ListExpr.symbolAtom(tupleParaName), 
												ListExpr.symbolAtom(fsName))));
						ffeed_last = ListExpr.append(ffeed_last, ListExpr.theEmptyList()); 
						//Search type file at local disk

						ffeed_last = ListExpr.append(ffeed_last, 
								ListExpr.threeElemList(
										ListExpr.intAtom(secondoSlaveIdx), 
										ListExpr.intAtom(secondoSlaveIdx), 
										ListExpr.intAtom(duplicateTimes)));
						//Search data file at local disk too

						ffeedOneRow = ListExpr.threeElemList(
								ListExpr.symbolAtom("loopsel"), 
								suffixTRel, 
								ListExpr.threeElemList(
									ListExpr.symbolAtom("fun"), 
										ListExpr.twoElemList(
											ListExpr.symbolAtom(tupleParaName), 
											ListExpr.symbolAtom("TUPLE")),
											ffeedOneRow));
						
						if (rowNum == 0){
							inputStreamList = ffeedOneRow;
						}
						else{
							//Concatenate the former one with the current one.
							inputStreamList = ListExpr.threeElemList(
									ListExpr.symbolAtom("concat"), 
									inputStreamList, ffeedOneRow);
						}
						restFileLoc = restFileLoc.rest();
						rowNum++;
					}
				}
				else
				{
					if (newFileNameList.isEmpty()){
						newFileNameList = ListExpr.oneElemList(restNameList.first());
						newFileNameList_last = newFileNameList;
						newFileLocList = ListExpr.oneElemList(fileLocList.first());
						newFileLocList_last = newFileLocList;
					}
					else{
						newFileNameList_last = 
							ListExpr.append(newFileNameList_last, restNameList.first());
						newFileLocList_last = 
							ListExpr.append(newFileLocList_last, fileLocList.first());
					}
				}
				
				restNameList = restNameList.rest();
				restLocList  = restLocList.rest();
			}
			mapFileNames = newFileNameList.toString().replace("\n", " ");
			mapFileLocs = newFileLocList.toString().replace("\n", " ");
			
			ListExpr pattern = null;
			if (isInputFile){
				pattern = ListExpr.stringAtom(InputObjectName);
			}
			else{
				pattern = ListExpr.stringAtom("InputObjectName");
				inputStreamList = ListExpr.twoElemList(
						ListExpr.symbolAtom("feed"),
						ListExpr.symbolAtom(InputObjectName));
			}
			
			//Use the intermediate result to replace the input flist 
			interResultName = "<DLFMark:" + interResultName + "/>";
			ListExpr interPattern  = ListExpr.stringAtom(interResultName);
			reduceQueryList = 
				ExtListExpr.replace(reduceQueryList, pattern, interPattern);
			reduceQuery = reduceQueryList.toString().replace("\n", " ");
			//Build up the map query
			comMapQuery = 
				ExtListExpr.replace(comMapQuery, InterSymbol, inputStreamList);
			
			secEntity.open(mapperIPAddr, databaseName, mapperPortNum, true);
			if (replaced)
			{
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
									outputKind.ordinal() 
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
