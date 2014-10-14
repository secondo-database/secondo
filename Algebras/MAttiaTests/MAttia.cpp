/*
 MAttia.cpp

  Created on: Jan 13, 2009
      Author: m.attia

*/

#include "MAttia.h"
#include "FTextAlgebra.h"
#include "SecondoInterface.h"
#include "fstream"

  void NDefUnit(MBool* arg, CcBool* nval, MBool* res)
  {
    bool debugme=false;
    res->Clear();
    res->StartBulkLoad();
    UBool uarg;
    UBool uBool(nval->GetBoolval()), uBool2(nval->GetBoolval());
    uBool.constValue= *nval;
    if(!arg->IsDefined()||arg->GetNoComponents() == 0)
    {
      res->SetDefined(false);
      return;
    }
    arg->Get(0, uarg);
    //uarg.Print(cout);
    uBool2.CopyFrom(&uarg);  
    res->MergeAdd(uBool2);
    //res->Print(cout);
    uBool.timeInterval.lc = !uarg.timeInterval.rc;
    uBool.timeInterval.start = uarg.timeInterval.end;
    for( int i = 1; i < arg->GetNoComponents(); i++) 
    {
      arg->Get(i, uarg);

      uBool.timeInterval.rc = !uarg.timeInterval.lc;
      uBool.timeInterval.end = uarg.timeInterval.start;

      if(uBool.timeInterval.start < uBool.timeInterval.end
          || (uBool.timeInterval.start == uBool.timeInterval.end
              && uBool.timeInterval.lc && uBool.timeInterval.rc))
      {
        res->MergeAdd(uBool);
        //res->Print(cout);
      }

        uBool2.CopyFrom(&uarg);  
        res->MergeAdd(uBool2);
        //res->Print(cout);
        uBool.timeInterval.lc = !uarg.timeInterval.rc;
        uBool.timeInterval.start = uarg.timeInterval.end;
    }
    res->EndBulkLoad(false);
    if(debugme)
    {
      cout<<"NDefUnit is called";
      cout<<"\nInput1:"; arg->Print(cout);
      cout<<"\nInput2:"; nval->Print(cout);
      cout<<"\nOutput:"; res->Print(cout);
      cout.flush();
    }
  }  

int Randint(int u)//Computes a random integer in the range 0..u-1,
//for u >= 2
{
  if ( u < 2 ) {u=2; srand ( time(NULL) );}
  // For u < 2 also initialize the random number generator
  // rand creates a value between [0,RAND_MAX]. The calculation procedure
  // below is recommended in the manpage of the rand() function.
  // Using rand() % u will yield poor results.
  return (int) ( (float)u * rand()/(RAND_MAX+1.0) );
}

string ToString( int number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}

string GenerateConnectedRandomConstraints(int numPreds, int numConstraints)
{
  string aliases[]={"a","b","c","d","e","f","g","h","i","j","k","l","m","n",
      "o","p","q","r","s","t","u","v","w","x","y","z"};

  string connectors[]= {"vec(\"aabb\")"  , "meanwhile", "vec(\"bbaa\")"  ,  
      "vec(\"aa.bb\")"  ,  "vec(\"bb.aa\")"  , "vec(\"abab\")"  ,  
      "vec(\"baba\")"   ,  "vec(\"baab\")"  ,  "vec(\"abba\")"  ,  
      "vec(\"a.bab\")"  ,  "vec(\"a.bba\")"  , "vec(\"baa.b\")" ,  
      "vec(\"aba.b\")"  , "immediately"    , "vec(\"a.ba.b\")"  ,  
      "vec(\"a.abb\")"  , "vec(\"a.a.bb\")"  , "vec(\"ba.ab\")" ,  
      "then",           "vec(\"bb.a.a\")"  ,  "vec(\"bba.a\")"  ,  
      "vec(\"b.baa\")"  , "vec(\"b.b.aa\")" , "vec(\"ab.ba\")"  ,  
      "vec(\"aa.b.b\")"  ,  "vec(\"aab.b\")"  ,  "follows", 
      "vec(\"a.ab.b\")"  ,  "vec(\"a.a.b.b\")"  ,"vec(\"b.ba.a\")",  "later"};

  string result= "";
  string str="";

  int* ConnectedSet= new int[numPreds];
  int c=0;
  int op1, op2, op3;
  int seed;
  int numConnected=0;

  while(true)
  {
    result= "";
    str="";
    numConnected=0;
    c=0;
    for(int i=0; i<numPreds;i++)
      ConnectedSet[i]=0;


    seed = Randint(numPreds);
    ConnectedSet[seed]=1;
    while(c++ < numConstraints)
    {
      do{
        op1= Randint(numPreds);
        op2= Randint(numPreds);
      }while(op1==op2 || (ConnectedSet[op1] == 0 && ConnectedSet[op2]==0));

      ConnectedSet[op1] = 1 ; ConnectedSet[op2]=1;
      op3=Randint(31);
      str= "stconstraint(\"" + aliases[op1] + "\", \"" + aliases[op2] 
             + "\", " + connectors[op3] + ")";
      if(result=="")
        result += str;
      else
        result += ", " +str;
    }

    for(int i=0; i<numPreds;i++)
      numConnected+= ConnectedSet[i];
    if(numConnected == numPreds)
      return result;

  }
  delete[] ConnectedSet;
  return "";
}

