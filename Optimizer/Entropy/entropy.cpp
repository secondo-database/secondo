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
#define EPSILON     1.0e-96
#define TOL_INIT    1.0e-8
#define TOL_SOL     1.0e-6


/*
Since this method is very sensible to the initial point, we are solving two problems:
first we find a viable solution (VS) to be the initial point, then we find the
optimal solution (OS). To find a viable solution we just set a boundary constrained
problem.

Once we've found a viable solution we don't need the boundary constraints for
probabilities since the entropy function is convex and its value is zero in the
valid boundary:

---- E(0) => - 0 ln(0) = 0, E(1) => - 1 ln(1) = 0.
----

Without these constraints the second problem converges faster and more accurattely
because we don't have to use the log penalty function, which introduces round off
errors.

*/

#define BOUND_CONSTRAINTS

using namespace std;
extern bool OptNIPSLike_interpolate;

void init_f_VS(int ndim, ColumnVector& x);
void init_f_OS(int ndim, ColumnVector& x);
void f_VS(int mode, int ndim, const ColumnVector& x, double& fx,
          ColumnVector& gx, SymmetricMatrix& Hx, int& result);
void f_OS(int mode, int ndim, const ColumnVector& x, double& fx,
          ColumnVector& gx, SymmetricMatrix& Hx, int& result);

void print_constraints(Matrix& cteA, ColumnVector& cteB);

BoundConstraint* build_bound_contraints( const vector<double>& marginalProbability,
                                         const vector<pair<int,double> >& jointProbability )
{
  const int nPred = marginalProbability.size(),
            nGiven = jointProbability.size(),
            nVars = 1 << marginalProbability.size();
  ColumnVector lower(nVars), upper(nVars);   // Constraint: lower <= x <= upper

  // Boundary constraints for probability
  LOOP( i, lower ) lower(i) = 0.0;
  LOOP( i, upper ) upper(i) = 1.0;

  return new BoundConstraint(nVars, lower, upper);
}

LinearEquation* build_linear_contraints( const vector<double>& marginalProbability,
                                         const vector<pair<int,double> >& jointProbability )
{
  const int nPred = marginalProbability.size(),
            nGiven = jointProbability.size(),
            nVars = 1 << marginalProbability.size();
  Matrix cteA(nPred+nGiven+1, nVars); // Constraint: cteA . X = cteB
  ColumnVector cteB(nPred+nGiven+1);

  // The sum of all probabilities must be 1
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

  return new LinearEquation(cteA, cteB);
}

ostream& operator << (ostream& o, const ColumnVector& v)
{
  LOOP( i, v )
    o << v(i) << endl;

  return o;
}

