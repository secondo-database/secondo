import java.util.*;
import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;
import java.util.zip.*;


/** Class collecting information about 
  *  Algebra modules
  **/
public class AlgebraInfo{

   private boolean valid;                      // a valid Info ?
   private String algName = null;              // Name of the Algebra 
   private int secondo_Major_Version = -1;     // version informtion
   private int secondo_Minor_Version = 0;      // version information
   private int secondo_SubMinor_Version = 0;   // version information
   private String specFile = null;             // name of the specfile included in the zip
   private String exampleFile = null;          // name of the example file included in the zip
   private String copyright = null;            // name of the file containing the copyright notice
   private Vector<String> algebraDeps = new Vector<String>();  // needed algabras except Standard
   private Vector<String> libNames = new Vector<String>();     // Names of needed libraries, e.g. GSL
   private Vector<String> libFlags = new Vector<String>();     // lib names, e.g. gsl
   private Vector<String> sourceFiles = new Vector<String>();  // all sources of the algebra


   public AlgebraInfo(Node n){
      valid = readAlgebra(n);
   }

  /** Check for validity **/
  public boolean isValid(){
    return valid;
  }


/**
 *Returns possible Algebra directory names for an algebra name.
 *Algebra name without Algebra and extended by "-C++".  
**/
private static Vector<String> getPossibleAlgebraDirNames(String algName){
  Vector<String> res = new Vector<String>(4);
  res.add(algName);
  if(algName.endsWith("Algebra")){
     res.add(algName.substring(0,algName.length()-7));
  }else{
     res.add(algName+"Algebra");
  }
  for(int i=0;i<2;i++){
    res.add(res.get(i).toString()+"-C++");
  }
  return res;
}

  /** Conversion to a string **/
  public String toString(){
    String res =  "[ AlgebraInfo: ";
    res += "valid = " + valid +", ";
    res += "AlgName = " + algName+",";
    res += "Version = " + secondo_Major_Version+"."+secondo_Minor_Version+"."+secondo_SubMinor_Version+", ";
    res += "SpecFile = " + specFile + ", " ;
    res += "ExampleFile = " + exampleFile+", ";
    res += "copyright = " + copyright+" ,";
    res += "AlgDeps: (";
    for(int i=0;i<algebraDeps.size();i++){
       res += algebraDeps.get(i);
       if(i<algebraDeps.size()-1){
          res += ",";
       }
    }
    res += "), ";
    res += "LibNames: (";
    for(int i=0;i<libNames.size();i++){
       res += libNames.get(i);
       if(i<libNames.size()-1){
          res += ",";
       }
    }
    res += "), ";
    res += "LibFlags: (";
    for(int i=0;i<libFlags.size();i++){
       res += libFlags.get(i);
       if(i<libFlags.size()-1){
          res += ",";
       }
    }
    res += "), ";

    res += "SourceFiles: (";
    for(int i=0;i<sourceFiles.size();i++){
       res += sourceFiles.get(i);
       if(i<sourceFiles.size()-1){
          res += ",";
       }
    }
    res += "]";
    return res; 
  }



  /**
    *  Returns the names of all sources to be installed into the algebra directory.
    **/
  TreeSet<String> getAllSources(){
     TreeSet<String> res = new TreeSet<String>();
     res.add(specFile);
     res.add(exampleFile);
     res.addAll(sourceFiles);
     return res;  
  }

