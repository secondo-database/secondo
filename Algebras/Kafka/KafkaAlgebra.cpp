/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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

*/

//
// Created by gstancul on 14.06.19.
//

#include "KafkaConsumer.h"


extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

namespace kafka {

    class KafkaAlgebra : public Algebra {
    public:
        KafkaAlgebra() : Algebra() {

            AddOperator(&kafkaConsumerOp);

        }
    };

} // End namespace

extern "C"
Algebra *
InitializeKafkaAlgebra(NestedList *nlRef,
                       QueryProcessor *qpRef) {
    return new kafka::KafkaAlgebra;
}

