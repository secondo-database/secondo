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

#ifndef BPTREESEARCHENUMERATOR_H
#define BPTREESEARCHENUMERATOR_H

#include "Attribute.h"
#include "BPTree.h"
#include "BPTreeNode.h"

//
// Wird bei der Suche im BPTree durchlaufen,
// liefert alle gesuchten Werte
//
namespace fialgebra {

    // Muss hier bekannt gemacht werden wegen Ringabhaengigkeit
    // von BPTree.h und BPTreeSearchEnumerator.h
    class BPTree;

    class BPTreeSearchEnumerator {

    public:
        // ctor, dtor
        BPTreeSearchEnumerator( BPTree* tree,
                                BPTreeNode* startNode,
                                const Attribute* minValue,
                                const Attribute* maxValue );
        ~BPTreeSearchEnumerator();

        // Liefert das aktuelle Element in der Ergebnisliste.
        size_t GetCurrentId();

        const Attribute& GetCurrentValue();

        // Springt zum naechsten Element in der Ergebnisliste;
        // Liefert bei Erfolg true, am Ende der Liste false.
        bool MoveNext();

        // Tree muss public zugreiffbar sein, damit man ihn
        // im Operator Frange etc. wieder loeschen kann. Sonst
        // muesste man sich den Enumerator und den Baum merken.
        BPTree* GetTree();

    private:
        BPTree* _tree;
        // Aktuelle Position
        BPTreeNode* _currentNode;
        // Muss signed sein, weil wir -1 brauchen
        long        _currentIndex;

        // Intervall-Grenzen
        const Attribute* _minValue;
        const Attribute* _maxValue;

    }; // class BPTreeSearchEnumerator

} // namespace fialgebra

#endif // BPTREESEARCHENUMERATOR_H






























