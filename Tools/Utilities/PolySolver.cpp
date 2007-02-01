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

#include <cmath>
#include <limits>
#include <iostream>
#include "PolySolver.h"

using namespace std;

// #define POLYSOLVERDEBUG

#define PS_FACTOR 0.00000001

bool PS_AlmostEqual( const double d1, const double d2 )
{
  int i1, i2;
  double dd1 = frexp( d1, &i1 ),
         dd2 = frexp( d2, &i2 );

  return i1 == i2 &&
         fabs(dd1 - dd2) < PS_FACTOR;
}

/*
1.1 Method ~Swap~

Just changes place of the two given arguments.

*/

void PS_Swap(double& a, double& b)
{
  double i;
  i = a;
  a = b;
  b = i;
}

/*
2. Methods to Solve Polynomic Equations

The Following methods implement algebraic algorithms to solve equations
of the form $a_n x^n + a_{n-1} x^{n-1} + \ldots + a_1 x^1 + a_0 = 0$, for
$1 \leq n \leq 4$. There is no general solution for polynomials with degree
$n>4$.

2.1 Method ~SolvePoly~ 1

Solves the Polynom ax+b=0 and gives back the number of solutiones.

*/
int SolvePoly(const double &a, const double &b, double &solution)
{
#ifdef POLYSOLVERDEBUG
    cout<<"SolvePoly 1 called with a: "<<a<<" ,b: "<<b<<endl;
#endif
  if(a == 0.0)
    return 0;
  solution = (-b / a);
#ifdef POLYSOLVERDEBUG
    cout<<"SolvePoly1 ends with 1 solution"<<endl;
    cout<<"solution = "<<solution<<"  "<<(a*solution)+b<<endl;
#endif
  return 1;
}

/*
2.2 Method ~SolvePoly~ 2

Solves the Polynom ax\^2+bx+c=0 and gives back the number of solutiones.
The solutions are given back in ordered style if sort is true.

*/
int SolvePoly(const double &a, const double &b,
              const double &c, double solution[2],
              const bool &sort)
{
  int number = 0;
  double d;
#ifdef POLYSOLVERDEBUG
  cout<<"SolvePoly 2 called with a: "<<a<<" ,b: "<<b<<" ,c: "<<c<<endl;
#endif

  if(a != 0.0){ //SolvePoly2 needed
    d = pow(b, 2) - 4 * a * c;
    if (d < 0)
      number = 0;
    else if (d == 0) {
      solution[0] = -b / 2 / a;
      number = 1;
    }
    else {
      if (sort) {
        solution[0] = (a > 0) ? ((-b - sqrt(d)) / 2 / a)
                    : ((-b + sqrt(d)) / 2 / a);
        solution[1] = (a > 0) ? ((-b + sqrt(d)) / 2 / a)
                    : ((-b - sqrt(d)) / 2 / a);
      }
      else {
        solution[0] = (-b + sqrt(d)) / 2 / a;
        solution[1] = (-b - sqrt(d)) / 2 / a;
      }
      number = 2;
    }
  }
  else { //use SolvePoly1
    number = SolvePoly(b, c, solution[0]);
  }
#ifdef POLYSOLVERDEBUG
  cout<<"SolvePoly2 ends with  "<<number<<" solutions"<<endl;
  for (int i = 0; i < number; i++)
    cout<<"solution["<<i<<"] = "<<solution[i]
        <<"  "<<(a*pow(solution[i],2)+b*solution[i]+c)<<endl;
#endif
  return number;
}

/*
2.3 Method ~SolvePoly~ 3

Solves the Polynom ax\^3+bx\^2+cx+d=0 and gives back the number
of solutiones. The solutions are given back ordered style.

*/
int SolvePoly(const double &a, const double &b, const double &c,
              const double &d, double solution[3])
{
  int number = 0;
  double disk, p, q;

#ifdef POLYSOLVERDEBUG
  cout<<"SolvePoly3 called with a: "<<a<<" ,b: "<<b
      <<" ,c: "<<c<<" ,d: "<<d<<endl;
#endif
  if(a != 0.0) { //SolvePoly3 needed
    p = ( 3 * a * c - pow(b ,2)) / 9 / pow(a, 2);
    q = (2 * pow(b ,3) - 9 * a * b * c + 27 * pow(a, 2) * d) / 54 / pow(a, 3);
    disk = pow(p, 3) + pow (q, 2);
#ifdef POLYSOLVERDEBUG
    cout<<"p = "<<p<<", q = "<<q<<" disk = "<<disk<<endl;
#endif

    if (disk > 0) {  //one real solution
      double u = -q + sqrt(disk);
      u = u < 0 ? -pow(-u, 1.0 / 3.0) : pow(u, 1.0 / 3.0);
      double v = -q - sqrt(disk);
      v = v < 0 ? -pow(-v, 1.0 / 3.0) : pow(v, 1.0 / 3.0);
#ifdef POLYSOLVERDEBUG
      cout<<"u "<<u<<", v "<<v<<endl;
#endif
      solution[0] = u + v - b / 3 / a;
      number = 1;
    }
    else if (disk == 0) {
      double u = q<0 ? -pow(-q, 1.0 / 3.0) : pow(q, 1.0 / 3.0);
      solution[0] = 2 * u - b / 3 / a;
      solution[1] = -u - b / 3 / a;
      if (solution[0] > solution[1] )
        PS_Swap(solution[0], solution[1]);
      number = 2;
    }
    else {
      double phi = acos(-q / sqrt(abs(pow(p, 3))));
      solution[0] = 2 * sqrt(abs(p)) * cos(phi / 3) - b / 3 / a;
      solution[1] = -2 * sqrt(abs(p)) * cos(phi / 3 + 1.047197551196598)
                    - b / 3 / a;
      solution[2] = -2 * sqrt(abs(p)) * cos(phi / 3 - 1.047197551196598)
                    - b / 3 / a;

      if ( solution[0] > solution[1])
        PS_Swap(solution[0], solution[1]);
      if ( solution[1] > solution[2])
        PS_Swap(solution[1], solution[2]);
      if ( solution[0] > solution[1])
        PS_Swap(solution[0], solution[1]);
      number = 3;
    }
  }
  else { //use SolvePoly2
    double sol2[2];
    number = SolvePoly(b, c, d, sol2, true);
    for(int i = 0; i < number; i++)
      solution[i] = sol2[i];
  }
#ifdef POLYSOLVERDEBUG
  cout<<"SolvePoly3 ends with  "<<number<<" solutions"<<endl;
  for (int i = 0; i < number; i++)
    cout<<"solution["<<i<<"] = "<<solution[i]<<"  "
        <<(a*pow(solution[i],3)+b*pow(solution[i],2)+c*solution[i]+d)<<endl;
#endif
  return number;
}

