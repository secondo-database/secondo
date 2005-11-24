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

public class CSV2Secondo{


public boolean printType(String Line){
  //
  neededValues=0;
  StringTokenizer ST1 = new StringTokenizer(Line,"\t");
  Types= new Vector();
  Names = new Vector();
  while (ST1.hasMoreTokens()){
     String Attr = ST1.nextToken();
     StringTokenizer ST2 = new StringTokenizer(Attr," ");
     String Name="";
     String Type="";
     if(ST2.hasMoreTokens())
        Name = ST2.nextToken().trim();
     else
        error("no name in attr:"+Attr);
     if(ST2.hasMoreTokens())
        Type = ST2.nextToken().trim().toLowerCase();
     else
        error("Missing Type in Attr"+Attr);      
     if( !Type.equals("int") & !Type.equals("text") & !Type.equals("real") &
          !Type.equals("point") & !Type.equals("string") && !Type.equals("spoint")){
           error("unknown type "+Type);
     }
     neededValues++;
     Types.add(Type);
     Names.add(Name);
     if(Type.equals("point") || Type.equals("spoint")) // this types need two values
         neededValues++;          
  }
  // print the type
  System.out.println("(rel (tuple ( ");
  for(int i=0;i<Types.size();i++){
     if(Types.get(i).equals("spoint"))
        System.out.println("    ("+Names.get(i)+" point )");
     else
        System.out.println("    ("+Names.get(i)+" "+Types.get(i)+" )");   
  }
  System.out.println("    )))");
  Values = new Vector(Types.size());
  return true;
  
}

private void printScheme(){
   System.err.println("Scheme:");
   for(int i=0;i<Types.size();i++)
      System.err.println(i+"  "+Types.get(i)+"    : "+Names.get(i));
   System.err.println("-------------------");   

}


/**
  converts a single line into a SECONDO relation tuple.
*/
private boolean printLine(String Line){
  if(Line==null)
     return false;
  
  // first collect all values in the appropriate String
  MyStringTokenizer ST = new MyStringTokenizer(Line,Separators.charAt(0));
  Values.clear();
  while(ST.hasMoreTokens())
        Values.add(ST.nextToken());
        
  if(Values.size()<neededValues){
       printScheme();
       error("too little values for the given scheme in Line \n"+Line);
  }     
  if(!allowMoreValues && Values.size()>neededValues){
     printScheme();
     System.err.println("Values="+Values);
     System.err.println("neededValue ="+neededValues);
     System.err.println("foundValues ="+Values.size());
  
     error("too much values for the given schema in line \n");
  }   
        
  
  System.out.print(" ( "); // open the tuple
  int ValuePos = 0;
  String Value1,Value2;
  for(int i=0;i<Types.size();i++){
      String Type = (String) Types.get(i);
      if(Type.equals("point")|| Type.equals("spoint")){ // we need the next entry
         Value1 = ((String)Values.get(ValuePos)).trim();
         ValuePos++;
         Value2 = ((String) Values.get(ValuePos)).trim();
         ValuePos++;
         if(Type.equals("point"))
             System.out.print("("+Value1+" "+Value2+" ) ");
         else
             System.out.print("("+Value2+" "+Value1+" ) ");    
         if(checknumeric){
            try{
               Double.parseDouble(Value1);
              Double.parseDouble(Value2);
           }catch(Exception e){
               error("error in numric check for coordinate of a point in Line \n"+Line);          
           }
        }
      }else if(Type.equals("int")){
         Value1 = ((String)Values.get(ValuePos)).trim();
         ValuePos++;
         System.out.print(" "+Value1+" ");
         if(checknumeric){
             try{
                Integer.parseInt(Value1);
             }
             catch(Exception e){
                error("wrong format for an integer in Line\n"+Line);
             }   
         }
      }   
      else if(Type.equals("string")){
        Value1=(String)Values.get(ValuePos);
        ValuePos++;
        if(Value1.length()>MAXSTRINGLENGTH){
             System.err.println("Warning: String too long");
             System.err.println("Line :"+Line);
             System.err.println("Attr :"+Names.get(i));
        }       
         System.out.print("\""+Value1+"\"");
      }   
      else if(Type.equals("text")){
         Value1 = (String)Values.get(ValuePos);
         ValuePos++;
         System.out.print(" (<text>"+Value1+"</text--->) ");
      }   
      else if(Type.equals("real")){
        Value1=((String)Values.get(ValuePos)).trim();
        ValuePos++;
        if(checknumeric){
             try{
                Double.parseDouble(Value1);
             }
             catch(Exception e){
                error("wrong format for an integer in Line\n"+Line);
             }   
         }

         System.out.print(" "+Value1+" ");
      }   
      else{
           error("unknow type found : "+Type);
      }
  }
  System.out.println(" )");  // close tuple
  return true;
}


private static void error(String Message){
   System.err.println(Message);
   System.exit(1);
}

public void convert(String CfgFile,String SourceFile){
   Properties Cfg = new Properties();
  try{
     FileInputStream CfgIn = new FileInputStream(CfgFile);
     Cfg.load(CfgIn);
     CfgIn.close();
  } catch(Exception e){
      error("Error in loading configuration");
  }
  
  Enumeration Keys = Cfg.propertyNames();
  boolean SeparatorFound = false;
  boolean SchemeFound = false;
  boolean ObjectFound = false;
  String Key;
  // check for Separator and Scheme key
  while(Keys.hasMoreElements()){
     Key = (String) Keys.nextElement();
     if(Key.equals("Separator"))
        SeparatorFound =true;
     if(Key.equals("Object"))
        ObjectFound =true;
     if(Key.equals("Scheme"))
        SchemeFound = true;   
  }
  if(!SeparatorFound)
     error("Missing Separator entry in config file");
  if(!SchemeFound)
     error("Missing Scheme entry in config file");   
  
  Separators = Cfg.getProperty("Separator");
  if(Separators.equals(""))
     error("empty separator specification in configuration file ");
     
  String Scheme = Cfg.getProperty("Scheme");
  if(Scheme.equals(""))
     error("empty scheme specification in configuration file ");
  String NoLines = Cfg.getProperty("IgnoreLines");
  
  String OName = "CSV_Obj";
  if (ObjectFound)
  {
    OName = Cfg.getProperty("Object");
    if(OName.equals(""))
       error("empty object specification in configuration file ");
  }

  LinesToIgnore = new Vector();
  if(NoLines!=null){
      StringTokenizer ST = new StringTokenizer(NoLines,",");
      LinesToIgnore.add(ST.nextToken().trim());
  }
  String Comment = Cfg.getProperty("Comment");
  checknumeric = Cfg.getProperty("checknumeric")!=null;
  allowMoreValues = Cfg.getProperty("allowMoreValues")!=null;
    
  try{
      BufferedReader CSVin = new BufferedReader(new FileReader(SourceFile));
   
      if (! ObjectFound)
      {
        int iodot=SourceFile.indexOf(".");
        OName = SourceFile;
        if(iodot>0)
          OName = SourceFile.substring(0,iodot);
      }

      boolean ok = true;
      System.out.println("( OBJECT "+OName+" () "); // open object
      if(!printType(Scheme)){
        error("type analyse failed");
      }
      System.out.println("("); // open value list , tuple
      int LNo = 0;
      String Line;
      while(CSVin.ready() ){
        Line = CSVin.readLine();
	if(LinesToIgnore==null || !LinesToIgnore.contains(""+new Integer(LNo))){
            if(Comment==null || !Line.startsWith(Comment)){
               ok = printLine(Line);
               if(!ok){
                  System.err.println("Error in processing line "+LNo+" = "+Line);
	       }
           }    
        }
        LNo++;
        if((LNo%5000)==0)
           System.err.print(".");    
      }
      System.out.println(") () )"); // close valueList and object
      CSVin.close();
  }catch(Exception e){
    e.printStackTrace();
  }
}


public static void main(String[] args){
  
  if(args.length<2){
     error("missing parameter\nusage: java CVS2Secondo ConfigFile SourceFile [>TargetFile]");
  }
  File CfgFile = new File(args[0]);
  if(!CfgFile.exists()){
     error("Config file not found");
  }
  File SourceFile = new File(args[1]);
  if(!SourceFile.exists())
      error("Source file not found ");
  CSV2Secondo C = new CSV2Secondo();
  C.convert(args[0],args[1]);      
}

private static final int INTEGER=0;
private static final int TEXT=1;
private static final int STRING=2;
private static final int POINT=3;
private static final int FLOAT=4;
private static final int SPOINT=5;
private static final int MAXSTRINGLENGTH=48;
private Vector Types;
private Vector Names;
private String Separators;
private Vector LinesToIgnore;
private boolean checknumeric;
private Vector Values;
private boolean allowMoreValues;
private int neededValues;

private static class MyStringTokenizer{

public MyStringTokenizer(String S,char delim){
  MyString = S;
  this.delim=delim;
}

public boolean hasMoreTokens(){
  return MyString.length()>0;
}

public String nextToken(){
  if(MyString.length()==0) return "";


  int index = MyString.indexOf(delim);
  if (index<0){
    String res = MyString;
    MyString="";
    return res;
  } else{
    String res = MyString.substring(0,index);
    MyString = MyString.substring(index+1);
    return res;
  }

}

private String MyString;
private char delim;

}




}



