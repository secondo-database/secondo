package ParallelSecondo;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.EOFException;
import java.io.IOException;
import java.net.ConnectException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;

/*
The purpose of this program is created a java class, 
that can accept the binary tuple data sent from Secondo RemoteStreamAlgebra, 
which must contains a key attribute value in the front of each tuple.
The key attribute must be one of the four basic data type: 
int, bool, real or string. 

At the same time, this class also support the function to send 
the binary data back to Secondo 
after extracting the key attribute value out. 
 
 
*/

public class RemoteStream {
   
  public enum ROLE{
      SERVER, CLIENT
    };
    
//  public static int MAX_TUPLESIZE = 65535;
  public static int MAX_TUPLESIZE = 655359; //Big TUPLE-BUFFER
  private static int SOCKET_SIZE = 65536;
  private static int SOCKET_METASIZE = 8;
//  public static int SOCKTUP_SIZE = 1016;
  public static int SOCKTUP_SIZE = SOCKET_SIZE - SOCKET_METASIZE;

  public RemoteStream(String r, String hn, int pt)
  {
    if (r.toUpperCase().equals("SERVER"))
      role = ROLE.SERVER;
    else if (r.toUpperCase().equals("CLIENT"))
      role = ROLE.CLIENT;
    else
      System.err.println("Error role type: " + r.toUpperCase());
    
    if (hn.length() <= 0 || pt < 0)
      System.err.println("Error hostname or port");
    
    sock_ID = 0;
    hostName = hn;
    port = pt;
    endOfStream = false;
    loadTupleNum = 0;
    initialized = true;
    connected = false;

  }

  public boolean sendSocket(byte[] buf, int offset, int sock_Num)
    throws IOException{

    byte[] newBuf = new byte[SOCKET_SIZE];
    if (buf == null){
      System.arraycopy(Int2Byte(sock_ID), 0, newBuf, 0, 4);
      System.arraycopy(Int2Byte(-1), 0, newBuf, 4, 4);
      outSocketStream.write(newBuf, 0, SOCKET_SIZE);
      outSocketStream.flush();
      return true;
    }

    
    System.arraycopy(Int2Byte(sock_ID), 0, newBuf, 0, 4);
    System.arraycopy(Int2Byte(sock_Num), 0, newBuf, 4, 4);
    System.arraycopy(buf, offset, newBuf, 8, SOCKTUP_SIZE);
    outSocketStream.write(newBuf, 0, SOCKET_SIZE);
    outSocketStream.flush();

    int get_SockID = inSocketStream.readInt();
    if (get_SockID != (sock_ID + 1))
      throw new IOException("Error[sendSocket]: incorrect return sock_ID");
      
    sock_ID++;
    return true;
}
  
/*
Receive one socket, and copy the data into ~buf~ array. 
Return ~TRUE~ if there are successive sockets, 
or ~FALSE~ if it's the last socket of the current tuple buffer. 

*/
  public boolean receiveSocket(byte[] buf, int offset) throws IOException
  {
      //Read the sock_ID ... 
      int get_SockID = inSocketStream.readInt();

      if (get_SockID != sock_ID)
        throw new IOException("Error: receive error sock_ID " + get_SockID
        		+ "\t while expect " + sock_ID);
      
      //Read the sock_Num ...
      int sock_Num = inSocketStream.readInt();
//      System.err.println("remotestream get sock_id: " + get_SockID 
//      	  + " with sock_num: " + sock_Num);
      
//      System.err.println("to read socket from " + offset + " with " + SOCKTUP_SIZE);
      //Read the left bytes of Socket into ~buf~
      int readCount = 0;
      while (readCount < SOCKTUP_SIZE){
      	readCount += inSocketStream.read(buf, offset + readCount, SOCKTUP_SIZE - readCount);
      }
//      System.err.println("read result: " + readCount);

      sock_ID++;
      outSocketStream.writeInt(sock_ID);
      outSocketStream.flush();
      
      if (sock_Num > 1)
        return true;
      else
      {
        if (sock_Num < 0)
          endOfStream = true;   //No more sockets ... 
        return false;
      }
  }
  
