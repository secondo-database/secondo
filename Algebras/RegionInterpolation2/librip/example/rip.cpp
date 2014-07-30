/*
  1 rip.cpp: Example application which uses the librip to calculate
    the interpolation of two regions in textual nested list
    representation.

*/

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <stdio.h>
#include <librip.h>

using namespace std;

#define NUM 257
static double num;

#define SECONDO_STYLE // Output suitable for import into SECONDO

// Get the next token from input file (simple lexer)
static int getToken (FILE *f) {
   char buf[1024], *ptr = buf;
   
   do {
      int ch = fgetc(f);
      switch (ch) {
       case ')':
       case '(':
	 if (buf != ptr) {
	    ungetc(ch, f);
	    num = atof(buf);
	    return NUM;
	 }
	 return ch;
	 
       case '-':
       case '0':
       case '1':
       case '2':
       case '3':
       case '4':
       case '5':
       case '6':
       case '7':
       case '8':
       case '9':
       case '.':
	 *ptr++ = ch;
	 *ptr = '\0';
	 break;
	 
       case ' ':
       case '\r':
       case '\n':
       case '\t':
	 if (buf != ptr) {
	    num = atof(buf);
	    return NUM;
	 }
	 break;
	 
       case EOF:
	 return 0;
	 
       default:
	 printf("Parse error: '%c'\n", ch);
	 exit(EXIT_FAILURE);
      }
   } while (!feof(f));
   
   return 0;
}

// Parse the textual nested list and create a 
// RList-object from it (a simple parser).
static void parse (FILE *f, RList *nl) {
   do {
      int token = getToken(f);
      switch (token) {
       case '(':
	 parse(f, nl->nest());
	 break;
       case NUM:
	 nl->append(num);
	 break;
       case ')':
	 return;
	 
      }
   } while (1);
}

// Parse the given file with the textual representation
// of a nested list of a region and return a corresponding
// RList object
static RList* parseFile (const char *filename) {
   FILE *f;
   
   RList *ret = new RList();
   
   f = fopen(filename, "r");
   if (getToken(f) != '(') {
      printf("Parse error! (Beginning)\n");
      exit(EXIT_FAILURE);
   } else {
      parse(f, ret);
      if (getToken(f) != 0) {
	 printf("Parse error! (End)\n");
	 exit(EXIT_FAILURE);
      }
   }
   
   fclose(f);
   
   return ret;
}


// Program entry point
int main (int argc, char **argv) {
   const char *arg = "distance";

   if (argc < 5) {
       cerr << "Usage:   rip <sregfile> <dregfile> <srctime> <dsttime> [args]\n"
            << "          timeformat:  YYYY-MM-DD hh:mm:ss\n"
            << "Example: rip box1 box2 \"2014-01-01 00:00\" "
            << "\"2014-01-02 12:00\" Overlap:30\n\n";
      exit(1);
   }
   if (argc == 6)
     arg = argv[5];
   RList *sreg = parseFile(argv[1]);
   RList *dreg = parseFile(argv[2]);
   
   RList ret = regioninterpolate(*sreg, *dreg, argv[3], argv[4], arg);
   
   stringstream ss;
#ifdef SECONDO_STYLE
   ss << "(OBJECT obj () mregion ";
#endif
   ss << ret.ToString() << "\n";
#ifdef SECONDO_STYLE
   ss << ")\n";
#endif

   cout << ss.str();
   
   exit(0);
}

