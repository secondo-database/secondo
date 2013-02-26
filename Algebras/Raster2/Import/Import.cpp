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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <fstream>
//#include <unzip.h>
//#include "../RasterStorage.h"
#include "Import.h"

const string esriGridDataFile =
    "/home/fapra/workspace/ImportTest/src/esridata/w001001.adf";
const string esriGridIndexFile =
    "/home/fapra/workspace/ImportTest/src/esridata/w001001x.adf";

/*
TODO
larger memory blocks
evaluate count of data blocks
compare differences to other resolutions
check correct positions in file
error checks

*/

/*
print and evaluate Esri Grid data file TEST

*/
void testReadPartsOfEsriGridDataFile() {

  int length;
  ifstream fileStream (esriGridDataFile.c_str(),
      ios::in|ios::binary|ios::ate);

  if (fileStream.is_open())
  {
    length = fileStream.tellg();
    cout << "Reading Esri Data File w001001.adf" << endl;
    cout << "----------------------------------" << endl << endl;
    cout << "Number of bytes in Esri Data file: " << length << endl;
    // correct: validated against hex editor

    cout << endl << "Start Byte 24: " << endl;
    char* sizeBytesBlock = new char[4];
    fileStream.seekg (24, ios::beg);
    fileStream.read(sizeBytesBlock, 4);
    cout << "Size of whole file as int32 (RFileSize): ";
    int t_num = *((int*) sizeBytesBlock);
    cout << t_num << endl << endl;
    delete[] sizeBytesBlock;

    cout << "Start Byte 100: " << endl;
    char* tilesSizeBlock = new char[2];
    fileStream.seekg (100, ios::beg);
    fileStream.read(tilesSizeBlock, 2);
    cout << "Size of current tile data as int16 (RTileSize): ";
    int t_tiles_num = *((short*) tilesSizeBlock);
    cout << t_tiles_num << endl << endl;
    delete[] tilesSizeBlock;

    cout << "Start Byte 102: " << endl;
    char* tileTypeBlock = new char[1];
    fileStream.seekg (102, ios::beg);
    fileStream.read(tileTypeBlock, 1);
    char tileType = *((char*) tileTypeBlock);
    cout << "TileType: 0x" << std::hex << setw(2) << setfill('0');
    cout << int(tileType) << endl << endl;
    delete[] tileTypeBlock;

    cout << "Start Byte 103: " << endl;
    char* rMinSizeBlock = new char[1];
    fileStream.seekg (103, ios::beg);
    fileStream.read(rMinSizeBlock, 1);
    char rMinSize = *((char*) rMinSizeBlock);
    cout << "RMinSize: 0x" << std::hex << setw(2) << setfill('0');
    cout << int(rMinSize) << endl;
    const int minSize = int(rMinSize);
    cout << "MinSize as int16: " << minSize << endl << endl;
    delete[] rMinSizeBlock;

    cout << "Start Byte 104: " << endl;
    char* rMinBlock = new char[minSize];
    fileStream.seekg (104, ios::beg);
    fileStream.read(rMinBlock, minSize);
    cout << "Minimum value pixels for this tile (RMin): ";
    int minSizeValue = *((int*) rMinBlock);
    cout << std::dec << minSizeValue << endl << endl;
    delete[] rMinBlock;


    // first data position (byte 104 + MinSize)
    cout << "Start Byte 104 + RMinSize (" << minSize << " Bytes): " << endl;
    fileStream.seekg (104 + minSize, ios::beg);
    cout << "First 10 data integers:" << endl;
    cout << "-----------------------" << endl;
    char* intTestBlock;
    for (int i=0; i<10; i++) {
        intTestBlock = new char[4];
        fileStream.read(intTestBlock, 4);
        int int_num = *((int*) intTestBlock);
        cout << int_num << endl;
    }
    cout << endl;
    delete[] intTestBlock;
  }
  else {
      cout << "Unable to open file";
  }
}

