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
	 * 
	 */

	public static void main(String[] args) {
		
		final int paraLength = 16;
		String usage = "Usage HadoopReduce2 " +
				"<databaseName>  <CreateObjectName> <CreateQuery> " +
				"<DLF_Name_List> <DLF_fileLoc_List> " +
				"<DLO_Name_List> <DLO_loc_List> " +
				"<FilePath> "+
				"<InputObjectName_1> <duplicateTimes_1> <PartAttributeName_1> " +
				"<InputObjectName_2> <duplicateTimes_2> <PartAttributeName_2> " +
				"<reduceTasksNum> <FListKind>"; 

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
		int reduceTasksNum 			= Integer.parseInt(args[14]);
		FListKind outputKind 		= FListKind.values()[Integer.parseInt(args[15])];
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


		//Apply each map task with one row of the fileLoc list.
		
		ListExpr allDLFLocList = new ListExpr();
		allDLFLocList.readFromString(DLF_Loc_ListStr);
		ListExpr allDLFNameList = new ListExpr();
		allDLFNameList.readFromString(DLF_Name_ListStr);
		ListExpr DLFByMappers = HPA_AuxFunctions.flist2Mapper(allDLFNameList, allDLFLocList, mapTasksNum);
		DLFByMappers = HPA_AuxFunctions.divMRDLO(allDLFNameList, DLFByMappers, mapTasksNum);
		
		ListExpr allDLOLocList = new ListExpr();
		allDLOLocList.readFromString(DLO_Loc_ListStr);
		ListExpr allDLONameList = new ListExpr();
		allDLONameList.readFromString(DLO_Name_ListStr);
		ListExpr DLOByMappers = HPA_AuxFunctions.flist2Mapper(allDLONameList, allDLOLocList, mapTasksNum);
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

			for (int slaveIdx = 0; slaveIdx < mapTasksNum; slaveIdx++)
			{
				ListExpr aomLoc, afmLoc, aomName, afmName;
				aomLoc = afmLoc = aomName = afmName = ListExpr.theEmptyList();
				if (!aoml_rest.isEmpty()) aomLoc = aoml_rest.first();
				if (!afml_rest.isEmpty()) afmLoc = afml_rest.first();
				if (!aomn_rest.isEmpty()) aomName = aomn_rest.first();
				if (!afmn_rest.isEmpty()) afmName = afmn_rest.first();
				
				boolean allDLOexist = true, allDLFexist = true;

				if (!afmName.isEmpty()){
					allDLFexist = HPA_AuxFunctions.allObjectExist(afmName, afmLoc);
				}
				
				if (!aomName.isEmpty()){
					allDLOexist = HPA_AuxFunctions.allObjectExist(aomName, aomLoc);
				}

				if (allDLOexist && allDLFexist)
				{
					
					//For mappers
					String fmnstr = HPA_AuxFunctions.plainStr(afmName);
					String fmlstr = HPA_AuxFunctions.plainStr(afmLoc);
				
					ListExpr aorLoc, afrLoc, aorName, afrName;
					aorLoc = afrLoc = aorName = afrName = ListExpr.theEmptyList();
					if (!aorl_rest.isEmpty()) aorLoc 	= aorl_rest.first();
					if (!afrl_rest.isEmpty()) afrLoc 	= afrl_rest.first();
					if (!aorn_rest.isEmpty()) aorName = aorn_rest.first();
					if (!afrn_rest.isEmpty()) afrName = afrn_rest.first();

					//For reducers
					String frlstr = HPA_AuxFunctions.plainStr(afrLoc);
					String frnstr = HPA_AuxFunctions.plainStr(afrName);
					
					String fileName = JOBID + "_INPUT_"+ slaveIdx + ".dat";
					PrintWriter out = new PrintWriter(
							FileSystem.get(conf).create(
									new Path(inputPath + "/" + fileName)));
					
					out.print( "" + 
							slaveIdx 							+ inDim +
							databaseName 					+ inDim +
							CreateObjectName 			+ inDim +
							CreateQuery 					+ inDim +
							fmnstr								+ inDim +
							fmlstr								+ inDim +
							frnstr								+ inDim +
							frlstr								+ inDim +
							CreateFilePath				+ inDim +
							outputKind.ordinal()  + inDim +
							inObjName[0]					+ inDim +
							duplicateTimes[0]			+ inDim +
							PAName[0]							+ inDim +
							inObjName[1]					+ inDim +
							duplicateTimes[1]			+ inDim +
							PAName[1]							
					);
					out.close();
					
				}
				
				if (!aomn_rest.isEmpty()) aomn_rest = aomn_rest.rest();
				if (!aoml_rest.isEmpty()) aoml_rest = aoml_rest.rest();
				if (!aorn_rest.isEmpty()) aorn_rest = aorn_rest.rest();
				if (!aorl_rest.isEmpty()) aorl_rest = aorl_rest.rest();

				if (!afmn_rest.isEmpty()) afmn_rest = afmn_rest.rest();
				if (!afml_rest.isEmpty()) afml_rest = afml_rest.rest();
				if (!afrn_rest.isEmpty()) afrn_rest = afrn_rest.rest();
				if (!afrl_rest.isEmpty()) afrl_rest = afrl_rest.rest();

				
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
