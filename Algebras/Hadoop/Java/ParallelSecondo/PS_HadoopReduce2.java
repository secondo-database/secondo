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
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

import sj.lang.ListExpr;

public class PS_HadoopReduce2 implements Constant{

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
	 * FilePath
	 * InputObjectName_1
	 * duplicateTimes_1
	 * PartAttributeName_1
	 * InputObjectName_2
	 * duplicateTimes_2
	 * PartAttributeName_2
	 * ReduceTaskNum
	 * FListKind
	 * [ MapQuery(2) ]
	 * 
	 */

	public static void main(String[] args) {
		
		final int paraLength = 17;
		String usage = "Usage HadoopReduce2 " +
				"<databaseName>  <CreateObjectName> <CreateQuery> " +
				"<DLF_Name_List> <DLF_fileLoc_List> " +
				"<DLO_Name_List> <DLO_loc_List> " +
				"<FilePath> "+
				"<InputObjectName_1> <duplicateTimes_1> <PartAttributeName_1> " +
				"<InputObjectName_2> <duplicateTimes_2> <PartAttributeName_2> " +
				"<reduceTasksNum> <FListKind> [ MapQuery(2) ]"; 

		if (args.length != paraLength)
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
		String CreateFilePath		= args[7];
		
		String[] inObjName 		= {args[8], args[11]};
		int[] duplicateTimes 	= { Integer.parseInt(args[9]), 
															Integer.parseInt(args[12])};
		String[] PAName 			= {args[10], args[13]};
		int reduceTasksNum 		= Integer.parseInt(args[14]);
		FListKind outputKind 	= FListKind.values()[Integer.parseInt(args[15])];
		String UEMapQueryStrs	= args[16];
		int mapTasksNum = slaves.size();

		if (outputKind == FListKind.DLO && reduceTasksNum > slaves.size()){
			System.err.println("Warning! It is not allowed to produce DLO flist " +
					"with columns more than slave data servers.");
			System.err.println("Change the reduce tasks number from " + 
					reduceTasksNum + " to " + slaves.size() + " compulsively");
			reduceTasksNum = slaves.size();
		}
		
		ListExpr fpList = ListExpr.oneElemList(ListExpr.textAtom(CreateFilePath));
		CreateFilePath = fpList.toString().replace('\n', ' ');  //In case empty path

		ListExpr UEMapQuery = new ListExpr();
		UEMapQuery.readFromString(UEMapQueryStrs);
		ListExpr[] UEMapQueryLists = new ListExpr[2];
		UEMapQueryLists[0] =UEMapQuery.first(); 
		UEMapQueryLists[1] =UEMapQuery.second();
		boolean runMapper[] = {!UEMapQueryLists[0].isEmpty(), !UEMapQueryLists[1].isEmpty()};
//		boolean runMapper = ! (UEMapQueryLists[0].isEmpty() && UEMapQueryLists[1].isEmpty());
		ListExpr[] AllMapDLFList = new ListExpr[2]; 
		ListExpr[] AllMapDLOList = new ListExpr[2];
		int[] mapTaskNums = new int[2];
		String[] MapQueryStr = {"", ""};
		String Map_DLF_Name_ListStr[] = {"", ""};
		
//------------------------------------------------------------------------------------------
		
		//Prepare two sections of parameters for the mappers.
		for (int i = 0; i < 2; i++)
		{
			if (runMapper[i])
			{
				MapQueryStr[i] = HPA_AuxFunctions.plainStr(UEMapQueryLists[i].first()); 						
				//Map map query
				ListExpr mamfnList = UEMapQueryLists[i].second().first();		//Map DLF name list
				Map_DLF_Name_ListStr[i] = HPA_AuxFunctions.plainStr(mamfnList);
				
				ListExpr mamflList = UEMapQueryLists[i].second().second(); 	//Map DLF location list
				ListExpr mamonList = UEMapQueryLists[i].third().first(); 		//Map DLO name list
				ListExpr mamolList = UEMapQueryLists[i].third().second();		//Map DLO location list
				mapTaskNums[i] = UEMapQueryLists[i].fourth().intValue();
				
				AllMapDLFList[i] = HPA_AuxFunctions.flist2Mapper(mamfnList, mamflList, mapTaskNums[i]);
				AllMapDLOList[i] = HPA_AuxFunctions.flist2Mapper(mamonList, mamolList, mapTaskNums[i]);
			}
			else
			{
				AllMapDLFList[i] = ListExpr.theEmptyList();
				AllMapDLOList[i] = ListExpr.theEmptyList();
			}
		}

		//Apply each map task with one row of the fileLoc list, for the executed flists. 
		ListExpr allDLFNameList = new ListExpr(), allDLFLocList = new ListExpr();
		allDLFNameList.readFromString(DLF_Name_ListStr);
		allDLFLocList.readFromString(DLF_Loc_ListStr);
		ListExpr DLFByMappers = HPA_AuxFunctions.flist2Mapper(allDLFNameList, allDLFLocList, mapTasksNum);
		DLFByMappers = HPA_AuxFunctions.divMRDLO(allDLFNameList, DLFByMappers, mapTasksNum);

		ListExpr allDLONameList = new ListExpr(), allDLOLocList = new ListExpr();
		allDLONameList.readFromString(DLO_Name_ListStr);
		allDLOLocList.readFromString(DLO_Loc_ListStr);
		ListExpr DLOByMappers = HPA_AuxFunctions.flist2Mapper(allDLONameList, allDLOLocList, mapTasksNum);
		DLOByMappers = HPA_AuxFunctions.divMRDLO(allDLONameList, DLOByMappers, mapTasksNum);
//------------------------------------------------------------------------------------------
		
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
			
			ListExpr[] amapDLF_rest = new ListExpr[2];
			ListExpr[] amapDLO_rest = new ListExpr[2];
			
			
			if (runMapper[0])
			{
				amapDLF_rest[0] = AllMapDLFList[0];
				amapDLO_rest[0] = AllMapDLOList[0];
			}
			
			if (runMapper[1])
			{
				amapDLF_rest[1] = AllMapDLFList[1];
				amapDLO_rest[1] = AllMapDLOList[1];
			}

			for (int mapperIdx = 0; mapperIdx < mapTasksNum; mapperIdx++)
			{
				boolean allDLOexist = true, allDLFexist = true;
				ListExpr aomLoc, afmLoc, aomName, afmName;
				aomLoc 	= ListExpr.theEmptyList();
				afmLoc 	= ListExpr.theEmptyList(); 
				aomName = ListExpr.theEmptyList();
				afmName = ListExpr.theEmptyList();
				if (!aoml_rest.isEmpty()) aomLoc = aoml_rest.first();
				if (!afml_rest.isEmpty()) afmLoc = afml_rest.first();
				if (!aomn_rest.isEmpty()) aomName = aomn_rest.first();
				if (!afmn_rest.isEmpty()) afmName = afmn_rest.first();
				ListExpr[] amap_DLF = {ListExpr.theEmptyList(),ListExpr.theEmptyList()}, 
									 amap_DLO = {ListExpr.theEmptyList(),ListExpr.theEmptyList()};

				if (!afmName.isEmpty())
					allDLFexist = HPA_AuxFunctions.allObjectExist(afmName, afmLoc);
				if (!aomName.isEmpty())
					allDLOexist = HPA_AuxFunctions.allObjectExist(aomName, aomLoc);
				boolean[] mapDLFExist = {false, false};
				boolean[] mapDLOExist = {false, false};
				
				for (int i=0; i<2; i++)
				{
					if (runMapper[i])
					{
						if (!amapDLF_rest[i].isEmpty()) amap_DLF[i] = amapDLF_rest[i].first();
						if (!amapDLO_rest[i].isEmpty()) amap_DLO[i] = amapDLO_rest[i].first();
						
						if (!amap_DLF[i].isEmpty())
							mapDLFExist[i] = HPA_AuxFunctions.allMapperFOExist(amap_DLF[i]);
						if (!amap_DLO[i].isEmpty())
							mapDLOExist[i] = HPA_AuxFunctions.allMapperFOExist(amap_DLO[i]);
					}
				}

				if (!allDLFexist && (runMapper[0] || runMapper[1])){
					allDLFexist = mapDLFExist[0] || mapDLFExist[1]; 
				}
				if (!allDLOexist && (runMapper[0] || runMapper[1])){
					allDLOexist = mapDLOExist[0] || mapDLOExist[1]; 
				}

				if (allDLOexist || allDLFexist)
				{
					
					//For mappers
					String fmnstr = HPA_AuxFunctions.plainStr(afmName);
					String fmlstr = HPA_AuxFunctions.plainStr(afmLoc);
					String omnstr = HPA_AuxFunctions.plainStr(aomName);
					String omlstr = HPA_AuxFunctions.plainStr(aomLoc);
				
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
					
					if (runMapper[0] || runMapper[1])
					{
						String[] dlfLocStr = new String[2];
						dlfLocStr [0] = HPA_AuxFunctions.plainStr(amap_DLF[0]);
						dlfLocStr [1] = HPA_AuxFunctions.plainStr(amap_DLF[1]);
						int[] slaveIdx = {0, 0}; 
						slaveIdx[0] = HPA_AuxFunctions.findFirstSlave(amap_DLF[0]);
						slaveIdx[1] = HPA_AuxFunctions.findFirstSlave(amap_DLF[1]);
						if (slaveIdx[0] == 0)
							slaveIdx[0] = HPA_AuxFunctions.findFirstSlave(amap_DLO[0]);
						if (slaveIdx[1] == 0)
							slaveIdx[1] = HPA_AuxFunctions.findFirstSlave(amap_DLO[1]);

						out.print("" +
								mapperIdx 							+ inDim +//0
								databaseName 						+ inDim +//1
								CreateObjectName 				+ inDim +//2
								CreateQuery 						+ inDim +//3
								CreateFilePath					+ inDim +//4
								outputKind.ordinal()  	+ inDim +//5
								fmnstr									+ inDim +//6
								fmlstr									+ inDim +//7
								omnstr									+ inDim +//8
								omlstr									+ inDim +//9
								frnstr									+ inDim +//10
								frlstr									+ inDim +//11
								//-------------------------------
								inObjName[0]						+ inDim +//12
								slaveIdx[0]							+ inDim +//13
								duplicateTimes[0]				+ inDim +//14
								PAName[0]								+ inDim +//15
								MapQueryStr[0]					+ inDim +//16
								Map_DLF_Name_ListStr[0] + inDim +//17
								dlfLocStr [0]						+ inDim +//18
								//-------------------------------
								inObjName[1]						+ inDim +//19
								slaveIdx[1]							+ inDim +//20
								duplicateTimes[1]				+ inDim +//21
								PAName[1]								+ inDim +//22	
								MapQueryStr[1]					+ inDim +//23
								Map_DLF_Name_ListStr[1] + inDim +//24
								dlfLocStr [1]						+ inDim +//25
						"");
						
						out.close();
					}
					else
					{
						
						out.print( "" + 
								mapperIdx 						+ inDim +//0
								databaseName 					+ inDim +//1
								CreateObjectName 			+ inDim +//2
								CreateQuery 					+ inDim +//3
								fmnstr								+ inDim +//4
								fmlstr								+ inDim +//5
								omnstr								+ inDim +//6 
								omlstr								+ inDim +//7
								frnstr								+ inDim +//8
								frlstr								+ inDim +//9
								CreateFilePath				+ inDim +//10
								outputKind.ordinal()  + inDim +//11
								inObjName[0]					+ inDim +//12
								duplicateTimes[0]			+ inDim +//13
								PAName[0]							+ inDim +//14
								inObjName[1]					+ inDim +//15
								duplicateTimes[1]			+ inDim +//16
								PAName[1]											 //17	
						);
						out.close();
					}
				}
				
				if (!aorn_rest.isEmpty()) aorn_rest = aorn_rest.rest();
				if (!aorl_rest.isEmpty()) aorl_rest = aorl_rest.rest();
				if (!afrn_rest.isEmpty()) afrn_rest = afrn_rest.rest();
				if (!afrl_rest.isEmpty()) afrl_rest = afrl_rest.rest();
				if (!aomn_rest.isEmpty()) aomn_rest = aomn_rest.rest();
				if (!aoml_rest.isEmpty()) aoml_rest = aoml_rest.rest();
				if (!afmn_rest.isEmpty()) afmn_rest = afmn_rest.rest();
				if (!afml_rest.isEmpty()) afml_rest = afml_rest.rest();

				if  (runMapper[0])
				{
					if (!amapDLO_rest[0].isEmpty()) amapDLO_rest[0] = amapDLO_rest[0].rest();
					if (!amapDLF_rest[0].isEmpty()) amapDLF_rest[0] = amapDLF_rest[0].rest();
				}
				
				if (runMapper[1])
				{
					if (!amapDLO_rest[1].isEmpty()) amapDLO_rest[1] = amapDLO_rest[1].rest();
					if (!amapDLF_rest[1].isEmpty()) amapDLF_rest[1] = amapDLF_rest[1].rest();
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
			if  (runMapper[0] || runMapper[1])
				job.setMapperClass(PS_HadoopReduce2_QMap.class);
			else
				job.setMapperClass(PS_HadoopReduce2_Map.class);
			job.setReducerClass(PS_HadoopReduce2_Reduce.class);
			
			
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
