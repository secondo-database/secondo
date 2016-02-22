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

Author: David Ullrich

*/

#include "BPTreeSearchEnumerator.h"

namespace fialgebra {

    // ctor
    BPTreeSearchEnumerator::BPTreeSearchEnumerator(
        BPTree* tree,
        BPTreeNode* startNode,
        const Attribute* minValue,
        const Attribute* maxValue ) :
       // Members initialisieren
       _tree( tree ),
       _currentNode( startNode ),
       _currentIndex( -1 ),
       _minValue( minValue ),
       _maxValue( maxValue ) { }
    BPTreeSearchEnumerator::~BPTreeSearchEnumerator() {
      if ( _currentNode ) {
        delete _currentNode;
        _currentNode = NULL;
      } // if
    }

    BPTree* BPTreeSearchEnumerator::GetTree() {
        return _tree;
    }

    // Liefert das aktuelle Element in der Ergebnisliste.
    size_t BPTreeSearchEnumerator::GetCurrentId() {
        size_t id = _currentNode->GetIDAt( _currentIndex );
        return id;
    }

    const Attribute& BPTreeSearchEnumerator::GetCurrentValue() {
        Attribute& value = _currentNode->GetValueAt( _currentIndex );
        return value;
    }

    // Springt zum naechsten Element in der Ergebnisliste;
    // Liefert bei Erfolg true, am Ende der Liste false.
    bool BPTreeSearchEnumerator::MoveNext() {
        while ( true ) {

            _currentIndex++;

            // Wenn wir am Ende eines Blatts ankommen, muessen wir
            // in das naechste Blatt springen.
            if ( (unsigned long)_currentIndex >=
                 _currentNode->GetValueCount() ) {
                // Wenn es kein NextLeaf mehr gibt, sind wir fertig.
                size_t nextLeaf = _currentNode->GetNextLeaf();
                if ( nextLeaf == 0 ) break;

                // cleanup
                delete _currentNode;

                _currentIndex = -1;
                _currentNode = _tree->GetNodefromPageNumber( nextLeaf );
                continue;
            } // if

            Attribute& curValue = _currentNode->GetValueAt( _currentIndex );
            // -1 : curValue < minValue
            //  0 : curValue = minValue
            //  1 : curValue > minValue
            // Wenn kein minValue angegeben ist, sind alle Werte groesser.
            int compMin = 1;
            if ( _minValue != NULL ) compMin = curValue.Compare( _minValue );

            // -1 : curValue < maxValue
            //  0 : curValue = maxValue
            //  1 : curValue > maxValue
            // Wenn kein maxValue angegeben ist, sind alle Werte kleiner.
            int compMax = -1;
            if ( _maxValue != NULL ) compMax = curValue.Compare( _maxValue );

            // Wenn der aktuelle Key groesser als der max. Key ist,
            // sind wir fertig.
            if ( compMax > 0 ) break;

            // Wenn der aktuelle Key im min/max-Intervall liegt,
            // haben wir einen Treffer.
            if ( compMin >= 0 && compMax <= 0 ) return true;

            // Wenn wir bis hier hin kommen, ist der aktuelle Value
            // kleiner als der minValue. Wir laufen so lange weiter,
            // bis wir ins [min,max]-Intervall kommen.

        } // while

        return false;
    }

} // namespace fialgebra






































