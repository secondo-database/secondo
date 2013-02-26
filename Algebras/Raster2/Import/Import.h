/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/

#ifndef RASTER2_IMPORT_H
#define RASTER2_IMPORT_H

#include "FileSystem.h"
#include "WinUnix.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <cmath>

namespace raster2
{

class RasterData
{
public:

	// constructors
	RasterData(bool earthOrigin){
		origin.north = -1;
		origin.south = -1;
		origin.east = -1;
		origin.west = -1;
		useEarthOrigin = earthOrigin;
		endianTypeLittle = isLittleEndian();
		undefValueHGT = -32768; //given by HGT file format
		// minimum signed 32bit integer
		// nodata values taken from gdal library
		// frmts/aigrid/aigrid.h, line 37
		undefValueEsriGrid = -2147483647;
		undefValueEsriGridFloat =
		    -340282346638528859811704183484516925440.0;
		ImportHGTExtend = -1; //HGT extend only 3601x3601 or 1201x1201
	}

    // destructor
	virtual ~RasterData(){};

	// getter
	int getPageSize(){
		return PageSize;
	}

	int getGridSize(){
		return GridSize;
	}

	bool getEndianTypeLittle() {
		return endianTypeLittle;
	}


    /*This section covers common functions*/

	bool endsWith(const std::string& a1, const std::string& a2){
		size_t len1 = a1.length();
		size_t len2 = a2.length();
		if(len2 > len1){
			return false;
		}
		return a1.substr(len1-len2)==a2;
	}

	bool isLittleEndian() {
	  int endian_detect = 1;
	  return *(char *)&endian_detect == 1;
	}

	float convertFloat(const float inFloat)
	{
	   float retVal;
	   char *floatToConvert = ( char* ) & inFloat;
	   char *returnFloat = ( char* ) & retVal;

	   // swap the bytes into a temporary buffer
	   returnFloat[0] = floatToConvert[3];
	   returnFloat[1] = floatToConvert[2];
	   returnFloat[2] = floatToConvert[1];
	   returnFloat[3] = floatToConvert[0];

	   return retVal;
	}

	uint64_t convertEndian(const uint64_t n){
	   uint64_t x = n;
	   return (x>>56) |
	          ((x<<40) & 0x00FF000000000000ull) |
	          ((x<<24) & 0x0000FF0000000000ull) |
	          ((x<<8)  & 0x000000FF00000000ull) |
	          ((x>>8)  & 0x00000000FF000000ull) |
	          ((x>>24) & 0x0000000000FF0000ull) |
	          ((x>>40) & 0x000000000000FF00ull) |
	          (x<<56);
	}

        int16_t convertEndian(int16_t x){
         return  (( x & 0x00FF) << 8) | ( ( x & 0xFF00) >> 8);
        }

        int16_t convert16BitEndian(const int16_t n) {
          int16_t value = n;
          return ((value << 8) | ((value >> 8) & 0xFF));
        }

	int32_t convertEndian(int32_t val)
	{
	  int32_t tmp = val;
	    return (tmp << 24) |
	          ((tmp <<  8) & 0x00ff0000) |
	          ((tmp >>  8) & 0x0000ff00) |
	          ((tmp >> 24) & 0x000000ff);
	}

	uint32_t convertEndian(const uint32_t n)
	{
	  uint32_t x = n;
	  return  ((x & 0xFF) << 24)    |
	          ((x & 0xFF00) << 8)   |
	          ((x & 0xFF0000) >> 8) |
	          ((x & 0xFF000000)>> 24);

	}

	/*This section covers Functions related to HGT import*/

	int getImportHGTExtend(){
		return ImportHGTExtend;
	}

	int getOriginNorth() {
		return origin.north;
	}

	int getOriginSouth() {
		return origin.south;
	}

	int getOriginEast() {
		return origin.east;
	}

	int getOriginWest() {
		return origin.west;
	}

	int getOffsetNorth() {
		return offset.north;
	}

	int getOffsetSouth() {
		return offset.south;
	}

	int getOffsetEast() {
		return offset.east;
	}

