/*
  Tool for displaying of progress of processing of a text file.
  The input is taken from stdin
*/

#include <iostream>
using namespace std;

int main(int argc , char* argv[]){
   long number = 0;
   if(argc=1)
     number = atol(argv[1]);
   else
     number = 1000; // default
   cout << " Progress " << endl;
   cout << "Every dot corresponds to " << number << " lines " << endl;
   long count=1;
   int maxlinelength=1024;
   char cdummy[maxlinelength];
   int idummy=maxlinelength;
   while (!cin.eof()){
      cin.getline(cdummy,idummy);
      count++;
      if( (count % number)==0){
         cout << ".";
         cout.flush();
         count = 1;
      }
   }

}

