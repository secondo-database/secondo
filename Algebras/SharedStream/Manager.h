
/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[->] [$\rightarrow$]
 //[TOC] [\tableofcontents]
 //[_] [\_]

*/
#ifndef MANAGER_H
#define MANAGER_H

#endif //MANAGER_H

namespace sharedstream {
    class Manager{};

//Funktion (Operator) zum "manuellen" Eintrag einer StreamSource

//Funktion zum Empfang von Anfragen der Webschnittstelle

//Funktion zum Starten eines StreamProcessors
// und Übermittlung der Stromquelle

/*Funktion von Thomas
 * TODO: Includes rausbekommen, so kompiliert es nicht,
 * Distributed2 zu includen reicht nicht, die includes werden
 * anscheinend nicht mitgenommen
ConnectionInfo *con = ConnectionInfo::createConnection
 ("localhost", 1234, "SecondoConfig.ini");

if(!con){
    cerr << "Cold not connect to Secondo" << endl; //Fehlerbehandlung
    return;
}

if(!con->switchDatabase("newdatabase", true, false)){
    cerr << "switching to database failed" << endl;
    return;
}

string cmd = "let myObject = 10";
int errorCode;
string errorMsg;
string resultList;
double runtime;
Commandlog log;

con->
simpleCommand(cmd, errorCode, errorMsg, resultList,
false, runtime, false, false, log);
if(errorCode==0){
    cout << "command " << cmd << " successful" << endl;
} else {
    cout << "command " << cmd << " failed " << endl << errorMsg << endl;
}*/

//Funktion zum Übermitteln neuer Konstanten an einen StreamProcessor


//Struktur zum Speichern der Stromquelleneinträge
// (ip, port, Tupleart inkl. Attributen)

}
