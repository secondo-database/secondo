/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Department of Computer Science,
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

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"  
#include "Symbols.h"
#include "Algebras/Stream/Stream.h"
#include "ListUtils.h"
#include <AlgebraTypes.h>
#include <Operator.h>
#include <fstream>
#include <iostream>
#include "PointCloud.h"
#include "ImportPointCloud.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace routeplanningalgebra {

    /*
    ~importpointcloud~ operator

    Imports LSA point data into stream(pointcloud)
    string -> stream(pointcloud)

    Added and maintained by Gundula Swidersky, Dec 2017 

    This file is currently not needed, but kept until the operator is fully
    implemented.
    Maybe some functions will be added in next weeks.

    */




   
} // End of namespace routeplanningalgebra

std::ostream& operator<<(std::ostream& out, 
                         const routeplanningalgebra::lasHeader& header){

    out << " ------------ LAS Header -----------------" << endl;
    out << "signature: " <<  string(header.signature,4) << endl;
    out << "source_id: " <<  header.source_id << endl;
    out << "global_Encoding: " <<  header.global_Encoding << endl;
    out << "project_id1: " <<  header.project_id1 << endl;
    out << "project_id2: " <<  header.project_id2 << endl;
    out << "project_id3: " <<  header.project_id3 << endl;
    for(int i=0;i<8;i++){
       out << "project_id4["<<i<<"]: " <<  (int)(header.project_id4[i])<< endl;
    }
    out << "major_version: " <<  (int)header.major_version << endl;
    out << "minor_version: " <<  (int) header.minor_version << endl;
    out << "systemIdentifier[32]: " <<  string(header.systemIdentifier,32) 
        << endl;
    out << "generatingSoftware[32]: " <<  string(header.generatingSoftware,32)
        << endl;
    out << "dayOfYear: " <<  header.dayOfYear << endl;
    out << "year: " <<  header.year << endl;
    out << "header_size: " <<  header.header_size << endl;
    out << "offset_to_point_data: " <<  header.offset_to_point_data << endl;
    out << "number_of_variable_records: " <<  header.number_of_variable_records
        << endl;
    out << "point_data_format: " <<  (int)header.point_data_format << endl;
    out << "point_data_length: " <<  (int) header.point_data_length << endl;
    out << "legacy_number_of_points: " <<  header.legacy_number_of_points
        << endl;
    for(int i=0;i<5;i++){
      out << "legacy_number_of_points_by_return["<<i<<"]: " 
          <<  header.legacy_number_of_points_by_return[i] << endl;
    }
    out << "x_scale: " <<  header.x_scale << endl;
    out << "y_scale: " <<  header.y_scale << endl;
    out << "z_scale: " <<  header.z_scale << endl;
    out << "x_offset: " <<  header.x_offset << endl;
    out << "y_offset: " <<  header.y_offset << endl;
    out << "z_offset: " <<  header.z_offset << endl;
    out << "max_x: " <<  header.max_x << endl;
    out << "min_x: " <<  header.min_x << endl;
    out << "max_y: " <<  header.max_y << endl;
    out << "min_y: " <<  header.min_y << endl;
    out << "max_z: " <<  header.max_z << endl;
    out << "min_z: " <<  header.min_z << endl;
    if(header.major_version > 1 || header.minor_version >= 4){
       out << "start_of_waveform_data_packet_record: " 
           <<  header.start_of_waveform_data_packet_record << endl;
       out << "start_of_first_extended_vlr: " 
           <<  header.start_of_first_extended_vlr << endl;
       out << "number_of_vlr: " <<  header.number_of_vlr << endl;
       out << "number_of_point_records: " <<  header.number_of_point_records
           << endl;
       for(int i=0;i<15;i++){
          out << "number_of_points_by_return["<<i<<"]: " 
              <<  header.number_of_points_by_return[i] << endl;
       }
    }
    out << " ------------ LAS Header END -----------------" << endl;
    return out;
 }

