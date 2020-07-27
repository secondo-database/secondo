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

#ifndef ARIMA_H
#define ARIMA_H

#include <vector>
#include <armahelper.h>
#include <cmath>
//#include <Libraries/eigen-3.3.7/Eigen/Dense>
//#include <Libraries/eigen-3.3.7/Eigen/Core>
//#include <Libraries/eigen-3.3.7/unsupported/Eigen/NonLinearOptimization>
//#include <Libraries/eigen-3.3.7/unsupported/Eigen/NumericalDiff>

using std::vector;
using std::cout;
using std::string;
using std::min;

class Arima
{
public:
    Arima(int, int, int, int, vector<Tuple*>);
    vector<double> ar();
    vector<double> ma();
    vector<double> arma();
    vector<double> arima();
    vector<double> forecast();
    void printPACF(int lags);
    void printACF(int lags);

private:
    int order_ma;
    int order_ar;
    int order_differencing;
    int steps;

    vector<double> timeseries_data;
    vector<double> fitted_data;
    vector<double> differenced_data;


    double mean;

    //matrix of parameters for the AR-process
    vector<vector<long double>> phi;
    //Matrix of parameters for the MA-process
    vector<vector<double>> theta;
    //matrix of the partial autocorrelation coefficients of the process
    vector<vector<double>> pacf;
    //matrix of the autocorrelation coefficients of the process
    vector<vector<double>> acf;
    //innovations of the MA-process
    vector<double> ny;

    vector<double> acvf;
    void differencing();
    void innovations_algorithm();
    void durbin_levinson_algorithm();
    double gamma_hat(int index);
//    Eigen::VectorXd compute_ar_coeffs();
};

//template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
//struct Functor
//{
//    typedef _Scalar Scalar;
//    enum {
//        InputsAtCompileTime = NX,
//        ValuesAtCompileTime = NY,
//    };
//
//    typedef Eigen::Matrix<Scalar, InputsAtCompileTime,1> InputType;
//    typedef Eigen::Matrix<Scalar, ValuesAtCompileTime,1> ValueType;
//    typedef Eigen::Matrix<Scalar, ValuesAtCompileTime,
//    InputsAtCompileTime> JacobianType;
//
//    int m_inputs, m_values;
//
//    Functor() : m_inputs(ValuesAtCompileTime), m_values(InputsAtCompileTime){}
//    Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}
//
//    //Returns the number of values
//    int values() const {return m_values;}
//
//
//    int inputs() const {return m_inputs;}
//};
//
//struct LMFunctor : Functor<double>
//{
//       int operator()(const Eigen::VectorXd &x, Eigen::VectorXd &fvec) const
//    {
//        Eigen::VectorXd predictions(values());

//        for(int i = 0; i < inputs(); ++i)
//            predictions(i) = timeseries_data[i];

//        for(int k = inputs(); k < values()-1; ++k)
//        {
//            double predicted_value = 0;
//            for(int i = 0 ; min(i, inputs()) < inputs(); ++i)
//            {
//                double coeff = x(i);

//                predicted_value += coeff * (timeseries_data[k + 1 - i]
//                        - predictions[k + 1 - i]) ;

//            }
//            predictions(k) = predicted_value;
//            fvec(k) = predicted_value - timeseries_data[k];
//        }

//        return 0;
//    }

//    int order_ma;

//    int order() const {return order_ma;}

//    vector<double> timeseries_data;
//};

#endif // ARIMA_H