string GenerateRandSTPPExpr2(int alias)
{
  string aliases[]={"a","b","c","d","e","f","g","h","i","j","k","l","m","n",
      "o","p","q","r","s","t","u","v","w","x","y","z"};

  string pred="";
  int PredType= Randint(3);
  int op1, op2;

  switch(PredType)
  {
  case 0: //distance
    op1= Randint(300); //RandPoint
    op2= Randint(50);  //RandDistance
    pred= "distance(trip, point" + ToString(op1+1) + ")< " + 
    ToString(op2+1)+ ".0 as " + aliases[alias];
    break;
  case 1: //speed
    op1= Randint(30); //RandSpeed
    pred= "speed(trip)> "+ ToString(op1) + ".0 as " + aliases[alias];
    break;
  case 2:
    op1= Randint(16); //RandMSnow
    pred= "trip inside mreg" + ToString(op1)+ " as " + aliases[alias];
    break;
  }
  return pred;
}

bool GenerateSTPQExperiment2Queries(string selectfrom, int count, 
    int numpreds[], int numconstraints[], int numQueries[], 
    const char* filename)
{

  //string aliase[]={"a","b","c","d","e","f","g","h","i","j","k","l","m","n",
  //    "o","p","q","r","s","t","u","v","w","x","y","z"};
  // string ints[]= {"1","2","3","4","5","6","7","8","9","10",
  //    "11","12","13","14","15","16","17","18","19","20",
  //    "21","22","23","24","25","26","27","28","29","30",
  //    "31","32","33","34","35","36","37","38","39","40",
  //    "41","42","43","44","45","46","47","48","49","50",
  //    "51","52","53","54","55","56","57","58","59","60",
  //    "61","62","63","64","65","66","67","68","69","70",
  //    "71","72","73","74","75","76","77","78","79","80",
  //    "81","82","83","84","85","86","87","88","89","90",
  //    "91","92","93","94","95","96","97","98","99","100"};
  //string connectors[]= {"vec(\"aabb\")"  , "meanwhile", "vec(\"bbaa\")"  ,  
  //    "vec(\"aa.bb\")"  ,  "vec(\"bb.aa\")"  ,
  //    "vec(\"abab\")"  ,  "vec(\"baba\")"  ,  "vec(\"baab\")"  ,  
  //    "vec(\"abba\")"  ,  "vec(\"a.bab\")"  ,  "vec(\"a.bba\")"  ,
  //    "vec(\"baa.b\")"  ,  "vec(\"aba.b\")"  , "immediately", 
  //    "vec(\"a.ba.b\")"  ,  "vec(\"a.abb\")"  ,  "vec(\"a.a.bb\")"  ,
  //    "vec(\"ba.ab\")"  ,  "then", "vec(\"bb.a.a\")"  ,  "vec(\"bba.a\")"  ,  
  //    "vec(\"b.baa\")"  ,  "vec(\"b.b.aa\")"  ,
  //    "vec(\"ab.ba\")"  ,  "vec(\"aa.b.b\")"  ,  "vec(\"aab.b\")"  ,  
  //    "follows", "vec(\"a.ab.b\")"  ,  "vec(\"a.a.b.b\")"  ,
  //    "vec(\"b.ba.a\")",  "later"};
  string query="";


  ofstream out(filename);
  string RandPred="";
  for(int c=0; c<count; c++)
  {
    assert(numpreds[c]<26);
    for(int q=0;q<numQueries[c]; q++)
    {
      query= selectfrom + " where pattern([";
      int i;
      for(i=0; i<numpreds[c]-1;i++)  //RandPred
      {
        RandPred= GenerateRandSTPPExpr2(i);
        query+= RandPred + ", ";
      }
      RandPred= GenerateRandSTPPExpr2(i);
      query+= RandPred + "], [";

//      for( i=0; i<numconstraints[c]-1;i++) //RandConstraint
//      {
//        query+= "stconstraint(";
//        int op1 = Randint(numpreds[c]);
//        int op2 = Randint(numpreds[c]);
//        while(op1==op2)
//          op2 = Randint(numpreds[c]);
//        int conn= Randint(31);
//        query+= "\"" + aliase[op1] + "\"," +"\"" + aliase[op2] + "\"," + 
//        connectors[conn]+ "),";
//      }
//
//      query+= "stconstraint(";
//      int op1 = Randint(numpreds[c]);
//      int op2 = Randint(numpreds[c]);
//      while(op1==op2)
//        op2 = Randint(numpreds[c]);
//      int conn= Randint(31);
//      query+= "\"" + aliase[op1] + "\"," +"\"" + aliase[op2] + "\"," + 
//      connectors[conn]+ ")";

      string constraint_str= GenerateConnectedRandomConstraints(numpreds[c],
          numconstraints[c]);
      query+=   constraint_str + "]) ";

      out<< "\nrunSTPQExpr2("<< numpreds[c]<<", "<< numconstraints[c]<<
      ", "<<q << ", "<< query<<").";
      out.flush();

      cout<<endl<<query<<endl;
      cout.flush();
    }
  }
  out.close();
  return true;   
}