/*
print and evaluate Esri Grid index file TEST

*/
void testReadIndexFile() {

  int length;
  ifstream fileStream (esriGridDataFile.c_str(), ios::in|ios::binary|ios::ate);

  if (fileStream.is_open())
  {
    length = fileStream.tellg();
    cout << endl << "Reading Esri Index File w001001x.adf" << endl;
    cout << "------------------------------------" << endl << endl;
    cout << "number of bytes in Index file: " << length << endl << endl;

    cout << "Start Byte 24: " << endl;
    char* sizeBytesBlock = new char[4];
    fileStream.seekg (24, ios::beg);
    fileStream.read(sizeBytesBlock, 4);
    cout << "Size of whole file in shorts: ";
    int t_num = *((int*) sizeBytesBlock);
    cout << t_num << endl;
    cout << "Short values count: ";
    short shortValues = *((short*) sizeBytesBlock);
    cout << shortValues << endl << endl;
    delete[] sizeBytesBlock;

    cout << "Start Byte 100: " << endl;
    char* firstTileOffsetBytesBlock = new char[4];
    fileStream.seekg (76, ios::cur);
    fileStream.read(firstTileOffsetBytesBlock, 4);
    cout << "4 bytes as int from position 100: " << endl;
    cout << "Byte at position 100: ";
    int firstTile_num = *((int*) firstTileOffsetBytesBlock);
    cout << firstTile_num << endl;
    short firstTileOffsetShortValues = *((short*) firstTileOffsetBytesBlock);
    cout << "Offset to first tile: " << firstTileOffsetShortValues;
    cout << endl << endl;
    delete[] firstTileOffsetBytesBlock;

    cout << "Start Byte 104: " << endl;
    char* firstTileSizeBytesBlock = new char[4];
    fileStream.seekg (4, ios::cur);
    fileStream.read(firstTileSizeBytesBlock, 4);
    cout << "4 bytes as int from position 104: " << endl;
    cout << "Byte at position 104: ";
    int firstTileSize_num = *((int*) firstTileSizeBytesBlock);
    cout << firstTileSize_num << endl;
    short firstTileSizeShortValues = *((short*) firstTileSizeBytesBlock);
    cout << "Size of first tile: " << firstTileSizeShortValues;
    cout << endl << endl;
    delete[] firstTileSizeBytesBlock;
  }
}

/*
print memory blocks of 50 bytes of Esri Grid data file TEST

*/
void testPrintBlocksOf50Bytes() {

  const string filename = "/home/fapra/workspace/ImportTest/src/w001001.adf";

  char buffer[50];
  std::ifstream fileStream(filename.c_str(),
      std::ios_base::binary | std::ios_base::in);
  while (fileStream.read(buffer, sizeof(buffer)))
  {
      if (!fileStream.eof())
      {
          std::cout << std::string(buffer) << std::endl;
      }
  }

  std::cout << "Done" << std::endl;
  fileStream.close();
}



using namespace raster2;

int getRecordSize(void) {
//int anz = 0;

//size_t pageSize = static_cast<size_t>(WinUnix::getPageSize());
//size_t intSize = sizeof(int);

// two Keys for CellKey
//anz = static_cast<int>(sqrt ((pageSize - 2.*intSize)/sizeof(int)));

return 0;
}

/*
main()

*/
int main(int argc, char** argv) {

	if ( argc == 1 ) {

		cout << endl;
		cout << "This is only for DEV" << endl;
		cout << "Usage: " << "Import File/Path" << endl;
		cout << endl;
		exit(1);
	}

	cout << endl << "Starting main() ..." << endl << endl;

	// Constructor Parameter: HGTValueByteSize
	bool useEarthOrigin = false;
	RasterData *rasterSys = new RasterData(useEarthOrigin);

	// optional tests to be removed later
   //     testReadPartsOfEsriGridDataFile();
   //     testReadIndexFile();

//	rasterSys->ImportHGT(argv[1]);
//	rasterSys->getEsriGridHDR(argv[1]);

//	rasterSys->getEsriRasterHDR(argv[1], true);
//	int bla = rasterSys->getRasterXOffset();
//	bool test = rasterSys->checkRasterOffset(
//			rasterSys->getRasterXOffset(),
//			rasterSys->getRasterYOffset());
//	rasterSys->getEsriRasterData(argv[1]);
//	delete rasterSys;
//	rasterSys = 0;
//	int tilecount = 0;
//	int tilesize = 30;
//	int tilesizecounter = -1;
//	int x1 = 0;
//	for(int x = 0; x < 1201; x++){
//
//		if((x % tilesize) == 0){
//			tilecount++;
//			tilesizecounter = 0;
//		}
//
//		x1 = (tilecount*tilesize) - tilesizecounter;
//		cout << x1 << endl;
//		tilesizecounter++;
//	}
	rasterSys->getEsriRasterHDR(argv[1], true);
	double* buffer = rasterSys->getEsriRasterData(argv[1]);
	if(buffer != NULL)
     cout << "Import OK" << endl;
	else
     cout << "Import failed" << endl;

	cout << endl << "Done. Finished main()." << endl;
	return 0;
}

