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


public class ShapeHeader{

public void setFileCode(int Code){
   this.FileCode = Code;
}

public boolean readFrom(byte[] Buffer){
   if(Buffer.length!=100)
      return false;
   FileCode = NumericReader.getIntBig(Buffer,0);
   FileLength = NumericReader.getIntBig(Buffer,24);
   Version = NumericReader.getIntLittle(Buffer,28);
   ShapeType  = NumericReader.getIntLittle(Buffer,32);
   XMin = NumericReader.getDoubleLittle(Buffer,36);
   XMax = NumericReader.getDoubleLittle(Buffer,44);
   YMin = NumericReader.getDoubleLittle(Buffer,52);
   YMax = NumericReader.getDoubleLittle(Buffer,60);
   ZMin = NumericReader.getDoubleLittle(Buffer,68);
   ZMax = NumericReader.getDoubleLittle(Buffer,76);
   MMin = NumericReader.getDoubleLittle(Buffer,84);
   MMax = NumericReader.getDoubleLittle(Buffer,92);
   return true;


}

public void setFileLength(int length){
   this.FileLength = length;
}

public void setVersion(int Version){
   this.Version = Version;
}

public void setShapeType(int type){
   this.ShapeType=type;
}

public void setXMin(double XMin){
   this.XMin = XMin;
}
public void setXMax(double XMax){
   this.XMax = XMax;
}

public void setYMin(double YMin){
   this.YMin = YMin;
}

public void setYMax(double YMax){
   this.YMax = YMax;
}

public void setZMin(double ZMin){
   this.ZMin = ZMin;
}

public void setZMax(double ZMax){
   this.ZMax = ZMax;
}

public void setMMin(double MMin){
   this.MMin = MMin;
}

public void setMMax(double MMax){
   this.MMax = MMax;
}

public String toString(){
  return  "FileLength = " + FileLength +"\n"
   +"ShapeType =  " + extern.shapereader.ShapeType.getName(ShapeType) +"\n"
   +"Bounding Box = (" +XMin+","+YMin+") ->("+XMax+","+YMax+")";

}


public int getFileCode() {return FileCode;}
public int getFileLength() {return FileLength;}
public int getVersion() {return Version;}
public int getShapeType() {return ShapeType;}
public double getXMin() {return XMin;}
public double getXMax() {return XMax;}
public double getYMin() {return YMin;}
public double getYMax() {return YMax;}
public double getZMin() {return ZMin;}
public double getZMax() {return ZMax;}
public double getMMin() {return MMin;}
public double getMMax() {return MMax;}


private int FileCode;
private int FileLength;
private int Version;
private int ShapeType;
private double XMin;
private double YMin;
private double XMax;
private double YMax;
private double ZMin;
private double ZMax;
private double MMin;
private double MMax;

}
