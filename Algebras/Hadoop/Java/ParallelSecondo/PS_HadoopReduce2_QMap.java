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


public class PS_HadoopReduce2_QMap 
	extends Mapper<LongWritable, Text, IntWritable, Text> 
	implements Constant
{

	/* (non-Javadoc)
	 * @see org.apache.hadoop.mapreduce.Mapper#map(java.lang.Object, java.lang.Object, org.apache.hadoop.mapreduce.Mapper.Context)
	 */
	@Override
	protected void map(LongWritable key, Text value,
			Context context) throws IOException,
			InterruptedException {
		
		String parameters[] = value.toString().split(inDim);
		int 		mapperIdx					= Integer.parseInt(parameters[0]);							 							
		String 	databaseName 			= parameters[1];
		String 	reduceQuery 			= parameters[3];
		String CreateFilePath 		= parameters[4];
		ListExpr fpList = new ListExpr();
		fpList.readFromString(CreateFilePath);
		CreateFilePath = fpList.first().textValue();
		//-----------------------------------------------------------
		String[] 	InputObjectName		= { parameters[8], parameters[15]} ;
		int[] 		slaveIdx					= 
									{ Integer.parseInt(parameters[9]), Integer.parseInt(parameters[16])};							 							
		int[] 		duplicateTimes    = 
									{ Integer.parseInt(parameters[10]), Integer.parseInt(parameters[17])};
		String[] 	PAName						= { parameters[11], parameters[18]} ;
		String[] 	mapCreateQuery 		= { parameters[12], parameters[19]} ;
		String[] 	mapFileName 			= { parameters[13], parameters[20]} ;
		String[] 	mapFileLoc				= { parameters[14], parameters[21]} ;
		
		String[] interResultName = { 
				"P" + 1 + "_" + context.getJobName(),
				"P" + 2 + "_" + context.getJobName()};
		int secondoSlaveIdx = mapperIdx + 1;
		ListExpr reduceQueryList = new ListExpr();
		reduceQueryList.readFromString(reduceQuery);

		
		//----------- Prepare All Parameters

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
		try
		{
			ListExpr InterSymbol = ListExpr.symbolAtom(QUERYNLSTR);
			ListExpr[] comMapQuery = 
				{ListExpr.theEmptyList(), ListExpr.theEmptyList()};
			boolean bothReplaced = true;
			for (int side = 0; side < 2; side++)
			{
				comMapQuery[side] = ListExpr.twoElemList(
					ListExpr.symbolAtom("query"),
						ListExpr.twoElemList(
								ListExpr.symbolAtom("tconsume"), 
								ListExpr.threeElemList(
										ListExpr.symbolAtom("project"), 
										ListExpr.sixElemList(
												ListExpr.symbolAtom("fdistribute"),
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
											ListExpr.oneElemList(ListExpr.symbolAtom("Suffix")))));
				
				boolean replaced = true;
				ListExpr pattern = null;
				ListExpr mapDLFNameList = new ListExpr(), mapDLFLocList = new ListExpr();
				mapDLFNameList.readFromString(mapFileName[side]);
				mapDLFLocList.readFromString(mapFileLoc[side]);
				if (!mapDLFNameList.isEmpty())
				{
					pattern = mapDLFNameList.first();
					comMapQuery[side] = ExtListExpr.replace(comMapQuery[side], InterSymbol, pattern);
					comMapQuery[side] = HPA_AuxFunctions.loc2Ffeed(
							comMapQuery[side], ListExpr.oneElemList(pattern), 
							mapDLFLocList, duplicateTimes[side]);
					replaced = (!comMapQuery[side].isEmpty());
				}
				else
				{
					pattern = ListExpr.twoElemList(
							ListExpr.symbolAtom("feed"), 
							ListExpr.symbolAtom(InputObjectName[side]));
					comMapQuery[side] = ExtListExpr.replace(
							comMapQuery[side], InterSymbol, pattern);
				}
				
				interResultName[side] = "<DLFMark:" + interResultName[side] + "/>";
				ListExpr interPattern  = ListExpr.stringAtom(interResultName[side]);
				ListExpr orgnlPattern  = ListExpr.stringAtom(InputObjectName[side]);
				reduceQueryList = ExtListExpr.replace(reduceQueryList, orgnlPattern, interPattern);
				
				bothReplaced &= replaced;
			}
			reduceQuery = HPA_AuxFunctions.plainStr(reduceQueryList);
			
			secEntity.open(mapperIPAddr, databaseName, mapperPortNum, true);
			for (int side = 0; side < 2; side++)
			{
				if (!comMapQuery[side].isEmpty())
				{
					List<Integer> rColumns = new ArrayList<Integer>();
					ListExpr resultList = new ListExpr();
					secEntity.query(comMapQuery[side].toString(), resultList);
					resultList = resultList.second();
					ListExpr rest = resultList;
					while (!rest.isEmpty()) 
					{
						ListExpr item = rest.first();
						rColumns.add(item.first().intValue());
						rest = rest.rest();
					}
					
					if (!bothReplaced) {
						// Empty the reduce query, if not both input been replaced by  inter-result-name
						reduceQuery = "";
					}

					for (int column : rColumns)
					{
						context.write(new IntWritable(column), 
								new Text("" +
										side										+inDim+
										secondoSlaveIdx					+inDim+
										interResultName[side]		+inDim+
										databaseName						+inDim+
										parameters[2]						+inDim+//CreateObjectName
										CreateFilePath					+inDim+
										reduceQuery							+inDim+
										parameters[6]						+inDim+//frnstr
										parameters[7]						+inDim+//frlstr
										parameters[5]						+inDim+//outputKind.ordinal
								""));
					}
				}
				else
				{
					System.err.println("2: The construction of map query fails");
				}
			}
			secEntity.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RemoteStreamException("Catch IOException in Map task");
		}
	}
}
