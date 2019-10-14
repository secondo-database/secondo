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

1.1 Feed

Feeds all tuples of the given relation into an tuple stream, which can then be
further processed. The tuples' order in the stream is not specified.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_OPERATORS_FEED_H_
#define ALGEBRAS_NESTEDRELATION2_OPERATORS_FEED_H_

#include "Stream.h"

namespace nr2a {

class NRelIterator;

class Feed
{
  public:
    struct Info : OperatorInfo
    {
      Info();
    };

    virtual ~Feed();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int FeedArel(Word* args, Word& result, int message, Word& local,
        Supplier s);
    static int FeedNrel(Word* args, Word& result, int message, Word& local,
        Supplier s);

    static CreateCostEstimation costEstimators[];
    template <class T>
    static CostEstimation * GetCostEstimator();

  protected:

  private:
    Feed();
    // Declared, but not defined => Linker error on usage

    struct LocalInfoArel :
        public Nr2aLocalInfo<LinearProgressEstimator<LocalInfoArel> >
    {
      public:
        LocalInfoArel(ARelIterator * iter, void * predecessor,
            const ProgressInfo base);
        virtual ~LocalInfoArel();

        ARelIterator *iter;
    };

    struct LocalInfoNrel :
        public Nr2aLocalInfo<LinearProgressEstimator<LocalInfoNrel> >
    {
      public:
        LocalInfoNrel(NRelIterator * iter, void * predecessor,
            const ProgressInfo base);
        virtual ~LocalInfoNrel();

        NRelIterator *iter;
    };

};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_OPERATORS_FEED_H_*/