	int getOffsetWest() {
		return offset.west;
	}

	int getHGTUndef() {
		return undefValueHGT;
	}

	void calculateCoordinates(string LengthOrientation,
				int LengthOrientationValue,
				string WidthOrientation,
				int WidthOrientationValue){

		if(WidthOrientation.compare("N") == 0){ //OK
			if(origin.north == -1){
				if(origin.south == -1) {
				useEarthOrigin ?
				origin.north = 0 :
				origin.north = WidthOrientationValue;

				useEarthOrigin ?
				offset.north = WidthOrientationValue :
				offset.north = 0;
				} else
                 offset.south = origin.south + WidthOrientationValue;
			} else
             offset.north = WidthOrientationValue - origin.north;
		}

		if(WidthOrientation.compare("S") == 0){ //OK
			if(origin.south == -1){
				if(origin.north == -1){
				useEarthOrigin ?
				origin.south = 0 :
				origin.south = WidthOrientationValue;

				useEarthOrigin ?
				offset.south = 0 - WidthOrientationValue :
				offset.south = 0;
				} else
                 offset.north = 0 - (origin.north + WidthOrientationValue);
			} else
             offset.south = origin.south - WidthOrientationValue;
		}

		if(LengthOrientation.compare("E") == 0){ //OK
			if(origin.east == -1){
				if(origin.west == -1) {
				useEarthOrigin ?
				origin.east = 0 :
				origin.east = LengthOrientationValue;

				useEarthOrigin ?
				offset.east = LengthOrientationValue :
				offset.east = 0;
				} else
                 offset.west = origin.west + LengthOrientationValue;
			} else
             offset.east = LengthOrientationValue - origin.east;
		}

		if(LengthOrientation.compare("W") == 0){ //OK
			if(origin.west == -1){
				if(origin.east == -1) {
				useEarthOrigin ?
				origin.west = 0 :
				origin.west = LengthOrientationValue;

				useEarthOrigin ?
				offset.west = 0 - LengthOrientationValue :
				offset.west = 0;
				} else
                 offset.east = 0 - (origin.east + LengthOrientationValue);
			} else
             offset.west = origin.west - LengthOrientationValue;
		}
	}

	int getYOffset() {
		if( origin.north != -1)
			return offset.north;

		if(origin.south != -1)
			return offset.south;

		return 0;
	}

	int getXOffset() {
		if(origin.east != -1)
			return offset.east;

		if(origin.west != -1)
			return offset.west;

		return 0;
	}

	void setOriginNorth(int originNorth) {
		origin.north = originNorth;
	}

	void setOriginSouth(int originSouth) {
		origin.south = originSouth;
	}

	void setOriginEast(int originEast) {
		origin.east = originEast;
	}

	void setOriginWest(int originWest) {
		origin.west = originWest;
	}

	string getWidthOrientationFromFileName(const string HGTFile){
		string filename = HGTFile.substr(HGTFile.find_last_of("/") + 1);
        transform(filename.begin(), filename.end(),
         filename.begin(), ::toupper);
		return filename.substr(0, 1);
	}

	string getLengthOrientationFromFileName(const string HGTFile){
		string filename = HGTFile.substr(HGTFile.find_last_of("/") + 1);
        transform(filename.begin(), filename.end(),
         filename.begin(), ::toupper);
		return filename.substr(3, 1);
	}

	int getWidthOrientationValueFromFileName(const string HGTFile){
		string filename = HGTFile.substr(HGTFile.find_last_of("/") + 1);
		return atoi(filename.substr(1, 2).c_str());
	}

	int getLengthOrientationValueFromFileName(const string HGTFile){
		string filename = HGTFile.substr(HGTFile.find_last_of("/") + 1);
		return atoi(filename.substr(4, 3).c_str());
	}

	int getHGTExtendfromFile(const string currentHGTFile) {
	 double currentHGTFileSize =
         FileSystem::GetFileSize(currentHGTFile.c_str());

	 if(currentHGTFileSize == -1){
	  cout << "Error accessing file " << currentHGTFile.c_str() << endl;
	  return -1;
	 }

	 // HGT Files are written in 16bit signed integer, gives two bytes
	 int currentValueCount = currentHGTFileSize / 2;
	 return sqrt(currentValueCount);
	}

