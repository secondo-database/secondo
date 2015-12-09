/*

Nov. 2008 M. Spiekermann   

The classes GMatrix and GVector were introduced in order to
provide a more readable programming interface for GSL types and
operations. In its initial version the supported features are
very limited since I only need a cholesky decomposition in order
to provide the generation of random sequences for 
multivariate normal distribution  


*/

#ifndef SEC_GSL_LINALG_H
#define SEC_GSL_LINALG_H

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

#include <iostream>
#include <fstream>
#include <iomanip>

#include <assert.h>


class GMatrix;

class GVector {

  public: 

  GVector(size_t size, double val = 0.0 )
  {
    v = gsl_vector_alloc(size);
    gsl_vector_set_all(v, val);    
    refs = 1;
  }	  

  inline GVector(const GVector& rhs) : v(rhs.v) { refs++; }


  inline GVector& operator=(const GVector& rhs)
  { 
     v = rhs.v;	  
     refs++; 
     return *this; 
  }

  ~GVector() 
  {
    refs--;
    if (refs == 0) {   
      gsl_vector_free(v);
    }       
  }	  


  inline GVector operator*(const GMatrix& m) const;
  

  inline const double& operator[] (size_t i) const 
  {
    return v->data[i * v->stride];
  }

  inline double& operator[] (size_t i) 
  {
    return v->data[i * v->stride];
  }

  inline size_t getDim() const { return v->size; } 
 

  inline void put(ostream& os, const string& sep) const
  {	  
  ios_base::fmtflags oldflags = os.flags();
  os << setprecision(2) << fixed;  
  size_t i = 0;
  for ( ;i < getDim()-1; i++) { 
      os << (*this)[i] << sep; 	    
  }
  os << (*this)[i];
  os.flags(oldflags);
  }

  private:

  gsl_vector* v;
  size_t refs;

};	


class GMatrix {

  public:	
  GMatrix(size_t rows, size_t cols, double val = 0.0)
  {	  
    m = gsl_matrix_alloc(rows, cols);
    gsl_matrix_set_all(m, val);
  }	  

  ~GMatrix() 
  {
    gsl_matrix_free(m);
  }    

  inline const double* operator[] (size_t i) const 
  {
    return &(m->data[i * m->tda]);
  }

  inline double* operator[] (size_t i) 
  {
    return &(m->data[i * m->tda]);
  }

  inline GVector operator*(const GVector& v) const
  {
    assert( v.getDim() == getCols() );
    GVector r( v.getDim() );

    for(int i = 0; i < getRows(); i++) {
      for(int j = 0; j < getCols(); j++) {	  	      
        r[i] += (*this)[i][j] * v[j];
      }  
    }

    return r;  
  }	  

  inline size_t getRows() const { return m->size1; }
  inline size_t getCols() const { return m->size2; }

 
  void setDiag(double val) 
  {
    for(int i = 0; i < min(getRows(), getCols()); i++) {
      (*this)[i][i] = val;	      
    }  
  }	  

  void topRight_to_lowerLeft()
  {
    size_t k = min( getRows(), getCols() );

    for(int i = 0; i < k; i++) {
      for(int j = i+1; j < k; j++) { 	    
        (*this)[j][i] = (*this)[i][j];	      
      }
    }   
  }	  

  void set_topRight(double val = 0.0, const bool topRight = true)
  {
    size_t k = min( getRows(), getCols() );

    for(int i = 0; i < k; i++) {
      for(int j = i+1; j < k; j++) { 	    
	if (topRight) {      
          (*this)[i][j] = val;
        } else {
          (*this)[j][i] = val;
        }		
      }
    }   
  }	  

  void set_lowerLeft(double val = 0.0)
  {
     set_topRight(val, false);	  
  }	  

  void doCholeskyDecomposition() {

    // to do error handling !!!	  
    gsl_linalg_cholesky_decomp(m);
  }	  
  

  private:
  
  gsl_matrix* m;

};	


  inline GVector GVector::operator*(const GMatrix& m) const
  {
    assert( getDim() == m.getRows() );
    GVector r( m.getRows() );

    for(int i = 0; i < getDim(); i++) {
      for(int j = 0; j < m.getRows(); j++) {
        //cout << (*this)[j] << " * " << m[j][i] 
	//<< " = "  << (*this)[j] * m[j][i] << endl;	    
        r[i] += (*this)[j] * m[j][i];
      }  
    }

    return r;  
  }	



ostream& operator<<(ostream& os, const GMatrix& m)
{
  ios_base::fmtflags oldflags = os.flags();
  os << setprecision(2) << fixed;  
  for (size_t i = 0; i < m.getRows(); i++) { 
    for (size_t j = 0; j < m.getCols(); j++) {	    
      os << m[i][j] << " "; 	    
    }
    os << endl;
  } 
  os.flags(oldflags);
  return os; 
}	

ostream& operator<<(ostream& os, const GVector& v)
{
  os << "( ";	
  v.put(os, ", ");
  os << " )" << endl;
  return os; 
}	



#endif


