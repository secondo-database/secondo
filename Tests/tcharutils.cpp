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

April 2006, M. Spiekermann. A small test for string manipulating functions

*/



#include <iostream>
#include <string>

#include "CharTransform.h"

using namespace std; 
 
void printWrapped(const int n, const string& s)
{
  cout << string(n,'-') << endl;
  cout << s << endl;
  cout << string(n,'-') << endl;
  
} 


int
main () {

 string tab1 = "Hier sind ein paar Tabulatoren: -\t- -\t- -\t-";
 cout << "\"" << tab1 << "\"" << endl;
 cout << "Das Ergebnis von expandTab(str,5) sieht dann so aus:" << endl;
 cout << "\"" << expandTabs(tab1,5) << "\"" << endl;
 
 
 string z = "Hallo, dies ist ein kurzer Test-Text, der nach spätestens "
            "40 Zeichen "
            "umgebrochen werden soll. Mal sehen, ob das funktioniert.";

 string y = wordWrap(40,z);
 printWrapped(40,y);

 z = "Man kann dies auch mit einem hängenden Einzug kombinieren! "
     "Dann wird ab der zweiten Spalte eingerückt.";

 y = wordWrap("Hinweis: ",40,z);
 printWrapped(40,y);
            

 z = "Trennzeichen sind das Leerzeichen oder ,.:!?;)]-+*><=\t "
     "Nach diesen Zeichen wird ein Umbruch durchgeführt sofern die Position "
     "weniger als 2/3 der Zeilenlänge entspricht. Falls "
     "ein white space { ,\\n} nach der Umbruchstelle folgt, so wird dies "
     "verschluckt, also nicht am Anfang der nächsten Zeile ausgegeben. "
     "Tabulatoren "
     "werden vor der Verarbeitung durch 4 Leerzeichen ersetzt. Wird keine "
     "passende "
     "Umbruchstelle gefunden, so wird die Zeile nach 4/5 der Länge "
     "umgebrochen. Die "
     "letzte Zeile muß jedoch mindestens 1/3 der Zeilenlänge besitzen! "
     "xx xx xx xx xx xx";
 
 y = wordWrap(40,z);
 printWrapped(40,y);

 z = "Mehrere ... wie z.B. in ((stream (tuple ((x1 t1) ... (xn tn)))) "
     "(stream (tuple ((y1 d1) ... (ym dm)))) xi yj) -> "
     "(stream (tuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)))) werden nicht "
     "getrennt, da nach einem Umbruchzeichen nicht direkt ein weiteres "
     "folgen darf. ";
 
 y = wordWrap(27,z);
 printWrapped(27,y);


 z = "Eine Umbruchstelle kann durch das Zeichen \\n definiert werden. "
     "Dann erfolgt ein \n"
     "erzwungener Umbruch!";
 y = wordWrap(70,z);
 printWrapped(70,y);

 z = "Man kann dies auch mit einem \\n\n"
     "hängenden Einzug kombinieren! "
     "Dann wird ab der zweiten \\n\n"
     "Spalte eingerückt.";

 y = wordWrap("Hinweis: ",20,z);
 printWrapped(20,y);
            

 
}
