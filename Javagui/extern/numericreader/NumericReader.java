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

package extern.numericreader;


/** class to extract numbers from a byte array*/
public class NumericReader{

static long toBig(long i){
   long x = i;
   x = (x>>56) |
       ((x<<40) & 0x00FF000000000000l) |
       ((x<<24) & 0x0000FF0000000000l) |
       ((x<<8)  & 0x000000FF00000000l) |
       ((x>>8)  & 0x00000000FF000000) |
       ((x>>24) & 0x0000000000FF0000) |
       ((x>>40) & 0x000000000000FF00) |
       (x<<56);
       return x;
 }
 
 static double toBig(double d){
   return Double.longBitsToDouble( toBig(Double.doubleToLongBits(d)));
 }

static int toBig(int i){
   return((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff);
}




public static int getInt(byte b){
int res = (int) b;
if(b<0)
   res = res+256;
return res;   

}


/** returns a Integer */
public static int getIntBig(byte[] a,int offset){
  if(a.length-offset <4) return -1;
  int res = 0;
  for(int i=offset;i<offset+4;i++)
     res = res*256+getInt(a[i]);
  return res;
}


public static int getIntLittle(byte[] a,int offset){
   return toBig(getIntBig(a,offset));
}


public static long getLongLittle(byte[] a,int offset){
   return toBig(getLongBig(a,offset));
}


public static long getLongBig(byte[] a,int offset){
  if(a.length-offset <8) return -1;
  long res = 0;
  for(int i=offset;i<offset+8;i++)
     res = res*256+getInt(a[i]);
  return res;

}

public static double getDoubleBig(byte[] a, int offset){
  return Double.longBitsToDouble(getLongBig(a,offset));
}

public static double getDoubleLittle(byte[] a, int offset){
  long L =  getLongLittle(a,offset);
  double D = Double.longBitsToDouble(L);
  return D;   
}


public static int getShortBig(byte[] Buffer,int offset){
   if(Buffer.length-offset <2) return -1;
   int res = 0;
   for(int i=offset;i<offset+2;i++)
     res = res*256+getInt(Buffer[i]);
   return res;
}

public static int getShortLittle(byte[] a,int offset){
  if(a.length-offset<2) return -1;
  int res = 0;
  int last = offset+2;
  for(int i=0;i<2;i++)
     res = 256*res + getInt(a[last-i-1]);
  return res;
}


}
