/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Spatial Algebra Test

January, 2005. Leonardo Guerreiro Azevedo

[TOC]

1 Overview


For more detailed information see SpatialAlgebraTest.h.

2 Defines and Includes

*/

#include "SpatialAlgebraTest.h"

extern NestedList* nl;
extern QueryProcessor* qp;


CLine* GetLine(string strLine)
{
  ListExpr listExp,errorInfo;
  bool correct;
  CLine *l;

  if ( nl->ReadFromString(strLine,listExp) )
  {
    errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
    l = (CLine*) InLine( nl->TheEmptyList(), listExp, 0, errorInfo, correct ).addr;
    if (correct)
      return l;
    else
      cout<<"Er: error on creating line: "<<strLine;

  }
  else
    cout<<"Er: nl->ReadFromString(strLine,listExp); "<<strLine;
  return NULL;
}

CRegion* GetRegion(string strRegion)
{
  ListExpr listExp,errorInfo;
  bool correct;
  CRegion *r;

  if ( nl->ReadFromString(strRegion,listExp) )
  {
    errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
    r = (CRegion*) InRegion( nl->TheEmptyList(), listExp, 0, errorInfo, correct ).addr;
    if (correct)
      return r;
    else
      cout<<"Er: error on creating line: "<<strRegion;

  }
  else
    cout<<"Er: nl->ReadFromString(strLine,listExp); "<<strRegion;
  return NULL;
}

Rectangle* GetRect(string strRect)
{
  ListExpr listExp,errorInfo;
  bool correct;
  Rectangle *r;
  if (nl->ReadFromString(strRect,listExp))
  {
  r = (Rectangle*) InRectangle( nl->TheEmptyList(), listExp, 0, errorInfo, correct ).addr;
    if (correct)
      return r;
    else
      cout<<"Er: error on creating rectangle: "<<strRect;
  }
  else
    cout<<"Er: nl->ReadFromString(strRect,listExp); "<<strRect;
  return NULL;
}
void PrintCHS(CHalfSegment &chs)
{
  cout<<"( ";
  cout<<chs.GetLP().GetX()<<" "<<chs.GetLP().GetY();
  cout<<" "<<chs.GetRP().GetX()<<" "<<chs.GetRP().GetY();
  cout<<" )";
}
void LinePrinting(CLine &l)
{
  cout<<endl<<"( ";
  for(int i=0; i < l.Size();i++)
  {
    CHalfSegment chs;
    l.Get(i,chs);
    PrintCHS(chs);
  }
  cout<<" ) "<<endl;
}

void PrintCHSRegion(CRegion &r)
{
  cout<<endl<<"No. of elements of the Region: "<<r.Size()<<endl;
  r.SelectFirst();
  for( int i=0; i < r.Size(); i++)
  {
    CHalfSegment chs;
    r.GetHs( chs );
    r.SelectNext();
    cout<<chs<<endl;
  }
}

void PrintCHSLine(CLine &l)
{
  cout<<endl<<"No. of elements of the Line: "<<l.Size()<<endl;
  l.SelectFirst();
  for( int i=0; i < l.Size(); i++)
  {
    CHalfSegment chs;
    l.GetHs( chs );
    l.SelectNext();
    cout<<chs<<endl;
  }
}

void PrintRegionAsLine(CRegion &r)
{
  cout<<endl<<"( ";
  r.SelectFirst();
  for(int i=0; i < r.Size();i++)
  {
    CHalfSegment chs;
    r.Get(i,chs);
    if (chs.GetLDP())
      PrintCHS(chs);
  }
  cout<<" ) "<<endl;
}

bool CompareLineAndRegion(CLine &l, CRegion &r)
{
  l.SelectFirst();
  r.SelectFirst();
  if (l.Size()!=r.Size())
    return false;
  for(int i=0;i<l.Size();i++)
  {
    CHalfSegment chsL, chsR;
    l.GetHs(chsL);
    r.GetHs(chsR);

    if (!AlmostEqual(chsL.GetLP(), chsR.GetLP()))
      return false;
    if (!AlmostEqual(chsL.GetRP(), chsR.GetRP()))
      return false;

    l.SelectNext();
    r.SelectNext();
  }
  return true;
}

bool CompareRegionAndRegion(CRegion &l, CRegion &r)
{
  l.SelectFirst();
  r.SelectFirst();
  if (l.Size()!=r.Size())
    return false;
  for(int i=0;i<l.Size();i++)
  {
    CHalfSegment chsL, chsR;
    AttrType attrL, attrR;
    l.GetHs(chsL);
    r.GetHs(chsR);
    attrL = chsL.GetAttr();
    attrR = chsR.GetAttr();
    cout<<endl<<"LR: "<<chsL;
    cout<<endl<<"RR: "<<chsR;

    //if ( (chsL.GetLP() != chsR.GetLP()) ||
    //     (chsL.GetRP() != chsR.GetRP()) ||
    //     (chsL.GetLDP() != chsR.GetLDP()) ||
    //     (attrL.faceno != attrR.faceno) ||
    //     (attrL.cycleno != attrR.cycleno) ||
    //     (attrL.edgeno != attrR.edgeno) ||
         //(attrL.coverageno != attrR.coverageno) ||
    //     (attrL.insideAbove != attrR.insideAbove) ||
    //     (attrL.partnerno !=attrR.partnerno))
    // return false;

    //if (!AlmostEqual(chsL.GetLP(), chsR.GetLP()))
    //  return false;
    //if (!AlmostEqual(chsL.GetRP(), chsR.GetRP()))
    //  return false;
    l.SelectNext();
    r.SelectNext();
  }
  return true;
}
void CleanRegion(CRegion &r)
{
  AttrType attr;
  r.SelectFirst();
  while(!r.EndOfHs())
  {
    attr = r.GetAttr();
    attr.edgeno=0;
    attr.faceno=0;
    attr.cycleno=0;
    r.UpdateAttr(attr);
    r.SelectNext();
  }
}
void GetFaceCycle(CRegion r,vector<int> &face,int &maxFace)
{
  r.SelectFirst();
  maxFace=0;
  for(int i=0; i<r.Size();i++)
  {
    AttrType attr;
    attr = r.GetAttr(i);
    if(face[attr.faceno]<attr.cycleno)
      face[attr.faceno]=attr.cycleno;
    if(attr.faceno>maxFace)
      maxFace = attr.faceno;
  }
}
void GetStrRegion(CRegion &r, ostringstream &s)
{
  vector<int> face(r.Size()/3,0);
  int maxFace=0;
  GetFaceCycle(r,face,maxFace);
  face.resize(maxFace+1);
  s<<"(";
  for(int f=0; f<int(face.size()); f++)
  {
    //Print face f
    s<<" (";
    for(int c=0;c<=face[f];c++)
    {
      Point pAtual;
      s<<" ( ";
      for(int i=0;i<r.Size();i++)
        for(int j=0;j<r.Size();j++)
        {
          CHalfSegment chsAux,firstCHS;
          AttrType attr;
          attr = r.GetAttr(j);
          if (attr.faceno==f && attr.cycleno==c && attr.edgeno==i)
          {
            CHalfSegment chsAux;
            r.Get(j,chsAux);
            if (chsAux.GetLDP())
            {
              if (i==0)
              {
                s<<" ("<<chsAux.GetLP().GetX()<<" "<<chsAux.GetLP().GetY()
                    <<" ) ("<<chsAux.GetRP().GetX()<<" "<<chsAux.GetRP().GetY()<<" )";
                pAtual= chsAux.GetRP();
                firstCHS=chsAux;
              }
              else
              {
                if(chsAux.GetRP()==pAtual)
                {
                  if( (chsAux.GetLP()!=firstCHS.GetLP()) &&
                      (chsAux.GetLP()!=firstCHS.GetRP()) )
                  {
                    s<<" ( "<<chsAux.GetLP().GetX()<<" "<<chsAux.GetLP().GetY()<<" ) ";
                    pAtual = chsAux.GetLP();
                  }
                }
                else
                {
                  if( (chsAux.GetRP()!=firstCHS.GetLP()) &&
                      (chsAux.GetRP()!=firstCHS.GetRP()) )
                  {
                    s<<" ( "<<chsAux.GetRP().GetX()<<" "<<chsAux.GetRP().GetY()<<" ) ";
                    pAtual = chsAux.GetRP();
                  }
                }
              }
            }
            break;
          }
        }
      s<<" )";
    }
    s<<" )";
  }
  s<<" )";
}
void PrintRegion(CRegion &r)
{
  ostringstream s;
  GetStrRegion(r,s);
  cout<<s.str();
}
/*

PARTNER NUMBER TEST CLASS


*/