	bool checkHGTFile(string currentHGTFile, int currentHGTExtend) {
		if(checkHGTExtend(currentHGTExtend)){
	  		cout << "Grid extend    : " << currentHGTExtend -1
	  			 << "x" << currentHGTExtend - 1 << endl;
		} else {
			 cout << "Wrong grid extend, skipping file" << endl;
			 return false;
		}

	  	if(!checkGeoCoordinates(
            getLengthOrientationFromFileName(currentHGTFile),
            getLengthOrientationValueFromFileName(currentHGTFile),
            getWidthOrientationFromFileName(currentHGTFile),
            getWidthOrientationValueFromFileName(currentHGTFile))){
	  		 return false;
	  	}

	  	if(!checkHGTFileExtension(currentHGTFile))
	  	 return false;

	    return true;
	}

	bool checkHGTExtend(int currentHGTExtend){
		if(currentHGTExtend != 3601 && currentHGTExtend != 1201)
			return false;

		if(ImportHGTExtend == -1)
			ImportHGTExtend = currentHGTExtend;

		if(currentHGTExtend == ImportHGTExtend)
			return true;
		else
			return false;
	}

	bool checkGeoCoordinates(string LengthOrientation,
			int LengthOrientationValue,
			string WidthOrientation,
			int WidthOrientationValue){

		if(LengthOrientationValue > 180){
         cout << "Length orientation value is out of range" << endl << endl;
         return false;
		}

		if(WidthOrientationValue > 90){
         cout << "Width orientation value is out of range" << endl << endl;
         return false;
		}

		if((WidthOrientation.compare("N") != 0) &&
				(WidthOrientation.compare("S") != 0)) {
         cout << "Width orientation not valid" << endl << endl;
         return false;
		}

		if((LengthOrientation.compare("E") != 0) &&
				(LengthOrientation.compare("W") != 0)) {
         cout << "Length orientation not valid" << endl << endl;
         return false;
		}

        return true;
	}

	bool checkHGTFileExtension(string currentHGTFile){
		transform(currentHGTFile.begin(), currentHGTFile.end(),
				currentHGTFile.begin(), ::tolower);

		if(!endsWith(currentHGTFile.c_str(),".hgt")){
         cout << "Wrong file type, skipping file" << endl;
         return false;
        }

		return true;
	}

	int16_t* getHGTData(const string HGTFile) {
		ifstream* f =
		new ifstream(HGTFile.c_str(), ios::in|ios::binary|ios::ate);

		if(!f->is_open()){
		    delete f;
		    f = 0;
		    cerr << "Failed to open HGT File" << endl;
		    return NULL;
		}

		// go to begin of file
		f->seekg (0, ios::beg);
		int16_t* buffer = new int16_t[ImportHGTExtend*ImportHGTExtend];

		// read the complete file into buffer
		f->read( (char*) buffer, ImportHGTExtend*ImportHGTExtend*2);

		if(!f->good()){
		  cerr << "error in reading from file" << endl;
		  f->close();
		  delete f;
		  delete[] buffer;
		  return NULL;
		}

		f->close();
		delete f;
		f = 0;

		return buffer;
	}

        /*
        This section covers Functions related to Esri Grid import

        */

	struct EsriGridHDRData {
		string HMagic;
		int32_t HCellType;
		int32_t CompFlag;
		int32_t HTilesPerRow;
		int32_t HTilesPerColumn;
		int32_t HTileXSize;
		int32_t HTileYSize;
		double HPixelSizeX;
		double HPixelSizeY;
		double XRef;
		double YRef;
	} currentGridHDR;

