//This file is part of SECONDO.

//Copyright (C) 2018, University in Hagen, 
//Faculty of Mathematics and Computer Science, 
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

package extern.nl;
import sj.lang.ListExpr;
import java.io.File;
import extern.*;

public class NLImporter implements SecondoImporter{
  private String error = "";
  
  public String getFileDescription(){ return "Nested List (AscII)"; }

  public void setMaxStringLength(int len){
    // handled by nl module
  }

  public String getErrorString(){
    return error;
  }

  public boolean supportsFile(File f){
     return !f.isDirectory();
  }
  
  public ListExpr getList(String fileName){
     ListExpr l = ListExpr.getListExprFromFile(fileName); 
     if(l==null){
       error = "cannot load the file"; 
       return null;
     }
     error = "no error";
     return ImportManager.extractFromObject(l);
  }

}


