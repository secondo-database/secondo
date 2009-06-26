
import java.util.*;
import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;
import java.util.zip.*;
import java.util.regex.Pattern;


public class SecondoExtension{

   protected boolean valid;
   protected int secondo_Major_Version = -1;     // version informtion
   protected int secondo_Minor_Version = 0;      // version information
   protected int secondo_SubMinor_Version = 0;   // version information
   protected String copyright=null;

   boolean filesPresent(ZipFile f, Vector<String> names){
     for(int i=0;i<names.size();i++){
        if(f.getEntry(names.get(i))==null){
          System.err.println("Entry " + names.get(i) + " not found");
          return false;
        }
     }
     return  true;
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
}
