/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] header File of the Hybrid Trajectory Algebra

Started 2013, Hamza Issa\'{e}s


[TOC]

\section{Overview}
\section{Defines and Includes}

*/
#ifndef __HYBRID_TRAJECTORY_ALGEBRA__
#define __HYBRID_TRAJECTORY_ALGEBRA__

namespace hyt {

class BaseClass{

public :

	
	BaseClass();

	void incrementIndex();
	void resetIndex();


	virtual  void Get(Attribute** result)=0;
	virtual int GetNoComponents()=0;
	virtual bool isFinished()=0;
	virtual int getNextStart(Instant& ir,
  bool& leftIn,bool istarting=false)=0;
	virtual int getNearestEnd(Instant& ir,bool& rightIn)=0;
    virtual void setLastfinishInstant(Instant instant,bool included)=0;
	virtual void setLastStartInstant(Instant instant,bool included)=0;
	virtual void makeIncrementifOK()=0;
protected :

	int scaningIndex;
};

template <class Mapping,class Unit> class GeneralMapping:public BaseClass{
	
public:
	GeneralMapping(Mapping* elem):BaseClass(){
		this->mapp=elem;
		this->finished=false;
}

	int GetNoComponents(){
	return mapp->GetNoComponents();
}
	void Get(Attribute** result){
	
		Interval<Instant>tempresult(this->laststartingInstant,
this->lastfinishInstant,
this->limitstartInstantSemantic,this->limitfinsihSemantic);
		Unit r(true);

		if(this->finished){
			*result=new Unit(false);
			return;
		}
		this->mapp->Get(scaningIndex,r);
		
		Interval<Instant> iv1=r.getTimeInterval();
		bool lgc=(iv1.lc)||
 (iv1.lc==false&&this->limitstartInstantSemantic==false);

		bool rgc=(iv1.rc)||
  (iv1.rc==false&&this->limitstartInstantSemantic==false);

		bool inside=((iv1.start<this->laststartingInstant)||
   (iv1.start==this->laststartingInstant && lgc)) &&
			((iv1.end>this->laststartingInstant)||
   (iv1.end==this->laststartingInstant &&rgc));

		if( inside==false)
		{
			*result=new Unit(false);
			return;
		}

		Unit res(true);
		r.AtInterval(tempresult,res);
		*result=new Unit(res);
}
	void makeIncrementifOK(){
	
		if(this->finished==true)
			return;
		Unit result(true);
		this->mapp->Get(scaningIndex,result); 
		Interval<Instant> iv1=result.getTimeInterval();
		bool equalEnd= ( iv1.end==lastfinishInstant && 
   (iv1.rc==limitfinsihSemantic));
		if(equalEnd==true){
		   this->scaningIndex ++;
		   	if(this->scaningIndex>=mapp->GetNoComponents())
			this->finished=true;
	   }
	
	
	
}
	void setLastStartInstant(Instant instant,bool included){

		this->limitstartInstantSemantic=included;
		this->laststartingInstant=instant;
	
}
	void setLastfinishInstant(Instant instant,bool included){
	
		this->limitfinsihSemantic=included;
		this->lastfinishInstant=instant;
	
}
	int getNextStart(Instant& ir,bool& leftIn,bool istarting=false){
		
		if(this->finished)
			return -1;


		if(istarting==true){
			Unit result(true);
			this->mapp->Get(0,result);
			Interval<Instant> iv1=result.getTimeInterval();
			ir=iv1.start;
			leftIn=iv1.lc;
			return 0;
		}


		Unit result(true);
		this->mapp->Get(scaningIndex,result);
		Interval<Instant> iv1=result.getTimeInterval();

		bool lgc=(iv1.lc)||
 (iv1.lc==false&&this->limitfinsihSemantic==false) ;

		bool rgc=(iv1.rc)||
 (iv1.rc==false&&this->limitfinsihSemantic==false);

		bool inside=((iv1.start<this->lastfinishInstant)
              ||(iv1.start==this->lastfinishInstant && lgc)) &&
			          ((iv1.end>this->lastfinishInstant)
             ||(iv1.end==this->lastfinishInstant &&rgc));

		if(inside){
		    ir=lastfinishInstant;
			leftIn=!limitfinsihSemantic;
			return 1;
		}

			ir=iv1.start;
			leftIn=iv1.lc;

		return 0;

	
}

	int getNearestEnd(Instant& ir,bool& rightIn){
	
		if(this->finished)
			return -1;
			
	   	Unit result(true);
		this->mapp->Get(scaningIndex,result);
		Interval<Instant> iv1=result.getTimeInterval();

		
		bool lgc=(iv1.lc)||
 (iv1.lc==false&&this->limitstartInstantSemantic==false) ;

		bool rgc=(iv1.rc)||
 (iv1.rc==false&&this->limitstartInstantSemantic==false);
		
		bool inside=((iv1.start<this->laststartingInstant)
            ||(iv1.start==this->laststartingInstant && lgc)) 
            &&((iv1.end>this->laststartingInstant)
            ||(iv1.end==this->laststartingInstant &&rgc));

	
		if(inside){
		    ir=iv1.end;
			rightIn=iv1.rc;
			return 0;
		}

			
		ir=iv1.start;
		

	    rightIn=!iv1.lc;


		return 0;
	
}
	 bool isFinished(){
		return this->finished;	
}
private :
	Mapping* mapp;
	Instant lastfinishInstant;
	bool limitfinsihSemantic;
    

	Instant laststartingInstant;
	bool limitstartInstantSemantic;
	bool finished;
};

 class FullRefinment{

 private :


	 bool firsttime;

	 ListExpr ote;
	 int numberofAttribute;
	
	 Tuple* tuple;
	 vector<string> inType;

	 map<int,BaseClass*> totals;

	 void fillmaps();



 public:
	 ~FullRefinment();
	 FullRefinment(Tuple* args,vector<string> inputType,
   ListExpr outputTupleExpression);
	 void getNext(Tuple** result);
	 bool haveMoreUnits();

};

}
#endif