/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



*/

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <stdint.h>


class lasPoint{
  public:
    lasPoint(): defined(false),X(0),Y(0),Z(0),Intensity(0) {
    }

    virtual ~lasPoint(){}

    virtual bool read(std::ifstream& in, 
                      size_t recordSize);

    static uint32_t ownsize() {
      return 14; 
    }

    virtual std::ostream& print(std::ostream& out) const;

   // members
    bool defined;
    int32_t X;
    int32_t Y;
    int32_t Z;
    uint16_t Intensity;
};

/*
Point class 0 contains all common members of
point data format 0..5

*/
class lasPoint0 : public lasPoint{
  public:
    uint8_t returnNumber;
    uint8_t numberOfReturns;
    bool scanDirectionFlag;
    bool edgeOfFlightLine;
    unsigned char classification;
    char scan_angle_rank;
    unsigned char user_data;
    uint16_t point_source_id;

    lasPoint0():lasPoint(), 
        returnNumber(0),
        numberOfReturns(0),
        scanDirectionFlag(false),
        edgeOfFlightLine(false),
        classification(0),
        scan_angle_rank(0),
        user_data(0),
        point_source_id(0) {}

    virtual bool read(std::ifstream& in, size_t recordSize);

    static uint32_t ownsize(){
      return lasPoint::ownsize() + 6;
    }

    virtual std::ostream& print(std::ostream& out) const;
};


/*
Class representing point data format 1

*/
class lasPoint1 : public lasPoint0{
  public:
    double gps_time;

    virtual bool read(std::ifstream& in, size_t recordSize);
    
    static uint32_t ownsize(){
      return lasPoint0::ownsize() + 8;
    }

    virtual std::ostream& print(std::ostream& out) const;
};


/*
Class representing point data format 2

*/
class lasPoint2 : public lasPoint0{
  public:
    uint16_t red;
    uint16_t green;
    uint16_t blue; 

    virtual bool read(std::ifstream& in, size_t recordSize);
    
    static uint32_t ownsize(){
      return lasPoint0::ownsize() + 6;
    }

    virtual std::ostream& print(std::ostream& out) const;
};


/*
 data format 3

*/
class lasPoint3 : public lasPoint1{
  public:
    uint16_t red;
    uint16_t green;
    uint16_t blue;

    virtual bool read(std::ifstream& in, size_t recordSize);
    
    static uint32_t ownsize(){
      return lasPoint1::ownsize() + 6;
    }

    virtual std::ostream& print(std::ostream& out) const;
};


/*
data format 4

*/
class lasPoint4 : public lasPoint1{
  public:
     uint8_t wavePacketDescriptorIndex;
     uint64_t byteOffsetToWaveformData;
     uint32_t waveFormPacketSize;
     float returnPointWaveformLocation;
     float x_t;
     float y_t;
     float z_t;

    virtual bool read(std::ifstream& in, size_t recordSize);
    
    static uint32_t ownsize(){
      return lasPoint1::ownsize() + 29;
    }

    virtual std::ostream& print(std::ostream& out) const;
};

/*
data format 5

*/
class lasPoint5 : public lasPoint3{
  public:
     uint8_t wavePacketDescriptorIndex;
     uint64_t byteOffsetToWaveformData;
     uint32_t waveFormPacketSize;
     float returnPointWaveformLocation;
     float x_t;
     float y_t;
     float z_t;

    virtual bool read(std::ifstream& in, size_t recordSize);
    
    static uint32_t ownsize(){
      return lasPoint3::ownsize() + 29;
    }

    virtual std::ostream& print(std::ostream& out) const;
};


/*
Returns a point having the best subclass for the 
given format or 0 if format is unknown.
The return type depends on the available implentations.

*/
lasPoint* getLasPoint(int format); 










