
/*
---- 
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science, 
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

paragraph [1] title: [{\Large \bf ]   [}]
[->] [$\rightarrow$]

[1] UGrid 
March 2009 Brigitte Metzker

0 Overview



Support functions

  * nearlyequal
  * initialize
  * split
  * ostream  
  * enter
  * dump
  * calculateDistance
  * calculateSpeed
  * calculateDir
  * calculatePrePos
     
1 Preliminaries

*/

#ifndef __UGRID_UTILITIES_H__
#define __UGRID_UTILITIES_H__

#include "stdarg.h"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#include <iostream>
#include <stack>
#include <limits>
#include <string.h>

// Includes required by extensions for the NearestNeighborAlgebra:
#include <vector>
#include <queue>


using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "DBArray.h"

namespace ugridAlgebra
{

/*
 class ugridutilities                           
 some auxiliary functions for processing moving objects        
  
  Methoden zum Berechnen der Distanz, die das MovingObject zwischen zwei 
  UpdateUnits zurueckgelegt hat,
  zum Berechnen des Zeitintervalls zwischen zwei UpdateUnits, 
  zum Berechnen der Geschwindigkeit, in der die Distanz zurueckgelegt wurde 
  zum Berechnen der Richtung, in der sich das MovingObject bewegen wird und
  zum Berechnen der naechsten Position des MovingObjects = die Position, an  
  der das MovingObject in 1 Minute sein wird, wenn es sich mit der 
  berechneten Geschwindigkeit in die berechnete Richtung fortbewegt.

*/
class UGridUtilities
{
public:

	/*
	  Methode zum Berechnen der zurueckgelegten Distanz

	*/
	double Distance (double x, double y, double nx, double ny);
	
	/*
	  Methode zum Berechnen der Geschwindigkeit

	*/
    double Speed (double s, long t);
    
    /* 
	 Methode zum Berechnen der Bewegungsrichtung

	*/
    double Direction (double x, double y, double nx, double ny);


	/*
	 Methode zum Berechnen der Next-Position

	*/
    void predictPosition (double &nx, double &ny, double x, double y);

    /*
	  Methode zum Berechnen der ersten Next-Position
	  Aufruf beim ersten Update; 
	  zu diesem Zeitpunkt noch keine CurrentUnit zu dieser ID 

	*/
    void firstPredictPosition (double &nx, double &ny, double x, double y);
};
	/*
	  Abspeichern der UpdateInfo nach Hinzufuegen der berechneten 
	  Next-Position als CurrentUnit

	*/

/*
  For representation movingObjects the class movingObjects offers      
  the data types                               

*/

/* 
  Keys for Moving Objects

*/

class MovingObjectKey 
{
public:
	int    id;
	string licence;
	MovingObjectKey(int _id, string _licence);
};
ostream& operator << (ostream &cout, const MovingObjectKey &mok);
//istream& operator >> (istream &cin, const MovingObjectKey &mok);

struct CompareKeys {
  bool operator() (const MovingObjectKey mok1, 
	               const MovingObjectKey mok2)const;
};

/*
 Associated information for Moving Objects

*/

class MovingObject {
  
public:
	string description;      //describes the kind os motorcar
	
	MovingObject(string _description = "");
};
ostream& operator << (ostream &cout, const MovingObject &mo);
//istream& operator >> (istream &cin, const MovingObject &mo);

typedef map<MovingObjectKey,MovingObject,CompareKeys> mobmap;

class StoreMovingObject {
private:
	mobmap theStore;

public:
	// Funktion zum Eingeben
	void enter(int id, string licence, string description);
	// Funktion zum Auslesen
	bool read(int id, string f_licence , string f_description);

