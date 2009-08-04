
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

import java.util.Vector;
import java.util.TreeSet;
import java.io.InputStream;

import org.w3c.dom.*;
import javax.xml.parsers.*;


/** The class ExtensionInfo collects all extensions of a single plugin **/

public class ExtensionInfo{

  private boolean valid;                      // a valid Info ?
  private String fileName = null;             // filename of the corresponding zip file

  Vector<AlgebraInfo>   algebraInfos = new Vector<AlgebraInfo>(); // the algebra extensions
  Vector<ViewerInfo>    viewerInfos = new Vector<ViewerInfo>();   // the viewerExtensions
  Vector<HoeseInfo>     hoeseInfos = new Vector<HoeseInfo>();     // set of display classes
  Vector<OptimizerInfo> optInfos = new Vector<OptimizerInfo>();     // optimizer extensions


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


  /** Analyses the complete xml file **/
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
          AlgebraInfo algebraInfo = new AlgebraInfo(n);
          if(!algebraInfo.isValid()){
             System.err.println("XML file corrupted, error in reading algebra extension");
             return false;
          }
          algebraInfos.add(algebraInfo);
          isExtension = true;
       } else if(name.equals("Viewer")){
            ViewerInfo viewerInfo = new ViewerInfo(n);
            if(!viewerInfo.isValid()){
               System.err.println("ViewerInfo invalid");
               viewerInfo = null;
               return false;
            }
            viewerInfos.add(viewerInfo);
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
           OptimizerInfo oinfo = new OptimizerInfo(n);
           if(!oinfo.isValid()){
              System.err.println("Invalid Optimizer extension found");
              return false;
           }
           isExtension = true;
           optInfos.add(oinfo);
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
  public Vector<AlgebraInfo> getAlgebraInfos(){
     return algebraInfos;
  }

  /** Returns the ViewerInfos */
  public Vector<ViewerInfo> getViewerInfos(){
    return viewerInfos;
  }

  /** Returns the collected HoseInfos **/
  public Vector<HoeseInfo> getHoeseInfos(){
    return hoeseInfos;
  }

  /** Returns the stored OptimizerInfo **/
  public Vector<OptimizerInfo> getOptimizerInfos(){
     return optInfos;
  }

}


