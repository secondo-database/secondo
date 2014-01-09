import java.io.File;
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
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

public class GenMOD implements Constant {

  private final static String JOBID = "GENMOD_JOB_" +
  new Timestamp(System.currentTimeMillis()).toString()
  .replace("-", "").replace(".", "")
  .replace(" ", "").replace(":", "");
  
  public static void main(String[] args)
  {
    if (args.length != 5){
      System.err.println("Usage: GenMOD <db_name> <scale_factor> <simulate_days> <mapGenScript> <reduceGenScript>");
      System.exit(-1);
    }
  
    String databaseName = args[0];
    double scaleFactor = Double.parseDouble(args[1]);
    int simulateDays = Integer.parseInt(args[2]);
    String mapGenFilePath = args[3];
    String reduceGenFilePath = args[4];
    int totalNumOfVehicles = (int) (2000 * Math.sqrt(scaleFactor));
    int sampleSize = 100;  //Default value from old script
    
    File mapGenFile = new File(mapGenFilePath);
    if (!mapGenFile.exists())
    {
    	System.err.println("Generator script for map step" + mapGenFilePath + " NOT exist");
    	System.exit(-1);
    }
    File reduceGenFile = new File(reduceGenFilePath);
    if (!reduceGenFile.exists())
    {
    	System.err.println("Generator script for map step" + reduceGenFilePath + " NOT exist");
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
    try{
    	Scanner scanner = new Scanner(new FileInputStream(slFile));
		int lineNum = 1;
		while (scanner.hasNextLine()){
			String[] line = scanner.nextLine().split(sysDim);
			slaves.add(new PSNode(lineNum++, line[0], line[1], 
					Integer.parseInt(line[2])));
		}
	} catch (FileNotFoundException e1) {
		// TODO Auto-generated catch block
		e1.printStackTrace();
	}
    int numOfNodes = slaves.size();
    
//=========== =========== =========== =========== =========== =========== ===========
    
    String inputPathStr = "INPUT-MOD";
    String outputPathStr = "OUTPUT-MOD";
    
    try {
      Configuration conf = new Configuration();
      FileSystem.get(conf).delete(new Path(inputPathStr), true);
      FileSystem.get(conf).delete(new Path(outputPathStr), true);
      FileSystem.get(conf).copyFromLocalFile(
    		  new Path(mapGenFilePath), 
    		  new Path(mapGenFileGPath));
      FileSystem.get(conf).copyFromLocalFile(
    		  new Path(reduceGenFilePath), 
    		  new Path(reduceGenFileGPath));

      // Create the input files,
      // each map task will get its own task parameter
      int factor = totalNumOfVehicles / numOfNodes;
      int sampleFactor = sampleSize / numOfNodes;
      int sLicence = 1, eLicence,
          sSample = 1, eSample,
          mapIdx = 1;
      for (PSNode slave : slaves) {
    	eLicence = sLicence + factor - 1;
    	if ( (totalNumOfVehicles - eLicence) < factor )
    		eLicence = totalNumOfVehicles;
    	eSample = sSample + sampleFactor - 1;
    	if ( (sampleSize - eSample) < sampleFactor)
    		eSample = sampleSize;

    	String fileNameStr = JOBID + "_" + sLicence + "_" + eLicence + ".data";

        //generate the random seeds
        PrintWriter out = new PrintWriter(FileSystem.get(conf)
            .create(new Path(inputPathStr + "/" + fileNameStr)));
        
        out.println(
        		slave.getIpAddr() 	+ inDim + 
        		slave.getPortNum() 	+ inDim +
        		databaseName 				+ inDim +
        		scaleFactor 				+ inDim +
        		sLicence 						+ inDim + 
        		eLicence 						+ inDim +
        		mapIdx 							+ inDim +
        		sSample 						+ inDim + 
        		eSample 						+ inDim +
        		totalNumOfVehicles 	+ inDim +
        		simulateDays 				+ inDim +
        		"");
        out.close();
        sLicence += factor;
        sSample += sampleFactor;
        mapIdx++;
      }
      
      Job job = new Job();
      job.setJarByClass(GenMOD.class);
      FileInputFormat.addInputPath(job, new Path(inputPathStr));
      FileOutputFormat.setOutputPath(job, new Path(outputPathStr));
      job.setMapperClass(MapRunScripts.class);
      job.setReducerClass(ReduceAggre.class);
      job.setNumReduceTasks(mapIdx - 1);
      job.setMapOutputKeyClass(IntWritable.class);
      job.setMapOutputValueClass(Text.class);
      job.setOutputKeyClass(Text.class);
      job.setOutputValueClass(Text.class);
      
      System.exit(job.waitForCompletion(true) ? 0 : 1);
    } catch (Exception e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
    
    
  }
}
