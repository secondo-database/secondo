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

package wrapper;
import java.io.*;

public class Serializer{


/** Reads a object from buffer and returns it.
  * If the buffer does not conatain an object,
  * null is returned
 
*/
public static Object readFrom(byte[] buffer){
     try{
         ObjectInputStream ois;
         ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
         Object res =  ois.readObject();
         ois.close();
         return res;
      } catch(Exception e){
           return null;
      }
}


/** Writes an object into a array of bytes.
  * If the Object cannot serailized, the 
  * result will be null.
  **/
public static  byte[] writeToByteArray(Serializable o){
       try{
          ByteArrayOutputStream byteout = new ByteArrayOutputStream();
          ObjectOutputStream objectout = new ObjectOutputStream(byteout);
          objectout.writeObject(o);
          objectout.flush();
          byte[] res = byteout.toByteArray();
          objectout.close();
          return  res;
       } catch(Exception e){
          e.printStackTrace();
          return null;
       }
 }

}
