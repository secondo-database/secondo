package sj.lang;


import java.io.*;


public class MyDataInputStream extends InputStream{

public MyDataInputStream(InputStream I){
  In=I;
}

/** closes the Stream */
public void close()throws IOException{
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

public int read() throws IOException{
  return In.read();
}

public int read(byte[] b)throws IOException{
  return In.read(b);
}


public int read(byte[] b,int off,int len)throws IOException{
  return In.read(b,off,len);
}

public void reset() throws IOException{
  In.reset();
}

public long skip(long l)throws IOException{
  return In.skip(l);
}

/** reads a boolean from Stream*/
public boolean readBool() throws IOException{
  int i=In.read();
  if(i<0) throw new IOException();
  if(i==0)
    return false;
  else
    return true;
}

/** reads a Line of text from the stream */
public String readLine() throws IOException{
  StringBuffer r = new StringBuffer();
  char c;
  int len = 0;
  do{
     int i=In.read();
     if(i<0) throw new IOException();
     c = (char) i;
     len ++;
     if(c!='\n')
        r.append(c);;
     } while(c!='\n');
  return r.toString();
}

/* returns the positive value of b */
private int getUnsigned(byte b){
  if(b<0)
    return 256+b;
  else
    return b;
}

/** read a integer from stream */
public int readInt() throws IOException{
  In.read(IntBuffer);
  int res = 0;
  for(int i=0;i<4;i++)
     res = res*256+getUnsigned(IntBuffer[i]);
  return res;
}


/** reads a short value from stream */
public short readShort() throws IOException{
   In.read(ShortBuffer);
   int tmp = 0;
   for(int i=0;i<2;i++)
      tmp = 256*tmp+getUnsigned(ShortBuffer[i]);
   return (short) tmp;
}


/** reads a float from stream */
public float readReal() throws IOException{
   int tmp = readInt();
   float res = Float.intBitsToFloat(tmp);
   return res;
}

/** reads a single byte from stream */
public byte readByte() throws IOException{
   int i = In.read();
   if(i<0) throw new IOException();
   return (byte) i;
}

/** reads a string with specified length from stream */
public String readString(int size) throws IOException{
  byte[] TMP = new byte[size];
  In.read(TMP);
  return new String(TMP);
}

//*********************************************************************************************
// the following methods are for tests only all readed bytes are
// writed in a output stream

/** reads a boolean from Stream*/
public boolean readBool(OutputStream o) throws IOException{
  int i=In.read();
  if(i<0) throw new IOException();
  o.write((byte)i);
  if(i==0)
    return false;
  else
    return true;
}

/** reads a Line of text from the stream */
public String readLine(OutputStream o) throws IOException{
  StringBuffer r = new StringBuffer();
  char c;
  int len = 0;
  do{
     int i=In.read();
     if(i<0) throw new IOException();
     c = (char) i;
     len ++;
     if(c!='\n')
        r.append(c);;
     } while(c!='\n');

  String res = r.toString();
  o.write(res.getBytes());
  return res;
}


/** read a integer from stream */
public int readInt(OutputStream o) throws IOException{
  In.read(IntBuffer);
  o.write(IntBuffer);
  int res = 0;
  for(int i=0;i<4;i++)
     res = res*256+getUnsigned(IntBuffer[i]);
  return res;
}


/** reads a short value from stream */
public short readShort(OutputStream o) throws IOException{
   In.read(ShortBuffer);
   o.write(ShortBuffer);
   int tmp = 0;
   for(int i=0;i<2;i++)
      tmp = 256*tmp+getUnsigned(ShortBuffer[i]);
   return (short) tmp;
}


/** reads a float from stream */
public float readReal(OutputStream o) throws IOException{
   int tmp = readInt(o);
   float res = Float.intBitsToFloat(tmp);
   return res;
}

/** reads a single byte from stream */
public byte readByte(OutputStream o) throws IOException{
   int i = In.read();
   o.write((byte)i);
   if(i<0) throw new IOException();
   return (byte) i;
}

/** reads a string with specified length from stream */
public String readString(int size,OutputStream o) throws IOException{
  byte[] TMP = new byte[size];
  In.read(TMP);
  o.write(TMP);
  return new String(TMP);
}



private InputStream In;
private byte[] IntBuffer = new byte[4];  // avoid creating a buffer frequently
private byte[] ShortBuffer = new byte[2]; // avoid creating a buffer frequently
}

