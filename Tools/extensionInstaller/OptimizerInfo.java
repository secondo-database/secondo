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

/** Information about optimizer extensions **/
public class OptimizerInfo extends SecondoExtension{

   /** set of blocks (code snippets with position information **/
   Vector<Block> blocks=new Vector<Block>();    // blocks for inserting
   String name = null;

   /** returns the tag markling the start of this extension **/
   public String getStartTag(){
     return "% Extension:Start:"+name;
   }
   /** returns a regular expression for recognizing the start of this extension **/
   public String getStartTagTemplate(){
     return "%\\s+Extension:Start:\\s*"+name+"\\s*";
   }

   /** returns a regular expression for recognizing the end of that extension **/
   public String getEndTagTemplate(){
     return "%\\s+Extension:End:\\s*"+name+"\\s*";
   }
  
   /** returns the end tag of that extension **/
   public String getEndTag(){
     return "% Extension:End:"+name;
   }

   /** creates a new OptimizerInfo from the given Node **/
   public OptimizerInfo(Node n){
     valid = readOptimizerInfo(n);
   }

   /** Reads in the information from n1 **/
   private boolean readOptimizerInfo(Node n1){
      NodeList nl = n1.getChildNodes();
      NamedNodeMap nm = n1.getAttributes();
      if(!readFolder(nm.getNamedItem("folder"))){
        return false;
      }
      for(int i=0; i< nl.getLength(); i++){
         Node n = nl.item(i);
         String name = n.getNodeName();
         if(name.equals("Dependencies")){
           if(!readDependencies(n)){
             return false;
           }
         } else if(name.equals("Name")){
           if(!n.hasChildNodes()){
              System.err.println("Empty name entry found");
              return false;
           }
           String ename = n.getFirstChild().getNodeValue().trim();
           if(ename.length()==0){
             System.err.println("empty  name found");
             return false;
           }
           this.name = ename;
         } else if(name.equals("Copyright")){
           if(!readCopyright(n)){
              return false;
           }
         } else if(name.equals("Files")){
            if(!readFiles(n)){
              return false;
            }
         } else if(name.equals("Block")){
           if(!readBlock(n)){
             return false;
           }
         } else if(!name.equals("#text") && !name.equals("comment")){
            System.err.println("unknown node type found in optimizer section " + name);
         }
      }
      if(secondo_Major_Version<0){
         System.err.println("Secondo version missing");
         return false;
      }
      if(files.size()==0 && blocks.size()==0){
         System.err.println("empty extension");
         return false;
      }
      if(name==null){
        System.err.println("The extension must have a name");
        return false;
      }
      return true;
   } 

   /** extracts the dependencies from n1 **/
   private boolean readDependencies(Node n1){
       NodeList nl = n1.getChildNodes();
       for( int i=0;i < nl.getLength(); i++){
         Node n = nl.item(i);
         String name = n.getNodeName();
         if(name.equals("SecondoVersion")){
           if(!readSecondoVersion(n)){
              return false;
           }
         } else if(!name.equals("#text") && !name.equals("#comment")){
           System.err.println("unknown key found in optimizer.dependencies.secondoversion " + name);
         }
       }
       return true;
   }


   /** reads a block from n1 **/
   private boolean readBlock(Node n1){
      NamedNodeMap m = n1.getAttributes();
      // filename
      Node n = m.getNamedItem("file");
      if(n==null){
          System.err.println(" block without file attribute found");
           return false;
      } 
      Block b = new Block();
      b.file = n.getNodeValue().trim();
      if(b.file.length()==0){
          System.err.println(" block without file attribute found");
           return false;
      } 
      // sectionname
     n = m.getNamedItem("section");
     if(n==null){
        System.err.println("no section attribute found in block");
        return false;
     }
     b.section = n.getNodeValue().trim();
     if(b.section.length()==0){
        System.err.println(" empty section attribute found in a block");
        return false;
     }
     // position
     b.first = false; // last position
     n = m.getNamedItem("position");
     if(n!=null){
       String pos = n.getNodeValue().trim();
       if(pos.equals("first")){
          b.first = true;
       } else if(pos.equals("last")){
          b.first = false;
       } else {
          System.err.println(" invalid position information " + pos+ " (first, last are allowed)");
          return false;
       }
     }
     if(!n1.hasChildNodes()){
         System.err.println("empty block is not allowed");
         return false;
     }
     b.content = n1.getFirstChild().getNodeValue();
     if(b.content==null){
         System.err.println("Empty block found in XML file");
         return false;
     }
     if(b.content.startsWith("\n")){
        b.content = b.content.substring(1);
     }
     b.content= b.content.replaceAll("\\s+$","");
     if(b.content.trim().length()==0){
        System.err.println("empty block found");
        return false;
     }
     blocks.add(b);
     return true;
   }