  /** Alanyse of the algebra part of the xml file **/
  private boolean readAlgebra(Node n){
     NodeList nl = n.getChildNodes();
     for(int i=0; i < nl.getLength(); i++){
        Node n2 = nl.item(i);
        String name = n2.getNodeName();
        if(name.equals("Dependencies")){
          readDependencies(n2);
        } else if(name.equals("SourceCode")){
          readSources(n2);
        } else if(name.equals("SpecFile")){
           if(specFile!=null){
             System.err.println("only a single spec file is allowed");
             return false;
           } else {
             if(n2.hasChildNodes()){
                specFile = n2.getFirstChild().getNodeValue().trim();
             } else {
               System.err.println("empty Spec not allowed");
               return false;
             }
           }

        }else if(name.equals("ExampleFile")){
           if(exampleFile!=null){
             System.err.println("only a single example file is allowed");
             return false;
           } else {
             if(n2.hasChildNodes()){
                exampleFile = n2.getFirstChild().getNodeValue().trim();
             } else {
               System.err.println("empty Example not allowed");
               return false;
             }
           }
       } else if(name.equals("Copyright")){
           if(copyright!=null){
             System.err.println("only a single copyright is allowed");
             return false;
           } else {
             if(n2.hasChildNodes()){
                copyright = n2.getFirstChild().getNodeValue().trim();
             } else {
               System.err.println("empty Copyright not allowed");
               return false;
             }
           }
       } else if(name.equals("Name")){
           if(algName!=null){
             System.err.println("only a single name is allowed");
             return false;
           } else {
             if(n2.hasChildNodes()){
                algName = n2.getFirstChild().getNodeValue().trim();
             } else {
               System.err.println("empty Copyright not allowed");
               return false;
             }
           }
       } else if(!name.equals("#text")){
           System.out.println("Unsupported node name in algebra " + name );
       }
    }
    return algName!=null &&
           secondo_Major_Version >0 && 
           secondo_Minor_Version>=0 &&
           secondo_SubMinor_Version>=0 &&
           sourceFiles.size()>0 &&
           specFile!=null &&
           exampleFile!=null &&
           copyright!=null;
  }  // readAlgebra 


  /** Collects the names of the source files from the xml file. **/
  boolean  readSources(Node n1){
     NodeList nl = n1.getChildNodes();
     for(int i=0;i<nl.getLength(); i++){
        Node n = nl.item(i);
        if(n.getNodeName().equals("file")){
            if(n.hasChildNodes()){
               String f  = n.getFirstChild().getNodeValue().trim();
               if(f.length()>0){
                  sourceFiles.add(f);
               }
            } 
        } else if(!n.getNodeName().equals("#text")){
          System.out.println("Unsupported node type in Sources");
        }
     }
     return sourceFiles.size() > 0;
  }

  /** Extracts the dependencies form the xml file **/ 
  boolean readDependencies(Node n1){
    NodeList nl = n1.getChildNodes();
    for(int i=0;i<nl.getLength(); i++){
       Node n = nl.item(i);
       String name = n.getNodeName();
       if(name.equals("Algebra")){
          if(n.hasChildNodes()){
             String  a = n.getFirstChild().getNodeValue().trim();
             if(a.length()>0){
                  algebraDeps.add(a);
             }
          } 
       } else if(name.equals("SecondoVersion")){
         readVersion(n);
       } else if(name.equals("Libraries")){
         readLibraries(n);
       } else if(!name.equals("#text")){
         System.err.println("unknown element found in dependencies: " + name);
       }
    }
    return true;

  }

  /** Extracts the version information from the xml file **/
  boolean readVersion(Node n1){
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
       } else if(!name.equals("#text")){
           System.err.println("Unknown version information found" + name);
       }
    }
    return secondo_Major_Version>0 &&
           secondo_Minor_Version>=0 &&
           secondo_SubMinor_Version>=0;
  }

  /** Extracts the needed algebras from the xml file. **/
  boolean readLibraries(Node n1){
     NodeList nl = n1.getChildNodes();
     for(int i=0;i<nl.getLength(); i++){
       Node n = nl.item(i);
       if(n.getNodeName().equals("Lib")){
          NamedNodeMap m = n.getAttributes();
          for(int j=0;j<m.getLength();j++){
             Node a = m.item(j);
             if(a.getNodeName().equals("name")){
                libNames.add(a.getNodeValue().trim());
             } else if(a.getNodeName().equals("flag")){
                libFlags.add(a.getNodeValue().trim());
             } else {
                System.err.println("Unknown Attribute in library");
             }
          }
       } else if(!n.getNodeName().equals("#text")) {
          System.err.println("unknown node found in lib" + n.getNodeName());
       }
     }
     return true; 
  }
 