  public int SendTupleBlock(byte[] buffer, int curPos) 
    throws IOException
  {
    int offset = 0;
    int sock_Num = (int)Math.ceil((double)curPos / SOCKTUP_SIZE);
    //System.out.println("*****Get sock_Num is: " + sock_Num);
    int sentSok_Num = 0;
    if (buffer == null)
      System.err.println("Error: buffer is NULL. ");  
    while(offset < curPos)
    {
      sendSocket(buffer, offset, sock_Num);
      sock_Num--;
      sentSok_Num++;
      offset += SOCKTUP_SIZE;
    }
    
    /*    sock_Num = (int)Math.ceil((double)curPos / SOCKTUP_SIZE);
    if (sock_Num != sentSok_Num)
      System.err.println("Should send: " + sock_Num 
          + "but sent: " + sentSok_Num);
    */
    return sentSok_Num;
  }
  
  public int LoadTuples(byte[] srcBuf, byte[] desBuf, int curPos) 
  throws Exception
  {
    int offset = 0;
    int desOffset = 0;
    while (offset < curPos) {

      int keySize =  Byte2Int(srcBuf, offset);
      if (keySize <= 0)
        break;
      offset += 4;
      
      String keyValue = "";
      if(keyType.equals("string"))
      {
        keyValue = Byte2String(srcBuf, offset, keySize);
      }
      else if (keyType.equals("int"))
      {
        keyValue = "" + Byte2Int(srcBuf, offset);
      }
      else if (keyType.equals("real"))
      {
        keyValue = "" + Byte2Real(srcBuf, offset);
      }
      else if (keyType.equals("bool"))
      {
        keyValue = "" + Byte2Bool(srcBuf, offset);
      }
      offset += keySize;
      
      short tupleSize = Byte2UnsignedShort(srcBuf, offset);
      tupleSize += 2; //The whole tuple size should contain the short length
      //copy the tuple data to ~desBuf~
      System.arraycopy(srcBuf, offset, desBuf, desOffset, tupleSize);
      offset += tupleSize;
      desOffset += tupleSize; 
      System.out.println("Read key[" + keyValue 
          + "] tupleSize[" + tupleSize + "]");
      
      loadTupleNum++;
    }

    return desOffset;
  }

  public void setKeyType(String _kt){
    keyType = _kt;
  }  
  
  public boolean getTheLastSocket(){
    return endOfStream;
  }
  
  public boolean getConnected(){
    return connected;
  }
  
  public void Connect() 
    throws IOException, InterruptedException{
    
    if (initialized){
      //try {
        connected = open(hostName, port);
      /*} catch (Exception e) {
        System.err.println("Error while accessing " + hostName 
            + " in port: " + port);
        e.printStackTrace();
        connected = false;
      }*/
    }else{
      throw new IOException("Uninitialized RemoteStream Object");
    }
  }
  
  private boolean open(String hostName, int pt)
    throws IOException, InterruptedException{

    if(ROLE.SERVER == role)
    {
      //START the server socket, and wait for accept it. 
      //Then use ~socket~ only to communicate with client. 
      server = new ServerSocket(pt);
      socket = server.accept();
      
      if (socket.isConnected()){
        socket.setKeepAlive(true);
        socket.setSoTimeout(0);
//        socket.setSendBufferSize(1024);
//        socket.setReceiveBufferSize(1024);
        //socket.setReuseAddress(true);
        //Use self-defined DataInputStream, to transport data with 
        //little-endian bytes ordering
        inSocketStream = new RMDataInputStream(
            new BufferedInputStream(socket.getInputStream()));
        outSocketStream = new RMDataOutputStream(
            new BufferedOutputStream(socket.getOutputStream()));
        
        System.out.println("Connected with Secondo as [" + role + "] in " + hostName + ":" + pt);
      } else
        throw new IOException("Error: Server socket doesn't accpeted");
    }
    else if(ROLE.CLIENT == role)
    {
      //Start the socket to get the connection only.
      socket = null;
      int waitCounter = 0;
      while(socket == null){
        try{
          System.out.println("RemoteStream try connect ... ");
          	
          if (waitCounter++ > 10)
            throw new IOException(
            		"Error: Can't connect to server [" + hostName 
            		+ "] on port: " + pt);
          
          //socket = new Socket(hostName, pt);
          InetSocketAddress srv = new InetSocketAddress(hostName, pt);
          socket = new Socket();
          socket.connect(srv, 0);
          socket.setKeepAlive(true);
          socket.setSoTimeout(0);
//          socket.setSendBufferSize(1024);
//          socket.setReceiveBufferSize(1024);
        }catch (ConnectException e){
          socket = null;
          Thread.sleep(500);
        }        
      }
      
      inSocketStream = new RMDataInputStream(
          new BufferedInputStream(socket.getInputStream()));
      outSocketStream = new RMDataOutputStream(
          new BufferedOutputStream(socket.getOutputStream()));
    }
    
    return true;
  }

