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
		
		final int paraLength = 14;
		String usage = "Usage HadoopReduce2 " +
				"<databaseName>  <CreateObjectName> <CreateQuery> " +
				"<DLF_Name_List> <DLF_fileLoc_List> " +
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
		System.out.println("PARALLEL_SECONDO_SLAVES is: " + slFile);
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
		String CreateFilePath		= args[5];
		
		String[] inObjName 		= {args[6], args[9]};
		int[] duplicateTimes 	= { Integer.parseInt(args[7]), 
															Integer.parseInt(args[10])};
		String[] PAName 			= {args[8], args[11]};
		int reduceTasksNum 			= Integer.parseInt(args[12]);
		FListKind outputKind 		= FListKind.values()[Integer.parseInt(args[13])];
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
		ListExpr[] mapFileLoc = new ListExpr[mapTasksNum];       //Head of each list
		ListExpr[] mapFileLoc_last = new ListExpr[mapTasksNum];  //Tail of each list
		ListExpr mapFileNameList = new ListExpr(), 
						 mapFileNameList_last = null;
		ListExpr reduceFileNameList = new ListExpr(), 
						 reduceFileNameList_last = null;
		ListExpr reduceFileLocList = new ListExpr(), 
		         reduceFileLocList_last = null;
		
		ListExpr allLocList = new ListExpr();
		allLocList.readFromString(DLF_Loc_ListStr);
		ListExpr allNameList = new ListExpr();
		allNameList.readFromString(DLF_Name_ListStr);
		ListExpr locRest = allLocList, nameRest = allNameList;
		
		while (!locRest.isEmpty())
		{
			String dlfName = nameRest.first().stringValue();
			
			//Check if it is input parameter
			if (dlfName.matches(INDLFPattern))
			{
				//If the two input parameters are DLF flists, 
				//then distribute their locations to different mappers 
				if (mapFileNameList.isEmpty()){
					mapFileNameList = ListExpr.oneElemList(nameRest.first());
					mapFileNameList_last = mapFileNameList;
				}
				else{
					mapFileNameList_last = 
						ListExpr.append(mapFileNameList_last, nameRest.first());
				}
				
				ListExpr aFileMatrix = locRest.first();
				int rowNumber = 1;
				ListExpr[] aFileLoc = new ListExpr[mapTasksNum];
				ListExpr[] aFileLoc_last = new ListExpr[mapTasksNum];
				//Divide each file location to slaves
				while (!aFileMatrix.isEmpty())
				{
					ListExpr aFileRow = aFileMatrix.first();
					if (!aFileRow.isEmpty())
					{
						int flCounter = aFileRow.first().intValue() - 1;
						if ( aFileLoc[flCounter] == null){
							aFileLoc[flCounter] = ListExpr.oneElemList(
									ListExpr.threeElemList(
											ListExpr.intAtom(rowNumber),
											aFileRow.second(), 
											aFileRow.third()));
							aFileLoc_last[flCounter] = aFileLoc[flCounter];
						}
						else
						{
							aFileLoc_last[flCounter] =
								ListExpr.append(aFileLoc_last[flCounter],
									ListExpr.threeElemList(
											ListExpr.intAtom(rowNumber),
											aFileRow.second(), 
											aFileRow.third()));
							
						}
					}
					aFileMatrix= aFileMatrix.rest();
					rowNumber++;
				}
				
				//Merge locations for each slave
				for (int i = 0; i < mapTasksNum; i++)
				{
					if (aFileLoc[i] == null){
						aFileLoc[i] = ListExpr.theEmptyList();
					}
					if (mapFileLoc[i] == null)
					{
						mapFileLoc[i] = 
							ListExpr.oneElemList(aFileLoc[i]);
						mapFileLoc_last[i] = mapFileLoc[i];
					}
					else{
						mapFileLoc_last[i] = 
							ListExpr.append(mapFileLoc_last[i], aFileLoc[i]);
					}
				}
			}
			else
			{
//collect non-input DLF kind parameters in the lists prepared for reduce step.
				if (reduceFileNameList.isEmpty())
				{
					reduceFileNameList = ListExpr.oneElemList(nameRest.first());
					reduceFileLocList = ListExpr.oneElemList(locRest.first());
					reduceFileNameList_last = reduceFileNameList;
					reduceFileLocList_last = reduceFileLocList;
					
				}
				else
				{
					reduceFileNameList_last = 
						ListExpr.append(reduceFileNameList_last, nameRest.first());
					reduceFileLocList_last = 
						ListExpr.append(reduceFileLocList_last, locRest.first());
				}
			}
			
			locRest = locRest.rest();
			nameRest = nameRest.rest();
		}

		//Prepare the input for mappers
    String inputPath = "INPUT";
    String outputPath = "OUTPUT";
    Configuration conf = new Configuration();
		try
		{
			FileSystem.get(conf).delete(new Path(outputPath), true);
			FileSystem.get(conf).delete(new Path(inputPath), true);
			
			String reduceFileNameStr = reduceFileNameList.toString().
				replaceAll("\n", " ").replaceAll("\t", " ");
			String reduceFileLocStr = reduceFileLocList.toString().
				replaceAll("\n", " ").replaceAll("\t", " ");
			
			
			for (int slaveIdx = 0; slaveIdx < mapTasksNum; slaveIdx++)
			{
				if (mapFileLoc[slaveIdx] == null){
					mapFileLoc[slaveIdx] = ListExpr.theEmptyList();
				}
				
				String mapFileLocStr = mapFileLoc[slaveIdx].toString().
					replaceAll("\n", " ").replaceAll("\t", " ");
				String mapFileNameStr = mapFileNameList.toString().
					replaceAll("\n", " ").replaceAll("\t", " ");
				String fileName = JOBID + "_INPUT_"+ slaveIdx + ".dat";
				PrintWriter out = new PrintWriter(
						FileSystem.get(conf).create(
									new Path(inputPath + "/" + fileName)));
			
				out.print( "" + 
						slaveIdx 							+ inDim +
						databaseName 					+ inDim +
						CreateObjectName 			+ inDim +
						CreateQuery 					+ inDim +
						mapFileNameStr 				+ inDim +
						mapFileLocStr 				+ inDim +
						reduceFileNameStr 		+ inDim +
						reduceFileLocStr 			+ inDim +
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
