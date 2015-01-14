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

		String databaseName         = args[0];
		String CreateObjectName     = args[1];
		String CreateQuery	 		= args[2];
		String DLF_Name_List	 	= args[3];
		String DLF_fileLoc_List	    = args[4];
		String DLO_Name_List	 	= args[5];
		String DLO_loc_List			= args[6];
		int duplicateTimes 			= Integer.parseInt(args[7]);
		FListKind outputKind 		= FListKind.values()[Integer.parseInt(args[8])];
		String CreateFilePath		= args[9];
		int MapTasksNum 			= Integer.parseInt(args[10]);
		
		ListExpr fpList = ListExpr.oneElemList(ListExpr.textAtom(CreateFilePath));
		CreateFilePath = fpList.toString().replace('\n', ' ');  //In case empty path
		
		
		ListExpr allDLFNameLists = new ListExpr();
		allDLFNameLists.readFromString(DLF_Name_List);
		ListExpr allDLFLocLists = new ListExpr();
		allDLFLocLists.readFromString(DLF_fileLoc_List);
//		ListExpr allDLFLists = HPA_AuxFunctions.flist2Mapper2(allDLFNameLists, allDLFLocLists, MapTasksNum, slaves.size());
		ListExpr allDLFLists = HPA_AuxFunctions.flist2Mapper(allDLFNameLists, allDLFLocLists, MapTasksNum);

		ListExpr allDLONameLists = new ListExpr();
		allDLONameLists.readFromString(DLO_Name_List);
		ListExpr allDLOLocLists = new ListExpr();
		allDLOLocLists.readFromString(DLO_loc_List);
//		ListExpr allDLOLists = HPA_AuxFunctions.flist2Mapper2(allDLONameLists, allDLOLocLists, MapTasksNum, slaves.size());
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
			ListExpr aMapperDLF_Rest = allDLFLists;//traverse the nestedlist
			
			System.out.println(HPA_AuxFunctions.plainStr(allDLFLists) + "\n\n");

			for (int mapperIdx = 1; mapperIdx <= MapTasksNum; mapperIdx++)
			{
				
				ListExpr amDLO = ListExpr.theEmptyList();
				ListExpr amDLF = ListExpr.theEmptyList();
				
				if (!aMapperDLO_Rest.isEmpty()) amDLO = aMapperDLO_Rest.first(); 
				if (!aMapperDLF_Rest.isEmpty()) amDLF = aMapperDLF_Rest.first(); 
				//example:( ( ( (1 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () () () ()) ( (1 (1 2 3 4 5) <text>/opt/psec/cheng/PSFS</text--->) () () () ())) 
								
				boolean allDLOexist = true; 
				if (amDLO.listLength() == 2)
				  allDLOexist = bothMapperFOExist(amDLO.first(), amDLO.second());
				boolean allDLFexist = true; 
				if (amDLF.listLength() == 2)
				  allDLFexist = bothMapperFOExist(amDLF.first(), amDLF.second());
				
				if (allDLFexist && allDLOexist)
				{
					String dlfLocStr = HPA_AuxFunctions.plainStr(amDLF);
					String dloLocStr = HPA_AuxFunctions.plainStr(amDLO);
					int slaveIdx = HPA_AuxFunctions.findFirstSlave(amDLF);
					if (slaveIdx == 0){
						slaveIdx = HPA_AuxFunctions.findFirstSlave(amDLO);
					}
					
/*
It is certain that all used DLO sub-objects exist on the target mapper, 
hence it is not necessary to deliver their inforatmion any more.  

*/
					
					String fileName = JOBID + "_INPUT_"+ mapperIdx + ".dat";
					PrintWriter out = new PrintWriter(
							FileSystem.get(conf).create(
									new Path(inputPath + "/" + fileName)));
					
					out.print( "" +
                            slaveIdx                    + inDim +  //0
                            mapperIdx                   + inDim +  //1
                            databaseName                + inDim +  //2
                            CreateObjectName            + inDim +  //3
                            CreateQuery                 + inDim +  //4
                            DLF_Name_List               + inDim +  //5
                            dlfLocStr                   + inDim +  //6
                            duplicateTimes              + inDim +  //7
                            CreateFilePath              + inDim +  //8
                            outputKind.ordinal()                   //9

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
			job.setJarByClass(PS_HadoopMap.class);
			
			FileInputFormat.addInputPath(job, new Path(inputPath));
			FileOutputFormat.setOutputPath(job, new Path(outputPath));
			job.setMapperClass(PS_HadoopMap2_Map.class);
			job.setReducerClass(PS_HadoopMap2_Reduce.class);
			
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
	
	private static boolean bothMapperFOExist(ListExpr loc1, ListExpr loc2){
	  
	  //DLO and DLF can be exist separately
	  if (loc1.isEmpty() || loc2.isEmpty()) return true;
	  
	  if (loc1.listLength() != loc2.listLength()) return false;
	  
	  ListExpr obj1 = loc1, obj2 = loc2;
	  while (!obj1.isEmpty()){
		 ListExpr row1 = obj1.first(), row2 = obj2.first();
		 if (row1.isEmpty() ^ row2.isEmpty()) return false;
		 if (!row1.isEmpty() && !row2.isEmpty()){
		   if (row1.first().intValue() != row2.first().intValue()){
			   //rows are not stored on the same data Server
			   return false;
		   }
		 }
		 obj1 = obj1.rest();
		 obj2 = obj2.rest();
	  }
	  return true;
	}
	
}
