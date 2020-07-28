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
#include "arima.h"

using std::to_string;
using std::max;


//  Constructor with all necessary data to predict a timeseries
//  using AR, MA, ARMA or ARIMA processes.

Arima::Arima(int p, int d, int q, int no_predictions,
             vector<Tuple*> timeseries_tuple_data):
    order_ma(q), order_ar(p), order_differencing(d), steps(no_predictions),
    timeseries_data(vector<double>(timeseries_tuple_data.size(),0)),
    fitted_data(vector<double>(timeseries_tuple_data.size()
                               + no_predictions, 0)),
    differenced_data(vector<double>(timeseries_tuple_data.size(), 0)),
    phi(timeseries_tuple_data.size(),
        vector<long double>(timeseries_tuple_data.size(), 0)),
    theta(timeseries_tuple_data.size(),
          vector<double>(timeseries_tuple_data.size(), 0)),
    pacf(timeseries_tuple_data.size(),
         vector<double>(timeseries_tuple_data.size(), 0)),
    acf(timeseries_tuple_data.size(),
        vector<double>(timeseries_tuple_data.size(), 0)),
    ny(timeseries_tuple_data.size(),0),
    acvf(timeseries_tuple_data.size(),0)
{
    ARMAHelper::getTSData(timeseries_tuple_data, timeseries_data);
    mean = ARMAHelper::computeMean(timeseries_data);
    ARMAHelper::computeACF(timeseries_data, acf);
    ARMAHelper::computePACF(timeseries_data, pacf, acf);

    for(size_t i = 0; i < timeseries_data.size(); ++i)
        acvf[i] = ARMAHelper::computeACVF(timeseries_data, i);
}


//Computes the coefficients for an AR process using the OLS method.
Eigen::VectorXd Arima::compute_ar_coeffs()
{
    int i,k;

  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> R(order_ar, order_ar);
  Eigen::Matrix<double, Eigen::Dynamic, 1> r(order_ar);

    for(i = 0; i < order_ar; ++i)
    {
        for(k= 0; k < order_ar; ++k)
            R(i,k) = acf[i][k];
    }

    for(i = 0; i < order_ar; ++i)
    {
        r(i) = acf[0][i+1];
    }

    return R.colPivHouseholderQr().solve(r);
}

void Arima::durbin_levinson_algorithm()
{
    int i,j;

    vector<long double> nu(order_ar, 0);
    phi[0][0] = 1;
    phi[1][1] = gamma_hat(1) / gamma_hat(0);
    nu[0] = gamma_hat(0);
    nu[1] = gamma_hat(0) - phi[1][1] * gamma_hat(1);

    for(i = 2; i < order_ar; ++i)
    {
        long double sum_dividend = 0.0;
        long double sum_divisor = 0.0;
        for(j = 0; j <= i-1; ++j)
        {
            sum_dividend += phi[i-1][j] * gamma_hat(i-j);
            sum_divisor += phi[i-1][j] * gamma_hat(j);
        }

        phi[i][i] = (gamma_hat(i) -sum_dividend) / (1 -sum_divisor);

        for(j = 1; i == 2 ? j <= i-1 : j < i-1; j++)
        {
            phi[i][j] = phi[i-1][j] - (phi[i][i] *phi[i-1][i-1]);
        }
    }
}