bool RunSTPQExperiment1Queries(string selectfrom, int count, 
    int numpreds[], int numconstraints[], int numQueries[])
{

  string aliase[]={"a","b","c","d","e","f","g","h","i","j","k","l","m","n",
      "o","p","q","r","s","t","u","v","w","x","y","z"};
  string ints[]= {"1","2","3","4","5","6","7","8","9","10",
      "11","12","13","14","15","16","17","18","19","20",
      "21","22","23","24","25","26","27","28","29","30"};

  string query="";
  srand ( time(NULL) );
  ListExpr res=nl->TheEmptyList();
  SecErrInfo err;
  SecondoInterface sec;
  string sres;
  StopWatch qqueryTime;
  ofstream out("STPQ.txt");
  ofstream stat("STPQ_Stat.txt");
  stat<<"#preds\t#cons\tquery#\tTTime\tCPUTime";
  double sumTime=0, sumCPU=0;
  double queryReal;
  double queryCPU;
  for(int c=0; c<count; c++)
  {
    assert(numpreds[c]<26);
    out<<"\n==============================================\n"
    <<"Number of predicates: " <<numpreds[c]<<"\t Number of constraints:"
    <<numconstraints[c]<<"\t Number of examples: "<<numQueries[c]<<endl;
    sumTime=0, sumCPU=0;
    for(int q=0;q<numQueries[c]; q++)
    {
      query= selectfrom + " filter [.stpattern[";
      int i;
      for(i=0; i<numpreds[c]-1;i++)
      {
        int index= Randint(30);
        query+= " " + aliase[i] + ": passmbool(mb" + ints[index] + "), ";
      }
      int index= Randint(30);
      query+= " " + aliase[i] + ": passmbool(mb" + ints[index] + "); ";

//      for( i=0; i<numconstraints[c]-1;i++)
//      {
//        query+= "stconstraint(";
//        int op1 = Randint(numpreds[c]);
//        int op2 = Randint(numpreds[c]);
//        while(op1==op2)
//          op2 = Randint(numpreds[c]);
//        int conn= Randint(31);
//        query+= "\"" + aliase[op1] + "\"," +"\"" + aliase[op2] + "\"," + 
//        connectors[conn]+ "),";
//      }
//
//      query+= "stconstraint(";
//      int op1 = Randint(numpreds[c]);
//      int op2 = Randint(numpreds[c]);
//      while(op1==op2)
//        op2 = Randint(numpreds[c]);
//      int conn= Randint(31);
//      query+= "\"" + aliase[op1] + "\"," +"\"" + aliase[op2] + "\"," + 
//      connectors[conn]+ ")";

      string constraint_str= 
        GenerateConnectedRandomConstraints(numpreds[c],numconstraints[c]);
      query+=  constraint_str +  "]] count \n";

      out<<endl<<query<<endl;
      out.flush();

      cout<<endl<<query<<endl;
      cout.flush();

      try{
        qqueryTime.start();
        sec.Secondo(query, res, err);   

        queryReal = qqueryTime.diffSecondsReal();
        queryCPU = qqueryTime.diffSecondsCPU();
        out<< "\nMeasured Times (elapsed / cpu): " << queryReal << " / " 
        << queryCPU << endl<<"================================\n";
        stat<<numpreds[c]<<"\t"<<numconstraints[c]<<"\t"<<q<<"\t"<<queryReal
        <<"\t"<<queryCPU<<endl;
        sumTime+= queryReal;
        sumCPU+= queryCPU;
      }
      catch(...)
      {
        out.close();
        stat.close();
      }  
    }
    stat<<endl<<"Total Time:"<< sumTime<<"\tAvg Time:"
    <<sumTime/numQueries[c];
    stat<<endl<<"\tTotal CPU:"<< sumCPU<<"\tAvg CPU:"
    <<sumCPU/numQueries[c]<<endl<<endl;
  }
  out.close();
  stat.close();
  return true;   
}


