
/*
---- 
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Department of Computer Science, 
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
#include <iostream>
#include <stack>
#include <cmath>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RectangleAlgebra.h"
#include "RTreeAlgebra.h"
#include "DBArray.h"
#include "CPUTimeMeasurer.h"
#include "TupleIdentifier.h"
#include "Messages.h"
#include "Progress.h"
#include "UGridAlgebra.h"
#include "Symbols.h"        
#include "UGridUtilities.h"    

using namespace ugridAlgebra;

/*
  some auxiliary functions for processing moving objects  
  Methoden 
  zum Berechnen der Distanz, die das MovingObject zwischen zwei 
      UpdateUnits zurueckgelegt hat,
  zum Berechnen des Zeitintervalls zwischen zwei UpdateUnits, 
  zum Berechnen der Geschwindigkeit, in der die Distanz zurueckgelegt wurde 
  zum Berechnen der Richtung, in der sich das MovingObject bewegen wird und
  zum Berechnen der naechsten Position des MovingObjects = die Position, an 
      der das MovingObject in 1 Minute sein wird,wenn es sich mit der 
      berechneten Geschwindigkeit in die berechnete Richtung fortbewegt.

*/

/*
Einige Variablen

*/ 
  double weg, prePos;
  double speed, direction;
  double a = 1;
  double b = asin(a);
  double pi = b * 2;              // Berechnung der Kreiszahl PI 
                                  // aus Sinus 90 Grad = PI/2
  //double PI = asin(a) * 2;      // Kreiszahl PI = 3.146264371 
                                  // in SpatialAlgebra.h definiert
  double m;
  StoreMovingObject ut_Store;
  StACurrentUnit ut_CurrentUnit;
  StAHistoryUnit ut_HistoryUnit;

	/*
	 Methode zum Berechnen der zurueckgelegten Distanz

	*/
    double UGridUtilities::Distance (double x, double y, double nx, double ny)
    {
	  double dx = abs(nx - x);   // nx = X-Koordinate der Endposition, 
	                             //x = X-Koordinate der Startposition
	  double dy = abs(ny - y);   // ny = Y-Koordinate der Endposition, 
	                             //y = Y-Koordinate der Startposition
	  weg = sqrt(pow(dx,2) + pow(dy,2));  
	  // Berechnung des Weges nach Pythagoras - bis ca. 20 km genau:
	  // Wurzel aus der Summe der 2er-Potenz von dx und 
	  // der 2er-Potenz von dy 
	  return weg;
    }
	
	/*
	 Methode zum Berechnen der Geschwindigkeit

	*/
	double UGridUtilities::Speed (double s, long t)
    {
	  speed = s/t;  // Geschwindigkeit = Weg / Zeit
	  return speed;
    }
    

    /* 
	 Methode zum Berechnen der Bewegungsrichtung

	*/
	double UGridUtilities::Direction (double x, double y, 
		                              double nx, double ny)
    {
      if (nx == x)                      // Bewegung parallel zur y-Achse
	  {
         if (ny > y)                    // Bewegung senkrecht nach oben
			 return 90.0;
		 return 270.0;                  // Bewegung senkrecht nach unten
	  }
	  if (ny == y)                      // Bewegung parallel zur x-Achse
	  {
         if (nx > x)                    // Bewegung nach rechts
			 return 0.0;
		 return 180.0;                  // Bewegung nach links
	  }
	  if (ny > y)                       // Bewegung nach oben
	  {
	     m = (ny - y) / (nx - x);          
		 // m = Steigung, (x,y) = Startposition,(nx,ny) = Endeposition
	     direction = atan(m) * 180 / PI;   
		 // Richtung = arctan der Steigung mal 180 * PI
	  }
	  if (ny < y)                       // Bewegung nach unten
	  {
	     m = (ny - y) / (nx - x);          
		 // m = Steigung, (x,y) = Startposition,(nx,ny) = Endepositio
	     direction = (atan(m) * 180 / PI) + 180.0;   
		 // Richtung = arctan der Steigung mal 180 * PI
	  }
	  return direction;
    }

	/*
	 Methode zum Berechnen der Next-Position

	*/
	void UGridUtilities::predictPosition (double &nx, double &ny, 
		                                  double x, double y)
	{
	  nx = x + (60 * speed * cos(direction / 180 * PI )); 
	  // berechnete x-Koord in einer Minute
	  ny = y + (60 * speed * sin(direction / 180 * PI )); 
	  // berechnete y-Koord in einer Minute 
    }

    /*
	  Methode zum Berechnen der ersten Next-Position
	  Aufruf beim ersten Update; 
	  zu diesem Zeitpunkt noch keine CurrentUnit zu dieser ID 

	*/

    void UGridUtilities::firstPredictPosition (double &nx, double &ny, 
		                                       double x, double y)
	{
      // Berechnung erfolgt zum ersten Mal fuer diese ID: Als speed wird 
	  // 16.66 angenommen, das entspricht einer Geschwindigkeit von 
	  // 60 Stundenkilometern, und es wird ein Richtungswinkel 
	  // von 45 angenommen, d.h. m = 30. 
      speed = 16.66;                         // angenommene Geschwindigkeit 
	                                         // = 60 km / h
	  m = y/x;                               // Steigungswinkel 
	  a = atan(m);                           // Steigungswinkel in Grad
	  nx = x + (60 * speed * cos(a));        // berechnete x-Koordinate
	                                         // in einer Minute
	  ny = y + (60 * speed * sin(a));        // berechnete y-Koordinate 
	                                         // in einer Minute 
	  MobPos npos;
	  npos.x = nx;
	  npos.y = ny;
	}

