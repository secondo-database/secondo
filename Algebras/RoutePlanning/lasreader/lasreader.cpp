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
#include "lasreader.h"


/*
Implementation of class lasreader.

*/

void lasreader::init(){
   points_read = 0;
   toRead = 0;
   converter = 0;
   file.open(filename.c_str(), std::ios::in | std::ios::binary);
   ok = file.good();
   if(ok){
     ok = header.read(file);
   }
   if(ok){
      ok = readVLRs();
   }
   if(ok){
      ok = readEVLRs();
   }
   createConverter();
   if(ok){
      file.seekg(header.offset_to_point_data, std::ios::beg);
   }
   toRead = header.numOfPoints();
   setBox();
}

// reads the box from header and 
// -- if converter is present -- converts into wgs84 coordinates
void lasreader::setBox() {
   min_x = header.min_x; 
   min_y = header.min_x;
   min_z = header.min_z;
   max_x = header.max_x; 
   max_y = header.max_x;
   max_z = header.max_z;
   if(converter){
      double p1[] = {min_x,min_y,min_z};
      ok = ok && converter->to_lon_lat_ele(p1,min_x,min_y,min_z);
      double p2[] = {max_x,max_y,max_z};
      ok = ok && converter->to_lon_lat_ele(p2,max_x,max_y,max_z);
   } 
}


void lasreader::createConverter(){
   converter = 0;
   // check for projection related vlrs and store them into
   // variables
   converter = new GeoProjectionConverter();
   std::string userId = "LASF_Projection";
   bool set = false;

   LASF_Projection_34735* prj_34735 = 0;
   LASF_Projection_34736* prj_34736 = 0;
   LASF_Projection_34737* prj_34737 = 0;
  

   for(size_t i=0;i<knownvlrs.size();i++){
      vlr* current_vlr = knownvlrs[i];
      if(current_vlr->check(userId,2111)){
          set = set || converter->set_projection_from_ogc_wkt(
                             ((LASF_Projection_2111*)current_vlr)->getData());
      } else if(current_vlr->check(userId,2112)){
         set = set || converter->set_projection_from_ogc_wkt(
                             ((LASF_Projection_2112*)current_vlr)->getData());
      } else if(current_vlr->check(userId,34735)){
          prj_34735 = (LASF_Projection_34735*)current_vlr;
      }  else if(current_vlr->check(userId,34736)){
          prj_34736 = (LASF_Projection_34736*)current_vlr;
      }  else if(current_vlr->check(userId,34737)){
          prj_34737 = (LASF_Projection_34737*)current_vlr;
      } 
   }

   // use geotiff if not already set by wkt and geotiff is found
   if(!set && prj_34735!=0){
      char* geoasciiparams = prj_34737?prj_34737->getData():0;
      double* geodoubleparams = prj_34736?prj_34736->getData():0;

      int numOfGeoKeys = prj_34735->numOfKeys();
      GeoProjectionGeoKeys* keys = new GeoProjectionGeoKeys[numOfGeoKeys];
      // fill keys 
      for(int i=0;i<numOfGeoKeys;i++){
         GeoProjectionGeoKeys oneKey;
         LASF_Projection_34735::sKeyEntry entry = prj_34735->getEntry(i);
         oneKey.key_id = entry.wKeyID;
         oneKey.tiff_tag_location = entry.wTIFFTagLocation;
         oneKey.count = entry.wCount;
         oneKey.value_offset = entry.wValue_Offset;
         keys[i] = oneKey; 
      }
 
      set = converter->set_projection_from_geo_keys(
                   numOfGeoKeys,keys,geoasciiparams,geodoubleparams); 
      
     
     if(geodoubleparams){
        delete[] geodoubleparams;
     }
     if(geoasciiparams){
       delete[] geoasciiparams;
     }
     delete[] keys;
 
   }
   if(!set){ // no valid proejction information found, ignore 
             // avoid conversion into wgs1984
     delete converter;
     converter = 0;
   } else {
      converter->set_gcs(GEO_GCS_WGS84);
   }

}



lasreader::~lasreader(){
   file.close();
   for(size_t i=0;i<knownvlrs.size();i++){
     delete knownvlrs[i];
   }
   if(converter){
      delete converter;
   }
}


lasPoint* lasreader::next(){
   if(toRead==0 || !ok ||  !file.good() ){
     return 0;
   }
   // todo create point depending on the point type
   lasPoint* res = getLasPoint(header.point_data_format); 
   if(!res){
      std::cerr << "Invalid point data format " 
                << header.point_data_format << std::endl;
      return 0;
   }
   res->read(file,header.point_data_length);
   ok = file.good();
   points_read++;
   toRead--;
   return res;
}

bool lasreader::getCoordinates(lasPoint* p,
                               double& x, 
                               double& y, 
                               double& z, 
                               bool& valid){
   if(!p){
     return false;
   }
   x = p->X*header.x_scale+header.x_offset;
   y = p->Y*header.y_scale+header.y_offset;
   z = p->Z*header.z_scale+header.z_offset;
   valid = true;
   // check whether point is in header's bbox
   if(x < header.min_x) valid = false;
   if(x > header.max_x) valid = false;
   if(y < header.min_y) valid = false;
   if(y > header.max_y) valid = false;
   if(z < header.min_z) valid = false;
   if(z > header.max_z) valid = false;
   return true;
}

bool lasreader::toLatLon(lasPoint* p,
                         double& x,
                         double& y,
                         double& z,
                         bool& valid) {
  if(!getCoordinates(p,x,y,z,valid)){
     return false;
  }
  if(!converter){
    return true;
  }
  double point[] = {x,y,z};
  return converter->to_lon_lat_ele(point,x,y,z);
}   


bool lasreader::readVLRs(){
  if(!ok) return  false;
  if(header.number_of_variable_records == 0){
    return true;
  }
  file.seekg(header.header_size, std::ios::beg);
  for(size_t i=0;i<header.number_of_variable_records && file.good();i++) {
    vlr* rec = vlr::read(file,false);
    if(rec){
       knownvlrs.push_back(rec);
    }
  }
  return file.good();
}

bool lasreader::readEVLRs(){
  if(!ok) return  false;
  if(header.number_of_evlr == 0){
    return true;
  }
  file.seekg(header.start_of_first_extended_vlr, std::ios::beg);
  for(size_t i=0;i<header.number_of_variable_records && file.good();i++) {
    vlr* rec = vlr::read(file,true);
    if(rec){
       knownvlrs.push_back(rec);
    }
  }
  return file.good();
}




