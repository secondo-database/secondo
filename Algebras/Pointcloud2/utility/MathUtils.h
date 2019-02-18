/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 MathUtils class

*/

#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace pointcloud2 {

class MathUtils {
public:
    /* initializes the random number generator with the given seed.
     * If seed is 0, the current time will be used; otherwise
     * the results of any consecutive getRnd() calls will be reproducible */
    static void initializeRnd(unsigned seed);

    /* returns a random number in [0.0, 1.0[ */
    static double getRnd();

    /* returns a random number in [min, max[ */
    static double getRnd(const double min, const double max);

    /* returns a random int in [0, size - 1] */
    static size_t getRndInt(const size_t size);
};



/*
2 Matrix class

*/
template<int colCount>
class Matrix {
    std::vector<std::array<double, colCount>> _mat;

public:
    Matrix(std::vector<std::array<double, colCount>> mat)
        : _mat(mat) {
    }

    double get(const size_t line, const size_t column) const {
        return _mat[line][column];
    }

    void print() const {
        for (int i = 0; i < _mat.size(); ++i) {
            for (int j = 0; j < colCount; ++j)
                std::cout << _mat[i][j] << " ";
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    void swapLines(const unsigned index1, const unsigned index2) {
        std::iter_swap(_mat.begin() + index1, _mat.begin() + index2);
    }

    void multiplyLine(const unsigned index, const double multiplicand) {
        auto& line = _mat[index];
        for(int i = 0; i < colCount; ++i)
            line[i] *= multiplicand;
    }

    /* adds to factor * lineSource to lineDest, starting from startColumn
     * (assuming that left of startColumn everything is 0.0) */
    void addToLine(const unsigned lineDest,
            const unsigned lineSource, const double factor, const
            unsigned startColumn) {

        auto& source = _mat[lineSource];
        auto& dest = _mat[lineDest];
        for (int i = startColumn; i < colCount; ++i)
            dest[i] += factor * source[i];
    }
};

template class Matrix<4>;
template class Matrix<5>;

/*
Solves the linear system presented by the given (unknownCount)
equations. If the linear system cannot be solved, an empty vector
is returned. unknownCount is the number of equations; the augmented
matrix has an extra column, therefore unknownCount + 1 columns.

Example of usage:

----
auto res = solveLinearSystem<3>(
{{0, 0, 1, 2}, {3, -1, 5, 2}, {1, 2, 3, 1}});
std::cout << res[0] << ", " << res[1] << ", " << res[2] << std::endl;
----

*/
template<size_t unknownCount>
std::vector<double> solveLinearSystem(
     std::vector<std::array<double, unknownCount + 1>> eqs) {

    assert (eqs.size() == unknownCount);
    Matrix<unknownCount + 1> mat(eqs);
    std::vector<double> results;

    // iterate over lines
    size_t lineCount = unknownCount;
    for (size_t i = 0; i < lineCount; ++ i) {
        // if the element on the diagonal is 0, try to swap with
        // a line below i
        if (mat.get(i, i) == 0.0) {
            bool found = false;
            for (size_t j = i + 1; j < lineCount; ++j) {
                if (mat.get(j, i) != 0.0) {
                    mat.swapLines(i, j);
                    found = true;
                    break;
                }
            }
            if (!found)
                return results;
        }

        // make the diagonal element 1.0
        mat.multiplyLine(i, 1 / mat.get(i, i));

        // eliminate the elements below mat[i][i]
        for (size_t j = i + 1; j < lineCount; ++j)
            mat.addToLine(j, i, -mat.get(j, i), i);
    }

    // compile the results
    results.resize(unknownCount);
    for (int res = lineCount - 1; res >= 0; --res) { // must be int!! size_t
                                                     // is always >= 0 ...
        size_t i(res);
        double result = mat.get(i, unknownCount);
        for (size_t j = i + 1; j < unknownCount; ++j)
            result -= mat.get(i, j) * results[j];
        results[i] = result;
    }

    return results;
}

std::string formatInt(const long num);

} /* namespace pointcloud2 */
