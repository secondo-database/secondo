//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

public void writeLong(long v) throws IOException{
  for(int i=0;i<8;i++){
    longbuffer[7-i]=(byte)(v&255);
    v = v>>>8;
  }
  OS.write(longbuffer);
}

public void writeReal(double d) throws IOException{
    writeReal((float)d);
}

public void writeDouble(double d) throws IOException{
    long i = Double.doubleToLongBits(d);
    writeLong(i);
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
private byte[] longbuffer = new byte[8];
}
