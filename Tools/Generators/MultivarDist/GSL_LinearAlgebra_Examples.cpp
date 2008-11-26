/*
Initial Version

*/   

#include "GSL_LinearAlgebra.h"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <iostream>

using namespace std;

class GaussDist {

  public:
    GaussDist( double stddev, double mean = 0.0) : sigma(stddev), mu(mean) 
    {		  
       //cout << "mu =" << mu << endl;
       //cout << "sigma =" << sigma << endl;
     
       gsl_rng_env_setup();
       const gsl_rng_type * T = gsl_rng_default;
       r = gsl_rng_alloc (T);
    }

    ~GaussDist() {
       
        gsl_rng_free (r);
	r = 0;
    }	    


    inline double nextVal() 
    {
      return  mu + gsl_ran_gaussian (r, sigma);
    }  

   private:

    double sigma;
    double mu;

    gsl_rng * r;
};	


void gaussTest(void)
{
  GaussDist g(2.0, 3.0);	
  for (int i = 0; i < 100; i++) 
  {
   cout << g.nextVal() << endl;
  }
  cout << endl;
}

void multVarTest(void)
{
  GaussDist g1(2.0, 10.0);	
  GaussDist g2(2.0, 20.0);	
  GaussDist g3(2.0, 30.0);	

  GMatrix c(3, 3);

  c.setDiag(1);

  c[0][1] = 0.9;
  c[0][2] = 0.8;
  c[1][2] = 0.7;

  cout << "Correlation Matrix C" << endl;
  cout << c << endl;

  c.topRight_to_lowerLeft();
  c.doCholeskyDecomposition();
  c.set_lowerLeft(0.0);

  cout << " Cholesky Decomp. computing U with C = U * transposed(U):" << endl;
  cout << c << endl;


  ofstream of;
  of.open("multvar.csv");

  for (int i = 0; i < 100; i++) 
  {
   GVector v(3);
   v[0] = g1.nextVal();
   v[1] = g2.nextVal();
   v[2] = g3.nextVal();

   GVector r = v * c;

   r.put(of, ", ");
   of << endl;

  } 
  of.close();
}






int main(int argc, char* argV) 
{

  //gaussTest();

  multVarTest();

  return 0;

  GVector v1(3);
  GVector v2(2);

  v1[0] = 1;
  v1[1] = 2;
  v1[2] = 3;

  cout << "Vector v1 = " << v1 << endl;
  cout << "Vector v2 = " << v2 << endl;


  GMatrix gm(3, 3);

  gm.setDiag(1);

  gm[0][1] = 0.6;
  gm[0][2] = 0.3;
  gm[1][2] = 0.5;

  gm.topRight_to_lowerLeft();

  cout << "A positive definite Matrix gm:" << endl;
  cout << "#Rows = " << gm.getRows() << endl;
  cout << "#Cols = " << gm.getCols() << endl;


  cout << gm << endl;

  gm.doCholeskyDecomposition();


  cout << "The Cholesky Decomposition of gm:" << endl;
  cout << gm << endl;


  GVector v3 = gm * v1;
  cout << "gm * v1 = " << v3 << endl;

  GVector v4(3);
  
  v4[0] = -0.3999;
  v4[1] = -1.6041;
  v4[2] = -1.0106;

  cout << "Vector v4 = " << v4 << endl; 

  gm.set_lowerLeft(0.0);
  cout << gm << endl;

  GVector v5 = v4 * gm;
  cout << "gm * v4 = " << v5 << endl;



}	

