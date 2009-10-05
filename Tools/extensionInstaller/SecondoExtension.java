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

/** Super class for all Secondo extensions **/
public class SecondoExtension{

   protected boolean valid;                      // flag whether the Info is valid
   protected int secondo_Major_Version = -1;     // version informtion
   protected int secondo_Minor_Version = 0;      // version information
   protected int secondo_SubMinor_Version = 0;   // version information
   protected Vector<StringTriple> files = new Vector<StringTriple>(); // Files to install
   protected String copyright=null;                               // name of the file containing the copyright notice
   protected String folder = null;               // sourcefolder, . if null


   protected boolean readFolder(Node n){
     if(n!=null){
       String tmp = n.getNodeValue().trim();
       while(tmp.startsWith("/")){
         tmp = tmp.substring(1,tmp.length()-1);
       }
       while(tmp.endsWith("/")){
         tmp = tmp.substring(0,tmp.length()-1);
       }
       if(tmp.length()>0){
          folder = tmp;
       } else{
          folder = null;
       }
     }
     return true;
   }

   protected String getFolderString(){
      return folder==null?"":folder+"/";
   }

   String getEntryName(StringTriple t){
      String d = folder==null?"":folder+"/";
      String sd = t.third==null?"":t.third+"/";
      return d+sd+t.first;
   }

   String getTargetName(StringTriple t){
     if(t.second==null){
        return t.first;
     } else {
        return t.second+"/"+t.first;
    }
   }

   boolean copyFiles(String baseDir, ZipFile zipFile){
      String fold = getFolderString(); // global folder in zip file
      for(int i=0;i<files.size();i++){
         StringTriple t = files.get(i);
         String middle = t.second==null?"":t.second.replace('/',File.separatorChar)+File.separator;
         String filename = baseDir + middle+t.first;
         if(!copyZipEntryToFile(new File(filename), zipFile, zipFile.getEntry(getEntryName(t)))){
           return false;
         }
      }
      return true;
   }

  /** Checks whether the files, given in names, are present in f **/
   boolean filesPresent(ZipFile f, Vector<String> names){
     for(int i=0;i<names.size();i++){
        if(f.getEntry(names.get(i))==null){
          System.err.println("Entry " + names.get(i) + " not found");
          return false;
        }
     }
     return  true;
   }

   /** checks whether this extension is valid **/
   public boolean isValid(){
     return valid;
   }

   /** delete a directory or a file. if f is a directory, removing is done
     * recursively.
     */
   public boolean remove(File f){
     try{
        if(!f.exists()){
          return false;
        }
        if(!f.isDirectory()){
           f.delete();
           return true;
        } else {
          File[] content = f.listFiles();
          boolean res = true;
          for(int i=0;i<content.length;i++){
             if(!remove(content[i])){
               res = false;
             }
          }
          f.delete();
          return res;
        }
     } catch(Exception e){
         return false;   
     }
   }


   /** Reads the copyright information from n **/
   protected boolean readCopyright(Node n){
      if(!n.hasChildNodes()){
        return false;
      }
      String cr = n.getFirstChild().getNodeValue().trim();
      if(cr.length()==0){
        System.err.println("empty copyright file found");
        return false;
      }
      copyright = cr;
      return true;
   }


  /** Extracts the version information from the xml file **/
  boolean readSecondoVersion(Node n1){
    NodeList nl = n1.getChildNodes();
    for(int i=0;i<nl.getLength(); i++){
       Node n = nl.item(i);
       String name = n.getNodeName();
       if(name.equals("Major")){
          if(n.hasChildNodes()){
             String  a = n.getFirstChild().getNodeValue().trim();
             secondo_Major_Version=Integer.parseInt(a.trim());
          } 
       } else if(name.equals("Minor")){
          if(n.hasChildNodes()){
             String  a = n.getFirstChild().getNodeValue().trim();
             secondo_Minor_Version=Integer.parseInt(a.trim());
          } 
       } else if(name.equals("SubMinor")){
          if(n.hasChildNodes()){
             String  a = n.getFirstChild().getNodeValue().trim();
             secondo_SubMinor_Version=Integer.parseInt(a.trim());
          } 
       } else if(!name.equals("#text") && !name.equals("#comment")){
           System.err.println("Unknown version information found" + name);
       }
    }
    return secondo_Major_Version>0 &&
           secondo_Minor_Version>=0 &&
           secondo_SubMinor_Version>=0;
  }


