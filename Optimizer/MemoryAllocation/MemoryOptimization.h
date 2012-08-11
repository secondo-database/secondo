/*

$Header$
@author Nikolai van Kempen

*/

#include "SWI-Prolog.h"

using namespace std;

foreign_t pl_memoryOptimization(term_t t_minOpMemory, term_t t_maxMemory,
  term_t t_dimension, term_t sufficientMemory, term_t memoryAssignments);