	/*
        Esri Grid Config Data from the Esri config files.

        */
        struct EsriGridConfigData {
	  string filePath; // the esri grid file path
          double llxCoord; // Lower left X (easting) of the grid.
          double llyCoord; // Lower left Y (northing) of the grid.
          double urxCoord; // Upper right X (easting) of the grid.
          double uryCoord; // Upper right Y (northing) of the grid.
          size_t fileSize;
          size_t numTiles;
          size_t actualNumberOfColumns;
          size_t actualNumberOfRows;
          // Minimum value of a raster cell in this grid.
          double esriGridMinimum;
          // Maximum value of a raster cell in this grid.
          double esriGridMaxmimum;

        } esriGridConfigData;

        int getEsriGridUndef() {
          return undefValueEsriGrid;
        }

        float getEsriGridFloatUndef() {
          return undefValueEsriGridFloat;
        }

        /*
        Delivers the integer value for bit data value RTileTypes
        0x01 and 0x04.

        */
        int getIntBitDataValueByRTileType(
            ifstream *fileStream,
            const uint8_t *rTileType,
            int pixelCounter)
        {
          int bitDataValue = 0;
          const uint8_t tileType = *rTileType;
          char valueBlock[1];
          fileStream->read(valueBlock, 1);
          unsigned char *byteValue =
              &(*(unsigned char*)&(valueBlock));

          // (raw 1bit data)
          if (tileType == 0x01)
          {
              // algorithm taken from gdal library
              // frmts/aigrid/gridlib.c, line 214
              if(byteValue[pixelCounter>>3] &
                  (0x80 >> (pixelCounter & 0x7)))
              {
                  bitDataValue = 1;
              }
              else
              {
                  bitDataValue = 0;
              }
          }

          // (raw 4bit data)
          else if (tileType == 0x04)
          {
              // algorithm taken from gdal library
              // frmts/aigrid/gridlib.c, line 180
              if (pixelCounter % 2 == 0)
              {
                  bitDataValue = ((*(byteValue) & 0xf0) >> 4);
              }
              else {
                  bitDataValue = (*(byteValue++) & 0xf);
              }
          }

          else
          {
              cout << endl;
              cout << "Could not determine bit data value";
              cout << " from rTileType";
              cout << endl;
          }
          return bitDataValue;
        }

        /*
        Delivers the integer value for length encoded RTileTypes.

        */
        int getIntValueByRTileType(
            ifstream *fileStream,
            const uint8_t *rTileType,
            int pixelCounter)
        {
          int intValue = 0;
          const uint8_t tileType = *rTileType;

          // (run length encoded 32bit)
          if (
              (tileType == 0xE0) ||
              (tileType == 0x20))
          {
              int32_t int32 = getInt32FromFileStream(fileStream);
              intValue = (int)int32;
              if (isLittleEndian())
              {
                  intValue = ((int) convertEndian(int32));
              }
          }

          // (run length encoded 16bit)
          else if (
              (tileType == 0xF0) ||
              (tileType == 0x10))
          {
              int16_t int16 = getInt16FromFileStream(fileStream);
              if (isLittleEndian())
              {
                  int16 = convert16BitEndian(int16);
              }
              intValue = (int)int16;
          }

          // (run length encoded 8bit)
          else if (
              (tileType == 0x08) ||
              (tileType == 0xFC) ||
              (tileType == 0xF8))
          {
              intValue = 
              	(int)getUInt8FromFileStream(fileStream);
          }

          // (raw 1bit/4bit data)
          else if (
              (tileType == 0x01) ||
              (tileType == 0x04))
          {
              intValue = getIntBitDataValueByRTileType(
                  fileStream, rTileType, pixelCounter);
          }

          else
          {
              cout << endl;
              cout << "Could not determine intValue from rTileType";
              cout << endl;
          }
          return intValue;
        }

	double getDouble(char* buffer, int offset){
	   uint64_t i = *( uint64_t*)&(buffer[offset]);
	   if(endianTypeLittle){
	        i = convertEndian(i);
	   }
	   double res = 0;
           double* pres = static_cast<double*>(static_cast<void*>(&i));

           if(pres != 0)
           {
            res = *pres;
           }

	   return res;
	}

