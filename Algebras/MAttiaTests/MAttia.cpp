/*
 MAttia.cpp

  Created on: Jan 13, 2009
      Author: m.attia

*/

#include "MAttia.h"
#include "FTextAlgebra.h"
#include "SecondoInterface.h"
#include <fstream>
#include "DateTime.h"
#include "ListUtils.h"
#include <string>

string ToString( int number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}

string ToString( double number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}


int ExportMPoint(MPoint* arg, Instant* startTime, Instant* timeUnit, 
    string comma, string& result)
{
  bool debugme=false;
  if(debugme)
  {
    timeUnit->Print(cout);
    startTime->Print(cout);
  }
  Periods argDefTime(0);
  Instant endInstant(instanttype), curInstant(*startTime);
  result="";
  Point argPoint(0,0);
  Intime<Point> argInTime(*startTime, argPoint);
  int count=0;
  
  arg->DefTime(argDefTime);
  argDefTime.Maximum(endInstant);
  
  while(curInstant < endInstant)
  {
      if(argDefTime.Contains(curInstant))
      {
        arg->AtInstant(curInstant, argInTime);
        argPoint= static_cast<Point>(argInTime.value);
        result+= comma + ToString(argPoint.GetX()) + comma +
                 ToString(argPoint.GetY());
      }
      else
      {
        result+= comma + "undef"+ comma + "undef";
      }
      curInstant.Add(timeUnit);
      count++;
      if(debugme)
      {
        cout<< endl<< result <<endl;
        curInstant.Print(cout);
        cout.flush();
      }
  }
  return count;
}




  void NDefUnit(MBool* arg, CcBool* nval, MBool* res)
  {
    bool debugme=true;
    res->Clear();
    res->StartBulkLoad();
    const UBool *uarg;
    UBool uBool(nval->GetBoolval()), uBool2(nval->GetBoolval());
    uBool.constValue= *nval;
    if(!arg->IsDefined()||arg->GetNoComponents() == 0)
    {
      res->SetDefined(false);
      return;
    }
    arg->Get(0, uarg);
    uarg->Print(cout);
    uBool2.CopyFrom(uarg);  
    res->MergeAdd(uBool2);
    res->Print(cout);
    uBool.timeInterval.lc = !uarg->timeInterval.rc;
    uBool.timeInterval.start = uarg->timeInterval.end;
    for( int i = 1; i < arg->GetNoComponents(); i++) 
    {
      arg->Get(i, uarg);

      uBool.timeInterval.rc = !uarg->timeInterval.lc;
      uBool.timeInterval.end = uarg->timeInterval.start;

      if(uBool.timeInterval.start < uBool.timeInterval.end
          || (uBool.timeInterval.start == uBool.timeInterval.end
              && uBool.timeInterval.lc && uBool.timeInterval.rc))
      {
        res->MergeAdd(uBool);
        res->Print(cout);
      }

        uBool2.CopyFrom(uarg);  
        res->MergeAdd(uBool2);
        res->Print(cout);
        uBool.timeInterval.lc = !uarg->timeInterval.rc;
        uBool.timeInterval.start = uarg->timeInterval.end;
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
    int numpreds[], int numconstraints[], int numQueries[], char* filename)
{

  string aliase[]={"a","b","c","d","e","f","g","h","i","j","k","l","m","n",
      "o","p","q","r","s","t","u","v","w","x","y","z"};
  string ints[]= {"1","2","3","4","5","6","7","8","9","10",
      "11","12","13","14","15","16","17","18","19","20",
      "21","22","23","24","25","26","27","28","29","30",
      "31","32","33","34","35","36","37","38","39","40",
      "41","42","43","44","45","46","47","48","49","50",
      "51","52","53","54","55","56","57","58","59","60",
      "61","62","63","64","65","66","67","68","69","70",
      "71","72","73","74","75","76","77","78","79","80",
      "81","82","83","84","85","86","87","88","89","90",
      "91","92","93","94","95","96","97","98","99","100"};
  string connectors[]= {"vec(\"aabb\")"  , "meanwhile", "vec(\"bbaa\")"  ,  
      "vec(\"aa.bb\")"  ,  "vec(\"bb.aa\")"  ,
      "vec(\"abab\")"  ,  "vec(\"baba\")"  ,  "vec(\"baab\")"  ,  
      "vec(\"abba\")"  ,  "vec(\"a.bab\")"  ,  "vec(\"a.bba\")"  ,
      "vec(\"baa.b\")"  ,  "vec(\"aba.b\")"  , "immediately", 
      "vec(\"a.ba.b\")"  ,  "vec(\"a.abb\")"  ,  "vec(\"a.a.bb\")"  ,
      "vec(\"ba.ab\")"  ,  "then", "vec(\"bb.a.a\")"  ,  "vec(\"bba.a\")"  ,  
      "vec(\"b.baa\")"  ,  "vec(\"b.b.aa\")"  ,
      "vec(\"ab.ba\")"  ,  "vec(\"aa.b.b\")"  ,  "vec(\"aab.b\")"  ,  
      "follows", "vec(\"a.ab.b\")"  ,  "vec(\"a.a.b.b\")"  ,
      "vec(\"b.ba.a\")",  "later"};
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
  cout<<nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 1 &&
   nl->IsAtom(nl->First(args)) && (nl->SymbolValue(nl->First(args))== "instant")
   ,"Operator randommbool expects one parameter.");
  return nl->SymbolAtom("mbool");
}

