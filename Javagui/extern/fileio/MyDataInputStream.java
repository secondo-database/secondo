package extern.fileio;


import java.io.*;


public class MyDataInputStream{

public MyDataInputStream(InputStream I){
  In=I;
}

public void close()throws IOException{
   In.close();
}

public boolean readBool() throws IOException{
  int i=In.read();
  if(i<0) throw new IOException();
  if(i==0) 
    return false;
  else
    return true;    
}

private int getUnsigned(byte b){
  if(b<0)
    return 256+b;
  else  
    return b;
}

public int readInt() throws IOException{
  In.read(IntBuffer);  
  int res = 0;
  for(int i=0;i<4;i++)
     res = res*256+getUnsigned(IntBuffer[i]);
  return res;    
}

public short readShort() throws IOException{
   In.read(ShortBuffer);
   int tmp = 0;
   for(int i=0;i<2;i++)
      tmp = 256*tmp+getUnsigned(ShortBuffer[i]);
   return (short) tmp; 
}


public float readReal() throws IOException{
   int tmp = readInt();
   float res = Float.intBitsToFloat(tmp);
   return res;
}

public byte readByte() throws IOException{
   int i = In.read();
   if(i<0) throw new IOException();
   return (byte) i;
}

public String readString(int size) throws IOException{
  byte[] TMP = new byte[size];
  In.read(TMP);
  return new String(TMP);
}


private InputStream In;
private byte[] IntBuffer = new byte[4];  // avoid creating a buffer frequently
private byte[] ShortBuffer = new byte[2]; // avoid creating a buffer frequently
}