/** This fumction checks whether all dependencies are solved.  **/
private static boolean checkDependencies(String secondoDir, Vector<AlgebraInfo> infos){
  // get SecondoVersion
  File versionFile = new File(secondoDir + "/include/version.h");
  if(!versionFile.exists()){
    System.err.println("Version file '"+versionFile.getAbsolutePath()+"' not found");
    return false;
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
         return false;
      }
  } catch (Exception e){
    e.printStackTrace();
    return false;
  }


  // get Algebra Names 
  TreeSet<String> names  = new TreeSet<String>();
  for(int i=0;i<infos.size();i++){
     names.add( infos.get(i).getAlgebraName());
  }
 
  for(int i=0;i<infos.size();i++){
     AlgebraInfo info = infos.get(i);
      
     // check version
     String name = info.getAlgebraName();
     if( major < info.getSecondo_Major_Version() ||
         minor < info.getSecondo_Minor_Version() ||
         subminor < info.getSecondo_SubMinor_Version()){
       System.err.println("Algebra " + name + " requires a Secondo Version " +
                          info.getSecondo_Major_Version() + "." +
                          info.getSecondo_Minor_Version() + "." +
                          info.getSecondo_Minor_Version()
                          + " but the currently installed version is " +
                          major + "." + minor+"." + subminor);
       return false;  
     }
     // check required algebras
     Vector algebraDeps = info.getAlgebraDeps();
     String algDir = secondoDir+File.separator+"Algebras"+File.separator;
     for(int j=0;j<algebraDeps.size();j++){
       String alg = algebraDeps.get(j).toString();
       if(!names.contains(alg)){ // algs installed together
          File a = new File(algDir+alg);
          if(!a.exists() || !a.isDirectory()){
             if(alg.endsWith("Algebra")){
                alg = alg.substring(0,alg.length()-7);
                a = new  File(algDir+alg);
             }
             if(!a.exists() || !a.isDirectory()){
                alg = alg+"-C++";
                a = new File(algDir+alg);
             }
             if(!a.exists() || !a.isDirectory()){
                System.err.println(a.getAbsolutePath());
                System.err.println("The algebra "+ name+
                                   " requires the algebra " + alg +
                                   " but this algebra is not installed.");
                return false;
             }
          }
       }
     } 

     // show library dependencies
     Vector libDeps = info.getLibNames();
     for(int j=0;j<libDeps.size();j++){
       System.out.println("The Algebra "+ name +" requires the library " + 
                           libDeps.get(j) + " to be installed. Please ensure to "+
                           "have installed this library before compiling secondo."); 

     }
  }
  return true;  
}