/*
  Processing moving objects    

*/
MovingObjectKey::MovingObjectKey(int _id, string _licence)
{
 id = _id;
 licence = _licence;
}

ostream& operator << (ostream &cout, const MovingObjectKey &mok)
{
 return cout << " " << mok.id << "  " << mok.licence << "  ";
}

bool CompareKeys::operator ()(const MovingObjectKey mok1, 
	 const MovingObjectKey mok2) const
{
 return mok1.id < mok2.id;
}

MovingObject::MovingObject(string _description)
{
 description = _description;
}

ostream& operator << (ostream &cout, const MovingObject &mo)
{
 return cout << mo.description << ';';
}


void StoreMovingObject::enter(int id, std::string licence, 
				std::string description)
{
 theStore[MovingObjectKey(id, licence)]=MovingObject(description);
}

void StoreMovingObject::dump(void)
{
	cout << endl;
	cout << "myStore : " << endl 
		<< "   #  "<<"id" << "  \t" << "licence" << "\t" 
		<< "description" << endl;
cout << "-----------------------------------------------------------------"
	<< endl;
	cout << endl;
	int cnt = 0;
	for ( mobmap::iterator ii=theStore.begin();
		  ii !=theStore.end();
		  ++ii)
	{
		++cnt;
		cout << cnt << "  :  " << (*ii).first.id << "     \t" 
			 << (*ii).first.licence << "\t" 
			 << (*ii).second.description << ";" << endl;
	}
	cout << endl;
}

bool StoreMovingObject::read(int f_id, std::string f_licence, 
			std::string description)
{
	for ( mobmap::iterator ii=theStore.begin();
		  ii !=theStore.end();
		  ++ii)
	{
		if ((*ii).first.id == f_id
		   && (*ii).first.licence == f_licence) 
	    {
		   return true;   // identische UpdateUnit ist schon bearbeitet.
		}
	}
	return false;
}

/*
  Processing of current Units     

*/
//extern UGridUtilities myUtilities;
CurrentMovingObjectKey::CurrentMovingObjectKey(int _cid)
{
 cid = _cid;
 //timestamp = _timestamp;
}

ostream& operator << (ostream &cout, const CurrentMovingObjectKey &cmok)
{
 return cout  << " " << cmok.cid <<  "  ";
}

bool CurrentCompareKeys::operator ()(const CurrentMovingObjectKey cmok1, 
		const CurrentMovingObjectKey cmok2) const
{
 return cmok1.cid < cmok2.cid;
}
CurrentMovingObject::CurrentMovingObject(long _stimestamp, long _etimestamp, 
		 double _x, double _y, double _nx, double _ny)
{
 stimestamp = _stimestamp;
 etimestamp = _etimestamp;
 x = _x;
 y = _y;
 nx = _nx;
 ny = _ny;
}