ListExpr PassMBoolTypeMap(ListExpr args)
{
  cout<<nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 1 &&
    nl->IsAtom(nl->First(args)) && (nl->SymbolValue(nl->First(args))== "mbool")
    , "Operator passmbool expects one parameter.");
  return nl->SymbolAtom("mbool");
}


ListExpr RunSTPQExperiment1QueriesTM(ListExpr args)
{
  cout<<nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 1 &&
   nl->IsAtom(nl->First(args)) && (nl->SymbolValue(nl->First(args))== "string") 
    , "Operator RunSTPQExperiment1QueriesTM expects one parameter.");
  return nl->SymbolAtom("bool");
}


ListExpr RunSTPQExperiment2QueriesTM(ListExpr args)
{
  cout<<nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 1 &&
    nl->IsAtom(nl->First(args)) && (nl->SymbolValue(nl->First(args))== "string")
    , "Operator RunSTPQExperiment1QueriesTM expects one parameter.");
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

ListExpr ExportMPointsTM(ListExpr args){
  if(nl->ListLength(args)!=5){
    ErrorReporter::ReportError("Five argument expected");
    return nl->SymbolAtom( "typeerror" );
  }
  ListExpr tmp;
  int posMP=0, posID=0;
  if(listutils::isTupleStream(nl->First(args)) &&
      listutils::isSymbol(nl->Second(args))  &&
      listutils::isSymbol(nl->Third(args)) &&
      listutils::isSymbol(nl->Fourth(args), "duration")  &&
      listutils::isSymbol(nl->Fifth(args), "string"))
  {
    posMP= listutils::findAttribute(
          nl->Second(nl->Second(nl->First(args))), 
          nl->ToString(nl->Second(args)), tmp);
    posID= listutils::findAttribute(
              nl->Second(nl->Second(nl->First(args))), 
              nl->ToString(nl->Third(args)), tmp);
    
    if(posMP != 0 && posID != 0 && nl->IsEqual(tmp, "int") )
      return   nl->ThreeElemList( nl->SymbolAtom("APPEND"),
                   nl->TwoElemList( nl->IntAtom(posMP), nl->IntAtom(posID)),
                   nl->SymbolAtom("int"));
  }
    
  ErrorReporter::ReportError("stream(tuple X) x string x string x duration x "
      "string expexted\nbut got:" + nl->ToString(args) );
  return nl->SymbolAtom( "typeerror" );
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

/*
Value mapping function for the operator ~ndefunit~

*/
int NDefUnitVM( ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  bool debugme=true;
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

/*
Value mapping function for the operator ~exportmpoints~

*/
int ExportMPointsVM( ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  bool debugme=true;
  bool first=true;
  Word elem;
  int countMP = 0, countP=0 ;
  Instant startTime(instanttype), tmpInstant(instanttype);
  Intime<Point> startInTime(startTime, new Point(0,0));
  string line="";
  
  qp->Open(args[0].addr);
  Instant* timeUnit= static_cast<Instant*>(args[3].addr);
  string fileName= static_cast<CcString*>(args[4].addr)->GetValue();
  int posMP = static_cast<CcInt*>(args[5].addr)->GetIntval();
  int posID = static_cast<CcInt*>(args[6].addr)->GetIntval();
  timeUnit->Print(cout);
  ofstream exportFile;
  exportFile.open(fileName.c_str() , ios::out);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    MPoint* mp= dynamic_cast<MPoint*>
                  ((static_cast<Tuple*>(elem.addr))->GetAttribute(posMP-1));
    mp->Initial(startInTime);
    tmpInstant= startInTime.instant;
    if(first)
    {
      first= false;
      startTime= tmpInstant;
    }
    assert(startTime <= tmpInstant);
    countP= ExportMPoint(mp, &startTime, timeUnit, ":", line);
    
    int ID= dynamic_cast<CcInt*>
        ((static_cast<Tuple*>(elem.addr))->GetAttribute(posID-1))->GetIntval();
    exportFile<< "obj:" << ID << ":"<< countP << line << endl;
    
    static_cast<Tuple*>(elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
    countMP++;
  }
  exportFile.close();
  result = qp->ResultStorage(s);
  static_cast<CcInt*>(result.addr)->Set(true, countMP);
  qp->Close(args[0].addr);
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
  CcString* filename = (CcString*) args[0].addr;
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

  res->Set(true, 
      GenerateSTPQExperiment2Queries("select count(*) from trains ", 49,  
          numpreds, numconss, munexmpl, "/home/mattia/Desktop/SQLExpr3.txt"));
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

OperatorInfo ExportMPointsOperatorInfo( "exportmpoints",
    "stream x string x string x duration x string -> int",
    "Trains addcounter[cnt, 0] feed exportmpoints(.Trip, .cnt, "
    "create_duration(0, 1000), \"flockFile.dat\")", 
    "Exports the MPoints in a format suitable for Reporting Flock",
"");

Operator ndefunit( NDefUnitOperatorInfo,
    NDefUnitVM,
    NDefUnitTM);
Operator exportmpoints( ExportMPointsOperatorInfo,
    ExportMPointsVM,
    ExportMPointsTM);

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
  AddOperator(&exportmpoints);
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
