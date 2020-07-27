/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]

*/
#ifndef ARMAHELPER_H
#define ARMAHELPER_H

#include "secondohelper.h"

using std::vector;

class ARMAHelper : public SecondoHelper
{
public:

    static void computeACF(vector<double> &, vector<vector<double>>&);
    static void computePACF(vector<double> &,
                            vector<vector<double>>&, vector<vector<double>>&);
    static double computeACVF(vector<double> &ts_data, int lag);
    static void getTSTime(vector<Tuple*> &ts_data, vector<double>& result);
    static double computeSigma(const vector<double>& ts_data);
private:
    ARMAHelper();

};

#endif // ARMAHELPER_H