bool PartnerNoTest::MainPartnerNoTest(ostringstream &testResult)

{
  string strRegion1 = "((((3 0)(10 1)(3 1))((3.1 0.1)(3.1 0.9)(6 0.8))))",
         strRegion2 = "( ( ( (3.0 0.0)(3.0 1.0) (5.0 0.8 ) (4.0 0.6) (6.0 0.3  ) ) ) )";
  CRegion  *r1 = GetRegion(strRegion1), *r2 = GetRegion(strRegion2);
  int resultOk1[12]= { 11, 2, 1,10, 9, 6, 5, 8, 7, 4, 3, 0 };
  int resultOk2[10]= { 9, 2, 1,6, 8, 7, 3, 5, 4, 0 };

  if (PartnerNoTest1(testResult,r1,resultOk1))
  if (PartnerNoTest1(testResult,r2,resultOk2))
    return true;
  return false;
}
bool PartnerNoTest::PartnerNoTest1(ostringstream &testResult,CRegion  *r,int resultOK[] )
{
  cout<<endl;
  for (int i=0; i<r->Size();i++)
  {
    CHalfSegment chs;
    r->Get(i,chs);
    if (chs.attr.partnerno!=resultOK[i])
    {
      testResult << "Er: PartnerNoTest1: partnerno!=resultOk.";
      cout <<endl<< "Er: PartnerNoTest1: partnerno!=resultOk"<<*r<<endl;
      return false;
    }
    //cout<<chs<<endl;
  }
  return true;
}
// --------------------------------------------------------------------------------------------------------
// ----------------- INSIDE ABOVE TEST CLASS --------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
bool InsideAboveTest::InsideAboveTest1(ostringstream &testResult,CRegion *r,
                                               bool resultOK[], int N_HALF_SEGMENTS )
{
  if (N_HALF_SEGMENTS != r->Size())
  {
    testResult << "Er: N_HS("<<N_HALF_SEGMENTS<<"!=CR("<<r->Size()<<").";
    return false;
  }

  for(int i=0;i<r->Size();i++)
  {
    CHalfSegment chs;
    r->Get(i,chs);
    if ( chs.attr.insideAbove != resultOK[i] )
    {
      testResult << "Er: chs[" << i << "]!=result["<< i <<"]";
      return false;
    }
  }

  return true;
}

bool InsideAboveTest::InsideAboveTest1(ostringstream &testResult)
{
  CRegion *r1 = GetRegion("( ( ( (3.0 0.0) (10.0 1.0) (3.0 1.0) ) ( (3.1 0.1) (3.1 0.9) (6.0 0.8))))");
  const int N_HALF_SEGMENTS1=12;
  bool resultOK1[N_HALF_SEGMENTS1] = { true, false, false, false, false, true, true, true, true, false, false, true };
  if (!InsideAboveTest1(testResult, r1, resultOK1, N_HALF_SEGMENTS1))
  {
    testResult<<"(1)";
    return false;
  }
   //_______________________________
  CRegion *r2 = GetRegion("( ( ( (3.0 0.0) (3.0 1.0) (5.0 0.8) (4.0 0.6) (6 0.3 ) ) ) )");
  const int N_HALF_SEGMENTS2=10;
  bool resultOK2[N_HALF_SEGMENTS2] = { true, false, false, false, false, true, false, true, false, true };

  if (!InsideAboveTest1(testResult, r2, resultOK2, N_HALF_SEGMENTS2))
  {
    testResult <<"(2)";
    return false;
  }
  //_______________________________
  CRegion *r3 = GetRegion("( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )");
  const int N_HALF_SEGMENTS3=10;
  bool resultOK3[N_HALF_SEGMENTS3] = { true, false, false, true, false, false, false, true, false, true };


  if (!InsideAboveTest1(testResult, r3, resultOK3, N_HALF_SEGMENTS3))
  {
    testResult <<"(3)";
    return false;
  }

  return true;
}

/*
 --------------------------------------------------------------------------------------------------------
 ----------------- Window CLIPPING In TEST CLASS --------------------------------------------------------------
 --------------------------------------------------------------------------------------------------------

*/