/** inserts the algebra into makefile.algebras.  **/
private boolean modifyMakeFileAlgebras(File f){
    if(!f.exists()){
      System.err.println("makefile.algebras.sample does not exist. Check the secondo installation");
      return false;
    }
    try{ 
      String content = "";
      BufferedReader in = new BufferedReader(new FileReader(f));
      boolean inActivateAllAlgebrasSection = false;
      boolean inNotMinAlgebraSet = false;
      Vector<String> algebras = new Vector<String>(); // already existing algebras
      Vector<String> libs = new Vector<String>();     // already existing libs in all algs section

      while(in.ready()){
        String line = in.readLine();
        if(line.matches("ifdef\\s+SECONDO_ACTIVATE_ALL_ALGEBRAS\\s*")){
           content += line + "\n";
           inActivateAllAlgebrasSection = true;
        } else if(line.matches("ifndef\\sSECONDO_MIN_ALGEBRA_SET\\s*")){
           content += line  + "\n";
           inNotMinAlgebraSet = true;
        } else if(line.matches("endif\\s*")){
           if(inActivateAllAlgebrasSection){
              inActivateAllAlgebrasSection = false;
              // add librarie here
              TreeSet<String> libFlags = new TreeSet<String>(getLibFlags());
              for(int i=0;i<libs.size();i++){
                libFlags.remove(libs.get(i));
              }              

              if(libFlags.size()>0){
                  String flags = "";
                  Iterator it = libFlags.iterator();
                  while(it.hasNext()){
                      flags += " " + it.next();
                  }
                  content += "ALGEBRA_DEPS += "+flags+"\n";
              }
           } else if(inNotMinAlgebraSet){
              inNotMinAlgebraSet = false;
              if(algebras.indexOf(getAlgebraName())<0){
                 // make algebra entries
                 content += "ALGEBRA_DIRS += " + getAlgebraDir()+"\n";
                content += "ALGEBRAS += " + getAlgebraName()+"\n"; 
                Vector<String> libFlags = getLibFlags();
                if(libFlags.size()>0){
                   String flags = "";
                   for(int j=0;j<libFlags.size();j++){
                       flags +=" "+libFlags.get(j);
                   }
                   content += "ALGEBRA_DEPS += "+flags+"\n";
                }
                content +="\n";
              } else {
                System.out.println("Algebra already present in "+f);
              }
           }
           content += line + "\n";
        } else { // any other line, copy it
           if(line.matches("ALGEBRAS\\s*(:|\\+)=\\s\\w+")){
               String alg = line.replaceAll(".*=","").trim();
               algebras.add(alg);
           } else if(inActivateAllAlgebrasSection &&
                     line.matches("ALGEBRA_DEPS\\s*\\+=.*")){
             String l = line.replaceAll(".*=","").trim();
             StringTokenizer st = new StringTokenizer(l);
             while(st.hasMoreTokens()){
                libs.add(st.nextToken());
             }
                
           }
           content += line +"\n";
        }
      }
      in.close();
      PrintStream out = new PrintStream(new  FileOutputStream(f));
      out.println(content);
      out.close();
    } catch(Exception e){
       e.printStackTrace();
       System.err.println("could not update makefile.algebras.sample");
       return false;
    }
    return true;

}

