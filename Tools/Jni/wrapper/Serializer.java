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
