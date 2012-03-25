/*

[1] RobustGeometryAlgebra

Februar 2012 Katja Koch
Overview




Defines and Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "GenericTC.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoConfig.h"
#include "AvlTree.h"
#include "AVLSegment.h"
#include "AlmostEqual.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "RegionTools.h"
#include "Symbols.h"
#include "NList.h"
#include "LogMsg.h"
#include "ConstructorTemplates.h"
#include "TypeMapUtils.h"
#include "SpatialAlgebra.h"
#include "RobustGeometryAlgebra.h"
#include "AVLSegment.h"
#include "HalfSegment.h"

#include <vector>
#include <queue>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <queue>
#include <iterator>
#include <sstream>
#include <limits>
#include <errno.h>
#include <cerrno>


#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

extern NestedList* nl;
extern QueryProcessor* qp;

/*
Algebra Implementation
Type Mapping Functions
These functions check whether the correct argument types are supplied for an
operator; if so, returns a list expression for the result type, otherwise the
symbol ~typeerror~.

*/

/*
An algebra module must provide for each type functions which take a nested
list and create an instance of the class representing the type and vice versa.
These functions are called In- and Out-functions. ->see SpatialAlgebra In/Out

*/

/*
This means that for a type constructor the functions create, delete,
close, and clone must be implemented. The remaining two functions open
and save may be implemented. If they are not implemented,
the default persistent storage mechanism is used. ->see SpatialAlgebra

*/

/*
Type investigation auxiliaries

Within this algebra module, we have to handle with values of different
types:  ~line~ and ~Points~.
->see SpatialAlgebra

*/

enum SpatialTypeRG {stpoint,stpoints,stline,stregion,stbox,sterror};

SpatialTypeRG
SpatialTypeOfSymbolRG( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == Point::BasicType()  ) return (stpoint);
    if ( s == Points::BasicType() ) return (stpoints);
    if ( s == Line::BasicType()   ) return (stline);
    if ( s == Region::BasicType() ) return (stregion);
    if ( s == Rectangle<2>::BasicType()   ) return (stbox);
  }
  return (sterror);
}

/*
Type mapping

*/

