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


/** The class HoeseInfo corresponds to a set of display classes to install **/
public class HoeseInfo extends JavaExtension{


   /** Conversion into a string representation **/   
   public String toString(){
     String res = "[HoeseInfo: " + 
                         " SecondoVersion = " + secondo_Major_Version + "."
                                               + secondo_Minor_Version + "."
                                               + secondo_SubMinor_Version +
                         ", JavaVersion = " + java_Major_Version + "."
                                               + java_Minor_Version  +
                         ", mainClass = "      + mainClass +
                         ", copyright = "      + copyright;
     res += "), files = (";
     for(int i=0;i<files.size();i++){
       if(i>0) res +=", ";
       res += files.get(i);
     }
     res += "), libDeps= (";
     for(int i=0;i<libDeps.size();i++){
       if(i>0) res +=", ";
       res += libDeps.get(i);
     }
     res +=")]";
     return res;
   }

 
  /** Creates a new HoeseInfoo object from an XML 
    * description.
    **/
   public HoeseInfo(Node n){
      valid = readHoeseInfo(n);
   }


   /** Checks wether all files are present in the zip file **/
   public boolean filesPresent(ZipFile f){
      Vector<String> names = new Vector<String>();
      String fold = folder==null?"":folder+"/";
      names.add(fold+mainClass);
      for(int i=0;i<files.size();i++){
         names.add(getEntryName(files.get(i)));
      }
      for(int i=0;i<libDeps.size();i++){
        StringBoolPair entry = libDeps.get(i);
        if(entry.firstB){ // lib is provided
           names.add(fold + entry.firstS);
        }
      }
      return filesPresent(f, names);
   }

   /** Reads the infoamtion from n1 **/
   private boolean readHoeseInfo(Node n1){
     NamedNodeMap nm = n1.getAttributes();
     if(!readFolder(nm.getNamedItem("folder"))){
        return false;
     }
     NodeList nl = n1.getChildNodes();
     for(int i=0;i<nl.getLength();i++){
        Node n = nl.item(i);
        String name = n.getNodeName();
        if(name.equals("Mainclass")){
           if(mainClass!=null){
              System.err.println("XML coruppted: only one Mainclass allowed.");
              return false;
           } 
           if(n.hasChildNodes()) {
              mainClass = n.getFirstChild().getNodeValue().trim(); 
           } else {
              System.err.println("XML corrupted: mainclass cannot be empty");
              return false;
           } 
           if(!mainClass.endsWith(".class") && !mainClass.endsWith(".java")) {
              System.err.println("Mainclas has to end with .java or .class");
              return false;
           }
        } else if(name.equals("Files")){
            readFiles(n);
        } else if(name.equals("Dependencies")){
            readDependencies(n);
        } else if(name.equals("Copyright")){
           if(copyright!=null){
              System.err.println("XML coruppted: only one copyright allowed.");
              return false;
           } 
           if(n.hasChildNodes()) {
              copyright = n.getFirstChild().getNodeValue().trim(); 
           } else {
              System.err.println("XML corrupted: copyright cannot be empty");
              return false;
           }  
            
        } else if(!name.equals("#text") && !name.equals("#comment")){
           System.out.println("Unknown node for a Hoese extension "+ name);
        }
     }
     return checkValidity();
   }

  /** Check whether all information is available **/
  boolean checkValidity(){
   if(secondo_Major_Version<0){
     System.err.println("version information missing");
     return false;
   }
   if(mainClass==null){
      System.err.println("mainclass missing");
      return false;
   }
   return true;
  }


  /** Checks whether all dependencies are fullfilled and no conflicts are present **/
  static boolean check(String secondoDir, Vector<HoeseInfo> infos){
    if(!checkConflicts(secondoDir,infos)){
       return false;  
    } 
    if(!checkDependencies(secondoDir,infos)){
      return false;
    }
    return true;
  }

  /** Checks for conflict between packages and conflict between pacjages 
    * and the installed system.
    **/ 
  static boolean checkConflicts(String secondoDir, Vector<HoeseInfo> infos){
    // first names of the display classes  must be disjoint
    TreeSet<String> files = new TreeSet<String>();
    String s = File.separator;
    char sc = File.separatorChar;
    String mainDir = secondoDir+s+"Javagui"+s;
    String algDir = mainDir+"viewer"+s+"hoese"+s+"algebras"+s;
    for(int i=0;i<infos.size();i++){
       HoeseInfo info = infos.get(i);
       // create filenames for all files to be installed
       // use relative filenames starting from the Javagui directory
       // mainclass
       String fn =  algDir + info.mainClass;       
       if(files.contains(fn)){
         System.err.println("Conflict: File " + fn + " found twice");
         return false;
       } 
       files.add(fn);
       // files
       for(int j=0;j<info.files.size();j++){
          StringTriple triple = info.files.get(j);
          String middle = triple.second==null?"":triple.second.replace('/',sc)+s;
          fn = algDir+middle + triple.first; 
          if(files.contains(fn)){
            System.err.println("Conflict: File " + fn + " found twice");
            return false;
          }
          files.add(fn);
       }
       // libDeps which should be installed
       for(int j=0;j<info.libDeps.size();j++){
         StringBoolPair e = info.libDeps.get(j);
         if(e.firstB){ // lib provided by the module
            fn = mainDir + "lib" + s + e.secondS.replace('/',sc) + s +  e.firstS;
            if(files.contains(fn)){
              System.err.println("Conflict: File " + fn + " found twice");
              return false;
            }
            files.add(fn);
         } 
       }
    }
    // the packages are pairwise conflict free

    // check for already installed files
    Iterator<String> it = files.iterator();
    while(it.hasNext()){
      String fn = it.next();
      File f = new File(fn);
      if(f.exists()){
         System.err.println("try to install file " + fn +" which is already installed");
         return false;
      }
    }
    return true;
  } // checkConflicts#


