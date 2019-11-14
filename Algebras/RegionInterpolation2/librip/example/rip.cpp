/*
  1 rip.cpp: Example application which uses the librip to calculate
    the interpolation of two regions in textual nested list
    representation.

*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <librip.h>

using namespace std;

static RList file2rlist(char *fname) {
   ifstream i1(fname, std::fstream::in);
   RList ret = RList::parse(i1);

   return ret;
}

// Program entry point
int main (int argc, char **argv) {
   const char *arg = "distance";

   if (argc < 5) {
    cerr << "Usage:   rip <regfile> <time> <regfile> <time> ... [args]\n"
         << "          timeformat:  YYYY-MM-DD hh:mm:ss\n"
         << "Example: rip box1 \"2014-01-01 00:00\" "
         << "box2 \"2014-01-02 12:00\" box3 \"2014-01-03 11:00\" Overlap:30\n";
         << "\n"
      exit(1);
   }
   int nrregs = (argc - 1) / 2;

   if (argc%2==0) {
	   arg = argv[argc-1];
   }

   RList ret;
   for (int i = 0; i < nrregs-1; i++) {
	   RList sreg = file2rlist(argv[i*2+1]);
	   RList dreg = file2rlist(argv[i*2+3]);
	   char *siv = argv[i*2+2];
	   char *div = argv[i*2+4];
       RList interp;
       if (sreg.items[0].getType() == NL_SYM) {
	       interp = regioninterpolate(sreg.items[4], dreg.items[4],
			       siv, div, arg);
       } else {
	       interp = regioninterpolate(sreg, dreg, siv, div, arg);
       }

	   ret.append(interp.items[0]);

   }
   
   cout << ret.obj("mregion", "mregion").ToString() << "\n";
   
   exit(0);
}

