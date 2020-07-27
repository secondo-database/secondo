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
#include "armahelper.h"


 // @brief ARMAHelper::ARMAHelper Utility-Class that provides
 //functions utilized in the forecasting
 // of stationary univariate time series.
ARMAHelper::ARMAHelper()
{

}




 // @brief computeACF computes the sample autocorrelation function (SACF)
 //of the given data vector.
 // @param ts_data
void ARMAHelper::computeACF(vector<double> &ts_data,
                            vector<vector<double>>& acf)
{
    int ts_size = ts_data.size();

    double gamma_hat_0 = computeACVF(ts_data, 0);

    for(int row = 0; row < ts_size; row++)
    {
        for(int column = 0; column < ts_size; column++)
        {
            acf[row][column] =  computeACVF(ts_data, abs(row - column))
                    /gamma_hat_0;
        }
    }

}




 // @brief ARMAHelper::computePACF computes the sample partial autocorrelation
 //function (PACF) using the
 // Durbin-Levinson algorithm
 // from the given data for all lags.
void ARMAHelper::computePACF(vector<double>& ts_data,
                             vector<vector<double>>& pacf,
                             vector<vector<double>>& acf){
     vector<vector<long double>> phi(ts_data.size(),
                                     vector<long double>(ts_data.size(), 0));
     phi[0][0] = 1;

     vector<double> rho = acf[ts_data.size() -1];
     phi[1][1] = rho[1];

     for(size_t tau = 1; tau < rho.size(); ++tau)
     {
         long double sum_dividend = 0;
         long double sum_divisor = 0;
         for(size_t i = 1; i <= tau -1; ++i)
         {
             sum_dividend += phi[tau-1][i]*rho[tau-i];
             sum_divisor += phi[tau-1][i]*rho[i];
         }
         phi[tau][tau] = (rho[tau] - sum_dividend) / (1 - sum_divisor);

         for(size_t i = 0; tau == 2 ? i <= tau -1 : i < tau -1; ++i)
         {
             phi[tau][i] = phi[tau-1][i] - (phi[tau][tau] * phi[tau-1][tau-1]);
         }
     }

     for(size_t i = 0; i < phi.size(); ++i)
     {
         for(size_t j = 0; j < phi[i].size(); j++)
         {
             pacf[i][j] = (double) phi[i][j];
         }
     }

     printMatrix(pacf, "PACF-Matrix:");

}


 // @brief computeACVF computes the sample autocovariance of the given
 //time series data at lag t
 // @param ts_data time series data
double ARMAHelper::computeACVF(vector<double>& ts_data, int lag)
{
    double mean = computeMean(ts_data);

    int ts_length = ts_data.size();
    double sum = 0;

    for( int i = 0; i < ts_length - lag; i++)
    {
        sum += (ts_data[i + lag] - mean) * (ts_data[i] - mean);
    }


    return sum / ts_length;
}


 // @brief ARMAHelper::getTSTime extracts a vector with
 //string representations of the
 // time index of the time series
void ARMAHelper::getTSTime(vector<Tuple *>& ts_data, vector<double>& timeindex )
{
    for(size_t i = 0; i < ts_data.size(); ++i)
    {
        Instant* instant = (Instant*) ts_data[i]->GetAttribute(0);
        timeindex[i] = instant->ToDouble();
    }
}


 // Computes the standard deviation of the timeseries data.

double ARMAHelper::computeSigma(const vector<double> &ts_data)
{
    double mu = computeMean(ts_data);

    int n = ts_data.size();
    double sum = 0.0;

    for(int i = 0; i < n; ++i)
    {
        sum += pow(ts_data[i] -mu, 2);
    }

    return sqrt(sum/n);
}