  public String readLine() throws IOException
  {
    if (initialized)
      return inSocketStream.readLine();
    else
      return "";
  }
  
  public void writeLine(String s) throws IOException{
    
    if(initialized){
      outSocketStream.writeString(s);
      outSocketStream.flush();
    }
    else
      System.err.println("Error the sender socket is not initialized.");
    
    System.out.println("send line :" + s);
  }
  
  public void close() 
  {
    try{
    if (connected)
    {
      inSocketStream.close();
      outSocketStream.close();
      socket.close();
      
      if(ROLE.SERVER == role && server != null){
        server.close();
        server = null;
      }
      
      connected = false;
    }
    }catch (IOException e){
      e.printStackTrace();
    }
    System.err.println(hostName + " as [" + role + "] is closed.");
  }
  
  synchronized static short Byte2UnsignedShort(byte[] buf, int offset) 
  throws EOFException
  {
    int byte1 = buf[offset];
    int byte2 = buf[offset + 1];
    if (byte2 == -1) throw new EOFException();        
    return (short) (((byte2 << 24) >>> 16) + (byte1 << 24) >>> 24);
  }
  
  private synchronized static byte[] Int2Byte(int value)
      throws IOException
  {
    byte[] bTemp = new byte[4];

    //According to Little-endian ordering
    for (int i = 0; i < 4; i++) {
      bTemp[i] = (byte) ((value >> (i * 8)) & 0xff);
    }
    return bTemp;
  }

  synchronized static int Byte2Int(byte[] value, int offset) throws IOException
  {
    //int len = 4;
    int res = 0, tmp;
    //According to Little-endian ordering
    //System.out.print("Byte2Int: [");
    for (int i = 3; i >= 0; i--) {
      res <<= 8;
      tmp = value[i + offset] & 0xFF;
      //System.out.print(toHex(value[i + offset]));
      res |= tmp;
    }
    //System.out.println("]");
    return res;
  }
  
  public static final String toHex(byte b) {
    return ("" + "0123456789ABCDEF".charAt(0xf & b >> 4) + "0123456789ABCDEF".charAt(b & 0xf));
   }

  
  synchronized static String Byte2String(byte[] value, int offset, int len) throws IOException
  {
/*    if (len < 10){
    System.out.print("Byte2String("+ len + "): [");
    for(int i=0; i < 20; i++)
      System.out.print(toHex(value[offset+i]));
    System.out.println("]");
    }*/
    
    return new String(value, offset, len - 1);      
  }
  
  private synchronized static double Byte2Real(byte[] value, int offset) throws IOException
  {
    return Double.longBitsToDouble(Byte2Long(value, offset));
  }
  
  private synchronized static long Byte2Long(byte[] value, int offset)  throws IOException
  {
    long res = 0;
    for (int i = 7; i >= 0; i--){
      res <<= 8;
      int tmp = value[i + offset] & 0xFF;
      res |= tmp;
    }
    return res;
  }
     
  private synchronized static boolean Byte2Bool(byte[] value, int offset) throws IOException
  {
    int i = (int)value[offset];
    if (i<0) throw new IOException();
    if (i ==0 )
      return false;
    else
      return true;    
  }
  
  private ROLE role;
  private String hostName;
  private int port;
  private int sock_ID;  
  private boolean initialized;
  private boolean connected;
  private boolean endOfStream;
  private String keyType;
  
  private int loadTupleNum;
  
  private ServerSocket server;
  private Socket socket;
  private RMDataInputStream inSocketStream;
  private RMDataOutputStream outSocketStream;
}

class RemoteStreamException extends RuntimeException{
  private static final long serialVersionUID = -222798966184550543L;

  RemoteStreamException(String message){
    super(message);
  }
}
