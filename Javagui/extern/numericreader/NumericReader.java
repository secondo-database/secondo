package extern.numericreader;


/** class to extract numbers from a byte array*/
public class NumericReader{

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
  if(a.length-offset<4) return -1;
  int res = 0;
  int last = offset+4;
  for(int i=0;i<4;i++)
     res = 256*res + getInt(a[last-i-1]);
  return res;
}


public static long getLongLittle(byte[] a,int offset){
  if(a.length-offset<8) return -1;
  long res = 0;
  int last = offset+8;
  for(int i=0;i<8;i++){
     res = 256*res + getInt(a[last-i-1]); 
  }   
       
  return res;
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
