/*

$Header$
@author Nikolai van Kempen

Provides the nonlinear algorithm to optimize a fomula that represents a cost
function that is aware of the memory dependet costs.

See \$SECONDO\_HOME/Optimizer/MemoryAllocation/ for more information.

*/
#include <string.h>
#include <list>
#include "stdlib.h"
#include "SWI-Prolog.h"
#include <nlopt.hpp>
#include <cstdio>
#include <vector>
#include <iostream>
#include <exception>

using namespace std; 

struct OpMemoryConstraint {
  int dimension;
  double sufficientMemory;
};

/*
Forwards the predicate references to the objective function.

*/

struct FData {
  predicate_t pF;
  predicate_t pD;
};

struct OptimizeResult {
  int rc;
  std::vector<double> x;
};

/*
A exception is needed to pass-through an error during predicate evaluation.

*/
class EvalException: public exception {
  virtual const char* what() const throw() {
    return "Predicate evaluation error.";
  }
} evalex;

/*
Generalized function to evaluate our objective and derivate predicates.

*/
double evalPredicate(predicate_t p, int i, unsigned n, const double *x) {
  int arity=i>=0?3:2;
  term_t tt = PL_new_term_refs(arity);
  term_t ct = tt; // The current term
  if (i>=0) {
    PL_unify_integer(ct, i);
    ct++;
  }
  PL_put_nil(ct);
  for(int i=0;i<n;i++) {
    term_t elem = PL_new_term_ref();
    // => The list is built tail-to-head!
    PL_put_float(elem, x[n-(i+1)]);
    PL_cons_list(ct, elem, ct);
  }

  qid_t qid=PL_open_query(NULL, PL_Q_NORMAL, p, tt);

  if (!PL_next_solution(qid)) {
    cout << "The objective function or one of the derivatives returns with ";
    cout << "no solution. Use the trace/debug predicates to analyze ";
    cout << "the error." << endl;
    PL_close_query(qid);
    throw evalex; // continue is not tolerable
  }

  ct++;
  double dresult=-1;
  if (!PL_get_float(ct, &dresult)) {
    cout << "The objective function or one of the derivatives returns with ";
    cout << "an invalid value. Use the trace/debug predicates to analyze ";
    cout << "the error." << endl;
    PL_close_query(qid);
    throw evalex; // continue is not tolerable
  }
  PL_close_query(qid);
  return dresult;
}

/*
The function that should be minimized.

*/
double objectiveFunction(unsigned n, const double *x, double *grad, void *data)
{
  FData* fd = (FData*) data;

  double dresult=evalPredicate(fd->pF, -1, n, x);

  if (grad) {
    // Only needed if choosen algorithm demands it.
    for (int i=0; i < n; i++) {
      grad[i] = evalPredicate(fd->pD, i, n, x); 
    }
  }
  return dresult;
}

/*
The constraint function form within nlopt is always c(x)<=0
  
Max memory is limited by the maximum memory value within a shelf.

A shelf decriptons contains within the data vector:
- The first element that is amount of memory that can be assigned to a shelf.
- The next n elements contains either a 0 or a 1.
	- 0 means: This operator is not of any interest here. But the operator
							MUST exists with a 1 in another data vector.
	- 1 means: This operator is within this shelf and needs memory.
  
*/
double maxMemoryConstraint(unsigned n, const double *x, double *grad,
    void *dataptr) {
  std::vector<int> data = *(std::vector<int>*) dataptr;
  int i=0;
  if (grad) {
    for(i=0; i<n; i++) {
			if (data[i+1] > 0)
      	grad[i] = 1;
			else
      	grad[i] = 0;
    }
  }
	//count << "Shelf-Memory: " << data[0] << endl;
  int rc=-1 * data[0]; // Contains the max memory for this shelf
  for(i=0; i<n; i++) {
		if (data[i+1] > 0)
    	rc=rc+x[i];
  }
  return rc;
}


/*
This could be done in the same way as with the objective and derivate functions,
with prolog predicates, but this functions stays that simple, even if the 
other both metioned functions will be more complex.

Limits the memory assignments for one operator to it's sufficientMemory value.

*/
double sufficientMemoryConstraint(unsigned n, const double *x, double *grad, 
    void *data) {
  OpMemoryConstraint* d = (OpMemoryConstraint*) data;
  double suffMem = d->sufficientMemory;
  int dim=d->dimension;
  if (grad) {
    for(int i=0; i<n; i++) {
      grad[i] = 0;
    }
    grad[dim] = 1;
  }
  double result= -suffMem + x[dim];
  return result;
}