   /** check for fullfilled dependencies and absence of conflicts **/
   static boolean check(String secondoDir, Vector<OptimizerInfo> infos){
      if(!checkConflicts(secondoDir,infos)){
        return false;
      }
      if(!checkDependencies(secondoDir,infos)){
        return false;
      }
      return true;
   }

   /** Checks whether no conflicts are presnet **/
   static boolean checkConflicts(String secondoDir, Vector<OptimizerInfo> infos){
      // check for disjoint names of the extensions
      TreeSet<String> names = new TreeSet<String>();
      TreeSet<String> fnames = new TreeSet<String>();
      String s = File.separator;
      char sc = File.separatorChar;
      String mainDir = secondoDir+s+"Optimizer"+s;

      for(int i=0;i<infos.size();i++){
         OptimizerInfo info = infos.get(i);
         if(names.contains(info.name)){
           System.err.println("try to install extension " + info.name+" twice");
           return false;
         }
         names.add(info.name);
         for(int j=0;j<info.files.size();j++){
            StringTriple triple  = info.files.get(j);
            String start = triple.second==null?"":triple.second.replace('/',sc) + s;
            String fn = start + triple.first;
            if(fnames.contains(fn)){
               System.err.println("File " + mainDir+fn+" is tried to copy twice");
               return false;
            }
            fnames.add(fn);
         }
      }

      // check for already existing files
      Iterator<String> it = fnames.iterator();
      while(it.hasNext()){
        File f = new File(mainDir+it.next());
        if(f.exists()){
          System.err.println("try to install the already existing file " + f);
          return false;
        }
      } 

      // check whether all files to modify are present and 
      // do not contain any of the extensions to install
      for(int i=0;i<infos.size();i++){
         OptimizerInfo info = infos.get(i);
         String start = info.getStartTagTemplate();
         String end = info.getEndTagTemplate();
         Vector<Block> blocks = info.blocks;
         TreeSet<String> filesToModify = new TreeSet<String>();
         for(int j=0;j<blocks.size();j++){ 
            Block b = blocks.get(j);
            String fn = mainDir + b.file.replace('/',sc);
            File f = new File(fn);
            if(!f.exists()){
              System.err.println("Try to modify the non-existent file " + f);
              return false;
            }
            // check the file, it must exist, cannot contain the extension an must have 
            // the correct section
            BufferedReader in = null;
            boolean startFound = false;
            boolean endFound = false;
            String sstart = b.getSectionStartTemplate();
            String send = b.getSectionEndTemplate();
            try{
              in = new BufferedReader(new FileReader(f));
              while(in.ready()){
                String line = in.readLine();
                if(line.matches(start) || line.matches(end)){
                   System.err.println("Extension " + info.name+" already present in file" + f);
                   try{in.close();} catch(Exception e){}
                   return false;
                }
                if(line.matches(sstart)){
                   if(startFound){
                      System.err.println("Section " + b.section+" found twice in file " + f);
                      try{in.close();} catch(Exception e){}
                      return false;
                   }
                   startFound = true;
                }
                if(line.matches(send)){
                   if(!startFound){
                     System.err.println("In file " + f + " section " + b.section + " ends before it starts");
                     try{in.close();} catch(Exception e){}
                     return false;
                   }
                   if(endFound){
                      System.err.println("End of section " + b.section + " gound  twice in file " + f);
                      try{in.close();}catch(Exception e){}
                      return false;
                   }
                   endFound = true;
                }
              }
              if(!startFound){
                 System.err.println("Section " + b.section + " not found in file " + f);
                 try{in.close();}catch(Exception e){}
                 return false;
              }
              if(!endFound){
                 System.err.println("End of section " + b.section + " not found in file " + f);
                 try{in.close();}catch(Exception e){}
                 return false;
              }
            } catch(Exception e){
               System.err.println("Problem in reading file "+f);
               return false;
            } finally{
               if(in!=null){
                 try{
                   in.close();
                 } catch(Exception e){}
               }
            }
         } // end of processing blocks
  
         // checks whether all new files are not present
         for(int j=0;j<info.files.size();j++){
           StringTriple t = info.files.get(j);
           String middle = t.second==null?"":t.second.replace('/',sc)+s;
           File f = new File(mainDir + middle+t.first);
           if(f.exists()){
             System.err.println("Try to install file " + f+" which is already present");
           }
         }
    }
    return true;
   }

