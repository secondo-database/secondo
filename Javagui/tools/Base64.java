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
