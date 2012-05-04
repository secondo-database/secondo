package ParallelSecondo;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

import org.apache.hadoop.io.BooleanWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;

import sj.lang.ListExpr;

public class PS_HadoopMap_Map 
	extends Mapper<LongWritable, Text, IntWritable, BooleanWritable> 
	implements Constant
{

	@Override
	protected void map(LongWritable key, Text value, Context context) 
	throws IOException, InterruptedException {
	
		String parameters[] = value.toString().split(inDim);
		int 		slaveIdx					= Integer.parseInt(parameters[0]);							 							
		String 	databaseName 			= parameters[1];		
		String 	CreateObjectName 	= parameters[2];		
		String 	CreateQuery 			= parameters[3];
		String 	mapFileName 			= parameters[4];
		String 	mapFileLoc				= parameters[5];
		int 		duplicateTimes    = Integer.parseInt(parameters[6]);
		
		String  CreateFilePath = parameters[7];
		ListExpr fpList = new ListExpr();
		fpList.readFromString(CreateFilePath);
		CreateFilePath = fpList.first().textValue();
		
		int secondoSlaveIdx  = slaveIdx + 1;
		
		FListKind outputKind = FListKind.values()[Integer.parseInt(parameters[8])];
		
		ListExpr fileNameList = new ListExpr(), fileLocList = new ListExpr();
		fileNameList.readFromString(mapFileName);
		fileLocList.readFromString(mapFileLoc);
		ListExpr queryList = new ListExpr();
		queryList.readFromString(CreateQuery);
		
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
			//Prepare the creation query first
			ListExpr comCreateQuery;
			ListExpr comQuery_last;
			if (outputKind == FListKind.DLO)
			{
				//DLO
				comCreateQuery = ListExpr.oneElemList(ListExpr.symbolAtom("let"));
				comQuery_last = comCreateQuery;
				comQuery_last = 
					ListExpr.append(comQuery_last, ListExpr.symbolAtom(CreateObjectName));
				comQuery_last = 
					ListExpr.append(comQuery_last, ListExpr.symbolAtom("="));
				comQuery_last = 
					ListExpr.append(comQuery_last, ListExpr.symbolAtom(QUERYNLSTR));
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
									ListExpr.intAtom(secondoSlaveIdx),
									ListExpr.intAtom(1)), 
							ListExpr.theEmptyList(), 
							ListExpr.theEmptyList()));
			}
			
			ListExpr restNameList = fileNameList;
			ListExpr restLocList  = fileLocList;
			boolean replaced = true;             
				//When the input is a DLO, then no need for replace
			
			while (!restNameList.isEmpty())
			{
				ListExpr nameList = restNameList.first();
				String fileName = nameList.stringValue();
				fileName = fileName.substring(fileName.lastIndexOf(':') + 1, 
						fileName.lastIndexOf("/>"));
				
				ListExpr fileLoc = restLocList.first();
				if (fileLoc.isEmpty()){
					replaced = false;
					break;
				}
				
				ListExpr inputStreamList = null;
				if (!fileLoc.isEmpty())
				{
					int rowNum = 0;
					ListExpr restFileLoc = fileLoc;
					while (!restFileLoc.isEmpty())
					{
						int 			row 			= restFileLoc.first().first().intValue();
						ListExpr 	columns 	= restFileLoc.first().second();
						String   	filePath	= restFileLoc.first().third().textValue();
						
						ListExpr suffixTRel = null;
						if (!columns.isEmpty()){
							ListExpr trelLast = null;
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
						}
						else{
							suffixTRel = ListExpr.theEmptyList();
						}
						
						// Avoid generating normal relation files in database
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
							//Concat the former one with the current one.
							inputStreamList = ListExpr.threeElemList(
									ListExpr.symbolAtom("concat"), 
									inputStreamList, ffeedOneRow);
						}
						
						restFileLoc = restFileLoc.rest();
						rowNum++;
					}
				}

				if (replaced) {
					// Replace the flist with the inputstream.
					queryList = ExtListExpr.replace(queryList, nameList, inputStreamList);
				}
				
				restNameList = restNameList.rest();
				restLocList  = restLocList.rest();
			}
						
			secEntity.open(mapperIPAddr, databaseName, mapperPortNum, true);
			if (replaced)
			{
				comCreateQuery = 
					ExtListExpr.replaceFirst(comCreateQuery, QUERYNLSTR, queryList);
				
				//It is possible that the database doesn't exist in slave database
				ListExpr resultList = new ListExpr();
				
				if (outputKind == FListKind.DLO){
					secEntity.query("delete " + CreateObjectName, resultList, true);
				}
				secEntity.query(comCreateQuery.toString(), resultList);
				
				//Create a local object or partition file in this slave
				context.write(
						new IntWritable(secondoSlaveIdx), new BooleanWritable(true));
			}
			else{
				//Nothing is created in this slave
				context.write(
						new IntWritable(secondoSlaveIdx), new BooleanWritable(false));
			}
			secEntity.close();
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RemoteStreamException("Catch IOException in Map task");
		}

	}
	
}