bool WindowClippingIn::ChsClippingInTest(ostringstream &testResult,CLine *testLine,
                                        Rectangle *window, CLine *clippedLine, bool acceptedLines[])
{

    bool result=true;
    //cout <<endl<<"=========== lineClippingTest"<<endl;

    if ( (testLine==NULL) || (window==NULL) )
    {
    testResult<<"Er: input"<<endl;
      return false;
    }

  result = true;
  for (int i=0; i<testLine->Size();i++)
  {
    CHalfSegment chs,chsInside;
    bool inside=false,isIntersectionPoint=false;
    testLine->Get(i,chs);
    if (chs.GetLDP())
      chs.WindowClippingIn(*window,chsInside, inside,isIntersectionPoint);
    else
      continue;
    //cout<<endl<<"===> "<<(inside?"Clipped":"Dicarded")<<endl;
    //cout<<"       Line: "<<chs<<endl;
    //if (accept == acceptedLines[i])
    //{
    if (inside && !isIntersectionPoint)
    {
      CHalfSegment chsInsideResult;
      clippedLine->Get(i,chsInsideResult);
     // cout<<"ClippedLine: "<<chsInside<<endl;
     // cout<<"Result     : "<<chsInsideResult<<endl;
      // Olhar operador ==
      //if (chsInside != chsInsideResult)
      //{
      //  testResult << "Er: testLine != clippedLine";
      //  result = false;
      //  break;
      //}
     }

     //else
     //{
     //  if (clippedLine!=NULL)
     //  {
     //    testResult << "Er:ChsClippingInTest: c(NULL),r(!NULL)";
     //    result = false;
     //  }
      //testResult << "Er: accept != acceptedLines[i]";
      //result = false;
      //break;
     //}


    }
    return result;
}

//void LineClipping(double &x0, double &y0,double &x1, double &y1, double xmin, double xmax,
//                    double ymin, double ymax, bool accept)

bool WindowClippingIn::ChsClippingInTest(ostringstream &testResult)
{
   Rectangle *window1 = GetRect("( 2.0 4.5 -1.0 2.0 )"),
             *window2 = GetRect("( 2.8 8.0 3.0 7.5 )");
   CLine         *line1 = GetLine("( (3.0 0.0 3.0 1.0) )"),
          *clippedLine1 = GetLine("( (3.0 0.0 3.0 1.0) )"),
                 *line2 = GetLine("( (3.0 1.0 5.0 0.8) )"),
          *clippedLine2 = GetLine("( (3.0 1.0 4.5 0.85))"),
                 *line3 = GetLine("( (5.0 0.8 4.0 0.6) )"),
          *clippedLine3 = GetLine("( (4.0 0.6 4.5 0.7))"),
                 *line4 = GetLine("( (4.0 0.6 6.0 0.3) )"),
          *clippedLine4 = GetLine("( (4.0 0.6 4.5 0.525))"),
                 *line5 = GetLine("( (6.0 0.3 3.0 0.0) )"),
          *clippedLine5 = GetLine("((3.0 0.0 4.5 0.15))"),
                 *line6 = GetLine("( (3.0 0.0 3.0 1.0) (3.0 1.0 5.0 0.8 ) (5.0 0.8 4.0 0.6) (4.0 0.6 6.0 0.3  ) (6.0 0.3 3.0 0.0 ) )"),
          *clippedLine6 = GetLine("( (3.0 0.0 3.0 1.0) (3.0 1.0 4.5 0.85) (4.0 0.6 4.5 0.7) (4.0 0.6 4.5 0.525) (3.0 0.0 4.5 0.15))"),
                 *line7 = GetLine("((1.0 1.0 7.0 0.0) (1.0 1.0 6.0 8.0) (4.0 4.0 7.0 0.0) (4.0 4.0 9.0 6.0) (6.0 8.0 9.0 6.0) )"),
          *clippedLine7 = GetLine("( (2.8 3.52 5.64286 7.5) (4.0 4.0 4.75 3.0) (4.0 4.0 8.0 5.6) (6.75 7.5 8.0 6.66667) )");
   bool acceptedLine1[1]={ true },
        acceptedLine2[1]={ true },
        acceptedLine3[1]={ true },
        acceptedLine4[1]={ true },
        acceptedLine5[1]={ true },
        acceptedLine6[5] = { true, true, true, true, true },
        acceptedLine7[5] = { true, true, true, true, false };

   if (ChsClippingInTest(testResult,line1,window1,clippedLine1,acceptedLine1))
     if (ChsClippingInTest(testResult,line2,window1,clippedLine2,acceptedLine2))
       if (ChsClippingInTest(testResult,line3,window1,clippedLine3,acceptedLine3))
         if (ChsClippingInTest(testResult,line4,window1,clippedLine4,acceptedLine4))
           if (ChsClippingInTest(testResult,line5,window1,clippedLine5,acceptedLine5))
             if (ChsClippingInTest(testResult,line6,window1,clippedLine6,acceptedLine6))
               if (ChsClippingInTest(testResult,line7,window2,clippedLine7,acceptedLine7))
            return true;
   return false;
}

/*
 --------------------------------------------------------------------------------------------------------
 ----------------- Line CLIPPING TEST CLASS --------------------------------------------------------------
 --------------------------------------------------------------------------------------------------------

*/
bool LineClippingTest::LineClippingInTest(ostringstream &testResult,CLine *testLine,
                                        Rectangle *window, CLine *rClippedLine)
{
    bool accepted=true;
    CLine clippedLine(0);


    if ( (testLine==NULL) || (window==NULL) )
    {
    testResult<<"Er: input"<<endl;
      return false;
    }


  testLine->WindowClippingIn(*window,clippedLine,accepted);

  if (!accepted)
  {
    if (!(rClippedLine==NULL))
    {
      testResult << "Er:LineClippingInTest: c(NULL),r(!NULL)";
      return false;
    }
    else
      cout<<endl<<"There is no part of the line outside the window.";
  }
  else
    if (rClippedLine==NULL)
    {
      testResult << "Er:LineClippingInTest: c(!NULL),r(NULL)";
      return false;
      }

    return true;
}


bool LineClippingTest::LineClippingInTest(ostringstream &testResult)
{
Rectangle *window1 = GetRect("( 2.0 4.5 -1.0 2.0 )"),
          *window2 = GetRect("( 2.8 8.0 3.0 7.5 )");
   CLine         *line1 = GetLine("( (3.0 0.0 3.0 1.0) )"),
          *clippedLine1 = GetLine("( (3.0 0.0 3.0 1.0) )"),
                 *line2 = GetLine("( (3.0 0.0 3.0 1.0) (3.0 1.0 5.0 0.8 ) (5.0 0.8 4.0 0.6) (4.0 0.6 6.0 0.3  ) (6.0 0.3 3.0 0.0 ) )"),
          *clippedLine2 = GetLine("( (3.0 0.0 3.0 1.0) (3.0 1.0 4.5 0.85) (4.0 0.6 4.5 0.7) (4.0 0.6 4.5 0.525) (3.0 0.0 4.5 0.15))"),
                 *line3 = GetLine("((1.0 1.0 7.0 0.0) (1.0 1.0 6.0 8.0) (4.0 4.0 7.0 0.0) (4.0 4.0 9.0 6.0) (6.0 8.0 9.0 6.0) )"),
          *clippedLine3 = GetLine("( (2.8 3.52 5.64286 7.5) (4.0 4.0 4.75 3.0) (4.0 4.0 8.0 5.6) (6.75 7.5 8.0 6.66667) )"),
                 *line4 = GetLine("( (2.0 -1.0 2.0 -5.0) )"),
          *clippedLine4 = NULL;

   if (LineClippingInTest(testResult,line1,window1,clippedLine1))
   if (LineClippingInTest(testResult,line2,window1,clippedLine2))
   if (LineClippingInTest(testResult,line3,window2,clippedLine3))
   if (LineClippingInTest(testResult,line4,window1,clippedLine4))
     return true;
   return false;
}

