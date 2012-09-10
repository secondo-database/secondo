package ParallelSecondo;

import java.io.IOException;
import java.io.OutputStream;
import sj.lang.MyDataOutputStream;


public class RMDataOutputStream extends MyDataOutputStream {
  
  
  public RMDataOutputStream(OutputStream O) {
    super(O);
    // TODO Auto-generated constructor stub
    this.OS = O;
  }
  
  public void close() throws IOException{
    OS.close();
  }
  
  
  public void writeInt(int v) throws IOException{
    for(int i=0;i<4;i++){
      intbuffer[i]=(byte)(v&255);
      v = v>>>8;
    }
    OS.write(intbuffer);
  }
  
  public void writeString(String S) throws IOException{
    byte[] bytes;
    bytes = S.getBytes();
    OS.write(bytes);
    OS.write((byte)'\n');  //Indicate the end of the string
  }
  
  public void flush()throws IOException{
    OS.flush();
  }
  
  private OutputStream OS;
  private byte[] intbuffer = new byte[4];
  private byte[] longbuffer = new byte[8];
}