ostream& operator << (ostream &cout, const CurrentMovingObject &cmo)
{
 return cout << cmo.stimestamp << "  " << cmo.etimestamp << "  " << cmo.x 
	         << ';' << cmo.y << "  " << cmo.nx << ';' << cmo.ny << '>';
}

void StACurrentUnit::enter(int cid, long stimestamp, long etimestamp, 
			double x, double y, double nx, double ny)
{
  theCurrentUnit[CurrentMovingObjectKey(cid)] = 
	             CurrentMovingObject(stimestamp,etimestamp,x,y,nx,ny);
}

/*
  Funktion zur Ausgabe des Speichers

*/
void StACurrentUnit::dump(void)
{
  cout << endl;
  cout << "CurrentMap: " << endl
       << "   #   "<< "id" << "  \t" << "stimestamp" << "\t" 
	   << "x;y-coord" << "\t" <<  "nx;ny-coord" << endl;
  cout << "-------------------------------------------------------------------"
	   << endl;
  cout << endl;
  int count = 0;
  for ( currentmap::iterator ii = theCurrentUnit.begin();
   	    ii !=theCurrentUnit.end();
		++ii)
	{
      ++count;
	  cout << count << "  :  " << (*ii).first.cid << "\t" 
		   << (*ii).second.stimestamp
		   << "\t" << (*ii).second.x << ";" << (*ii).second.y << "     "
		   << "\t" << (*ii).second.nx << ";" << (*ii).second.ny<< endl;
	}
  cout << endl; 
}

/*
  search entry

*/
bool StACurrentUnit::searchEntry(int c_id)
 {
	 for ( currentmap::iterator ii=theCurrentUnit.begin();
		  ii !=theCurrentUnit.end();
		  ++ii)
	{
	  if ((*ii).first.cid == c_id)
	  {
         return (true);
	  }
	}
    return (false);
}

/*
  read entry

*/
void StACurrentUnit::readEntry (int id, long &cutimestamp, 
		double &cux, double &cuy)
{
  for ( currentmap::iterator ii=theCurrentUnit.begin();
	    ii !=theCurrentUnit.end();  ++ii)
  {
    if ((*ii).first.cid == id)
	  {
         cutimestamp = (*ii).second.stimestamp;
		 cux = (*ii).second.x;
		 cuy = (*ii).second.y;
	  }
  }
  ;
}

/*
  Funktion zum Lesen der Id

*/
int StACurrentUnit::getId(int c_id)
 {
	 for ( currentmap::iterator ii=theCurrentUnit.begin();
		  ii !=theCurrentUnit.end();
		  ++ii)
	{
	  if ((*ii).first.cid == c_id)
	  {
         return (*ii).second.stimestamp;
	  }
	  
	}
	 return 0;
}

