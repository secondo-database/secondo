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

package extern.shapereader;
import extern.numericreader.*;

public class RecordHeader{
  
  public int getRecordNumber(){
     return RecordNumber;
  }

  public int getContentLength(){
     return ContentLength;
  }

  public boolean readFrom(byte[] Buffer){
     if(Buffer.length!=8)
        return false;
     RecordNumber = NumericReader.getIntBig(Buffer,0);
     ContentLength = NumericReader.getIntBig(Buffer,4);
     return true;
  }

  public String toString(){
     return "RecordNumber ="+RecordNumber+"\nContentLength="+ContentLength;
  }
   
  private int RecordNumber=0;
  private int ContentLength=0;
}
