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

package tools;

import extern.*;
import sj.lang.ListExpr;

public class xxx2secondo{

private static void convertFile(String FileName){

  ListExpr LE;
  System.out.println("convert File: "+FileName);
  LE = IM.importFile(FileName);
  if(LE==null){
     System.err.println("error in converting File: "+FileName);
     return;
  }
  String ObjectName;
  int ExtPos = FileName.lastIndexOf(".");
  if(ExtPos>0)
      ObjectName = FileName.substring(0,ExtPos).trim();
  else
      ObjectName = FileName;
  int PathPos;
  PathPos = Math.max(ObjectName.lastIndexOf("/"),ObjectName.lastIndexOf("\\"));
  if(PathPos>=0)
     ObjectName = ObjectName.substring(PathPos+1);
  // replace not allowed characters
  ObjectName = ObjectName.replace(' ','_');
  ObjectName = ObjectName.replace('-','_');
  ObjectName = ObjectName.replace('+','_');
  ObjectName = ObjectName.replace('.','_');
  ListExpr outList;
  if(LE.listLength()!=2)
      outList=LE;
  else
      if(OldStyle){
           outList = ListExpr.sixElemList(ListExpr.symbolAtom("OBJECT"),
                                          ListExpr.symbolAtom(ObjectName),
                         ListExpr.theEmptyList(),
                         LE.first(),
                         LE.second(),
                         ListExpr.theEmptyList());
       } else{ // new style
           outList = ListExpr.fiveElemList(ListExpr.symbolAtom("OBJECT"),
                                          ListExpr.symbolAtom(ObjectName),
                         ListExpr.theEmptyList(),
                         LE.first(),
                         LE.second()
                         );
       }

  String outFileName = FileName+".obj";
  if(outList.writeToFile(outFileName)!=0)
     System.err.println("error in writing file "+outFileName);

}


public static void main(String[] args){
  ListExpr.initialize(500000);
  IM = new ImportManager();
  int START=0;
  if(args.length>0 && args[0].equals("-oldstyle")){
      OldStyle=true;
      START++;
  }
     
  for(int i=START;i<args.length;i++)
      convertFile(args[i]);
}

private static boolean OldStyle = false;
private static ImportManager IM;

}
