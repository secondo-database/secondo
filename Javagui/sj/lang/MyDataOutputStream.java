package sj.lang;

import java.io.*;


/** a class for writing types in a Stream with buffering */
public class MyDataOutputStream extends OutputStream{

public MyDataOutputStream(OutputStream O){
  this.OS= O;
}



public void writeBoolean(boolean b) throws IOException{
   byte v = (byte) (b?1:0);
   OS.write(v);
}

public void writeInt(int v) throws IOException{
  for(int i=0;i<4;i++){
    intbuffer[3-i]=(byte)(v&255);
    v = v>>>8;
  }
  OS.write(intbuffer);
}

public void writeReal(double d) throws IOException{
    writeReal((float)d);
}


public void writeReal(float f) throws IOException{
  int i = Float.floatToIntBits(f);
  writeInt(i);
}

public void writeString(String S) throws IOException{
  byte[] bytes = S.getBytes();
  OS.write(bytes);
}

public void writeByte(byte b) throws IOException{
  OS.write(b);
}

public void writeShort(short b) throws IOException{
  OS.write((byte)(b/256));
  OS.write((byte)(b%256));
}  


public void writeShort(int i) throws IOException{
  writeShort((short)i);
}

public void close() throws IOException{
  OS.close();
}

public void flush()throws IOException{
  OS.flush();
}

public void write(byte[] b)throws IOException{
  OS.write(b);
}

public void write(byte[] b, int off, int len)throws IOException{
  OS.write(b,off,len);
}

public void write(int b) throws IOException{
  OS.write(b);
}
private OutputStream OS;
private byte[] intbuffer = new byte[4];
}