/*
Template class for import of HGT files.

*/

int RasterData::ImportHGT(const string HGTDataRoot){
	if(!checkRootIsValid(HGTDataRoot))
		return 1;

	vector<string> Files = buildFilesVector(HGTDataRoot);
	if(Files.size() == 0) {
		cout << "No files found." << endl;
	} else {
		// sort using default comparison (operator <):
		std::sort(Files.begin(), Files.end());

		//iterate thru the files
		vector<string>::const_iterator FilesIterator;
		for(FilesIterator=Files.begin();
				FilesIterator!=Files.end();
				FilesIterator++)
		{
		 const string &filename = static_cast<string>( *FilesIterator );
		 cout << "processing file: \""
				 << filename.c_str()
				 << "\"" << endl;

		 //check if we are dealing with an Archive
		 if(endsWith(filename.c_str(),".zip"))
		 {
		  cout << "ZIP-File support currently not available" << endl;
/*		    	 //build a new Vector with Archive content
		    	 vector<string> HGTFiles = getFileFromArchive(filename.c_str(), HGTDataRoot);
		    	 if(HGTFiles.size() != 0) {
					 // sort using default comparison (operator <):
					 std::sort(HGTFiles.begin(), HGTFiles.end());

		    		 //iterate through the archive files
		    		 vector<string>::const_iterator HGTIterator;
		    		 for(HGTIterator=HGTFiles.begin();HGTIterator!=HGTFiles.end();HGTIterator++){
		    		     const string &HGTfilename = static_cast<string>( *HGTIterator );

		    		     processFile(HGTfilename.c_str());
		    			 if(remove(HGTfilename.c_str())) {
		    				 cout << "Error deleting file \"" << HGTfilename.c_str() << "\"" << endl;
		    			 }
		    		 }
		    	 }*/
		  }
		 	 else
		  {
		   processFile(filename.c_str());
		  }
		  cout << endl;
		}
	}

	return 0;
}


bool RasterData::checkRootIsValid(string HGTDataRoot){

	struct stat state;

	int rc = stat(HGTDataRoot.c_str(),&state);
	if(rc!=0){
	  cerr << "Directory \""
			  << HGTDataRoot
			  << "\" does not exists." << endl;
	  return false;
	}

	int type = state.st_mode & S_IFMT;
	if(type != S_IFDIR){
	   cerr << "\"" << HGTDataRoot << "\" is not a directory." << endl;
	   return false;
	}

	return true;
}


vector<string> RasterData::buildFilesVector(string HGTDataRoot) {
	vector<string> Files;
	DIR* hdir;
	struct dirent* entry;
	hdir = opendir(HGTDataRoot.c_str());
	struct stat state;
	do
	{
	 entry = readdir(hdir);
	 if(entry)
	 {
	  string fnShort = entry->d_name;
	  string fnFQ = HGTDataRoot + "/" + entry->d_name;

	  int rc = stat(fnFQ.c_str(),&state);
	  if(rc==0)
	  {
	   int type = state.st_mode & S_IFMT;
	   if((type == S_IFREG) &&
			   (endsWith(fnShort,".zip")
					   || endsWith(fnShort,".hgt")))
			Files.push_back(fnFQ);
	  }
	 }
	} while(entry);
	closedir(hdir);

	return Files;
}

