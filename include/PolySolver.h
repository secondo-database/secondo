/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]

//[TOC] [\tableofcontents]

[1] PolySolver - Methods to Solve Polynomic Equations

The methods of this file used to be part of the ~TemporalLiftedAlgebra.cpp~.
They have been moved to this file, as the methods may be useful even
outside this algebra.

There were two mayor errors, that have been corrected by Christian D[ue]ntgen
in December 2006. Arguments were changed to be ~const~ and ~references~
whereever practicable.

*/

#ifndef _POLYSOLVER_H_
#define _POLYSOLVER_H_

bool PS_AlmostEqual( const double d1, const double d2 );

/*
1.1 Method ~Swap~

Just changes place of the two given arguments.

*/

void PS_Swap(double& a, double& b);


/*
2. Methods to Solve Polynomic Equations

The Following methods implement algebraic algorithms to solve equations
of the form $a_n x^n + a_{n-1} x^{n-1} + \ldots + a_1 x^1 + a_0 = 0$, for
$1 \leq n \leq 4$. There is no general solution for polynomials with degree
$n>4$.

2.1 Method ~SolvePoly~ 1

Solves the Polynom ax+b=0 and gives back the number of solutiones.

*/
int SolvePoly(const double &a, const double &b, double &solution);


/*
2.2 Method ~SolvePoly~ 2

Solves the Polynom ax\^2+bx+c=0 and gives back the number of solutiones.
The solutions are given back in ordered style if sort is true.

*/
int SolvePoly(const double &a, const double &b,
              const double &c, double solution[2],
              const bool &sort);

/*
2.3 Method ~SolvePoly~ 3

Solves the Polynom ax\^3+bx\^2+cx+d=0 and gives back the number
of solutiones. The solutions are given back ordered style.

*/
int SolvePoly(const double &a, const double &b, const double &c,
              const double &d, double solution[3]);

/*
2.4 Method ~SolvePoly~ 4

Solves the Polynom ax\^4+bx\^3+cx\^2+dx+e=0 and gives back the number of
solutiones. The solutions are given back in an ordered style.

*/
int SolvePoly(const double &a, const double &b,
              const double &c, const double &d, const double &e,
              double solution[4]);

#endif // _POLYSOLVER_H_