/*
  Funktion zum Lesen des MovingObjects

*/
void StACurrentUnit::getMovingObject(int c_id, double &nx, double &ny)
 {
	for ( currentmap::iterator ii=theCurrentUnit.begin();
		  ii !=theCurrentUnit.end();
		  ++ii)
	{
	  if ((*ii).first.cid == c_id)
	  {
         nx = (*ii).second.x;
		 ny = (*ii).second.y;
	  }
	}
	;
}
void StACurrentUnit::selectAllId(long qu_time, 
		double qu_x1, double qu_x2,double qu_y1, double qu_y2)
{
   UGridUtilities myUtilities;
   int count = 0;
   double dist60;               // Weg des MovingObjects in einer Minute
   double pos_xmin, pos_xmax;   // erreichbare x-Koordinaten 
   double pos_ymin, pos_ymax;   // erreichbare y-Koordinaten
   double est_xmin, est_xmax;   // derzeit moegliche x-Koordinaten 
   double est_ymin, est_ymax;   // derzeit moegliche y-Koordinaten
   long dist_time;              // vergangene Zeit seit letztem Update
   
   long startCalc = time(NULL);  // Startzeitpunkt der Berechnungen
   for ( currentmap::iterator ii=theCurrentUnit.begin();
		 ii !=theCurrentUnit.end();
		 ++ii)  
   {
      ++count;     // Zaehler fuer die Anzahl der bearbeiteten CurrentUnits  
	  
	  // Berechnen des zurueckgelegten Weges innerhalb von 60 Sekunden
		dist60 = myUtilities.Distance((*ii).second.x, (*ii).second.y,
			       (*ii).second.nx,(*ii).second.ny);
	  
	  //Berechnen der vergangenen Zeit seit dem letzten Update
	  dist_time = startCalc - (*ii).second.stimestamp;
	  
	  // Berechnen aller möglichen Positionen, 
	  // in denen jetzt das MovingObject sein kann
	  // (est_xmin;est_ymin) -- (est_xmax;est_ymax)
	  // x + dist_time * dist60/60 * cos(y/x *180/PI) und
	  // cos(y/x * 180/PI) in [-1 , +1]
	  // y + dist_time * dist60/60 * sin(y/x *180/PI) und
	  // sin(y/x * 180/PI) in [-1 , +1]
	  est_xmin = (*ii).second.x - dist_time * dist60 / 60;  
	  est_xmax = (*ii).second.x + dist_time * dist60 / 60;  
	  est_ymin = (*ii).second.y + dist_time * dist60 / 60;  
	  est_ymax = (*ii).second.y - dist_time * dist60 / 60;  
	  
	  // Berechnen der noch verbleibenden Zeit
	  long restTime = qu_time - startCalc;
	  
	  // Berechnen der möglichen Positionen, die das
	  // Moving Object demnach zur referenzierten Zeit einnehmen kann 
      pos_xmin = est_xmin - restTime * dist60 / 60;
	  pos_xmax = est_xmax + restTime * dist60 / 60;
	  pos_ymin = est_ymin + restTime * dist60 / 60;
	  pos_ymax = est_ymax - restTime * dist60 / 60;

	  // Das MovingObject kann sich demnach zur geforderten Zeit an den
	  // Positionen pos_xmin;pos_ymin) -- (pos_xmax;pos_ymax) befinden
	  if (pos_xmin >= qu_x1 && pos_ymin >= qu_y1 && 
		  pos_xmax <= qu_x2 && pos_ymax <= qu_y2)
	  {
	     cout << "??? " << endl; //count << " : " << (*ii).first << endl;
	  }
	  else 
	  {
         if (pos_xmax <= qu_x1 && pos_ymax <= qu_y1)
		 {
		   cout << "ßßß" << endl; //<< count << " : " << (*ii).first 
		                          //<< "nicht moeglich. " << endl; 
		                          //Nothing: sicher nicht moeglich
		 }
		 else
		 {
		  cout <<  pos_xmin << ";" << pos_ymin << " bis " 
			  << pos_xmax  << ";" << pos_ymax;
		  cout << endl;
		 }
	  }
   }
   // long endCalc = time(NULL);   // Endzeitpunkt der Berechnungen
   // long factTime = endCalc - startCalc;
}