vector<double> Arima::ar()
{
    int i, k, nobs = timeseries_data.size();

    vector<double> prediction(nobs + steps, 0);
    vector<double> demeaned_data(nobs, 0);
    double mse = 0.0;


    //demean

  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> R(order_ar, order_ar);
  Eigen::Matrix<double, Eigen::Dynamic, 1> r(order_ar);

    for(i = 0; i < order_ar; ++i)
    {
        for(k= 0; k < order_ar; ++k)
            R(i,k) = acf[i][k];
    }

    for(i = 0; i < order_ar; ++i)
    {
        r(i) = acf[0][i+1];
    }

    Eigen::VectorXd phis = compute_ar_coeffs();
    durbin_levinson_algorithm();

    cout << "Rhos: ";
    for(i = 0; i < order_ar; ++i)
    {
          cout << to_string(phis.coeff(i)) <<"; ";
    }
    cout << "Durbin-Levinson estimated coefficients:";
    for(i = 0; i < order_ar; ++i)
    {
        cout << to_string(phi[order_ar-1][i]) <<"; ";
    }
    cout <<endl;

    for(i = 0; i < nobs; ++i)
    {
        demeaned_data[i] = timeseries_data[i] - mean;
    }

    for(i = 0; i < nobs; ++i)
        prediction[i] = demeaned_data[i];

    for( k = order_ar; k < nobs + steps; ++k)
    {
        double predicted_value = 0;
        for(i = 0 ; i < order_ar; ++i)
        {
            double coeff = phis.coeff(i);

            if(k < nobs)
            {
                predicted_value += coeff * demeaned_data[k -1 -i];
            }
            else
            {
                predicted_value += coeff * prediction[k -1 -i];
            }
        }
        prediction[k] = predicted_value;
    }

    for(i = 0; i < nobs + steps; ++i)
    {
        prediction[i] = prediction[i] + mean;

        if( i < nobs)
            mse += pow(prediction[i]-timeseries_data[i], 2);
    }

    cout << "MSE: " << to_string(mse/nobs) << endl;
    return prediction;
}



 // Computes the autocorrelation to the given lag.
double Arima::gamma_hat(int index)
{
    int t, n = timeseries_data.size();

    double sum = 0.0;

    for(t = 1; t < n - abs(index); ++t)
    {
        sum += (timeseries_data[t+abs(index)] - mean)+(timeseries_data[t]
                                                       - mean);
    }

    return sum / n;
}

 //Prints the ACF value to the given lag
void Arima::printACF(int lags)
{
    int size = acf.size();

    for(int i = 0; i < lags && i < size; ++i)
    {
        cout << "ACF at lag " << to_string(i +1) << ": "
             << to_string(acf[0][i]) << endl;
    }
}


 //Prints the PACF values to the lag

void Arima::printPACF(int lags)
{
    int size = pacf.size();

    for(int i = 0; i < lags && i < size; ++i)
    {
        cout << "PACF at lag " << to_string(i +1)
             << ": " << to_string(pacf[0][i]) << endl;
    }
}


 // Computes the forecast utilizing the complette ARIMA process if
 //the parameters are
 // submitted. If not it uses the MA, AR or ARMA process respectively.
vector<double> Arima::forecast()
{
    int i, j, n, m, nobs;

    m = max(order_ar, order_ma);

    innovations_algorithm();
    Eigen::VectorXd phis = compute_ar_coeffs();

    if(order_differencing != 0)
        differencing();

    double mean = ARMAHelper::computeMean(timeseries_data);
    nobs = timeseries_data.size();

    vector<double> prediction(timeseries_data.size() + steps, 0);
    vector<double>centered_data(nobs, 0);

    //Center data
    for(i = 0; i < nobs; ++i)
        centered_data[i] = timeseries_data[i] - mean;

    //Initialize the predictions for MA
    for(i = 0; i < order_ma; ++i)
    {
        prediction[i] = timeseries_data[i];
    }


    for(n = min(order_ar, order_ma); n < nobs + steps -1; ++n )
    {
        double predicted_value = 0.0;

        if( n < m )
        {
            for(j = 0; j < n; j++)
            {
                predicted_value += theta[order_ar][j] *
                        (centered_data[n+1-j] -prediction[n+1-j]);
            }
        }
        else
        {
            double prediction_ma = 0.0;
            double prediction_ar = 0.0;

            for(j = 0; j < order_ar; ++j)
            {
                prediction_ma += theta[order_ar][j] *
                        (centered_data[n+1-j] -prediction[n+1-j]);
            }
                for(i = 0 ; i < order_ar; ++i)
                {
                    double coeff = phis.coeff(i);
                    if(n < nobs)
                    {
                        prediction_ar += coeff * centered_data[n -1 -i];
                    }
                    else
                    {
                        prediction_ar += coeff * prediction[n -1 -i];
                    }

            }

            predicted_value = prediction_ar + prediction_ma;
        }

        prediction[n+1] = predicted_value;
    }

    double mse = 0;
    //restore values
    for(i = 0; i < nobs + steps; ++i)
    {
        prediction[i] = prediction[i] + mean;

        if(i < nobs)
            mse += pow(prediction[i] - timeseries_data[i],2);
    }

    cout << "MSE: " << to_string(mse/nobs) << endl;

    return prediction;
}


 //Computes the moving average process utilizing the innovations algorithm as
 // a estimator for the theta parameters.