void print_cond_prob( int npred, const ColumnVector& x )
{
  double p = 0.0;
  cout << x << endl;

  for( int i = 1; i <= (1 << npred); i++ )
    p += x(i);

  cout << "All (must be 1): " << p << endl;

  for( int jp = 1; jp < (1 << npred); jp++ )
  {
    p = 0.0;
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

 // Auxiliar function that check ranges for marginal probabilities and sum of all probs
 // assuring feasilbility. For example, if p(A) = 0.6 and p(B) = 0.7, p(A.B) must be in
 // the range [0.3, 0.6]
 void adjust_ranges( const int nVars, ColumnVector& minP, ColumnVector& maxP )
 {
   for( int i = 1; i < nVars; i++ )
     for( int j = 1; j < nVars; j <<= 1 )
       if( (i & j) == 0 )
       {
         if( minP(i) + minP(j) >= 1)
           minP(i|j) = fmax(minP(i|j), minP(i) + minP(j) - 1) ;

         maxP(i|j) = fmin(maxP(i|j), fmin(maxP(i), maxP(j)));
       }
}

// This function ensures that there will be no selectivity equals to 0 or 1
// since in these cases the algorithm does not converge. Since the results
// are approximated, we'll change the intermediate results by a small fraction
// when it happens. Also, we check for feasibility since the marginal probabilities
// are measured in one sample and the joint probabilities in another, which may
// cause strange results.
void adjust_joint_probabilities( const vector<double>& margProb,
                                 vector<pair<int,double> >& jntProb )
{
  const int nVars = 1 << margProb.size();
  ColumnVector minP(nVars), maxP(nVars);
  int njp = 0;

  // Error in input size - just truncate
  if( jntProb.size() >= margProb.size() )
  {
    vector<pair<int,double> > aux = jntProb;

    jntProb.resize(margProb.size() - 1);
    for( int i = 0; i < jntProb.size(); i++ )
      jntProb[i] = aux[i];
  }
  // Generic Ranges
  for( int i = 1; i <= nVars; i++ )
  {
    minP(i) = 0.0 + TOL_INIT;
    maxP(i) = 1.0 - TOL_INIT;
  }

  // Ranges for marginal Probabilities
  for( int i = 0; i < margProb.size(); i++ )
    minP(1 << i) = maxP(1 << i) = margProb[i];

  // Verify feasibility
  adjust_ranges( nVars, minP, maxP );

  // Check each joint probability against ranges and adjust them
  for( int i = 0; i < jntProb.size(); i++ )
  {
    const int node = jntProb[i].first;
    double prob = jntProb[i].second;

    // Change probabilities to agree with ranges
    if( prob < minP(node) + TOL_INIT )
      prob = minP(node) + TOL_INIT;
    else if( prob > maxP(node) - TOL_INIT )
      prob = maxP(node) - TOL_INIT;

    // Define new ranges
    minP(node) = jntProb[i].second = maxP(node) = prob;

    jntProb[i].second = prob;
    adjust_ranges( nVars, minP, maxP );
  }
}

void print_constraints( Matrix& cteA, ColumnVector& cteB )
{
  for( int i = 1; i <= cteA.Nrows(); i++ )
  {
    for( int j = 1; j <= cteA.Ncols(); j++ )
      cout << cteA(i, j) << " ";

    cout << "= " << cteB(i) << endl;
  }
}

// Computes a solution assuming that there is no correlation between predicates
void find_independent_solution( const vector<double>& margProb, ColumnVector& X )
{
  const int nVars = 1 << margProb.size();
  ColumnVector B(nVars);
  Matrix A(nVars, nVars);

  // Compute joint probabilities
  for( int pred = 1; pred < nVars; pred++ )
  {
    double prob = 1.0;

    for( int i = 1; i <= nVars; i++ )
      A(pred, i) = ((pred & i) == pred)? 1.0 : 0.0;

    for( int j = 0; j < margProb.size(); j++ )
      if( (pred & BIT(j)) == BIT(j) )
        prob *= margProb[j];

    B(pred) = prob;
  }

  for( int i = 1; i <= nVars; i++ )
    A(nVars, i) = 1.0;

  B(nVars) = 1.0;

  // Find the solution of the linear equations
  X = A.i() * B;
}

// The algorithm is very sensible to the initial point, so we compute a viable
// solution and use it as initial point
void find_viable_solution( const vector<double>& margProb,
                           const vector<pair<int,double> >& jntProb,
                           TOLS& tols,
                           ColumnVector& x )
{
  const int nVars = 1 << margProb.size();
  CompoundConstraint *constraints =
    new CompoundConstraint(build_bound_contraints(margProb, jntProb),
                           build_linear_contraints(margProb, jntProb));
  NLF2 viableSolutionProblem(nVars, f_VS, init_f_VS, constraints);
  OptNIPS solver(&viableSolutionProblem, tols);

  solver.setOutputFile("output.txt", 0);
  solver.setSearchStrategy(LineSearch);
  solver.setMeritFcn(NormFmu);
  solver.optimize();
  solver.printStatus("Viable Solution");
  x = viableSolutionProblem.getXc();
  solver.cleanup();

  for( int i=1; i <= nVars; i++ )
    if( x(i) < EPSILON )
      x(i) = EPSILON;
}


MeritFcn meritFcn;

void find_optimal_solution( const vector<double>& margProb,
                            const vector<pair<int,double> >& jntProb,
                            TOLS& tols,
                            ColumnVector& x )
{
  const int nVars = 1 << margProb.size();
  CompoundConstraint *constraint =
    new CompoundConstraint(build_linear_contraints(margProb, jntProb));
  NLF2 entropyProblem(nVars, f_OS, init_f_OS, constraint);
  OptNIPS solver(&entropyProblem, tols);

  solver.setOutputFile("output.txt", 1);
  solver.setSearchStrategy(LineSearch);
  solver.setMeritFcn(meritFcn);
  solver.optimize();
  solver.printStatus("Solution for Entropy");
  x = entropyProblem.getXc();
  solver.cleanup();

  for( int i=1; i <= nVars; i++ )
    if( x(i) < EPSILON )
      x(i) = EPSILON;

  cout << "Entropy: " << - entropyProblem.getF() << endl;

#ifdef STAND_ALONE
  print_cond_prob(margProb.size(), x);
#endif
}

// Computes the combined probabilities from solution
void compute_probabilities(const ColumnVector &x,
                           vector<pair<int,double> >& estimatedProbability)
{
  estimatedProbability.clear();

  for( int jp = 1; jp < x.Nrows(); jp++ )
  {
    double p = 0.0;

    for( int i = 0; i < x.Nrows(); i++ )
      if( (jp & i) == jp )
        p += x(i);

    estimatedProbability.push_back(pair<int,double>(jp, p));
  }
}

// Some tests are done to ensure that this is a valid solution regardless of the
// convergence or not of the problem.
void verify_solution( const ColumnVector &x,
                      const vector<pair<int,double> >& jointProbability,
                      const vector<pair<int,double> >& estimatedProbability)
{
  double sum = 0.0;

  // Check for negative probabilities - a small tolerance is allowed
  for( int i=1; i <= x.Nrows(); i++ )
    if( x(i) < - TOL_SOL )
      throw 0;

  // Check the sum of probabilities
  for( int i=1; i <= x.Nrows(); i++ )
    sum += x(i);

  if( fabs( sum - 1 ) > TOL_SOL )
    throw 0;

  // Check for negative joint probabilities - no tolerance
  for( int i=0; i < estimatedProbability.size(); i++ )
    if( estimatedProbability[i].second < 0 )
      throw 0;

  // Check if the solution is close to measured joint probabilities
  for( int i=0, j=0; i < estimatedProbability.size() && j < jointProbability.size(); i++ )
    if( jointProbability[j].first == estimatedProbability[i].first )
    {
      if( fabs( jointProbability[j].second - estimatedProbability[i].second ) > TOL_SOL )
        throw 0;

      j++;
    }
}

// These varibles are used to bypass a flaw int the design of the library: we need
// more parameters in the init_f function.
static vector<double>* ptrMargProb;
static vector<pair<int,double> >* ptrJointProb;
static ColumnVector* ptrInitPoint;

void maximize_entropy( vector<double>& marginalProbability,
                       vector<pair<int,double> >& jointProbability,
                       vector<pair<int,double> >& estimatedProbability )
{
  TOLS tols;
  const int nVars = 1 << marginalProbability.size(),
            maxIter = 10000 / nVars;
  ColumnVector initPoint(nVars),
               x(nVars);

  ptrMargProb = &marginalProbability;
  ptrJointProb= &jointProbability;
  ptrInitPoint = &initPoint;

  estimatedProbability.clear();

  // Sets tolerance values
  tols.setDefaultTol();
  tols.setMinStep(1e-16);
  tols.setStepTol(1e-12);
  tols.setFTol(1e-12);
  tols.setCTol(1e-12);
  tols.setGTol(1e-12);
  tols.setLSTol(1e-12);
  tols.setMaxIter(maxIter);
  tols.setMaxBacktrackIter(maxIter);
  tols.setMaxFeval(maxIter);

  // Library flaw
  OptNIPSLike_interpolate = true;

  try
  {
    // Some adjustments are made to ensure convergence when we have any selectivity = 1
    adjust_joint_probabilities(marginalProbability, jointProbability);

    if( marginalProbability.size() > 2 && jointProbability.size() > 0 )
    {
      try
      {
        // First optimization problem
        find_viable_solution(marginalProbability, jointProbability, tols, initPoint);
        compute_probabilities(initPoint, estimatedProbability);
        verify_solution(initPoint, jointProbability, estimatedProbability);
        meritFcn = ArgaezTapia;
      }
      catch(...)
      { // If some error occurs, try using a neutral approach for init point
        cout << "Viable point search failed..." << endl;
        for( int i = 1; i <= nVars; i++ )
          initPoint(i) = 1.0;

        meritFcn = NormFmu;
      }

      // Second optimization problem
      find_optimal_solution(marginalProbability, jointProbability, tols, x);
      compute_probabilities(x, estimatedProbability);
      verify_solution(x, jointProbability, estimatedProbability);
    }
    else // In these cases we don't need to use a numerical method
      if( marginalProbability.size() > 1 && jointProbability.size() == 0 )
      {
        find_independent_solution(marginalProbability, x );
        compute_probabilities(x, estimatedProbability);
      }
      else if( marginalProbability.size() == 2 && jointProbability.size() == 1 )
      {
        estimatedProbability.push_back(pair<int,double>(1, marginalProbability[0]));
        estimatedProbability.push_back(pair<int,double>(2, marginalProbability[1]));
        estimatedProbability.push_back(jointProbability[0]);
      }
      else // marginalProbability.size() == 1 && jointProbability.size() == 0
      {
        estimatedProbability.push_back(pair<int,double>(1, marginalProbability[0]));
      }
    cout << "Entropy approach converged!" << endl;
  }
  catch(...)
  {
    // If any problem occurs, use the "independence of predicates" approach.
    find_independent_solution(marginalProbability, x );
    compute_probabilities(x, estimatedProbability);

    cout << "Unable to use the entropy approach!" << endl;
  }
}

void init_f_VS(int ndim, ColumnVector& x)
{
  double val = 1.0;

  // Adjust initial values for first problem
  if( ndim > 3 && ptrJointProb->size() > 1 )
  {
    double v1 = (*ptrJointProb)[ptrJointProb->size()-1].second,
           v2 = (*ptrJointProb)[ptrJointProb->size()-2].second - v1;

    val = (val - v1 - v2) / (ndim - 2);
    for( int i = 1; i <= ndim; i++ )
      x(i) = val;

    x(ndim-1) = v1;
    x(ndim/2 - 1) = v2;
  }
  else
    for( int i = 1; i <= ndim; i++ )
      x(i) = 1.0 / ndim;

#ifdef STAND_ALONE
  cout << "Initial point: " << endl << x << endl;
#endif
}

// ptrInitPoint holds the solution of first optimization problem
void init_f_OS(int, ColumnVector& x)
{
  x = *ptrInitPoint;

#ifdef STAND_ALONE
  cout << "Viable point: " << endl << x << endl;
#endif
}

// Some adjustments to make the entropy function continuous and differentiable at 0.
void f_OS(int mode, int ndim, const ColumnVector& x, double& fx,
          ColumnVector& gx, SymmetricMatrix& hx, int& result)
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
        gx(i) = log(EPSILON);
    }

    result = NLPGradient;
  }

  if( mode & NLPHessian )
  {
    LOOP( i, x )
    {
      const double xi = x(i);

      if( xi > EPSILON )
        hx(i,i) = 1.0/xi;
      else
        hx(i,i) = 0;
    }

    result = NLPHessian;
  }
}

// Very simple function to get consistent probabilities for start point
void f_VS(int mode, int ndim, const ColumnVector& x, double& fx,
       ColumnVector& gx, SymmetricMatrix& hx,
       int& result)
{
  if( mode & NLPFunction )
  {
    double val = 0.0;

    LOOP( i, x ) val += - x(i);

    fx = val;
    result = NLPFunction;
  }

  if( mode & NLPGradient )
  {
    LOOP( i, x ) gx(i) = - 1.0;

    result = NLPGradient;
  }

  if( mode & NLPHessian )
  {
    LOOP( i, x ) hx(i,i) = 0.0;

    result = NLPHessian;
  }
}



