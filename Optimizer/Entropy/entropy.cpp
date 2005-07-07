/*
entropy.cpp

*/


#include "NLF.h"
#include "BoundConstraint.h"
#include "LinearEquation.h"
#include "OptNIPS.h"
#include "OptCG.h"
#include "OptPDS.h"
#include "OptFDNewton.h"
#include "OptQNewton.h"
#include "OptConstrNewton.h"
#include "entropy.h"

#include <iostream>
#include <iomanip>

#define LOOP( i, v ) for( int i = 1; i <= v.Nrows(); i++ )
#define BIT(j) (1<<(j))

extern bool OptNIPSLike_interpolate;
//Teste
void maximize_entropy( vector<double>& marginalProbability,
                       vector<pair<int,double> >& jointProbability,
                       vector<pair<int,double> >& estimatedProbability );

//#define BOUND_CONSTRAINTS
/*
Actually we don't need the boundary constraints for probabilities since the
entropy function is convex and its value is zero in the valid boundary:

---- E(0) => - 0 ln(0) = 0, E(1) => - 1 ln(1) = 0.
----

Without these constraints the method converges faster and more accurattely since we don't
have to use the log penalty function. They are here only for debugging purposes.

*/

using namespace std;

void init_f(int ndim, ColumnVector& x);
void f(int mode, int ndim, const ColumnVector& x, double& fx,
       ColumnVector& gx, SymmetricMatrix& Hx, int& result);
void print_constraints(Matrix& cteA, ColumnVector& cteB);
void eliminate_variables( const Matrix& cteA );

CompoundConstraint* build_contraints( int& nVars,
                                      vector<double>& marginalProbability,
                                      vector<pair<int,double> >& jointProbability,
                                      Matrix& cteA, ColumnVector& cteB )
// The number of independent variables after simplifications can change
{
  const int nPred = marginalProbability.size(),
            nGiven = jointProbability.size();

  nVars = 1 << nPred;
  cteB.ReSize(nPred+nGiven+1);
  cteA.ReSize(nPred+nGiven+1, nVars); // Constraint: cteA . X = cteB

#ifdef BOUND_CONSTRAINTS
  ColumnVector lower(nVars), upper(nVars);   // Constraint: lower <= x <= upper

  // Boundary constraints for probability
  LOOP( i, lower ) lower(i) = 0.0;
  LOOP( i, upper ) upper(i) = 1.0;
#endif

  // The sum of all probability must be 1
  for(int i = 1; i <= nVars; i++)
    cteA(1,i) = 1.0;
  cteB(1) = 1.0;

  // Constraints for marginal probability
  for( int j = 0; j < nPred; j++ )
  {
    for( int i = 1; i <= nVars; i++ )
      cteA(j+2, i) = (BIT(j) & i)? 1.0 : 0.0;

    cteB(j+2) = marginalProbability[j];
  }

  // Constraints for joint probability
  for( int j = 0; j < nGiven; j++ )
  {
    const int jointPred = jointProbability[j].first;

    for( int i = 1; i <= nVars; i++ )
      cteA(j+nPred+2, i) = ((jointPred & i) == jointPred)? 1.0 : 0.0;

    cteB(j+nPred+2) = jointProbability[j].second;
  }

  print_constraints( cteA, cteB );

#ifdef BOUND_CONSTRAINTS
  return new CompoundConstraint( new BoundConstraint(nVars, lower, upper),
                                 new LinearEquation(cteA, cteB) );
#else
  return new CompoundConstraint( new LinearEquation(cteA, cteB) );
#endif
}

ostream& operator << (ostream& o, const ColumnVector& v)
{
  LOOP( i, v )
    o << v(i) << endl;

  return o;
}

void print_cond_prob( int npred, const ColumnVector& x )
{
  cout << x << endl;

  for( int jp = 1; jp < (1 << npred); jp++ )
  {
    double p = 0.0;
    cout << jp << ": ";
    for( int i = 0; i < (1 << npred); i++ )
      if( (jp & i) == jp )
      {
        cout << i << " ";
        p += x(i);
      }

    cout << " => " << p << endl;
  }
}

#ifdef STAND_ALONE

int main( int argc, const char* argv[] )
{
  if( argc == 1 )
  {
    cout << "Computes conditional probability using Maximum Entropy" << endl
         << "Usage: entropy n [p1 p2 p3 ... pn] [cp1 cp2...]" << endl
         << "where n is the number of predicates, (p1..pn) is the probability "
         << "of each predicate and (cp1..cpn) is the joint probability "
         << "assuming the following implicit order:" << endl
         << "cp1 = P(p1 and p2), cp2 = P(p1 and p2 and p3) and so on." << endl << endl;

    exit(0);
  }

  int npred = atoi( argv[1] );
  int ngiven = argc - npred - 2;
  int nvars = 1 << npred;

  vector<double> marginalProb;
  vector<pair<int,double> > jointProb;
  vector<pair<int,double> > estimProb;

  for( int i = 0; i < npred; i++ )
    marginalProb.push_back( atof( argv[i+2] ) );

  for( int i = 0, j = 1; i < ngiven; i++ )
  {
    j |= BIT(i+1);
    jointProb.push_back( pair<int,double>( j, atof( argv[i+npred+2] ) ) );
  }

  maximize_entropy(marginalProb, jointProb, estimProb);
}

