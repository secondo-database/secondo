import java.util.*;
import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;
import java.util.zip.*;
import java.util.regex.Pattern;

public class JavaExtension extends SecondoExtension{

   protected int java_Major_Version = 0;     // version informtion
   protected int java_Minor_Version = 0;      // version information
   protected String mainClass = null;
   protected Vector<StringPair> files = new Vector<StringPair>();
   protected Vector<StringBoolPair> libDeps = new Vector<StringBoolPair>();


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

  protected boolean checkLibDeps(String secondoDir){
     String s = File.separator;
     String libDir = secondoDir+s+"lib"+s;
     for(int i=0;i<libDeps.size();i++){
        StringBoolPair e = libDeps.get(i);
        if(!e.firstB){ // dependency to a non provided lib
           String fn = libDir + e.secondS.replaceAll("/",s) + s +  e.firstS;
           File f = new File(fn);
           if(f.exists()){
              System.err.println("lib " + fn +" required but not found");
              return false;
           }
        }
     }
     return true;
  }


  protected String getJavaVersionString(){
     return "" + java_Major_Version + "."+  java_Minor_Version;
  }
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

   /** Reads the required files and its location **/
   protected boolean readFiles(Node n1){
     NodeList nl = n1.getChildNodes();
     for(int i=0;i<nl.getLength();i++){
       Node n = nl.item(i);
       String name = n.getNodeName();
       if(!n.equals("File") && !n.equals("#text") && !n.equals("#comment")){
         System.err.println("Unknown node name for files detected: " + name);
       } else {
          StringPair pair = new StringPair();
         // get the filename
         if(n.hasChildNodes()){
            String fn = n.getFirstChild().getNodeValue().trim();
            if(fn.length()>0){
               pair.first = fn;
            } 
         }
         if(pair.first==null){
            System.err.println("XMLFile corrupt: filename missing");
            return false;
         }
         // get the location
         NamedNodeMap m = n.getAttributes();
         Node loc  = m.getNamedItem("location");
         if(loc==null){
            System.err.println("XML-file corupted: location of a file is missing");
            return false;
         }
         String tmp = loc.getNodeValue().trim();
         while(tmp.startsWith("/")){
              tmp = tmp.substring(1,tmp.length()-1);
         }
         while(tmp.endsWith("/")){
             tmp = tmp.substring(0,tmp.length()-1);
         }
         if(tmp.length()==0){
            System.err.println("invalid value for location");
            return false;
         }
         pair.second = tmp;
         files.add(pair);  
       }
     }
     return true;
   } 

}


