/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Database generator for RTrees

[TOC]

0 Overview

*/

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
  // number of tuples for each relation
  const int size_intRel     = 100;
  const int size_realRel    = 100;
  const int size_boolRel    = 100;
  const int size_stringRel  = 100;
  const int size_pointRel   = 100;
  const int size_pointsRel  = 100;
  const int size_lineRel    = 100;
  const int size_regionRel  = 100;
  const int size_slineRel   = 100;
  const int size_ipointRel  = 100;
  const int size_upointRel  = 100;
  const int size_mpointRel  = 100;
  const int size_rect2Rel   = 100;
  const int size_rect3Rel   = 100;
  const int size_rect4Rel   = 100;
  const int size_rect8Rel   = 10;

  // general modulo const
  const int n = 10;
  
  // offset for right/top, outercircle/holecircle, etc.
  const int offset = 10;

  ofstream output("DB_RTreeTest");
  output << "(DATABASE RTREETEST (TYPES)(OBJECTS" << endl;

  // integer relation
  output << "(OBJECT intRel()(rel(tuple((I int))))(";
  for (int i = 1; i <= size_intRel; i++)
  {
   output << endl;
   output << "( " << (int)rand() << " )";
  }
  output << "))" << endl;

  // floating point relation
  output << "(OBJECT realRel()(rel(tuple((R real))))(";
  for (int i = 1; i <= size_realRel; i++)
  {
   output << endl;
   output << "( " << (float)rand() << " )";
  }
  output << "))" << endl;

  // boolean relation
  output << "(OBJECT boolRel()(rel(tuple((B bool))))(";
  string boolValue;
  for (int i = 1; i <= size_boolRel; i++)
  {
   if ((int)rand()%2 == 1) boolValue = "TRUE";
   else boolValue = "FALSE";
   output << endl;
   output << "( " << boolValue << " )";
  }
  output << "))" << endl;

  // string relation
  output << "(OBJECT stringRel()(rel(tuple((S string))))(";
  for (int i = 1; i <= size_stringRel; i++)
  {
   output << endl;
   output << "( \"" << (char)(65+(int)rand()%26) << (char)(65+(int)rand()%26);
   output <<           (char)(65+(int)rand()%26) << (char)(65+(int)rand()%26);
   output <<           (char)(65+(int)rand()%26) << (char)(65+(int)rand()%26);
   output <<           (char)(65+(int)rand()%26) << (char)(65+(int)rand()%26);
   output <<           (char)(65+(int)rand()%26) << (char)(65+(int)rand()%26);
   output << "\" )";
  }
  output << "))" << endl;

  // point relation
  output << "(OBJECT pointRel()(rel(tuple((P point))))(";
  for (int i = 1; i <= size_pointRel; i++)
  {
   output << endl;
   output << "(( " << (int)rand()%n << "." << (int)rand()%n << " ";
   output << (int)rand()%n << "." << (int)rand()%n << " ))";
  }
  output << "))" << endl;

  // points relation
  output << "(OBJECT pointsRel()(rel(tuple((Ps points))))(";
  for (int i = 1; i <= size_pointsRel; i++)
  {
   output << endl << "(( ";
   for (int k = 1; k <= 5; k++)
   {
    output << endl;
    output << "   ( " << (int)rand()%n << "." << (int)rand()%n << " ";
    output <<            (int)rand()%n << "." << (int)rand()%n << " )";
   }
   output << " ))";
  }
  output << "))" << endl;

  // line relation
  output << "(OBJECT lineRel()(rel(tuple((L line))))(";
  for (int i = 1; i <= size_lineRel; i++)
  {
   output << endl;
   output << "((( " << (int)rand()%n << "." << (int)rand()%n << " ";
   output <<           (int)rand()%n << "." << (int)rand()%n << " ";
   output <<           (int)rand()%n << "." << (int)rand()%n << " ";
   output <<           (int)rand()%n << "." << (int)rand()%n << " )))";
  }
  output << "))" << endl;

  // region relation
  output << "(OBJECT regionRel()(rel(tuple((Rg region))))(";
  for (int i = 1; i <= size_regionRel; i++)
  {
   output << endl;
   output << "(((( ";
   output << "(" << offset+(int)rand()%offset << "." << (int)rand()%n;
   output << " " <<        (int)rand()%offset << "." << (int)rand()%n << ") ";
   output << "(" << offset+(int)rand()%offset << "." << (int)rand()%n;
   output << " " <<        (int)rand()%offset << "." << (int)rand()%n << ") ";
   output << "(" << offset+(int)rand()%offset << "." << (int)rand()%n;
   output << " " <<        (int)rand()%offset << "." << (int)rand()%n << ") ";
   output << "(" << offset+(int)rand()%offset << "." << (int)rand()%n;
   output << " " <<        (int)rand()%offset << "." << (int)rand()%n << ") ";
   output << "))))";
  }
  output << "))" << endl;

  // sline relation
  output << "(OBJECT slineRel()(rel(tuple((Sl sline))))(";
  for (int i = 1; i <= size_slineRel; i++)
  {
   output << endl;
   output << "((( ";
   output << (int)rand()%offset << "." << (int)rand()%n << " ";
   output << (int)rand()%offset << "." << (int)rand()%n << " ";
   output << (int)rand()%offset << "." << (int)rand()%n << " ";
   output << (int)rand()%offset << "." << (int)rand()%n << " ";
   output << ")))";
  }
  output << "))" << endl;

  // ipoint relation
  output << "(OBJECT ipointRel()(rel(tuple((Ip ipoint))))(";
  for (int i = 1; i <= size_ipointRel; i++)
  {
   output << endl;
   output << "(( ";
   output << "\"2010-02-13\"" ;
   output << "(" << (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<        (int)rand()%offset << "." << (int)rand()%n << ")";
   output << "))";
  }
  output << "))" << endl;

  // upoint relation
  output << "(OBJECT upointRel()(rel(tuple((Up upoint))))(";
  for (int i = 1; i <= size_upointRel; i++)
  {
   output << endl;
   output << "(( ";
   output << "(\"2010-01-01\" \"2010-02-15\" TRUE TRUE)" ;
   output << "(" << (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<        (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<        (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<        (int)rand()%offset << "." << (int)rand()%n << ")";
   output << "))";
  }
  output << "))" << endl;

  // mpoint relation
  output << "(OBJECT mpointRel()(rel(tuple((Mp mpoint))))(";
  for (int i = 1; i <= size_mpointRel; i++)
  {
   output << endl;
   output << "(( ";
   output << "((\"2010-01-01\" \"2010-02-15\" TRUE TRUE)";
   output << "(" << (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<        (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<        (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<        (int)rand()%offset << "." << (int)rand()%n << "))" << endl;
   output << "))";
  }
  output << "))" << endl;

  // rectangle(2D) relation
  output << "(OBJECT rect2Rel()(rel(tuple((R2 rect))))(";
  for (int i = 1; i <= size_rect2Rel; i++)
  {
   output << endl;
   output << "(( " << (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ))";
  }
  output << "))" << endl;

  // rectangle(3D) relation
  output << "(OBJECT rect3Rel()(rel(tuple((R3 rect3))))(";
  for (int i = 1; i <= size_rect3Rel; i++)
  {
   output << endl;
   output << "(( " << (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ))";
  }
  output << "))" << endl;

  // rectangle(4D) relation
  output << "(OBJECT rect4Rel()(rel(tuple((R4 rect4))))(";
  for (int i = 1; i <= size_rect4Rel; i++)
  {
   output << endl;
   output << "(( " << (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ))";
  }
  output << "))" << endl;

  // rectangle(8D) relation
  output << "(OBJECT rect8Rel()(rel(tuple((R8 rect8))))(";
  for (int i = 1; i <= size_rect8Rel; i++)
  {
   output << endl;
   output << "(( " << (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ";
   output <<          (int)rand()%offset << "." << (int)rand()%n << " ";
   output << offset + (int)rand()%offset << "." << (int)rand()%n << " ))";
  }
  output << "))" << endl;

  // RTrees -------------------------------------------------------------------
  output << "(OBJECT SEC_DERIVED_OBJ()"<< endl;
  output << "(rel(tuple((Name string)(Value text)(UsedObjs text))))(";

  // RTree point
  output << endl << "(\"rtreePoint\" <text>(creatertree pointRel P)";
  output << "</text---> <text>( pointRel )</text--->)";

  // RTree points
  output << endl << "(\"rtreePoints\" <text>(creatertree pointsRel Ps)";
  output << "</text---> <text>( pointsRel )</text--->)";

  // RTree line
  output << endl << "(\"rtreeLine\" <text>(creatertree lineRel L)";
  output << "</text---> <text>( lineRel )</text--->)";

  // RTree region
  output << endl << "(\"rtreeRegion\" <text>(creatertree regionRel Rg)";
  output << "</text---> <text>( regionRel )</text--->)";

  // RTree sline
  output << endl << "(\"rtreeSLine\" <text>(creatertree slineRel Sl)";
  output << "</text---> <text>( slineRel )</text--->)";

  // RTree upoint
  output << endl << "(\"rtreeUPoint\" <text>(creatertree upointRel Up)";
  output << "</text---> <text>( upointRel )</text--->)";

  // RTree rect 2D
  output << endl << "(\"rtreeRect2\" <text>(creatertree rect2Rel R2)";
  output << "</text---> <text>( rect2Rel )</text--->)";

  // RTree rect 3D
  output << endl << "(\"rtreeRect3\" <text>(creatertree rect3Rel R3)";
  output << "</text---> <text>( rect3Rel )</text--->)";

  // RTree rect 4D
  output << endl << "(\"rtreeRect4\" <text>(creatertree rect4Rel R4)";
  output << "</text---> <text>( rect4Rel )</text--->)";

  // RTree rect 8D
  output << endl << "(\"rtreeRect8\" <text>(creatertree rect8Rel R8)";
  output << "</text---> <text>( rect8Rel )</text--->)";

  output << "))" << endl;

  // End of database
  output << "))" << endl;
}