	// Funktion zur Ausgabe des Speichers
	void dump(void);
};

/*
 For representation current information the class currentUnit offers      
                          the following data types                         

 Keys for CurrentUnits of Moving Objects

*/

class CurrentMovingObjectKey 
{
public:
	int cid;
	CurrentMovingObjectKey(int _cid);
};
ostream& operator << (ostream &cout, const CurrentMovingObjectKey &cmok);

struct CurrentCompareKeys {
  bool operator() (const CurrentMovingObjectKey cmok1, 
	               const CurrentMovingObjectKey cmok2)const;
};

/*
  Associated information for CurrentUnits of Moving Objects

*/

class CurrentMovingObject {
  
public:
	long stimestamp;  // Zeitstempel = Zeitpunkt des Eintrags
	long etimestamp;  // Zeitstempel = Zeitpunkt in einer Minute
	double x;         // x-Koordinate der aktuellen Position
	double y;         // y-Koordinate der aktuellen Position
	double nx;        // x-Koordinate der nächsten Position
	double ny;        // y-Koordinate der nächsten Position 

	CurrentMovingObject(long _stimestamp = 0, long _etimestamp = 0, 
		                double _x = 0, double _y = 0, double _nx = 0, 
						double _ny = 0);
};
ostream& operator << (ostream &cout, const CurrentMovingObject &cmo);

typedef map<CurrentMovingObjectKey,CurrentMovingObject,CurrentCompareKeys> 
          currentmap;

class StACurrentUnit
{
private:
	currentmap theCurrentUnit;

public:
	// Funktion zum Eingeben
	void enter(int cid, long stimestamp, long etimestamp, 
		       double x, double y, double nx, double ny);

	// Funktion zur Ausgabe des Speichers
	void dump(void);

    // search entry
    bool searchEntry(int c_id);

	// read entry
    void readEntry (int id, long &cutimestamp, double &cux, double &cuy);

	// Funktion zum Lesen der Id
    int getId(int c_id);

	// Funktion zum Lesen des MovingObjects
    void getMovingObject(int c_id, double &nx, double &ny);
	
	// Funktion zum Selektieren der Ids, die in einer bestimmten Zeit
	// in einem bestimmten Bereich sein werden.
	void selectAllId(long qu_time, 
	  double qu_x1, double qu_x2, double qu_y1, double qu_y2);
	// Funktion zum Selektieren der Id, dis als erste
	// in einem bestimmten Bereich sein wird.
	void selectFirstId(double qu_x1, double qu_x2, 
	  double qu_y1, double qu_y2);
};

/*
   For representation history information the class historyUnit offers       
                        the following data types                             
  Keys for HistoryUnits of Moving Objects

*/

class HistoryMovingObjectKey 
{
public:
	int  id;
	long starttime;
	HistoryMovingObjectKey(int _id, long _starttime);
};
ostream& operator << (ostream &cout, const HistoryMovingObjectKey &hmok);

struct HistoryCompareKeys {
	bool operator() (const HistoryMovingObjectKey hmok1, 
		             const HistoryMovingObjectKey hmok2)const;
};

/*
  Associated information for HistoryUnits of Moving Objects

*/

class HistoryMovingObject {
  
public: 
	long endtime;    // endtime gehoert nicht mehr ins Zeitinterval
	double x;        // x-Koordinate der aktuellen Position
	double y;        // y-Koordinate der aktuellen Position
	double nx;       // x-Koordinate der nächsten Position
	double ny;       // y-Koordinate der nächsten Position 

	HistoryMovingObject(long _endtime = 0, double _x = 0, double _y = 0, 
		                double _nx = 0, double _ny = 0);

};
ostream& operator << (ostream &cout, const HistoryMovingObject &hmo);
//istream& operator >> (istream &cin, const MovingObject &mo);


typedef map<HistoryMovingObjectKey,HistoryMovingObject,HistoryCompareKeys> 
          historymap;
class StAHistoryUnit
{
private:
	historymap theHistoryUnit;
public:
	// Funktion zum Eingeben
	void enter(int id, long starttime, long endtime, double x, double y, 
		       double nx, double ny);

	// Funktion zur Ausgabe des Speichers
	void dump(void);
};
} // end namespace ugridAlgebra
#endif