  /** checks whether all dependencies are fullfilled **/
  static  boolean checkDependencies(String secondoDir, Vector<OptimizerInfo> infos){
     Version iver = readSecondoVersion(secondoDir);
     if(iver==null){
       return false;
     }
     // check for correct Secondo version
     for(int i=0;i<infos.size();i++){
        OptimizerInfo info = infos.get(i);
	Version rver = new Version(info.secondo_Major_Version, info.secondo_Minor_Version, 
			           info.secondo_SubMinor_Version);
        if(!rver.isSmallerOrEqual(iver)){
            System.err.println("Installed Secondo version is too old for the extension");
            return false;
        }  
     }
     return true;
  }

  /** checks wether all required files are present within the given zip file **/
  boolean filesPresent(ZipFile f){
     if(files.size()==0){
        return true;
     }
     Vector<String> names = new Vector<String>();
     for(int i=0;i<files.size();i++){
        names.add(files.get(i).first);
     }
     return filesPresent(f,names);
  }

  
  /** installs the extension **/
  boolean install(String secondoDir, ZipFile f){
     String s = File.separator;
     char sc = File.separatorChar;
     String mainDir = secondoDir+s+"Optimizer"+s;
     // copy files
     for(int i=0;i<files.size();i++){
       StringTriple t = files.get(i);
       String middle = t.second==null?"":t.second.replace('/',sc)+s;
       String fn = mainDir+middle+t.first;
       System.out.println("Copy to File " + fn);
       if(!SecondoExtension.copyZipEntryToFile(new File(fn), f, f.getEntry(getEntryName(t)))){
           return false;
       } 
     } 
     // modify blocks
     for(int i=0;i<blocks.size();i++){
       Block b = blocks.get(i);
       if(!installBlock(secondoDir,b)){
          return false;
       }
     }   
     showCopyright(f); 
     return true; 
  }

  /** installs a single block **/
  boolean installBlock(String secondoDir, Block b){
      if(b.content==null){
         return true;
      }
      // read the complete file into a string
      StringBuffer contentb = new StringBuffer(262144);
      BufferedReader in = null;
      boolean ok = true;
      String s = File.separator;
      String mainDir = secondoDir+s+"Optimizer"+s;
      File f = new File(mainDir + b.file);
      try{
        in = new BufferedReader(new FileReader(f));
        while(in.ready()){
           contentb.append(in.readLine()+"\n");
        }
      } catch(Exception e){
        System.err.println("Error while reading file " + f);
        ok = false;
      } finally{
        try{if(in!=null)in.close();}catch(Exception e){}
      }
      if(!ok){
         return false;
      }

      String content = contentb.toString();
      // insert block content into file content 
      if(!b.first){ // insert at the end of the section
         String[] parts = content.split("\n"+b.getSectionEndTemplate()+"\n");
         if(parts.length!=2){
           System.err.println("Error during processing file " + f + ", section end " + b.getSectionEnd()+ " not found");
           return false;
         } 
         content = parts[0] +"\n"+ getStartTag()+"\n"+b.content+"\n"+getEndTag()+"\n"+b.getSectionEnd()+"\n"+parts[1];
      } else { // insert content at the beginb of the section
         String[] parts = content.split("\n"+ b.getSectionStartTemplate()+"\n");
         if(parts.length!=2){
           System.err.println("Error during processing file " + f + ", section start" + b.getSectionStart()+ " not found");
           return false;
         }
         content = parts[0] + "\n"+b.getSectionStart()+"\n"+getStartTag()+"\n"+b.content+"\n"+getEndTag()+"\n"+parts[1];
      }
      // write modified content to file
      PrintWriter out = null;
      try{
        out = new PrintWriter(new FileWriter(f));
        out.print(content);
      } catch(Exception e){
        System.err.println("Problem in writing file " + f);
        ok = false;

      } finally{
          if(out!=null){try{out.close();}catch(Exception e){}}
      }
      return ok; 
  } 





}