/** Installs the algebra.  */
boolean install(String secondoDir, String zipFileName ){
  System.out.println("install algabra " + getAlgebraName());
  // create algebra directory
  File algDir = new File(secondoDir + 
                         File.separator +
                         "Algebras" + 
                         File.separator +
                         getAlgebraDir());
  algDir.mkdir();

  // collect all Filename to be installed into the algebra directory
  TreeSet<String> names = getAllSources(); 

  try{
     ZipFile zipFile = new ZipFile(zipFileName);
     Iterator<String> it = names.iterator();
     while(it.hasNext()){
       String name = it.next();
       ZipEntry entry = zipFile.getEntry(name);
       if(entry==null || entry.isDirectory()){
         System.err.println("File " + name +" not present in the zip file ");
         return false;
       } 
       // copy the file
       System.out.println("copy file "  + name);
       InputStream in = null;
       OutputStream out = null;
       try{
         in = zipFile.getInputStream(entry);
         byte[] buffer = new byte[1024];
         String fileName =  algDir.getAbsolutePath()+ File.separator +  name;
         out = new FileOutputStream(fileName);
         int read = 0;
         int size = 0;
         while((read=in.read(buffer))>=0){
            size += read;
            out.write(buffer,0,read);
         }
       } finally{
         if(in!=null){
           try{in.close();} catch(Exception e){ System.err.println("Problem in closing in file");}
         }
         if(out!=null){
           try{out.close();} catch(Exception e){System.out.println("Problen in closing out file");}
         }
       }
     }

    System.out.println("Source files successful installed");

    System.out.println("modify AlgebraList.i.cfg");
    try{
       // copy the complete file into a string and get the highest algebra number
       int number = 0;
       File f = new File (secondoDir +
                          File.separator +
                          "Algebras" + File.separator +
                          "Management" + File.separator+
                          "AlgebraList.i.cfg");
       if(!f.exists()){
          System.err.println("AlgebraList.i.cfg not found, please check your Secondo installation");
          return false;
       }
       BufferedReader in = new BufferedReader(new FileReader(f));
       String content = new String(); 
       int maxNum = 0;
    
       boolean foundInList=false; 
       while(in.ready()&&!foundInList){
         String line = in.readLine();
         content += line+"\n";
         if(line.matches("\\s*ALGEBRA_INCLUDE\\([0-9]+\\s*,\\s*\\w*\\s*\\)\\s*")){
           String n1 =  line.replaceAll(",.*","");
           n1 = n1.replaceAll("\\D","");
           int num = Integer.parseInt(n1); 
           if(num>maxNum){
             maxNum = num;
          }
         }
         if(line.matches("\\s*ALGEBRA_INCLUDE\\([0-9]+\\s*,\\s*" + 
            getAlgebraName()+"\\s*\\)\\s*")){
            foundInList = true;
         } 
       }
       in.close();
       if(!foundInList){
          // Append the entry for the new Algebra
          content +="ALGEBRA_INCLUDE("+(maxNum+1)+","+getAlgebraName()+")\n";
          // write the file
          PrintStream out = new PrintStream(new  FileOutputStream(f));
          out.println(content);
          out.close();
       } else{
          System.out.println("Algebra already found in AlgebraList.i.cfg");
       }
    }catch(Exception e){
       e.printStackTrace();
       System.err.println("AlgebraList.i.cfg could not be updated");
       return false;
    }
   
    System.out.println("modify makefile.algebras and makefile.algebras.sample");

    File f = new File(secondoDir + File.separator + "makefile.algebras.sample");
 
    if(!modifyMakeFileAlgebras(f)){
       return false;
    }   
    f = new File(secondoDir + File.separator + "makefile.algebras");
    if(f.exists()){
       if(!modifyMakeFileAlgebras(f)){
         return false;
       }
    }
    // show copyrightnotice
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


    return true;
  } catch(Exception e){
    e.printStackTrace();
    System.err.println("error in installing algebra " + getAlgebraName());
    return false;
  }
  


}


  static boolean check(String secondoDir, Vector<AlgebraInfo> infos){
     if(!checkConflicts(secondoDir,infos)){
        return false;
     }
     if(!checkDependencies(secondoDir,infos)){
       return false;
     }
     return true;
  }


 /** checks whether the algebra is already installed or if
  * the same algebra should be installed twice.
  **/
 static boolean checkConflicts(String secondoDir, Vector<AlgebraInfo> infos){
  // AlgebraNames of all infos dijoint
  TreeSet<String> names  = new TreeSet<String>();
  for(int i=0;i<infos.size();i++){
     names.add( infos.get(i).getAlgebraName());
  } 
  if(names.size() < infos.size()){
     System.err.println("multiple used algebra names");
     return false;
  }

  // AlgebraNames not already included
  String algDir = secondoDir+File.separator+"Algebras"+File.separator;
  for(int i=0;i<infos.size();i++){
     AlgebraInfo info = infos.get(i);
     Vector<String> algDirNames = getPossibleAlgebraDirNames(info.getAlgebraName());
     boolean found = false;
     for(int j=0;j<algDirNames.size();j++){
        File f = new File(algDir+algDirNames.get(j).toString());
        if(f.exists()){
           found=true;
        }
     }
     if(found){
        System.err.println("Algebra " + info.getAlgebraName() + " already installed");
        return false;
     }
  }
  return true; 

}

  /** Returns the name of the Algebra **/
  String getAlgebraName(){
    return algName;
  }

  /** Returns version information **/
  public int getSecondo_Major_Version(){
     return secondo_Major_Version;
  }
  
  /** Returns version information **/
  public int getSecondo_Minor_Version(){
    return secondo_Minor_Version;
  }
  
  /** Returns version information **/
  public int getSecondo_SubMinor_Version(){
    return secondo_SubMinor_Version;
  }

  /* Returns the names of the needed Algebras **/
  public Vector<String> getAlgebraDeps(){
    return algebraDeps;
  }

  /** Returns the names of the needed libraries. **/
  public Vector<String> getLibNames(){
    return libNames;
  }

 
  /** Returns the name of the algebra directory, normally 
    * the algebra name without the "Algebra".
    **/
  public String getAlgebraDir(){
     if(algName.endsWith("Algebra")){
        return algName.substring(0,algName.length()-7);
     }else{
        return algName;
     }
  }

  /** Returns libary names used fro linking. **/
  public Vector<String> getLibFlags(){
     return libFlags;
  }

  /** Returns the filkename for copyright information. **/
  public String getCopyright(){
     return copyright;
  }



}
