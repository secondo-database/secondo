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

public class PS_HadoopMap2_Map 
	extends Mapper<LongWritable, Text, Text, BooleanWritable> 
	implements Constant
{

	@Override
	protected void map(LongWritable key, Text value, Context context) 
	throws IOException, InterruptedException {
	
//1. read all given parameters
		//Expect 10 parameters
		String parameters[] = value.toString().split(inDim);
		int     slaveIdx         = Integer.parseInt(parameters[0]);							 							
		int     mapperIdx        = Integer.parseInt(parameters[1]);							 							
		String 	databaseName     = parameters[2];		
		String 	CreateObjectName = parameters[3];		
		String 	CreateQuery      = parameters[4];
		String  DLF_Name_List    = parameters[5];  
		String  dlfLocStr        = parameters[6];  
		int 	duplicateTimes   = Integer.parseInt(parameters[7]);

		String  CreateFilePath = parameters[8];	
		ListExpr fpList = new ListExpr();
		fpList.readFromString(CreateFilePath);
		CreateFilePath = fpList.first().textValue();
		
		FListKind outputKind = FListKind.values()[Integer.parseInt(parameters[9])];	
		
		ListExpr fileNameList = new ListExpr(), fileLocList = new ListExpr();
		fileNameList.readFromString(DLF_Name_List);
		fileLocList.readFromString(dlfLocStr);
		ListExpr queryList = new ListExpr();
		queryList.readFromString(CreateQuery);
		
//2. determine the processing node		
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


//3. create the processing query
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
									ListExpr.intAtom(mapperIdx),
									ListExpr.intAtom(1)), 
							ListExpr.theEmptyList(), 
							ListExpr.theEmptyList()));
			}
			
			queryList = HPA_AuxFunctions.loc2Ffeed(
					queryList, fileNameList, fileLocList, duplicateTimes);
			boolean replaced = (!queryList.isEmpty());
						
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
						new Text(mapperIdx + " " + slaveIdx), new BooleanWritable(true));
			}
			else{
				//Nothing is created in this slave
				context.write(
						new Text(mapperIdx + " " + slaveIdx), new BooleanWritable(false));
			}
			secEntity.close();
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RemoteStreamException("Catch IOException in Map task");
		}

	}
	
}
