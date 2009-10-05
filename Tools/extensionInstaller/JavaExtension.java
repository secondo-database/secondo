/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

import java.util.*;
import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;
import java.util.zip.*;
import java.util.regex.Pattern;

/** Superclass for all Java extensions, i.e. viewer and display classes **/

public class JavaExtension extends SecondoExtension{

   protected int java_Major_Version = 0;     // version informtion
   protected int java_Minor_Version = 0;      // version information
   protected String mainClass = null;         // name of the main class

   protected Vector<StringBoolPair> libDeps = new Vector<StringBoolPair>(); // dependencies to libraries


  /** Extracts the version information from the xml file **/
  boolean readJavaVersion(Node n1){
    NodeList nl = n1.getChildNodes();
    for(int i=0;i<nl.getLength(); i++){
       Node n = nl.item(i);
       String name = n.getNodeName();
       if(name.equals("Major")){
          if(n.hasChildNodes()){
             String  a = n.getFirstChild().getNodeValue().trim();
             java_Major_Version=Integer.parseInt(a.trim());
          } 
       } else if(name.equals("Minor")){
          if(n.hasChildNodes()){
             String  a = n.getFirstChild().getNodeValue().trim();
             java_Minor_Version=Integer.parseInt(a.trim());
          } 
       } else if(!name.equals("#text") && !name.equals("#comment")){
           System.err.println("Unknown version information found" + name);
       }
    }
    return true;
  }


  /** Checks whether to current java version is at least this required by this extension **/
  protected boolean checkJavaVersion(){
     String V = System.getProperty("java.version");
     int cv_major= 0;
     int cv_minor= 0;
     StringTokenizer st = new StringTokenizer(V,".");
     if(st.hasMoreTokens()){
       String mv = st.nextToken();
       if(mv.matches("[0-9]+")){
         cv_major = Integer.parseInt(mv);
       } else {
         System.err.println("invalid version information " + mv);
         return false;
       } 
     }
     if(st.hasMoreTokens()){
       String mv = st.nextToken();
       if(mv.matches("[0-9]+")){
         cv_minor = Integer.parseInt(mv);
       } else {
         System.err.println("invalid version information " + mv);
         return false;
       }
     }
     
     if(java_Major_Version > cv_major ||
        (java_Major_Version == cv_major &&
         java_Minor_Version > cv_minor)){
         System.err.println("Java version" +
                            getJavaVersionString()+" required, but found  Java version " + V);
         return false;
       }
     return true;
  }

  /** Checks whether the libraries are present or provided by the extension **/
  protected boolean checkLibDeps(String secondoDir){
     String s = File.separator;
     char sc = File.separatorChar;
     String libDir = secondoDir+s+"lib"+s;
     for(int i=0;i<libDeps.size();i++){
        StringBoolPair e = libDeps.get(i);
        if(!e.firstB){ // dependency to a non provided lib
           String fn = libDir + e.secondS.replace('/',sc) + s +  e.firstS;
           File f = new File(fn);
           if(f.exists()){
              System.err.println("lib " + fn +" required but not found");
              return false;
           }
        }
     }
     return true;
  }

  /** converts the java version information to a string **/
  protected String getJavaVersionString(){
     return "" + java_Major_Version + "."+  java_Minor_Version;
  }


  /** Modifies the makefile.inc, i.e. adds libraries to the classpath used during compiling **/
  protected boolean modifyMakeFileInc(String secondoDir){
     // extract the lib names to add to makefile.inc
     Vector<StringBoolPair> libext = new Vector<StringBoolPair>();
     for(int i=0;i<libDeps.size();i++){
         StringBoolPair p = libDeps.get(i);
         if(p.secondB){
            libext.add(p);
         }
     }
     if(libext.size()==0){
        return true;
     }


     // appends libraries to makefile.inc
     String s = File.separator;
     File f = new File(secondoDir + s + "Javagui"+s+"makefile.inc");
     boolean ok = true;
     if(!f.exists()){
        System.err.println("Javagui/makefile.inc not found, check your Secondo installation");
        return false;
     }
     BufferedReader in = null;
     PrintWriter out = null;
     int pos = -1;
     try{
        in = new BufferedReader(new FileReader(f));
        String content = "";
        while(in.ready()){
           String line = in.readLine(); 
           if(!line.matches("\\s*JARS\\s*:=.*")){
              content += line +"\n";
           } else {
              content += line +"\n";
              pos = content.length();
              for(int i=libext.size()-1;i>=0;i--){
                 if(line.matches(".*"+libext.get(i).firstS+"\\s*")){
                   System.out.println("Lib " + libext.get(i).firstS+"already included in makefile.inc");
                   libext.remove(i);
                 }
              } 
           }
        }

        try{in.close(); in=null;}catch(Exception e){}

        if(libext.size()==0){
            return true;
        }
        String block = "";
        for(int i=0;i<libext.size();i++){
           StringBoolPair p = libext.get(i);
           String subdir = p.secondS.equals(".")?"":p.secondS;
           block += "JARS := $(JARS):$(LIBPATH)/"+subdir+p.firstS+"\n";
        }
        if(pos<0){
          content += block; 
        } else {
           String p1 = content.substring(0,pos);
           String p2 = content.substring(pos,content.length());
           content = p1+block+p2;
        }
        out = new PrintWriter(new FileOutputStream(f));
        out.print(content);
     }catch(Exception e){
       e.printStackTrace();
       ok = false;
     }finally{
       if(in!=null) try{in.close();}catch(Exception e){}
       if(out!=null) try{out.close();}catch(Exception e){}
     }
     return ok;
  }

  /** Reads the dependencies from n1 **/
  protected boolean readDependencies(Node n1){
    NodeList nl = n1.getChildNodes();
    for(int i=0;i<nl.getLength();i++){
      Node n = nl.item(i);
      String name = n.getNodeName();
      if(name.equals("SecondoVersion")){
         readSecondoVersion(n);
      } else if(name.equals("JavaVersion")){
        readJavaVersion(n);
      } else if(name.equals("Library")){
         NamedNodeMap m = n.getAttributes();
         Node file = m.getNamedItem("file");
         if(file==null){
           System.err.println("file missing for hoese.dependencies.library");
           return false;
         }       
         StringBoolPair entry = new StringBoolPair();
         entry.firstS = file.getNodeValue().trim();
         Node loc = m.getNamedItem("location");
         if(loc==null){
            System.err.println("location missing in hoese.dependencies.library");
            return false;
         }
         entry.secondS = loc.getNodeValue().trim();
         Node provided = m.getNamedItem("provided");
         if(provided==null){
           entry.firstB = false;
         } else {
           String p = provided.getNodeValue().trim().toLowerCase();
           entry.firstB = p.equals("yes");
         }
         Node classpath = m.getNamedItem("classpath");
         if(classpath==null){
           entry.secondB=false;
         } else {
           String p = classpath.getNodeValue().trim().toLowerCase();
           entry.secondB=p.equals("true");
         }
         libDeps.add(entry);
      }  else if(!name.equals("#text") && !name.equals("#comment")){
        System.err.println("unknown node found in hoese.library section : " + name );
      }
    }
    return true;
  }


}


