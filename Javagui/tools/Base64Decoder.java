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

    
   /** decodes a String object 
     * if the string don't contains correct base64 encoded text, null is returned
     **/
static    public byte[] decode(String Data){
     int inLength = Data.length();
     int maxSize = (inLength*3)/4+1;   
     byte[] buffer = new byte[maxSize]; // buffer containing all bytes
     byte[] InBuffer = new byte[4]; // buffer containing the four  inputs
     byte[] OutBuffer = new byte[3];
     // fill the InBuffer with allowed values
     int pos = 0; // Position in String
     int InPos = 0; // Positio in inbuffer
     int OutPos =0;  // Position in outbuffer
     int actualLength = 0; // 
     int len = Data.length(); 
     int filled = 0;
     while(pos<len){
         // fill the InBuffer
         InPos=0;
         while(InPos<4){
            byte c = (byte) Data.charAt(pos);
            if(isAllowed(c)){
               InBuffer[InPos]=(byte)c;
               InPos++;
               pos++;
               if(pos>len && InPos!=4 && InPos!=0){ // unexpected end of input
                  return null;
               } 
            }else{
               pos++;
            }            
         }
         if(InPos==4){
            // at this point InBuffer is filled with allowed chars
         
            // cat inbuffer
            int all =  (getIndex(InBuffer[0])<<18)+
	               (getIndex(InBuffer[1])<<12) +
                       (getIndex(InBuffer[2])<<6) +
	                getIndex(InBuffer[3]);
             // extract outbytes
             for(int i=2;i>=0;i--){
                 OutBuffer[i]=(byte)(all & 255);
	         all = all >> 8;
             }
             filled=3; // three bytes in outbuffer
             if(InBuffer[3]=='='){
                 filled=2;
             }
             if(InBuffer[2]=='='){
	      filled=1;
             }
             for(int i=0;i<filled;i++){
                 buffer[OutPos]=OutBuffer[i];
                 OutPos++;
             }
        }        
     } // all bytes are decoded in the buffer
     // copy buffer into an array of bytes with minimum size
     byte[] result = new byte[OutPos];
     for(int i=0;i<OutPos;i++)
         result[i] = buffer[i];

     return result;
  

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
      if(in!=null)
         in.close();
      if(rin!=null)
         rin.close();
    }

    public InputStream getInputStream(){
       return new Base64InputStream(this);
    }


    private byte[] inbuffer;
    private byte[] outbuffer;
    private int currentPos;
    private boolean endReached;
    private int filled;
    private InputStream in;
    private Reader rin;


    private class Base64InputStream extends InputStream{
        public Base64InputStream(Base64Decoder Dec){
	   this.Decoder = Dec;
	}
	public int read() throws IOException{ return Decoder.getNext();}
	public void close() throws IOException{ Decoder.close();}
        
        public int read(byte[] buffer,int off, int len)throws IOException{
           System.out.println("read called");
           int pos = off;
           if(len>0){
              int last = Decoder.getNext();
              while(pos<len && last>=0){
                  buffer[pos] = (byte) last;
                  pos++;
                  last = Decoder.getNext();
              }
           }
           return pos-off;
        }
	
	public int available() throws IOException{
            int sav = in!=null ? Decoder.in.available(): 1;
	    int nls = sav /72;
	    return sav*3/4-nls;
	}
        Base64Decoder Decoder;
    }


}

