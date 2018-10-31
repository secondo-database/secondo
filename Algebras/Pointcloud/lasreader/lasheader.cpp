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


#include "lasheader.h"
#include <assert.h>

/*
1 Implementation of las components

*/
   bool lasHeader::read(std::ifstream&  file){
      file.read(reinterpret_cast<char*>(&signature),  4);
      if( (std::string(signature,4)!="LASF")){
         return false;
      }
      file.read(reinterpret_cast<char*>(&source_id), 2);
      file.read(reinterpret_cast<char*>(&global_Encoding),2);
      file.read(reinterpret_cast<char*>(&project_id1),4);
      file.read(reinterpret_cast<char*>(&project_id2),2);
      file.read(reinterpret_cast<char*>(&project_id3),2);
      file.read(reinterpret_cast<char*>(&project_id4),8);
      file.read(reinterpret_cast<char*>(&major_version),1);
      file.read(reinterpret_cast<char*>(&minor_version), 1);
      file.read(systemIdentifier, 32);
      file.read(generatingSoftware, 32);
      file.read(reinterpret_cast<char*>(&dayOfYear), 2);
      file.read(reinterpret_cast<char*>(&year), 2);
      file.read(reinterpret_cast<char*>(&header_size), 2);
      file.read(reinterpret_cast<char*>(&offset_to_point_data), 4);
      file.read(reinterpret_cast<char*>(&number_of_variable_records), 4);
      file.read(reinterpret_cast<char*>(&point_data_format), 1);
      file.read(reinterpret_cast<char*>(&point_data_length), 2);
      file.read(reinterpret_cast<char*>(&legacy_number_of_points), 4);
      file.read(reinterpret_cast<char*>(
                            &legacy_number_of_points_by_return), 20);
      file.read(reinterpret_cast<char*>(&x_scale), 8);
      file.read(reinterpret_cast<char*>(&y_scale), 8);
      file.read(reinterpret_cast<char*>(&z_scale), 8);
      file.read(reinterpret_cast<char*>(&x_offset), 8);
      file.read(reinterpret_cast<char*>(&y_offset), 8);
      file.read(reinterpret_cast<char*>(&z_offset), 8);
      file.read(reinterpret_cast<char*>(&max_x), 8);
      file.read(reinterpret_cast<char*>(&min_x), 8);
      file.read(reinterpret_cast<char*>(&max_y), 8);
      file.read(reinterpret_cast<char*>(&min_y), 8);
      file.read(reinterpret_cast<char*>(&max_z), 8);
      file.read(reinterpret_cast<char*>(&min_z), 8);
      if(major_version==1 && minor_version < 4){
        start_of_waveform_data_packet_record = 0;
        start_of_first_extended_vlr = 0;
        number_of_evlr = 0;
        number_of_point_records = 0;
        memset(reinterpret_cast<char*>(number_of_points_by_return),0,120);
      } else {
        file.read(reinterpret_cast<char*>(
                               &start_of_waveform_data_packet_record), 8);
        file.read(reinterpret_cast<char*>(&start_of_first_extended_vlr), 8);
        file.read(reinterpret_cast<char*>(&number_of_evlr), 4);
        file.read(reinterpret_cast<char*>(&number_of_point_records), 8);
        file.read(reinterpret_cast<char*>(&number_of_points_by_return),
                                          120);
     }

      return true;
   }

   std::ostream& lasHeader::print(std::ostream& out) const{
    out << " ------------ LAS Header -----------------" << std::endl;
    out << "signature: " <<  std::string(signature,4) << std::endl;
    out << "source_id: " <<  source_id << std::endl;
    out << "global_Encoding: " <<  global_Encoding << std::endl;
    out << "project_id1: " <<  project_id1 << std::endl;
    out << "project_id2: " <<  project_id2 << std::endl;
    out << "project_id3: " <<  project_id3 << std::endl;
    for(int i=0;i<8;i++){
       out << "project_id4["<<i<<"]: " <<  (int)(project_id4[i])<< std::endl;
    }
    out << "major_version: " <<  (int)major_version << std::endl;
    out << "minor_version: " <<  (int) minor_version << std::endl;
    out << "systemIdentifier[32]: " <<  std::string(systemIdentifier,32) 
        << std::endl;
    out << "generatingSoftware[32]: " <<  std::string(generatingSoftware,32)
        << std::endl;
    out << "dayOfYear: " <<  dayOfYear << std::endl;
    out << "year: " <<  year << std::endl;
    out << "header_size: " <<  header_size << std::endl;
    out << "offset_to_point_data: " <<  offset_to_point_data << std::endl;
    out << "number_of_variable_records: " <<  number_of_variable_records
        << std::endl;
    out << "point_data_format: " <<  (int)point_data_format << std::endl;
    out << "point_data_length: " <<  (int) point_data_length << std::endl;
    out << "legacy_number_of_points: " <<  legacy_number_of_points
        << std::endl;
    for(int i=0;i<5;i++){
      out << "legacy_number_of_points_by_return["<<i<<"]: " 
          <<  legacy_number_of_points_by_return[i] << std::endl;
    }
    out << "x_scale: " <<  x_scale << std::endl;
    out << "y_scale: " <<  y_scale << std::endl;
    out << "z_scale: " <<  z_scale << std::endl;
    out << "x_offset: " <<  x_offset << std::endl;
    out << "y_offset: " <<  y_offset << std::endl;
    out << "z_offset: " <<  z_offset << std::endl;
    out << "max_x: " <<  max_x << std::endl;
    out << "min_x: " <<  min_x << std::endl;
    out << "max_y: " <<  max_y << std::endl;
    out << "min_y: " <<  min_y << std::endl;
    out << "max_z: " <<  max_z << std::endl;
    out << "min_z: " <<  min_z << std::endl;
    if(major_version > 1 || minor_version >= 4){
       out << "start_of_waveform_data_packet_record: " 
           <<  start_of_waveform_data_packet_record << std::endl;
       out << "start_of_first_extended_vlr: " 
           <<  start_of_first_extended_vlr << std::endl;
       out << "number_of_evlr: " <<  number_of_evlr << std::endl;
       out << "number_of_point_records: " <<  number_of_point_records
           << std::endl;
       for(int i=0;i<15;i++){
          out << "number_of_points_by_return["<<i<<"]: " 
              <<  number_of_points_by_return[i] << std::endl;
       }
    }
    out << " ------------ LAS Header END -----------------" << std::endl;
    return out;
   }

  std::ostream& operator<<(std::ostream& out, const lasHeader& header){
    return header.print(out);
  }





