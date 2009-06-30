import java.util.Vector;
import java.util.TreeSet;
import java.io.InputStream;

import org.w3c.dom.*;
import javax.xml.parsers.*;

public class ExtensionInfo{

  private boolean valid;                      // a valid Info ?
  private String fileName = null;             // filename of the corresponding zip file
  AlgebraInfo algebraInfo = null;
  ViewerInfo viewerInfo = null;
  Vector<HoeseInfo> hoeseInfos = new Vector<HoeseInfo>();


  /** Creates a new ExtensionInfo **/
  public  ExtensionInfo(String extensionFile, InputStream i){
    valid = readDocument(i);
    fileName = extensionFile;
  }

  /** Check for validity **/
  public boolean isValid(){
    return valid;
  }

  /** Reads the complete xml file **/
  private boolean readDocument(InputStream i){
    try{
      Document d = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(i);
      return processDoc(d);
    } catch( Exception e ) {
      e.printStackTrace();
      return false;
    }
  }


  /** Analyses the complte xml file **/
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
    for(int i=0; i< nl.getLength();i++){
       Node n = nl.item(i);
       String name = n.getNodeName();
       if(name.equals("Algebra")){
          if(algebraInfo!=null){
             System.err.println("only one algebra per module possible");
             return false;
          }
          algebraInfo = new AlgebraInfo(n);
          if(!algebraInfo.isValid()){
             System.err.println("XML file corrupted, error in reading algebra extension");
             return false;
          }
          isExtension = true;
       } else if(name.equals("Viewer")){
            if(viewerInfo!=null){
                System.err.println("module contains more than one viewer section");
                return false;
            }
            viewerInfo = new ViewerInfo(n);
            if(!viewerInfo.isValid()){
               System.err.println("ViewerInfo invalid");
               viewerInfo = null;
               return false;
            }
            isExtension = true;
       } else if(name.equals("HoeseExtension")){
          HoeseInfo hoeseInfo = new HoeseInfo(n);
          if(!hoeseInfo.isValid()){
              System.out.println("Invalid HoeseViewer extension found");
              return false;
          } 
          isExtension = true;
          hoeseInfos.add(hoeseInfo);
       } else if(name.equals("Optimizer")){

       } else if(name.equals("Kernel")){
          System.err.println("Kernel not supported yet");

       } else if(!name.equals("#text")){
           System.err.println("unknown entry " + name );
       }
     }
     return isExtension;
  }

  /** Returns the names the zip file **/
  public String getFileName(){
     return fileName;
  }

  /** Returns the AlgebraInfo */
  public AlgebraInfo getAlgebraInfo(){
     return algebraInfo;
  }

  /** Returns the ViewerInfo */
  public ViewerInfo getViewerInfo(){
    return viewerInfo;
  }

  /** Returns the collected HoseInfos **/
  public Vector<HoeseInfo> getHoeseInfos(){
    return hoeseInfos;
  }


}


