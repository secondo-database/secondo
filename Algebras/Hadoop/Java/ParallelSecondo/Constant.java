package ParallelSecondo;

public interface Constant {

	public final static String exDim = "/";    //Delimiter for external parameters to the Hadoop job
	public final static String inDim = "#SEP#";//Delimiter for internal parameters of the Hadoop job
	public final static String sysDim = ":";   //Delimiter for systematic parameters
	
	public final static String metaDB = "hadoopmeta";
	public final static String metaRel = "psMetaJobs";
	
	public final static String relDim = "_";
	public final static String fsName = "XxxAttrFSN";  //file suffix name
	
	public final static String frsName = "XxxAttrFRS";  //file row suffix name
	public final static String fcsName = "XxxAttrFCS";  //file column suffix name
	public final static String fssName = "XxxAttrFSS";  //file slave suffix name
	public final static String fptName = "XxxAttrFPT";  //file path name

	public final static String[] fspName = {"XxxAttrFSNP1","XxxAttrFSNP2"};  //file suffix name
	
	public final static String[] frspName = {"XxxAttrFRSP1","XxxAttrFRSP2"};  //file row suffix name
	public final static String[] fcspName = {"XxxAttrFCSP1","XxxAttrFCSP2"};  //file column suffix name
	public final static String[] fsspName = {"XxxAttrFSSP1","XxxAttrFSSP2"};  //file slave suffix name
	public final static String[] fptpName = {"XxxAttrFPTP1","XxxAttrFPTP2"};  //file path name
	
	public final static String eptStr = "<emptyString>";  //indicate a parameter when it's empty
	public final static String rdbStr = "<READ DB/>";     //tell the mapper to read data from database
	public final static String QUERYNLSTR = "QUERYNL";
	public final static String INDLFPattern = "<DLFMark:Arg[1-2]:[\\w,_]*/>";
	
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
	
	public enum FListKind {UNDEF, DLO, DLF}
}

class RemoteStreamException extends RuntimeException{
	  private static final long serialVersionUID = -222798966184550543L;

	  RemoteStreamException(String message){
	    super(message);
	  }
}