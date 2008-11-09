/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] 

April 2008, initial version created by M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

*/

#include "Segment.h"

namespace mregionops {

ostream& operator <<(ostream& o, const Segment2D& s) {

    o << s.GetStart() << " -> " << s.GetEnd();

    return o;
}

ostream& operator <<(ostream& o, const Segment3D& s) {

    o << s.GetStart() << " -> " << s.GetEnd();

    return o;
}

} // end of namespace mregionops