void StACurrentUnit::selectFirstId(double qu_x1, double qu_x2, 
			double qu_y1, double qu_y2)
{
   UGridUtilities myUtilities;
   int count = 0;
   double dist60;             // Weg des MovingObjects in einer Minute
   double pos_xmin, pos_xmax; // erreichbare x-Koordinaten 
   double pos_ymin, pos_ymax; // erreichbare y-Koordinaten
   double est_xmin, est_xmax; // derzeit moegliche x-Koordinaten 
   double est_ymin, est_ymax; // derzeit moegliche y-Koordinaten
   long dist_time;            // vergangene Zeit seit letztem Update
   long startCalc = time(NULL);  // Startzeitpunkt der Berechnungen
   for ( currentmap::iterator ii=theCurrentUnit.begin();
		 ii !=theCurrentUnit.end();
		 ++ii)  
   {
      ++count;
		dist60 = myUtilities.Distance((*ii).second.x, (*ii).second.y,
			          (*ii).second.nx,(*ii).second.ny);
	  dist_time = (*ii).second.stimestamp + 20;
	  est_xmin = (*ii).second.x - dist_time * dist60 / 60;
	  est_xmax = (*ii).second.x + dist_time * dist60 / 60;
	  est_ymin = (*ii).second.y - dist_time * dist60 / 60;
	  est_ymax = (*ii).second.y + dist_time * dist60 / 60;
      pos_xmin = est_xmin -  dist60 / 60;
	  pos_xmax = est_xmax +  dist60 / 60;
	  pos_ymin = est_ymin -  dist60 / 60;
	  pos_ymax = est_ymax +  dist60 / 60;
	  if (pos_xmin >= qu_x1 && pos_xmax <= qu_x2 && 
		  pos_ymin >= qu_y1 && pos_ymax <= qu_y2)
	  {
	     cout << " ok for count " << endl; 
	  }
	  else 
	  {
         if (pos_xmin >= qu_x2 && pos_xmax <= qu_x1 && 
			 pos_ymin >= qu_y2 && pos_ymax <= qu_y1)
		 {
			 ;  //Nothing: sicher nicht moeglich
		 }
		 else
		 {
			 cout << "  :  maybe : " << endl; 
			 cout << "moegliche Positionen : " << pos_xmin << ";" 
			<< pos_ymin << "   " << pos_xmax  << ";" << pos_ymax;
			 cout << endl;
		 }
	  }
   } // End for iterator ii
   long endCalc = time(NULL);   // Endzeitpunkt der Berechnungen
   long factTime = endCalc - startCalc;
   cout << endl << "Zeit fuer die Bearbeitung von  " << count 
	    << " CurrentUnits = " << factTime << " Sekunden." << endl;
}

/*
  Processing of history Units     

*/
HistoryMovingObjectKey::HistoryMovingObjectKey(int _id, long _starttime)
{
 id = _id;
 starttime = _starttime;
}

ostream& operator << (ostream &cout, const HistoryMovingObjectKey &hmok)
{
 return cout << " " << hmok.id << "  " << hmok.starttime << "  ";
}

bool HistoryCompareKeys::operator ()(const HistoryMovingObjectKey hmok1, 
		const HistoryMovingObjectKey hmok2) const
{
 return hmok1.id < hmok2.id || (hmok1.id==hmok2.id && 
	    hmok1.starttime < hmok2.starttime);
}
HistoryMovingObject::HistoryMovingObject(long _endtime, double _x, 
			double _y, double _nx, double _ny)
{
 endtime = _endtime;
 x = _x;
 y = _y;
 nx = _nx;
 ny = _ny;
}

ostream& operator << (ostream &cout, const HistoryMovingObject &hmo)
{
 return cout << hmo.endtime << "  " << hmo.x << ';' << hmo.y << "  " 
	         << hmo.nx << ';' << hmo.ny;
}

/*
  HistoryUnit.2   Auxiliary Functions                   

*/
void StAHistoryUnit::enter(int id, long starttime, long endtime, double x, 
			double y, double nx, double ny)
{
  theHistoryUnit[HistoryMovingObjectKey(id, starttime)] = 
	  HistoryMovingObject(endtime,x,y,nx,ny);
}

/*
 Funktion zur Ausgabe des Speichers

*/
void StAHistoryUnit::dump(void)
 {
	 cout << endl;
	 cout << "HistoryMap: " << endl
		 << "  #  " << "id" <<"\t" << "starttime" << "    " 
		 << "endtime" << "   \t   " 
		  << "x;y-coord " <<"\t" << "nx;ny-coord" << endl;
    cout << "-------------------------------------------------"
	     << "-----------------------------" << endl;
	 cout << endl;
	 int hcount = 0;
	 for ( historymap::iterator ii=theHistoryUnit.begin();
		  ii !=theHistoryUnit.end();
		  ++ii)
	{
		++hcount;
		cout << hcount << "  :  " << (*ii).first.id << "   \t" 
		<< (*ii).first.starttime << "   " << (*ii).second.endtime 
		<< "\t   " << (*ii).second.x << ";" << (*ii).second.y << "\t"
		<< (*ii).second.nx << ";" << (*ii).second.ny << "\t" << endl;
	}
    cout << endl;
}