ListExpr RandomMBoolTypeMap(ListExpr args)
{
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  if(!Instant::checkType(nl->First(args))){
    return listutils::typeError("instant expected");
  }
  return nl->SymbolAtom("mbool");
}

ListExpr PassMBoolTypeMap(ListExpr args)
{
  //cout<<nl->ToString(args);
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  if(!MBool::checkType(nl->First(args))){
    return listutils::typeError("mbool expected");
  }
  return nl->SymbolAtom("mbool");
}


ListExpr RunSTPQExperiment1QueriesTM(ListExpr args)
{
  //cout<<nl->ToString(args);
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  if(!CcString::checkType(nl->First(args))){
    return listutils::typeError("string expected");
  }
  return nl->SymbolAtom("bool");
}


ListExpr RunSTPQExperiment2QueriesTM(ListExpr args)
{
  //cout<<nl->ToString(args);
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  if(!CcString::checkType(nl->First(args))){
    return listutils::typeError("string expected");
  }
  return nl->SymbolAtom("bool");
}

/*
~NDefUnitTypeMap~

signatures:
  mbool x bool -> mbool

*/
ListExpr NDefUnitTM(ListExpr args){
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("Two argument expected");
    return nl->SymbolAtom( "typeerror" );
  }
  if(nl->IsEqual(nl->First(args),"mbool") && 
      nl->IsEqual(nl->Second(args),"bool")){
    return   nl->SymbolAtom("mbool");
  }
  ErrorReporter::ReportError("mbool x bool expected");
  return nl->SymbolAtom( "typeerror" );
}

ListExpr RandomShiftDelayTM( ListExpr args )
{
  string err = " operator expects (mpoint duration real real)";
  if(!nl->HasLength(args,4)){
    return listutils::typeError("wrong number of arguments");
  }
  if(    !MPoint::checkType(nl->First(args))
      || !Duration::checkType(nl->Second(args))
      || !CcReal::checkType(nl->Third(args))
      || !CcReal::checkType(nl->Fourth(args))){
     return listutils::typeError(err);
   }
   return (nl->SymbolAtom("mpoint"));
}

ListExpr TestTM(ListExpr args){
  return nl->SymbolAtom( "bool" );
}

