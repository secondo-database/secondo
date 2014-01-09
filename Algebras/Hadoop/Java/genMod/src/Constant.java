

public interface Constant {

	public final static String exDim = "/";    //Delimiter for external parameters to the Hadoop job
	public final static String inDim = "#SEP#";//Delimiter for internal parameters of the Hadoop job
	public final static String sysDim = ":";   //Delimiter for systematic parameters
	
	public final static String metaDB = "hadoopmeta";
	public final static String metaRel = "psMetaJobs";
	
	public final static String relDim = "_";
	public final static String fsName = "xxxAttrFSN";  //file suffix name
	
	public final static String frsName = "XxxAttrFRS";  //file row suffix name
	public final static String fssName = "XxxAttrFSS";  //file slave suffix name
	
	
	public final static String mapGenFileGPath = "SCRIPTS/BerlinMOD_Generator_map.sec";  //BerlinMOD Generator script file name for map step
	public final static String reduceGenFileGPath = "SCRIPTS/BerlinMOD_Generator_reduce.sec";  //BerlinMOD Generator script file name for reduce step
	public final static String partRectOBJName = "PART_WORLD_BBOX_rect3";
	public final static String GlobalRectOBJName = "STAT_WOLRD_BBOX_rect3";
	public final static String partRectFileName = "PARTREC"; //Bounding box name for each slave
	public final static String partRectAttr = "PartRect"; //Bounding box name for each slave
	public final static String allVehicleNumOBJName = "P_NUMALLCARS"; //Bounding box name for each slave
	
	
	
/*
The fsName is used to indicate cell files' suffices, 
while using loopsel+ffeed operation. 
  
*/

/*
During the processor, we make up some query nested-lists by own, 
not by the parser. 
To indicate different parameters, we define following parameter 
data types.    
*/
	public final static String fptTuple = "xxxTP";
	
	public final static String secMAName = "Machines";
	//The array objects in Secondo that contains all nodes' names. 
}

class RemoteStreamException extends RuntimeException{
	  private static final long serialVersionUID = -222798966184550543L;

	  RemoteStreamException(String message){
	    super(message);
	  }
}