OptimizeResult minimize(int minOpMemory, int maxMemoryInMiB, 
		std::vector< std::vector<int> > vConstraints, int dimension, 
    std::vector<int> vSufficientMemoryInMiB) {
  /*
    Check out 
    http://ab-initio.mit.edu/wiki/index.php/NLopt_Algorithms
    for information about the different implemented algorithm.
    You can switch to a locale minimum alogorithm if we
    can assure that a local minimum is a global minium.
    For cost functions this is the case and we demand it here.
    More precicly, only continuously, decreasing functions are allowed to be
    memory depend cost functions are allowed. Depending on the choosen 
    algorithm, the fuctions must be differentiable, and even more, the 
    derive must be provided.
  
  nlopt::opt opt(nlopt::LN_COBYLA, dimension);
  */
  nlopt::opt opt(nlopt::LD_MMA, dimension);

  /*
	The lower and upper bounds are the search area.
  The minimum is the minimum amount of memory that should be assigned to 
	an operator.
  */
  int lowerBound = minOpMemory;
  std::vector<double> lb(dimension);
  for(int i=0;i<dimension;i++) {
    lb[i]=lowerBound; // minimum memory per operator
  }
  opt.set_lower_bounds(lb);
  
  /*
  The maximum amount of memory that can assigned to an operator 
  is the global memory property from the secondo config file.
  Note that this is guranteed by the linear constraint function, this 
	is just to limit the search area.
  */
  std::vector<double> ub(dimension);
  for(int i=0;i<dimension;i++) {
    lb[i]=maxMemoryInMiB; // maximum memory per operator
  }
  opt.set_upper_bounds(lb);

  // Predicate references 
  FData fd;
  fd.pF = PL_predicate("objectiveFunction", 2, "optopmem");
  fd.pD = PL_predicate("derivativeFunction", 3, "optopmem");

	// Setup the objective funtion
  opt.set_min_objective(objectiveFunction, &fd);

  OptimizeResult oResult;
  if (maxMemoryInMiB < minOpMemory) {
    cout << "Error: maxMemoryInMiB can't be below minOpMemory. Abort." << endl;
    oResult.rc=-1;
    return oResult;
  }

  // The precision is algorithm dependent.
  // Because the result is round up, a small percision should be good enough.
  // But still note that a precision with 1 MiB means not that the result
  // will differ not more that 1 MiB from the optimal result. It still
  // depends on the algorithm how the precision is handeld.
  double precision=0.25d; // 1e-8
  double precisionConstr=0.1d; // constraint precision

  // Ensure the total amount of memory within a shelf won't be crossed.
  for(int i=0;i<vConstraints.size();i++) {
    opt.add_inequality_constraint(maxMemoryConstraint, &vConstraints[i],
      precisionConstr);
  }

  // Limit the operators assigned memory to the sufficient memory.
	// Because after this sufficient memory value, it is not possible to
	// realize the profit that may indicates by the cost function.
  std::vector<OpMemoryConstraint> opcdata(dimension);
  for(int i=0;i<vSufficientMemoryInMiB.size();i++) {
    int suffOpMem=vSufficientMemoryInMiB[i];
    opcdata[i].dimension=i;
    opcdata[i].sufficientMemory=suffOpMem;
    // if there is no room for double optimization, this would result in 
    // long or even endless search for a valid result, so give the algortihm 
    // some room where the algorithm can find a solution.
    //cout << "sufficientMemory:" << opcdata[i].sufficientMemory;
    if (opcdata[i].sufficientMemory <= lowerBound+0.9d) {
    	//cout << "Modify sufficientMemory:" << opcdata[i].sufficientMemory;
      opcdata[i].sufficientMemory=lowerBound+0.9d;
    	//cout << " to:" << opcdata[i].sufficientMemory << endl;
		}

    opt.add_inequality_constraint(sufficientMemoryConstraint, &opcdata[i],
      precisionConstr);
  }

  // Stopping criteria
 	//opt.set_xtol_rel(precision);
	//opt.set_ftol_abs(0.00001d);
  opt.set_xtol_abs(precision);

  // Set the initial guess. This could be improved, of course
  std::vector<double> x(dimension);
  for(int i=0;i<dimension;i++) {
    x[i]=minOpMemory;
  }

  double minf;
  nlopt::result result = opt.optimize(x, minf);
  oResult.x=x;
  oResult.rc=0;
  return oResult;
}