void CreateRandomMBool(Instant starttime, MBool& result)
{
  bool debugme=false,bval=false;
  result.Clear();
  int rnd,i=0,n;
  UBool unit(true);
  Interval<Instant> intr(starttime, starttime, true, false);

  Instant tstart(starttime),tendtstart(starttime);
  rnd=rand()%20;
  n=++rnd;
  while(i++<n)
  {
    rnd=rand()%50000;
    while(rnd<2)
      rnd=rand()%50000;
    intr.end.Set(intr.start.GetYear(), intr.start.GetMonth(),
        intr.start.GetGregDay(), intr.start.GetHour(),intr.start.GetMinute(),
        intr.start.GetSecond(),intr.start.GetMillisecond()+rnd);
    //bval= ((rand()%2048)>1200);
    unit.constValue.Set(true, bval);
    unit.timeInterval= intr;
    result.Add(unit);
    intr.start= intr.end;
    bval=!bval;
  }
  if(debugme)
    result.Print(cout);
}
int RandomMBool(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  MBool* res = (MBool*) result.addr;
  DateTime* tstart = (DateTime*) args[0].addr;
  CreateRandomMBool(*tstart,*res);
  return 0;
}

int PassMBool(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  MBool* res = (MBool*) result.addr;
  MBool* inp = (MBool*) args[0].addr;
  res->CopyFrom(inp);
  return 0;
}


void RandomShiftDelay( const MPoint* actual, const Instant* threshold, 
    double dx, double dy, MPoint& res)
{
  bool debugme= false;

  MPoint delayed(actual->GetNoComponents());
  UPoint first(0), next(0);
  UPoint *shifted,*temp, *cur;
  int xshift=0, yshift=0;
  int rmillisec=0, rday=0;
  actual->Get( 0, first );
  cur=new UPoint(first);
  for( int i = 1; i < actual->GetNoComponents(); i++ )
  {
    actual->Get( i, next );

    rmillisec= rand()% threshold->GetAllMilliSeconds();
    rday=0;
    if(threshold->GetDay()> 0) rday = rand()% threshold->GetDay();
    DateTime delta(rday,rmillisec,durationtype) ;
    xshift= rand() % ((int)dx +1);
    yshift= rand() % ((int)dy +1);
    shifted= new UPoint(*cur);
    delete cur;
    temp= new UPoint(next);
    if(rmillisec > rand()%24000 )
    {
      if((shifted->timeInterval.end + delta) <  next.timeInterval.end )
      {
        shifted->timeInterval.end += delta ;
        temp->timeInterval.start= shifted->timeInterval.end;
        shifted->p1.Set(shifted->p1.GetX()+xshift, shifted->p1.GetY()+yshift);
        temp->p0.Set(shifted->p1.GetX(), shifted->p1.GetY());
      }
    }
    else
    {
      if((shifted->timeInterval.end - delta) >shifted->timeInterval.start)
      {
        shifted->timeInterval.end -= delta ;
        temp->timeInterval.start= shifted->timeInterval.end;
        shifted->p1.Set(shifted->p1.GetX()-xshift, shifted->p1.GetY()-yshift);
        temp->p0.Set(shifted->p1.GetX(), shifted->p1.GetY());
      }
    }
    cur=temp;
    if(debugme)
    {
      cout.flush();
      cout<<"\n original "; cur->Print(cout);
      cout<<"\n shifted " ; shifted->Print(cout);
      cout.flush();
    }
    delayed.Add(*shifted);
    delete shifted;
  }
  delayed.Add(*temp);
  delete temp;
  res.CopyFrom(&delayed);
  if(debugme)
  {
    res.Print(cout);
    cout.flush();
  }
  return;
}

/*
Value mapping function for the operator ~ndefunit~

*/
int NDefUnitVM( ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  bool debugme=false;
  MBool *arg = static_cast<MBool*>( args[0].addr );
  CcBool *ndefval = static_cast<CcBool*>( args[1].addr );

  MBool* tmp=new MBool(0);
  result= qp->ResultStorage(s);
  MBool* res= (MBool*) result.addr;
  NDefUnit(arg, ndefval, res);
  //res->CopyFrom(tmp);
  tmp->Destroy();
  delete tmp;
  if(debugme)
  {
    cout.flush();
    res->Print(cout);
    cout.flush();
  }
  return 0;
}




