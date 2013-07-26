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

public class PS_HadoopMapAll implements Constant {

	
	private final static String JOBID = "HASEC_JOB_" +
  new Timestamp(System.currentTimeMillis()).toString()
  .replace("-", "").replace(".", "")
  .replace(" ", "").replace(":", "");

	/**
	 * 
	 * @param args
	 * 
	 * database name
	 * sub-query
	 * dlf_name_list
	 * dlf_loc_list
	 * dlo_name_list
	 * dlo_loc_list
	 * 
	 * 
	 * 
	 */
	
	
	public static void main(String[] args){
		
		
		final int paraLength = 6;
		String usage = "Usage PS_HadoopMapAll <databaseName> " +
				"<TaskQuery> " +
				"<DLF_Name_List> <DLF_fileLoc_List> " +
				"<DLO_Name_List> <DLO_loc_List> ";
		
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
		String CreateQuery	 		= args[1];
		String DLF_Name_List	 	= args[2];
		String DLF_fileLoc_List	= args[3];
		String DLO_Name_List	 	= args[4];
		String DLO_loc_List			= args[5];
		int MapTasksNum 				= slaves.size();
		
		ListExpr allDLFNameLists = new ListExpr();
		allDLFNameLists.readFromString(DLF_Name_List);
		ListExpr allDLFLocLists = new ListExpr();
		allDLFLocLists.readFromString(DLF_fileLoc_List);

		ListExpr allDLFLists = HPA_AuxFunctions.flist2Mapper(allDLFNameLists, allDLFLocLists, MapTasksNum);
		ListExpr allDLONameLists = new ListExpr();
		allDLONameLists.readFromString(DLO_Name_List);
		ListExpr allDLOLocLists = new ListExpr();
		allDLOLocLists.readFromString(DLO_loc_List);
		ListExpr allDLOLists = HPA_AuxFunctions.flist2Mapper(allDLONameLists, allDLOLocLists, MapTasksNum);

		//Prepare the input for mappers
    String inputPath = "INPUT";
    String outputPath = "OUTPUT";
    Configuration conf = new Configuration();
    try
    {
			FileSystem.get(conf).delete(new Path(outputPath), true);
			FileSystem.get(conf).delete(new Path(inputPath), true);
			ListExpr aMapperDLO_Rest = allDLOLists;
			ListExpr aMapperDLF_Rest = allDLFLists;

			for (int mapperIdx = 1; mapperIdx <= MapTasksNum; mapperIdx++)
			{
				ListExpr amDLO = aMapperDLO_Rest.first();
				ListExpr amDLF = aMapperDLF_Rest.first();

				boolean allDLOexist = HPA_AuxFunctions.allMapperFOExist(amDLO);
				boolean allDLFexist = HPA_AuxFunctions.allMapperFOExist(amDLF);

				if (allDLFexist && allDLOexist)
				{
					String dlfLocStr = HPA_AuxFunctions.plainStr(amDLF);
					int slaveIdx = HPA_AuxFunctions.findFirstSlave(amDLO);
					if (slaveIdx == 0){
						slaveIdx = mapperIdx;
					}
					
					String fileName = JOBID + "_INPUT_"+ mapperIdx + ".dat";
					PrintWriter out = new PrintWriter(
							FileSystem.get(conf).create(
									new Path(inputPath + "/" + fileName)));
					
					out.print( "" + 
							slaveIdx	 						+ inDim +
							mapperIdx	 						+ inDim +
							databaseName 					+ inDim + 
							CreateQuery 					+ inDim + 
							DLF_Name_List 				+ inDim +
							dlfLocStr 						+ inDim +
							""
					);
					
					out.close();
				}
				aMapperDLO_Rest = aMapperDLO_Rest.rest();
				aMapperDLF_Rest = aMapperDLF_Rest.rest();
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
			job.setJarByClass(PS_HadoopMapAll.class);
			
			FileInputFormat.addInputPath(job, new Path(inputPath));
			FileOutputFormat.setOutputPath(job, new Path(outputPath));
			job.setMapperClass(PS_HadoopMapAll_Map.class);
			job.setReducerClass(PS_HadoopMapAll_Reduce.class);
			
			job.setMapOutputKeyClass(Text.class);
			job.setMapOutputValueClass(Text.class);
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
