package tools;
import java.io.*;


public class Base64Decoder{

  private static boolean isAllowed(byte b){
     return getIndex(b)>=0;
  }

   /** return the unsigned interger value for b */
   private static int unsigned(byte b){
      return b<0? (256+b) : b;
   }




   // returns the index of the specific byte in the Table
   private static int getIndex(byte b){
     if(b>='A' && b<='Z')
        return b-'A';
     if(b>='a' && b<='z')
        return b-'a'+26;
     if(b>='0' && b<='9')
        return b-'0'+52;
     if(b=='+')
        return 62;
     if(b=='/')
        return 63;
     if(b=='=')
        return 0;
     return -1;
   }


   /** creates a new Base64-Decoder */
   public Base64Decoder(InputStream input){
       in = input;
       rin = null;
       outbuffer = new byte[3];
       inbuffer = new byte[4];
       currentPos = 5; // need to fill the outbuffer
       endReached = false;
       filled=3;
    }

   /** creates a new Base64Decoder */
   public Base64Decoder(Reader R){
       in = null;
       rin = R;
       outbuffer = new byte[3];
       inbuffer = new byte[4];
       currentPos = 5; // need to fill the outbuffer
       endReached = false;
       filled=3;
   }


  /** return the next byte of decoded data as positive integer
    * if the end of data reached -1 is returned
    */
  public int getNext() throws IOException{

       if(currentPos<filled){  // we have data in buffer
          int res = unsigned(outbuffer[currentPos]);
          currentPos++;
	  return res;
       }

       if(endReached) // no more data to load
          return -1;

       // fill the inbuffer
       int nextin;
       for(int i=0;i<4;i++){ // get the next allowed input bytes
           nextin = in!=null? in.read():rin.read();
	   while(nextin>=0 & !isAllowed((byte)nextin)) // override not allowed bytes
	       nextin = in!=null? in.read():rin.read();
	   if(nextin<0 & i>0) // not a full quadrupel found
	       throw new IOException("unexpected end of input");
	   if(nextin<0){ // end of input
	      endReached=true;
	      return -1;
	   }
          inbuffer[i]= (byte)nextin;  //store value
       }

       // cat inbuffer
       int all =  (getIndex(inbuffer[0])<<18)+
		  (getIndex(inbuffer[1])<<12) +
                  (getIndex(inbuffer[2])<<6) +
		   getIndex(inbuffer[3]);

      // extract outbytes
       for(int i=2;i>=0;i--){
          outbuffer[i]=(byte)(all & 255);
	  all = all >> 8;
       }


       filled=3; // three bytes in outbuffer
       if(inbuffer[3]=='='){
           filled=2;
	   endReached=true;
       }
       if(inbuffer[2]=='='){
           endReached=true;
	   filled=1;
       }
       currentPos = 1; // the first byte is given now
       return unsigned(outbuffer[0]);
    }

    public void close() throws IOException{
      in.close();
    }


    private byte[] inbuffer;
    private byte[] outbuffer;
    private int currentPos;
    private boolean endReached;
    private int filled;
    private InputStream in;
    private Reader rin;

}

