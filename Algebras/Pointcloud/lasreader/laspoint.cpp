/*
----
This file is part of SECONDO.

Copyright (C) 2018, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#include "laspoint.h"

/*
1 Implementation of laspoint

*/

bool lasPoint::read(std::ifstream& in, 
                  size_t recordSize){
   in.read( reinterpret_cast<char*>(&X),4);
   in.read( reinterpret_cast<char*>(&Y),4);
   in.read( reinterpret_cast<char*>(&Z),4);
   in.read( reinterpret_cast<char*>(&Intensity),2);
   size_t ignore = recordSize - lasPoint::ownsize();
   if(ignore > 0){
     in.seekg(ignore, std::ios::cur);
   }
   defined =  in.good(); 
   return defined;
}


std::ostream& lasPoint::print(std::ostream& out) const{
   if(!defined){
     out << "undefined" << std::endl;
     return out;
   }
   out << "X = " << X << std::endl;
   out << "Y = " << Y << std::endl;
   out << "Z = " << Z << std::endl;
   out << "Intensity " << Intensity << std::endl;
   return out;
}

/*
2 lasPoint0 implementation

*/

bool lasPoint0::read(std::ifstream& in, size_t recordSize){
   if(!lasPoint::read(in,lasPoint::ownsize())){
      return false;
   }
   uint8_t data;
   in.read(reinterpret_cast<char*>(&data),1);
   uint8_t mask = 7; // last 3 bits
   returnNumber = data & mask;
   numberOfReturns = (data << 3 ) & mask;
   mask = 1 << 5;
   scanDirectionFlag = (data & mask) >0;
   mask = 1 << 6;
   edgeOfFlightLine = (data & mask) > 0;
   in.read(reinterpret_cast<char*>(&classification),1);
   in.read(reinterpret_cast<char*>(&scan_angle_rank),1);
   in.read(reinterpret_cast<char*>(&user_data),1);
   in.read(reinterpret_cast<char*>(&point_source_id),2);
   size_t ignore = recordSize - lasPoint0::ownsize();
   if(ignore > 0){
     in.seekg(ignore, std::ios::cur);
   }
   defined = in.good(); 
   return defined;
}


std::ostream& lasPoint0::print(std::ostream& out) const{
   if(!defined){
     out << "undefined" << std::endl;
     return out;
   }
   lasPoint::print(out);
   out << "return number = " << (int)returnNumber << std::endl;
   out << "number of returns = " << (int)numberOfReturns << std::endl;
   out << "scan direction flag = " << (scanDirectionFlag?"true":"false") 
       << std::endl;
   out << "edge of flight line = " << (edgeOfFlightLine?"true":"false")
       << std::endl;
   out << "classification = " << (int) classification << std::endl;
   out << "scan angle rank; = " << (int) scan_angle_rank << std::endl;
   out << "user data = " << (int) user_data << std::endl;
   out << "point source id = " << point_source_id << std::endl;
   return out;
}

/*
3 lasPoint1 implementation

*/

bool lasPoint1::read(std::ifstream& in, size_t recordSize){
   if(!lasPoint0::read(in,lasPoint0::ownsize())){
     return false;
   }
   in.read(reinterpret_cast<char*>(&gps_time),8);
   size_t ignore = recordSize - lasPoint1::ownsize();
   if(ignore > 0){
     in.seekg(ignore, std::ios::cur);
   }
   defined = in.good(); 
   return defined;
}

std::ostream& lasPoint1::print(std::ostream& out) const{
   if(!defined){
     out << "undefined" << std::endl;
     return out;
   }
   lasPoint0::print(out);
   out << "gps time = " << gps_time << std::endl;
   return out;
}


/*
3 lasPoint2 implementation

*/

bool lasPoint2::read(std::ifstream& in, size_t recordSize){
   if(!lasPoint0::read(in,lasPoint0::ownsize())){
     return false;
   }
   in.read(reinterpret_cast<char*>(&red),2);
   in.read(reinterpret_cast<char*>(&green),2);
   in.read(reinterpret_cast<char*>(&blue),2);
   size_t ignore = recordSize - lasPoint2::ownsize();
   if(ignore > 0){
     in.seekg(ignore, std::ios::cur);
   }
   defined = in.good(); 
   return defined;
}

std::ostream& lasPoint2::print(std::ostream& out) const{
   if(!defined){
     out << "undefined" << std::endl;
     return out;
   }
   lasPoint0::print(out);
   out << "red   = " << red << std::endl;
   out << "green = " << green << std::endl;
   out << "blue  = " << blue << std::endl;
   return out;
}

/*
4 lasPoint3 implementation

*/

