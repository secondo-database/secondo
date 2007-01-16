


import java.io.*;
import java.util.*;
import viewer.hoese.LEUtils;
import sj.lang.ListExpr;
import tools.*;


/*
VAG.Com csv Format

1. line: starting time,e.g. Monday, 08, January, 2007 , 04:42:30
2. line: vehicle identification (ignored)
3. line: empty
4. line: used groups (ignored)
5. line: names of the measured values
6. line: allowed ranges 
7. line: units
8. line: begin of the data

Notes:
- time is gives as distance to the start time in seconds
- if an value is measured in different groups (same name), the 
  frequency of the measures increases



*/

public class VagCom2Secondo{

private double currentTime;
private double startTime;
private UnitWriter writer;


/** Computes a double value from a textual decsription of a time value.  
*/
private double getStartTime(String time){
    // format is:  weekday, day, month, year, hour:minute:second
    // note: month is a string, not a number
    MyStringTokenizer st1 = new MyStringTokenizer(time,',');
    String weekday = st1.nextToken();
    String day = st1.nextToken();
    String monthstr =  st1.nextToken();
    int month = 0;
    if(monthstr.equals("January")){
        month = 1;
    } else if (monthstr.equals("February")){
        month = 2;
    } else if (monthstr.equals("March")){
        month = 3;
    } else if (monthstr.equals("April")){
        month = 4;
    } else if (monthstr.equals("May")){
        month = 5;
    } else if (monthstr.equals("June")){
        month = 6;
    } else if (monthstr.equals("July")){
        month = 7;
    } else if (monthstr.equals("August")){
        month = 8;
    } else if (monthstr.equals("September")){
        month = 9;
    } else if (monthstr.equals("October")){
        month = 10;
    } else if (monthstr.equals("November")){
        month = 11;
    } else if (monthstr.equals("December")){
        month = 12;
    } else {
        System.err.println("Error in reading StartTime");
        System.exit(12);
    }
    String year = st1.nextToken();
    String time1 = st1.nextToken();

    String timeString= year+"-"+month+"-"+day+"-"+time1;
    //System.err.println("TimeString is " + timeString );

    Double timeD = LEUtils.convertStringToTime(timeString);
    if(timeD==null){
       System.err.println("error in reading startTime");
       System.exit(0);
    } 
    return timeD.doubleValue();
    
}  


/** Converts a name, e.g. "Coolant. Temp." into another string which can be used
  * as a name for an attribute.
  **/  
private String name2Symbol(String name){
  return name.replace('.','_').replace(' ','_').replace('(','_').replace(')','_').replace('?','_');
}

private boolean writeContent(String line, int pos, String kind,PrintStream out){
		MyStringTokenizer st = new MyStringTokenizer(line,',');
		int number = 0;
		boolean done = false;
    boolean ok = true;
		while(st.hasMoreTokens() && !done){
				 String token = st.nextToken();
				 number++;
				 if(number==pos){
						 done = true;
						 out.print(" \""+token+"\" ");
				 }
		}   
		if(!done){
			System.err.println("could not extract " + kind + " at position " 
												 + pos);
      out.print(" \"\" ");
			ok = false;
		}
    return ok;

}


/* Processes the rows given in the argument.
   The value in the array has to be [time1, value 1, time2 value2, ...].
   This function produces the complete name, the allowed range and the
   unit from row[1] as string attributes.
   A third attribute will contain the moving value given by the data section.

*/

private void processRows(File f,int[] rows, PrintStream out){

   if(rows==null){ // invalid input
      return;
   }
   if(rows.length<2){ // invalid input
       return;
   }
   if(rows.length % 2 != 0){ // invalid input
       return;
   }

   try{
       BufferedReader in = new BufferedReader((new FileReader(f)));

       String line;
       int linecount=0;
       boolean ok = true;
       while(in.ready() && ok){
          line = in.readLine();
          linecount++;

          //System.err.println("line " + linecount + " = " + line);

          if(line!=null){
              switch(linecount){
                 case 1: // ignore starttime information 
                        break;
                 case 2: //vehicle information (ignored)
                        break;
                 case 3: // an empty line
                        break;
                 case 4: // groups (ignore)
                        break;
                 case 5: // extract the name as string type
                        writeContent(line,rows[1],"AttrName", out);
                        break;
                 case 6: // extract the ranges
                        writeContent(line,rows[1],"range", out);
                        break;
                 case 7: // extract the units
                        writeContent(line,rows[1],"unit", out);
                        break;
                 default: // extract the data
                        // the first time value
                      //  System.err.println("process line  no " + linecount + " ( " + line +")");
                        int grp = 0;
                        int timeIndex = rows[2*grp+0];
                        int attrIndex = rows[2*grp+1];

                      //  System.err.println("attrIndex = " + attrIndex);
                      //  System.err.println("timeIndex = " + timeIndex);


                        if(attrIndex <= timeIndex){
                            System.err.println("Error: AttrIndex <= timeIndex");
                            System.exit(12);
                        }
                        MyStringTokenizer st = new MyStringTokenizer(line,',');
                        boolean done = false;
                        int number=0;
                        while(st.hasMoreTokens() && ! done){
                            number++;
                            String token = st.nextToken();
                            if(number==timeIndex){
                               //System.err.println("read time from " + token);
                               try{
                                 double diff = Double.parseDouble(token);
                                 currentTime = startTime+diff/86400.0;
                               }catch(Exception e){
                                 e.printStackTrace();
                                 done = true; // ignore this line 
                               }
                            }
                            if(number==attrIndex){
                               try{
                                //  System.err.println("read value from " + token);
                                  token = token.replaceAll(" ","");
                                  double value = Double.parseDouble(token);
                                //  System.err.println("add ("+currentTime+" ,  "+ value+") to the writer " );
                                  writer.add(value,currentTime);
                                  grp++;
                                  if(2*grp>=rows.length){
                                    done = true;
                                  } else{ // update indexes
                                     timeIndex = rows[2*grp+0];
                                     attrIndex = rows[2*grp+1];
                        //System.err.println("attrIndex = " + attrIndex);
                        //System.err.println("timeIndex = " + timeIndex);
                                  }
                               }  catch(Exception e){
                                  e.printStackTrace();
                                  done = true;
                               }      
                            }
                        }
                        if(!done){
                             System.err.println("Error in line " + linecount);
                        }
                        break;

              }
          }
       }
       in.close();
   } catch (Exception e){
      e.printStackTrace();
      System.exit(1);
   }
}



private void processFile(File f , PrintStream out){
    try{
       BufferedReader in = new BufferedReader(new FileReader(f));
       // we need the line numbers 5 and 6 containing the names and ranges
       String line1 = in.readLine(); // time
       String line2 = in.readLine(); // car
       String line3 = in.readLine(); // empty
       String line4 = in.readLine(); // groups
       String line5 = in.readLine(); // names 
       String line6 = in.readLine(); // ranges
       in.close();

       startTime = getStartTime(line1);
       int timeIndex = -1;
       int number = 0;
      
       // write the header
       MyStringTokenizer stNames = new MyStringTokenizer(line5,',');
      
       out.println("(rel (tuple (");
       Vector used = new Vector(20);
       used.add("");
       while(stNames.hasMoreTokens()){
           String token = stNames.nextToken();
           if(!used.contains(token)){
               String s = name2Symbol(token);
               out.println("(" + s+"Name string )"  ) ;     
               out.println("(" + s+"Range string )"  );      
               out.println("(" + s+"Unit string )"  ) ;     
               out.println("(" + s+"Value mreal )"  ) ;     
               used.add(token);
           }
       }
       out.println(")))");

       out.println("((");  // open value list and tuple 


       Vector Positions = new Vector(20); // all positions
       stNames = new MyStringTokenizer(line5,',');
       MyStringTokenizer stRanges = new MyStringTokenizer(line6,',');
       number = 0;
       timeIndex = -1;
       while(stNames.hasMoreTokens() && stRanges.hasMoreTokens()){
            number++;
            String sName = stNames.nextToken();
            String sRange = stRanges.nextToken();
            if(sRange.equals("TIME")){
               timeIndex=number;
            } else {
              if(!sName.equals("")){
                if(timeIndex<0){
                    System.err.println("Attrname found before time is defined");
                } else{
                    TimeAndPos tp = new TimeAndPos(sName,timeIndex,number);
                    int tpIndex = Positions.indexOf(tp);
                    if(tpIndex<0){
                        Positions.add(tp);
                    } else {
                        ((TimeAndPos)Positions.get(tpIndex)).merge(tp);
                    }
                }
              }
            }
       }

       // write some statistics
       //System.err.println("found " + Positions.size()+" different attributes");
       //System.err.println("These are: " );
       //for(int i=0;i<Positions.size();i++){
       //    System.err.println(((TimeAndPos)Positions.get(i)).name);
       //}


       for(int i=0;i<Positions.size();i++){
           TimeAndPos tp = (TimeAndPos) Positions.get(i);
           int tpsize = tp.positions.size();
           //if(tpsize>1){
           //    System.err.println("Attr " + tp.name + " occurs " + tpsize+ " times ");
           //}
           int[] Pos = new int[tpsize*2];
           for(int k=0;k<tpsize;k++){
                Ints v = (Ints) tp.positions.get(k);
                Pos[k*2] = v.a;
                Pos[2*k+1] = v.b;
           }
           writer = new UnitWriter(out);
           processRows(f,Pos,out);   
           writer.finish();   // finish

       }
       out.println("))"); // close tuple and value list
    } catch(Exception e){
       e.printStackTrace();
    }
}


private static class TimeAndPos implements Comparable{

