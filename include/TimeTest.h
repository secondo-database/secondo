/*

Small class for time measurement. the class uses static functions
which return the elpased time since the last call formatted in
a string.

November 2002 M. Spiekermann

*/

#ifndef TIMETEST_H
#define TIMETEST_H

class TimeTest {

   private:
     TimeTest() {};
     ~TimeTest() {};
     
   public:
     // return difference in real time since last call 
     static const string diffReal();
     
     //  return difference in CPU time since last call
     static const string diffCPU();

};

#endif
