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
#include <vector>

#include "vlr.h"
#include "lasheader.h"
#include "../../Pointcloud/lasreader/prjconvert/geoprojectionconverter.hpp"
#include "laspoint.h"

class lasPoint;


class lasreader{
public:
   lasreader(const std::string& _filename) : filename(_filename){
     init();
   }

   ~lasreader();

   // returns the coordinates as lat long on wgs 84 ellipsoid
   // valid checks whether the point is inside the 
   // bounding box given in header
   // the return value says whether the components 
   // could be computed successfully
   bool toLatLon(lasPoint* point, 
                 double& x, 
                 double& y, 
                 double& z, 
                 bool& valid);

   // computes the lasPoint into valid coordinates 
   // according to the scale and offset information
   // from the las header. Returns false if the 
   // point is null
   bool getCoordinates(lasPoint* point,
                 double& x, 
                 double& y, 
                 double& z, 
                 bool& valid);

   // returns the next point record
   lasPoint* next(); 

   bool isOk() const{
     return ok;
   }

   void printHeader(std::ostream& out) const{
     if(!ok) {
       out << "invalid " << std::endl;
     } else {
        header.print(out);
     }
   }
   

   void printVLRs(std::ostream& out) const{
     if(!ok) {
       out << "no vlrs" << std::endl;
     } else {
       out << " ---- variable length records ----- " << std::endl; 
       for(size_t i=0; i< knownvlrs.size() ; i++){
         knownvlrs[i]->print(out);
         out << "  -----" << std::endl;
       }
       out << " ---- end of variable length records ----- " << std::endl;       
     }
   }

   bool getBox(double& min_x, double& min_y, double& min_z,
               double& max_x, double& max_y, double& max_z){
     min_x = this->min_x;
     min_y = this->min_y;
     min_z = this->min_z;
     max_x = this->max_x;
     max_y = this->max_y;
     max_z = this->max_z;
     return ok;
   }

   size_t getNumPoints() const{
      return ok?header.numOfPoints(): 0; 
   }

   int getPointFormat(){
      return header.point_data_format;
   }


private:
  std::string filename;  
  std::ifstream file;
  bool ok;
  lasHeader header;
  uint64_t points_read;
  uint64_t toRead;
  std::vector<vlr*> knownvlrs;
  GeoProjectionConverter* converter;

  // box in wgs84 
  double min_x;
  double min_y;
  double min_z;
  double max_x;
  double max_y;
  double max_z;
   


  void  init();
  // creates a projection converter from the known 
  // variable length records
  void createConverter();

  bool readVLRs();
  bool readEVLRs();


  // sets box from header using converter
  void setBox();

};

