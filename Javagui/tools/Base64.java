package tools;

import java.io.*;

/*
  a simple program which encoded a binary file into the base64 format
  and vice versa.
*/

public class Base64{


public static void main(String[] args){

  if(args.length<2 | args.length>3){
    System.err.println("usage: java [-d] Base64 source target");
    System.exit(0);
  }

  boolean decode = false;
  if(args.length==3){
     if(!args[0].trim().toUpperCase().equals("-D")){
         System.err.println("usage: java [-d] Base64 source target");
         System.exit(0);
     }
     decode = true;
  }

  String InFileName = args[args.length-2];
  String OutFileName = args[args.length-1];

  try{
     BufferedInputStream in = new BufferedInputStream(new FileInputStream(InFileName));
     BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(OutFileName));

     int next;
     if(decode){
       Base64Decoder Dec = new Base64Decoder(in);
       while((next=Dec.getNext())>=0)
          out.write((byte)next);
     }else{
       Base64Encoder Enc = new Base64Encoder(in);
       while((next=Enc.getNext())>=0)
           out.write((byte)next);
       out.write('\n');
     }
     in.close();
     out.close();
  }catch(Exception e){
     System.err.println("Error");
     e.printStackTrace();
  }



}



}
