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

package extern.binarylist;

import sj.lang.*;
import java.io.*;
import extern.*;

public class BinaryList implements SecondoImporter{


public ListExpr getList(String FileName){
  File F = new File(FileName);
  if(!F.exists()){
     LastError = "file not found";
     return null;
  }
  LastError="no error";
  try{
     BufferedInputStream FIS = new BufferedInputStream(new FileInputStream(F),4096);
     if(FIS==null)
        LastError = "error in reading file";
     ListExpr Res = ListExpr.readBinaryFrom(FIS);
     try{FIS.close();}catch(Exception e2){}
     return Res;
     } catch(Exception E){
        LastError = "error in opening file";
        return null;
     }
}



public String getErrorString(){
   return LastError;
}


private String LastError;


}
