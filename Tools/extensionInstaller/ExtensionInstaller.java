
import java.io.*;
import java.util.zip.*;
import java.util.Vector;
import java.util.TreeSet;
import java.util.Enumeration;
import java.util.Iterator;
import org.w3c.dom.*;
import javax.xml.parsers.*;

class ExtensionInfo{

  private boolean valid;
  private String fileName = null;
  private String algName = null;
  private int secondo_Major_Version = -1;
  private int secondo_Minor_Version = 0;
  private int secondo_SubMinor_Version = 0;
  private String specFile = null;
  private String exampleFile = null;
  private String copyright = null;
  private Vector algebraDeps = new Vector();
  private Vector libNames = new Vector();
  private Vector libFlags = new Vector();
  private Vector sourceFiles = new Vector();


  // returns the names of all sources to be installed into the algebra directory
  TreeSet getAllSources(){
     TreeSet res = new TreeSet();
     res.add(specFile);
     res.add(exampleFile);
     res.addAll(sourceFiles);
     return res;  
  }


  public  ExtensionInfo(String extensionFile, InputStream i){
    valid = readDocument(i);
    fileName = extensionFile;
  }

  public boolean isValid(){
    return valid;
  }

  public String toString(){
    String res =  "[ ExtensionInfo: ";
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

  private boolean readDocument(InputStream i){
    try{
      Document d = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(i);
      return processDoc(d);
    } catch( Exception e ) {
      e.printStackTrace();
      return false;
    }
  }


  private boolean processDoc(Document d){
    Element root = d.getDocumentElement();
    if(root==null){
        System.err.println("Error in document");
        return false;
    }
    if(!root.getTagName().equals("SecondoExtension")){
       System.err.println("not a SecondoExtension file");
       return false;
    }
    NodeList nl = root.getChildNodes();
    boolean isExtension = false;
    boolean algebraInstalled = false;
    for(int i=0; i< nl.getLength();i++){
       Node n = nl.item(i);
       String name = n.getNodeName();
       if(name.equals("Algebra")){
          if(algebraInstalled){
             System.err.println("only one algebra per module possible");
             return false;
          }
          if(!readAlgebra(n)){
              return false;
          }
          isExtension = true;
          algebraInstalled = true;
       } else if(name.equals("Viewer")){
          System.err.println("Viewer not supported yet");

       } else if(name.equals("HoeseExtension")){
          System.err.println("HoeseViewer not supported yet");
       } else if(name.equals("Optimizer")){

       } else if(name.equals("Kernel")){
          System.err.println("Kernel not supported yet");

       } else if(!name.equals("#text")){
           System.err.println("unknown entry " + name );
       }
     }
     return isExtension;
  }


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

  String getAlgebraName(){
    return algName;
  }

  public int getSecondo_Major_Version(){
     return secondo_Major_Version;
  }
  public int getSecondo_Minor_Version(){
    return secondo_Minor_Version;
  }
  public int getSecondo_SubMinor_Version(){
    return secondo_SubMinor_Version;
  }

  public Vector getAlgebraDeps(){
    return algebraDeps;
  }

  public Vector getLibNames(){
    return libNames;
  }

  public String getFileName(){
     return fileName;
  }
 
  public String getAlgebraDir(){
     if(algName.endsWith("Algebra")){
        return algName.substring(0,algName.length()-7);
     }else{
        return algName;
     }
  }

  public Vector getLibFlags(){
     return libFlags;
  }

}



/*
   This class will install a given set of extensions to the
   Secondo Extensible Datanbase System.

*/
public class  ExtensionInstaller{

private File secondoDir=null;

public ExtensionInstaller(String secondoDirectory){
  if(!secondoDirectory.endsWith(File.separator)){
     secondoDirectory += File.separator;
  }
  File f = new File(secondoDirectory);
  
  if(f.exists()){
    secondoDir = f;
  } else {
    secondoDir = null;
  }
}

boolean checkConflicts(Vector infos){
  // AlgebraNames of all infos dijoint
  TreeSet names  = new TreeSet();
  for(int i=0;i<infos.size();i++){
     names.add( ((ExtensionInfo)infos.get(i)).getAlgebraName());
  } 
  if(names.size() < infos.size()){
     System.err.println("multiple used algebra names");
     return false;
  }
  // AlgebraNames not already included
  for(int i=0;i<infos.size();i++){
     String name = ((ExtensionInfo)infos.get(i)).getAlgebraDir();
     File f = new File(secondoDir+name);
     if(f.exists()){
        System.err.println("Algebra " + name + " already exists");
     }
  }
  return true; 

}

/*
 This fumction checks whether all dependencies are solved.

*/
boolean checkDependencies(String secondoDir, Vector infos){
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
  TreeSet names  = new TreeSet();
  for(int i=0;i<infos.size();i++){
     names.add( ((ExtensionInfo)infos.get(i)).getAlgebraName());
  }
 
  for(int i=0;i<infos.size();i++){
      ExtensionInfo info = (ExtensionInfo)infos.get(i);
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
       System.err.println("The Algebra "+ name +" requires the library " + 
                           libDeps.get(j) + " to be installed. Please ensure to "+
                           "have installed this library before compiling secondo."); 

     }
  }
  return true;  
}

private boolean modifyMakeFileAlgebras(File f, ExtensionInfo info){
    if(!f.exists()){
      System.err.println("makefile.algebras.sample does not exist. Check the secondo installation");
      return false;
    }
    try{ 
      String content = "";
      BufferedReader in = new BufferedReader(new FileReader(f));
      boolean inActivateAllAlgebrasSection = false;
      boolean inNotMinAlgebraSet = false;
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
              Vector libFlags = info.getLibFlags();
              if(libFlags.size()>0){
                  String flags = "";
                  for(int j=0;j<libFlags.size();j++){
                      flags += " " + libFlags.get(j);
                  }
                  content += "ALGEBRA_DEPS "+flags+"\n";
              }
           } else if(inNotMinAlgebraSet){
              inNotMinAlgebraSet = false;
              // make algebra entries
              content += "ALGEBRA_DIRS += " + info.getAlgebraName()+"\n";
              content += "ALGEBRAS += " + info.getAlgebraDir()+"\n"; 
              Vector libFlags = info.getLibFlags();
              if(libFlags.size()>0){
                 String flags = "";
                 for(int j=0;j<libFlags.size();j++){
                     flags +=" "+libFlags.get(j);
                 }
                 content += "ALGEBRA_DEPS "+flags+"\n";
              }
           }
           content += line + "\n";
        } else { // any other line, copy it
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


private boolean install(String secondoDir, ExtensionInfo info){
  System.out.println("install algabra " + info.getAlgebraName());
  // create algebra directory
  File algDir = new File(secondoDir+File.separator+info.getAlgebraName());
  algDir.mkdir();

  // collect all Filename to be installed into the algebra directory
  TreeSet names = info.getAllSources(); 

  try{
     ZipFile zipFile = new ZipFile(info.getFileName());
     Iterator it = names.iterator();
     while(it.hasNext()){
       String name = (String)it.next();
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
         out = new FileOutputStream(algDir.getAbsolutePath()+
                                    File.separator +
                                    name);
         int read = 0;
         while(in.read(buffer)>=0){
            out.write(buffer,0,read);
         }
       } finally{
         if(in!=null){
           try{in.close();} catch(Exception e){}
         }
         if(out!=null){
           try{out.close();} catch(Exception e){}
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
     
       while(in.ready()){
         String line = in.readLine();
         content += line+"\n";
         if(line.matches("\\s*ALGEBRA_INCLUDE\\([0-9]\\s*,\\s*\\w*\\s*\\)\\s*")){
           System.out.println("found" + line);
           String n1 =  line.replaceAll("\\D","");
           int num = Integer.parseInt(n1); 
           if(num>maxNum){
             maxNum = num;
           }
         } 
       }
       in.close();
       // Append the entry for the new Algebra
       content +="ALGEBRA_INCLUDE("+(maxNum+1)+","+info.getAlgebraName()+")\n";
       // write the file
       PrintStream out = new PrintStream(new  FileOutputStream(f));
       out.println(content);
       out.close();
    }catch(Exception e){
       e.printStackTrace();
       System.err.println("AlgebraList.i.cfg could not be updated");
       return false;
    }
   
    System.out.println("modifying AlgebraList.i.cfg finished"); 

    System.out.println("modify makefile.algebras and makefile.algebras.sample");

    File f = new File(secondoDir + File.separator + "makefile.algebras.sample");
 
    if(!modifyMakeFileAlgebras(f,info)){
       return false;
    }   
    f = new File(secondoDir + File.separator + "makefile.algebras");
    if(f.exists()){
       if(!modifyMakeFileAlgebras(f,info)){
         return false;
       }
    }


    return true;
  } catch(Exception e){
    e.printStackTrace();
    System.err.println("error in installing algebra " + info.getAlgebraName());
    return false;
  }
  


}







public boolean installExtensions(String[] extensionFiles){
  try{
     // step one: extract the ExtensionInformations
     Vector infos = new Vector(extensionFiles.length);
     for(int i=0; i< extensionFiles.length; i++){
        ZipFile zipFile = new ZipFile(extensionFiles[i]);
        Enumeration entries = zipFile.entries();
        boolean found = false;
        while(entries.hasMoreElements() && ! found){
           ZipEntry entry = (ZipEntry) entries.nextElement();
           if(!entry.isDirectory() && entry.getName().equals("SecondoExtension.xml")){
              ExtensionInfo info = new ExtensionInfo(extensionFiles[i],
                                                     zipFile.getInputStream(entry));
              if(!info.isValid()){
                 throw new Exception("Invalid xml file");
              }else{
                infos.add(info);
              }
              found = true;
           }
        }
        if(!found){
          throw new Exception("xml file not found");
        }
        zipFile.close();
     }
     // all zip files was read successfully
     if(!checkConflicts(infos)){
         System.err.println("Conflict found" );
         return false;
     }
     if(!checkDependencies(secondoDir.getAbsolutePath(), infos)){
       System.err.println("problem in dependencies found" );
       return false;
     }  

     System.out.println("No problems found, start to install the algebra(s)");
     for(int i=0;i<infos.size();i++){
        install(secondoDir.getAbsolutePath(), (ExtensionInfo) infos.get(i));
     }
  } catch ( Exception e){
    e.printStackTrace();
    return false;
  }

  return false; // implementation not finished yet 

}






}