int RunSTPQExperiment1QueriesVM(Word* args, Word& result, 
    int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  CcString* selectfrom = (CcString*) args[0].addr;
  int numpreds[]={2,2,2,2,
      3,3,3,3,3,
      4,4,4,4,4,4,  
      5,5,5,5,5,5,5,  
      6,6,6,6,6,6,6,6,    
      7,7,7,7,7,7,7,7,7,   
      8,8,8,8,8,8,8,8,8,8
  };
  int numconss[]={1,2,3,4,
      2,4,3,5,6,
      3,4,5,6,7,8,
      4,5,6,7,8,9,10,
      5,6,7,8,9,10,11,12,
      6,7,8,9,10,11,12,13,14,
      7,8,9,10,11,12,13,14,15,16
  };
  int munexmpl[]={100,100,100,100,100,100,100,100,100,100,100,100,100,
      100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
      100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
      100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
      100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
      100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100}; 


  res->Set(true, RunSTPQExperiment1Queries(selectfrom->GetValue(), 49,  
      numpreds, numconss, munexmpl));

  return 0;
}



int RunSTPQExperiment2QueriesVM(Word* args, Word& result, 
    int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  //CcString* filename = (CcString*) args[0].addr;
  int numpreds[]={2,2,2,2,
      3,3,3,3,3,
      4,4,4,4,4,4,  
      5,5,5,5,5,5,5,  
      6,6,6,6,6,6,6,6,    
      7,7,7,7,7,7,7,7,7,   
      8,8,8,8,8,8,8,8,8,8
  };
  int numconss[]={1,2,3,4,
      2,4,3,5,6,
      3,4,5,6,7,8,
      4,5,6,7,8,9,10,
      5,6,7,8,9,10,11,12,
      6,7,8,9,10,11,12,13,14,
      7,8,9,10,11,12,13,14,15,16
  };
  int munexmpl[]={10,10,10,10,10,10,10,10,10,10,10,10,10,
      10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
      10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
      10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
      10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
      10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}; 

  string file = "/home/mattia/Desktop/SQLExpr3.txt";
  res->Set(true, 
      GenerateSTPQExperiment2Queries("select count(*) from trains ", 49,  
          numpreds, numconss, munexmpl, file.c_str()));
  return 0;
}

int RandomShiftDelayVM(ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  MPoint *pActual = static_cast<MPoint*>( args[0].addr );
  Instant *threshold = static_cast<Instant*>(args[1].addr );
  double dx = static_cast<CcReal*>(args[2].addr )->GetRealval();
  double dy = static_cast<CcReal*>(args[3].addr )->GetRealval();
  
  
  MPoint* shifted = (MPoint*) qp->ResultStorage(s).addr;

  if(pActual->GetNoComponents()<2 || !pActual->IsDefined())
    shifted->CopyFrom(pActual);
    else
    {
      RandomShiftDelay(pActual, threshold, dx, dy, *shifted);
    }
  result= SetWord(shifted); 
  //This looks redundant but it is really necessary. After 2 hours of 
  //debugging, it seems that the "result" word is not correctly set 
  //by the query processor to point to the results.

  return 0;
}


int StretchVM( ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  //bool debugme=false; 
  //MPoint *mpoint = static_cast<MPoint*>( args[0].addr );
  //MPoint *mpref  = static_cast<MPoint*>( args[1].addr );
  //int numAnchors = static_cast<CcInt*>( args[2].addr )->GetIntval();
  //double distThreshold = static_cast<CcReal*>( args[3].addr )->GetRealval();
  
  return 0;
}

int TestVM( ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  //bool debugme=false;
  result = qp->ResultStorage(s);
  CcBool* res= static_cast<CcBool*>(result.addr);
//
//  list<int> lst;
//
//  for(int i=0; i<10; ++i)
//    lst.push_back(i);
//  list<int>::iterator it= lst.begin();
//  cerr<<*it<<endl;
//  ++it;
//  cerr<<*it<<endl;
//
//  for(int i=100; i<103; ++i)
//    lst.push_front(i);
//
//  lst.erase(it++);
//  cerr<<*it<<endl;

  
  DbArray<Page> dbarr(1);
  Page val1;
  for(int i=0; i<4000; ++i)
       val1.ar[i]= 'C';
  dbarr.Put(0,val1);
  Page val2('a');
  dbarr.Get(0,val2);
  cerr<<val2.ar;
  res->Set(true, true);
  return 0;
}

