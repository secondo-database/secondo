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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Image Similarity Algebra

March 2017 Michael Loris

The Image Similarity Algebra for SECONDO provides operations to store 
pictures in JPEG format into m-trees. It consists a ~Signature~ object 
representing a the signature of a single JPEG image. The object is a 
subtype of the Attribute class. The Image Similarity Algebra consists of 
multiple files, including external files for k-means clustering.

*/


#ifndef SQFD_H_
#define SQFD_H_


#include "JPEGImage.h"
#include <math.h>
#include <vector>
#include "ImageSimilarityAlgebra.h"





class SQFDCalculator
{
public:
	double calcSQFD(std::vector<FeatureSignatureTuple> fst1,
                    std::vector<FeatureSignatureTuple> fst2);
};


#endif
