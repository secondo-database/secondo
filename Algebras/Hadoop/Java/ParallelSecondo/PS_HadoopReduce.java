package ParallelSecondo;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

import sj.lang.ListExpr;

public class PS_HadoopReduce implements Constant{
	
	private final static String JOBID = "HASEC_JOB_" +
  new Timestamp(System.currentTimeMillis()).toString()
  .replace("-", "").replace(".", "")
  .replace(" ", "").replace(":", "");

	/**
	 * @param args
	 * 
	 * DatabaseName
	 * CreateObjectName
	 * CreateQuery
	 * DLF_Name_List
	 * DLF_fileLoc_List
	 * DLO_Name_List
	 * DLO_Loc_List
	 * duplicateTimes
	 * fList_Kind
	 * createFilePath
	 * InputObjectName 
	 * [ 	MapQuery 
	 * 	 	Map_DLF_Name_List
	 * 	 	Map_DLF_Loc_List
	 * 	 	Map_DLO_Name_List
	 * 	 	Map_DLO_Loc_List
	 * ]
	 * PartAttributeName
	 * ReduceTaskNum
	 */
	public static void main(String[] args) {

		final int paraLength = 13;
		final int paraLength2 = 19;
		String usage = "Usage HadoopReduce <databaseName> " +
				"<CreateObjectName> <CreateQuery> " +
				"<DLF_Name_List> <DLF_fileLoc_List> " +
				"<DLO_Name_List> <DLO_loc_List> " +
				"<duplicateTimes> <FListKind> <FilePath> " +
				"<InputObjectName> [ <MapQuery> " +
				"<Map_DLF_Name_List> <Map_DLF_Loc_List> " +
				"<Map_DLO_Name_List> <Map_DLO_Loc_List> " +
				"<MapTaskNum> ] " +
				"<PartAttributeName> <reduceTasksNum>"; 
		
		boolean runMapper = false;
		if (args.length == paraLength2)
		{
			runMapper = true;
		}
		else if (args.length != paraLength)
		{
			System.err.println(usage);
			System.out.println("You input " + args.length + " arguments");
      System.exit(-1);
		}

		//Get the master and slave nodes information from the files set by
		//PARALLEL_SECONDO_MASTER & PARALLEL_SECONDO_SLAVES
		String slFile = System.getenv().get("PARALLEL_SECONDO_SLAVES");
		if (slFile.length() == 0)
		{
			System.err.println(
				"The Slave list PARALLEL_SECONDO_SLAVES " +
				"is not defined at current node.");
			System.exit(-1);
		}

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

		String databaseName 		= args[0];
		String CreateObjectName	= args[1];
		String CreateQuery	 		= args[2];
		String DLF_Name_ListStr	= args[3];
		String DLF_Loc_ListStr	= args[4];
		String DLO_Name_ListStr	= args[5];
		String DLO_Loc_ListStr	= args[6];
		int duplicateTimes 			= Integer.parseInt(args[7]);
		FListKind outputKind 		= FListKind.values()[Integer.parseInt(args[8])];
		String CreateFilePath		= args[9];
		
		String InputObjectName = "", PAName = "";
		String MapQueryStr = "", 
						Map_DLF_Name_ListStr = "", Map_DLF_Loc_ListStr = "",	
						Map_DLO_Name_ListStr = "", Map_DLO_Loc_ListStr = "";
		int mapTasksNum, reduceTasksNum;
		if (!runMapper)
		{
			InputObjectName	= args[10];
			PAName 					= args[11];
			reduceTasksNum 	= Integer.parseInt(args[12]);
			mapTasksNum 		= slaves.size();
		}
		else
		{
			InputObjectName				= args[10];
			MapQueryStr						= args[11];
			Map_DLF_Name_ListStr 	= args[12];
			Map_DLF_Loc_ListStr 	= args[13];
			Map_DLO_Name_ListStr	= args[14];
			Map_DLO_Loc_ListStr 	= args[15];
			mapTasksNum 						= Integer.parseInt(args[16]);
			PAName 								= args[17];
			reduceTasksNum 				= Integer.parseInt(args[18]);
		}
		
		
		if (outputKind == FListKind.DLO && reduceTasksNum > slaves.size()){
			System.err.println("Warning! It is not allowed to produce DLO flist " +
					"with columns more than slave data servers.");
			System.err.println("Change the reduce tasks number from " + 
					reduceTasksNum + " to " + slaves.size() + " compulsively");
			reduceTasksNum = slaves.size();
		}
			
		
		ListExpr fpList = ListExpr.oneElemList(ListExpr.textAtom(CreateFilePath));
		CreateFilePath = fpList.toString().replace('\n', ' ');  //In case empty path
		
		ListExpr allMapDLFList = new ListExpr();
		ListExpr allMapDLOList = new ListExpr();
		if (runMapper)
		{
			//Prepare the DLO and DLF list for the mapper
			ListExpr allMap_DLFNameList = new ListExpr();
			allMap_DLFNameList.readFromString(Map_DLF_Name_ListStr);
			ListExpr allMap_DLFLocList 	= new ListExpr();
			allMap_DLFLocList.readFromString(Map_DLF_Loc_ListStr);
			ListExpr allMap_DLONameList = new ListExpr();
			allMap_DLONameList.readFromString(Map_DLO_Name_ListStr);
			ListExpr allMap_DLOLocList 	= new ListExpr();
			allMap_DLOLocList.readFromString(Map_DLO_Loc_ListStr);
			
			allMapDLFList = HPA_AuxFunctions.flist2Mapper2(allMap_DLFNameList, allMap_DLFLocList, mapTasksNum, slaves.size());
			allMapDLOList = HPA_AuxFunctions.flist2Mapper2(allMap_DLONameList, allMap_DLOLocList, mapTasksNum, slaves.size());
		}
		
/*
Different from the HadoopMap operator, 
the function query here is processed inside the reduce step, 
therefore, only the function argument is redistributed in the map step, 
while all other DLF flists are decomposed in the reduce query only.  

*/
		
		//Apply each map task with one row of the fileLoc list.
		ListExpr allDLFLocList = new ListExpr();
		allDLFLocList.readFromString(DLF_Loc_ListStr);
		ListExpr allDLFNameList = new ListExpr();
		allDLFNameList.readFromString(DLF_Name_ListStr);
		ListExpr DLFByMappers = HPA_AuxFunctions.flist2Mapper(allDLFNameList, allDLFLocList, mapTasksNum);
		DLFByMappers = HPA_AuxFunctions.divMRDLO(allDLFNameList, DLFByMappers, mapTasksNum);
		
		ListExpr allDLONameList = new ListExpr();
		allDLONameList.readFromString(DLO_Name_ListStr);
		ListExpr allDLOLoclList = new ListExpr();
		allDLOLoclList.readFromString(DLO_Loc_ListStr);
		ListExpr DLOByMappers = HPA_AuxFunctions.flist2Mapper(allDLONameList, allDLOLoclList, mapTasksNum);
		DLOByMappers = HPA_AuxFunctions.divMRDLO(allDLONameList, DLOByMappers, mapTasksNum);


		
		//Prepare the input for mappers
    String inputPath = "INPUT";
    String outputPath = "OUTPUT";
    Configuration conf = new Configuration();
		try
		{
			FileSystem.get(conf).delete(new Path(outputPath), true);
			FileSystem.get(conf).delete(new Path(inputPath), true);

			ListExpr allDLOMappers = DLOByMappers.first();  //All DLO list for mappers (include name and loc)
			ListExpr allDLOReducers = DLOByMappers.second();  //All DLO list for reducers (include name and loc)
			ListExpr allDLFMappers = DLFByMappers.first();  //All DLF list for mappers (include name and loc)
			ListExpr allDLFReducers = DLFByMappers.second();  //All DLF list for reducers (include name and loc)
			
			ListExpr aomn_rest = allDLOMappers.first();
			ListExpr aoml_rest = allDLOMappers.second();
			ListExpr aorn_rest = allDLOReducers.first();
			ListExpr aorl_rest = allDLOReducers.second();
			
			ListExpr afmn_rest = allDLFMappers.first();
			ListExpr afml_rest = allDLFMappers.second();
			ListExpr afrn_rest = allDLFReducers.first();
			ListExpr afrl_rest = allDLFReducers.second();
			
			ListExpr amapDLF_rest = new ListExpr();
			ListExpr amapDLO_rest = new ListExpr();
			if (runMapper)
			{
				amapDLF_rest = allMapDLFList;
				amapDLO_rest = allMapDLOList;
			}
			
			for (int mapperIdx = 0; mapperIdx < mapTasksNum; mapperIdx++)
			{
				
				boolean allDLOexist = true, allDLFexist = true;
				ListExpr aomLoc, afmLoc, aomName, afmName;
				aomLoc 	= ListExpr.theEmptyList();
				afmLoc 	= ListExpr.theEmptyList();
				aomName = ListExpr.theEmptyList();
				afmName = ListExpr.theEmptyList();
				ListExpr amap_DLF, amap_DLO;  //Prepared for running Map stage 
				amap_DLF = ListExpr.theEmptyList(); 
				amap_DLO = ListExpr.theEmptyList();

				if (runMapper)
				{
					if (!amapDLF_rest.isEmpty()) amap_DLF = amapDLF_rest.first();
					if (!amapDLO_rest.isEmpty()) amap_DLO = amapDLO_rest.first();

					allDLOexist = HPA_AuxFunctions.allMapperFOExist(amap_DLO);
					allDLFexist = HPA_AuxFunctions.allMapperFOExist(amap_DLF);
				}
				else
				{
					if (!aoml_rest.isEmpty()) aomLoc = aoml_rest.first();
					if (!afml_rest.isEmpty()) afmLoc = afml_rest.first();
					if (!aomn_rest.isEmpty()) aomName = aomn_rest.first();
					if (!afmn_rest.isEmpty()) afmName = afmn_rest.first();

					if (!aomName.isEmpty())
						allDLOexist = HPA_AuxFunctions.allObjectExist(aomName, aomLoc);
					if (!afmName.isEmpty())
						allDLFexist = HPA_AuxFunctions.allObjectExist(afmName, afmLoc);
				}
				System.out.println(allDLOexist + ", " + allDLFexist);
				
				if (allDLOexist && allDLFexist)
				{
					//For mappers
					String fmnstr = HPA_AuxFunctions.plainStr(afmName);
					String fmlstr = HPA_AuxFunctions.plainStr(afmLoc);

					
					//For reducers
					ListExpr aorLoc, afrLoc, aorName, afrName;
					aorLoc = afrLoc = aorName = afrName = ListExpr.theEmptyList();
					if (!aorl_rest.isEmpty()) aorLoc 	= aorl_rest.first();
					if (!afrl_rest.isEmpty()) afrLoc 	= afrl_rest.first();
					if (!aorn_rest.isEmpty()) aorName = aorn_rest.first();
					if (!afrn_rest.isEmpty()) afrName = afrn_rest.first();

					String frlstr = HPA_AuxFunctions.plainStr(afrLoc);
					String frnstr = HPA_AuxFunctions.plainStr(afrName);

					String fileName = JOBID + "_INPUT_"+ mapperIdx + ".dat";
					PrintWriter out = new PrintWriter(
							FileSystem.get(conf).create(
									new Path(inputPath + "/" + fileName)));
					
					if (runMapper)
					{
						String dlfLocStr = HPA_AuxFunctions.plainStr(amap_DLF);
						int slaveIdx = HPA_AuxFunctions.findFirstSlave(amap_DLF);
						if (slaveIdx == 0)
						{
							slaveIdx = HPA_AuxFunctions.findFirstSlave(amap_DLO);
						}

						out.print("" +
								slaveIdx 							+ inDim +//0
								mapperIdx							+ inDim +//1
								databaseName					+ inDim +//2
								MapQueryStr						+ inDim +//3
								Map_DLF_Name_ListStr	+ inDim +//4
								dlfLocStr							+ inDim +//5
								CreateObjectName			+ inDim +//6
								CreateQuery						+ inDim +//7
								frnstr								+ inDim +//8
								frlstr								+ inDim +//9
								duplicateTimes				+ inDim +//10
								outputKind.ordinal()  + inDim +//11
								CreateFilePath				+ inDim +//12
								InputObjectName				+ inDim +//13
								PAName												 //14
						);
						
						out.close();
					}
					else
					{
						
/*						String fileName = JOBID + "_INPUT_"+ mapperIdx + ".dat";
						PrintWriter out = new PrintWriter(
								FileSystem.get(conf).create(
										new Path(inputPath + "/" + fileName)));
*/						
						out.print( "" + 
								mapperIdx 						+ inDim +
								databaseName 					+ inDim + 
								CreateObjectName 			+ inDim +
								CreateQuery 					+ inDim +
								fmnstr		 						+ inDim +
								fmlstr 								+ inDim +
								frnstr 								+ inDim +
								frlstr 								+ inDim +
								duplicateTimes				+ inDim +
								outputKind.ordinal()  + inDim +
								CreateFilePath				+ inDim +
								InputObjectName				+ inDim +
								PAName								
						);
						out.close();
					}
				}
				
				if (!aorn_rest.isEmpty()) aorn_rest = aorn_rest.rest();
				if (!aorl_rest.isEmpty()) aorl_rest = aorl_rest.rest();
				if (!afrn_rest.isEmpty()) afrn_rest = afrn_rest.rest();
				if (!afrl_rest.isEmpty()) afrl_rest = afrl_rest.rest();
				
				if (runMapper)
				{
					if (!amapDLO_rest.isEmpty()) amapDLO_rest = amapDLO_rest.rest();
					if (!amapDLF_rest.isEmpty()) amapDLF_rest = amapDLF_rest.rest();
				}
				else
				{
					if (!aomn_rest.isEmpty()) aomn_rest = aomn_rest.rest();
					if (!aoml_rest.isEmpty()) aoml_rest = aoml_rest.rest();
					if (!afmn_rest.isEmpty()) afmn_rest = afmn_rest.rest();
					if (!afml_rest.isEmpty()) afml_rest = afml_rest.rest();
				}
			}
			
		}
		catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(-1);
		}
		
		//Create the job
		try {
			Job job = new Job();
			job.setJarByClass(PS_HadoopMap.class);
			
			FileInputFormat.addInputPath(job, new Path(inputPath));
			FileOutputFormat.setOutputPath(job, new Path(outputPath));
			if (runMapper)
				job.setMapperClass(PS_HadoopReduce_QMap.class);
			else
				job.setMapperClass(PS_HadoopReduce_Map.class);
			job.setReducerClass(PS_HadoopReduce_Reduce.class);
			
			job.setMapOutputKeyClass(IntWritable.class);
			job.setMapOutputValueClass(Text.class);
			job.setOutputKeyClass(IntWritable.class);
			job.setOutputValueClass(Text.class);  

			job.setJobName(JOBID);
			job.setNumReduceTasks(reduceTasksNum);
			System.exit(job.waitForCompletion(true) ? 0 : 1);
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


	}
	
}
