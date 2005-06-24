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
import java.io.File;
import extern.binarylist.*;

public class ImportManager{


/** returns the content of the file in Nested-List format,
    if an error is occurred null is returned, the errormessage
    can be obtain with the getErrorText-Method.
    How the file is converted is determined by the file extension */
public ListExpr importFile(String FileName){
  long t1=System.currentTimeMillis();
  ErrorText = "no error";
  File F = new File(FileName);
     if(!F.exists()){
         ErrorText = "file not exists";
	 return null;
     }
  if(FileName.toLowerCase().endsWith(".dbf")){
     ListExpr Res = dbf3Reader.getList(FileName);
     if(Res!=null){
        // System.out.println("import of "+FileName+" has taken "+(System.currentTimeMillis()-t1) +" ms");
        return Res;
     }
     else{
        ErrorText = dbf3Reader.getErrorString();
     }
  }

  if(FileName.toLowerCase().endsWith(".shp")){
      String DBFile = FileName.substring(0,FileName.length()-3)+"dbf";
      File F2 = new File(DBFile);
      ListExpr Res;
     if(F2.exists()){ // try to load combined shape-dbf
         Res = shapedbfreader.getList(FileName);
	     if(Res==null){
             ErrorText = shapedbfreader.getErrorString();
	         System.out.println("combined shape-dbf failed :"+ErrorText);
	         t1 = System.currentTimeMillis();
	     }
	 	 else{
	         // System.out.println("import of "+FileName+" has taken "+(System.currentTimeMillis()-t1) +" ms");
	         return Res;
         }
     }
     Res = shapereader.getList(FileName);
     if(Res==null)
        ErrorText = shapereader.getErrorString();
     else{
        // System.out.println("import of "+FileName+" has taken "+(System.currentTimeMillis()-t1) +" ms");
            return Res;
     }
 }

  // a binary nested list
  if(FileName.toLowerCase().endsWith(".bnl")){
     BinaryList BN = new BinaryList();
     ListExpr LE = BN.getList(FileName);
     if(LE==null)
        ErrorText = BN.getErrorString();
     else{
        // System.out.println("import of "+FileName+" has taken "+(System.currentTimeMillis()-t1) +" ms");
        return extractFromObject(LE);
     }
  }

  // ever try to load this file as nested list
  t1 = System.currentTimeMillis();
  ListExpr R = ListExpr.getListExprFromFile(FileName);
  if(R==null){
     if(ErrorText.equals("no error"));
        ErrorText ="cannot load this file";
     return null;
  }else{
   // System.out.println("import of "+FileName+" has taken "+(System.currentTimeMillis()-t1) +" ms");
   return extractFromObject(R);
  }

}



private ListExpr extractFromObject(ListExpr LE){
   if(LE==null)
      return null;
   if(LE.listLength()==6 && LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
      LE.first().symbolValue().equals("OBJECT")){
      ListExpr res =ListExpr.twoElemList(LE.fourth(),LE.fifth());
      LE.first().destroy();
      LE.second().destroy();
      LE.third().destroy();
      LE.sixth().destroy();
      return res;
   }
   return LE;

}

/** set the maximal string length in all
  * importers which handle with strings
  */
public void setMaxStringLength(int len){
  if(len>0)
    dbf3Reader.setMaxStringLength(len);
}


private String ErrorText="";
private Dbf3Reader dbf3Reader = new Dbf3Reader();
private ShapeReader shapereader = new ShapeReader();
private ShapeDbf shapedbfreader = new ShapeDbf();



}