	int32_t getInt32(char* buffer, int offset){
	   int32_t i = *( int32_t*)&(buffer[offset]);
	   if(endianTypeLittle){
	        i = convertEndian(i);
	   }
	   return i;
	}

	uint32_t getUInt32(char* buffer, int offset){
	   uint32_t i = *( uint32_t*)&(buffer[offset]);
	   if(endianTypeLittle){
	        i = convertEndian(i);
	   }
	   return i;
	}

  /*
  Reads a 4 byte block from the current position
  of a given fileStream.

  */
  int32_t getInt32FromFileStream(ifstream *fileStream)
  {
    int32_t Int32 = 0;

    char valueBlock[4];
    fileStream->read(valueBlock, 4);

    int32_t* pInt32 = reinterpret_cast<int32_t*>(&valueBlock);

    if(pInt32 != 0)
    {
      Int32 = *pInt32;
    }

    return Int32;
  }

  /*
  Reads short signed int from the current position
  of a given fileStream.

  */
  int16_t getInt16FromFileStream(ifstream *fileStream)
  {
    int16_t Int16 = 0;

    char valueBlock[2];
    fileStream->read(valueBlock, 2);

    int16_t* pInt16 = reinterpret_cast<int16_t*>(&valueBlock);

    if(pInt16 != 0)
    {
      Int16 = *pInt16;
    }

    return Int16;
  }

  /*
  Reads an unsigned char integer from the current position
  of a given fileStream.

  */
  uint8_t getUInt8FromFileStream(ifstream *fileStream) {
    uint8_t uint8 = 0;
    char valueBlock[1];
    fileStream->read(valueBlock, 1);
    uint8_t* puInt8 = reinterpret_cast<uint8_t*>(&valueBlock);
    if(puInt8 != 0)
    {
        uint8 = *puInt8;
    }
    return uint8;
  }

  /*
  Reads a signed char integer from the current position
  of a given fileStream.

  */
  int8_t getInt8FromFileStream(ifstream *fileStream) {
    int8_t int8 = 0;
    char valueBlock[1];
    fileStream->read(valueBlock, 1);
    int8_t* pInt8 = reinterpret_cast<int8_t*>(&valueBlock);
    if(pInt8 != 0)
    {
        int8 = *pInt8;
    }
    return int8;
  }

  /*
  Reads a signed int from the current position
  of a given fileStream and a given number of bytes.

  */
  int getIntFromFileStream(ifstream *fileStream, const uint8_t byteSize)
  {
    int result = 0;

    if ((int)byteSize == 0)
    {
        result = 0;
    }

    else if ((int)byteSize == 1)
    {
        result = (int)getInt8FromFileStream(fileStream);
    }

    else if ((int)byteSize == 2)
    {
        int16_t int16 = getInt16FromFileStream(fileStream);
        if (isLittleEndian())
        {
            int16 = convert16BitEndian(int16);
        }
        result = (int)int16;
    }

    else if ((int)byteSize == 4)
    {
        int32_t int32 = getInt32FromFileStream(fileStream);
        if (isLittleEndian())
        {
            int32 = convertEndian(int32);
        }
        result = (int)int32;
    }

    return result;
  }

	bool CellTypeReal(){
		return currentGridHDR.HCellType == 2;
	}

	int32_t getCellTypeFromFile(string HDRFile){
	 int32_t celltype = -1;
	 int celltypeOffset = 16;

	 if(endsWith(HDRFile, "/"))
          HDRFile.append("hdr.adf");
         else
          HDRFile.append("/hdr.adf");

	 ifstream* f =
          new ifstream(HDRFile.c_str(), ios::in|ios::binary|ios::ate);

         if(!f->is_open()){
          delete f;
          f = 0;
          return -1;
         }

         char buffer[sizeof(int32_t)];
         f->seekg(celltypeOffset, ios::beg);
         f->read(buffer, sizeof(int32_t));
         celltype = getInt32(buffer,0);

         if(!f->good()){
          f->close();
          delete f;
          return -1;
         }

         f->close();
         delete f;
         f = 0;

	 return celltype;
	}