bool LineClippingTest::LineClippingOutTest(ostringstream &testResult,CLine *testLine,
                                        Rectangle *window, CLine *rClippedLine)
{
    bool accepted=true;
    CLine clippedLine(0);


  if ( (testLine==NULL) || (window==NULL) )
  {
    testResult<<"Er: LineClippingOutTest: input"<<endl;
    return false;
  }

  testLine->WindowClippingOut(*window,clippedLine,accepted);

  //cout<<endl<<"=========== lineClippingOutTest";
  //cout<<endl<<"Original: ";
  //LinePrinting(*testLine);
  if (accepted)
  {
    if (rClippedLine==NULL)
    {
      testResult << "Er:LineClippingOutTest: c(!NULL),r(NULL)";
      return false;
    }
    //cout<<endl<<"Clipped : ";
    //LinePrinting(clippedLine);
    //cout<<endl<<"Result: ";
    //LinePrinting(*rClippedLine);
  }
  else
  {
    if (rClippedLine==NULL)
      cout<<endl<<"There is no part of the line outside the window.";
    else
    {
      testResult << "Er:LineClippingOutTest: c(NULL),r(!NULL)";
      return false;
    }
  }
  return true;
}


bool LineClippingTest::LineClippingOutTest(ostringstream &testResult)
{
  string strWindow1 = "( 2.0 4.5 -1.0 2.0 )",
         strWindow2 = "( 2.8 8.0 3.0 7.5 )";

  string
         strLine1 = "( ( 3 0 6 0.3 )( 3 0 3 1 )( 3 0 3 1 )( 3 1 5 0.8 )( 4 0.6 6 0.3 ) ";
         strLine1 += "( 4 0.6 5 0.8 ) ( 3 1 5 0.8 )( 4 0.6 5 0.8 )( 4 0.6 6 0.3 )( 3 0 6 0.3 ) ) ";

  string
    strClippedLine1  = "( ( 4.5 0.15 6 0.3 )( 4.5 0.525 6 0.3 )( 4.5 0.7 5 0.8 )( 4.5 0.85 5 0.8 ) ";
    strClippedLine1 += "  ( 4.5 0.85 5 0.8 )( 4.5 0.7 5 0.8 )( 4.5 0.525 6 0.3 )( 4.5 0.15 6 0.3 ) )";
  string
    strLine2  = "( ( 1 1 7 0 )( 1 1 6 8 )( 4 4 7 0 )( 4 4 9 6 )( 1 1 6 8 ) ";
    strLine2 += "( 6 8 9 6 )( 4 4 7 0 )( 1 1 7 0 )( 6 8 9 6 )( 4 4 9 6 ) ) ";
  string
    strClippedLine2  = "( ( 1 1 7 0 )( 1 1 2.8 3.52 )( 1 1 2.8 3.52 )( 4.75 3 7 0 )( 5.64286 7.5 6 8 ) ";
    strClippedLine2 += "( 5.64286 7.5 6 8 )( 6 8 6.75 7.5 )( 6 8 6.75 7.5 )( 4.75 3 7 0 )( 1 1 7 0 ) ";
    strClippedLine2 += "( 8 5.6 9 6 )( 8 6.66667 9 6 )( 8 6.66667 9 6 )( 8 5.6 9 6 ) ) ";

  string
         strLine3 = "( (3.0 0.0 3.0 1.0) )",
         strLine4 = "( (2.0 -1.0 2.0 -5.0) )",
    strClippedLine4 = "( (2.0 -1.0 2.0 -5.0) )";;

  Rectangle *window1 = GetRect("( 2.0 4.5 -1.0 2.0 )"),
                *window2 = GetRect("( 2.8 8.0 3.0 7.5 )");
  CLine  *line1 = GetLine(strLine1), *clippedLine1 = GetLine(strClippedLine1),
         *line2 = GetLine(strLine2), *clippedLine2 = GetLine(strClippedLine2),
         *line3 = GetLine(strLine3), *clippedLine3 = NULL,
         *line4 = GetLine(strLine4), *clippedLine4 = GetLine(strClippedLine4);

   if (LineClippingOutTest(testResult,line1,window1,clippedLine1))
   if (LineClippingOutTest(testResult,line2,window2,clippedLine2))
   if (LineClippingOutTest(testResult,line3,window1,clippedLine3))
   if (LineClippingOutTest(testResult,line4,window1,clippedLine4))
     return true;
   return false;
}
// --------------------------------------------------------------------------------------------------------
// ----------------- Region CLIPPING TEST CLASS --------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

