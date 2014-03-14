

import java.io.*;
import java.util.*;

class Opsyntax{
  Opsyntax(String n, String k , int num){
    name = n;
    kind = k;
    args = num;
  }

  public String toString(){
    return name + " : " + kind + ", " + args;
  }

  public boolean equals(Object o){
     if(!(o instanceof Opsyntax)) return false;
     Opsyntax s = (Opsyntax)o;
     return name.equals(s.name) && kind.equals(s.kind); // && (args==s.args);
  }

  String name;
  String kind;
  int args;
}

public class CompareOpSyntax{


  static Opsyntax getSyntax(String line){
      line = line.trim();
      if(!line.startsWith("secondoOp")){
         return null;
      }
      int start = line.indexOf("(");
      int end = line.indexOf(".");
      if(start<0 || end<0) return null;
      line = line.substring(start,end);
      start = line.indexOf("(");
      end = line.lastIndexOf(")");

      line = line.substring(start+1, end-(start));
      StringTokenizer st = new StringTokenizer(line,", ");
      if(st.countTokens()!=3){
         System.err.println("1: invalid line " + line);
         return null;
      }
      String opName = st.nextToken();
      String Kind = st.nextToken();
      String nums = st.nextToken();
      int num = -1;

      if(opName.startsWith("(") && opName.endsWith(")")){
        opName = opName.substring(1, opName.length()-1);
      }

      try{
        num = Integer.parseInt(nums);

      }  catch(Exception e){
         System.err.println("2: invalid line " + line);
         return null;
      }
      Opsyntax res = new Opsyntax(opName,Kind,num);
    //  System.out.println("read " + res);
      return res;

   }


  static void processLine(String line, HashMap<String, Opsyntax> map){   
      Opsyntax s = getSyntax(line);
      if(s!=null){
         map.put(s.name,s);
      }
  }

  public static void main(String[] args){
     
     HashMap<String, Opsyntax> map = new HashMap<String, Opsyntax>();

     try{
       BufferedReader in = new BufferedReader(new FileReader(new File(args[0])));
       while(in.ready()){
         String line = in.readLine();
         processLine(line,map);
       }
       in.close();

       in = new BufferedReader(new FileReader(new File(args[1])));
       while(in.ready()){
          String line = in.readLine();
          Opsyntax s = getSyntax(line);
          if(s!=null){
             Opsyntax ms = map.get(s.name);
             if(ms!=null && !ms.equals(s) && !ms.kind.equals("special")){
                System.out.println("differences at " + s + "\n original = " + map.get(s.name));
             }
          }
       }



     } catch(Exception e){
       e.printStackTrace();
       System.exit(1);
     }


  }



}