  /** Checks whether the dependencies are fullfilled */
  static boolean checkDependencies(String secondoDir, Vector<HoeseInfo> infos){
     TreeSet<String> providedLibs = new TreeSet<String>();
     for(int i=0;i<infos.size();i++){
       HoeseInfo info = infos.get(i);
       if(!info.checkJavaVersion()){
         return false;
       }
       for(int j=0;j<info.libDeps.size();j++){
           StringBoolPair e = info.libDeps.get(j);
           if(e.firstB){ // lib is provided
             providedLibs.add(e.firstS);
           }
       }

     }
     String s = File.separator;
     char sc = File.separatorChar;
     String mainDir = secondoDir+s+"Javagui"+s;
     for(int i=0;i<infos.size();i++){
        HoeseInfo info = infos.get(i);
        // check dependencies on libraries
        for(int j=0;j<info.libDeps.size();j++){
           StringBoolPair e = info.libDeps.get(j);
           if(!e.firstB && !providedLibs.contains(e.firstS)){ // dependency to a non provided lib
              String fn = mainDir + "lib" + s + e.secondS.replace('/',sc) + s +  e.firstS;
              File f = new File(fn);
              if(f.exists()){
                System.err.println("DisplayClass "+ info.mainClass +" tries to install the file "
                                    + fn + " which is already present.");
                return false;
              }
           }
        }
     }
   return true;
  }

  /** If there are additional libraries, the classpath used by the start script
    * is extended by these files.
    **/ 
  private boolean addToStartScript(File f){
    // check whether the file is to mofify, i.e. if libs are required
    Vector<String> libFlags = new Vector<String>();
    for(int i=0;i<libDeps.size();i++){
        StringBoolPair p = libDeps.get(i);
        if(p.secondB){
           String flag = "lib" + File.separator;
           if(!p.secondS.equals(".")){
               flag += p.secondS + File.separator;
           }
           flag += p.firstS;
           libFlags.add(flag);
        }
    }
    if(libFlags.size()<1){ // no libraries are required
      return true;
    }
    
    Vector<String> remainingFlags = new Vector<String>();

    BufferedReader in = null;
    PrintWriter out = null;
    try{
      String content ="";
      in  = new BufferedReader(new FileReader(f));
      while(in.ready()){
        String line = in.readLine();
        if(!line.startsWith("CP=")){
           content+=line + "\n";
        } else {
           // analyse String for already used libraries
           String tmp = line.substring(3).trim(); 
           Pattern p = Pattern.compile("\\$S");
           String[] libs = p.split(tmp);
           TreeSet<String> usedLibs = new TreeSet<String>();
           for(int i=0;i<libs.length;i++){
              usedLibs.add(libs[i]);
           }

           for(int i=0;i<libFlags.size();i++){
               if(usedLibs.contains(libFlags.get(i)) || usedLibs.contains("\""+libFlags.get(i)+"\"" )){
                   System.err.println("lib " + libFlags.get(i) +" already used");
               } else {
                 remainingFlags.add(libFlags.get(i));
               }
           }   
           if(remainingFlags.size()==0){
              try{in.close();} catch(Exception e){}
              in = null;
              return true;
           } else {
             line = line.trim();
             for(int i=0;i<remainingFlags.size();i++){
                line += "$S\""+remainingFlags.get(i)+"\"";
             }
             content += line +"\n"; 
           }
        }
      }
      try{in.close();} catch(Exception e){}
     

       in = null;
       out = new PrintWriter(new FileOutputStream(f));
       out.print(content);
    } catch(Exception e){
      System.err.println("problem in modifying start script");
    } finally{
       if(in!=null){
         try{in.close();}catch(Exception e){}
       }
       if(out!=null){
         try{out.close();}catch(Exception e){}
       }
    }
    return true;
  }
  
