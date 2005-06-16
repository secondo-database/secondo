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

/** This tools extracts keywords from a file.
  * To do this all different words included in this file which
  * are not contained in another file containing excludings are
  * printed out. Additionally the minimal length of words can
  * be specified.
  */

public class KeyWordCreator{

/** shows the parameters and exits the programm
  */

private static void showUsage(String Message){
   System.err.println(Message);
   System.err.println("Usage: java KeyWordCreator Source Exclusions Target LineWord [MinimalLength]");
   System.err.println("If LineWord is equals to - , a Relation (DocID Word) is created");
   System.err.println("Otherwise two Relations (wordid word) and (wordid docid)");
   System.err.println("are created");
   System.exit(1);
}



private static void createTwoRelations(TreeSet Exclusions,String SourceFile,
                                       String WordTarget, String RelTarget, int mlength){

 String Line;
 TreeSet Words = new TreeSet();
 TreeSet LineWordRel = new TreeSet();
 int LineNo = 1;
 int IDNo = 1;
 try{
    BufferedReader BR = new BufferedReader(new FileReader(SourceFile));
    while(BR.ready()){
       Line = BR.readLine();
       if(Line!=null){
           if(LineNo % 5000 ==0){
              System.out.println("Process Line Number "+LineNo);
           }
           int pos = Line.indexOf(" ");
           String IDStr;
           if(pos>0){
               IDStr = Line.substring(0,pos);
               Line = Line.substring(pos);
           } else
               IDStr = "-1";  // invalid docid
           StringTokenizer ST = new StringTokenizer(Line.toLowerCase(),DELIM);
           String Token;
           while(ST.hasMoreTokens()){
               Token = ST.nextToken();
               if((Token.length()>=mlength) &&
                  !Exclusions.contains(Token)){
                  WordWithID wwi = new WordWithID();
                  wwi.word = Token;
                  wwi.id = 0;
                  if(Words.contains(wwi)){ // old keyword
                     WordWithID Existing = (WordWithID) Words.tailSet(wwi).first();
                     LWR lwr = new LWR();
                     lwr.LineNo = IDStr;
                     lwr.WordID = Existing.id;
                     LineWordRel.add(lwr);
                  } else{ // new keyWord found
                     IDNo++;
                     wwi.id = IDNo;
                     Words.add(wwi);
                     LWR lwr = new LWR();
                     lwr.LineNo = IDStr;
                     lwr.WordID = IDNo;
                     LineWordRel.add(lwr);
                     if(IDNo % 5000 ==0)
                        System.out.println("Process Wordnumber "+IDNo);
                  }
               }
           }
           LineNo++;
       }
    }
    BR.close();
 }catch(Exception e){
    e.printStackTrace();
    System.exit(1);
 }

 System.out.println("Relations created");
 System.out.println(LineNo+" documents scanned");
 int Size1 = Words.size();
 int Size2 = LineWordRel.size();
 System.out.println(Size1+ "Words   ;\n"+Size2+" References between lines and words");
 int o1 = Size1 / 10;
 // write the word-id-Relation to File
 try{
    Writer out = new OutputStreamWriter(new FileOutputStream(WordTarget));
    out.write("( OBJECT "+WordTarget+" ()\n"); // open object
    out.write("   (rel (tuple ( (id int)(word string))))\n"); // type
    out.write("   (\n"); // open value
    Iterator it = Words.iterator();
    int i=0;
    while(it.hasNext()){
       out.write("       "+it.next()+"\n");
       if(i++%o1==0)
          System.out.println(i+" words written");
    }
    out.write("   )\n"); // close value
    out.write("())\n"); // close object
    out.close();
 } catch(Exception e){
    showUsage("Error in writing file ");
 }

 // write the line-id-Relation to File
 int o2 = Size2/10;
 try{
    Writer out = new OutputStreamWriter(new FileOutputStream(RelTarget));
    out.write("( OBJECT "+RelTarget+" ()\n"); // open object
    out.write("   (rel (tuple ( (docid int)(wordid int))))\n"); // type
    out.write("   (\n"); // open value
    Iterator it = LineWordRel.iterator();
    int i=0;
    while(it.hasNext()){
       out.write("       "+it.next()+"\n");
       if(i++%o2==0)
          System.out.println(i+" word-line relationsships written");
    }
    out.write("   )\n"); // close value
    out.write("())\n"); // close object
    out.close();
 } catch(Exception e){
    showUsage("Error in writing file ");
 }
}


private static void createSingleRelation(TreeSet Exclusions,String SourceFile,
                                         String TargetFile, int mlength){

 TreeSet KeyWords = new TreeSet(); // stores keyWords of as single line
 int LineNo =1;
 String Line;
 try{
    BufferedReader BR = new BufferedReader(new FileReader(SourceFile));
    Writer out = new OutputStreamWriter(new FileOutputStream(TargetFile));
    out.write("( OBJECT "+TargetFile+" ()\n"); // open object
    out.write("   (rel (tuple ( (docid int)(word string))))\n"); // type
    out.write("   (\n"); // open value
    while(BR.ready()){
       Line = BR.readLine();
       if(Line!=null){
           if(LineNo % 5000 ==0){
              System.out.println("Process Line Number "+LineNo);
           }
           // get the documentid 
           int pos = Line.indexOf(" ");
           String DocIDString;
           if(pos>0){
              DocIDString = Line.substring(0,pos);
              Line = Line.substring(pos); 
           } else
              DocIDString = "-1";
           StringTokenizer ST = new StringTokenizer(Line.toLowerCase(),DELIM);
           String Token;
           KeyWords.clear(); // remove old Words
           while(ST.hasMoreTokens()){
               Token = ST.nextToken();
               if((Token.length()>=mlength) &&
                  !Exclusions.contains(Token)){
                    KeyWords.add(Token);
                }
           }
           // write keywords from this line to the relation
           Iterator it = KeyWords.iterator();
           while(it.hasNext()){
              out.write("     ( "+DocIDString+" \""+it.next()+"\" )\n");
           }
           LineNo++;
        }
    }
    BR.close();
    out.write("   )\n"); // close value
    out.write("())\n"); // close object
    out.close();
  } catch(Exception e){
    e.printStackTrace();
    System.err.println("Error in Converting SourceFile "+SourceFile);
    System.exit(0);
  }
}

public static void main(String[] args){

 if(args.length!=4 && args.length!=5 )
     showUsage("Wrong number of parameters");

 File F = new File(args[0]);
 if(!F.exists())
     showUsage("File not found");

 File E = new File(args[1]);
 if(!E.exists())
     showUsage("Exclusionsfile not found");

  int mlength=0;
  if(args.length==5){
    try{
       mlength = Integer.parseInt(args[4]);
    }catch(Exception e){
       showUsage("minimal length is not an integer");
    }
 }

 String Line;
 // first we insert all words to exclude
 TreeSet Exclusions = new TreeSet();
 try{
    BufferedReader BR = new BufferedReader(new FileReader(E));
    while(BR.ready()){
       Line = BR.readLine();
       if(Line!=null){
           StringTokenizer ST = new StringTokenizer(Line.toLowerCase(),DELIM);
           String Token;
           while(ST.hasMoreTokens()){
              Token = ST.nextToken();
              if(Token.length()>=mlength)
                 Exclusions.add(Token);
           }
       }
    }
    BR.close();
 }catch(Exception e){
    e.printStackTrace();
    System.exit(1);
 }

  System.out.println("Start Conversion with:");
  if(!args[3].equals("-")){
     System.out.println("Source-File = "+args[0]);
     System.out.println("Ex-File = "+args[1]);
     System.out.println("Word-File = "+args[2]);
     System.out.println("Word-Doc File = "+args[3]);
     System.out.println("MLength  = "+mlength);
     createTwoRelations(Exclusions,args[0],args[2],args[3],mlength);
  } else{
     System.out.println("Source-File = "+args[0]);
     System.out.println("Ex-File = "+args[1]);
     System.out.println("Single-Word-File = "+args[2]);
     System.out.println("MLength  = "+mlength);
     createSingleRelation(Exclusions,args[0],args[2],mlength);
  }
}// main

private static class LWR implements Comparable{
    String LineNo;
    int WordID;
    public boolean equals(Object o){
      return compareTo(o)==0;
    }
    public int compareTo(Object o){
      if(! (o instanceof LWR))
         return -1;
      LWR lwr = (LWR)o;
      if(WordID<lwr.WordID)
         return -1;
      if(WordID>lwr.WordID)
         return 1;
      if(LineNo.compareTo(lwr.LineNo)<0)
         return -1;
      if(LineNo.compareTo(lwr.LineNo)>0)
         return 1;
      return 0;
    }
    public String toString(){
       return "( "+LineNo+" "+WordID+" )";
    }
    public int compare(Object o){
       return compareTo(o);
    }

}

private static class WordWithID implements Comparable{
   String word;
   int id;
   public boolean equals(Object o){
      return compareTo(o)==0;
   }
   public int compare(Object o){
       return compareTo(o);
    }
   public int compareTo(Object o){
      if(! (o instanceof WordWithID))
        return -1;
      WordWithID wwi = (WordWithID) o;
      return word.compareTo(wwi.word);
   }
   public String toString(){
      return "( "+id+"  \""+word+"\" )";
   }
}

private static class Word_DocID implements Comparable{
   String word;
   String docid;
   public boolean equals(Object o){
      return compareTo(o)==0;
   }
   public int compare(Object o){
       return compareTo(o);
    }
   public int compareTo(Object o){
      if(! (o instanceof Word_DocID))
        return -1;
      Word_DocID wd = (Word_DocID) o;
      if(docid.compareTo(wd.docid)<0)
         return -1;
      if(docid.compareTo(wd.docid)>0)
         return 1;
      return word.compareTo(wd.word);
   }
   public String toString(){
      return "( "+docid+"  \""+word+"\" )";
   }
}




// all included chars are used to speparate words
private static final String DELIM = " \t\n\r\f,;:\".(){}?!'`¸&$%[]=#<>\\/*+~#-0123456789^@_";



}
