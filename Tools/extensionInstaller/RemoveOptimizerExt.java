

import java.io.*;
import java.util.TreeSet;

public class RemoveOptimizerExt{


public static void main(String[] args){

  for(int i=0;i<args.length;i++){
     File f = new File(args[i]);
     if(!f.exists()){
        System.err.println("file " + f + " does not exist");
     } else {
        String content = "";
        BufferedReader in = null;
        PrintWriter out  = null;
        try{
           in = new BufferedReader(new FileReader(f));
           String currentSection = null;
           TreeSet<String> names = new TreeSet<String>();
           while(in.ready()){
              String line = in.readLine();
              if(line.matches("%\\s+Extension:Start:\\s*\\w+\\s*")){
                 String name = line.replaceAll("%\\s+Extension:Start:","");
                 name = name.trim();
                 if(currentSection!=null){
                    System.err.println("nested Sections detected: try to start section " + 
                                       name + "within section " + currentSection);
                    throw new Exception("nested sections found");
                 } else if(names.contains(name)){
                    System.err.println("section " + name + "found twice");
                    throw new Exception("twice name found: "+ name);
                 } else {
                   content += line +"\n";
                   currentSection = name;
                   names.add(name);
                 }
              } else if(line.matches("%\\s+Extension:End:\\w+\\s*")){
                 String name = line.replaceAll("%\\s+Extension:End:","");
                 name = name.trim();
                 if(currentSection==null){
                    System.err.println("Try to end section " + name + 
                                       " but no section was start before");
                    throw new Exception("non started section closed: " + name);
                 } else if(!name.equals(currentSection)){
                     System.err.println("conflicting names, try to close section " + name +
                                        "but section " + currentSection + " is active");
                     throw new Exception("conflicting names");
                 }else {
                    content += line + "\n";
                    currentSection = null;
                 }
              } else { // a normal line
                 if(currentSection==null){
                     content += line + "\n";
                 }
              }
           }
           try{in.close(); in=null;} catch(Exception e){}
           if(currentSection!=null){
               System.out.println("section " + currentSection +" not closed");
               throw new Exception("unclosed section found " + currentSection);
           }
           out=new PrintWriter(new FileWriter(f));
           out.print(content);
           try{out.close(); out=null; } catch(Exception e){}
        } catch(Exception e){
          System.err.println("Error during processing file " + f);
        } finally {
          if(in!=null){try{in.close();in=null;}catch(Exception e){}}
          if(out!=null){try{out.close();out=null;}catch(Exception e){}}
        }

     }

  }

}


}
