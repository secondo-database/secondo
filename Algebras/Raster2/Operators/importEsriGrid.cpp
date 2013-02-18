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

#include "importEsriGrid.h"

namespace raster2
{
    /*
    Reads a a file for a filename into a given buffer.

    */
    bool readFileIntoBuffer(char* buffer, const string& fname,
        const uint32_t len)
    {
      ifstream f(fname.c_str(), ios::in|ios::binary|ios::ate);
       if(!f.is_open()){
           cerr << "could not open " + fname << endl;
           return false;
       }
       f.seekg(0,ios::beg);
       f.read(buffer,len);
       if(!f.good()) {
           f.close();
           cerr << "error in reading " << fname << endl;
           return false;
       }
       f.close();
       return true;
    }

    void readEsriGridBounds(RasterData *EsriRasterData,
        const string boundingFileName) {
       char buffer[32];
       if(!readFileIntoBuffer(buffer, boundingFileName, 32)) {
         return;
       }

       EsriRasterData->esriGridConfigData.llxCoord =
           EsriRasterData->getDouble(buffer,0);;
       EsriRasterData->esriGridConfigData.llyCoord =
           EsriRasterData->getDouble(buffer,8);
       EsriRasterData->esriGridConfigData.urxCoord =
           EsriRasterData->getDouble(buffer,16);
       EsriRasterData->esriGridConfigData.uryCoord =
           EsriRasterData->getDouble(buffer,24);

       if (DEBUG_OUTPUT_ENABLED) {
           cout << "bbox : ( (";
           cout << EsriRasterData->esriGridConfigData.llxCoord;
           cout << ", ";
           cout << EsriRasterData->esriGridConfigData.llyCoord;
           cout << ") -> (";
           cout << EsriRasterData->esriGridConfigData.urxCoord;
           cout << ", ";
           cout << EsriRasterData->esriGridConfigData.uryCoord;
           cout << "))" << endl;
       }
    }

    void readEsriGridIndexConfig(RasterData *EsriRasterData,
        ifstream *indexFile) {

      if (DEBUG_OUTPUT_ENABLED) {
          cout << endl << endl << "Reading EsriGrid Index File:" << endl;
          cout << "============================" << endl << endl;
      }

      indexFile->seekg(24,ios::beg);
      char buffer[4];
      indexFile->read(buffer,4);

      // filesize in bytes
      EsriRasterData->esriGridConfigData.fileSize =
          EsriRasterData->getUInt32(buffer,0) * 2;
      const uint32_t indexFileSize =
          EsriRasterData->esriGridConfigData.fileSize;

      // detail information for tiles start at byte 100
      // (8 bytes for every tile)
      EsriRasterData->esriGridConfigData.numTiles =
          (indexFileSize - 100) / INDEX_BUFFER_SIZE;

      if (DEBUG_OUTPUT_ENABLED) {
          cout << "filesize = " << indexFileSize << endl;
          cout << "numTiles = ";
          cout << EsriRasterData->esriGridConfigData.numTiles;
          cout << endl << endl;
      }
    }

    void readStatisticData(RasterData *EsriRasterData,
        const string statisticsFileName) {
       char buffer[32];
       if (!readFileIntoBuffer(buffer, statisticsFileName, 32))
       {
          return;
       }
       const double minValue =
           EsriRasterData->getDouble(buffer,0);
       const double maxValue =
           EsriRasterData->getDouble(buffer,8);
       EsriRasterData->esriGridConfigData.esriGridMinimum = minValue;
       EsriRasterData->esriGridConfigData.esriGridMaxmimum = maxValue;

       if (DEBUG_OUTPUT_ENABLED) {
           cout << endl << endl << "Reading Esri Statistics" << endl;
           cout << "============================" << endl << endl;
           cout << "SMin             : " << minValue << endl;
           cout << "SMax             : " << maxValue << endl;
           cout << "SMean            : " <<
               EsriRasterData->getDouble(buffer,16) << endl;
           cout << "SStdDev          : ";
           cout << EsriRasterData->getDouble(buffer,24);
           cout << endl << endl;
       }
    }

    /*
    Determines the actual number of rows and columns.

    */
    void evaluateActualTilesRowAndColumnCount(RasterData *EsriRasterData) {

      // actual number of columns
      EsriRasterData->esriGridConfigData.actualNumberOfColumns =
          EsriRasterData->currentGridHDR.HTilesPerRow;

      // actual number of rows
      const double numberOfRows =
          EsriRasterData->esriGridConfigData.actualNumberOfRows =
              EsriRasterData->esriGridConfigData.numTiles /
              EsriRasterData->esriGridConfigData.actualNumberOfColumns;
      EsriRasterData->esriGridConfigData.actualNumberOfRows =
          ceil(numberOfRows);
    }

