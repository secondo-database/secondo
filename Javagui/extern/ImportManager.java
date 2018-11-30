//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package extern;

import sj.lang.ListExpr;
import extern.dbfreader.Dbf3Reader;
import extern.shapedbf.ShapeDbf;
import extern.shapereader.ShapeReader;
import extern.stlreader.StlReader;
import extern.gpxreader.GpxReader;
import extern.nl.NLImporter;
import java.io.File;
import extern.binarylist.*;
import tools.Reporter;
import java.util.Vector;

public class ImportManager{


/** returns the content of the file in Nested-List format,
    if an error is occurred null is returned, the errormessage
    can be obtain with the getErrorText-Method.
    How the file is converted is determined by the file extension */
public ListExpr importFile(String fileName){
  error = "";
  File F = new File(fileName);
  if(!F.exists()){
    error = "file does not exists";
    return null;
  }
  for(int i=0;i<importers.size();i++){
    SecondoImporter imp = importers.get(i);
    if(imp.supportsFile(F)){
      ListExpr res = imp.getList(fileName);
      if(res!=null){
         error = "no error";
         return res;
      } else {
        if(error.length()>0) error += "\n";
        error += imp.getFileDescription()+ " : " +imp.getErrorString();
      }
    }
  }
  return null;
}



public static ListExpr extractFromObject(ListExpr LE){
   if(LE==null)
      return null;
   int length = LE.listLength();
   if(length==6 && LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
      LE.first().symbolValue().equals("OBJECT")){
      ListExpr res =ListExpr.twoElemList(LE.fourth(),LE.fifth());
      LE.first().destroy();
      LE.second().destroy();
      LE.third().destroy();
      LE.sixth().destroy();
      if(!gui.Environment.OLD_OBJECT_STYLE){
        Reporter.writeWarning("Old styled ListExpr found");
      }
      return res;
   }
   if(length==5 && LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
      LE.first().symbolValue().equals("OBJECT")){
      ListExpr res =ListExpr.twoElemList(LE.fourth(),LE.fifth());
      LE.first().destroy();
      LE.second().destroy();
      LE.third().destroy();
      return res;
   }
   return LE;

}

/** set the maximal string length in all
  * importers which handle with strings
  */
public void setMaxStringLength(int len){
  if(len > 0){
    for(int i=0;i<importers.size();i++){
      importers.get(i).setMaxStringLength(len);
    }
  }
}


public Vector<String> getSupportedFormats(){
  return supportedFormats;
}


public ImportManager(){
   importers = new Vector<SecondoImporter>();
   importers.add(new ShapeDbf()); 
   importers.add(new Dbf3Reader());
   importers.add(new ShapeReader()); 
   importers.add(new StlReader());
   importers.add(new BinaryList());
   importers.add(new GpxReader());
   importers.add(new NLImporter()); 

   supportedFormats = new Vector<String>();
   for(int i=0;i<importers.size();i++){
     supportedFormats.add(importers.get(i).getFileDescription());
   }
}



private String error="";
Vector<SecondoImporter> importers;
private Vector<String> supportedFormats;


}
