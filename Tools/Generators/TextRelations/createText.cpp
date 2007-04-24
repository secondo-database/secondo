/*
April 2007, M. Spiekermann

*/   

#include <iostream>

using namespace std;

int main(int argc, char* argv[]){

 if (argc != 4 ) {
   cerr << "Please specify the number of tuples, " 
	<< "the flob size, and the object name!" << endl << endl
        << "  Example: "
        << argv[0] << " 50 200 text50" << endl << endl;
   exit(1);
 }

 const int n = atoi(argv[1]);
 const int l = atoi(argv[2]);


 cout << "(OBJECT " <<  argv[3] << endl 
      << "  () " << endl
      << "  (rel(tuple( (Nr int) (Len int) (Flob text) ))) (" << endl;

 for (int i = 0; i < n ; i++) {
   
   string text(l, 'x');

   cout << "  (" << i << " " 
	         << l << " " 
	         <<  "<text>" << text << "</text--->)" << endl;
 }	 
 cout << "))" << endl;

}	
