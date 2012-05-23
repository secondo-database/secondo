/*
October 2009, S.Jungnickel

Implementation is based on createTextRelation.cpp

This command line tool creates random text [A-Z][a-z][0-9] 
instead of static text.

*/   

#include <iostream>
#include "math.h"
#include <stdlib.h>
#include "RandomText.h"

using namespace std;

int main(int argc, char* argv[]){

 if (argc != 4 ) {
   cerr << "Please specify the relation size (bytes), " 
   << "the tuple size (bytes), and the object name!" << endl << endl
        << "  Example: "
        << argv[0] << " 1024 64 text64" << endl << endl;
   exit(1);
 }

 const int relSize = atoi(argv[1]);
 const int tupleSize = atoi(argv[2]);
 
 // calculate necessary text length
 const int l = tupleSize - 53;
 
 const int tuples = (int)ceil( (double)relSize / (double)tupleSize );


 cout << "(OBJECT " <<  argv[3] << endl 
      << "  () " << endl
      << "  (rel(tuple( (Nr int) (Len int) (Rval real)"
      << " (Ival int) (Flob text) ))) (" 
      << endl;

 for (int i = 0; i < tuples ; i++) {
   
   cout << "  (" << i << " " 
           << l << " "
         << (double)rand() << " "
         << rand() << " "
        <<  "<text>" << randomText(l) << "</text--->)" << endl;
 }
 cout << "))" << endl;

}

