/*
1 Progress.cpp

This little prrogram counts the lines coming from stdin.
After a number of lines (given as argument or 1000 as default),
a dot is written to stdout. This can be useful for the user while
processing large text files. The standard application will be in a
pipe together with the tee command.

*/

#include <iostream>
using namespace std;

int main(int argc , char* argv[]){
   long number = 0;
   if(argc==2)
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

