package extern.numericreader;


/** class to extract numbers from a char array*/
public class NumericReader{

/** returns a Integer */
public static int getIntBig(char[] a,int offset){
  if(a.length-offset <4) return -1;
  int res = 0;
  for(int i=offset;i<offset+4;i++)
     res = res*256+(int)a[i];
  return res;
}


public static int getIntLittle(char[] a,int offset){
  if(a.length-offset<4) return -1;
  int res = 0;
  int last = offset+4;
  for(int i=0;i<4;i++)
     res = 256*res + (int)a[last-i-1];
  return res;
}


public static long getLongLittle(char[] a,int offset){
  if(a.length-offset<8) return -1;
  long res = 0;
 /* res =   a[offset]
        + (((long)a[offset+1])<<8)
        + (((long)a[offset+2])<<16)
        + (((long)a[offset+3])<<24)
        + (((long)a[offset+4])<<32)
        + (((long)a[offset+5])<<40)
        + (((long)a[offset+6])<<48)
        + (((long)a[offset+7])<<56);
  */
  int last = offset+8;
  for(int i=0;i<8;i++)
     res = 256*res + (long)a[last-i-1];
  return res;

}

public static long getLongBig(char[] a,int offset){
  if(a.length-offset <8) return -1;
  long res = 0;
  for(int i=offset;i<offset+8;i++)
     res = res*256+(long)a[i];
  return res;

}

public static double getDoubleBig(char[] a, int offset){
  return Double.longBitsToDouble(getLongBig(a,offset));
}

public static double getDoubleLittle(char[] a, int offset){
  return Double.longBitsToDouble(getLongLittle(a,offset));
}

public static int getShortBig(char[] Buffer,int offset){
   if(Buffer.length-offset <2) return -1;
   int res = 0;
   for(int i=offset;i<offset+2;i++)
     res = res*256+(int)Buffer[i];
   return res;
}

public static int getShortLittle(char[] a,int offset){
  if(a.length-offset<2) return -1;
  int res = 0;
  int last = offset+2;
  for(int i=0;i<2;i++)
     res = 256*res + (int)a[last-i-1];
  return res;
}


}