const string PatternSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream x) ((map1 x mbool1)...(mapn x mbooln))) -> "
  "(stream x)</text--->"
  "<text>_ pattern [ fun1,...,funn ]</text--->"
  "<text>Only tuples, fulfilling the specified pattern "
  "are passed on to the output "
  "stream.</text--->"
  "<text>under construction </text--->"
  ") )";

const string RandomShiftDelaySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mpoint x duration x real x real-> mpoint</text--->"
  "<text>randomdelay(schedule, delay_threshold, dx, dy)</text--->"
  "<text>Given an mpoint and a duration value, the operator randomly shift the" 
  "start and end intstants of every unit in the mpoint. This gives the "
  "effect of having positive and negative delays and spatial shifts in the "
  "movement. The random shift values are bound by the given threshold."
  "</text---> <text>query randomdelay(train7)</text--->"
  ") )";

const string TestSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> -> bool</text--->"
  "<text>query test()</text--->"
  "<text>A dummy opertor to test code blocks. The idea is to have a fast way to"
  " code blocks. This operator has a type map, value map, spec, ..., and all "
  "other necessary functions for a SECONDO operator. One just needs to place "
  "the code block in the value mapping function and let it yield true if the "
  "code block works fine, false otherwise. test  </text---> "
  "<text>query test()</text--->"
  ") )";
/*

4.1.4 Definition of operator ~pattern~

*/
Operator opdefrandmbool (
    "randommbool",         // name
    PatternSpec,       // specification
    RandomMBool,           // value mapping
    Operator::SimpleSelect, // trivial selection function
    RandomMBoolTypeMap    // type mapping
);
Operator opdefpassmbool (
    "passmbool",         // name
    PatternSpec,       // specification
    PassMBool,           // value mapping
    Operator::SimpleSelect, // trivial selection function
    PassMBoolTypeMap    // type mapping
);



Operator opRunSTPQExperiment1Queries(
    "runstpqexpr1",         // name
    PatternSpec,       // specification
    RunSTPQExperiment1QueriesVM,           // value mapping
    Operator::SimpleSelect, // trivial selection function
    RunSTPQExperiment1QueriesTM    // type mapping
);

Operator opRunSTPQExperiment2Queries(
    "runstpqexpr2",         // name
    PatternSpec,       // specification
    RunSTPQExperiment2QueriesVM,           // value mapping
    Operator::SimpleSelect, // trivial selection function
    RunSTPQExperiment2QueriesTM    // type mapping
);

OperatorInfo NDefUnitOperatorInfo( "ndefunit",
    "mbool x bool -> mbool",
    "ndefunit(mdirection(train7) > 90.0, TRUE)",
    "Replaces the undefined periods within a moving constant with a constant "
    "unit having the given value",
"");
Operator ndefunit( NDefUnitOperatorInfo,
    NDefUnitVM,
    NDefUnitTM);

Operator randomshiftdelay (
    "randomshiftdelay",               // name
    RandomShiftDelaySpec,             // specification
    RandomShiftDelayVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    RandomShiftDelayTM          // type mapping
);

Operator test (
    "mytest",               // name
    TestSpec,             // specification
    TestVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    TestTM          // type mapping
);


MAttiaAlgebra::MAttiaAlgebra():Algebra() {
  // TODO Auto-generated constructor stub

/*

5.2 Registration of Types


*/


/*
5.3 Registration of Operators

*/
  //AddOperator(&opdefrandmbool);
  //AddOperator(&opdefpassmbool);
  AddOperator(&opRunSTPQExperiment1Queries);
  AddOperator(&opRunSTPQExperiment2Queries);
  AddOperator(&ndefunit);
  AddOperator(&randomshiftdelay);
  AddOperator(&test);
}

MAttiaAlgebra::~MAttiaAlgebra() {
  // TODO Auto-generated destructor stub
}



extern "C"
Algebra*
InitializeMAttiaAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new MAttiaAlgebra;
    }