static ListExpr intersectionTM( ListExpr args )
{
   ListExpr arg1, arg2;
   if ( nl->ListLength( args ) == 2 )
   {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if ( SpatialTypeOfSymbolRG( arg1 ) == stline &&
         SpatialTypeOfSymbolRG( arg2 ) == stline )
         return (nl->SymbolAtom( Points::BasicType() ));
    }
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

int RobustGeometrySetOpSelect(ListExpr args)
{
  string a1 = nl->SymbolValue(nl->First(args));
  string a2 = nl->SymbolValue(nl->Second(args));

  if(a1==Line::BasicType())
  {
    if(a2==Line::BasicType())   return 1;
    return -1;
  }

  return -1;
}


/*
Implementation of ~BOEvent~

*/

/*
Constructors

~Standard Constructor~

*/

robustGeometry::BOEvent::BOEvent():
	   x(0),y(0),
	   owner(avlseg::none),
	   pointType(nothing)
	{ };

robustGeometry::BOEvent::BOEvent
(const avlseg::ExtendedHalfSegment& hs,
			const double x,
			const double y,
			const robustGeometry::boPointType pointtype ,
			const avlseg::ownertype owner)
	{
		setX(x);
		setY(y);
		setExtededHalfSegment(hs);
		setPointType(pointType);
		setOwner(owner);
	};

robustGeometry::BOEvent::BOEvent
(const avlseg::ExtendedHalfSegment& ahs,
	const avlseg::ExtendedHalfSegment& bhs,
	const double x,
	const double y,
	const robustGeometry::boPointType pointtype )
	{
		setX(x);
		setY(y);
		setAboveEHS(ahs);
		setBelowEHS(bhs);
		setPointType(pointType);
	};

robustGeometry::BOEvent::BOEvent
(const double x,
 const double y,
 const robustGeometry::boPointType pointType):
	owner(avlseg::none)
	{
		setX( x );
		setY( y );
		setPointType( pointType );
	}


/*
~Print~

This function writes this event to __out__.

*/

void robustGeometry::BOEvent::Print(ostream& out)const
{

	out << "X :"<<getX( )<<
		   ",Y : " << getY( ) <<
		   ", owner :" << getOwner( ) <<
		   ", pointType : "<< getPointType();
}

const string intersectionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{line} x"
  "   {line} -> T, "
  " where T = points if any point or point type is one of the "
  " arguments or the argument having the smaller dimension </text--->"
  "<text>intersectionBO(arg1, arg2)</text--->"
  "<text>intersectionBO of two spatial objects</text--->"
  "<text>query intersectionBO(tiergarten, thecenter) </text--->"
  ") )";

/*
class MakeBO

*/

class MakeBo
{

public:
	 void IntersectionBO(const Line& line1,
			 const Line& line2,Points& result);
	  int intersect(double sx1,double sy1,double ex2,
			double ey2,double sx3,double sy3,double ex4,
			double ey4,double x,double y);
	  void checkIS(robustGeometry::BOEvent currEv,
			  avlseg::AVLSegment currAS);
	  void findAndSwap(avlseg::AVLSegment aboveAS,
			  avlseg::AVLSegment belowAS);
	  void planeSweepCase1(robustGeometry::BOEvent currEv);
	  void planeSweepCase2(robustGeometry::BOEvent currEv);
	  void planeSweepCase3(robustGeometry::BOEvent currEv);
	  avlseg::AVLSegment* getAbove(double aktXPos,double aktYPos);
	  avlseg::AVLSegment* getBelow(double aktXPos,double aktYPos);
	  Points* outputPoints;
   MakeBo()
   {
	   Points* outputPoints= new Points();

   };
   ~MakeBo() {};

private:
  //avltree::AVLTree<avlseg::AVLSegment> sweepLine;
  vector <avlseg::AVLSegment > sL;
  vector <robustGeometry::BOEvent> events;

};


//todo zu Klasse MakeBO als operator
bool sortBOEvents( const robustGeometry::BOEvent &i,
		const robustGeometry::BOEvent &j)
{
	if( i.getX()==j.getX()) return (i.getY()<j.getY());
	return (i.getX()<j.getX());
}

void MakeBo::findAndSwap(avlseg::AVLSegment aboveAS,
		avlseg::AVLSegment belowAS)
{
	avlseg::AVLSegment tmp;
	int pos1=-1;
	int pos2=-1;

	//Suche in Segmente X-Koord gleich, Y-Koord groesser
	vector< avlseg::AVLSegment >::iterator row_it = sL.begin();
	vector< avlseg::AVLSegment >::iterator row_end = sL.end();

	int i = 0;
	double tempx1=0.0;
	double tempx2=0.0;
	double tempy1=0.0;
	double tempy2=0.0;
	for ( ; row_it != row_end; ++row_it)
	{

	    avlseg::AVLSegment tmpAS = *row_it;
	    tempx1=tmpAS.getX1();
		tempy1=tmpAS.getY1();
		tempx2=tmpAS.getX2();
		tempy2=tmpAS.getY2();

		if((tempx1 == aboveAS.getX1()) && (tempy1 == aboveAS.getY1()) &&
		(tempx1 == aboveAS.getX2()) && (tempy1 == aboveAS.getY2()))
		{
			pos1=i;
		}
		if((tempx1 == belowAS.getX1()) && (tempy1 == belowAS.getY1()) &&
		(tempx1 == belowAS.getX2()) && (tempy1 == belowAS.getY2()))
		{
			pos2=i;
		}
		i++;
		if(pos1>=0 && pos2>=0)
		{
			tmp = aboveAS;
			sL.at(pos1)=belowAS;
			sL.at(pos2)=tmp;
		}
	}

}
int MakeBo::intersect(double sx1,double sy1,double ex2,double ey2,
		double sx3,double sy3,double ex4,double ey4,double x,double y)
{
  //return =0 OK, 1 lines parallel, 2 no intersection point
  //x,y intersection point
	double a1=0.0;
	double a2=0.0;
	double b1=0.0;
	double b2=0.0;
	double c1=0.0;
	double c2=0.0; // Coefficients of line
	double m=0.0;

	a1= ey2-sy1;
	b1= sx1-ex2;
	c1= ex2*sy1-sx1*ey2;
  //a1*x + b1*y + c1 = 0 is segment 1

	a2= ey4-sy3;
	b2= sx3-ex4;
	c2= ex4*sy3-sx3*ey4;
  //a2*x + b2*y + c2 = 0 is segment 2

	m= a1*b2-a2*b1;
	if (m == 0) return 1;

	x=(b1*c2-b2*c1)/m;
  // intersection range

	if(x < sx1 || x < sx3 || x > ex2 || x > ex4) return 2;

	y=(a2*c1-a1*c2)/m;


  return 0;
}

avlseg::AVLSegment* MakeBo::getAbove(double aktXPos,double aktYPos)
{
	//Suche in Segmente X-Koord gleich, Y-Koord groesser
	vector< avlseg::AVLSegment >::iterator row_it = sL.begin();
	vector< avlseg::AVLSegment >::iterator row_end = sL.end();

	double tempx1=0.0;
	double tempx2=0.0;
	double tempy1=0.0;
	double tempy2=0.0;
	for ( ; row_it != row_end; ++row_it)
	{

	    avlseg::AVLSegment tmpAS = *row_it;
	    tempx1=tmpAS.getX1();
		tempy1=tmpAS.getY1();
		tempx2=tmpAS.getX2();
		tempy2=tmpAS.getY2();

		if((tempx1 == aktXPos) && (tempy1 >= aktYPos))
			//|| ((tempx1 == aktXPos) && (tempy1 >= aktYPos)))
		{
			return &tmpAS;

		}
	}
	return NULL;
}
avlseg::AVLSegment* MakeBo::getBelow(double aktXPos,double aktYPos)
{
	//Suche in Segmente X-Koord gleich, Y-Koord groesser
	vector< avlseg::AVLSegment >::iterator row_it = sL.begin();
	vector< avlseg::AVLSegment >::iterator row_end = sL.end();

	double tempx1=0.0;
	double tempx2=0.0;
	double tempy1=0.0;
	double tempy2=0.0;
	for ( ; row_it != row_end; ++row_it)
	{

	    avlseg::AVLSegment tmpAS = *row_it;
	    tempx1=tmpAS.getX1();
		tempy1=tmpAS.getY1();
		tempx2=tmpAS.getX2();
		tempy2=tmpAS.getY2();

		if((tempx1 == aktXPos) && (tempy1 <= aktYPos))
			//|| ((tempx1 == aktXPos) && (tempy1 >= aktYPos)))
		{
			return &tmpAS;
		}
	}
	return NULL;
}

void MakeBo::checkIS(robustGeometry::BOEvent currEv,
		avlseg::AVLSegment currAS)
{
	double sx1=0.0;
	double sy1=0.0;
	double ex1=0.0;
	double ey1=0.0;
	double sx2=0.0;
	double sy2=0.0;
	double ex2=0.0;
	double ey2=0.0;
	double is_x=0.0;
	double is_y=0.0;
	int isIntersected = 1;

	avlseg::ownertype currOwner;
	avlseg::ownertype nextOwner;

	sx2 = currAS.getX1();
	sy2 = currAS.getY1();
	ex2 = currAS.getX2();
	ey2 = currAS.getY2();
	nextOwner = currAS.getOwner();

	sx1 = currEv.getX();
	sy1 = currEv.getY();
	ex1 = currEv.getExtendedHalfSegment().GetRightPoint().GetX();
	ey1 = currEv.getExtendedHalfSegment().GetRightPoint().GetY();
	currOwner = currEv.getOwner();

	//check owner first, second, todo kÃ¶nnten auch beide none sein
	if (currOwner != nextOwner)
	{
		isIntersected = intersect
				(sx1,sy1,ex1,ey1,sx2,sy2,ex2,ey2,is_x,is_y);
		if (isIntersected==0)
		{

			bool found_it = false;
			for (int i=0;i<events.size(); i++ )
			{
			//eliminate duplicate value
				if (( events[i].getX()== is_x ) &&
						( events[i].getY()== is_y )
					&& ( events[i].getPointType() ==
				robustGeometry::intersect ))
				{	found_it = true;
					break;
				}
			}
			if (!found_it)
			{
				robustGeometry::BOEvent
				intersP = robustGeometry::BOEvent(
				currAS.convertToExtendedHs( true,
						currAS.getOwner() ),
					currEv.getExtendedHalfSegment(),
					is_x,
					is_y,
					robustGeometry::intersect);
					events.push_back(intersP);
			}
			cout << "Check IS Intersect at : x "
					<< is_x << " y " << is_y << endl;
		}
	}
}
void MakeBo::planeSweepCase1(robustGeometry::BOEvent currEv)
{
	double sx1=0.0;
	double sy1=0.0;

	avlseg::AVLSegment* currAS=0;
	(currEv.getExtendedHalfSegment(),currEv.getOwner());
	sL.push_back(*currAS);

	//currEv muss startP sein, Schnittpb falsch, wenn s/e vertauscht?
	sx1 = currEv.getX();
	sy1 = currEv.getY();

	cout << "Case 1 X " << sx1 << " Y " << sy1;
	currAS = getAbove(sx1,sy1);
	if (currAS!=0)
	{
		cout << "Above : CurrAS X1 " << currAS->getX1()
				<< " Y1 " << currAS->getY1() <<
				" X2 " << currAS->getX2()
				<< " Y2 " << currAS->getY2();
		checkIS(currEv,*currAS);
	}

	currAS = getBelow(sx1,sy1);
	if (currAS!=0)
	{
		cout << "Below : CurrAS X1 " << currAS->getX1()
				<< " Y1 " << currAS->getY1() <<
		        " X2 " << currAS->getX2()
		        << " Y2 " << currAS->getY2();
		checkIS(currEv,*currAS);
	}
	cout << endl;
}

void MakeBo::planeSweepCase2(robustGeometry::BOEvent currEv)
{
	double sx1=0.0;
	double sy1=0.0;
	avlseg::AVLSegment* currAS=0;

	(currEv.getExtendedHalfSegment(),currEv.getOwner() );

	sL.erase( remove(sL.begin(),sL.end(),*currAS), sL.end() );

	sx1 = currEv.getX();
	sy1 = currEv.getY();

	cout << "Case 2 X " << sx1 << " Y " << sy1;
	currAS = getAbove(sx1,sy1);
	if (currAS!=0)
	{
		cout << "Above : CurrAS X1 " << currAS->getX1()
				<< " Y1 " << currAS->getY1() <<
				" X2 " << currAS->getX2()
			  << " Y2 " << currAS->getY2();
		checkIS(currEv,*currAS);
	}

	currAS = getBelow(sx1,sy1);
	if (currAS!=0)
	{
		cout << "Below : CurrAS X1 " << currAS->getX1()
				<< " Y1 " << currAS->getY1() <<
				" X2 " << currAS->getX2()
				 << " Y2 " << currAS->getY2();
		checkIS(currEv,*currAS);
	}
	cout << endl;
}
void MakeBo::planeSweepCase3(robustGeometry::BOEvent currEv)
{
	Point currIsP;
	double ix1=0.0;
	double iy1=0.0;

	avlseg::AVLSegment* currAS=0;

	//find corresponding segments
	avlseg::AVLSegment aboveEHS(currEv.getAboveEHS(),
			currEv.getOwner() );
	avlseg::AVLSegment belowEHS(currEv.getBelowEHS(),
			currEv.getOwner() );

	ix1 = currEv.getX();
	iy1 = currEv.getY();
	currIsP = Point(true, ix1,iy1);

	//if(!outputPoints->Contains(currIsP,geoid))
	*outputPoints += currIsP;

    //Swap their positions in SL so that belowEHS is now above aboveEHS;
	//search in SL, swap the entries
	findAndSwap(aboveEHS,belowEHS);

    //let segA = the segment above belowEHS in SL;

	cout << "Case 3 X " << ix1 << " Y " << iy1;
	currAS = getAbove(ix1,iy1);
	if (currAS!=0)
	{
		cout << "Above : CurrAS X1 "
		<< currAS->getX1() << " Y1 " << currAS->getY1() <<
		 " X2 " << currAS->getX2() << " Y2 "
		 << currAS->getY2();
		checkIS(currEv,*currAS);
	}

    //Let segB = the segment below aboveEHS in SL;
	currAS = getBelow(ix1,iy1);
	if (currAS!=0)
	{
		cout << "Below : CurrAS X1 " << currAS->getX1()
				<< " Y1 " << currAS->getY1() <<
				 " X2 " << currAS->getX2()
			  << " Y2 " << currAS->getY2();
		checkIS(currEv,*currAS);
	}
	cout << endl;

}

/*
intersection-operator for two line-objects

*/
void MakeBo::IntersectionBO
(const Line& line1, const Line& line2, Points& result)
{
	// Initialisation
	result.Clear();

	result.SetDefined(true);
	if(line1.Size()==0 || line2.Size()==0)
	{
		return; //empty line
	}

	//Initialize event queue: all segment endpoints
	//Sort x by increasing x and y
	  priority_queue<avlseg::ExtendedHalfSegment,
	                 vector<avlseg::ExtendedHalfSegment>,
	                 greater<avlseg::ExtendedHalfSegment> > q1;
	  priority_queue<avlseg::ExtendedHalfSegment,
	                 vector<avlseg::ExtendedHalfSegment>,
	                 greater<avlseg::ExtendedHalfSegment> > q2;

	  avltree::AVLTree<avlseg::AVLSegment> sss;
	  avlseg::ownertype owner;

	  int pos1 = 0;
	  int pos2 = 0;
	  avlseg::ExtendedHalfSegment nextExtHs;
	  int src = 0;

	  //const avlseg::AVLSegment* member=0;
	  //const avlseg::AVLSegment* leftN = 0;
	  //const avlseg::AVLSegment* rightN = 0;

	  avlseg::AVLSegment left1,right1,common1, left2,right2;
	  avlseg::AVLSegment tmpL,tmpR;
	  avlseg::ownertype boOwner;

	  result.StartBulkLoad();

	  //int i = 0;

	  while( ( owner=selectNext(line1,pos1,
 	  			                 line2,pos2,
 	  			                 q1,q2,
 	  			                 nextExtHs,
 	  			                 src))!= avlseg::none)
 	{
		  cout << "owner: " << owner << endl;
		  boOwner = owner;
		  cout << "boOwner: " << boOwner << endl;

 		bool found_it = false;

 		for (int i=0;i<events.size(); i++ )
 	  	{
 		//eliminate duplicate value
 			if (events[i].getExtendedHalfSegment().
 				GetLeftPoint() == nextExtHs.GetLeftPoint( ) &&
 				events[i].getExtendedHalfSegment().
				GetRightPoint() == nextExtHs.GetRightPoint( ))
			{	found_it = true;
			    break;
			}
		}
		if (found_it) continue;

		cout << "after eliminate duplicate value " <<  endl;
		cout << " nextExtHs.GetLeftPoint().IsDefined() : "
				<< nextExtHs.GetLeftPoint().IsDefined() << endl;
		cout << " nextExtHs.GetRightPoint().IsDefined() : "
		<< nextExtHs.GetRightPoint().IsDefined() << endl;
		robustGeometry::BOEvent nextEventl =
				robustGeometry::BOEvent(
			nextExtHs,
			nextExtHs.GetLeftPoint().GetX(),
			nextExtHs.GetLeftPoint().GetY(),
			robustGeometry::start,
			boOwner);

		cout << "before nextEventl " <<  endl;
		cout <<	 "nextEventl.getExtendedHalfSegment()."
				"GetLeftPoint().IsDefined()" << nextEventl.
				getExtendedHalfSegment().GetLeftPoint().
				IsDefined() << endl;
		cout <<	 "nextEventl.getExtendedHalfSegment()"
				".GetRightPoint().IsDefined()" << nextEventl.
				getExtendedHalfSegment().GetRightPoint().
				IsDefined() << endl;
		events.push_back(nextEventl);


		cout << "after nextEventl " <<  endl;

		robustGeometry::BOEvent nextEventr = robustGeometry::BOEvent(
			nextExtHs,
			nextExtHs.GetRightPoint().GetX(),
			nextExtHs.GetRightPoint().GetY(),
			robustGeometry::end,
			boOwner);

		events.push_back(nextEventr);
		cout << "after nextEventr " <<  endl;
 	};
	 cout << "before sortBOEvents " <<  endl;
	sort( events.begin(), events.end(), sortBOEvents);
	cout << "after sortBOEvents " <<  endl;

	for (int i=0;i<events.size(); i++ )
 	{
 		cout << "events:" << i << " "; events[i].Print(cout);
 		cout << endl;
 	};


	robustGeometry::BOEvent currEv;
	while (!events.empty())
	{
		currEv = events[0];
		int i =0;
		cout << "events:" << i << " "; currEv.Print(cout); cout << endl;

	   if(currEv.getPointType()==robustGeometry::start)
	   {
			planeSweepCase1(currEv);
	   }
	   else if(currEv.getPointType()==robustGeometry::end)
	   {
			planeSweepCase2(currEv);
	   }
	   else if(currEv.getPointType()==robustGeometry::intersect)
	   {
			planeSweepCase3(currEv);
	   }

		//cout << "line1:"; line1.Print(cout); cout << endl;
		//cout << "Pos1:" << pos1 << endl;
		//cout << "line2:"; line2.Print(cout); cout << endl;
		//cout << "Pos2:" << pos2 << endl;
		//cout << "currHs:"; currHs.Print(cout); cout << endl;
		//cout << "src:" << src << endl;

	//avlseg::AVLSegment current(nextHs,owner);
	//cout << "owner:" << owner << endl;
	//cout << "sss:"; sss.Print(cout); cout << endl;
	//member = sss.getMember(current,leftN,rightN);
	//if (member) {  cout << "member:";
	//member->Print(cout); cout << endl; };
	//cout << "AVLSegment current"; current.Print(cout); cout << endl;

	   sort( events.begin(), events.end(), sortBOEvents);
	   events.erase(events.begin());
	   //iter = v.erase(iter);

	}
	cout <<" outputPoints ";
	MakeBo:: outputPoints->Print(cout);  cout << endl;

	result = *outputPoints;
	cout << "result = "; result.Print(cout); cout << endl;
  //result.EndBulkLoad(true,false,false);
}

/*
~Intersection~ operation.

*/

static int intersectionBO_ll( Word* args, Word& result, int message,
Word& local, Supplier s )
{

   result = qp->ResultStorage( s );
   Line *line1 = ((Line*)args[0].addr);
   Line *line2 = ((Line*)args[1].addr);

   cerr << "line1 = "; line1->Print(cerr); cerr << endl;
   cerr << "line2 = "; line2->Print(cerr); cerr << endl;

   if (line1->IsDefined() && line2->IsDefined() )
   {
      if (line1->IsEmpty() || line2->IsEmpty() )
      {
    	  cout << __PRETTY_FUNCTION__ << ": lines are empty!"
    		           	                  << endl;
          ((Points *)result.addr)->SetDefined( false );
         return (0);
      }
      else if (line1->BoundingBox().IsDefined() &&
    		  line2->BoundingBox().IsDefined() )
      {
         if (line1->BoundingBox().Intersects(line2->BoundingBox()))
         {
            MakeBo bo;
            cout << __PRETTY_FUNCTION__ << ": BoundingBox Intersects!"
                  << endl;
            bo.IntersectionBO( *line1, *line2,
            	*static_cast<Points*>(result.addr));

            return(0);
         }
         else
         {
        	 cout << __PRETTY_FUNCTION__ << ": "
        			 "BoundingBox don't intersects!"
        	                  << endl;
            ((Points *)result.addr)->Clear();
            return (0);
         }
      }
      else
      {
    	 MakeBo bo;
    	 cout << __PRETTY_FUNCTION__ << ": BoundingBox isn't defined!"
    	                   << endl;
         bo.IntersectionBO( *line1, *line2,*static_cast<Points*>(result.addr));
         return(0);
      }
   }
   else
   {
	   cout << __PRETTY_FUNCTION__ << ": lines aren't defined!"
	           	                  << endl;
     ((Points *)result.addr)->Clear();
     ((Points *)result.addr)->SetDefined( false );

     return (0);
   }
}

/*
Operator Description

*/

struct intersectionInfo : OperatorInfo
{
	intersectionInfo()
	{
		name = "intersectionBO";
		signature = Line::BasicType() + " x " + Line::BasicType()
		+ " -> " + Points::BasicType();
		syntax = "_ intersectionBO _";
		meaning = "Intersection predicate for two Lines.";
	}
};

/*
Selection function ~RGSetOpSelect~
This select function is used for the ~intersectionBO~ operator.

*/
int
RGSetOpSelect(ListExpr args)
{
  string a1 = nl->SymbolValue(nl->First(args));
  string a2 = nl->SymbolValue(nl->Second(args));

  if(a1==Line::BasicType())
  {
    if(a2==Line::BasicType())   return 0;
    return -1;
  }

  return -1;
}

ValueMapping intersectionVM [] =  {intersectionBO_ll};
/*
Definition of the operators

*/
Operator test
( "intersectionBO", intersectionSpec,1,
intersectionVM,RGSetOpSelect,intersectionTM );

/*
Creating the Algebra

*/

class RobustGeometryAlgebra : public Algebra
{

	public:
	RobustGeometryAlgebra() : Algebra()
	{
		AddOperator( &test );
	}
	~RobustGeometryAlgebra() {};
};

/*
Algebra Initialization

*/

extern "C"
Algebra*
InitializeRobustGeometryAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RobustGeometryAlgebra());
}

