/*
----
This file is part of SECONDO.

Copyright (C) 2020,
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
#include "secondohelper.h"

using std::vector;
using std::string;
using std::to_string;

SecondoHelper::SecondoHelper()
{

}



 // @brief getTSData extracts the data contained in a timeseries and
 // returns it as a vector of doubles.
 // @param ts_data Tuples of the time series
 // @return time series data
void SecondoHelper::getTSData(vector<Tuple *> &ts_data, vector<double>& data)
{
    for(size_t i = 0; i < ts_data.size(); ++i)
    {
        data[i] = (((CcReal*)ts_data[i]->GetAttribute(1))->GetRealval());
    }
}


// Helper function for debugging purposes;
void SecondoHelper::printMatrix(const vector<vector<double> > &matrix,
                                const string description)
{
    cout << description << endl;
    for(size_t row = 0; row < matrix.size(); ++row)
    {
        cout << "Row : " << to_string(row);
        for(size_t column = 0; column < matrix[row].size(); ++column)
        {
            cout << "column: " << to_string(matrix[row][column]) << "|";
        }
        cout << endl;
    }

}


 // Helper function to print vector and name/ text for vector;
void SecondoHelper::printVector(const vector<double> &vector, const string name)
{
   cout <<endl;
   cout << name << endl;
   for(size_t i = 0; i < vector.size(); i++)
   {
       cout << "Value " << to_string(i+1) <<": "
            << to_string(vector[i]) << " | ";
   }
   cout <<endl;
}

 // @brief computeMean computes the mean value of the given vector
 // @param ts_data data points
 // @return mean value

double SecondoHelper::computeMean(const vector<double>& ts_data)
{
    double mean = 0;
    for(auto value : ts_data)
    {
        mean += value;
    }

    return mean / ts_data.size();
}
