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

public class PS_HadoopMap2 implements Constant {

	private final static String JOBID = "HASEC_JOB_"
			+ new Timestamp(System.currentTimeMillis()).toString()
					.replace("-", "").replace(".", "").replace(" ", "")
					.replace(":", "");

	/**
	 * @param args
	 * 
	 *            DatabaseName CreateObjectName CreateQuery DLF_Name_List
	 *            DLF_fileLoc_List DLO_Name_List DLO_loc_List duplicateTimes
	 *            FListKind FilePath mapTaskNum
	 */
	public static void main(String[] args) {

/*

*/
		final int paraLength = 11;
		String usage = "Usage PS_HadoopMap <databaseName> " +
				"<CreateObjectName> <CreateQuery> " +
				"<DLF_Name_List> <DLF_fileLoc_List> " +
				"<DLO_Name_List> <DLO_loc_List> " +
				"<duplicateTimes> <FListKind> <FilePath> <mapTaskNum>"; 
		
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
		String DLF_Name_List	 	= args[3];
		String DLF_fileLoc_List	= args[4];
		String DLO_Name_List	 	= args[5];
		String DLO_loc_List			= args[6];
		int duplicateTimes 			= Integer.parseInt(args[7]);
		FListKind outputKind 		= FListKind.values()[Integer.parseInt(args[8])];
		String CreateFilePath		= args[9];
		int MapTasksNum 				= Integer.parseInt(args[10]);
		
		ListExpr fpList = ListExpr.oneElemList(ListExpr.textAtom(CreateFilePath));
		CreateFilePath = fpList.toString().replace('\n', ' ');  //In case empty path
		
		
		ListExpr allDLFNameLists = new ListExpr();
		allDLFNameLists.readFromString(DLF_Name_List);
		ListExpr allDLFLocLists = new ListExpr();
		allDLFLocLists.readFromString(DLF_fileLoc_List);
		ListExpr allDLFLists = HPA_AuxFunctions.flist2Mapper2(allDLFNameLists, allDLFLocLists, MapTasksNum, slaves.size());

		ListExpr allDLONameLists = new ListExpr();
		allDLONameLists.readFromString(DLO_Name_List);
		ListExpr allDLOLocLists = new ListExpr();
		allDLOLocLists.readFromString(DLO_loc_List);
		ListExpr allDLOLists = HPA_AuxFunctions.flist2Mapper2(allDLONameLists, allDLOLocLists, MapTasksNum, slaves.size());
		
		System.out.println( HPA_AuxFunctions.plainStr(allDLFLists));
		
		//System.exit(-1);
		
		/*
		 * allDLFLists example: 
		 * 
		 ( ( ( (1 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () () () ()) ( (1 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () () () ())) 
   ( (() (2 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () () ()) (() (2 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () () ())) 
   ( (() () (3 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () ()) (() () (3 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () ())) 
   ( (() () () (4 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) ()) (() () () (4 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) ())) 
   ( (() () () () (5 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->)) (() () () () (5 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->)))
   )
		 * 
		 * 
		 * 
		 */
		//Prepare the input for mappers
    String inputPath = "INPUT";
    String outputPath = "OUTPUT";
    Configuration conf = new Configuration();
		try
		{
			FileSystem.get(conf).delete(new Path(outputPath), true);
			FileSystem.get(conf).delete(new Path(inputPath), true);
			ListExpr aMapperDLO_Rest = allDLOLists;
			ListExpr aMapperDLF_Rest = allDLFLists;//traverse the nestedlist

			for (int mapperIdx = 1; mapperIdx <= MapTasksNum; mapperIdx++)
			{
				ListExpr amDLO = aMapperDLO_Rest.first();
				//example:( ( ( (1 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () () () ()) ( (1 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () () () ())) 
				  
				ListExpr amDLF = aMapperDLF_Rest.first();
	
				//allMapperFOExist check if two objects exist.
				boolean allDLOexist = HPA_AuxFunctions.allMapperFOExist(amDLO.first()) && HPA_AuxFunctions.allMapperFOExist(amDLO.second());
				boolean allDLFexist = HPA_AuxFunctions.allMapperFOExist(amDLF.first()) && HPA_AuxFunctions.allMapperFOExist(amDLF.second());

				if (allDLFexist && allDLOexist)
				{
					String dlfLocStr = HPA_AuxFunctions.plainStr(amDLF);
					String dloLocStr = HPA_AuxFunctions.plainStr(amDLO);
					int slaveIdx = HPA_AuxFunctions.findFirstSlave(amDLF.first());
					if (slaveIdx == 0){
						slaveIdx = HPA_AuxFunctions.findFirstSlave(amDLO.first());
					}
					
					
					String fileName = JOBID + "_INPUT_"+ mapperIdx + ".dat";
					PrintWriter out = new PrintWriter(
							FileSystem.get(conf).create(
									new Path(inputPath + "/" + fileName)));
					
					out.print( "" +
                            slaveIdx                                                        + inDim +
                            mapperIdx                                                       + inDim +
                            databaseName                                    + inDim +
                            CreateObjectName                        + inDim +
                            CreateQuery                                     + inDim +
                            DLF_Name_List                           + inDim +
                            dlfLocStr                                               + inDim +
                            DLO_Name_List               + inDim +
                            dloLocStr                   + inDim +
                            duplicateTimes                          + inDim +
                            CreateFilePath                          + inDim +
                            outputKind.ordinal()

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
		System.exit(-1);
		//Create the job
		try {
			Job job = new Job();
			job.setJarByClass(PS_HadoopMap.class);
			
			FileInputFormat.addInputPath(job, new Path(inputPath));
			FileOutputFormat.setOutputPath(job, new Path(outputPath));
			job.setMapperClass(PS_HadoopMap_Map.class);
			job.setReducerClass(PS_HadoopMap_Reduce.class);
			
			job.setMapOutputKeyClass(Text.class);
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
