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

import java.io.*;
import java.util.*;

/**
This class converts the open-geo-db
file (txt version) into a Secondo database.
The output is written to std-out.
Unfortunately, the dataformat for the
open-geo-db has been changed. So i'm not sure,
that this converter works with other versions of this
data.
**/
public class Converter{



/*
  prints the header of a database definition
*/
private static void printDBProlog(){
	System.out.println("(DATABASE OPENGEODB");
  if(oldStyle){
     System.out.println("    (DESCRIPTIVE ALGEBRA)");
     System.out.println("    (TYPES)");
     System.out.println("    (OBJECTS)");
	   System.out.println("    (EXECUTABLE ALGEBRA)");
  }
	System.out.println("    (TYPES)");
	System.out.println("    (OBJECTS");
        System.out.println("       (OBJECT License () text ");
        System.out.println("          <text>");
        System.out.println("OpenGeoDB");
        System.out.println("Copyright (c) 2003 OpenGeoDB-Team");
        System.out.println("For more information visit: http://www.opengeodb.de");
        System.out.println("This program is free software. You can redistribute");
        System.out.println("it and/or modify it under the terms of the ");
        System.out.println("GNU Lesser General Public License (LGPL) ");
        System.out.println("as published by the Free Software Foundation; ");
        System.out.println("either version 2 of the License.\n");
        System.out.println("    2004-04-24 ");     
        System.out.println("\n </text---> ");
        if(oldStyle) System.out.print("()");
        System.out.println(" )\n");
}

/*
   finished a database definition
*/
private static void printDBEpilog(){
	System.out.println("    )");
	System.out.println(")");
}

/*
  prints out the message and exists the program
*/
private static void error(String Message){
   System.err.println(Message);
   System.exit(-1);
}

/*
  this functions returns LAND if the String Abk.rzungen Bundesl.ander is found
  otherwise SEARCHLAND is returned
*/
private static int searchLand(String Line){
  if(Line.startsWith("#") &&
     Line.indexOf("Abk")>0 &&
     Line.indexOf("rzungen ")>0 &&
     Line.indexOf("Bundesl")>0 &&
     Line.indexOf("nder")>0)
     return LAND;
  return SEARCHLAND;
}


/*
  this funtion write a single tuple of the Bundeslaender relation
*/
private static int land(String Line){
   if(Line.indexOf("kennzeichen")>0)
       return SEARCHKFZ;
   StringTokenizer ST = new StringTokenizer(Line," ");
   tmpS.clear();
   while(ST.hasMoreTokens())
      tmpS.add(ST.nextToken().trim());

   if(tmpS.size()<3)
     return LAND;
   System.out.print("(");    // open tupel
   System.out.print("\""+tmpS.get(1)+"\" ");
   System.out.print("\"");
   for(int i=2;i<tmpS.size();i++){
      if(i>2) System.out.print(" ");
      System.out.print(tmpS.get(i));
   }
   System.out.println("\")");
   return LAND;
}

/*
  this function search fro "kennzeichen" in Line
*/
private static int searchKFZ(String Line){
   if(Line.indexOf("kennzeichen")>0)
      return KFZ;
   return SEARCHKFZ;
}

/*
 prints a single tuple of the Kennzeichen relation
*/
private static int kfz(String Line){
   if(Line.indexOf("#####")>=0){
      return SEARCHORT;
   }
   StringTokenizer ST = new StringTokenizer(Line," ");
   tmpS.clear();
   while(ST.hasMoreTokens())
      tmpS.add(ST.nextToken().trim());
   if(tmpS.size()<2)
      return KFZ;
   System.out.print("(\""+tmpS.get(1)+"\" "+"<text>");
   for(int i=2;i<tmpS.size();i++){
      if(i>2) System.out.print(" ");
      System.out.print(tmpS.get(i));
   }
   System.out.println("</text---> )");
   return KFZ;
}

/*
  prints the header of the Orte relation
*/
private static int searchOrt(String Line){
   if(!Line.startsWith("#")){
      System.out.println("    (OBJECT Orte");
	 System.out.println("       ()");
	 System.out.println("       (rel ");
	 System.out.println("         (tuple");
	 System.out.println("           (");
	 System.out.println("             (key int)");
         System.out.println("             (Staat string)");
	 System.out.println("             (Bundesland string)");
	 System.out.println("             (Regierungsbezirk string)");
	 System.out.println("             (Landkreis text)");
	 System.out.println("             (Verwaltungszusammenschluss text)");
	 System.out.println("             (Ort string)");
	 System.out.println("             (Ortsteil string)");
	 System.out.println("             (Gemeindeteil string)");
	 System.out.println("             (Position point)");
	 System.out.println("             (Kzeichen string))))");
	 System.out.println("        (");
	 return ORT;
   }
   return SEARCHORT;
}

/*
  prints out a single tupe of the Orte Relation
*/
private static int processOrt(String Line){
   if(Line.startsWith("#"))
      return ORT;
   StringTokenizer ST = new StringTokenizer(Line,";");
   tmpS.clear();
   while(ST.hasMoreTokens())
      tmpS.add(ST.nextToken().trim());
   if(tmpS.size()<13){
     if(tmpS.size()>2)
        System.err.println("error in reading Ort "+Line);
      return ORT;
   }
   try{
      Double.parseDouble((String)tmpS.get(9));
      Double.parseDouble((String)tmpS.get(10));
   } catch(Exception e){
      System.err.println("wrong coordinates in: "+ Line);
      return ORT;
   }
   System.out.print(" (");
   System.out.print(tmpS.get(0)+" "); // key
   System.out.print("\""+tmpS.get(1)+"\" "); // staat
   System.out.print("\""+tmpS.get(2)+"\" "); // land
   System.out.print("\""+tmpS.get(3)+"\" "); // Regierungsbezierk
   System.out.print("<text>"+tmpS.get(4)+"</text---> "); // Landkreis
   System.out.print("<text>"+tmpS.get(5)+"</text---> "); // Verwaltungsuzusammenschluss
   System.out.print("\""+tmpS.get(6)+"\" "); // Ort
   System.out.print("\""+tmpS.get(7)+"\" "); // Ortsteil
   System.out.print("\""+tmpS.get(8)+"\" "); // Gemeindeteil
   System.out.print(" ("+tmpS.get(10)+" "+tmpS.get(9)+") "); // Koordinaten
   System.out.println("\""+tmpS.get(11)+"\" )"); // Kennzeichen

   // check for lengths
   boolean err = false;
   if( ((String)tmpS.get(1)).length() > MAXLENGTH){
      err = true;
      System.err.println("Staat too long");
   }
   if( ((String)tmpS.get(2)).length() > MAXLENGTH){
      err = true;
      System.err.println("Land too long");
   }
   if( ((String)tmpS.get(3)).length() > MAXLENGTH){
      err = true;
      System.err.println("Regierungsbezirk too long");
   }
   if( ((String)tmpS.get(6)).length() > MAXLENGTH){
      err = true;
      System.err.println("Ort too long");
   }
   if( ((String)tmpS.get(7)).length() > MAXLENGTH){
      err = true;
      System.err.println("Ortsteil too long");
   }
   if( ((String)tmpS.get(8)).length() > MAXLENGTH){
      err = true;
      System.err.println("Gemeindeteil too long");
   }
   if( ((String)tmpS.get(11)).length() > MAXLENGTH){
      err = true;
      System.err.println("Kennzeichen too long");
   }
   if(err){
     System.err.println("Error in Line "+Line);
   }
   Vector PLZs = new Vector();
   StringTokenizer PLZST = new StringTokenizer((String)tmpS.get(12),",.");
   while(PLZST.hasMoreTokens())
       PLZs.add(PLZST.nextToken().trim());
   PLZTuple PT = new PLZTuple();
   PT.Ident = Integer.parseInt((String)tmpS.get(0));
   PT.Plzs = PLZs;
   PLZRel.add(PT);
   return ORT;
}

/*
  prints out the PLZ relation stored in the TreeSet PLZRel
*/
private static void printPLZ(){
  System.out.println("    (OBJECT PLZ");
  System.out.println("       ()");
  System.out.println("       (rel ");
  System.out.println("         (tuple");
  System.out.println("           (");
  System.out.println("             (key int)");
  System.out.println("             (plz int))))");
  System.out.println("        (");
  Iterator i = PLZRel.iterator();
  while(i.hasNext()){
     PLZTuple T = (PLZTuple) i.next();
     for(int j=0;j<T.Plzs.size();j++){
         boolean err = false;
	 try{
            Integer.parseInt((String)T.Plzs.get(j));
	 }catch(Exception e){ err=true;}
	 if(err){
            System.err.println("wrong plz in city whith id "+T.Ident);
            System.out.println("  ( "+T.Ident+" -1 ) ");
	 }else
           System.out.println("  ( "+T.Ident+" "+T.Plzs.get(j)+" ) ");
     }
  }
  if(oldStyle)
      System.out.println("        ) () )");
  else
      System.out.println("        )  )");

}

/*
  distributes  Line to the appropriate method
*/
private static int processLine(String Line, int Mode){
   if(Mode==SEARCHLAND){
      Mode= searchLand(Line);
      if(Mode!=SEARCHLAND){  // Laeder found, print objectprolog
         System.out.println("    (OBJECT Bundeslaender");
	 System.out.println("       ()");
	 System.out.println("       (rel ");
	 System.out.println("         (tuple");
	 System.out.println("           (");
	 System.out.println("             (Ident string)");
         System.out.println("             (Name string))))");
	 System.out.println("        (");
      }
   } else
      if(Mode==LAND){
         Mode=land(Line);
         if(Mode!=LAND){
             if(oldStyle)
                System.out.println("       ) () )");
             else
                System.out.println("       )  )");

         }
   }
   if(Mode==SEARCHKFZ){
      Mode=searchKFZ(Line);
      if(Mode!=SEARCHKFZ){
         System.out.println("    (OBJECT Kennzeichen");
	 System.out.println("       ()");
	 System.out.println("       (rel ");
	 System.out.println("         (tuple");
	 System.out.println("           (");
	 System.out.println("             (Kzeichen string)");
         System.out.println("             (Landkreis text))))");
	 System.out.println("        (");
      }
   } else
   if(Mode==KFZ){
      Mode=kfz(Line);
      if(Mode!=KFZ){
         if(oldStyle) 
             System.out.println("      ) () )");
         else
             System.out.println("      )  )");
      }
   }
   if(Mode==SEARCHORT){
     Mode=searchOrt(Line);
   }
   if(Mode==ORT){
      Mode=processOrt(Line);
   }
   return Mode;
}


/*
  main function for converting the opengeodb to a secondo database
*/
public static void main(String[] args){
  int start=0;
  if(args.length>0 && args[0].equals("--oldstyle")){
     oldStyle=true;
     start++;
  }
  if(args.length<start+1)
     error("missing filename");
  File F = new File(args[start]);
  if(!F.exists())
     error("File not found");
  try{
     BufferedReader in = new BufferedReader(new FileReader(F));
     String Line;
     printDBProlog();
     int Mode = SEARCHLAND;
     while(in.ready() && Mode!=STOP){
       Line = in.readLine();
       Mode = processLine(Line,Mode);
     }
     if(Mode!=STOP);
       if(oldStyle){
          System.out.println("     ) () )"); // close value and object for cities
       } else{
          System.out.println("     )  )");
       }
     if(Mode!=STOP)
        printPLZ();
     in.close();
     printDBEpilog();
  } catch(Exception e){
    e.printStackTrace();
    error("error while reading file");
  }
}

/*
  aid class for storing a identificator with a set of plzs
*/
private static class PLZTuple implements Comparable{
   int Ident;
   Vector Plzs;

   public int compare(Object o){
      if(!(o instanceof PLZTuple))
         return -1;
      PLZTuple T = (PLZTuple) o;
      return Ident-T.Ident;
   }

   public int compareTo(Object o){
      return compare(o);
   }
}


private static final int SEARCHLAND = 0;
private static final int LAND = 1;
private static final int SEARCHKFZ=2;
private static final int KFZ=3;
private static final int SEARCHORT=4;
private static final int ORT=5;
private static final int STOP=6;
private static Vector tmpS = new Vector(20); // Vector containing the attribute of a single line
private static TreeSet PLZRel=new TreeSet();
private static boolean oldStyle=false;

private static final int MAXLENGTH = 48;


}