  /** Copyies the content of entry e within zip to f **/
  static boolean copyZipEntryToFile(File f, ZipFile zip, ZipEntry e){
     boolean ok = true;
     File path = f.getParentFile();
     if(!path.exists()){
       path.mkdirs();
     }
     InputStream in = null;
     OutputStream out = null;
     try{
       in = zip.getInputStream(e);
       byte[] buffer = new byte[1024];
       out = new FileOutputStream(f);
       int read = 0;
       int size = 0;
       while((read=in.read(buffer))>=0){
         size += read;
         out.write(buffer,0,read);
       }

     } catch(Exception ex){
       System.err.println("Problem in copying file " + f);
       ok = false;
     } finally{
         if(in!=null){
           try{in.close();} catch(Exception ex){ System.err.println("Problem in closing in file");}
         }
         if(out!=null){
           try{out.close();} catch(Exception ex){System.out.println("Problen in closing out file");}
         }
 
     }
     return ok;
  }


   /** Reads the required files and its location **/
   protected boolean readFiles(Node n1){
     NodeList nl = n1.getChildNodes();
     for(int i=0;i<nl.getLength();i++){
       Node n = nl.item(i);
       String name = n.getNodeName();
       if(!name.toLowerCase().equals("file") && !name.equals("#text") && !name.equals("#comment")){
         System.err.println("Unknown node name for files detected: " + name);
       } else if(name.toLowerCase().equals("file")){
          StringTriple triple = new StringTriple();
         // get the filename
         if(n.hasChildNodes()){
            String fn = n.getFirstChild().getNodeValue().trim();
            if(fn.length()>0){
               triple.first = fn;
            } 
         }
         if(triple.first==null){
            System.err.println("XMLFile corrupt: filename missing");
            return false;
         }
         // get the location
         NamedNodeMap m = n.getAttributes();
         Node loc  = m.getNamedItem("location");
         if(loc!=null){
            String tmp = loc.getNodeValue().trim();
            while(tmp.startsWith("/")){
                tmp = tmp.substring(1,tmp.length()-1);
            }
            while(tmp.endsWith("/")){
               tmp = tmp.substring(0,tmp.length()-1);
            }
           triple.second = tmp;
         }
         Node source = m.getNamedItem("sourcefolder");
         if(source!=null){
            String tmp = source.getNodeValue().trim();
            while(tmp.startsWith("/")){
                tmp = tmp.substring(1,tmp.length()-1);
            }
            while(tmp.endsWith("/")){
               tmp = tmp.substring(0,tmp.length()-1);
            }
           triple.third = tmp;
         }
         files.add(triple);  
       }
     }
     return true;
   } 


   /** extracts the secondo version from the existing system. 
     * If any error occurs, the result will be null.
     **/
   public static Version readSecondoVersion(String secondoDir){
       String s = File.separator;
       File versionFile = new File(secondoDir + s + "include" + s + "version.h");
       if(!versionFile.exists()){
         System.err.println("Version file '"+versionFile.getAbsolutePath()+"' not found");
         return null;
       }
       int major = -1;
       int minor = -1;
       int subminor = -1;
       try{
          BufferedReader in = new BufferedReader(new FileReader(versionFile));
          while((major<0 || minor <0 || subminor <0) && in.ready()){
             String line = in.readLine();
             if(line!=null){
                line = line.trim();
                if(line.indexOf("SECONDO_VERSION_MAJOR") >=0){
                    line = line.replaceAll("[^0123456789]",""); // only keep digits
                    major = Integer.parseInt(line); 
                } else if(line.indexOf("SECONDO_VERSION_MINOR") >=0){
                    line = line.replaceAll("[^0123456789]",""); // only keep digits
                    minor = Integer.parseInt(line); 
                } else if(line.indexOf("SECONDO_VERSION_REVISION") >=0){
                    line = line.replaceAll("[^0123456789]",""); // only keep digits
                    subminor = Integer.parseInt(line); 
                }
             }
          }
          in.close();
          if(major<0 || minor < 0  || subminor <0){
             System.err.println("version not completely found");
             return null;
          }
       } catch (Exception e){
         e.printStackTrace();
         return null;
       }
      Version res = new Version();
      res.major = major;
      res.minor = minor;
      res.subminor = subminor;
      return res;
   }


   /** display the copyright notice **/
   protected void showCopyright(ZipFile zipFile){
    if(copyright==null){
       System.out.println("No copyright information available");
       return;
    }
    ZipEntry entry = zipFile.getEntry(getCopyright());
    if(entry==null){
       System.out.println("No copyright information available");
    } else {
       String cr = "";
       try{
          BufferedReader r = new BufferedReader(new InputStreamReader(zipFile.getInputStream(entry)));
          while(r.ready()){
            cr += r.readLine() + "\n";
          }
          r.close();
          System.out.println("================= COPYRIGHT NOTICE =============== \n\n"+cr);
          System.out.println("=================END OF COPYRIGHT NOTICE ========= \n");
       } catch(Exception e){
           System.err.println("Error during redaing the copyright");
       }  
    }
   }
  

   /** Returns the filename containing copyright information. **/
  public String getCopyright(){
     return copyright;
  }


}