  /** Adds subdirectory to the makefile within the algebras directory **/
  private boolean updateHoeseMake(File f){
    if(!f.exists()){
        System.err.println("File "+f.getAbsolutePath()+" not found. Check your Secondo installation");
        return false;
    }
    // check whether the file must be modyfied
    TreeSet<String> subdirsAll = new TreeSet<String>();
    TreeSet<String> subdirsClean = new TreeSet<String>();
    for(int i=0;i<files.size();i++){
       StringTriple t  = files.get(i);
       String loc = t.second==null?"":t.second.replaceAll("/.*","");
       if(!loc.equals(".") && loc.length()>0){
          subdirsAll.add(loc);
          subdirsClean.add(loc);
       }
    }
    if(subdirsAll.size()==0){ // no changes required
       return true;
    }
    BufferedReader in = null;
    PrintWriter out = null;
    int posAll = -1;
    int posClean = -1;
    try{
      in = new BufferedReader(new FileReader(f));
      String content = "";
      while(in.ready()){
         String line = in.readLine();
         if(line.matches("all:.*")) {
             content += line +"\n";
             while(in.ready() && ((line=in.readLine()).startsWith("\t"))){
                 if(line.matches("\t\\s*make\\s+-C\\s+\\w+\\s+all\\s*")){
                   // extract subdir information
                   String g = line.replaceAll("\t\\s*make\\s+-C\\s+","");
                   g = g.replaceAll(" .*","");
                   subdirsAll.remove(g); 
                 }
                 content += line + "\n";
             }
             posAll = content.length();
             content += line +"\n";
         } else if(line.matches("clean:.*")){
             content += line +"\n";
             while(in.ready() && ((line=in.readLine()).startsWith("\t"))){
                 if(line.matches("\t\\s*make\\s+-C\\s+\\w+\\s+clean\\s*")){
                   // extract subdir information
                   String g = line.replaceAll("\t\\s*make\\s+-C\\s+","");
                   g = g.replaceAll(" .*","");
                   subdirsClean.remove(g); 
                 }
                 content += line + "\n";
             }
             if(in.ready()){
                posClean = content.length();
                content += line +"\n";
             } else {
                content += line +"\n";
                posClean = content.length();
             }
         } else {
             content += line +"\n";
         }
      }

      try{in.close();in=null;}catch(Exception e){}
      if(subdirsAll.size()==0 && subdirsClean.size()==0){
         return true;
      }
      String blockAll = "";
      Iterator<String> it = subdirsAll.iterator();
      while(it.hasNext()){
        blockAll += "\tmake -C " + it.next() +" all\n";
      }
      String blockClean = "";
      it = subdirsClean.iterator();
      while(it.hasNext()){
        blockClean += "\tmake -C " + it.next() +" clean\n";
      }

      if(posAll > posClean){
        System.out.println("invalid makefile found");
        return false;
      }

      String p1 = content.substring(0,posAll);
      String p2 = content.substring(posAll,posClean);
      String p3 = content.substring(posClean, content.length());
      out = new PrintWriter(new FileOutputStream(f));
      out.print(p1);
      out.print(blockAll);
      out.print(p2);
      out.print(blockClean);
      out.print(p3);
    } catch (Exception e){
       e.printStackTrace();
       return false;
    } finally{
       if(out!=null){
          try{
            out.close();
          } catch(Exception e){}
       }
    }  
    return true;

  } 


  /** Installs the display classes */
  public boolean install(String secondoDir, String ZipFileName){
    // copy the files
     ZipFile f = null;
     String s = File.separator;
     char sc = File.separatorChar;
     String guiDir = secondoDir + s + "Javagui"+ s;
     String algDir = guiDir+"viewer"+s+"hoese"+s+"algebras"+s;
     String libDir = guiDir+"lib"+s;
     try{
       f = new ZipFile(ZipFileName);
       // copy mainClass
       String fn = algDir + mainClass;
       copyZipEntryToFile(new File(fn), f, f.getEntry(getFolderString() + mainClass));
       // copy Files
       for(int i=0; i< files.size(); i++){
         StringTriple triple = files.get(i);
         fn = algDir + getTargetName(triple).replace('/',sc);
         copyZipEntryToFile(new File(fn), f, f.getEntry(getEntryName(triple)));
       }
       // copy libraries
       for(int i=0;i<libDeps.size();i++){
          StringBoolPair ld = libDeps.get(i);
          if(ld.firstB){ // lib provided in zip file
            fn = libDir + ld.secondS.replace('/',sc) + s +  ld.firstS; 
            String fold = folder==null?"":folder.replace('/',sc);
            String libEntry = fold+ld.firstS;
            copyZipEntryToFile(new File(fn), f, f.getEntry(libEntry));
          }
       }
       
       File script = new File(guiDir+"sgui");
       if(!addToStartScript(script)){
         return false;
       }

       if(!modifyMakeFileInc(secondoDir)){
          System.err.println("problem in updating file makefile.inc");
          return false;
       }

       File hoesemake = new File(algDir+"makefile");
       updateHoeseMake(hoesemake);
       showCopyright(f);
     } catch(Exception e){
        e.printStackTrace();
        return false;
     } finally{
       if(f!=null){
         try{f.close();}catch(Exception e){}
       }
     }
     return true;  
  }

  


} // class HoeseInfo

