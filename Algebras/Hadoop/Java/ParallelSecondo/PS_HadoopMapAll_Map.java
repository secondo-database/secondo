package ParallelSecondo;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;


import org.apache.hadoop.io.BooleanWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;

import sj.lang.ListExpr;

/*

This map task should return: 

mapperIdx, slaveIdx, success, result. 
 
*/
public class PS_HadoopMapAll_Map extends
		Mapper<LongWritable, Text, Text, Text> implements Constant {

	/* (non-Javadoc)
	 * @see org.apache.hadoop.mapreduce.Mapper#map(java.lang.Object, java.lang.Object, org.apache.hadoop.mapreduce.Mapper.Context)
	 */
	@Override
	protected void map(LongWritable key, Text value, Context context) 
	throws IOException, InterruptedException {

		String parameters[] = value.toString().split(inDim);
		int 		slaveIdx					= Integer.parseInt(parameters[0]);							 							
		int 		mapperIdx					= Integer.parseInt(parameters[1]);							 							
		String 	databaseName 			= parameters[2];		
		String 	taskQuery 				= parameters[3];
		String 	mapFileName 			= parameters[4];
		String 	mapFileLoc				= parameters[5];
		int 		duplicateTimes    = 1;

		ListExpr fileNameList = new ListExpr(), fileLocList = new ListExpr();
		fileNameList.readFromString(mapFileName);
		fileLocList.readFromString(mapFileLoc);
		ListExpr queryList = new ListExpr();
		queryList.readFromString(taskQuery);

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
		ListExpr taskResult;
		try
		{
			ListExpr comTaskQuery;
			boolean replaced = true;
			ListExpr InterSymbol = ListExpr.symbolAtom(QUERYNLSTR);
			comTaskQuery = ListExpr.twoElemList(
					ListExpr.symbolAtom("query"), InterSymbol);
			if (fileNameList.listLength() > 0){
				queryList = HPA_AuxFunctions.loc2Ffeed(
						queryList, fileNameList, fileLocList, duplicateTimes);
				replaced = (!queryList.isEmpty());
			}
			
			secEntity.open(mapperIPAddr, databaseName, mapperPortNum, true);
			if (replaced)
			{
				comTaskQuery = ExtListExpr.replaceFirst(
						comTaskQuery, QUERYNLSTR, queryList);

				//It is possible that the database doesn't exist in slave database
				ListExpr resultList = new ListExpr();
				secEntity.query(comTaskQuery.toString(), resultList, true);
				
				System.out.println("The result is: " + resultList.toString());
				
				taskResult = ListExpr.twoElemList(
						ListExpr.boolAtom(true), resultList.second());
				
				//Create a local object or partition file in this slave
				context.write(
						new Text(mapperIdx + " " + slaveIdx),
						new Text(taskResult.toString()));
			}
			else
			{
				//Nothing is created in this slave
				taskResult = ListExpr.twoElemList(
						ListExpr.boolAtom(false), 
						ListExpr.theEmptyList());
				context.write(
						new Text(mapperIdx + " " + slaveIdx), 
						new Text(taskResult.toString()));
			}


		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			taskResult = ListExpr.twoElemList(
					ListExpr.boolAtom(false), 
					ListExpr.theEmptyList());
			context.write(
					new Text(mapperIdx + " " + slaveIdx), 
					new Text(taskResult.toString()));
			//throw new RemoteStreamException("Catch IOException in Map task");
		}
		secEntity.close();
	}

}
