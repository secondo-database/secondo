package ParallelSecondo;

import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

import sj.lang.ListExpr;

public class PS_HadoopReduce2_HDJ_Reduce 
extends	Reducer<IntWritable, BytesWritable, IntWritable, Text>
//extends	Reducer<IntWritable, Text, IntWritable, Text>
implements Constant
{

	int sendPort;
	SecExRunnable receiver;
	Thread receiverThread;
	RemoteStream sender;
	
	int recvTupleCount = 0;
	int sendSockBlockCount = 0;
	int tTpCount = 0;	//total tuple counter
	int zTpCount = 0;	//0Tuple counter

	String reducerIPAddr = "", reducerLoc = "";
	int 	 reducerPort = -1, reducerSN = -1;
  int hostSN = -1;     //Get the host's serial inside the slave list.
  int slaveListLength = 0;

  //Query parameters
  String dbName = null;
  String resultName = null, createPath = null;
  String jobID = null;
  String rqStr = null;
  int rcvByteCount = 0;
  long sT = 0, eT = 0;
  int resultRowNo;
  
  private byte[] zTuple = new byte[RemoteStream.SOCKTUP_SIZE];

	@Override
	protected void setup(Context context)
	throws IOException, InterruptedException {
		PSNode reducer 	= PSNode.SelectDataServer(-1);
		reducerIPAddr  	= reducer.getIpAddr();
		reducerLoc 			= reducer.getFileLoc();
		reducerPort			= reducer.getPortNum();
		reducerSN				= reducer.getSn();
		
		
		resultRowNo = context.getTaskAttemptID().getTaskID().getId() + 1;
		
		jobID = context.getJobName();
		System.out.println("SETUP[1]: ");
		//Get the reduce query
		PSNode master = PSNode.getMasterNode();
		String masterIP = master.getIpAddr();
		int masterPort  = master.getPortNum();
		QuerySecondo mSecEntity = new QuerySecondo();
		mSecEntity.open(masterIP, metaDBName, masterPort);
		ListExpr resultList = new ListExpr();
		String queryMeta = "query " + metaRelName + " feed " +
			"filter[.JobID = \""+ jobID + "\"] " + 
			"project[DBName, ResultName, CreateFilePath, RQ] tconsume";
		mSecEntity.query(queryMeta, resultList);
		dbName 			= resultList.second().first().first().stringValue();
		resultName 	= resultList.second().first().second().stringValue();
		createPath	= resultList.second().first().third().textValue();
		rqStr				= resultList.second().first().fourth().textValue();
		mSecEntity.close();
		
		System.out.println("SETUP[2]: ");
		//Get the zTuple
		Path zTuplePath = new Path(hadoopAuxiliaryPath + "/" + zTupleFileName);
		Configuration conf = new Configuration();
		FileSystem fs = FileSystem.get(conf);
		long zTupleSize = fs.getFileStatus(zTuplePath).getLen();
		FSDataInputStream fin = fs.open(zTuplePath);
		fin.read(zTuple, 0, (int) zTupleSize);
		fin.close();
		
		sendPort = HPA_AuxFunctions.getPort();
		
		ListExpr reduceQuery = new ListExpr();
		reduceQuery.readFromString(rqStr);
		ListExpr recieveStream = ListExpr.threeElemList(
				ListExpr.symbolAtom("project"), 
				ListExpr.threeElemList(
						ListExpr.symbolAtom("receive"), 
						ListExpr.stringAtom(reducerIPAddr), 
						ListExpr.intAtom(sendPort)), 
				ListExpr.oneElemList(ListExpr.symbolAtom("ValueT")));
		reduceQuery = ExtListExpr.replaceFirst(reduceQuery, QUERYNLSTR, recieveStream);
		reduceQuery = ListExpr.twoElemList(ListExpr.symbolAtom("query"), 
				ListExpr.fiveElemList(
						ListExpr.symbolAtom("fconsume"), 
						reduceQuery, 
						ListExpr.fourElemList(
								ListExpr.stringAtom(resultName), 
								ListExpr.textAtom(createPath), 
								ListExpr.intAtom(resultRowNo), 
								ListExpr.intAtom(1)), 
						ListExpr.theEmptyList(), 
						ListExpr.theEmptyList()));
		
		String[] queries = {reduceQuery.toString()};
		
		receiver = new SecExRunnable(reducerIPAddr, dbName, reducerPort, queries);
		if (!receiver.isInitialized())
			throw new RemoteStreamException("Error! Exception in setting up Secondo receiver.");
		receiverThread = new Thread(receiver);
		receiverThread.start();
		
    // Send the streamType to start the receive operator in Secondo.
    sender = new RemoteStream("server", reducerIPAddr, sendPort);
    sender.Connect();
    if (sender.getConnected()){
      sender.writeLine("(stream (tuple ((KeyT string)(ValueT text))))");
      String rtnInfo = sender.readLine();
      System.out.println("Get return string: --------  " + rtnInfo);
      if (!rtnInfo.equals("<GET TYPE/>"))
        throw new IOException("Error: Sender can't get return info.");
    }else{
      throw new IOException("Error: Sender can't listen to port:" + sendPort);
    }
	}

	
	@Override
	protected void reduce(IntWritable key, Iterable<BytesWritable> values,
			Context context) throws IOException,
			InterruptedException {
		
    byte[] tupleBlock = new byte[RemoteStream.MAX_TUPLESIZE];
    int offset = 0;
    int maxCap = Math.round(
        RemoteStream.MAX_TUPLESIZE / RemoteStream.SOCKTUP_SIZE)
        * RemoteStream.SOCKTUP_SIZE;
    int tbSize = maxCap;
    int pCnt = 0;
    for (BytesWritable value : values) {
      //Collect value into one TUPLE BUFFER, When the buffer is overfilled, 
      //then send them once. 

      int tupleSize = value.getLength();
      rcvByteCount += tupleSize;
      if ( tupleSize >= tbSize )
      {
        //If no more space for the current tuple,
        //then send and clean the tupleBlock
        int sock_Sent_Num;
        
        sock_Sent_Num = sender.SendTupleBlock(tupleBlock, offset);
        sendSockBlockCount += sock_Sent_Num;
        tupleBlock = new byte[RemoteStream.MAX_TUPLESIZE];
        offset = 0;
        tbSize = maxCap;
      }
      
      //Put the tuple into the block
      System.arraycopy(value.getBytes(), 0, tupleBlock, offset, tupleSize);
      offset += tupleSize;
      tbSize -= tupleSize;
      tTpCount++;
      pCnt++;
    }
    if (offset > 0)
    {
      //Still exists some tuples inside the block
      int sock_Sent_Num = sender.SendTupleBlock(tupleBlock, offset);
      sendSockBlockCount += sock_Sent_Num;
    }
    
    // Send a specific 0-tuple to separate tuples within different key ranges
    sender.sendSocket(zTuple, 0, 1);
    tTpCount++;
    zTpCount++;
	}


	/* (non-Javadoc)
	 * @see org.apache.hadoop.mapreduce.Reducer#cleanup(org.apache.hadoop.mapreduce.Reducer.Context)
	 */
	@Override
	protected void cleanup(Context context)
			throws IOException, InterruptedException {
    
    System.out.println(context.getTaskAttemptID().toString() 
        + " send total: " + tTpCount + " tuples with " + zTpCount + " 0Tuples");
    
      sender.sendSocket(null, 0, 0);
      sender.close();
    
      //Wait for the end up of the thread.
      //while(!secReceiver.isFinished()){Thread.sleep(500);}
      receiverThread.join();
      
      context.write(new IntWritable(resultRowNo), new Text("1 " + (reducerSN + 1)));
      
      eT = System.currentTimeMillis();
      System.out.println("From start to end, total used: " + ((eT - sT) / 1000) + "s");
  
	}
	
	
	

}