	int getEsriGridHDR(string HDRFile) {

	  esriGridConfigData.filePath = HDRFile;

	  if(endsWith(HDRFile, "/")) {
              HDRFile.append("hdr.adf");
          }
          else {
              esriGridConfigData.filePath.append("/");
              HDRFile.append("/hdr.adf");
          }

          size_t HDRFileSize =
                   FileSystem::GetFileSize(HDRFile.c_str());

          ifstream* f =
          new ifstream(HDRFile.c_str(), ios::in|ios::binary|ios::ate);

          if(!f->is_open()){
              delete f;
              f = 0;
              cerr << "Failed to open HDR File" << endl;
              return 1;
          }

          // read the complete file into buffer
          char buffer[HDRFileSize];
          f->seekg (0, ios::beg);
          f->read(  buffer, HDRFileSize);

          if(!f->good()){
            cerr << "error in reading from file" << endl;
            f->close();
            delete f;
            return 1;
          }

          string HMagic(buffer,8);
          currentGridHDR.HMagic = HMagic;
          currentGridHDR.HCellType = getInt32(buffer,16);
          currentGridHDR.CompFlag = getInt32(buffer,20);
          currentGridHDR.HPixelSizeX = getDouble(buffer,256);
          currentGridHDR.HPixelSizeY = getDouble(buffer,264);
          currentGridHDR.XRef = getDouble(buffer,272);
          currentGridHDR.YRef = getDouble(buffer,280);
          currentGridHDR.HTilesPerRow = getInt32(buffer,288);
          currentGridHDR.HTilesPerColumn = getInt32(buffer,292);
          currentGridHDR.HTileXSize = getInt32(buffer,296);
          currentGridHDR.HTileYSize = getInt32(buffer,304);

          f->close();
          delete f;
          f = 0;

          return 0;
	}

        /*
        This section covers Functions related to Esri Raster import

        */

	struct EsriRasterHDRData {
		int ncols;
		int nrows;
		double xllcorner;
		double xllcenter;
		double yllcorner;
		double yllcenter;
		double cellsize;
		int extend;
		int nodata_value;
		size_t posValues;
		size_t filesize;
	} originEsriHDR, currentEsriHDR;

	int getIntValue(string line, size_t offset){
		int value = -1;

		size_t posA = line.find_first_of(" ");
		size_t pos=line.find_first_not_of(" ", posA);

		if(pos!=string::npos){
			string buffer = line.substr(pos, (line.length() - pos));
			value = atoi(buffer.c_str());
		}

		return value;
	}

	double getDoubleValue(string line, size_t offset){
		double value = -1.0;

		size_t posA = line.find_first_of(" ");
		size_t pos=line.find_first_not_of(" ", posA);

		if(pos!=string::npos){
			string buffer = line.substr(pos, (line.length() - pos));
			value = atof(buffer.c_str());
		}

		return value;
	}

