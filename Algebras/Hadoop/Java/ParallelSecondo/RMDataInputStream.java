package ParallelSecondo;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Vector;

import sj.lang.MyDataInputStream;


public class RMDataInputStream extends MyDataInputStream {

  public RMDataInputStream(InputStream I) {
    super(I);
    // TODO Auto-generated constructor stub
    In = I;
  }
  
  public void close() throws IOException{
    In.close();
  }
  
  public int available() throws IOException{
    return In.available();
 }


 public void mark(int readLimit){
   In.mark(readLimit);
 }

 public boolean markSupported(){
   return In.markSupported();
 }

 /* returns the positive value of b */
 private int getUnsigned(byte b){
   if(b<0)
     return 256+b;
   else
     return b;
 }

 /** read a integer from stream */
 /** follows little-endian ordering */
 public int readInt() throws IOException{
   readFullBuffer(IntBuffer);
   int res = 0;
   for(int i=3;i>=0;i--){
      res = res*256+getUnsigned(IntBuffer[i]);
   }
   return res;
 }
 
 /** fills the buffer
  * if not enough data are available a IOException is thrown
  */
private void readFullBuffer(byte[] buffer) throws IOException{
  int size = buffer.length;
  int readed = 0;
  int small;
  while(readed<size){
     small = In.read(buffer,readed,size-readed);
     if(small<0)
       throw new IOException("not enough bytes available");
     readed += small;
  }
}

/** reads a Line of text from the stream *//*
public String readLine() throws IOException{
   return readLine(null);
}

*//** reads a Line of text from the stream *//*
public String readLine(OutputStream o) throws IOException{
  int len = 0;
  Vector a = new Vector(1000);
  int i;
  do{
     i=In.read();
     if(i<0) throw new IOException();
     len ++;
     if(((char)i)!='\n')
        a.add(new Byte((byte)i));
     } while(((char)i)!='\n');

  byte[] b = new byte[a.size()];
  for(int j=0;j<a.size();j++){
    b[j]= ((Byte)a.get(j)).byteValue();
  } 
  if(o!=null) o.write(b);
  if(gui.Environment.ENCODING!=null) {
     return new String(b,gui.Environment.ENCODING);
  } else {
     return new String(b); 
  }
}*/
  
  private InputStream In;
  private byte[] IntBuffer = new byte[4];  // avoid creating a buffer frequently
  private byte[] LongBuffer = new byte[8]; 
  private byte[] ShortBuffer = new byte[2]; // avoid creating a buffer frequently
}
