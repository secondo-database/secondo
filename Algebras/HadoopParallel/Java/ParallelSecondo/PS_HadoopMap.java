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
import org.apache.hadoop.io.BooleanWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

import sj.lang.ListExpr;

public class PS_HadoopMap implements Constant{
  
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
	 * duplicateTimes
	 * FListKind
	 * FilePath
	 */
	public static void main(String[] args) {

/*
12.Feb 2012

Create DLO kind flist at each slave database. 
The number of map tasks is as large as the number of slaves, 
each one runs only one map task, creates the local Secondo object 
according to the input CreateQuery,
and returns a boolean to indicate whether the creation is success.  


21.Mar 2012

Add DLF_Type and FilePath parameters, in case creating DLF result

DLF_Type: 

  * 0 : Create DLO flist,

  * 1 : DLF, export type is stream(tuple(T)), add fconsume at last
 
  * 2 : DLF, export type is stream(T), T in DATA, add transformstream fconsume at last
 
  * 3 : DLF, export type is T, T in DATA, add feed transformstream fconsume at last.


12.Apr. 2012
Use FListKind enum in Constant class to replace the DLF_Type argument.

*/
		final int paraLength = 8;
		String usage = "Usage Spread2 <databaseName> " +
				"<CreateObjectName> <CreateQuery> " +
				"<DLF_Name_List> <DLF_fileLoc_List> " +
				"<duplicateTimes> <FListKind> <FilePath>"; 
		
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
		String DLF_Name_List	 	= args[3];
		String DLF_fileLoc_List	= args[4];
		int duplicateTimes 			= Integer.parseInt(args[5]);
		FListKind outputKind 		= FListKind.values()[Integer.parseInt(args[6])];
		String CreateFilePath	= args[7];
		int MapTasksNum = slaves.size();
		
		ListExpr fpList = ListExpr.oneElemList(ListExpr.textAtom(CreateFilePath));
		CreateFilePath = fpList.toString().replace('\n', ' ');  //In case empty path
		
		
		//Apply each map task with one row of the fileLoc list.
		ListExpr[] mapFileLoc = new ListExpr[MapTasksNum];       //Head of each list
		ListExpr[] mapFileLoc_last = new ListExpr[MapTasksNum];  //Tail of each list
		ListExpr allLocLists = new ListExpr();
		allLocLists.readFromString(DLF_fileLoc_List);
		ListExpr rest = allLocLists;

		while (!rest.isEmpty())
		{
			ListExpr aFileMatrix = rest.first();

			int rowNumber = 1;
			ListExpr[] aFileLoc = new ListExpr[MapTasksNum];
			ListExpr[] aFileLoc_last = new ListExpr[MapTasksNum];
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
						aFileLoc_last[flCounter] = ListExpr.append(aFileLoc_last[flCounter],
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
			for (int i = 0; i < MapTasksNum; i++)
			{
				if (aFileLoc[i] == null){
					aFileLoc[i] = ListExpr.theEmptyList();
				}
				
				if (mapFileLoc[i] == null)
				{
					mapFileLoc[i] = ListExpr.oneElemList(aFileLoc[i]);
					mapFileLoc_last[i] = mapFileLoc[i];
				}
				else
				{
					mapFileLoc_last[i] = ListExpr.append(mapFileLoc_last[i], aFileLoc[i]);
				}
			}
			
			rest = rest.rest();
		}
		
		//Prepare the input for mappers
    String inputPath = "INPUT";
    String outputPath = "OUTPUT";
    Configuration conf = new Configuration();
		try
		{
			FileSystem.get(conf).delete(new Path(outputPath), true);
			FileSystem.get(conf).delete(new Path(inputPath), true);
			for (int slaveIdx = 0; slaveIdx < MapTasksNum; slaveIdx++)
			{
				if (mapFileLoc[slaveIdx] == null)
					mapFileLoc[slaveIdx] = ListExpr.theEmptyList();
				
				String locStr = mapFileLoc[slaveIdx].toString().
					replaceAll("\n", " ").replaceAll("\t", " ").
					replaceAll(" +", " ");

				String fileName = JOBID + "_INPUT_"+ slaveIdx + ".dat";
				PrintWriter out = new PrintWriter(
						FileSystem.get(conf).create(
									new Path(inputPath + "/" + fileName)));
			
				out.print( "" + 
						slaveIdx 							+ inDim +
						databaseName 					+ inDim + 
						CreateObjectName 			+ inDim +
						CreateQuery 					+ inDim + 
						DLF_Name_List 				+ inDim +
						locStr 								+ inDim +
						duplicateTimes				+ inDim +
						CreateFilePath				+ inDim + 
						outputKind.ordinal()  
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
			job.setMapperClass(PS_HadoopMap_Map.class);
			job.setReducerClass(PS_HadoopMap_Reduce.class);
			
			job.setMapOutputKeyClass(IntWritable.class);
			job.setMapOutputValueClass(BooleanWritable.class);
			job.setOutputKeyClass(IntWritable.class);
			job.setOutputValueClass(Text.class);  

			job.setJobName(JOBID);
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