	int getEsriRasterHDR(string RasterFile, bool init) {
		ifstream* f =
		new ifstream(RasterFile.c_str(), ios::in);

		if(!f->is_open()){
		    delete f;
		    f = 0;
		    cerr << "Failed to open Esri Raster File" << endl;
		    return 1;
		}

		size_t header = 0;
		size_t valuepos;
		char buffer[1000];

		//initialize data structs
		if(init)
		 memset(&originEsriHDR, -1, sizeof (originEsriHDR));
		memset(&currentEsriHDR, -1, sizeof (currentEsriHDR));

		//get filesize
		if(init)
		 originEsriHDR.filesize =
				 FileSystem::GetFileSize(RasterFile.c_str());
		 currentEsriHDR.filesize =
				 FileSystem::GetFileSize(RasterFile.c_str());

		//default, will be overwritten if differs
		if(init)
		 originEsriHDR.nodata_value = -9999;
		currentEsriHDR.nodata_value = -9999;

		//reading the header
		while((header != string::npos)) {
			f->getline(buffer, 1000, '\n');
			string name = buffer;
			transform(name.begin(), name.end(),
					name.begin(), ::tolower);

			valuepos = name.find("ncols");
			if (valuepos != string::npos){
			 if(init)
			  originEsriHDR.ncols = getIntValue(name, valuepos);
			 currentEsriHDR.ncols = getIntValue(name, valuepos);
			}

			valuepos = name.find("nrows");
			if (valuepos != string::npos){
			 if(init)
			  originEsriHDR.nrows = getIntValue(name, valuepos);
			 currentEsriHDR.nrows = getIntValue(name, valuepos);
			}

			valuepos = name.find("xllcorner");
			if (valuepos != string::npos){
			 if(init)
              originEsriHDR.xllcorner = getDoubleValue(name, valuepos);
             currentEsriHDR.xllcorner = getDoubleValue(name, valuepos);
			}


			valuepos = name.find("xllcenter");
			if (valuepos != string::npos){
			 if(init)
              originEsriHDR.xllcenter = getDoubleValue(name, valuepos);
             currentEsriHDR.xllcenter = getDoubleValue(name, valuepos);
			}

			valuepos = name.find("yllcorner");
			if (valuepos != string::npos){
			 if(init)
              originEsriHDR.yllcorner = getDoubleValue(name, valuepos);
             currentEsriHDR.yllcorner = getDoubleValue(name, valuepos);
			}

			valuepos = name.find("yllcenter");
			if (valuepos != string::npos){
			 if(init)
              originEsriHDR.yllcenter = getDoubleValue(name, valuepos);
             currentEsriHDR.yllcenter = getDoubleValue(name, valuepos);
			}

			valuepos = name.find("cellsize");
			if (valuepos != string::npos){
			 if(init){
	          originEsriHDR.cellsize = getDoubleValue(name, valuepos);
	          originEsriHDR.extend =
                static_cast<int>((1/originEsriHDR.cellsize)+0.5);
			 }
             currentEsriHDR.cellsize = getDoubleValue(name, valuepos);
	         currentEsriHDR.extend =
                static_cast<int>((1/currentEsriHDR.cellsize)+0.5);
			}

			valuepos = name.find("nodata_value");
			if (valuepos != string::npos){
			 if(init)
              originEsriHDR.nodata_value = getIntValue(name, valuepos);
             currentEsriHDR.nodata_value = getIntValue(name, valuepos);
			}

			header =
                name.find_first_of("abcdefghijklmnopqrstuvwxyz", 0);
			if(header == 0){
			 if(init)
              originEsriHDR.posValues = 1 + f->tellg();
             currentEsriHDR.posValues = 1 + f->tellg();
			}
		}

		f->close();
		delete f;
		f = 0;

		if(!validateHeaderData()){
			cout << "Wrong header information." << endl;
			return 1;
		}

		return 0;
	}

	bool validateHeaderData(){
		return (currentEsriHDR.ncols != -1) &&
	           (currentEsriHDR.nrows != -1) &&
	           !isnan(currentEsriHDR.cellsize) &&
	           (getUseCorner() || getUseCenter());
    }

	bool getUseCorner(){
		return isnan(originEsriHDR.xllcenter) &&
		       isnan(currentEsriHDR.xllcenter) &&
		       isnan(originEsriHDR.yllcenter) &&
		       isnan(currentEsriHDR.yllcenter);
	}

	bool getUseCenter(){
		return isnan(originEsriHDR.xllcorner) &&
		       isnan(currentEsriHDR.xllcorner) &&
		       isnan(originEsriHDR.yllcorner) &&
		       isnan(currentEsriHDR.yllcorner);
	}

	bool checkRasterExtend(){
		bool extendOK = true;

		if(originEsriHDR.cellsize != currentEsriHDR.cellsize){
	  	 cout << "Wrong Raster extend, skipping file" << endl;
	  	 extendOK = false;
		} else {
	  		cout << "Grid extend    : " << currentEsriHDR.extend
	  			 << "x" << currentEsriHDR.extend << endl;
		}

		return extendOK;
	}

