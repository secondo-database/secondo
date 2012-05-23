/*
October 2009, S.Jungnickel

Implementation is based on createTextRelation.cpp

This command line tool creates random text [A-Z][a-z][0-9] 
instead of static text.

*/   

#include <iostream>
#include <fstream>

#include "math.h"
#include <stdlib.h>
#include "RandomText.h"

using namespace std;

/*
Creates a relation with name ~relName~, ~a~ tuples with
constant string value in attribute ~Flob~ and ~b~ tuples
with random string value in attribute ~Flob~. The constant
string is specified in parameter ~str~.

*/

bool traceMode = false;

void createRelation(string path, string relName, int a, int b, int l,
                    const string str)
{
  int counter = 0;

  if ( traceMode )  
  {
    cout << endl
         << "relName: " << relName << endl
         << "path: " << path << endl
         << "constant tuples: " << a << endl
         << "random tuples: " << b << endl
         << "total tuples: " << (a+b) 
         << endl;
  }

  ofstream os;

  os.open(path.c_str());
 
  os << "(OBJECT " << relName << endl
     << "  () " << endl
     << "  (rel(tuple( (Nr int) (Len int) (Rval real)"
     << " (Ival int) (Flob text) ))) (" 
     << endl;

  // write tuples with constant strings
  for (int i = 0; i < a ; i++) 
  {
   
    os << "  (" << ++counter << " " 
       << l << " " 
       << (double)rand() << " " 
       << rand() << " " 
       <<  "<text>" << str << "</text--->)" << endl;
  } 

  // write tuples with random strings
  for (int i = 0; i < b ; i++) 
  {
    os << "  (" << ++counter << " " 
       << l << " " 
       << (double)rand() << " " 
       << rand() << " " 
       <<  "<text>" << randomText(l) << "</text--->)" << endl;
  }
 
  os << "))" << endl;

  os.close();
}

/*
This text relation generator produces two relations A and B with 
the following attributes

  * Nr - tuple number

  * Len - length of text attribute Flob
  
  * Rval - random real number 
  
  * Ival - random integer number

  * Flob - text attribute with random text content [a-z][A-Z][0-9]
  
Each tuple of both relations has the same size. If both relations
are joined using the attribute ~Flob~ as the join attribute the 
query has the specified selectivity. The generator supports the
following command line arguments
 
  * argv[1] tuplesize in bytes
  
  * argv[2] number of tuples in relation A
  
  * argv[3] number of tuples in relation B
  
  * argv[4] selectivity in % (0.00-1.00)
  
  * argv[5] path for output file of relation A
  
  * argv[6] path for output file of relation B
  
*/

int main(int argc, char* argv[]){

 if (argc != 7 ) 
 {
  cerr << endl
    << "Please specify the tuplesize (bytes), " 
    << "the number of tuples in relation A and relation B "
    << ", the selectivity in percent (0-1 size (bytes) " 
    << ", and the paths of the output files." 
    << endl 
    << endl
         << "  Example: "
         << argv[0] << " 256 100000 10000 0.001 ~/relA ~/relB" 
    << endl 
    << endl;
 exit(1);
 }

 const int tupleSize = atoi(argv[1]);
 const int n = atoi(argv[2]);
 const int m = atoi(argv[3]);
 const double s = atof(argv[4]);
 const string path1 = argv[5];
 const string path2 = argv[6];
 
 // calculate necessary text length
 const int l = tupleSize - 53;

 // calculate text for equal tuples 
 const string constant = randomText(l);
  
 // calculate number of equal tuples for relation A
 int n1 = (int)floor( (double)n * sqrt(s) );
 int n2 = n - n1;

 // calculate number of equal tuples for relation B
 int m1 = (int)floor( (double)m * sqrt(s) );
 int m2 = m - m1;
 
 // create relation A
 createRelation(path1, "relA", n1, n2, l, constant);

 // create relation B
 createRelation(path2, "relB", m1, m2, l, constant);
 
}

