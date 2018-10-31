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

#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <stdint.h>
#include <cstring>
#include <vector>

struct lasHeader{
   char signature[4];
   uint16_t source_id;
   uint16_t global_Encoding;
   uint32_t project_id1;
   uint16_t project_id2;
   uint16_t project_id3;
   unsigned char  project_id4[8];
   unsigned char major_version;
   unsigned char minor_version;
   char systemIdentifier[32];
   char generatingSoftware[32];
   uint16_t dayOfYear;
   uint16_t year;
   uint16_t header_size;
   uint32_t offset_to_point_data;
   uint32_t number_of_variable_records;
   unsigned char point_data_format;
   unsigned char point_data_length;
   uint32_t legacy_number_of_points;
   uint32_t legacy_number_of_points_by_return[5];
   double x_scale;
   double y_scale;
   double z_scale;
   double x_offset;
   double y_offset;
   double z_offset;
   double max_x;
   double min_x;
   double max_y;
   double min_y;
   double max_z;
   double min_z;
   uint64_t start_of_waveform_data_packet_record;
   uint64_t start_of_first_extended_vlr;
   uint32_t number_of_evlr;
   uint64_t number_of_point_records;
   uint64_t number_of_points_by_return[15];

   uint64_t numOfPoints() const{
     return minor_version>=4?number_of_point_records:legacy_number_of_points;
   }


   bool read(std::ifstream&  file);
   std::ostream& print(std::ostream& out) const;

};


std::ostream& operator<<(std::ostream& out, 
                 const lasHeader& header);