int RasterData::processFile(const string HGTFile) {
	double currentHGTFileSize = FileSystem::GetFileSize(HGTFile.c_str());
	int currentUndefCount = 0;
	int currentMax = 0;
	int currentMin = 0;
	size_t valueswritten = 0;
	bool topdown = true;
	int rownr = 0;

	// HGT Files are written in 16bit signed integer, leads to 2Bytes
	int currentValueCount = currentHGTFileSize / 2;
	int currentHGTExtend = sqrt(currentValueCount);

	if(checkHGTExtend(currentHGTExtend)){
		cout << "HGT Extend: " << currentHGTExtend
				<< "x" << currentHGTExtend << endl;
	} else {
		cout << "Wrong HGT extend, skipping file..." << endl;
		return 1;
	}

	if(!checkGeoCoordinates(getLengthOrientationFromFileName(HGTFile),
			getLengthOrientationValueFromFileName(HGTFile),
			getWidthOrientationFromFileName(HGTFile),
			getWidthOrientationValueFromFileName(HGTFile))){
		cout << "Skipping File..." << endl;
		return 1;
	} else {

	calculateCoordinates(getLengthOrientationFromFileName(HGTFile),
				getLengthOrientationValueFromFileName(HGTFile),
				getWidthOrientationFromFileName(HGTFile),
				getWidthOrientationValueFromFileName(HGTFile));
	}

	int Xoffset = getXOffset();
	int Yoffset = getYOffset();
	long fromX = Xoffset * (ImportHGTExtend-1);
	//long toX = (Xoffset * (ImportHGTExtend-1)) + (ImportHGTExtend - 2);
	long fromY = (Yoffset * (ImportHGTExtend-1));
	//long toY = (Yoffset * (ImportHGTExtend-1)) + (ImportHGTExtend - 2);

	int16_t* buffer = getHGTData(HGTFile);

	if(buffer == NULL)
		return 1;
	if(topdown) {
	/* within the HGT file the file starts
	 *  at the top of the represented tile
	 *  currentHGTExtend -1 because of
	 *  reading effectiviliy 3600 cells 0-3599
	 */
	for(int y = 0; y < currentHGTExtend -1 ; y++){
     cout << "row: " << currentHGTExtend - (y + 2) << endl;
     cout << "columnStart: 0 "
    	<< "("
    	<< (currentHGTExtend - (y + 2)) * currentHGTExtend
    	<< ")" << endl;

     for(int x = 0; x < currentHGTExtend - 1;x++){
	  int y1 = currentHGTExtend - (y + 2);
	  int pos = y1*currentHGTExtend + x;

	  //TODO: interim
	  rownr = x;

	  int16_t v = buffer[pos];

	  if(endianTypeLittle){ // hgt uses big endian
	    v = convertEndian(v);
	  }

	  if(v == undefValueHGT){
//	   cout << " undef ";
	   currentUndefCount++;
	  } else {
//	   cout << " " << v << " " ;

	   if(v < currentMin){
	     currentMin = v;
	   }

	   if(v > currentMax){
	    currentMax = v;
	   }
	  }

	  valueswritten++;
	 }

     cout << "columnEnd: " << rownr
    	<< " ("
    	<< (currentHGTExtend - (y + 2)) * currentHGTExtend + rownr
    	<< ")" << endl;
	}
	} else {
		/* within the HGT file the file starts
		 *  at the lower left corner of the represented
		 *  tile. CurrentHGTExtend -1 because of
		 *  reading effectiviliy 3600 cells 0-3599
		 */
		for(int y = 0; y < currentHGTExtend -1 ; y++){
	     cout << "row: " << y << endl;
	     cout << "columnStart: 0 "
	    	<< "("
	    	<< y * currentHGTExtend
	    	<< ")" << endl;

	     for(int x = 0; x < currentHGTExtend - 1;x++){
		  int y1 = y;
		  int pos = y1*currentHGTExtend + x;

		  //TODO: interim
		  rownr = x;

		  int16_t v = buffer[pos];

		  if(endianTypeLittle){ // hgt uses big endian
		    v = convertEndian(v);
		  }

		  if(v == undefValueHGT){
	//	   cout << " undef ";
		   currentUndefCount++;
		  } else {
	//	   cout << " " << v << " " ;

		   if(v < currentMin){
		     currentMin = v;
		   }

		   if(v > currentMax){
		    currentMax = v;
		   }
		  }

		  valueswritten++;
		 }

	     cout << "columnEnd: " << rownr
	    	<< " ("
	    	<< y * currentHGTExtend + rownr
	    	<< ")" << endl;
		}
	}
	// print out some statistics

	cout << "Grid Origin: (" << fromX << "," << fromY << ")" << endl;
	cout << "Min        : " << currentMin << endl;
	cout << "Max        : " << currentMax << endl;
	cout << "noUndef    : " << currentUndefCount << endl;
	cout << "valuesWritten : " << valueswritten << endl;

	delete[] buffer;
	return 0;
}
/* template <class T> vector<string> RasterData<T>::getFileFromArchive(string archiveFile, string workingDir) {
	//return value
	vector<string> HGTFiles;

	// Target Filename
	char filename[ MAX_FILENAME ];
    char extractFilename[ MAX_FILENAME ];

    // Buffer to hold data read from the zip file.
    char read_buffer[ READ_SIZE ];

    // open the zip file
	unzFile ZIPFile = unzOpen(archiveFile.c_str());
    if ( ZIPFile == NULL )
    {
        printf( "%s: not a valid archive\n",  archiveFile.c_str());
        return HGTFiles;
    }

    // Get info about the zip file
    unz_global_info global_info;
    if ( unzGetGlobalInfo( ZIPFile, &global_info ) != UNZ_OK )
    {
        printf( "could not read file global info\n" );
        unzClose( ZIPFile );
        return HGTFiles;
    }

     // Loop to extract all files
     for ( uLong i = 0; i < global_info.number_entry; ++i )
     {
         // Get info about current file.
         unz_file_info file_info;
         if ( unzGetCurrentFileInfo(
        	 ZIPFile,
             &file_info,
             filename,
             MAX_FILENAME,
             NULL, 0, NULL, 0 ) != UNZ_OK )
         {
             printf( "could not read file info\n" );
             unzClose( ZIPFile );
             return HGTFiles;
         }

         // Entry is a file, so extract it.
         if ( unzOpenCurrentFile( ZIPFile ) != UNZ_OK )
         {
        	 printf( "could not open file\n" );
        	 unzClose( ZIPFile );
        	 return HGTFiles;
         }

         //generate full qualified targetname
         sprintf(extractFilename, "%s/%s", workingDir.c_str(), filename);

         // Open a file to write out the data.
         FILE *out = fopen( extractFilename, "wb" );
         if ( out == NULL )
         {
        	 printf( "could not open destination target file\n" );
        	 unzCloseCurrentFile( ZIPFile );
        	 unzClose( ZIPFile );
        	 return HGTFiles;
         }

         //unpack the file
         cout << "> unpack file " << filename;
         int error = UNZ_OK;
         do
         {
        	 error = unzReadCurrentFile( ZIPFile, read_buffer, READ_SIZE );
        	 if ( error < 0 )
        	 {
        		 printf( "error %d\n", error );
        		 unzCloseCurrentFile( ZIPFile );
        		 unzClose( ZIPFile );
        		 return HGTFiles;
        	 }

        	 // Write data to file.
        	 if ( error > 0 )
        		 fwrite( read_buffer, error, 1, out );
         } while ( error > 0 );

         //check if fwrite completes & collect HGT Files from the Archive
         if((unsigned)ftell(out) == file_info.uncompressed_size) {
             cout << " -> done" << endl;

             if(endsWith(extractFilename,".hgt")) {
        		 HGTFiles.push_back(extractFilename);
        	 }
         } else {
             cout << " -> failed" << endl;
         }

         fclose( out );
         unzCloseCurrentFile( ZIPFile );
     }

     return HGTFiles;
}*/
