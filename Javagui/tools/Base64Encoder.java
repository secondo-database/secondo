package tools;

import java.io.*;

/** this class provides a Base64-Encoder */
public class Base64Encoder{


   /** returns the size of encoded code when the
     * original code is given
     */
   public static long getEncodedSize(long size){
     long s2 = (3-size%3)%3 + size; // take the next multiple of three
     s2 = s2*4/3;  // 3 bytes are encodes to 4 bytes
     return s2 + s2/72;  // after 72 characters a newline follows
   }


   /** creates the Encoder */
   public Base64Encoder(InputStream input){
     in = input;
   }


   /** the char-table for the base 64 format */
   static byte[] Table ={'A','B','C','D','E','F','G','H','I','J','K',
                        'L','M','N','O','P','Q','R','S','T','U','V',
	  	        'W','X','Y','Z','a','b','c','d','e','f','g',
		        'h','i','j','k','l','m','n','o','p','q','r','s',
		        't','u','v','w','x','y','z','0','1','2','3',
		        '4','5','6','7','8','9','+','/'};

   /** return the unsigned interger value for b */
   private static int unsigned(byte b){
      return b<0? (256+b) : b;
   }

   /** closes the underlying InputStream */
   public void close() throws IOException{
      in.close();
   }





  /** returns the next byte of the encoded data as positive integer value
    * if the end of data reached -1 is returned
    */
  public int getNext() throws IOException{
     if (CurrentPos<4){ // take data in buffer
         int res = outbuffer[CurrentPos];
	 CurrentPos++;
	 linelength++;
	 return res;
     }
     if(linelength==72){
       linelength=0;
       return (int) '\n';
     }
     // get new datas from InputString;
     int length = in.read(inbuffer);
     if(length<=0)  // no data in inputstream
         return -1;

     // here is make the main encoding
     // three bytes are coded into four bytes
     for(int i=length;i<3;i++)
        inbuffer[i] = 0;
    // code all given bytes in a single integer
    int all= 65536*unsigned(inbuffer[0])+256*unsigned(inbuffer[1])+unsigned(inbuffer[2]);
    // extract the 6 bit values
    // read out the 6 bit
    for(int i=3;i>=0;i--){
      byte index = (byte)(all & 63);
      outbuffer[i] = Table[index];
      all = all >>> 6;
    }
    for(int i=0;i<3-length;i++)
       outbuffer[3-i]='=';

     CurrentPos =1;
     linelength++;
     return (int) outbuffer[0];
  }

    public int available() throws IOException{
    double ain = in.available()*4.0/3.0;
    int aout = (int)ain<ain ? (int)ain+1 : (int)ain;
    aout = aout + 4-CurrentPos; // + Buffer
    aout = aout + aout/72;  // newlines
    return aout;
  }

  private InputStream in;
  private byte[] outbuffer = new byte[4];
  private byte[] inbuffer = new byte[3];

  private boolean isEnd = false;
  private int CurrentPos = 6;
  private int linelength=0;
}