/*

Does some simple checks and transforms between prolog terms and the c values.

*/
foreign_t pl_memoryOptimization(term_t t_minOpMemory, term_t t_maxMemory,
    term_t t_constraints, term_t t_dimension, term_t sufficientMemory, 
		term_t memoryAssignments) {

  int memoryMiB, dimension, minOpMemoryMiB;  
  if (!PL_get_integer(t_minOpMemory, &minOpMemoryMiB)) {
    cerr << "min operator memory parameter is not an integer" << endl;
    PL_fail;
  }
  if (!PL_get_integer(t_maxMemory, &memoryMiB)) {
    cerr << "max memory parameter is not an integer" << endl;
    PL_fail;
  }
  if (!PL_get_integer(t_dimension, &dimension)) {
    cerr << "dimension parameter is not an integer" << endl;
    PL_fail;
  }

  if (!PL_is_list(t_constraints)) {
    cerr << "The constraints parameter is not a list. Abort." << endl;
    PL_fail;
  }

	// t_constraint is a list of lists, refer to the ma.pl.
  int constraints=0; // Number of max memory constraints
  term_t l = PL_copy_term_ref(t_constraints);
  term_t h = PL_new_term_ref();
  while(PL_get_list(l, h, l)) {
  	constraints++;
	}

  std::vector< std::vector<int> > vConstraints(constraints, 
		vector<int>(dimension+1));

  l = PL_copy_term_ref(t_constraints);
  h = PL_new_term_ref();
  int i=0;
  while(PL_get_list(l, h, l)) {
		int c=0;
  	term_t hi = PL_new_term_ref();
  	term_t li = PL_copy_term_ref(h);
  	while(PL_get_list(li, hi, li)) {
    	int intValue;
    	PL_get_integer(hi, &intValue);
    	vConstraints[i][c] = intValue;
			c++;
		}
  	if (c!=dimension+1) {
    	cerr << "constraint parameter don't contain " << dimension+1
      	<< " values. Abort." << endl;
    	PL_fail;
  	}
    i++;
  }
  if (i==0) {
		// otherwiese the amount of memory is unristricted.
   	cerr << "Error: No constraint list provided. ";
		cerr << "At least one constraint list is needed. " << endl;
   	PL_fail;
 	}

  if (!PL_is_list(sufficientMemory)) {
    cerr << "sufficientMemory parameter is not a list. Abort." << endl;
    PL_fail;
  }

  std::vector<int> vSufficientMemoryInMiB(dimension);
  l = PL_copy_term_ref(sufficientMemory);
  h = PL_new_term_ref();
  i=0;
  while(PL_get_list(l, h, l)) {
    int intValue;
    PL_get_integer(h, &intValue);
    vSufficientMemoryInMiB[i] = intValue; 
    i++;
  }
  if (i!=dimension) {
    cerr << "sufficientMemory parameter don't contain " << dimension 
      << " values. Abort." << endl;
    PL_fail;
  }

  OptimizeResult oResult;
  try {
    oResult=minimize(minOpMemoryMiB, memoryMiB, vConstraints, dimension, 
      vSufficientMemoryInMiB);
  }
  catch(std::exception &e) {
    cerr << "Optimize failed due to the exception: " << e.what() << endl;
    PL_fail;
  }

  if (oResult.rc!=0) {
    cerr << "Optimize failed." << endl;
    PL_fail;
  }

  term_t result = PL_new_term_ref();
  PL_put_nil(result);
  for(int i=0;i<dimension;i++) {
    term_t elem = PL_new_term_ref();
    // => The list is built tail-to-head!
    PL_put_float(elem, oResult.x[dimension-(i+1)]);
    PL_cons_list(result, elem, result);
  }
  if (!PL_unify(memoryAssignments, result)) {
    cerr << "Result unification failed." << endl;
    PL_fail;
  }
  PL_succeed;
}

// eof
