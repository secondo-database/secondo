package tools;

import java.util.TreeSet;
import java.io.*;

/** this class read lines from stdin and
    prints out this lines without duplicates */

public class DupRemover{

public static void main(String[] args){
  try{
     BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
     String line;
     TreeSet Set = new TreeSet();
     while(in.ready()){
        line = in.readLine();
	if(!Set.contains(line)){
	    Set.add(line);
	    System.out.print(line+" ");
	}
     }

  } catch(Exception e){
     System.err.println("error in  removing duplicates");
  }
}

}