	int getRasterXOffset() {
	  if(getUseCorner())
		return (currentEsriHDR.xllcorner - originEsriHDR.xllcorner)
				* originEsriHDR.extend;

	  if(getUseCenter())
		return (currentEsriHDR.xllcenter - originEsriHDR.xllcenter)
				* originEsriHDR.extend;

	  return std::numeric_limits<int>::min();
	}

	int getRasterYOffset() {
	  if(getUseCorner())
		return (currentEsriHDR.yllcorner - originEsriHDR.yllcorner)
				* originEsriHDR.extend;

	  if(getUseCenter())
		return (currentEsriHDR.yllcenter - originEsriHDR.yllcenter)
				* originEsriHDR.extend;

	  return std::numeric_limits<int>::min();
	}

	bool checkRasterOffset(int XOffset, int YOffset){
	  bool offsetOK = true;
      if((XOffset == std::numeric_limits<int>::min()) ||
		 (YOffset == std::numeric_limits<int>::min())){
	   cout << "Invalid offset, skipping file" << endl;
       offsetOK = false;
      }

      return offsetOK;
	}

	bool checkNumber(char *value){
		bool numberOK = true;
		string number = value;
		size_t pos = number.find_first_not_of("-0123456789.\n\r", 0);
		if(number.empty() || pos != string::npos)
			numberOK = false;
		return numberOK;
	}

	double* getEsriRasterData(const string EsriRasterFile) {
		ifstream* f =
		new ifstream(EsriRasterFile.c_str(), ios::in);

		if(!f->is_open()){
		 delete f;
		 f = 0;
		 cerr << "Failed to open ASCII File" << endl;
		 return NULL;
		}

		long allValues = currentEsriHDR.nrows * currentEsriHDR.ncols;
		long unit = allValues/100;

		double* buffer = new double[allValues];
		char value[50];

		f->seekg(currentEsriHDR.posValues, ios::beg);

		cout << "Consistency    : ";

		for(int row = 0; row < currentEsriHDR.nrows; row++){
		 for(int col = 0; col < currentEsriHDR.ncols; col++){
		  f->getline(value, 50, ' ');

		  //If end of file is reached, something is wrong with the file.
		  if(checkNumber(value)){
	 	   buffer[(row*currentEsriHDR.ncols) + col] = atof(value);

	 	  if(f->eof()){
           if((row != currentEsriHDR.nrows-1) ||
        		   col != currentEsriHDR.ncols-1){
     		cout << "\rConsistency    : failed, skipping file" << endl;
     		row = currentEsriHDR.nrows;
     		col = currentEsriHDR.ncols;
     		buffer = NULL;
           } else
        	cout << "\rConsistency    : OK  " << endl;
	 	  } else {
	 	   //calculate percentage of progress
           if(allValues > 99){
            if(((row == 0) && (col == 0)) ||
	          ((((row * currentEsriHDR.ncols)+(col+1)) % unit == 0) &&
              (((row * currentEsriHDR.ncols)+(col+1)) >= unit))){
               cout << "\rConsistency    : "
                    << ((row * currentEsriHDR.ncols)+(col+1))
                    / unit << "%";
               fflush (stdout);
            }
           }
	 	  }
		 } else {
		  cout << "\rConsistency    : failed, skipping file" << endl;
		  row = currentEsriHDR.nrows;
		  col = currentEsriHDR.ncols;
		  buffer = NULL;
		 }
		}
	   }

	   f->close();
	   delete f;
	   f = 0;

	   return buffer;
	}

    /*This section covers interim Functions*/

	vector<string> buildFilesVector(string HGTDataRoot);

	bool checkRootIsValid(string HGTDataRoot);
	int ImportHGT(const string HGTDataRoot);
	int processFile(string HGTFile);

private:
	RasterData(){}

	int PageSize;
	int GridSize;
	int ImportHGTExtend;
	int undefValueHGT;
	int undefValueEsriGrid;
	float undefValueEsriGridFloat;
	bool useEarthOrigin;
	bool endianTypeLittle;

	struct GeoCoordinates {
		int north;
		int south;
		int east;
		int west;
	} origin, offset;
};

}

#endif // RASTER2_IMPORT_H