/*
2.4 Method ~SolvePoly~ 4

Solves the Polynom ax\^4+bx\^3+cx\^2+dx+e=0 and gives back the number of
solutiones. The solutions are given back in an ordered style.

*/
int SolvePoly(const double &a, const double &b,
              const double &c, const double &d, const double &e,
              double solution[4])
{
  int number1 = 0;
  int number2 = 0;
  double z;
  double sol3[3];
  double sol21[2];
  double sol22[2];
  double sol23[2];
  double sol24[2];

#ifdef POLYSOLVERDEBUG
    cout<<"SolvePoly 4 called with a: "<<a<<" ,b: "<<b
    <<" ,c: "<<c<<" ,d: "<<d<<" ,e: "<<e<<endl;
#endif

  if(a != 0.0){ //SolvePoly4 needed
    //Solve cubic resolvent
    number1 = SolvePoly(1.0, -c, (b * d - 4 * a * e),
              (4 * a * c * e - pow(b, 2)* e - a * pow(d, 2)), sol3);
#ifdef POLYSOLVERDEBUG
    for (int i = 0; i < number1; i++)
      cout<<"sol3["<<i<<"] = "<<sol3[i]<<endl;
#endif
    z = a > 0.0 ? sol3[number1 - 1] : sol3[0];  // new
#ifdef POLYSOLVERDEBUG
    cout<<"z "<<z<<endl;
#endif
    number1 = SolvePoly(1.0, -b, (a * (c - z)), sol21, false);
#ifdef POLYSOLVERDEBUG
    for (int i = 0; i < number1; i++)
      cout<<"sol21["<<i<<"] = "<<sol21[i]<<endl;
#endif
    if (number1 == 1)
      sol21[1] = sol21[0];
    number1 = SolvePoly(1.0, -z, (a * e), sol22, false);
#ifdef POLYSOLVERDEBUG
    for (int i = 0; i < number1; i++)
      cout<<"sol22["<<i<<"] = "<<sol22[i]<<endl;
#endif
    if (number1 == 1)
      sol22[1] = sol22[0];
    if ((b * z)<(2 * a * d)) {
      PS_Swap(sol21[0], sol21[1]);
    }
    // find solutions
    number1 = SolvePoly(a, sol21[0], sol22[0], sol23, true);
    number2 = SolvePoly(a, sol21[1], sol22[1], sol24, true);
    for (int i = 0; i < number1; i++)
      solution[i] = sol23[i];
    for (int i = 0; i < number2; i++)
      solution[number1 + i] = sol24[i];
    number1 += number2;
    for (int i = 0; i < number1 ; i++)
      for (int n = i + 1; n < number1 ; n++)
        if (solution[i] > solution [n])
          PS_Swap(solution[i],solution[n]);
    for (int i = 0; i < number1 - 1; i++)
      if (PS_AlmostEqual(solution[i], solution[i+1])) {
        for (int n = 0; n < number1 -1; n++)
          solution[n] = solution [n + 1];
        number1--;
      }
  }
  else { //use SolvePoly3
    number1 = SolvePoly(b, c, d, e, sol3);
    for(int i = 0; i < number1; i++)
      solution[i] = sol3[i];
  }
#ifdef POLYSOLVERDEBUG
  cout<<"SolvePoly4 ends with  "<<number1<<" solutions"<<endl;
  for (int i = 0; i < number1; i++)
      cout<<"solution["<<i<<"] = "<<solution[i]<<"  "<<(a*pow(solution[i],4)
      +b*pow(solution[i],3)+c*pow(solution[i],2)+d*solution[i]+e)<<endl;
#endif
  return number1;
}

#endif // _POLYSOLVER_H_