bool RegionClippingTest::MainRegionClippingInTest(ostringstream &testResult)
{
  string strWindow1 = "( 2.0 4.5 -1.0 2.0 )",
           strWindow2 = "( 2.8 8.0 3.0 7.5 )";

  string strRegion1 = "( ( ( (3 0)    (10 1)    (3 1) ) ( (3.1 0.1) (3.1 0.9) (6 0.8) ) ) )",
         strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )",
         strRegion3 = strRegion1,
         strRegion4 = "( ( ( (3 0)    (10 1)    (3 1) ) ( (3.1 0.5) (3.1 0.9) (6 0.8) ) ) )";

  Rectangle *window1 = GetRect("( 2.0 4.5 -1.0 2.0 )"),
            *window2 = GetRect("( 2.8 8.0 3.0 7.5 )"),
            *window3 = GetRect("( 3.5 4.5 -1.0 2.0 )"),
            *window4 = GetRect("( 3.5 4.5 0.35 0.958 )");

  string strLine1  = "( ( 3 0 4.5 0.214286 )( 3 0 3 1 )( 3 1 4.5 1 )( 3.1 0.1 4.5 0.437931 )";
         strLine1 += "( 3.1 0.1 3.1 0.9 )( 3.1 0.9 4.5 0.851724 )( 4.5 0.214286 4.5 0.437931 )";
         strLine1 += "( 4.5 0.851724 4.5 1 ) )";

  string strLine2  = "( ( 2.8 3 4.75 3 )  ( 2.8 3 2.8 3.52 ) ( 2.8 3.52 5.64286 7.5 )( 4 4 4.75 3 )";
         strLine2 += "  ( 4 4 8 5.6 )( 5.64286 7.5 6.75 7.5 ) ( 6.75 7.5 8 6.66667 )( 8 5.6 8 6.66667 ))";

  string strLine3  = "( ( 3.5 0.0714286 4.5 0.214286 )( 3.5 0.0714286 3.5 0.196552 )";
         strLine3 += "( 3.5 0.196552 4.5 0.437931 )( 3.5 0.886207 4.5 0.851724 )";
         strLine3 += "( 3.5 0.886207 3.5 1 )( 3.5 1 4.5 1 )( 4.5 0.214286 4.5 0.437931 )";
         strLine3 += "( 4.5 0.851724 4.5 1 ) )";
  string strLine4  = "( ( 3.5 0.35 4.5 0.35 )( 3.5 0.35 3.5 0.541379 )( 3.5 0.541379 4.5 0.644828 )";
         strLine4 += "( 3.5 0.886207 4.5 0.851724 )( 3.5 0.886207 3.5 0.958 )( 3.5 0.958 4.5 0.958 )";
         strLine4 += "( 4.5 0.35 4.5 0.644828 )( 4.5 0.851724 4.5 0.958 ) )";

  CRegion  *r1 = GetRegion(strRegion1),
           *r2 = GetRegion(strRegion2),
           *r3 = GetRegion(strRegion3),
           *r4 = GetRegion(strRegion4);
  CLine *l1=GetLine(strLine1),
        *l2 = GetLine(strLine2),
        *l3 = GetLine(strLine3),
        *l4 = GetLine(strLine4);
  bool result = false;
  if (MainGetDPointTest(testResult))
  if (VectorSizeTest(testResult))
  if (AngleTest(testResult))
  if (MainCreateNewSegmentsTest(testResult))
  if (MainCreateNewSegmentsWindowVertices(testResult))
  if (!GetClippedHSTest(testResult, r1, window1, l1))
      testResult<<"(1)";
  else
  if (!GetClippedHSTest(testResult, r2, window2, l2))
    testResult<<"(2)";
  else
  if (!GetClippedHSTest(testResult, r3, window3, l3))
    testResult<<"(3)";
  else
  if (!GetClippedHSTest(testResult, r4, window4, l4))
    testResult<<"(4)";
  else
  if (MainComputeCycleTest(testResult))
  if (MainComputeRegionTest(testResult))
  if (MainWindowClippingIn(testResult))
  if (MainWindowClippingOut(testResult))
    result = true;
  delete r1;
  delete r2;
  delete r3;
  delete r4;
  delete l1;
  delete l2;
  delete l3;
  delete l4;
  delete window1;
  delete window2;
  delete window3;
  delete window4;
  return true;
}

bool RegionClippingTest::VectorSizeTest(ostringstream &testResult)
{
  Point v, p1, p2;
  v.Set(2.8, 7.5);
  p1.Set(5.642, 7.5);
  p2.Set(6.0,8.0);

  if (!(abs(VectorSize(v,p1)-2.842)<0.00001))
  {
    testResult<<"Er: VectorSizeTest(1)";
    return false;
  }
  if (!(abs(VectorSize(p2,p1)-0.61495)<0.00001))
  {
    testResult<<"Er: VectorSizeTest(2)";
    return false;
  }
  return true;
}

bool RegionClippingTest::AngleTest(ostringstream &testResult)
{
  //double Angle(Point &v, bool horizontal, Point &p1,Point &p2);
  Point v, p1, p2;
  //1
  v.Set(2.8, 7.5);
  p1.Set(5.642, 7.5);
  p2.Set(6.0,8.0);
  if ( !(abs(Angle(v,p1,p2)-2.19218)<0.001))
  {
    testResult<<"Er: AngleTest(1)";
    return false;
  }
  //2
  v.Set(2.8, 7.5);
  p1.Set(6.75, 7.5);
  p2.Set(6.0,8.0);

  if ( !(abs(Angle(v,p1,p2)-0.588003)<0.001))
  {
    testResult<<"Er: AngleTest(2)";
    return false;
  }
  //3
  v.Set(8, 3);
  p1.Set(8, 5.6);
  p2.Set(9.0,6.0);

  if ( !(abs(Angle(v,p1,p2)-1.9513)<0.001))
  {
    testResult<<"Er: AngleTest(3)";
    return false;
  }

  //4
  v.Set(8, 3);
  p1.Set(8, 6.666);
  p2.Set(9.0,6.0);

   if ( !(abs(Angle(v,p1,p2)-0.983255)<0.001))
  {
    testResult<<"Er: AngleTest(4)";
    return false;
  }

  //5
  v.Set(2.8, 3);
  p1.Set(4.75, 3);
  p2.Set(7.0,0.0);

  if ( !(abs(Angle(v,p1,p2)-2.2143)<0.001))
  {
    testResult<<"Er: AngleTest(5)";
    return false;
  }

  //6
  v.Set(2.8, 3);
  p1.Set(2.8, 3.52);
  p2.Set(1.0,1.0);

  if ( !(abs(Angle(v,p1,p2)-0.620249)<0.001))
  {
    testResult<<"Er: AngleTest(6)";
    return false;
  }

  //7 ==> 180 grau (vertical edge)
  v.Set(2.8, 3);
  p1.Set(2.8, 3.52);
  p2.Set(2.8,5.0);

  if (  !(abs(Angle(v,p1,p2)-0)<0.001))
  {
    testResult<<"Er: AngleTest(7)";
    return false;
  }

  //8 ==> 90 graus (vertical edge)
  v.Set(2.8, 3);
  p1.Set(2.8, 3.52);
  p2.Set(0,3.52);

  if ( !(abs(Angle(v,p1,p2)-PI/2)<0.001))
  {
    testResult<<"Er: AngleTest(8)";
    return false;
  }

  //9 ==> 0 graus (horizontal edge)
  v.Set(2.8, 7.5);
  p1.Set(5.642, 7.5);
  p2.Set(0,7.5);

  if ( !(abs(Angle(v,p1,p2))<0.001))
  {
    testResult<<"Er: AngleTest(9)";
    return false;
  }

  //10 ==> 90 graus (horizontal edge)
  v.Set(2.8, 7.5);
  p1.Set(5.642, 7.5);
  p2.Set(5.642, 8.0);

  if ( !(abs(Angle(v,p1,p2)-PI/2)<0.001))
  {
    testResult<<"Er: AngleTest(10)";
    return false;
  }
  //11 ==> 270 graus (horizontal edge)
  v.Set(2.8, 7.5);
  p1.Set(5.642, 7.5);
  p2.Set(5.642, 6.0);

  if ( !(abs(Angle(v,p1,p2)-PI/2)<0.001))
  {
    testResult<<"Er: AngleTest(11)";
    return false;
  }
  return true;
}