#endif

void print_constraints( Matrix& cteA, ColumnVector& cteB )
{
  for( int i = 1; i <= cteA.Nrows(); i++ )
  {
    for( int j = 1; j <= cteA.Ncols(); j++ )
      cout << cteA(i, j) << " ";

    cout << "= " << cteB(i) << endl;
  }
}

// The algorithm is very sensible to the initial point, so we compute a solution
// assuming that there is no correlation between predicates, hoping that it
// is close enough to the ME solution.
void find_initial_point( vector<double>& marginalProbability, ColumnVector& X )
{
  const int nVars = 1 << marginalProbability.size();
  ColumnVector B(nVars);
  Matrix A(nVars, nVars);

  for( int pred = 1; pred < nVars; pred++ )
  {
    double prob = 1.0;

    for( int i = 1; i <= nVars; i++ )
      A(pred, i) = ((pred & i) == pred)? 1.0 : 0.0;

    for( int j = 0; j < marginalProbability.size(); j++ )
      if( (pred & BIT(j)) == BIT(j) )
        prob *= marginalProbability[j];

    B(pred) = prob;
  }

  for( int i = 1; i <= nVars; i++ )
    A(nVars, i) = 1.0;

  B(nVars) = 1.0;

  // Find the solution of the linear equations
  X = A.i() * B;
}

// This varible is used to bypass a flaw int the design of the library: we need
// more parameters in the init_f function.
static vector<double>* ptr_marginalProbability;

void maximize_entropy( vector<double>& marginalProbability,
                       vector<pair<int,double> >& jointProbability,
                       vector<pair<int,double> >& estimatedProbability )
{
  estimatedProbability.clear();

  try
  {
    int nVars, maxIter;
    Matrix A;
    ColumnVector B;
    CompoundConstraint* constraint = NULL;
    try
    {
      constraint = build_contraints(nVars, marginalProbability, jointProbability, A, B);
      NLF2 entropyProblem(nVars, f, init_f, constraint);

      // To prevent long calculations
      maxIter = 20000 / nVars;
      ptr_marginalProbability = &marginalProbability;
      OptNIPS objfcn(&entropyProblem);
      OptNIPSLike_interpolate = true;
      objfcn.setOutputFile("output.txt", 0);
      objfcn.setFcnTol(1e-12);
      objfcn.setConTol(1e-16);
      objfcn.setGradTol(1e-16);
      objfcn.setLineSearchTol(1e-16);
      objfcn.setStepTol(1e-16);
      objfcn.setMinStep(1e-24);
      objfcn.setMaxIter(maxIter); 
      objfcn.setSearchStrategy(LineSearch);
      objfcn.setMaxBacktrackIter(maxIter);
      objfcn.setMeritFcn(NormFmu);
      try
      {
        objfcn.optimize();
      }
      catch(...)
      {
        cout << "Failed!" << endl;
        // objfcn.optimize();
      }
      objfcn.printStatus("Solution from entropy");

      cout << "Entropy: " << - entropyProblem.getF() << endl;

#ifdef STAND_ALONE
      print_cond_prob(marginalProbability.size(), entropyProblem.getXc());
#endif

      ColumnVector const &x = entropyProblem.getXc();

      for( int jp = 1; jp < nVars; jp++ )
      {
        double p = 0.0;
        for( int i = 0; i < nVars; i++ )
          if( (jp & i) == jp )
            p += x(i);

        estimatedProbability.push_back(pair<int,double>(jp, p));
      }
      objfcn.cleanup();
    }
    catch(...)
    {
      cout << "Unable to use entropy approach!" << endl;
      delete constraint;
    }
  }
  catch(...)
  {
    cout << "Unable to use entropy approach!" << endl;
  }
}

void init_f(int ndim, ColumnVector& x)
{
  find_initial_point( *ptr_marginalProbability, x );
}

#define EPSILON 1.0e-64
// Some adjustments to make the entropy function continuous and differentiable at 0.
void f(int mode, int ndim, const ColumnVector& x, double& fx,
       ColumnVector& gx, SymmetricMatrix& Hx,
       int& result)
{
  if( mode & NLPFunction )
  {
    double val = 0.0;

    LOOP( i, x )
    {
      const double xi = x(i);

      if( xi > EPSILON )
        val += xi * log(xi);
      else
        val += xi * log(EPSILON);
    }

    fx = val;
    result = NLPFunction;
  }

  if( mode & NLPGradient )
  {
    LOOP( i, x )
    {
      const double xi = x(i);

      if( xi > EPSILON )
        gx(i) = log(xi) + 1;
      else
        gx(i) = log(EPSILON) + 1;
    }

    result = NLPGradient;
  }

  if( mode & NLPHessian )
  {
    LOOP( i, x )
    {
      const double xi = x(i);

      if( xi > EPSILON )
        Hx(i,i) = 1.0/xi;
      else
        Hx(i,i) = 0;
    }

    result = NLPHessian;
  }
}


