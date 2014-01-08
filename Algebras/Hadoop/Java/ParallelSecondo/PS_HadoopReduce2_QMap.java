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
		String AcceptFileNameList = parameters[6];
		String AcceptFileLocList = parameters[7];
		String AcceptDLOName = parameters[8];
		String AcceptDLOLoc = parameters[9];
		//-----------------------------------------------------------
		String[] 	InputObjectName		= { parameters[12], parameters[21]} ;
		int[] 		slaveIdx					= 
									{ Integer.parseInt(parameters[13]), Integer.parseInt(parameters[22])};							 							
		int[] 		duplicateTimes    = 
									{ Integer.parseInt(parameters[14]), Integer.parseInt(parameters[23])};
		String[] 	PAName						= { parameters[15], parameters[24]} ;
		String[] 	mapCreateQuery 		= { parameters[16], parameters[25]} ;
		String[] 	mapFileName 			= { parameters[17], parameters[26]} ;
		String[] 	mapFileLoc				= { parameters[18], parameters[27]} ;
		String[]	mapObjName				= { parameters[19], parameters[28]} ;
		String[]	mapObjLoc					= { parameters[20], parameters[29]} ;
		int       PSFSMode          = Integer.parseInt(parameters[30]);

		ListExpr recvFileList = new ListExpr();
		recvFileList.readFromString(AcceptFileLocList);

		
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
				String fdoName = "fdistribute";
				if (PSFSMode == 3)
					fdoName = "fdistribute3";
				
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
											ListExpr.oneElemList(ListExpr.symbolAtom("Suffix")))));
				
				boolean replaced = true;
				boolean isInputFile = true;
				ListExpr inputStream = null;
				
				ListExpr mapQueryList = new ListExpr();
				mapQueryList.readFromString(mapCreateQuery[side]);

				ListExpr mapDLFNameList = new ListExpr(), mapDLFLocList = new ListExpr();
				mapDLFNameList.readFromString(mapFileName[side]);
				mapDLFLocList.readFromString(mapFileLoc[side]);

				if (!mapQueryList.isEmpty())
				{
					//Embed the unexecuted map Query
					if (!mapDLFNameList.isEmpty())
					{
						inputStream = mapDLFNameList.first();
						comMapQuery[side] = ExtListExpr.replace(comMapQuery[side], InterSymbol, mapQueryList);
						comMapQuery[side] = HPA_AuxFunctions.loc2Ffeed(
								comMapQuery[side], ListExpr.oneElemList(inputStream), 
								mapDLFLocList, duplicateTimes[side]);
						replaced = (!comMapQuery[side].isEmpty());
					}
					else
					{
						ListExpr omnList = new ListExpr();
						ListExpr omlList = new ListExpr();
						omnList.readFromString(mapObjName[side]);
						omlList.readFromString(mapObjLoc[side]);
						ListExpr nmRest = omnList;
						boolean isObjExist = true;
						while (!nmRest.isEmpty())
						{
						  String objName = nmRest.first().stringValue();
						  if (objName.matches(DLOFPattern))
						  {
						    objName = objName.substring(objName.lastIndexOf(':') +1, objName.lastIndexOf("/>"));
						    if ( !HPA_AuxFunctions.objectExist(objName, omnList, omlList))
						    {
						      isObjExist = false;
						      break;
						    } 
					  	  }
						  nmRest = nmRest.rest();
						}
						if (isObjExist)
						{
						  comMapQuery[side] = ExtListExpr.replace(comMapQuery[side], InterSymbol, mapQueryList);
						  replaced = (!comMapQuery[side].isEmpty());
						}
						else
						{
						  comMapQuery[side] = ListExpr.theEmptyList();
						  replaced = false;
						}
					}
				}
				else
				{
					//Follow the old way
					if (InputObjectName[side].matches(INDLFPattern))
					{
						inputStream = ListExpr.stringAtom(InputObjectName[side]);
						comMapQuery[side] = ExtListExpr.replace(comMapQuery[side], InterSymbol, inputStream);
						ListExpr mapFileLocList = null;
						if (side == 0)
							mapFileLocList = recvFileList.first();
						else 
							mapFileLocList = recvFileList.second();
						comMapQuery[side] = HPA_AuxFunctions.loc2Ffeed(
								comMapQuery[side], ListExpr.oneElemList(inputStream), 
								ListExpr.oneElemList(mapFileLocList), duplicateTimes[side]);
						replaced = (!comMapQuery[side].isEmpty());
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
							comMapQuery[side] = ExtListExpr.replace(comMapQuery[side], InterSymbol, inputStream);
						}
						else{
							comMapQuery[side] = ListExpr.theEmptyList();
						}
					}
				}

				interResultName[side] = "<DLFMark:" + interResultName[side] + "/>";
				ListExpr interPattern  = ListExpr.stringAtom(interResultName[side]);
				ListExpr orgnlPattern  = null;
				if (isInputFile) 
					orgnlPattern = ListExpr.stringAtom(InputObjectName[side]);
				else
					orgnlPattern = inputStream;

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
										parameters[10]					+inDim+//frnstr
										parameters[11]					+inDim+//frlstr
										parameters[5]						+inDim+//outputKind.ordinal
										PSFSMode                +inDim+
								""));
					}
				}
				else
				{
					System.err.println("Warning: The construction of map query fails, " +
							"it is possible that the inputStream doesn't exist. ");
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
