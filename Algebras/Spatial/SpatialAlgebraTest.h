/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Header File of the Spatial Algebra Test


January, 2005 Leonardo Guerreiro Azevedo

1 Overview

*/

#ifndef __SPATIAL_ALGEBRA_TESTE_H__
#define __SPATIAL_ALGEBRA_TESTE_H__


#include "SpatialAlgebra.h"
#include "RectangleAlgebra.h"
#include <vector>
#include <iostream>
#include <string>

namespace PartnerNoTest
{
  bool MainPartnerNoTest(ostringstream &testResult);
  bool PartnerNoTest1(ostringstream &testResult,CRegion  *r,int resultOK[] );
}

namespace InsideAboveTest
{
  bool InsideAboveTest1(ostringstream &testResult);
  bool InsideAboveTest1(ostringstream &testResult,CRegion *r,bool resultOK[], int N_HALF_SEGMENTS );
}

namespace WindowClippingIn
{
  bool ChsClippingInTest(ostringstream &testResult);
  bool ChsClippingInTest(ostringstream &testResult,CLine *testLine,
                                        Rectangle *window, CLine *clippedLine, bool acceptedLines[]);
}

namespace LineClippingTest
{
  bool LineClippingInTest(ostringstream &testResult);
  bool LineClippingInTest(ostringstream &testResult,CLine *testLine,
                                        Rectangle *window, CLine *rClippedLine);
  bool LineClippingOutTest(ostringstream &testResult);
  bool LineClippingOutTest(ostringstream &testResult,CLine *testLine,
                                        Rectangle *window, CLine *rClippedLine);
}

namespace RegionClippingTest
{
  bool MainRegionClippingInTest(ostringstream &testResult);
  bool VectorSizeTest(ostringstream &testResult);
  bool AngleTest(ostringstream &testResult);

  bool GetClippedHSTest(ostringstream &testResult,CRegion *testRegion,
                                        Rectangle *window, CLine *clippedRegion);

  bool MainCreateNewSegmentsTest(ostringstream &testResult);
  bool CreateNewSegmentsHTest(ostringstream &testResult,double edge,string strEdge,WindowEdge wEdge);
  bool CreateNewSegmentsVTest(ostringstream &testResult,double edge,string strEdge, WindowEdge wEdge);

  bool MainCreateNewSegmentsWindowVertices(ostringstream &testResult);
  bool MainGetDPointTest(ostringstream &testResult);
  bool GetDPointTest(Coord pX, Coord pY, bool insideAbove, Coord p2X, Coord p2Y,
                                        Coord vX, Coord vY, bool rDirection);
  bool MainComputeCycleTest(ostringstream &testResult);
  bool ComputeCycleTest(string strCR,vector<int> v);
  bool MainComputeRegionTest(ostringstream &testResult);
  bool ComputeRegionTest(string strCR);
  bool MainWindowClippingIn(ostringstream &testResult);
  bool MainWindowClippingOut(ostringstream &testResult);

}

#endif //__SPATIAL_ALGEBRA_TESTE_H__