    template <typename T, typename Helper>
        void stype<T, Helper>::processConstantBlockData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint8_t minSize) {

      const int undefValue = esriRasterData->getEsriGridUndef();
      size_t tilePixelCounter = 0;
      const uint32_t tileLimitCount = cellColCount * cellRowCount;
      storage_type& rasterStorage = this->getStorage();

      int minValue =
          esriRasterData->getIntFromFileStream(dataFile, minSize);

      if (DEBUG_OUTPUT_ENABLED) {
          cout << "Min value: " << minValue << endl;
      }

      // Iterate over cell rows (y-direction)
      for (uint32_t cellRowIdx=0; cellRowIdx<cellRowCount;
          cellRowIdx++) {

          // Iterate over cell columns (x-direction)
          for (uint32_t cellColIdx=0; cellColIdx<cellColCount;
              cellColIdx++) {
              ++tilePixelCounter;
              if (tilePixelCounter > tileLimitCount) {
                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << endl << "Leaving processing of tile, ";
                      cout << "tilePixelCounter: " << tilePixelCounter;
                  }
                  return;
              }

              // the stype raster definition goes from bottom to top, so
              // the y-rasterIndex cell row values have to be turned over
              size_t rasterIndexX =
                  (tileColIdx*cellColCount + cellColIdx);
              size_t rasterIndexY = maxRasterIdxY -
                  (tileRowIdx*cellRowCount + cellRowIdx);

              // initialize raster index for current cell
              RasterIndex<2> rasterIndex = (int[2]) {
                rasterIndexX, rasterIndexY
              };
              if (undefValue != minValue) {
                  rasterStorage[rasterIndex] = minValue;
              }

              if (DEBUG_OUTPUT_ENABLED) {
                  cout << " " << minValue;
              }
          } // cell columns
      } // cell rows
    }

    template <typename T, typename Helper>
        void stype<T, Helper>::processEsriTilePixelValueData(
            ifstream *dataFile,
            const uint8_t *rTileType,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t nextTileOffset,
            const uint8_t rMinSize) {

      const int undefValue = esriRasterData->getEsriGridUndef();
      int tilePixelCounter = 0;
      storage_type& rasterStorage = this->getStorage();
      uint32_t currentTilePosition = 0;

      int minValue =
          esriRasterData->getIntFromFileStream(dataFile, rMinSize);

      if (DEBUG_OUTPUT_ENABLED) {
          cout << "Min value: " << minValue << endl;
      }

      // Iterate over cell rows (y-direction)
      for (uint32_t cellRowIdx=0; cellRowIdx<cellRowCount;
          cellRowIdx++) {

          // Iterate over cell columns (x-direction)
          for (uint32_t cellColIdx=0; cellColIdx<cellColCount;
              cellColIdx++) {
              currentTilePosition = dataFile->tellg();
              if ((currentTilePosition) > nextTileOffset) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << endl << "Leaving processing of tile, ";
                      cout << "tilePixelCounter: ";
                      cout << tilePixelCounter + 1;
                  }
                  return;
              }

              // the stype raster definition goes from bottom to top, so
              // the y-rasterIndex cell row values have to be turned over
              size_t rasterIndexX =
                  (tileColIdx*cellColCount + cellColIdx);
              size_t rasterIndexY = maxRasterIdxY -
                  (tileRowIdx*cellRowCount + cellRowIdx);

              // initialize raster index for current cell
              RasterIndex<2> rasterIndex = (int[2]) {
                rasterIndexX, rasterIndexY
              };

              // read pixel data memory block
              const int pixelValue =
                  esriRasterData->getIntValueByRTileType(
                      dataFile, rTileType,
                      tilePixelCounter);

              // write it to the raster storage cell
              int resultValue = pixelValue + minValue;
              if (undefValue != resultValue) {
                  rasterStorage[rasterIndex] = resultValue;
              }

              if (DEBUG_OUTPUT_ENABLED) {
                  cout << resultValue << " ";
              }
              ++tilePixelCounter;
          } // cell columns
      } // cell rows
    }

    /*
    RTileType 0xCF

    */
    template <typename T, typename Helper>
        void stype<T, Helper>::processEsriTile16BitLiteralRunsData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t nextTileOffset,
            const uint8_t rMinSize) {

      size_t tilePixelCounter = 0;
      storage_type& rasterStorage = this->getStorage();
      uint32_t currentTilePosition = 0;
      bool isNoDataType = false;
      uint8_t noDataRunPixelCount = 0;
      uint8_t dataRunPixelCount = 0;
      const int undefValue = esriRasterData->getEsriGridUndef();

      int minValue =
          esriRasterData->getIntFromFileStream(dataFile, rMinSize);

      if (DEBUG_OUTPUT_ENABLED) {
          cout << "Min value: " << minValue << endl;
      }

      // Iterate over cell rows (y-direction)
      for (uint32_t cellRowIdx=0; cellRowIdx<cellRowCount;
          cellRowIdx++) {

          // Iterate over cell columns (x-direction)
          for (uint32_t cellColIdx=0; cellColIdx<cellColCount;
              cellColIdx++) {
              ++tilePixelCounter;
              currentTilePosition = dataFile->tellg();
              if (currentTilePosition > nextTileOffset) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << endl << "Leaving processing of tile, ";
                      cout << "tilePixelCounter: ";
                      cout << tilePixelCounter;
                  }
                  return;
              }

              // read next byte and it's marker value
              // for the next run of bytes
              if ((dataRunPixelCount == 0) &&
                  (noDataRunPixelCount == 0)) {
                  // the first byte is a marker for a series of bytes´
                  const uint8_t markerByteValue =
                      esriRasterData->getUInt8FromFileStream(dataFile);

                      if (DEBUG_OUTPUT_ENABLED) {
                          cout << " marker:" << (int)markerByteValue;
                          cout << " ";
                      }

                  // check if the following run will be of NoData pixels
                  isNoDataType = (markerByteValue > 127);
                  if (isNoDataType)
                  {
                      noDataRunPixelCount = 256 - markerByteValue;
                  }
                  else
                    {
                      dataRunPixelCount = markerByteValue;
                    }
              }

              // the stype raster definition goes from bottom to top, so
              // the y-rasterIndex cell row values have to be turned over
              size_t rasterIndexX =
                  (tileColIdx*cellColCount + cellColIdx);
              size_t rasterIndexY = maxRasterIdxY -
                  (tileRowIdx*cellRowCount + cellRowIdx);

              // initialize raster index for current cell
              RasterIndex<2> rasterIndex = (int[2]) {
                rasterIndexX, rasterIndexY
              };

              // write NoData values into storage
              if (isNoDataType) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << " " << undefValue;
                  }
                  noDataRunPixelCount--;
              }
              // write 16Bit data integers into storage
              else {
                  // read data memory block
                  int16_t pixelValue =
                      esriRasterData->getInt16FromFileStream(dataFile);
                  if (esriRasterData->getEndianTypeLittle()) {
                      pixelValue =
                          esriRasterData->convert16BitEndian(pixelValue);
                  }

                  // write it to the raster storage cell
                  int resultValue = (int)pixelValue + minValue;
                  if (undefValue != resultValue) {
                      rasterStorage[rasterIndex] = resultValue;
                  }

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << " " << resultValue;
                  }
                  dataRunPixelCount--;
              }
          } // cell columns
      } // cell rows
    }

    /*
    RTileType 0xD7

    */
    template <typename T, typename Helper>
        void stype<T, Helper>::processEsriTileLiteralRunsData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t nextTileOffset,
            const uint8_t minSize) {

      size_t tilePixelCounter = 0;
      storage_type& rasterStorage = this->getStorage();
      uint32_t currentTilePosition = 0;
      bool isNoDataType = false;
      uint8_t noDataRunPixelCount = 0;
      uint8_t dataRunPixelCount = 0;
      const int undefValue = esriRasterData->getEsriGridUndef();

      int minValue =
          esriRasterData->getIntFromFileStream(dataFile, minSize);

      if (DEBUG_OUTPUT_ENABLED) {
          cout << "Min value: " << minValue << endl;
      }

      // Iterate over cell rows (y-direction)
      for (uint32_t cellRowIdx=0; cellRowIdx<cellRowCount;
          cellRowIdx++) {

          // Iterate over cell columns (x-direction)
          for (uint32_t cellColIdx=0; cellColIdx<cellColCount;
              cellColIdx++) {
              ++tilePixelCounter;
              currentTilePosition = dataFile->tellg();
              if (currentTilePosition > nextTileOffset) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << endl << "Leaving processing of tile, ";
                      cout << "currentTilePosition: ";
                      cout << currentTilePosition;
                      cout << ", nextTileOffset: " << nextTileOffset;
                      cout << ", tilePixelCounter: ";
                      cout << tilePixelCounter;
                  }
                  return;
              }

              // read next byte and it's marker value
              // for the next run of bytes
              if ((dataRunPixelCount == 0) &&
                  (noDataRunPixelCount == 0)) {
                  // the first byte is a marker for a series of bytes´
                  const uint8_t markerByteValue =
                      esriRasterData->getUInt8FromFileStream(dataFile);
                      //cout << " marker:" << (int)markerByteValue << " ";

                  // check if the following run will be of NoData pixels
                  isNoDataType = (markerByteValue > 127);
                  if (isNoDataType)
                  {
                      noDataRunPixelCount = 256 - markerByteValue;
                  }
                  else
                    {
                      dataRunPixelCount = markerByteValue;
                    }
              }

              // the stype raster definition goes from bottom to top, so
              // the y-rasterIndex cell row values have to be turned over
              size_t rasterIndexX =
                  (tileColIdx*cellColCount + cellColIdx);
              size_t rasterIndexY = maxRasterIdxY -
                  (tileRowIdx*cellRowCount + cellRowIdx);

              // initialize raster index for current cell
              RasterIndex<2> rasterIndex = (int[2]) {
                rasterIndexX, rasterIndexY
              };

              // write NoData values into storage
              if (isNoDataType) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << " " << undefValue;
                  }
                  noDataRunPixelCount--;
              }
              // write 8Bit data integers into storage
              else {
                  // read unsigned integer data memory block
                  const uint8_t pixelValue =
                      esriRasterData->getUInt8FromFileStream(dataFile);

                  // write it to the raster storage cell
                  int resultValue = (int)pixelValue + minValue;
                  if (undefValue != resultValue) {
                      rasterStorage[rasterIndex] = resultValue;
                  }

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << " " << resultValue;
                  }
                  dataRunPixelCount--;
              }
          } // cell columns
      } // cell rows
    }

    /*
    RTileType 0xDF

    */
    template <typename T, typename Helper>
        void stype<T, Helper>::processEsriTileRMinRunsData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t nextTileOffset,
            const uint8_t rMinSize) {

      size_t tilePixelCounter = 0;
      storage_type& rasterStorage = this->getStorage();
      uint32_t currentTilePosition = 0;
      bool isNoDataType = false;
      uint8_t noDataRunPixelCount = 0;
      uint8_t dataRunPixelCount = 0;
      const int undefValue = esriRasterData->getEsriGridUndef();

      int minValue =
          esriRasterData->getIntFromFileStream(dataFile, rMinSize);

      if (DEBUG_OUTPUT_ENABLED) {
          cout << "Min value: " << minValue << endl;
      }

      // Iterate over cell rows (y-direction)
      for (uint32_t cellRowIdx=0; cellRowIdx<cellRowCount;
          cellRowIdx++) {

          // Iterate over cell columns (x-direction)
          for (uint32_t cellColIdx=0; cellColIdx<cellColCount;
              cellColIdx++) {
              ++tilePixelCounter;
              currentTilePosition = dataFile->tellg();
              if (currentTilePosition > nextTileOffset) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << endl << "Leaving processing of tile, ";
                      cout << "tilePixelCounter: " << tilePixelCounter;
                  }
                return;
              }

              // read next byte and it's marker value
              // for the next run of bytes
              if ((dataRunPixelCount == 0) &&
                  (noDataRunPixelCount == 0)) {
                  // the first byte is a marker for a series of bytes´
                  const uint8_t markerByteValue =
                      esriRasterData->getUInt8FromFileStream(dataFile);

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << " marker:" << (int)markerByteValue << " ";
                  }

                  // check if the following run will be of NoData pixels
                  isNoDataType = (markerByteValue > 127);
                  if (isNoDataType)
                  {
                      noDataRunPixelCount = 256 - markerByteValue;
                  }
                  else
                  {
                    dataRunPixelCount = markerByteValue;
                  }
              }

              // the stype raster definition goes from bottom to top, so
              // the y-rasterIndex cell row values have to be turned over
              size_t rasterIndexX =
                  (tileColIdx*cellColCount + cellColIdx);
              size_t rasterIndexY = maxRasterIdxY -
                  (tileRowIdx*cellRowCount + cellRowIdx);

              // initialize raster index for current cell
              RasterIndex<2> rasterIndex = (int[2]) {
                rasterIndexX, rasterIndexY
              };

              // write NoData values into storage
              if (isNoDataType) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << " " << undefValue;
                  }
                  noDataRunPixelCount--;
              }
              // write minValue integers into storage
              else {
                  // write minValue to the raster storage
                  if (undefValue != minValue) {
                      rasterStorage[rasterIndex] = minValue;
                  }

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << " " << minValue;
                  }
                  dataRunPixelCount--;
              }
          } // cell columns
      } // cell rows
    }

    template <typename T, typename Helper>
        void stype<T, Helper>::processEsriTileCountLengthData(
            ifstream *dataFile,
            const uint8_t *rTileType,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t nextTileOffset,
            const uint8_t rMinSize) {

      size_t tilePixelCounter = 0;
      storage_type& rasterStorage = this->getStorage();
      uint32_t currentTilePosition = 0;
      const int undefValue = esriRasterData->getEsriGridUndef();

      uint8_t countByteValue = 0;
      int intValue = 0;
      int minValue =
          esriRasterData->getIntFromFileStream(dataFile, rMinSize);

      if (DEBUG_OUTPUT_ENABLED) {
          cout << "Min value: " << minValue << endl;
      }

      // Iterate over cell rows (y-direction)
      for (uint32_t cellRowIdx=0; cellRowIdx<cellRowCount;
          cellRowIdx++) {

          // Iterate over cell columns (x-direction)
          for (uint32_t cellColIdx=0; cellColIdx<cellColCount;
              cellColIdx++) {
              ++tilePixelCounter;
              currentTilePosition = dataFile->tellg();
              if (currentTilePosition > nextTileOffset) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << endl << "Leaving processing of tile, ";
                      cout << "tilePixelCounter: " << tilePixelCounter;
                      cout << endl;
                  }
                  return;
              }

              // read next byte and it's count marker value
              // for the next run of bytes
              if (countByteValue == 0)
              {
                  countByteValue =
                      esriRasterData->getUInt8FromFileStream(dataFile);

                  intValue = esriRasterData->getIntValueByRTileType(
                      dataFile, rTileType, tilePixelCounter);
              }

              // the stype raster definition goes from bottom to top, so
              // the y-rasterIndex cell row values have to be turned over
              size_t rasterIndexX =
                  (tileColIdx*cellColCount + cellColIdx);
              size_t rasterIndexY = maxRasterIdxY -
                  (tileRowIdx*cellRowCount + cellRowIdx);

              // initialize raster index for current cell
              RasterIndex<2> rasterIndex = (int[2]) {
                rasterIndexX, rasterIndexY
              };
              int result = esriRasterData->getEsriGridUndef();
              int tempResult = (int)intValue + minValue;
              if ((countByteValue > 0) && (tempResult > result))
                {
                  result = tempResult;
                }

              if (undefValue != result) {
                  rasterStorage[rasterIndex] = result;
              }

              if (DEBUG_OUTPUT_ENABLED) {
                  cout << result << " ";
              }
              --countByteValue;
          } // cell columns
      } // cell rows
    }

    template <typename T, typename Helper>
        void stype<T, Helper>::processEsriTileRMinCCITTRLEData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t nextTileOffset,
            const uint16_t rTileSize,
            const uint8_t rMinSize) {

      int tilePixelCounter = 0;

      storage_type& rasterStorage = this->getStorage();
      uint32_t currentTilePosition = 0;
      const int undefValue = esriRasterData->getEsriGridUndef();

      int minValue =
          esriRasterData->getIntFromFileStream(dataFile, rMinSize);

      if (DEBUG_OUTPUT_ENABLED) {
           cout << "Min value: " << minValue << endl;
      }

      // rTileType algorithm taken from gdal library
      // frmts/aigrid/gridlib.c, line 287-317
      int numPixelBytes = rTileSize - 4 - rMinSize;
      int nDstBytes = (numPixelBytes + 7) / 8;
      unsigned char *byteIntermediate;
      byteIntermediate = (unsigned char *) malloc(nDstBytes);

      // Iterate over cell rows (y-direction)
      for (uint32_t cellRowIdx=0; cellRowIdx<cellRowCount;
          cellRowIdx++) {

          // Iterate over cell columns (x-direction)
          for (uint32_t cellColIdx=0; cellColIdx<cellColCount;
              cellColIdx++) {
              ++tilePixelCounter;
              currentTilePosition = dataFile->tellg();
              if (currentTilePosition > nextTileOffset) {

                  if (DEBUG_OUTPUT_ENABLED) {
                      cout << endl << "Leaving processing of tile, ";
                      cout << "tilePixelCounter: " << tilePixelCounter;
                  }
                  return;
              }

              int intValue = 0;
              if (byteIntermediate != NULL)
              {
                  if(byteIntermediate[tilePixelCounter>>3] &
                        (0x80 >> (tilePixelCounter & 0x7)))
                  {
                    intValue = 1;
                  }
                  else
                  {
                    intValue = 0;
                  }
              }

              // the stype raster definition goes from bottom to top, so
              // the y-rasterIndex cell row values have to be turned over
              size_t rasterIndexX =
                  (tileColIdx*cellColCount + cellColIdx);
              size_t rasterIndexY = maxRasterIdxY -
                  (tileRowIdx*cellRowCount + cellRowIdx);

              // initialize raster index for current cell
              RasterIndex<2> rasterIndex = (int[2]) {
                rasterIndexX, rasterIndexY
              };

              int resultValue = intValue + minValue;
              if (undefValue != resultValue) {
                  rasterStorage[rasterIndex] = resultValue;
              }

              if (DEBUG_OUTPUT_ENABLED) {
                  cout << resultValue << " ";
              }
          } // cell columns
      } // cell rows
    }

    /*
    Processes an Esri Grid data file by the RTileType.

    */
    template <typename T, typename Helper>
    void stype<T, Helper>::processEsriGridByRTileType(
        ifstream *dataFile,
        RasterData *esriRasterData,
        const uint32_t tileColIdx,
        const uint32_t tileRowIdx,
        const size_t maxRasterIdxY,
        const uint32_t cellColCount,
        const uint32_t cellRowCount,
        const uint32_t nextTileOffset) {

      size_t pos = dataFile->tellg();

      if (DEBUG_OUTPUT_ENABLED) {
          cout << endl << "Tile Offset: " << pos;
      }

      char tileInfoBuffer[4];
      dataFile->read(tileInfoBuffer, 4);

      uint16_t rTileSize = tileInfoBuffer[0];
      unsigned char rTileType = tileInfoBuffer[2];
      uint8_t rMinSize = tileInfoBuffer[3];

      if (DEBUG_OUTPUT_ENABLED) {
          cout << endl << "MinSize: " << (int)rMinSize;

          // print cell index
          cout << endl  << "Raster storage cell coordinates start: (";
          cout << (int)(tileColIdx * cellColCount) << ", ";
          cout << (int)(maxRasterIdxY - (tileRowIdx * cellRowCount));
          cout << ")";

          cout << endl << "Esri Grid data for RTileType 0x";
          cout << std::hex << setw(2) << setfill('0');
          cout << int(rTileType) << ":" << std::dec << endl;
          cout << "======================================================";
          cout << endl;
      }

      /*
      Choose the RTileType handler implementation
      for compressed integer data.

      */
      switch(rTileType) {
        case 0x00 :
          processConstantBlockData(
              dataFile,
              esriRasterData,
              tileColIdx,
              tileRowIdx,
              maxRasterIdxY,
              cellColCount,
              cellRowCount,
              rMinSize);
          break;

        case 0x01 :
        case 0x04 :
        case 0x08 :
        case 0x10 :
        case 0x20 :
          processEsriTilePixelValueData(
              dataFile,
              &rTileType,
              esriRasterData,
              tileColIdx,
              tileRowIdx,
              maxRasterIdxY,
              cellColCount,
              cellRowCount,
              nextTileOffset,
              rMinSize);
          break;

        case 0xCF :
          processEsriTile16BitLiteralRunsData(
              dataFile,
              esriRasterData,
              tileColIdx,
              tileRowIdx,
              maxRasterIdxY,
              cellColCount,
              cellRowCount,
              nextTileOffset,
              rMinSize);
          break;

        case 0xD7 :
          processEsriTileLiteralRunsData(
              dataFile,
              esriRasterData,
              tileColIdx,
              tileRowIdx,
              maxRasterIdxY,
              cellColCount,
              cellRowCount,
              nextTileOffset,
              rMinSize);
          break;

        case 0xDF :
          processEsriTileRMinRunsData(
              dataFile,
              esriRasterData,
              tileColIdx,
              tileRowIdx,
              maxRasterIdxY,
              cellColCount,
              cellRowCount,
              nextTileOffset,
              rMinSize);
          break;

        case 0xE0 :
        case 0xF0 :
        case 0xFC :
        case 0xF8 :
          processEsriTileCountLengthData(
              dataFile,
              &rTileType,
              esriRasterData,
              tileColIdx,
              tileRowIdx,
              maxRasterIdxY,
              cellColCount,
              cellRowCount,
              nextTileOffset,
              rMinSize);
          break;

        case 0xFF :
          processEsriTileRMinCCITTRLEData(
              dataFile,
              esriRasterData,
              tileColIdx,
              tileRowIdx,
              maxRasterIdxY,
              cellColCount,
              cellRowCount,
              nextTileOffset,
              rTileSize,
              rMinSize);
          break;

        default   :
          // Handle ERROR
          break;
       }
    }

    ListExpr importEsriGridTypeMap(ListExpr args)
    {
      const std::string error_message =
                  "expected single argument {text|string}";

          if(!nl->HasLength(args, 1))
                return listutils::typeError(error_message);

          ListExpr arg = nl->First(args);
          if(nl->ListLength(arg) !=2)
            return listutils::typeError(
                "Error, argument has to consists of 2 parts");

          ListExpr type = nl->First(arg);
          ListExpr value = nl->Second(arg);

          if(!listutils::isSymbol(type,CcString::BasicType())
             && !listutils::isSymbol(type,FText::BasicType()))
                return listutils::typeError(
                        "expected argument of type {text|string}");

          //retrieve the value from the argument list
          Word res;
          bool success =
          QueryProcessor::ExecuteQuery(nl->ToString(value),res);

          if(!success)
            return listutils::typeError("could not evaluate the value of  " +
                   nl->ToString(value) );

          string path;
          if(listutils::isSymbol(type,CcString::BasicType())){
           CcString* resText = static_cast<CcString*>(res.addr);
           if(!resText->IsDefined()){
                resText->DeleteIfAllowed();
                return listutils::typeError(
                                "filename evaluated to be undefined");
           }

           path = resText->GetValue();
           resText->DeleteIfAllowed();
          } else {
           FText* resText = static_cast<FText*>(res.addr);
           if(!resText->IsDefined()){
                resText->DeleteIfAllowed();
                return listutils::typeError(
                                "filename evaluated to be undefined");
           }

           path = resText->GetValue();
           resText->DeleteIfAllowed();
          }

          RasterData *EsriRasterData = new RasterData(false);
          int32_t celltype = EsriRasterData->getCellTypeFromFile(path);
          if(celltype == -1){
           delete EsriRasterData;
           EsriRasterData = 0;
                return listutils::typeError(
                      "error while file operation");
          }

          ListExpr returnObject;
          if(celltype == 2)
           returnObject = nl->SymbolAtom(sreal::BasicType());
          else if(celltype == 1)
           returnObject = nl->SymbolAtom(sint::BasicType());
          else {
           delete EsriRasterData;
           EsriRasterData = 0;
           return listutils::typeError(
                 "wrong cell type in HDR File");
           }

          delete EsriRasterData;
          EsriRasterData = 0;

          //Return correct object along with additional bool parameter
          return returnObject;
     }

    int importEsriGridSelectFun(ListExpr args) {
     NList type(args);

     if(type.first().isSymbol(CcString::BasicType()))
      return 0;

     if(type.first().isSymbol(FText::BasicType()))
      return 1;

     return 2;
    }

    ValueMapping importEsriGridFuns[] = {
     importEsriGridFun<CcString>,
     importEsriGridFun<FText>,
     0
    };

    /*
    The Esri Grid Import ValueMapping.

    */
    template<class inClass>
    int importEsriGridFun
    (Word* args, Word& result, int message, Word& local, Supplier s)
    {
      cout << endl << endl;
      cout << "Starting EsriGrid import..." << endl << endl;

      string importPath =
              static_cast<inClass*>( args[0].addr )->GetValue();

      RasterData *EsriRasterData = new RasterData(false);

      if(EsriRasterData->getEsriGridHDR(importPath) == 0){
       //retrieve handle to result object
       result = qp->ResultStorage(s);

       if(EsriRasterData->CellTypeReal()) {
        sreal* esriObject = static_cast<sreal*>(result.addr);

        //only testdata, will later be filled with real data
        esriObject->importEsriGridFile(EsriRasterData);
       }
       else {
        sint* esriObject = static_cast<sint*>(result.addr);

        //only testdata, will later be filled with real data
        esriObject->importEsriGridFile(EsriRasterData);
       }
      }
      delete EsriRasterData;
      EsriRasterData = 0;

      return 0;
    }

    template <typename T, typename Helper>
    int stype<T, Helper>::importEsriGridFile(RasterData *EsriRasterData)
    {
      if (DEBUG_OUTPUT_ENABLED) {
        cout << "Magic_number     : " <<
            EsriRasterData->currentGridHDR.HMagic << endl;
        cout << "CellType         : " <<
            EsriRasterData->currentGridHDR.HCellType << endl;
        cout << "Compressed       : " <<
            EsriRasterData->currentGridHDR.CompFlag << endl;
        cout << "HPixelSizeX      : " <<
            EsriRasterData->currentGridHDR.HPixelSizeX << endl;
        cout << "HPixelSizeY      : " <<
            EsriRasterData->currentGridHDR.HPixelSizeY << endl;
        cout << "XRef             : " <<
            EsriRasterData->currentGridHDR.XRef << endl;
        cout << "YRef             : " <<
            EsriRasterData->currentGridHDR.YRef << endl;
        cout << "HTilesPerRow     : " <<
            EsriRasterData->currentGridHDR.HTilesPerRow << endl;
        cout << "HTilesPerColumn  : " <<
            EsriRasterData->currentGridHDR.HTilesPerColumn << endl;
        cout << "HTileXSize       : " <<
            EsriRasterData->currentGridHDR.HTileXSize << endl;
        cout << "HTileYSize       : " <<
            EsriRasterData->currentGridHDR.HTileYSize << endl;
      }

     /*
     Processing of Esri grid files:

      - read global information about tiles etc. from index file
      - iterate over tiles in index file
      - read data of every tile in index file
      - jump to every tile in data file from address given in index file
      - read esri tile data from data file

     */

     const string dirname =
         EsriRasterData->esriGridConfigData.filePath;
     const string boundingFileName =
         dirname + FILE_SEPARATOR + BOUNDING_FILE_NAME;
     const string indexFileName =
         dirname + FILE_SEPARATOR + INDEX_FILE_NAME;
     const string dataFileName =
         dirname + FILE_SEPARATOR + DATA_FILE_NAME;
     const string statisticsFileName =
         dirname + FILE_SEPARATOR + STATISTICS_FILE_NAME;

     readEsriGridBounds(EsriRasterData, boundingFileName);

     // initialize the index file
     ifstream indexFile(indexFileName.c_str(), ios::in|ios::binary);
     if(!indexFile.is_open())
     {
         cerr << "problem in opening Esri index file" << endl;
         return 0;
     }
     readEsriGridIndexConfig(EsriRasterData, &indexFile);

     // evaluate actual tiles per row and column
     evaluateActualTilesRowAndColumnCount(EsriRasterData);

     // read statistic data from Esri statistics file
     readStatisticData(EsriRasterData, statisticsFileName);

     // evaluate number of rows and columns that will be filled with tiles
     const uint32_t tileRowCount =
         EsriRasterData->esriGridConfigData.actualNumberOfRows;
     const uint32_t tileColCount =
         EsriRasterData->esriGridConfigData.actualNumberOfColumns;

     // evaluate cell(pixel) count for each tile
     const uint32_t cellRowCount =
         EsriRasterData->currentGridHDR.HTileYSize;
     const uint32_t cellColCount =
         EsriRasterData->currentGridHDR.HTileXSize;

     if (DEBUG_OUTPUT_ENABLED) {
         cout << "Number of cell columns (x-direction): ";
         cout << (EsriRasterData->esriGridConfigData.actualNumberOfColumns
               * EsriRasterData->currentGridHDR.HTileXSize) << endl;
         cout << "Number of cell rows    (y-direction): ";
         cout << EsriRasterData->esriGridConfigData.actualNumberOfRows
               * EsriRasterData->currentGridHDR.HTileYSize << endl;
     }

     indexFile.seekg(100,ios::beg);

     // initialize the Esri data file
     ifstream dataFile(dataFileName.c_str(), ios::in|ios::binary);
     if(!dataFile.is_open())
     {
         cerr << "Problem in opening EsriGrid data file." << endl;

         indexFile.close();
         dataFile.close();
         return 0;
     }

     //get storage of current sint/sreal
     storage_type& rasterStorage = this->getStorage();

     // cell length
     double cellLength = 1.0;
     if (EsriRasterData->currentGridHDR.HPixelSizeX ==
           EsriRasterData->currentGridHDR.HPixelSizeY)  {
         cellLength = EsriRasterData->currentGridHDR.HPixelSizeX;
     }

     // initialize grid
     // coordinates of lower left geographic corner
     grid = grid2(
         EsriRasterData->esriGridConfigData.llxCoord,
         EsriRasterData->esriGridConfigData.llyCoord,
         cellLength);

     // biggest and smallest defined values from all data cells
     minimum = EsriRasterData->esriGridConfigData.esriGridMinimum;
     maximum = EsriRasterData->esriGridConfigData.esriGridMaxmimum;

     const int undefValue = EsriRasterData->getEsriGridUndef();

     /*
     The data file w001001.adf contains data ordered by tiles.
     All data will be read tile by tile.
     So we do not have to iterate mainly over cell rows but
     over tiles. Therefore all cells of a tile will be read
     completely for a tile. Then all cells of the next tile
     will be read.

     */

     // the upper limit of cells in y-direction (count of cell rows)
     const size_t maxRasterIdxY = tileRowCount * cellRowCount;

     // cell count for debugging
     int debugTileCount = 0;

     if (DEBUG_OUTPUT_ENABLED) {
         cout << endl << endl;
         cout << "Storing Esri Grid Data to Raster Storage...";
         cout << endl << endl;
     }

     // Iterate over tile rows (y-direction)
     for (uint32_t tileRowIdx=0; tileRowIdx<tileRowCount;
         tileRowIdx++) {

         // Iterate over tile columns (x-direction)
         for (uint32_t tileColIdx=0; tileColIdx<tileColCount;
             tileColIdx++) {

             ++debugTileCount;
             if (DEBUG_OUTPUT_ENABLED) {
                 cout << endl << endl << std::dec;
                 cout << "Tile No: " << debugTileCount;
             }

             char indexBuffer[INDEX_BUFFER_SIZE];

             // read next position in index file
             indexFile.read(indexBuffer, INDEX_BUFFER_SIZE);

             // read next tile offset (next tile position)
             uint32_t offset =
                 EsriRasterData->getUInt32(indexBuffer, 0) * 2;

             uint32_t tileSize =
                 EsriRasterData->getUInt32(indexBuffer, 4) * 2;
             uint32_t nextTileOffset = offset + tileSize + 2;

             if (DEBUG_OUTPUT_ENABLED) {
                 cout << endl << "Tile Size (bytes): " << tileSize;
                 cout << endl << "Tile coordinates (x,y): ";
                 cout << "(" << tileColIdx << ",";
                 cout << (tileRowCount-tileRowIdx) << ")";
             }

             /*
             Process the tiles.

             */

             // read next tile
             dataFile.seekg(offset,ios::beg);

             /*
             For floating point and uncompressed integer files
             the data is just the tile size (RTileSize)
             in two bytes beginning from 'offset' followed
             by the pixel data as 4 byte words.

             Every compressed tile on the other hand contains
             the following information at the beginning:
             - byte 0-1: RTileSize
             - byte 2  : RTileType
             - byte 3  : RMinSize
             For compressed integer tiles it is necessary
             to interpret the RTileType to establish the
             details of the tile organization

             */

             // Read compressed data dependent on the RTileType.
             if (EsriRasterData->currentGridHDR.CompFlag == 0) {
              processEsriGridByRTileType(
                   &dataFile, EsriRasterData,
                   tileColIdx, tileRowIdx, maxRasterIdxY,
                   cellColCount, cellRowCount,
                   nextTileOffset);
             /*
             Not compressed data:
             Only read the first 2 bytes (RTileSize),
             then read the data (integer or float) in
             4 byte portions.

             */
             } else if (tileSize > 0) {

                 float undefValueFloat =
                     EsriRasterData->getEsriGridFloatUndef();

                 size_t pos = dataFile.tellg();

                 if (DEBUG_OUTPUT_ENABLED) {
                     cout << endl << "Tile Offset: " << pos << endl;
                 }

                 // Iterate over cell rows (y-direction)
                 for (uint32_t cellRowIdx=0; cellRowIdx<cellRowCount;
                     cellRowIdx++) {

                     // Iterate over cell columns (x-direction)
                     for (uint32_t cellColIdx=0; cellColIdx<cellColCount;
                         cellColIdx++) {

                         /*
                         Process one cell/pixel value.

                         */

                         // the stype raster definition goes from bottom
                         // to top, so the y-rasterIndex values (of the
                         // cell rows) have to be turned over
                         size_t rasterIdxY = maxRasterIdxY -
                             (cellRowIdx*cellRowCount + cellRowIdx);
                         size_t rasterIdxX =
                             (tileColIdx*cellColCount + cellColIdx);

                         // initialize raster index for current cell
                         RasterIndex<2> rasterIndex = (int[2]) {
                           rasterIdxY, rasterIdxX
                         };

                         // read data memory block
                         char* dataMemoryBlock = new char[4];
                         dataFile.read(dataMemoryBlock, 4);

                         // distinguish between integer and real values
                         if (EsriRasterData->CellTypeReal()) {

                            // floating points in single precision (32 bits)
                            float floatValue =
                                *((float*) dataMemoryBlock);
                            float f_nan = numeric_limits<float>::quiet_NaN();
                            if (floatValue != f_nan)
                              {
                                if (EsriRasterData->getEndianTypeLittle())
                                {
                                    floatValue =
                                    EsriRasterData->convertFloat(floatValue);
                                }
                                int intTemp = floatValue * 100;
                                floatValue = intTemp/100.0;

                                // write it to the raster storage
                                // TODO correct limits
                                if ((floatValue > undefValueFloat)
                                    && (floatValue < 1000)
                                    && (floatValue > -1000))
                                {
                                    rasterStorage[rasterIndex] = floatValue;
                                }
                              }

                            if (DEBUG_OUTPUT_ENABLED) {
                                cout << " " << floatValue;
                            }
                         }
                         else {
                             int32_t intValue = *((int32_t*) dataMemoryBlock);
                             if (EsriRasterData->getEndianTypeLittle())
                             {
                                 intValue =
                                     EsriRasterData->convertEndian(intValue);
                             }

                             // write it to the raster storage
                             if (undefValue != intValue)
                             {
                                 rasterStorage[rasterIndex] = intValue;
                             }

                             if (DEBUG_OUTPUT_ENABLED) {
                                 cout << " " << intValue;
                             }
                         } // cell type integer
                     } // cell columns
                 } // cell rows
             } // else not compressed data
             else {
                 cout << endl << "Tile will not be processed";
             }
         } // tile columns
     } // tile rows
     indexFile.close();
     dataFile.close();
     cout << endl << endl << "Done." << endl;
     cout << "Successfully imported EsriGrid Data" << endl << endl;

     return 0;
    }
}