  private String name;
  private Vector positions= new Vector(3);

  public TimeAndPos(String name, int a, int b){
       this.name = name;
       Ints ints = new Ints(a,b);
       positions.add(ints);
  }

  public boolean equals(Object obj){
     return compareTo(obj)==0;
  }
   
  public int compareTo(Object obj){
      if(!(obj instanceof TimeAndPos)){
         return -1;
      }
      return name.compareTo(((TimeAndPos)obj).name);
  }

  private void merge(TimeAndPos tp){
       for(int i=0;i<tp.positions.size();i++){
           Ints v = (Ints)tp.positions.get(i);
           if(!positions.contains(v)){
              positions.add(v);
           }
       }
  }



}


private static class Ints{
     private Ints(int a, int b){
          this.a = a;
          this.b = b;
     }
     int a;
     int b;
  }


private static void showUsage(){
    System.err.println("usage: java -classpath .:$JGPATH VagCom2Secondo <infile> [-o <outfile>] [-n <objname>]");
    System.err.println(" $JGPATH  : Path to the Javagui directory ");
    System.err.println("<infile>  : the file to convert ");
    System.err.println("<outfile> : the file to write (otherwise stdout) ");
    System.err.println("<objname> : name of the object to create (other wise just a list (type value)");
    System.exit(1);
}


public static void main(String[] args){

    if(args.length<1){
        System.err.println("Source file required");
        return;
    }

    VagCom2Secondo converter = new VagCom2Secondo();

    // search for arguments
    // allowed options are 
    //    -o <outFile>
    //    -n <OBJECTNAME>

    if( (args.length % 2) !=1){
         showUsage();
    } 
    String outfileName = null;
    String objName  = null; 
    for(int i=1;i<args.length-1;i=i+2){
       if(args[i].equals("-n")){
           if(objName!=null){
              System.err.println("object name already specified");
              System.exit(1);
           }
           objName = args[i+1];
       } else  if(args[i].equals("-o")){
           if(outfileName!=null){
              System.err.println("ouput file already specified");
              System.exit(1);
           }
           outfileName = args[i+1];
       } else{
           System.err.println("unknown option " + args[i]);
           showUsage();  
       }
    }
        

    PrintStream out = null;
    if(outfileName==null){
       out = System.out;
    } else {
      try{
        out = new PrintStream(new FileOutputStream(outfileName));
      } catch(Exception e){
        System.err.println("problem in opening outputfile ");
        System.exit(1);
      }
    }

    out.println("(");

    if(objName!=null){
        out.println("OBJECT " + objName + " () ");

    }

    converter.processFile(new File(args[0]),out);

    out.println(")");
    
    if(outfileName!=null){
      try{
         out.close();
      }catch(Exception e){}
    }

}



}