bool lasPoint3::read(std::ifstream& in, size_t recordSize){
   if(!lasPoint1::read(in,lasPoint1::ownsize())){
     return false;
   }
   in.read(reinterpret_cast<char*>(&red),2);
   in.read(reinterpret_cast<char*>(&green),2);
   in.read(reinterpret_cast<char*>(&blue),2);
   size_t ignore = recordSize - lasPoint3::ownsize();
   if(ignore > 0){
     in.seekg(ignore, std::ios::cur);
   }
   defined = in.good(); 
   return defined;
}

std::ostream& lasPoint3::print(std::ostream& out) const{
   if(!defined){
     out << "undefined" << std::endl;
     return out;
   }
   lasPoint1::print(out);
   out << "red   = " << red << std::endl;
   out << "green = " << green << std::endl;
   out << "blue  = " << blue << std::endl;
   return out;
}


/*
lasPoint4

*/

bool lasPoint4::read(std::ifstream& in, size_t recordSize){
   if(!lasPoint1::read(in,lasPoint1::ownsize())){
     return false;
   }
   in.read(reinterpret_cast<char*>(&wavePacketDescriptorIndex),1);
   in.read(reinterpret_cast<char*>(&byteOffsetToWaveformData),8);
   in.read(reinterpret_cast<char*>(&waveFormPacketSize),4);
   in.read(reinterpret_cast<char*>(&returnPointWaveformLocation),4);
   in.read(reinterpret_cast<char*>(&x_t),4);
   in.read(reinterpret_cast<char*>(&y_t),4);
   in.read(reinterpret_cast<char*>(&z_t),4);
   
   size_t ignore = recordSize - lasPoint4::ownsize();
   if(ignore > 0){
     in.seekg(ignore, std::ios::cur);
   }
   defined = in.good(); 
   return defined;
}

std::ostream& lasPoint4::print(std::ostream& out) const{
   if(!defined){
     out << "undefined" << std::endl;
     return out;
   }
   lasPoint1::print(out);
   out << "wave packet descriptor index  =" 
       << (int) wavePacketDescriptorIndex << std::endl;
   out << "byte offset to waveform data  =" 
       << byteOffsetToWaveformData << std::endl;
   out << "waveform packet size in bytes =" 
       << waveFormPacketSize << std::endl;
   out << "return point waveform location " 
       << returnPointWaveformLocation << std::endl;
   out << "X(t) = " << x_t << std::endl;
   out << "Y(t) = " << y_t << std::endl;
   out << "Z(t) = " << z_t << std::endl;
   return out;
}

/*
lasPoint5

*/

bool lasPoint5::read(std::ifstream& in, size_t recordSize){
   if(!lasPoint3::read(in,lasPoint3::ownsize())){
     return false;
   }
   in.read(reinterpret_cast<char*>(&wavePacketDescriptorIndex),1);
   in.read(reinterpret_cast<char*>(&byteOffsetToWaveformData),8);
   in.read(reinterpret_cast<char*>(&waveFormPacketSize),4);
   in.read(reinterpret_cast<char*>(&returnPointWaveformLocation),4);
   in.read(reinterpret_cast<char*>(&x_t),4);
   in.read(reinterpret_cast<char*>(&y_t),4);
   in.read(reinterpret_cast<char*>(&z_t),4);
   
   size_t ignore = recordSize - lasPoint5::ownsize();
   if(ignore > 0){
     in.seekg(ignore, std::ios::cur);
   }
   defined = in.good(); 
   return defined;
}

std::ostream& lasPoint5::print(std::ostream& out) const{
   if(!defined){
     out << "undefined" << std::endl;
     return out;
   }
   lasPoint3::print(out);
   out << "wave packet descriptor index  =" 
       << (int) wavePacketDescriptorIndex << std::endl;
   out << "byte offset to waveform data  =" 
       << byteOffsetToWaveformData << std::endl;
   out << "waveform packet size in bytes =" 
       << waveFormPacketSize << std::endl;
   out << "return point waveform location " 
       << returnPointWaveformLocation << std::endl;
   out << "X(t) = " << x_t << std::endl;
   out << "Y(t) = " << y_t << std::endl;
   out << "Z(t) = " << z_t << std::endl;
   return out;
}







lasPoint* getLasPoint(int format){
  if(format < 0) return 0;
  switch(format){
    case 0  : return new lasPoint0();
    case 1  : return new lasPoint1();
    case 2  : return new lasPoint2();
    case 3  : return new lasPoint3();
    case 4  : return new lasPoint4();
    case 5  : return new lasPoint5();
    case 6  : return new lasPoint(); // not implented yet
    case 7  : return new lasPoint(); // not implented yet;
    case 8  : return new lasPoint(); // not implented yet;
    case 9  : return new lasPoint(); // not implented yet;
    case 10 : return new lasPoint(); // not implented yet;
    default : return 0;
  } 
}



