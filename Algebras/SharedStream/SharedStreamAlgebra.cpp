
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
#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Stream.h"             // wrapper for secondo streams





#include "LogMsg.h"             // send error messages

#include "../../Tools/Flob/DbArray.h"  // use of DbArrays


#include "RelationAlgebra.h"           // use of tuples
//gibt aber einen Bug wg. Command-Logger
//#include "Distributed2Algebra.h"  //use of ConnectionInfo
#include "ErrorWriter.h"
#include "DArray.h"

#include <math.h>               // required for some operators
#include <stack>
#include <limits>

#include "SocketIO.h"

/*
 0.5 Global Variables

 Secondo uses some variables designed as singleton pattern. For accessing these
 global variables, these variables have to be declared to be extern:

*/

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

/*
 0.6 Namespace

 Each algebra file defines a lot of functions. Thus, name conflicts may arise
 with function names defined in other algebra modules during compiling/linking
 the system. To avoid these conflicts, the algebra implementation should be
 embedded into a namespace.

*/

namespace sharedstream {

/*Mit dieser Klasse kann sich eine Secondoinstanz zu einer Quelle fuer einen
 * unendlichen Strom erklaeren, so dass andere Secondoinstanzen als StreamProcessor
 * an den Strom anschliessen und mitlauschen koennen*/
    class StreamSource {
    public:

        /*Konstruktor*/

        StreamSource(int _port) {
            vector<ProcessorEntry> processors;

            //zufallsZahl = rand() % 4 + 1;
            //FakedStream(zufallsZahl)
        }

        /*Destruktor*/
        ~StreamSource() {
            // delete processors;
        }

        void addProcessor(string ip, string port) {

            ProcessorEntry *entry = new ProcessorEntry(ip, port);
            processors.push_back(*entry);
        }

        void deleteProcessor(string ip, string port) {
            //abgemeldeten StreamProcessor aus Liste loeschen.
        }

        void sendTuple(Tuple tuple) {
            for (unsigned int i = 0; i <= processors.size() - 1; ++i) {

                //Tupelversand. Nur wie?
                (processors[i].ipAdress, processors[i].port);
            }
        }

    private:
        struct ProcessorEntry {
            string ipAdress;
            string port;

            ProcessorEntry(string ipAdress, string port);
        };

        //TODO: Sockets in den Vektor?
        //vector, der die angemeldeten StreamProcessor mit IP und Port enthaelt.
        vector<ProcessorEntry> processors;

        //Objekt für den vorgetäuschten Strom (FakedStream)

        //TODO: Warum kennt er die verdammte Klasse nicht?
        //FakedStream* fakedStream;

    };//end streamsource

//darf nur einmal vorhanden sein
//TODO: Macht man das als Singleton?
    class Manager {

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

    private:
        //Struktur zum Speichern der Stromquelleneinträge
        // (ip, port, Tupleart inkl. Attributen)

    };//end Manager

    class StreamProcessor {

        //Funktion, um Kommunikation mit übermittelter Stromquelle zu starten

        //Funktion um B-Baum oder R-Baum über das Attribut anzulegen

        //Funktion um Tupel auszuwerten und ggf. Bäume upzudaten

        //Funktion um Relation upzudaten

        //Funktion um Mail mit Inhalt des Tupels bei Treffer zu versenden

    };//end StreamProcessor


    class WebCommunicator {

        //Schnittstelle für Webkommunikation
        //Funktion, um neue Anfragen an Manager zu übersenden
        /*
         * Eine Anfrage enthaelt folgende Daten:
         * - Namen und Anrede der Person
         * - E-Mail-Adresse der Person
         * - ausgewählter Strom
         * - ausgewähltes Attribut
         * - ausgewählter Vergleichsoperator
         * - bei räumlichen Datentypen ausgewählter Schnittmengentyp
         * (intersects region oder line)
         * - eingegebene Konstante
         */

    };//end WebCommunicator

    class FakedStream {
        //Klasse, die einen endlosen Strom als Auswahl aus 4
        // verschiedenen Tupelarten vorgaukelt
        //Beispiel-Strom mit Adressdaten (int, string, text) (Quelle suchen)
        //Beispiel-Strom mit Stadt-Daten (aus vorhandener DB?)
        //Beispiel-Strom mit Koordinaten (point) (Quelle suchen)
        //Beispiel-Strom mit line und region (aus vorhandener DB Berlin?)
    };//end FakedStream

/* auskommentierte Sachen wegen PDError
 * //#include "GenericTC.h"          // use of generic type constructors
 * /


}//end namespace

