

import java.util.*;
import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;
import java.util.zip.*;


public class OptimizerInfo extends SecondoExtension{

   Vector<Block> blocks;    // blocks for inserting
   Vector<String> files;   // new files

   public OptimizerInfo(Node n){
     valid = readOptimizerInfo(n);
   }

   private boolean readOptimizerInfo(Node n1){
      NodeList nl = n1.getChildNodes();
      for(int i=0; i< nl.getLength(); i++){
         Node n = nl.item(i);
         String name = n.getNodeName();
         if(name.equals("Dependencies")){
           if(!readDependencies(n)){
             return false;
           }
         } else if(name.equals("Copyright")){
           if(!readCopyright(n)){
              return false;
           }
         } else if(name.equals("File")){
           if(!n.hasChildNodes()){
              System.err.println("Empty file entry found");
              return false;
           }
           String fname = n.getFirstChild().getNodeValue().trim();
           if(fname.length()==0){
             System.err.println("empty file name foun");
             return false;
           }
           files.add(fname);
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
      return true;
   } 

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
     b.content = n.getNodeValue();
     if(b.content.trim().length()==0){
        System.err.println("empty block found");
        return false;
     }
     blocks.add(b);
     return true;
   }

}