bool RegionClippingTest::GetClippedHSTest(ostringstream &testResult,CRegion *testRegion,
                                        Rectangle *window, CLine *clippedRegion)
{
  CRegion *cr = new CRegion(0);
  //cout<<endl<<"Region that will be clipped"<<endl;
  //PrintCHSRegion(*testRegion);
  testRegion->GetClippedHS(*window,*cr,true);
  //cout<<endl<<*cr<<endl;
  //cout<<endl;
  //cout<<"Clipped region"<<endl;
  //PrintCHSRegion(*cr);
  //cout<<endl<<"Printing region as line";
  //PrintRegionAsLine(*cr);
  if (!CompareLineAndRegion(*clippedRegion,*cr))
  {
    testResult<<"Er: GetClippedHSTest";
    return false;
  }
  return true;
}

bool RegionClippingTest::MainCreateNewSegmentsTest(ostringstream &testResult)
{
  if (!CreateNewSegmentsHTest(testResult,7.5,"7.5",WTOP))
    return false;
  if (!CreateNewSegmentsHTest(testResult,3.0,"3.0",WBOTTOM))
    return false;

  if (!CreateNewSegmentsVTest(testResult,2.8,"2.8",WLEFT))
    return false;
  if (!CreateNewSegmentsVTest(testResult,8.0,"8.0",WRIGHT))
    return false;

  return true;
}
bool RegionClippingTest::CreateNewSegmentsHTest(ostringstream &testResult,
                         double edge,string strEdge, WindowEdge wEdge)
{

  vector<DPoint> pointsOnEdge;
  CRegion *cr;
  Point p,bPoint,ePoint;
  DPoint dp;
  CLine  *line;
  string strLine;
  int partnerno=0;
  string strWindow2 = "( 2.8 8.0 3.0 7.5 )";

  Rectangle *window2 = GetRect("( 2.8 8.0 3.0 7.5 )");

  bPoint.Set(window2->Left(),edge);
  ePoint.Set(window2->Right(),edge);

  //TESTE 1 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //UPPER EDGE: connecting points to the vertices of the window
  strLine = "( (2.8 " + strEdge + " 5.64286 " + strEdge + " ) ( 6.75 " + strEdge + " 8 " + strEdge + "))";
  line = GetLine(strLine);
  cr = new CRegion(0);
  cr->StartBulkLoad();


  dp.Set(5.64286, edge,true);
  pointsOnEdge.push_back(dp);


  dp.Set(6.75, edge,false);
  pointsOnEdge.push_back(dp);


  CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

  cr->EndBulkLoad();

  if (!CompareLineAndRegion(*line,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsTest:T1 ("<<edge<<")";
    return false;
  }
  delete line;
  delete cr;
  pointsOnEdge.clear();

  //TESTE 2 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //UPPER EDGE: connecting points to no vertices of the window
  line = GetLine("( (5.64286 " + strEdge + "  6.75 " + strEdge + "))");//
  cr = new CRegion(0);
  cr->StartBulkLoad();
  dp.Set(5.64286, edge,false);//
  pointsOnEdge.push_back(dp);

  dp.Set(6.75, edge,true);//
  pointsOnEdge.push_back(dp);

  CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

  cr->EndBulkLoad();

  if (!CompareLineAndRegion(*line,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsTest:T2 ("<<edge<<")";//
    return false;
  }
  delete line;
  delete cr;
  pointsOnEdge.clear();


  //TESTE 3 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //UPPER EDGE: one segment connecting points of the region, and another segment connecting
  //one point to the top-right edge.
    line = GetLine("( (5.64286 " + strEdge + "  6.75 " + strEdge + ") (7.0 " + strEdge + " 8 " + strEdge + "))");
    cr = new CRegion(0);
    cr->StartBulkLoad();
    dp.Set(5.64286, edge,false);//
    pointsOnEdge.push_back(dp);

    dp.Set(6.75, edge,true);//
    pointsOnEdge.push_back(dp);

    dp.Set(7.0, edge,false);//
    pointsOnEdge.push_back(dp);

    CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

    cr->EndBulkLoad();



    if (!CompareLineAndRegion(*line,*cr))
    {
      testResult<<"Er:MainCreateNewSegmentsTest:T3 ("<<edge<<")";//
      return false;
    }
    delete line;
    delete cr;
    pointsOnEdge.clear();

  //TESTE 4 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //UPPER EDGE: one segment connecting points of the region, and another segment connecting
  //one point to the top-left edge.
  line = GetLine("((2.8 " + strEdge + " 4.0 " + strEdge + " ) (5.64286 " + strEdge + "  6.75 " + strEdge + ") )");
  cr = new CRegion(0);
  cr->StartBulkLoad();

  dp.Set(5.64286, edge,false);//
  pointsOnEdge.push_back(dp);

  dp.Set(6.75, edge,true);//
  pointsOnEdge.push_back(dp);

  dp.Set(4.0, edge,true);//
  pointsOnEdge.push_back(dp);

  CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

  cr->EndBulkLoad();

  if (!CompareLineAndRegion(*line,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsTest:T4 ("<<edge<<")";//
    return false;
  }
  delete line;
  delete cr;
  pointsOnEdge.clear();

  return true;
}

bool RegionClippingTest::CreateNewSegmentsVTest(ostringstream &testResult,
                         double edge,string strEdge, WindowEdge wEdge)
{

  vector<DPoint> pointsOnEdge;
  CRegion *cr;
  Point p,bPoint,ePoint;
  DPoint dp;
  CLine  *line;
  string strLine;
  int partnerno=0;
  string strWindow2 = "( 2.8 8.0 3.0 7.5 )";

  Rectangle *window2 = GetRect("( 2.8 8.0 3.0 7.5 )");

  bPoint.Set(edge,window2->Bottom());
  ePoint.Set(edge,window2->Top());

  //TESTE 1 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //VERTICAL EDGE: connecting point to the upper vertice of the window
  line = GetLine("( ( " + strEdge + " 3.52 " + strEdge + " 3.0) )");
  cr = new CRegion(0);
  cr->StartBulkLoad();


  dp.Set(edge,3.52, false);
  pointsOnEdge.push_back(dp);


  CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

  cr->EndBulkLoad();

  if (!CompareLineAndRegion(*line,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsTest:T1 ("<<edge<<")";
    return false;
  }
  delete line;
  delete cr;
  pointsOnEdge.clear();

  //TESTE 2 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //VERTICAL EDGE: connecting point to the upper vertice of the window

  line = GetLine("( ( " + strEdge + " 3.52 " + strEdge + " 7.5) )");
  cr = new CRegion(0);
  cr->StartBulkLoad();


  dp.Set(edge,3.52, true);
  pointsOnEdge.push_back(dp);


  CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

  cr->EndBulkLoad();

  if (!CompareLineAndRegion(*line,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsTest:T2 ("<<edge<<")";
    return false;
  }
  delete line;
  delete cr;
  pointsOnEdge.clear();

  //TESTE 3 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //VERTICAL EDGE: connecting points to the vertices of the window
  strLine = "( ( " + strEdge + " 3.52 " + strEdge + " 3.0)";
  strLine +="  ( " + strEdge + " 4.52 " + strEdge + " 5.0)";
  strLine +="  ( " + strEdge + " 7.0  " + strEdge + " 7.5) )";

  line = GetLine(strLine);
  cr = new CRegion(0);
  cr->StartBulkLoad();


  dp.Set(edge,4.52, true);
  pointsOnEdge.push_back(dp);

  dp.Set(edge,3.52, false);
  pointsOnEdge.push_back(dp);

  dp.Set(edge,7.0, true);
  pointsOnEdge.push_back(dp);

  dp.Set(edge,5.0, false);
  pointsOnEdge.push_back(dp);

  CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

  cr->EndBulkLoad();

  if (!CompareLineAndRegion(*line,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsTest:T3 ("<<edge<<")";
    return false;
  }
  delete line;
  delete cr;
  pointsOnEdge.clear();
  //TESTE 4 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //VERTICAL EDGE: connecting points to no of the vertices of the window
  line = GetLine( " ( ( " + strEdge + " 4.52 " + strEdge + " 5.0))");

  cr = new CRegion(0);
  cr->StartBulkLoad();


  dp.Set(edge,4.52, true);
  pointsOnEdge.push_back(dp);

  dp.Set(edge,5.0, false);
  pointsOnEdge.push_back(dp);

  CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

  cr->EndBulkLoad();

  if (!CompareLineAndRegion(*line,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsTest:T4 ("<<edge<<")";
    return false;
  }
  delete line;
  delete cr;
  pointsOnEdge.clear();

  //TESTE 5 ==> strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )";
  //VERTICAL EDGE: connecting points to no of the vertices of the window
  strLine = " ( ( " + strEdge + " 4.52 " + strEdge + " 5.0)";
  strLine += "   ( " + strEdge + " 6.53 " + strEdge + " 7.0) )";
  line = GetLine(strLine);
  cr = new CRegion(0);
  cr->StartBulkLoad();


  dp.Set(edge,4.52, true);
  pointsOnEdge.push_back(dp);

  dp.Set(edge,5.0, false);
  pointsOnEdge.push_back(dp);

  dp.Set(edge,7.0, false);
  pointsOnEdge.push_back(dp);

  dp.Set(edge,6.53, true);
  pointsOnEdge.push_back(dp);

  CRegion::CreateNewSegments(pointsOnEdge,*cr,bPoint,ePoint,wEdge,partnerno,true);

  cr->EndBulkLoad();

  if (!CompareLineAndRegion(*line,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsTest:T5 ("<<edge<<")";
    return false;
  }
  delete line;
  delete cr;
  pointsOnEdge.clear();

  return true;
}

bool RegionClippingTest::MainCreateNewSegmentsWindowVertices(ostringstream &testResult)
{
  vector<DPoint> pointsOnEdge[4];
  CRegion *r = GetRegion("( ( ( (3 0)    (10 1)    (3 1) ) ( (3.1 0.5) (3.1 0.9) (6 0.8) ) ) )");
  CRegion *cr;
  CLine *l;
  Rectangle *window;
  int partnerno=0;

  cr = new CRegion(0);
  cr->StartBulkLoad();
  window = GetRect("( 3.5 4.5 0.25 0.4 )");
  l = GetLine("( ( 3.5 0.25 3.5 0.4) ( 3.5 0.4 4.5 0.4) ( 4.5 0.4 4.5 0.25) ( 4.5 0.25 3.5 0.25))");

  r->CreateNewSegmentsWindowVertices(*window,pointsOnEdge,*cr,partnerno,true);
  cr->EndBulkLoad();
  if (!CompareLineAndRegion(*l,*cr))
  {
    testResult<<"Er:MainCreateNewSegmentsWEdgesInside:T1";
    delete window;
    delete l;
    delete r;
    return false;
  }
  delete window;
  delete l;

  delete r;
  return true;
}

bool RegionClippingTest::MainGetDPointTest(ostringstream &testResult)
{
  //static DPoint* GetDPoint(const Point &p,const Point &p2,bool insideAbove,const Point &v,bool horizontal);

  if (!(GetDPointTest(1.0,0.0,true,1.0,1.0,0.0,0.0,true)))
  {//Horizontal edge: Angle 90 degree, region to the left
    testResult<<"Er:MainGetDPointTest(1)";
    return false;
  }

  if (!(GetDPointTest(1.0,0.0,false,1.0,1.0,0.0,0.0,false)))
  {//Horizontal edge: Angle 90 degree, region to the right
    testResult<<"Er:MainGetDPointTest(2)";
    return false;
  }

  if (!(GetDPointTest(1.0,0.0,false,0.999,1.0,0.0,0.0,true)))
  {//Horizontal edge: Angle almost 90 degree, region to right
        testResult<<"Er:MainGetDPointTest(3)";
        return false;
  }
  if (!(GetDPointTest(1.0,0.0,true,0.999,1.0,0.0,0.0,false)))
  {//Horizontal edge: Angle almost 90 degree, region to right
    testResult<<"Er:MainGetDPointTest(4)";
    return false;
  }
  if (!(GetDPointTest(1.0,0.0,false,1.0001,1.0,0.0,0.0,false)))
  {//Horizontal edge: Angle more than 90 degree, region to right
      testResult<<"Er:MainGetDPointTest(5)";
      return false;
  }

  if (!(GetDPointTest(1.0,0.0,true,1.0001,1.0,0.0,0.0,true)))
  {//Horizontal edge: Angle more than 90 degree, region to right
      testResult<<"Er:MainGetDPointTest(6)";
      return false;
  }

  return true;
}
bool RegionClippingTest::GetDPointTest(Coord pX, Coord pY, bool insideAbove, Coord p2X, Coord p2Y,
                                       Coord vX, Coord vY, bool rDirection)
{
  //static DPoint* GetDPoint(const Point &p,const Point &p2,bool insideAbove,const Point &v,bool horizontal);
  Point p,p2,v;
  DPoint *dp,result;
  p.Set(pX,pY);
  p2.Set(p2X,p2Y);
  v.Set(vX,vY);
  result.Set(pX,pY,rDirection);
  dp = DPoint::GetDPoint(p,p2,insideAbove,v);
  if (*dp != result)
    return false;

  return true;
}

bool RegionClippingTest::MainComputeCycleTest(ostringstream &testResult)
{

  string strCR1 = "( ( (  (1 1 ) (7 0 ) ( 4 4 )  ( 9 6 )  ( 6 8 )  ) ) )",
         strCR2 = "( ( (  (3 0 ) (10 1 ) ( 3 1 )  ) (  (3.1 0.1 ) (6 0.8 ) ( 3.1 0.9 )  ) ) )";
  vector<int> v;

  v.push_back(0);
  if (!ComputeCycleTest(strCR1,v))//*r != rClone)
  {
    testResult<<"Er:ComputeCycleTest(1)";
    return false;
  }
  v.clear();
  v.push_back(0);
  v.push_back(4);
  if (!ComputeCycleTest(strCR2,v))//*r != rClone)
  {
    testResult<<"Er:ComputeCycleTest(2)";
    return false;
  }
  return true;
}

bool RegionClippingTest::ComputeCycleTest(string strCR, vector<int> v)
{

  CRegion *r=GetRegion(strCR);
  bool *cycle;
  int edgeno=0;
  bool result = true;
  int c=0;
  ostringstream s;
  cycle = new bool[r->Size()];
  memset( cycle, false, r->Size() );
  CleanRegion(*r);

  for(unsigned int i=0; i<v.size();i++)
  {
    r->ComputeCycle(v[i], 0, c, edgeno, cycle);
    c++;
  }
  GetStrRegion(*r,s);
  cout<<endl<<s.str();
  cout<<endl<<strCR;
  if (strCR==s.str())//*r != rClone)
    result = true;
  delete r;
  delete cycle;
  return result;
}
bool RegionClippingTest::MainComputeRegionTest(ostringstream &testResult)
{
  string strCR1 = "( ( (  (1 1 ) (7 0 ) ( 4 4 )  ( 9 6 )  ( 6 8 )  ) ) )",
         strCR2 = "( ( (  (3 0 ) (10 1 ) ( 3 1 )  ) (  (3.1 0.1 ) (6 0.8 ) ( 3.1 0.9 )  ) ) )";

  if (!ComputeRegionTest(strCR1))
  {
    testResult<<"Er:ComputeCycleTest(1)";
    return false;
  }
  if (!ComputeRegionTest(strCR2))
  {
    testResult<<"Er:ComputeCycleTest(2)";
    return false;
  }
  return true;
}

bool RegionClippingTest::ComputeRegionTest(string strCR)
{
  CRegion *r=GetRegion(strCR),
          *rClone = r->Clone();
  ostringstream s;
  bool result = true;
  cout<<endl<<"Before computing"<<endl;
  PrintCHSRegion(*r);
  CleanRegion(*r);
  cout<<endl<<"Zero"<<endl;
  PrintCHSRegion(*r);
  r->ComputeRegion();
  GetStrRegion(*r,s);
  cout<<endl<<"MainComputeRegionTest"<<endl;
  cout<<endl<<"strCR: "<<strCR<<endl;
  cout<<endl<<"str  : "<<s.str()<<endl;
  cout<<endl<<"After computing"<<endl;
  PrintCHSRegion(*r);

  if (strCR==s.str())//*r != rClone)
    result=true;
  delete r;
  delete rClone;
  return result;
}

bool RegionClippingTest::MainWindowClippingIn(ostringstream &testResult)
{

  string strWindow1 = "( 2.0 4.5 -1.0 2.0 )",
           strWindow2 = "( 2.8 8.0 3.0 7.5 )";

  string strRegion1 = "( ( ( (3 0)    (10 1)    (3 1) ) ( (3.1 0.1) (3.1 0.9) (6 0.8) ) ) )",
         strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )",
         strRegion3 = strRegion1,
         strRegion4 = "( ( ( (3 0)    (10 1)    (3 1) ) ( (3.1 0.5) (3.1 0.9) (6 0.8) ) ) )";

  Rectangle *window1 = GetRect("( 2.0 4.5 -1.0 2.0 )"),
            *window2 = GetRect("( 2.8 8.0 3.0 7.5 )"),
            *window3 = GetRect("( 3.5 4.5 -1.0 2.0 )"),
            *window4 = GetRect("( 3.5 4.5 0.35 0.958 )");



  CRegion  *r1 = GetRegion(strRegion1),
           *r2 = GetRegion(strRegion2),
           *r3 = GetRegion(strRegion3),
           *r4 = GetRegion(strRegion4);

  CRegion *clippedRegion = new CRegion(0);

  r1->WindowClippingIn(*window1,*clippedRegion);

  cout<<endl<<"MainWindowClippingIn: "<<endl;
  PrintCHSRegion(*clippedRegion);

  //clippedRegion.ComputeRegion();

  //r1->WindowClippingIn(*window1,*clippedRegion);

  //cout<<"MainWindowClippingIn: "<<*r1;

  delete r1;
  delete r2;
  delete r3;
  delete r4;
  delete window1;
  delete window2;
  delete window3;
  delete window4;
  return true;

}

bool RegionClippingTest::MainWindowClippingOut(ostringstream &testResult)
{

  string strWindow1 = "( 2.0 4.5 -1.0 2.0 )",
           strWindow2 = "( 2.8 8.0 3.0 7.5 )";

  string strRegion1 = "( ( ( (3 0)    (10 1)    (3 1) ) ( (3.1 0.1) (3.1 0.9) (6 0.8) ) ) )",
         strRegion2 = "( ( ( (1.0 1.0)(7.0 0.0) (4.0 4.0) (9.0 6.0) (6.0 8.0) ) ) )",
         strRegion3 = strRegion1,
         strRegion4 = "( ( ( (3 0)    (10 1)    (3 1) ) ( (3.1 0.5) (3.1 0.9) (6 0.8) ) ) )";
  ostringstream s;

  Rectangle *window1 = GetRect("( 2.0 4.5 -1.0 2.0 )"),
            *window2 = GetRect("( 2.8 8.0 3.0 7.5 )"),
            *window3 = GetRect("( 3.5 4.5 -1.0 2.0 )"),
            *window4 = GetRect("( 3.5 4.5 0.35 0.958 )");



  CRegion  *r1 = GetRegion(strRegion1),
           *r2 = GetRegion(strRegion2),
           *r3 = GetRegion(strRegion3),
           *r4 = GetRegion(strRegion4);

  CRegion *clippedRegion = new CRegion(0);

  r1->WindowClippingOut(*window1,*clippedRegion);

  cout<<endl<<"MainWindowClippingOut: "<<endl;
  PrintCHSRegion(*r1);
  PrintCHSRegion(*clippedRegion);
  GetStrRegion(*clippedRegion,s);
  cout<<endl<<s.str();
  cout<<endl<<*clippedRegion;

  //clippedRegion.ComputeRegion();

  //r1->WindowClippingIn(*window1,*clippedRegion);

  //cout<<"MainWindowClippingIn: "<<*r1;

  delete r1;
  delete r2;
  delete r3;
  delete r4;
  delete window1;
  delete window2;
  delete window3;
  delete window4;
  return true;

}

