
import java.io.*;
import java.util.StringTokenizer;
import java.util.Vector;

public class Convert{



public static void processLine(String line){
 if(line == null){
   return;
 }
 line = line.trim();
 if(line.length()==0){
    return;
 }
 StringTokenizer st = new StringTokenizer(line);
 Vector v = new Vector();
 try{
    boolean first = true;
    while(st.hasMoreTokens()){
       if(first){
         v.add( new Integer(Integer.parseInt(st.nextToken())));
         first = false;
       } else {
          v.add( new Double(Double.parseDouble(st.nextToken())));
       }
    }

 } catch(Exception e){
    System.err.println("Error in processing line");
    return;
 }
 if( v.size()!=5){
   System.err.println("wrong format in line " + line );
   return;
 } 
 System.out.println("( " + v.get(0) + " (" + v.get(1) + " " + 
                           v.get(3) + " " + v.get(2) + " " + v.get(4) + "))");


}

public static void main(String[] args){
  try {
     String name = args[0];
     BufferedReader in = new BufferedReader(new FileReader(name));
     System.out.println("( ( rel(tuple( ( Id int) ( R rect)))) ");
     System.out.println("  (");
     String line;
     while(in.ready()){
        line  = in.readLine();
        processLine(line);
     }
     in.close();
     System.out.println(" )");
     System.out.println(")");
  }catch(Exception e){
    e.printStackTrace();
  }
  


}



}