vector<double> Arima::ma()
{
    innovations_algorithm();
    int i, k, nobs = timeseries_data.size();

    vector<double> prediction(nobs + steps, 0);
    vector<double> demeaned_data(nobs, 0);
    double mse = 0.0;

    //center data
    for(i = 0; i < nobs; ++i)
    {
        demeaned_data[i] = timeseries_data[i] - mean;
    }

    for(i = order_ma; i < nobs; ++i)
        prediction[i] = demeaned_data[i];

    int size = theta[order_ma].size();
    cout << "Coeffizients Theta: ";
    for(i = 0; i < order_ma && i < size; ++i)
    {
        cout << to_string(theta[order_ma][i]) << "; ";
    }
    cout << endl;

//    LMFunctor my_functor;
//    my_functor.m_values = timeseries_data.size();
//    my_functor.m_inputs = order_ma;
//    my_functor.order_ma = order_ma;
//    my_functor.timeseries_data = timeseries_data;

//    Eigen::VectorXd x(order_ma);

//    for(i = 0; i < order_ma; ++i)
//        x(i) = theta[order_ma][i];

//    Eigen::NumericalDiff<LMFunctor> numDiff(my_functor);
//    Eigen::LevenbergMarquardt<Eigen::NumericalDiff<LMFunctor>, double>
//            lm(numDiff);
//    lm.parameters.xtol = 1.0e-10;
//    lm.parameters.maxfev = 2000;
//    lm.minimize(x);

//    cout << "Levenberg-Marquardt optimized coeffizients: ";
//    for(i = 0; i < order_ma; ++i)
//    {
//        cout << to_string(x(i)) << "; ";
//    }
//    cout << endl;


    for( k = 0; k < nobs + steps; ++k)
    {
        double predicted_value = 0;
        for(i = 0 ; min(i, order_ma) < order_ma; ++i)
        {
            double coeff = theta[order_ma][i];
            if(i < nobs)
            {
                predicted_value += theta[order_ma][i]
                        * (timeseries_data[k + 1 - i] - prediction[k + 1 - i]) ;
            }
            else //no further observed values
            {
                predicted_value += coeff * (prediction[k + 1 - i]) ;
            }
        }
        prediction[k] = predicted_value;
    }

    for(i = 0; i < nobs + steps; ++i)
    {
        prediction[i] = prediction[i] + mean;
        if(i < nobs)
        {
            mse += pow(prediction[i] - timeseries_data[i], 2);
        }
    }

    cout << "MSE: " << to_string(mse/nobs) << endl;

    return prediction;

}



vector<double> Arima::arma()
{
    vector<double> prediction;



    return prediction;
}

vector<double> Arima::arima()
{
    vector<double> prediction;

    return prediction;
}


 // @brief Arima::innovations_algorithm classic innovations
 //algorithm to estimate the parameters
 // of an MA process.
void Arima::innovations_algorithm()
{
    ny[0] = acvf[0];

    for ( size_t i = 1; i < timeseries_data.size(); ++i)
    {
        for( size_t k = 0; k < i; ++k )
        {
            double sub = 0;
            for( size_t j = 0; j < k; ++j)
            {
                sub += theta[k][k-j] * theta[i][i-j] * ny[j];
            }
            theta[i][i-k] = 1 / ny[k] * (acvf[i-k] - sub);

            ny[i] = acvf[0];

            for ( size_t j = 0; j <= i; ++j)
            {
                ny[i] -= pow( theta[i][i-j], 2) * ny[j];
            }
        }
    }

    int size = timeseries_data.size() -1;

    for(int i = 0; i < size; ++i)
    {
        for(int j = 0; j < size; ++j)
        {
           theta[i][j] = theta[i + 1][j + 1];
        }
    }
}


 // Differences the timeseries data with the given order to make it stationary.
void Arima::differencing()
{
    int i, j, nobs = timeseries_data.size();

    for(i = 0; i < order_differencing; ++i)
    {
        for( j = 0; j < nobs - i + order_differencing ; ++j)
        {
            timeseries_data[j] = timeseries_data[j] - timeseries_data[i];
        }
    }
    timeseries_data.erase(timeseries_data.begin() + nobs - order_differencing);
    timeseries_data.shrink_to_fit();
}
